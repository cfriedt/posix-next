#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Interactive gcovr coverage viewer for POSIX-next.

Install dependencies::

    pip install -r modules/lib/posix/scripts/ci/requirements-coverage.txt

Example::

    python3 modules/lib/posix/scripts/ci/coverageui.py \\
        --framework posix -b localhost \\
        ~/posix-next/coverage/coverage-posix.json

When ``-d`` is omitted, the workspace root is inferred by walking upward from
the coverage JSON path (and then the current directory) looking for a Zephyr
west workspace (``.west/config``).

Requires network access for Bootstrap and Plotly.js CDNs in the browser.
"""

from __future__ import annotations

import argparse
import errno
import hashlib
import json
import mimetypes
import re
import socket
import sys
import threading
import traceback
from dataclasses import dataclass
from html import escape
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from typing import Any, Dict, Iterable, List, Optional, Tuple
from urllib.parse import parse_qs, quote, unquote, urlparse

try:
    from jinja2 import DictLoader, Environment, select_autoescape
except Exception as exc:  # pragma: no cover - import guard
    raise SystemExit(
        "Missing dependency: jinja2. Install with: pip install jinja2"
    ) from exc

try:
    from pygments import highlight
    from pygments.formatters.html import HtmlFormatter
    from pygments.lexers import get_lexer_for_filename, guess_lexer
    from pygments.lexers.special import TextLexer
except Exception as exc:  # pragma: no cover - import guard
    raise SystemExit(
        "Missing dependency: pygments. Install with: pip install pygments"
    ) from exc

try:
    import plotly.graph_objects as go
except Exception as exc:  # pragma: no cover - import guard
    raise SystemExit(
        "Missing dependency: plotly. Install with: pip install plotly"
    ) from exc


POSIX_INCLUDE_REL = "modules/lib/posix/include/zephyr/posix"


def format_posix_header_display(path: str) -> str:
    """Render a workspace header path as a POSIX include, e.g. <time.h>."""
    norm = path.replace("\\", "/")
    if norm == POSIX_INCLUDE_REL:
        return path
    prefix = f"{POSIX_INCLUDE_REL}/"
    if not norm.startswith(prefix):
        return Path(path).name

    rel = norm[len(prefix) :]
    basename = Path(rel).name
    if basename.startswith("posix_") and basename.endswith(".h"):
        rel = basename[len("posix_") :]
    return f"<{rel}>"


CDN_URLS = {
    "bootstrap_css": "https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css",
    "bootstrap_js": "https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js",
    "plotly_js": "https://cdn.plot.ly/plotly-2.35.2.min.js",
}

THEME_STORAGE_KEY = "coverageui-theme"


def build_pygments_css() -> str:
    """Theme-scoped Pygments styles for readable source in light and dark mode."""
    chunks: List[str] = []
    for data_theme, style_name in (("light", "friendly"), ("dark", "monokai")):
        formatter = HtmlFormatter(style=style_name, nowrap=True)
        css = formatter.get_style_defs(
            f'html[data-theme="{data_theme}"] .source-table td.code'
        )
        # Pygments sets a flat background on td.code that hides row coverage tinting.
        css = re.sub(
            rf'html\[data-theme="{re.escape(data_theme)}"\] \.source-table td\.code \{{[^}}]*\}}\s*',
            "",
            css,
            count=1,
        )
        chunks.append(css)
    chunks.append(
        """
html[data-theme="dark"] .source-table tr.cov-hit td.code,
html[data-theme="light"] .source-table tr.cov-hit td.code { background: var(--source-hit); }
html[data-theme="dark"] .source-table tr.cov-miss td.code,
html[data-theme="light"] .source-table tr.cov-miss td.code { background: var(--source-miss); }
html[data-theme="dark"] .source-table tr.cov-neutral td.code,
html[data-theme="light"] .source-table tr.cov-neutral td.code { background: var(--source-neutral); }
html[data-theme="dark"] .source-table tr:hover td.code,
html[data-theme="light"] .source-table tr:hover td.code { background: var(--source-hover); }
"""
    )
    return "\n\n".join(chunks)


MACRO_DEFINE_RE = re.compile(r"^\s*#\s*define\s+(\w+)")
HEADER_PROTO_START_RE = re.compile(
    r"^\s*(?:static\s+|inline\s+|extern\s+)*"
    r"(?:const\s+|volatile\s+|unsigned\s+|signed\s+|struct\s+|enum\s+)*"
    r"[\w\s]+?"
    r"(?:\s*\*+\s*)*"
    r"(\w+)\s*\("
)
HEADER_PROTO_SKIP_NAMES = frozenset(
    {"if", "while", "for", "switch", "return", "sizeof", "defined", "do"}
)
C_SOURCE_FUNC_RE = re.compile(
    r"^\s*(?:static\s+|inline\s+)*"
    r"(?:const\s+|volatile\s+|unsigned\s+|signed\s+|struct\s+|enum\s+)*"
    r"[\w\s]+?"
    r"(?:\s*\*+\s*)*"
    r"(\w+)\s*\("
)

# ISO C library symbols listed in POSIX option groups but implemented by libc.
ISO_C_EXTERNAL_SYMBOLS = frozenset(
    {"remove", "rename", "tmpfile", "tmpnam"},
)

# Symbols listed without () in V4_subprofiles.html (9699919799 / 9799919799).
V4_VARIABLE_SYMBOLS = frozenset(
    {
        "daylight",
        "environ",
        "errno",
        "optarg",
        "opterr",
        "optind",
        "optopt",
        "signgam",
        "stderr",
        "stdin",
        "stdout",
        "timezone",
        "tzname",
    }
)

# V4_subprofiles uses family shorthands; expand to concrete APIs for coverage lookup.
V4_SYMBOL_EXPANSIONS: Dict[str, List[str]] = {
    "pthread_barrierattr": [
        "pthread_barrierattr_destroy",
        "pthread_barrierattr_getpshared",
        "pthread_barrierattr_init",
        "pthread_barrierattr_setpshared",
    ],
}

# Option groups from POSIX.1-2017 V4_subprofiles (9699919799/xrat/V4_subprofiles.html),
# V1_chap02 XSI Option Group composites, and POSIX Options (e.g. _POSIX_MESSAGE_PASSING)
# that align with Kconfig/codecov components but are not V4 subprofiles.
EMBED_POSIX_MANIFEST = """{"POSIX_ASYNCHRONOUS_IO":{"label":"Asynchronous Input and Output Functions","kind":"subprofile","iso_c":false,"symbols":["aio_cancel","aio_error","aio_fsync","aio_read","aio_return","aio_suspend","aio_write","lio_listio"]},"POSIX_BARRIERS":{"label":"Barriers","kind":"subprofile","iso_c":false,"symbols":["pthread_barrier_destroy","pthread_barrier_init","pthread_barrier_wait","pthread_barrierattr"]},"POSIX_C_LANG_JUMP":{"label":"Jump Functions","kind":"iso_c","iso_c":true,"symbols":["longjmp","setjmp"]},"POSIX_C_LANG_MATH":{"label":"Maths Library","kind":"iso_c","iso_c":true,"symbols":["acos","acosf","acosh","acoshf","acoshl","acosl","asin","asinf","asinh","asinhf","asinhl","asinl","atan","atan2","atan2f","atan2l","atanf","atanh","atanhf","atanhl","atanl","cabs","cabsf","cabsl","cacos","cacosf","cacosh","cacoshf","cacoshl","cacosl","carg","cargf","cargl","casin","casinf","casinh","casinhf","casinhl","casinl","catan","catanf","catanh","catanhf","catanhl","catanl","cbrt","cbrtf","cbrtl","ccos","ccosf","ccosh","ccoshf","ccoshl","ccosl","ceil","ceilf","ceill","cexp","cexpf","cexpl","cimag","cimagf","cimagl","clog","clogf","clogl","conj","conjf","conjl","copysign","copysignf","copysignl","cos","cosf","cosh","coshf","coshl","cosl","cpow","cpowf","cpowl","cproj","cprojf","cprojl","creal","crealf","creall","csin","csinf","csinh","csinhf","csinhl","csinl","csqrt","csqrtf","csqrtl","ctan","ctanf","ctanh","ctanhf","ctanhl","ctanl","erf","erfc","erfcf","erfcl","erff","erfl","exp","exp2","exp2f","exp2l","expf","expl","expm1","expm1f","expm1l","fabs","fabsf","fabsl","fdim","fdimf","fdiml","floor","floorf","floorl","fma","fmaf","fmal","fmax","fmaxf","fmaxl","fmin","fminf","fminl","fmod","fmodf","fmodl","fpclassify","frexp","frexpf","frexpl","hypot","hypotf","hypotl","ilogb","ilogbf","ilogbl","isfinite","isgreater","isgreaterequal","isinf","isless","islessequal","islessgreater","isnan","isnormal","isunordered","ldexp","ldexpf","ldexpl","lgamma","lgammaf","lgammal","llrint","llrintf","llrintl","llround","llroundf","llroundl","log","log10","log10f","log10l","log1p","log1pf","log1pl","log2","log2f","log2l","logb","logbf","logbl","logf","logl","lrint","lrintf","lrintl","lround","lroundf","lroundl","modf","modff","modfl","nan","nanf","nanl","nearbyint","nearbyintf","nearbyintl","nextafter","nextafterf","nextafterl","nexttoward","nexttowardf","nexttowardl","pow","powf","powl","remainder","remainderf","remainderl","remquo","remquof","remquol","rint","rintf","rintl","round","roundf","roundl","scalbln","scalblnf","scalblnl","scalbn","scalbnf","scalbnl","signbit","sin","sinf","sinh","sinhf","sinhl","sinl","sqrt","sqrtf","sqrtl","tan","tanf","tanh","tanhf","tanhl","tanl","tgamma","tgammaf","tgammal","trunc","truncf","truncl"]},"POSIX_C_LANG_SUPPORT":{"label":"General ISO C Library","kind":"iso_c","iso_c":true,"symbols":["abs","asctime","atof","atoi","atol","atoll","bsearch","calloc","ctime","difftime","div","feclearexcept","fegetenv","fegetexceptflag","fegetround","feholdexcept","feraiseexcept","fesetenv","fesetexceptflag","fesetround","fetestexcept","feupdateenv","free","gmtime","imaxabs","imaxdiv","isalnum","isalpha","isblank","iscntrl","isdigit","isgraph","islower","isprint","ispunct","isspace","isupper","isxdigit","labs","ldiv","llabs","lldiv","localeconv","localtime","malloc","memchr","memcmp","memcpy","memmove","memset","mktime","qsort","rand","realloc","setlocale","snprintf","sprintf","srand","sscanf","strcat","strchr","strcmp","strcoll","strcpy","strcspn","strerror","strftime","strlen","strncat","strncmp","strncpy","strpbrk","strrchr","strspn","strstr","strtod","strtof","strtoimax","strtok","strtol","strtold","strtoll","strtoul","strtoull","strtoumax","strxfrm","time","tolower","toupper","tzname","tzset","va_arg","va_copy","va_end","va_start","vsnprintf","vsprintf","vsscanf"]},"POSIX_C_LANG_SUPPORT_R":{"label":"Thread-Safe General ISO C Library","kind":"iso_c","iso_c":true,"symbols":["asctime_r","ctime_r","gmtime_r","localtime_r","qsort_r","rand_r","strerror_r","strtok_r"]},"POSIX_C_LANG_WIDE_CHAR":{"label":"Wide-Character ISO C Library","kind":"iso_c","iso_c":true,"symbols":["btowc","iswalnum","iswalpha","iswblank","iswcntrl","iswctype","iswdigit","iswgraph","iswlower","iswprint","iswpunct","iswspace","iswupper","iswxdigit","mblen","mbrlen","mbrtowc","mbsinit","mbsrtowcs","mbstowcs","mbtowc","swprintf","swscanf","towctrans","towlower","towupper","vswprintf","vswscanf","wcrtomb","wcscat","wcschr","wcscmp","wcscoll","wcscpy","wcscspn","wcsftime","wcslen","wcsncat","wcsncmp","wcsncpy","wcspbrk","wcsrchr","wcsrtombs","wcsspn","wcsstr","wcstod","wcstof","wcstoimax","wcstok","wcstol","wcstold","wcstoll","wcstombs","wcstoul","wcstoull","wcstoumax","wcsxfrm","wctob","wctomb","wctrans","wctype","wmemchr","wmemcmp","wmemcpy","wmemmove","wmemset"]},"POSIX_C_LANG_WIDE_CHAR_EXT":{"label":"Extended Wide-Character ISO C Library","kind":"iso_c","iso_c":true,"symbols":["mbsnrtowcs","wcpcpy","wcpncpy","wcscasecmp","wcsdup","wcsncasecmp","wcsnlen","wcsnrtombs"]},"POSIX_C_LIB_EXT":{"label":"General C Library Extension","kind":"subprofile","iso_c":false,"symbols":["fnmatch","getopt","getsubopt","optarg","opterr","optind","optopt","stpcpy","stpncpy","strcasecmp","strdup","strfmon","strncasecmp","strndup","strnlen"]},"POSIX_CLOCK_SELECTION":{"label":"Clock Selection","kind":"subprofile","iso_c":false,"symbols":["clock_nanosleep","pthread_condattr_getclock","pthread_condattr_setclock"]},"POSIX_DEVICE_IO":{"label":"Device Input and Output","kind":"subprofile","iso_c":false,"symbols":["FD_CLR","FD_ISSET","FD_SET","FD_ZERO","clearerr","close","fclose","fdopen","feof","ferror","fflush","fgetc","fgets","fileno","fopen","fprintf","fputc","fputs","fread","freopen","fscanf","fwrite","getc","getchar","gets","open","perror","poll","printf","pread","pselect","putc","putchar","puts","pwrite","read","scanf","select","setbuf","setvbuf","stderr","stdin","stdout","ungetc","vfprintf","vfscanf","vprintf","vscanf","write"]},"POSIX_DEVICE_IO_EXT":{"label":"Extended Device Input and Output","kind":"subprofile","iso_c":false,"symbols":["dprintf","fmemopen","open_memstream","vdprintf"]},"POSIX_DEVICE_SPECIFIC":{"label":"General Terminal","kind":"subprofile","iso_c":false,"symbols":["cfgetispeed","cfgetospeed","cfsetispeed","cfsetospeed","ctermid","isatty","tcdrain","tcflow","tcflush","tcgetattr","tcsendbreak","tcsetattr","ttyname"]},"POSIX_DEVICE_SPECIFIC_R":{"label":"Thread-Safe General Terminal","kind":"subprofile","iso_c":false,"symbols":["ttyname_r"]},"POSIX_DYNAMIC_LINKING":{"label":"Dynamic Linking","kind":"subprofile","iso_c":false,"symbols":["dlclose","dlerror","dlopen","dlsym"]},"POSIX_FD_MGMT":{"label":"File Descriptor Management","kind":"subprofile","iso_c":false,"symbols":["dup","dup2","fcntl","fgetpos","fseek","fseeko","fsetpos","ftell","ftello","ftruncate","lseek","rewind"]},"POSIX_FIFO":{"label":"FIFO","kind":"subprofile","iso_c":false,"symbols":["mkfifo"]},"POSIX_FIFO_FD":{"label":"FIFO File Descriptor Routines","kind":"subprofile","iso_c":false,"symbols":["mkfifoat","mknodat"]},"POSIX_FILE_ATTRIBUTES":{"label":"File Attributes","kind":"subprofile","iso_c":false,"symbols":["chmod","chown","fchmod","fchown","umask"]},"POSIX_FILE_ATTRIBUTES_FD":{"label":"File Attributes File Descriptor Routines","kind":"subprofile","iso_c":false,"symbols":["fchmodat","fchownat"]},"POSIX_FILE_LOCKING":{"label":"Thread-Safe Stdio Locking","kind":"subprofile","iso_c":false,"symbols":["flockfile","ftrylockfile","funlockfile","getc_unlocked","getchar_unlocked","putc_unlocked","putchar_unlocked"]},"POSIX_FILE_SYSTEM":{"label":"File System","kind":"subprofile","iso_c":false,"symbols":["access","chdir","closedir","creat","fchdir","fpathconf","fstat","fstatvfs","getcwd","link","mkdir","mkstemp","opendir","pathconf","readdir","remove","rename","rewinddir","rmdir","stat","statvfs","tmpfile","tmpnam","truncate","unlink","utime"]},"POSIX_FILE_SYSTEM_EXT":{"label":"File System Extensions","kind":"subprofile","iso_c":false,"symbols":["alphasort","dirfd","getdelim","getline","mkdtemp","scandir"]},"POSIX_FILE_SYSTEM_FD":{"label":"File System File Descriptor Routines","kind":"subprofile","iso_c":false,"symbols":["faccessat","fdopendir","fstatat","linkat","mkdirat","openat","renameat","unlinkat","utimensat"]},"POSIX_FILE_SYSTEM_GLOB":{"label":"File System Glob Expansion","kind":"subprofile","iso_c":false,"symbols":["glob","globfree"]},"POSIX_FILE_SYSTEM_R":{"label":"Thread-Safe File System","kind":"subprofile","iso_c":false,"symbols":["readdir_r"]},"POSIX_I18N":{"label":"Internationalization","kind":"subprofile","iso_c":false,"symbols":["catclose","catgets","catopen","iconv","iconv_close","iconv_open","nl_langinfo"]},"POSIX_JOB_CONTROL":{"label":"Job Control","kind":"subprofile","iso_c":false,"symbols":["setpgid","tcgetpgrp","tcsetpgrp","tcgetsid"]},"POSIX_MAPPED_FILES":{"label":"Memory Mapped Files","kind":"subprofile","iso_c":false,"symbols":["mmap","munmap"]},"POSIX_MEMORY_PROTECTION":{"label":"Memory Protection","kind":"subprofile","iso_c":false,"symbols":["mprotect"]},"POSIX_MULTI_CONCURRENT_LOCALES":{"label":"Multiple Concurrent Locales","kind":"subprofile","iso_c":false,"symbols":["duplocale","freelocale","isalnum_l","isalpha_l","isblank_l","iscntrl_l","isdigit_l","isgraph_l","islower_l","isprint_l","ispunct_l","isspace_l","isupper_l","iswalnum_l","iswalpha_l","iswblank_l","iswcntrl_l","iswctype_l","iswdigit_l","iswgraph_l","iswlower_l","iswprint_l","iswpunct_l","iswspace_l","iswupper_l","iswxdigit_l","isxdigit_l","newlocale","strcasecmp_l","strcoll_l","strfmon_l","strncasecmp_l","strxfrm_l","tolower_l","toupper_l","towctrans_l","towlower","towupper","uselocale","wcscasecmp_l","wcscoll_l","wcsncasecmp_l","wcsxfrm_l","wctrans_l","wctype_l"]},"POSIX_MULTI_PROCESS":{"label":"Multiple Processes","kind":"subprofile","iso_c":false,"symbols":["_Exit","_exit","assert","atexit","clock","execl","execle","execlp","execv","execve","execvp","exit","fork","getpgrp","getpgid","getpid","getppid","getsid","setsid","sleep","times","wait","waitid","waitpid"]},"POSIX_MULTI_PROCESS_FD":{"label":"Multiple Processes File Descriptor Routines","kind":"subprofile","iso_c":false,"symbols":["fexecve"]},"POSIX_NETWORKING":{"label":"Networking","kind":"subprofile","iso_c":false,"symbols":["accept","bind","connect","endhostent","endnetent","endprotoent","endservent","freeaddrinfo","gai_strerror","getaddrinfo","gethostent","gethostname","getnameinfo","getnetbyaddr","getnetbyname","getnetent","getpeername","getprotobyname","getprotobynumber","getprotoent","getservbyname","getservbyport","getservent","getsockname","getsockopt","htonl","htons","if_freenameindex","if_indextoname","if_nameindex","if_nametoindex","inet_addr","inet_ntoa","inet_ntop","inet_pton","listen","ntohl","ntohs","recv","recvfrom","recvmsg","send","sendmsg","sendto","sethostent","setnetent","setprotoent","setservent","setsockopt","shutdown","socket","sockatmark","socketpair"]},"POSIX_PIPE":{"label":"Pipe","kind":"subprofile","iso_c":false,"symbols":["pipe"]},"POSIX_ROBUST_MUTEXES":{"label":"Robust Mutexes","kind":"subprofile","iso_c":false,"symbols":["pthread_mutex_consistent","pthread_mutexattr_getrobust","pthread_mutexattr_setrobust"]},"POSIX_REALTIME_SIGNALS":{"label":"Realtime Signals","kind":"subprofile","iso_c":false,"symbols":["sigqueue","sigtimedwait","sigwaitinfo"]},"POSIX_REGEXP":{"label":"Regular Expressions","kind":"subprofile","iso_c":false,"symbols":["regcomp","regerror","regexec","regfree"]},"POSIX_RW_LOCKS":{"label":"Reader Writer Locks","kind":"subprofile","iso_c":false,"symbols":["pthread_rwlock_destroy","pthread_rwlock_init","pthread_rwlock_rdlock","pthread_rwlock_timedrdlock","pthread_rwlock_timedwrlock","pthread_rwlock_tryrdlock","pthread_rwlock_trywrlock","pthread_rwlock_unlock","pthread_rwlock_wrlock","pthread_rwlockattr_destroy","pthread_rwlockattr_init","pthread_rwlockattr_getpshared","pthread_rwlockattr_setpshared"]},"POSIX_SEMAPHORES":{"label":"Semaphores","kind":"subprofile","iso_c":false,"symbols":["sem_close","sem_destroy","sem_getvalue","sem_init","sem_open","sem_post","sem_timedwait","sem_trywait","sem_unlink","sem_wait"]},"POSIX_SHELL_FUNC":{"label":"Shell and Utilities","kind":"subprofile","iso_c":false,"symbols":["pclose","popen","system","wordexp","wordfree"]},"POSIX_SIGNAL_JUMP":{"label":"Signal Jump Functions","kind":"subprofile","iso_c":false,"symbols":["siglongjmp","sigsetjmp"]},"POSIX_SIGNALS":{"label":"Signals","kind":"subprofile","iso_c":false,"symbols":["abort","alarm","kill","pause","raise","sigaction","sigaddset","sigdelset","sigemptyset","sigfillset","sigismember","signal","sigpending","sigprocmask","sigsuspend","sigwait"]},"POSIX_SIGNALS_EXT":{"label":"Extended Signals","kind":"subprofile","iso_c":false,"symbols":["psignal","psiginfo","strsignal"]},"POSIX_SINGLE_PROCESS":{"label":"Single Process","kind":"subprofile","iso_c":false,"symbols":["confstr","environ","errno","getenv","setenv","sysconf","uname","unsetenv"]},"POSIX_SPIN_LOCKS":{"label":"Spin Locks","kind":"subprofile","iso_c":false,"symbols":["pthread_spin_destroy","pthread_spin_init","pthread_spin_lock","pthread_spin_trylock","pthread_spin_unlock"]},"POSIX_SYMBOLIC_LINKS":{"label":"Symbolic Links","kind":"subprofile","iso_c":false,"symbols":["lchown()","lstat","readlink","symlink"]},"POSIX_SYMBOLIC_LINKS_FD":{"label":"Symbolic Links File Descriptor Routines","kind":"subprofile","iso_c":false,"symbols":["readlinkat","symlinkat"]},"POSIX_SYSTEM_DATABASE":{"label":"System Database","kind":"subprofile","iso_c":false,"symbols":["getgrgid","getgrnam","getpwnam","getpwuid"]},"POSIX_SYSTEM_DATABASE_R":{"label":"Thread-Safe System Database","kind":"subprofile","iso_c":false,"symbols":["getgrgid_r","getgrnam_r","getpwnam_r","getpwuid_r"]},"POSIX_THREADS_BASE":{"label":"Base Threads","kind":"subprofile","iso_c":false,"symbols":["pthread_atfork","pthread_attr_destroy","pthread_attr_getdetachstate","pthread_attr_getschedparam","pthread_attr_init","pthread_attr_setdetachstate","pthread_attr_setschedparam","pthread_cancel","pthread_cleanup_pop","pthread_cleanup_push","pthread_cond_broadcast","pthread_cond_destroy","pthread_cond_init","pthread_cond_signal","pthread_cond_timedwait","pthread_cond_wait","pthread_condattr_destroy","pthread_condattr_init","pthread_create","pthread_detach","pthread_equal","pthread_exit","pthread_getspecific","pthread_join","pthread_key_create","pthread_key_delete","pthread_kill","pthread_mutex_destroy","pthread_mutex_init","pthread_mutex_lock","pthread_mutex_timedlock","pthread_mutex_trylock","pthread_mutex_unlock","pthread_mutexattr_destroy","pthread_mutexattr_init","pthread_once","pthread_self","pthread_setcancelstate","pthread_setcanceltype","pthread_setspecific","pthread_sigmask","pthread_testcancel","sched_yield"]},"POSIX_THREADS_EXT":{"label":"Extended Threads","kind":"subprofile","iso_c":false,"symbols":["pthread_attr_getguardsize","pthread_attr_setguardsize","pthread_mutexattr_gettype","pthread_mutexattr_settype"]},"POSIX_TIMERS":{"label":"Timers","kind":"subprofile","iso_c":false,"symbols":["clock_getres","clock_gettime","clock_settime","nanosleep","timer_create","timer_delete","timer_getoverrun","timer_gettime","timer_settime"]},"POSIX_USER_GROUPS":{"label":"User and Group","kind":"subprofile","iso_c":false,"symbols":["getegid","geteuid","getgid","getgroups","getlogin","getuid","setegid","seteuid","setgid","setuid"]},"POSIX_USER_GROUPS_R":{"label":"Thread-Safe User and Group","kind":"subprofile","iso_c":false,"symbols":["getlogin_r"]},"POSIX_WIDE_CHAR_DEVICE_IO":{"label":"Device Input and Output","kind":"subprofile","iso_c":false,"symbols":["fgetwc","fgetws","fputwc","fputws","fwide","fwprintf","fwscanf","getwc","getwchar","putwc","putwchar","ungetwc","vfwprintf","vfwscanf","vwprintf","vwscanf","wprintf","wscanf"]},"XSI_C_LANG_SUPPORT":{"label":"XSI General C Library","kind":"subprofile","iso_c":false,"symbols":["_tolower","_toupper","a64l","daylight","drand48","erand48","ffs","getdate","hcreate","hdestroy","hsearch","initstate","insque","isascii","jrand48","l64a","lcong48","lfind","lrand48","lsearch","memccpy","mrand48","nrand48","random","remque","seed48","setstate","signgam","srand48","srandom","strptime","swab","tdelete","tfind","timezone","toascii","tsearch","twalk"]},"XSI_DBM":{"label":"XSI Database Management","kind":"subprofile","iso_c":false,"symbols":["dbm_clearerr","dbm_close","dbm_delete","dbm_error","dbm_fetch","dbm_firstkey","dbm_nextkey","dbm_open","dbm_store"]},"XSI_DEVICE_IO":{"label":"XSI Device Input and Output","kind":"subprofile","iso_c":false,"symbols":["fmtmsg","readv","writev"]},"XSI_DEVICE_SPECIFIC":{"label":"XSI General Terminal","kind":"subprofile","iso_c":false,"symbols":["grantpt","posix_openpt","ptsname","unlockpt"]},"XSI_FILE_SYSTEM":{"label":"XSI File System","kind":"subprofile","iso_c":false,"symbols":["basename","dirname","ftw","lockf","mknod","nftw","realpath","seekdir","sync","telldir","tempnam"]},"XSI_IPC":{"label":"XSI Interprocess Communication","kind":"subprofile","iso_c":false,"symbols":["ftok","msgctl","msgget","msgrcv","msgsnd","semctl","semget","semop","shmat","shmctl","shmdt","shmget"]},"XSI_JUMP":{"label":"XSI Jump Functions","kind":"subprofile","iso_c":false,"symbols":["_longjmp","_setjmp"]},"XSI_MATH":{"label":"XSI Maths Library","kind":"subprofile","iso_c":false,"symbols":["j0","j1","jn","y0","y1","yn"]},"XSI_MULTI_PROCESS":{"label":"XSI Multiple Process","kind":"subprofile","iso_c":false,"symbols":["getpriority","getrlimit","getrusage","nice","setpgrp","setpriority","setrlimit","ulimit"]},"XSI_SIGNALS":{"label":"XSI Signal","kind":"subprofile","iso_c":false,"symbols":["killpg","sigaltstack","sighold","sigignore","siginterrupt","sigpause","sigrelse","sigset"]},"XSI_SINGLE_PROCESS":{"label":"XSI Single Process","kind":"subprofile","iso_c":false,"symbols":["gethostid","gettimeofday","putenv"]},"XSI_SYSTEM_DATABASE":{"label":"XSI System Database","kind":"subprofile","iso_c":false,"symbols":["endgrent","endpwent","getgrent","getpwent","setgrent","setpwent"]},"XSI_SYSTEM_LOGGING":{"label":"XSI System Logging","kind":"subprofile","iso_c":false,"symbols":["closelog","openlog","setlogmask","syslog"]},"XSI_THREADS_EXT":{"label":"XSI Threads Extensions","kind":"subprofile","iso_c":false,"symbols":["pthread_attr_getinheritsched","pthread_attr_getschedpolicy","pthread_attr_getscope","pthread_attr_getstack","pthread_attr_setinheritsched","pthread_attr_setschedpolicy","pthread_attr_setscope","pthread_attr_setstack","pthread_getconcurrency","pthread_getschedparam","pthread_setconcurrency","pthread_setschedparam","pthread_setschedprio","pthread_attr_getstacksize","pthread_attr_setstacksize"]},"XSI_TIMERS":{"label":"XSI Timers","kind":"subprofile","iso_c":false,"symbols":["getitimer","setitimer"]},"XSI_USER_GROUPS":{"label":"XSI User and Group","kind":"subprofile","iso_c":false,"symbols":["endutxent","getresgid","getresuid","getutxent","getutxid","getutxline","pututxline","setregid","setresgid","setresuid","setreuid","setutxent"]},"XSI_WIDE_CHAR":{"label":"XSI Wide-Character Library","kind":"subprofile","iso_c":false,"symbols":["wcswidth","wcwidth"]},"XSI_CRYPT":{"label":"Encryption","kind":"xsi_composite","iso_c":false,"symbols":["crypt","encrypt","setkey"]},"XSI_STREAMS":{"label":"XSI STREAMS","kind":"xsi_composite","iso_c":false,"symbols":["fattach","fdetach","getmsg","getpmsg","ioctl","isastream","putmsg","putpmsg"]},"XSI_UNIX":{"label":"XSI","kind":"xsi_composite","iso_c":false,"symbols":["mmap","msync","munmap"]},"POSIX_NON_PORTABLE":{"label":"Non-Portable Extensions","kind":"subprofile","iso_c":false,"symbols":["pthread_getname_np","pthread_setname_np","pthread_timedjoin_np","pthread_tryjoin_np"]},"POSIX_PRIORITY_SCHEDULING":{"label":"Priority Scheduling","kind":"option","iso_c":false,"symbols":["sched_get_priority_max","sched_get_priority_min","sched_getparam","sched_getscheduler","sched_rr_get_interval","sched_setparam","sched_setscheduler"]},"POSIX_MESSAGE_PASSING":{"label":"Message Passing","kind":"option","iso_c":false,"symbols":["mq_close","mq_getattr","mq_notify","mq_open","mq_receive","mq_send","mq_setattr","mq_timedreceive","mq_timedsend","mq_unlink"]}}"""

EMBED_CSS = r"""
html[data-theme="dark"] {
  --bg: #0f1117;
  --surface: #171a23;
  --surface-2: #1f2431;
  --text: #edf1ff;
  --muted: #97a3be;
  --border: #2e3750;
  --link: #85b8ff;
  --link-hover: #b8d8ff;
  --green: #39d98a;
  --yellow: #ffc857;
  --red: #ff5d73;
  --external: #7f8aa7;
  --unknown: #56607a;
  --ring-bg: #2a3245;
  --ring-center: #141926;
  --source-bg: #121622;
  --source-border: #2d3550;
  --source-ln: #7f90b3;
  --source-line-border: #273049;
  --source-hit: rgba(57, 217, 138, 0.22);
  --source-miss: rgba(255, 93, 115, 0.24);
  --source-neutral: rgba(127, 138, 167, 0.06);
  --source-hover: rgba(128, 170, 255, 0.14);
  --navbar-bg: rgba(19, 22, 31, 0.86);
  --navbar-border: #283149;
  --hero-bg: linear-gradient(130deg, rgba(66, 128, 255, 0.16), rgba(59, 221, 160, 0.10) 45%, rgba(255, 164, 72, 0.08));
  --plot-bg: #131926;
}
html[data-theme="light"] {
  --bg: #eef1f8;
  --surface: #ffffff;
  --surface-2: #f4f6fb;
  --text: #1a2233;
  --muted: #5c677d;
  --border: #d5dbe8;
  --link: #0b5ed7;
  --link-hover: #084298;
  --green: #198754;
  --yellow: #cc9a06;
  --red: #dc3545;
  --external: #6c757d;
  --unknown: #868e96;
  --ring-bg: #dde3ef;
  --ring-center: #ffffff;
  --source-bg: #ffffff;
  --source-border: #d5dbe8;
  --source-ln: #6c757d;
  --source-line-border: #e3e8f2;
  --source-hit: rgba(25, 135, 84, 0.22);
  --source-miss: rgba(220, 53, 69, 0.18);
  --source-neutral: rgba(108, 117, 125, 0.06);
  --source-hover: rgba(13, 110, 253, 0.10);
  --navbar-bg: rgba(255, 255, 255, 0.92);
  --navbar-border: #d5dbe8;
  --hero-bg: linear-gradient(130deg, rgba(13, 110, 253, 0.08), rgba(25, 135, 84, 0.07) 45%, rgba(255, 193, 7, 0.08));
  --plot-bg: #f8f9fc;
}
html, body { background: radial-gradient(circle at 0 0, var(--surface-2), var(--bg) 50%); color: var(--text); min-height: 100%; }
.navbar { background: var(--navbar-bg) !important; border-bottom: 1px solid var(--navbar-border); backdrop-filter: blur(10px); }
.navbar-brand, .navbar-nav .nav-link { color: var(--text) !important; }
.navbar-nav .nav-link.active { color: var(--link) !important; font-weight: 600; }
.navbar-brand { letter-spacing: 0.03em; font-weight: 700; }
.navbar-toggler { border-color: var(--border); }
html[data-theme="dark"] .navbar-toggler-icon { filter: invert(1); }
.card { background: linear-gradient(180deg, var(--surface), var(--surface-2)); border: 1px solid var(--border); box-shadow: 0 8px 22px rgba(0,0,0,.08); color: var(--text); }
.card-header { border-bottom: 1px solid var(--border); color: var(--text); font-weight: 600; background: transparent; }
.table { color: var(--text); --bs-table-bg: transparent; }
.table-hover > tbody > tr:hover > * { color: var(--text); background-color: var(--source-hover); }
.table td, .table th { border-color: var(--border); vertical-align: middle; }
a { color: var(--link); text-decoration: none; }
a:hover { color: var(--link-hover); }
.muted { color: var(--muted); }
.badge-soft { border: 1px solid var(--border); background: var(--surface-2); color: var(--text); }
.cov-green { color: var(--green); }
.cov-yellow { color: var(--yellow); }
.cov-red { color: var(--red); }
.cov-external { color: var(--external); }
.cov-unknown { color: var(--unknown); }
.table .row-green { --bs-table-accent-bg: rgba(57, 217, 138, 0.07); }
html[data-theme="light"] .table .row-green { --bs-table-accent-bg: rgba(25, 135, 84, 0.08); }
.table .row-yellow { --bs-table-accent-bg: rgba(255, 200, 87, 0.08); }
html[data-theme="light"] .table .row-yellow { --bs-table-accent-bg: rgba(255, 193, 7, 0.10); }
.table .row-red { --bs-table-accent-bg: rgba(255, 93, 115, 0.09); }
html[data-theme="light"] .table .row-red { --bs-table-accent-bg: rgba(220, 53, 69, 0.08); }
.table .row-external { --bs-table-accent-bg: rgba(127, 138, 167, 0.08); }
.table .row-unknown { --bs-table-accent-bg: rgba(86, 96, 122, 0.08); }
.coverage-rings { display: flex; flex-wrap: wrap; gap: 14px; align-items: center; }
.ring-wrap { display: flex; gap: 8px; align-items: center; min-width: 90px; }
.ring { width: 52px; height: 52px; border-radius: 999px; position: relative; background: var(--ring-bg); box-shadow: inset 0 0 0 1px var(--border); }
.ring::after { content: ""; position: absolute; inset: 8px; border-radius: 999px; background: var(--ring-center); box-shadow: inset 0 0 0 1px var(--border); }
.ring span { position: absolute; inset: 0; display: grid; place-items: center; z-index: 2; font-weight: 700; font-size: .73rem; color: var(--text); }
.ring-label { color: var(--muted); font-size: .78rem; text-transform: uppercase; letter-spacing: .05em; }
.pathcrumb .breadcrumb { --bs-breadcrumb-divider-color: var(--muted); }
.pathcrumb .breadcrumb-item + .breadcrumb-item::before { color: var(--muted); }
.hero { padding: 1rem 1.2rem; border: 1px solid var(--border); border-radius: .8rem; background: var(--hero-bg); }
.hero h1, .hero h2 { margin-bottom: .4rem; color: var(--text); }
.smallcaps { letter-spacing: .08em; text-transform: uppercase; font-size: .76rem; color: var(--muted); }
.source-pane { border: 1px solid var(--source-border); border-radius: .7rem; overflow: auto; background: var(--source-bg); max-height: 78vh; }
.source-table { width: 100%; border-collapse: collapse; font-family: ui-monospace, SFMono-Regular, Menlo, Consolas, "Liberation Mono", monospace; font-size: .82rem; line-height: 1.45; }
.source-table tr td { border-top: 1px solid var(--source-line-border); }
.source-table td.ln { width: 82px; text-align: right; color: var(--source-ln); padding: 0 10px 0 8px; user-select: none; white-space: nowrap; border-right: 1px solid var(--source-line-border); vertical-align: top; background: var(--surface-2); }
.source-table td.ln a { color: inherit; }
.source-table td.code { padding: 0 10px; white-space: pre; background: transparent; }
.source-table td.proto-wheels { width: 118px; padding: 4px 8px; vertical-align: top; background: var(--surface-2); border-left: 1px solid var(--source-line-border); }
.coverage-rings.rings-inline { gap: 6px; flex-wrap: nowrap; }
.coverage-rings.rings-inline .ring-wrap { min-width: 0; gap: 4px; }
.coverage-rings.rings-inline .ring { width: 34px; height: 34px; }
.coverage-rings.rings-inline .ring::after { inset: 6px; }
.coverage-rings.rings-inline .ring span { font-size: .62rem; }
.coverage-rings.rings-inline .ring-label { font-size: .62rem; letter-spacing: .03em; }
.plot-wrap { min-height: 510px; border: 1px solid var(--border); border-radius: .8rem; background: var(--plot-bg); }
.footer-note { color: var(--muted); font-size: .84rem; }
.pill { font-size: .78rem; padding: .24rem .48rem; border-radius: .6rem; border: 1px solid var(--border); background: var(--surface-2); color: var(--text); }
.toast-wrap { position: fixed; right: 12px; bottom: 10px; z-index: 10000; }
.btn-outline-theme { color: var(--text); border-color: var(--border); }
.btn-outline-theme:hover { color: var(--text); background: var(--surface-2); border-color: var(--border); }
.theme-toggle .dropdown-menu { background: var(--surface); border-color: var(--border); }
.theme-toggle .dropdown-item { color: var(--text); }
.theme-toggle .dropdown-item:hover, .theme-toggle .dropdown-item:focus { background: var(--surface-2); color: var(--text); }
.theme-toggle .dropdown-item.active { background: var(--source-hover); color: var(--link); font-weight: 600; }
.cov-sort-th { cursor: pointer; user-select: none; white-space: nowrap; }
.cov-sort-th:hover { color: var(--link); }
.cov-sort-th::after {
  content: "↕";
  opacity: 0.35;
  margin-left: 0.35rem;
  font-size: 0.75em;
}
.cov-sort-th.sort-asc::after { content: "▲"; opacity: 0.95; }
.cov-sort-th.sort-desc::after { content: "▼"; opacity: 0.95; }
.total-line-coverage { display: flex; flex-direction: column; align-items: center; gap: 0.25rem; }
.total-line-coverage .total-line-pct {
  font-size: clamp(2.4rem, 5vw, 3.6rem);
  font-weight: 800;
  line-height: 1;
  letter-spacing: -0.03em;
  font-variant-numeric: tabular-nums;
}
.total-line-coverage .total-line-fraction {
  font-size: 0.78rem;
  letter-spacing: 0.02em;
}
.total-line-coverage--card {
  padding: 0.35rem 0 0.85rem;
  border-bottom: 1px solid var(--border);
  margin-bottom: 0.85rem;
}
.total-line-coverage--hero {
  align-items: flex-end;
  text-align: right;
  flex-shrink: 0;
  padding: 0.15rem 0.25rem;
}
"""

EMBED_JS = r"""
window.COVUI = window.COVUI || {};

window.COVUI.humanPct = function(v) {
  if (v === null || v === undefined || Number.isNaN(Number(v))) return "n/a";
  return `${Number(v).toFixed(1)}%`;
};

window.COVUI.classToBootstrap = function(cls) {
  if (cls === "green") return "success";
  if (cls === "yellow") return "warning";
  if (cls === "red") return "danger";
  if (cls === "external") return "secondary";
  return "dark";
};

window.COVUI.toast = function(msg) {
  const wrap = document.getElementById("toast-wrap");
  if (!wrap) return;
  const el = document.createElement("div");
  el.className = "alert alert-info py-2 px-3 mb-2";
  el.textContent = msg;
  wrap.appendChild(el);
  setTimeout(() => el.remove(), 2500);
};

window.COVUI.initTreemap = async function(targetId, endpoint, controlsId) {
  const mount = document.getElementById(targetId);
  if (!mount) return;
  try {
    const response = await fetch(endpoint, { headers: { "Accept": "application/json" } });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const payload = await response.json();
    const meta = payload.meta || {};
    const sizeModes = meta.size_modes || {};
    const storageKey = "coverageui-treemap-size";
    let mode = localStorage.getItem(storageKey) || meta.default_size_mode || "percent";
    if (!sizeModes[mode]) {
      mode = meta.default_size_mode || "percent";
    }

    const trace = payload.data && payload.data[0];
    if (!trace) throw new Error("treemap payload missing trace data");

    function valuesFor(selectedMode) {
      return sizeModes[selectedMode] || trace.values;
    }

    trace.values = valuesFor(mode);
    Plotly.newPlot(mount, payload.data, payload.layout, { responsive: true, displaylogo: false });

    const controls = controlsId ? document.getElementById(controlsId) : null;
    if (controls) {
      const radios = controls.querySelectorAll('input[type="radio"][name]');
      radios.forEach(function(radio) {
        radio.checked = radio.value === mode;
        radio.addEventListener("change", function() {
          if (!radio.checked) return;
          mode = radio.value;
          localStorage.setItem(storageKey, mode);
          Plotly.restyle(mount, { values: [valuesFor(mode)] }, [0]);
        });
      });
    }

    mount.on("plotly_click", function(e) {
      const point = e.points && e.points[0];
      if (!point || !point.customdata) return;
      const gid = point.customdata[0];
      if (gid) window.location.href = `/posix/${encodeURIComponent(gid)}`;
    });
  } catch (err) {
    console.error(err);
    window.COVUI.toast(`Failed loading treemap: ${err.message}`);
  }
};

window.COVUI.initSortableTable = function(tableId) {
  const table = document.getElementById(tableId);
  if (!table) return;
  const tbody = table.querySelector("tbody");
  const headers = table.querySelectorAll("th.cov-sort-th");
  if (!tbody || !headers.length) return;

  let sortKey = "group";
  let sortDir = 1;

  function metricValue(row, key) {
    const raw = row.dataset["sort" + key.charAt(0).toUpperCase() + key.slice(1)];
    if (key === "group") return (raw || "").toLowerCase();
    if (raw === "" || raw === undefined) return Number.NEGATIVE_INFINITY;
    const num = Number(raw);
    return Number.isFinite(num) ? num : Number.NEGATIVE_INFINITY;
  }

  function compareRows(a, b) {
    const av = metricValue(a, sortKey);
    const bv = metricValue(b, sortKey);
    let cmp = 0;
    if (typeof av === "string") {
      cmp = av.localeCompare(bv);
    } else {
      cmp = av === bv ? 0 : (av < bv ? -1 : 1);
    }
    if (cmp === 0) {
      cmp = metricValue(a, "group").localeCompare(metricValue(b, "group"));
    }
    return cmp * sortDir;
  }

  function applySort() {
    const rows = Array.from(tbody.querySelectorAll("tr"));
    rows.sort(compareRows);
    rows.forEach(function(row) { tbody.appendChild(row); });
    headers.forEach(function(th) {
      th.classList.remove("sort-asc", "sort-desc");
      if (th.dataset.sort === sortKey) {
        th.classList.add(sortDir === 1 ? "sort-asc" : "sort-desc");
      }
    });
  }

  headers.forEach(function(th) {
    th.addEventListener("click", function() {
      const key = th.dataset.sort;
      if (!key) return;
      if (sortKey === key) {
        sortDir *= -1;
      } else {
        sortKey = key;
        sortDir = key === "group" ? 1 : -1;
      }
      applySort();
    });
  });

  applySort();
};

window.COVUI.THEME_KEY = "coverageui-theme";

window.COVUI.resolveTheme = function(mode) {
  if (mode === "system") {
    return window.matchMedia("(prefers-color-scheme: dark)").matches ? "dark" : "light";
  }
  return mode === "dark" ? "dark" : "light";
};

window.COVUI.applyTheme = function(mode) {
  const resolved = window.COVUI.resolveTheme(mode);
  document.documentElement.setAttribute("data-theme", resolved);
  document.documentElement.setAttribute("data-theme-mode", mode);
  const label = document.getElementById("theme-btn-label");
  if (label) {
    label.textContent = mode.charAt(0).toUpperCase() + mode.slice(1);
  }
  document.querySelectorAll(".theme-opt").forEach(function(el) {
    el.classList.toggle("active", el.dataset.theme === mode);
  });
};

window.COVUI.initTheme = function() {
  const stored = localStorage.getItem(window.COVUI.THEME_KEY) || "system";
  window.COVUI.applyTheme(stored);
  document.querySelectorAll(".theme-opt").forEach(function(el) {
    el.addEventListener("click", function(e) {
      e.preventDefault();
      const mode = el.dataset.theme;
      localStorage.setItem(window.COVUI.THEME_KEY, mode);
      window.COVUI.applyTheme(mode);
    });
  });
  window.matchMedia("(prefers-color-scheme: dark)").addEventListener("change", function() {
    const mode = localStorage.getItem(window.COVUI.THEME_KEY) || "system";
    if (mode === "system") window.COVUI.applyTheme("system");
  });
};

document.addEventListener("DOMContentLoaded", function() {
  window.COVUI.initTheme();
});
"""

TEMPLATES: Dict[str, str] = {
    "macros.html": """
{% macro coverage_label(value, cls) -%}
<span class="pill cov-{{ cls }}">{% if value is not none %}{{ "%.1f"|format(value) }}%{% else %}n/a{% endif %}</span>
{%- endmacro %}

{% macro total_line_coverage(stats, line_cov, line_total, variant="card") -%}
  {% set cls = stats.coverage_class(metric='line', green=green_threshold, yellow=yellow_threshold) %}
  <div class="total-line-coverage total-line-coverage--{{ variant }}">
    <div class="total-line-pct cov-{{ cls }}">
      {% if stats.line_pct is not none %}{{ "%.1f"|format(stats.line_pct) }}%{% else %}n/a{% endif %}
    </div>
    <div class="total-line-fraction muted">
      {% if line_total > 0 %}{{ "{:,}".format(line_cov) }} / {{ "{:,}".format(line_total) }} lines covered{% else %}no instrumented lines{% endif %}
    </div>
  </div>
{%- endmacro %}

{% macro coverage_wheels(stats, color_for) -%}
<div class="coverage-rings">
  {% set metrics = [("Lines", stats.line_pct), ("Funcs", stats.func_pct), ("Branch", stats.branch_pct)] %}
  {% set available = metrics | selectattr(1) | list %}
  {% set mode = "triple" if available|length >= 3 else ("dual" if available|length == 2 else "single") %}
  {% for title, value in metrics if value is not none %}
    {% set cls = stats.coverage_class(metric=title|lower, green=green_threshold, yellow=yellow_threshold) %}
    <div class="ring-wrap {{ mode }}">
      <div class="ring" style="background: conic-gradient({{ color_for(cls) }} {{ value }}%, var(--ring-bg) {{ value }}%);">
        <span>{{ "%.0f"|format(value) }}</span>
      </div>
      <div class="ring-label">{{ title }}</div>
    </div>
  {% endfor %}
</div>
{%- endmacro %}

{% macro coverage_wheels_inline(stats, color_for) -%}
<div class="coverage-rings rings-inline">
  {% for title, value in [("Lines", stats.line_pct), ("Branch", stats.branch_pct)] if value is not none %}
    {% set cls = stats.coverage_class(metric=title|lower, green=green_threshold, yellow=yellow_threshold) %}
    <div class="ring-wrap dual">
      <div class="ring" style="background: conic-gradient({{ color_for(cls) }} {{ value }}%, var(--ring-bg) {{ value }}%);">
        <span>{{ "%.0f"|format(value) }}</span>
      </div>
      <div class="ring-label">{{ title }}</div>
    </div>
  {% endfor %}
</div>
{%- endmacro %}

{% macro coverage_row(label, stats, href=None, extra="", display=None, sortable=false) -%}
  {% set shown = display if display else label %}
  {% set cls = stats.coverage_class(metric='line', green=green_threshold, yellow=yellow_threshold) %}
  <tr class="row-{{ cls }}"
      {%- if sortable %}
      data-sort-group="{{ label }}"
      data-sort-lines="{{ stats.line_pct if stats.line_pct is not none else '' }}"
      data-sort-funcs="{{ stats.func_pct if stats.func_pct is not none else '' }}"
      data-sort-branch="{{ stats.branch_pct if stats.branch_pct is not none else '' }}"
      {%- endif %}>
    <td>
      {% if href %}<a href="{{ href }}"><code>{{ shown }}</code></a>{% else %}<code>{{ shown }}</code>{% endif %}
      {% if extra %}<div class="muted small">{{ extra }}</div>{% endif %}
    </td>
    <td>{{ coverage_label(stats.line_pct, cls) }}</td>
    <td>{{ coverage_label(stats.func_pct, stats.coverage_class(metric='func', green=green_threshold, yellow=yellow_threshold)) }}</td>
    <td>{{ coverage_label(stats.branch_pct, stats.coverage_class(metric='branch', green=green_threshold, yellow=yellow_threshold)) }}</td>
  </tr>
{%- endmacro %}

{% macro tree_row(label, stats, href=None, kind="", display=None) -%}
  {% set shown = display if display else label %}
  {% set cls = stats.coverage_class(metric='line', green=green_threshold, yellow=yellow_threshold) %}
  <tr class="row-{{ cls }}">
    <td>
      {% if href %}<a href="{{ href }}"><code>{{ shown }}</code></a>{% else %}<code>{{ shown }}</code>{% endif %}
    </td>
    <td class="muted small">{{ kind }}</td>
    <td>{{ coverage_label(stats.line_pct, cls) }}</td>
    <td>{{ coverage_label(stats.func_pct, stats.coverage_class(metric='func', green=green_threshold, yellow=yellow_threshold)) }}</td>
    <td>{{ coverage_label(stats.branch_pct, stats.coverage_class(metric='branch', green=green_threshold, yellow=yellow_threshold)) }}</td>
  </tr>
{%- endmacro %}
""",
    "base.html": """
{% import "macros.html" as m %}
<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <title>{{ title }} · coverageui</title>
    <script>
    (function(){
      var k="coverageui-theme",s=localStorage.getItem(k)||"system",t=s==="system"?(window.matchMedia("(prefers-color-scheme:dark)").matches?"dark":"light"):s;
      document.documentElement.setAttribute("data-theme",t);
      document.documentElement.setAttribute("data-theme-mode",s);
    })();
    </script>
    <link href="{{ cdn.bootstrap_css }}" rel="stylesheet">
    <style>{{ embed_css | safe }}</style>
    <style>{{ pygments_css | safe }}</style>
  </head>
  <body>
    <nav class="navbar navbar-expand-lg sticky-top">
      <div class="container-fluid px-3">
        <a class="navbar-brand" href="/">coverageui</a>
        <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#navc">
          <span class="navbar-toggler-icon"></span>
        </button>
        <div class="collapse navbar-collapse" id="navc">
          <ul class="navbar-nav me-auto mb-2 mb-lg-0">
            <li class="nav-item"><a class="nav-link {% if nav == 'home' %}active{% endif %}" href="/">Overview</a></li>
            <li class="nav-item"><a class="nav-link {% if nav == 'tree' %}active{% endif %}" href="/tree">Tree</a></li>
            <li class="nav-item"><a class="nav-link {% if nav == 'posix' %}active{% endif %}" href="/posix">POSIX</a></li>
          </ul>
          <div class="d-flex align-items-center gap-2 ms-lg-2">
            <div class="dropdown theme-toggle">
              <button class="btn btn-sm btn-outline-theme dropdown-toggle" type="button" data-bs-toggle="dropdown" aria-expanded="false">
                <span id="theme-btn-label">System</span>
              </button>
              <ul class="dropdown-menu dropdown-menu-end">
                <li><a class="dropdown-item theme-opt" href="#" data-theme="light">Light</a></li>
                <li><a class="dropdown-item theme-opt" href="#" data-theme="dark">Dark</a></li>
                <li><a class="dropdown-item theme-opt active" href="#" data-theme="system">System</a></li>
              </ul>
            </div>
            <span class="badge badge-soft">{{ framework|upper }}</span>
            {% if coverage_file_href %}
            <a class="small muted d-none d-xl-inline" href="/file?path={{ coverage_file_href|urlencode }}">{{ coverage_file }}</a>
            {% else %}
            <span class="small muted d-none d-xl-inline">{{ coverage_file }}</span>
            {% endif %}
          </div>
        </div>
      </div>
    </nav>
    <main class="container-fluid py-3 px-3 px-lg-4">
      {% block body %}{% endblock %}
    </main>
    <div id="toast-wrap" class="toast-wrap"></div>
    <script src="{{ cdn.bootstrap_js }}"></script>
    <script src="{{ cdn.plotly_js }}"></script>
    <script>{{ embed_js | safe }}</script>
    {% block scripts %}{% endblock %}
  </body>
</html>
""",
    "index.html": """
{% extends "base.html" %}
{% import "macros.html" as m %}
{% block body %}
<section class="hero mb-3">
  <div class="smallcaps">Interactive Coverage</div>
  <h1 class="h3">Workspace coverage dashboard</h1>
  <p class="mb-0 muted">Explore file-level and source-level execution data from gcovr JSON.</p>
</section>
<div class="row g-3">
  <div class="col-lg-4">
    <div class="card h-100">
      <div class="card-header">Overall Coverage</div>
      <div class="card-body">
        {{ m.total_line_coverage(overall, overall_line_cov, overall_line_total, "card")|safe }}
        {{ m.coverage_wheels(overall, color_for)|safe }}
      </div>
    </div>
  </div>
  <div class="col-lg-8">
    <div class="card h-100">
      <div class="card-header">Top Files by Missed Lines</div>
      <div class="card-body table-responsive">
        <table class="table table-sm table-hover align-middle mb-0">
          <thead><tr><th>Path</th><th>Lines</th><th>Funcs</th><th>Branch</th></tr></thead>
          <tbody>
            {% for row in worst_files %}
              {{ m.coverage_row(row.path, row.stats, "/file?path=" ~ row.path|urlencode, row.reason)|safe }}
            {% endfor %}
          </tbody>
        </table>
      </div>
    </div>
  </div>
</div>
{% endblock %}
""",
    "tree.html": """
{% extends "base.html" %}
{% import "macros.html" as m %}
{% block body %}
<section class="hero mb-3">
  <div class="smallcaps">Tree Browser</div>
  <h2 class="h4 mb-0">{{ current_path or "." }}</h2>
</section>
<div class="pathcrumb mb-2">
  <nav aria-label="breadcrumb">
    <ol class="breadcrumb">
      {% for crumb in breadcrumbs %}
      <li class="breadcrumb-item">
        <a href="/tree?path={{ crumb.path|urlencode }}">{{ crumb.name }}</a>
      </li>
      {% endfor %}
    </ol>
  </nav>
</div>
<div class="card">
  <div class="card-header">Directories and files</div>
  <div class="card-body table-responsive">
    <table class="table table-sm table-hover mb-0">
      <thead><tr><th>Name</th><th>Type</th><th>Lines</th><th>Funcs</th><th>Branch</th></tr></thead>
      <tbody>
        {% for row in directories %}
          {{ m.tree_row(row.name, row.stats, "/tree?path=" ~ row.path|urlencode, "directory", row.display_name)|safe }}
        {% endfor %}
        {% for row in files %}
          {{ m.tree_row(row.name, row.stats, "/file?path=" ~ row.path|urlencode, "file", row.display_name)|safe }}
        {% endfor %}
      </tbody>
    </table>
  </div>
</div>
{% endblock %}
""",
    "file.html": """
{% extends "base.html" %}
{% import "macros.html" as m %}
{% block body %}
<section class="hero mb-3">
  <div class="smallcaps">File Coverage</div>
  <h2 class="h4 mb-1"><code>{{ display_path or path }}</code></h2>
  <div class="muted">Resolved source: <code>{{ source_path }}</code></div>
</section>
<div class="row g-3">
  <div class="col-lg-4">
    <div class="card h-100">
      <div class="card-header">Coverage</div>
      <div class="card-body">
        {{ m.coverage_wheels(stats, color_for)|safe }}
      </div>
    </div>
  </div>
  <div class="col-lg-8">
    <div class="card h-100">
      <div class="card-header">Actions</div>
      <div class="card-body d-flex gap-2 flex-wrap">
        <a class="btn btn-primary btn-sm" href="/source?path={{ path|urlencode }}">Open highlighted source</a>
        <a class="btn btn-outline-theme btn-sm" href="/api/source?path={{ path|urlencode }}">Source JSON API</a>
        <a class="btn btn-outline-theme btn-sm" href="/tree?path={{ parent_path|urlencode }}">Back to directory</a>
      </div>
    </div>
  </div>
</div>
{% if prototypes %}
<div class="card mt-3">
  <div class="card-header">Function prototypes</div>
  <div class="card-body table-responsive">
    <table class="table table-sm table-hover mb-0">
      <thead><tr><th>Line</th><th>Symbol</th><th>Line · Branch</th><th>Implementation</th></tr></thead>
      <tbody>
      {% for p in prototypes %}
        {% set cls = p.stats.coverage_class(metric='line', green=green_threshold, yellow=yellow_threshold) %}
        <tr class="row-{{ cls }}">
          <td><a href="/source?path={{ path|urlencode }}#L{{ p.lineno }}">{{ p.lineno }}</a></td>
          <td><code>{{ p.name }}</code></td>
          <td>{{ m.coverage_wheels(p.stats, color_for)|safe }}</td>
          <td>{% if p.impl_href %}<a href="{{ p.impl_href }}">source</a>{% else %}<span class="muted">n/a</span>{% endif %}</td>
        </tr>
      {% endfor %}
      </tbody>
    </table>
  </div>
</div>
{% endif %}
{% endblock %}
""",
    "source.html": """
{% extends "base.html" %}
{% import "macros.html" as m %}
{% block body %}
<section class="hero mb-3">
  <div class="d-flex flex-wrap align-items-start justify-content-between gap-3">
    <div>
      <div class="smallcaps">Annotated Source</div>
      <h2 class="h4 mb-1"><code>{{ display_path or path }}</code></h2>
      <p class="mb-0 muted">Line rows are tinted by execution: green hit, red miss, gray neutral/excluded.{% if show_prototype_wheels %} Prototype rows include line and branch coverage wheels.{% endif %}</p>
    </div>
    {{ m.total_line_coverage(stats, line_cov, line_total, "hero")|safe }}
  </div>
</section>
<div class="source-pane">
  {{ source_table|safe }}
</div>
{% endblock %}
""",
    "_inline_wheels.html": """
{% import "macros.html" as m %}
{{ m.coverage_wheels_inline(stats, color_for)|safe }}
""",
    "posix.html": """
{% extends "base.html" %}
{% import "macros.html" as m %}
{% block body %}
<section class="hero mb-3">
  <div class="d-flex flex-wrap align-items-start justify-content-between gap-3">
    <div>
      <div class="smallcaps">POSIX Framework</div>
      <h2 class="h4 mb-2">Coverage by option group</h2>
      <p class="mb-0 muted">Treemap tile colour is line coverage; use the size control to switch between coverage % and instrumented line count. External ISO C groups appear only in the table.</p>
    </div>
    {{ m.total_line_coverage(overall, overall_line_cov, overall_line_total, "hero")|safe }}
  </div>
</section>
<div class="row g-3">
  <div class="col-xl-7">
    <div class="plot-wrap p-2">
      <div class="d-flex justify-content-end align-items-center mb-2 px-1">
        <div class="btn-group btn-group-sm" role="group" id="posix-treemap-controls" aria-label="Treemap tile size">
          <input type="radio" class="btn-check" name="posix-treemap-size" id="posix-treemap-size-pct" value="percent" autocomplete="off" checked>
          <label class="btn btn-outline-theme" for="posix-treemap-size-pct">Size: coverage %</label>
          <input type="radio" class="btn-check" name="posix-treemap-size" id="posix-treemap-size-lines" value="lines" autocomplete="off">
          <label class="btn btn-outline-theme" for="posix-treemap-size-lines">Size: line count</label>
        </div>
      </div>
      <div id="posix-treemap" style="height: 500px;"></div>
    </div>
  </div>
  <div class="col-xl-5">
    <div class="card h-100">
      <div class="card-header">Option Groups</div>
      <div class="card-body table-responsive">
        <table class="table table-sm table-hover mb-0" id="posix-option-groups">
          <thead><tr>
            <th class="cov-sort-th" data-sort="group">Group</th>
            <th class="cov-sort-th" data-sort="lines">Lines</th>
            <th class="cov-sort-th" data-sort="funcs">Funcs</th>
            <th class="cov-sort-th" data-sort="branch">Branch</th>
          </tr></thead>
          <tbody>
            {% for g in groups %}
              {{ m.coverage_row(g.gid, g.stats, "/posix/" ~ g.gid|urlencode, g.label, sortable=true)|safe }}
            {% endfor %}
          </tbody>
        </table>
      </div>
    </div>
  </div>
</div>
{% if warnings %}
<div class="card mt-3">
  <div class="card-header">Resolution Warnings</div>
  <div class="card-body">
    <ul class="mb-0">
      {% for w in warnings %}
      <li><code>{{ w.symbol }}</code>: {{ w.message }}</li>
      {% endfor %}
    </ul>
  </div>
</div>
{% endif %}
{% endblock %}
{% block scripts %}
<script>
window.COVUI.initTreemap("posix-treemap", "/api/posix/treemap", "posix-treemap-controls");
window.COVUI.initSortableTable("posix-option-groups");
</script>
{% endblock %}
""",
    "posix_group.html": """
{% extends "base.html" %}
{% import "macros.html" as m %}
{% block body %}
<section class="hero mb-3">
  <div class="d-flex flex-wrap align-items-start justify-content-between gap-3">
    <div>
      <div class="smallcaps">POSIX Group</div>
      <h2 class="h4 mb-1">{{ group.gid }}</h2>
      <div class="muted">{{ group.label }} · {{ group.kind }}</div>
    </div>
    {{ m.total_line_coverage(group.stats, group.line_cov, group.line_total, "hero")|safe }}
  </div>
</section>
<div class="row g-3 mb-3">
  <div class="col-lg-5">
    <div class="card h-100">
      <div class="card-header">Coverage</div>
      <div class="card-body">{{ m.coverage_wheels(group.stats, color_for)|safe }}</div>
    </div>
  </div>
  <div class="col-lg-7">
    <div class="card h-100">
      <div class="card-header">Group Metadata</div>
      <div class="card-body">
        <span class="pill">{{ "ISO C" if group.iso_c else "Non-ISO C" }}</span>
        <span class="pill">{{ group.kind }}</span>
        <span class="pill">{{ group.symbols|length }} symbols</span>
      </div>
    </div>
  </div>
</div>
<div class="card">
  <div class="card-header">Symbols</div>
  <div class="card-body table-responsive">
    <table class="table table-sm table-hover mb-0">
      <thead><tr><th>Symbol</th><th>Status</th><th>Line · Branch</th></tr></thead>
      <tbody>
      {% for s in symbols %}
        <tr class="row-{{ s.class }}">
          <td>
            {% if s.source_href %}
              <a href="{{ s.source_href }}"><code>{{ s.name }}</code></a>
            {% else %}
              <code>{{ s.name }}</code>
            {% endif %}
            {% if s.implementation_note %}
              <div class="muted small mt-1">{{ s.implementation_note }}</div>
            {% endif %}
          </td>
          <td><span class="pill cov-{{ s.class }}">{{ s.status }}</span></td>
          <td>{{ m.coverage_wheels(s.coverage_stats, color_for)|safe }}</td>
        </tr>
      {% endfor %}
      </tbody>
    </table>
  </div>
</div>
{% endblock %}
""",
    "posix_symbol.html": """
{% extends "base.html" %}
{% block body %}
<section class="hero mb-3">
  <div class="smallcaps">POSIX Symbol</div>
  <h2 class="h4 mb-1"><code>{{ symbol.name }}</code></h2>
  <div class="muted">Status: <span class="pill cov-{{ symbol.class }}">{{ symbol.status }}</span></div>
  {% if symbol.implementation_note %}
    <p class="mb-0 mt-2 muted">{{ symbol.implementation_note }}</p>
  {% endif %}
</section>
{% if symbol.source_href %}
<div class="card mb-3">
  <div class="card-header">Source</div>
  <div class="card-body">
    <a class="btn btn-primary btn-sm" href="{{ symbol.source_href }}">Open definition</a>
  </div>
</div>
{% endif %}
<div class="card mb-3">
  <div class="card-header">Option Groups</div>
  <div class="card-body d-flex gap-2 flex-wrap">
    {% for gid in symbol.groups %}
      <a class="btn btn-outline-theme btn-sm" href="/posix/{{ gid|urlencode }}">{{ gid }}</a>
    {% endfor %}
  </div>
</div>
<div class="card">
  <div class="card-header">Candidate Implementations</div>
  <div class="card-body table-responsive">
    <table class="table table-sm table-hover mb-0">
      <thead><tr><th>File</th><th>Execution Count</th><th>Function Name</th></tr></thead>
      <tbody>
        {% for c in symbol.candidates %}
          <tr><td><code>{{ c.path }}</code></td><td>{{ c.count }}</td><td><code>{{ c.name }}</code></td></tr>
        {% endfor %}
      </tbody>
    </table>
  </div>
</div>
{% endblock %}
""",
}


def _is_coverable_symbol(symbol: str) -> bool:
    return symbol not in V4_VARIABLE_SYMBOLS


def _to_pct(hit: Optional[int], total: Optional[int]) -> Optional[float]:
    if hit is None or total is None:
        return None
    if total <= 0:
        return None
    return (100.0 * hit) / float(total)


def _effective_branch_pct(branch_hit: int, branch_total: int) -> float:
    if branch_total <= 0:
        return 100.0
    return float(_to_pct(branch_hit, branch_total) or 100.0)


@dataclass
class CoverageStats:
    line_pct: Optional[float]
    func_pct: Optional[float]
    branch_pct: Optional[float]
    external: bool = False

    def coverage_class(
        self,
        metric: str = "line",
        green: float = 70.0,
        yellow: float = 50.0,
    ) -> str:
        if self.external:
            return "external"
        key = metric.lower()
        value = None
        if key.startswith("line"):
            value = self.line_pct
        elif key.startswith("func"):
            value = self.func_pct
        elif key.startswith("branch"):
            value = self.branch_pct
        if value is None:
            return "unknown"
        if value >= green:
            return "green"
        if value >= yellow:
            return "yellow"
        return "red"


def _color_for_class(cov_class: str) -> str:
    return {
        "green": "var(--green)",
        "yellow": "var(--yellow)",
        "red": "var(--red)",
        "external": "var(--external)",
        "unknown": "var(--unknown)",
    }.get(cov_class, "var(--unknown)")


class CoverageContainer:
    def __init__(
        self,
        coverage_json_path: Path,
        workspace: Path,
        framework: str = "default",
        framework_config_path: Optional[Path] = None,
        green_threshold: float = 70.0,
        yellow_threshold: float = 50.0,
        no_warn_libc: bool = False,
        cache_dir: Optional[Path] = None,
    ) -> None:
        self.coverage_json_path = coverage_json_path.resolve()
        self.workspace = workspace.resolve()
        self.framework = framework
        self.green_threshold = green_threshold
        self.yellow_threshold = yellow_threshold
        self.no_warn_libc = no_warn_libc
        self.cache_dir = cache_dir.resolve() if cache_dir else None
        if self.cache_dir:
            self.cache_dir.mkdir(parents=True, exist_ok=True)

        raw = json.loads(self.coverage_json_path.read_text(encoding="utf-8"))
        self.gcovr_meta = {
            "format_version": raw.get("gcovr/format_version"),
            "root": raw.get("root"),
            "generator": raw.get("gcovr/producer"),
        }
        self.files: List[Dict[str, Any]] = raw.get("files", [])
        self.line_map: Dict[Tuple[str, int], Dict[str, Any]] = {}
        self.file_stats: Dict[str, CoverageStats] = {}
        self.file_source_paths: Dict[str, str] = {}
        self.file_totals: Dict[str, Dict[str, int]] = {}
        self.function_index: Dict[str, List[Dict[str, Any]]] = {}
        self.file_functions: Dict[str, List[Dict[str, Any]]] = {}
        self.directory_agg: Dict[str, Dict[str, int]] = {}

        self.option_group_symbols: Dict[str, Dict[str, Any]] = self._load_manifest(
            framework_config_path
        )
        self.symbol_option_groups: Dict[str, List[str]] = {}
        for gid, info in self.option_group_symbols.items():
            for symbol in info.get("symbols", []):
                self.symbol_option_groups.setdefault(symbol, []).append(gid)

        self.symbol_resolution_warnings: List[Dict[str, str]] = []
        self.symbol_state: Dict[str, Dict[str, Any]] = {}
        self.group_state: Dict[str, Dict[str, Any]] = {}
        self.macro_index: Dict[str, List[Dict[str, Any]]] = {}
        self.overall = CoverageStats(None, None, None)
        self.overall_totals: Dict[str, int] = {
            "line_cov": 0,
            "line_total": 0,
            "func_cov": 0,
            "func_total": 0,
            "branch_cov": 0,
            "branch_total": 0,
        }

        self._build_indices()
        self._build_implementation_index()
        self._build_macro_index()
        self._build_symbol_resolution()
        self._build_group_state()

    def coverage_json_workspace_path(self) -> Optional[str]:
        try:
            rel = self.coverage_json_path.relative_to(self.workspace).as_posix()
        except ValueError:
            return None
        if self.resolve_file_source(rel) is None:
            return None
        return rel

    def _load_manifest(
        self, framework_config_path: Optional[Path]
    ) -> Dict[str, Dict[str, Any]]:
        if framework_config_path:
            try:
                data = json.loads(framework_config_path.read_text(encoding="utf-8"))
                if isinstance(data, dict):
                    return self._normalize_manifest(data)
            except Exception as exc:
                raise SystemExit(
                    f"Failed to read framework config '{framework_config_path}': {exc}"
                )
        try:
            return self._normalize_manifest(json.loads(EMBED_POSIX_MANIFEST))
        except Exception as exc:
            raise SystemExit(f"Embedded manifest is invalid JSON: {exc}")

    @staticmethod
    def _normalize_manifest(
        data: Dict[str, Dict[str, Any]],
    ) -> Dict[str, Dict[str, Any]]:
        out: Dict[str, Dict[str, Any]] = {}
        for gid, info in data.items():
            entry = dict(info)
            symbols: List[str] = []
            for sym in info.get("symbols", []):
                sym = str(sym).strip()
                if sym.endswith("()"):
                    sym = sym[:-2]
                expanded = V4_SYMBOL_EXPANSIONS.get(sym)
                if expanded:
                    symbols.extend(expanded)
                elif sym:
                    symbols.append(sym)
            entry["symbols"] = symbols
            out[gid] = entry
        return out

    def _function_ranges(self, path: str) -> List[Tuple[str, int, int]]:
        funcs = sorted(self.file_functions.get(path, []), key=lambda f: f["lineno"])
        named = [
            fn
            for fn in funcs
            if fn["name"] and not str(fn["name"]).startswith("<unknown")
        ]
        ranges: List[Tuple[str, int, int]] = []
        for idx, fn in enumerate(named):
            start = int(fn["lineno"])
            end = int(named[idx + 1]["lineno"]) - 1 if idx + 1 < len(named) else 10**9
            ranges.append((str(fn["name"]), start, end))
        return ranges

    def _line_belongs_to_symbol(
        self, path: str, lnum: int, symbol: str, function_name: str
    ) -> bool:
        if function_name == symbol:
            return True
        if function_name.startswith("<unknown"):
            for name, start, end in self._function_ranges(path):
                if name == symbol and start <= lnum <= end:
                    return True
        return False

    def _symbol_line_branch_totals(
        self, symbol: str, prefer_path: Optional[str] = None
    ) -> Tuple[int, int, int, int]:
        line_hit = line_total = 0
        branch_hit = branch_total = 0
        seen: set[Tuple[str, int]] = set()
        for (path, lnum), line in self.line_map.items():
            if prefer_path and path != prefer_path:
                continue
            fn = str(line.get("function_name") or "")
            if not self._line_belongs_to_symbol(path, lnum, symbol, fn):
                continue
            key = (path, lnum)
            if key in seen:
                continue
            seen.add(key)
            if line.get("gcovr/excluded"):
                continue
            line_total += 1
            if int(line.get("count") or 0) > 0:
                line_hit += 1
            for branch in line.get("branches") or []:
                if branch.get("gcovr/excluded"):
                    continue
                branch_total += 1
                if int(branch.get("count") or 0) > 0:
                    branch_hit += 1
        return line_hit, line_total, branch_hit, branch_total

    def _group_line_branch_totals(
        self, symbols: Iterable[str]
    ) -> Tuple[int, int, int, int]:
        symbol_list = list(symbols)
        if not symbol_list:
            return 0, 0, 0, 0

        line_hit = line_total = branch_hit = branch_total = 0
        seen_lines: set[Tuple[str, int]] = set()
        for (path, lnum), line in self.line_map.items():
            fn = str(line.get("function_name") or "")
            if not any(
                self._line_belongs_to_symbol(path, lnum, symbol, fn)
                for symbol in symbol_list
            ):
                continue
            key = (path, lnum)
            if key in seen_lines:
                continue
            seen_lines.add(key)
            if line.get("gcovr/excluded"):
                continue
            line_total += 1
            if int(line.get("count") or 0) > 0:
                line_hit += 1
            for branch in line.get("branches") or []:
                if branch.get("gcovr/excluded"):
                    continue
                branch_total += 1
                if int(branch.get("count") or 0) > 0:
                    branch_hit += 1
        return line_hit, line_total, branch_hit, branch_total

    def _normalize_coverage_file(self, raw_path: str) -> str:
        p = Path(raw_path)
        try:
            if p.is_absolute():
                resolved = p.resolve()
                if resolved.is_relative_to(self.workspace):
                    return resolved.relative_to(self.workspace).as_posix()
                return resolved.as_posix()
            return p.as_posix()
        except Exception:
            return raw_path

    def _safe_workspace_path(self, rel_or_abs: str) -> Optional[Path]:
        p = Path(rel_or_abs)
        if p.is_absolute():
            try:
                resolved = p.resolve()
            except FileNotFoundError:
                resolved = p
            if resolved.is_relative_to(self.workspace):
                return resolved
            return None
        resolved = (self.workspace / p).resolve()
        if resolved.is_relative_to(self.workspace):
            return resolved
        return None

    @staticmethod
    def _extract_line_counts(file_obj: Dict[str, Any]) -> Tuple[int, int]:
        if "line_covered" in file_obj and "line_total" in file_obj:
            return int(file_obj.get("line_covered", 0)), int(
                file_obj.get("line_total", 0)
            )
        covered = 0
        total = 0
        for line in file_obj.get("lines", []) or []:
            if line.get("gcovr/excluded"):
                continue
            total += 1
            covered += 1 if int(line.get("count", 0)) > 0 else 0
        return covered, total

    @staticmethod
    def _extract_function_counts(file_obj: Dict[str, Any]) -> Tuple[int, int]:
        if "function_covered" in file_obj and "function_total" in file_obj:
            return int(file_obj.get("function_covered", 0)), int(
                file_obj.get("function_total", 0)
            )
        covered = 0
        total = 0
        for func in file_obj.get("functions", []) or []:
            total += 1
            covered += 1 if int(func.get("execution_count", 0)) > 0 else 0
        return covered, total

    @staticmethod
    def _extract_branch_counts(file_obj: Dict[str, Any]) -> Tuple[int, int]:
        if "branch_covered" in file_obj and "branch_total" in file_obj:
            return int(file_obj.get("branch_covered", 0)), int(
                file_obj.get("branch_total", 0)
            )
        covered = 0
        total = 0
        for line in file_obj.get("lines", []) or []:
            if line.get("gcovr/excluded"):
                continue
            for branch in line.get("branches", []) or []:
                if branch.get("gcovr/excluded"):
                    continue
                total += 1
                covered += 1 if int(branch.get("count", 0)) > 0 else 0
        return covered, total

    def _add_directory_totals(self, rel_path: str, totals: Dict[str, int]) -> None:
        parent = Path(rel_path).parent
        nodes = [Path(".")]
        if parent != Path("."):
            current = Path("")
            for part in parent.parts:
                current = current / part
                nodes.append(current)
        for node in nodes:
            key = node.as_posix() or "."
            entry = self.directory_agg.setdefault(
                key,
                {
                    "line_cov": 0,
                    "line_total": 0,
                    "func_cov": 0,
                    "func_total": 0,
                    "branch_cov": 0,
                    "branch_total": 0,
                },
            )
            for k, v in totals.items():
                entry[k] += int(v)

    def _build_indices(self) -> None:
        total_line_cov = total_line_total = 0
        total_func_cov = total_func_total = 0
        total_branch_cov = total_branch_total = 0
        for file_obj in self.files:
            raw_path = str(file_obj.get("file", ""))
            rel_path = self._normalize_coverage_file(raw_path)
            self.file_source_paths[rel_path] = raw_path

            l_cov, l_total = self._extract_line_counts(file_obj)
            f_cov, f_total = self._extract_function_counts(file_obj)
            b_cov, b_total = self._extract_branch_counts(file_obj)

            total_line_cov += l_cov
            total_line_total += l_total
            total_func_cov += f_cov
            total_func_total += f_total
            total_branch_cov += b_cov
            total_branch_total += b_total

            self.file_totals[rel_path] = {
                "line_cov": l_cov,
                "line_total": l_total,
                "func_cov": f_cov,
                "func_total": f_total,
                "branch_cov": b_cov,
                "branch_total": b_total,
            }
            self.file_stats[rel_path] = CoverageStats(
                _to_pct(l_cov, l_total),
                _to_pct(f_cov, f_total),
                _to_pct(b_cov, b_total),
            )
            self._add_directory_totals(rel_path, self.file_totals[rel_path])

            for line in file_obj.get("lines", []) or []:
                line_num = int(line.get("line_number", 0))
                if line_num <= 0:
                    continue
                self.line_map[(rel_path, line_num)] = line

            for func in file_obj.get("functions", []) or []:
                name = str(
                    func.get("demangled_name")
                    or func.get("name")
                    or func.get("function_name")
                    or ""
                )
                if not name:
                    continue
                entry = {
                    "path": rel_path,
                    "count": int(func.get("execution_count", 0)),
                    "name": name,
                    "lineno": int(
                        func.get("lineno") or func.get("line_number") or 0
                    ),
                }
                self.function_index.setdefault(name, []).append(entry)
                if (
                    name.startswith("<unknown")
                    and rel_path.endswith(".c")
                    and self._is_local_posix(rel_path)
                ):
                    stem = Path(rel_path).stem
                    if stem in self.symbol_option_groups and stem != name:
                        self.function_index.setdefault(stem, []).append(entry)
                self.file_functions.setdefault(rel_path, []).append(
                    {
                        "name": name,
                        "lineno": entry["lineno"],
                        "count": entry["count"],
                    }
                )

        self.overall = CoverageStats(
            _to_pct(total_line_cov, total_line_total),
            _to_pct(total_func_cov, total_func_total),
            _to_pct(total_branch_cov, total_branch_total),
        )
        self.overall_totals = {
            "line_cov": total_line_cov,
            "line_total": total_line_total,
            "func_cov": total_func_cov,
            "func_total": total_func_total,
            "branch_cov": total_branch_cov,
            "branch_total": total_branch_total,
        }

    def directory_stats(self, path: str) -> CoverageStats:
        key = path or "."
        totals = self.directory_agg.get(key) or {
            "line_cov": 0,
            "line_total": 0,
            "func_cov": 0,
            "func_total": 0,
            "branch_cov": 0,
            "branch_total": 0,
        }
        return CoverageStats(
            _to_pct(totals["line_cov"], totals["line_total"]),
            _to_pct(totals["func_cov"], totals["func_total"]),
            _to_pct(totals["branch_cov"], totals["branch_total"]),
        )

    def _tree_immediate_children(self, current: str) -> Tuple[List[str], List[str]]:
        """Return sorted immediate child directory and file paths under *current*."""
        current = current.strip("/") if current else ""
        prefix = f"{current}/" if current else ""
        dirs: Dict[str, CoverageStats] = {}
        files: Dict[str, CoverageStats] = {}
        for file_path, stats in self.file_stats.items():
            if not file_path.startswith(prefix):
                continue
            rest = file_path[len(prefix) :]
            if "/" in rest:
                first = rest.split("/", 1)[0]
                child = f"{prefix}{first}".strip("/")
                dirs[child] = self.directory_stats(child)
            else:
                files[file_path] = stats

        self._merge_posix_include_entries(current, dirs, files)
        return sorted(dirs.keys()), sorted(files.keys())

    def resolve_tree_path(self, current: str) -> str:
        """Skip single-child directory chains until a fork or files appear."""
        current = current.strip("/") if current else ""
        original = current
        path = current

        while True:
            dir_paths, file_paths = self._tree_immediate_children(path)
            if file_paths or len(dir_paths) != 1:
                break
            path = dir_paths[0]

        if path == original:
            return path

        dir_paths, file_paths = self._tree_immediate_children(path)
        if file_paths or len(dir_paths) > 1:
            return path
        return original

    def list_tree(self, current: str) -> Dict[str, Any]:
        current = current.strip("/") if current else ""
        dir_paths, file_paths = self._tree_immediate_children(current)

        def _sort_key(path: str) -> str:
            return format_posix_header_display(path).lower()

        dir_rows = [
            {
                "name": Path(path).name,
                "display_name": Path(path).name,
                "path": path,
                "stats": self.directory_stats(path),
            }
            for path in dir_paths
        ]
        file_rows = [
            {
                "name": Path(path).name,
                "display_name": format_posix_header_display(path),
                "path": path,
                "stats": self.file_stats[path],
            }
            for path in sorted(file_paths, key=_sort_key)
        ]
        breadcrumbs = [{"name": ".", "path": ""}]
        accum = ""
        if current:
            for chunk in current.split("/"):
                accum = f"{accum}/{chunk}" if accum else chunk
                breadcrumbs.append({"name": chunk, "path": accum})
        return {
            "directories": dir_rows,
            "files": file_rows,
            "breadcrumbs": breadcrumbs,
        }

    def _merge_posix_include_entries(
        self,
        current: str,
        dirs: Dict[str, CoverageStats],
        files: Dict[str, CoverageStats],
    ) -> None:
        if current != POSIX_INCLUDE_REL and not current.startswith(
            f"{POSIX_INCLUDE_REL}/"
        ):
            return
        rel_sub = (
            ""
            if current == POSIX_INCLUDE_REL
            else current[len(POSIX_INCLUDE_REL) + 1 :]
        )
        scan_dir = self.workspace / POSIX_INCLUDE_REL / rel_sub
        if not scan_dir.is_dir():
            return
        empty = CoverageStats(None, None, None)
        for entry in sorted(scan_dir.iterdir(), key=lambda p: p.name.lower()):
            try:
                rel_path = entry.relative_to(self.workspace).as_posix()
            except ValueError:
                continue
            if entry.is_file() and entry.suffix == ".h":
                files.setdefault(rel_path, empty)
            elif entry.is_dir():
                dirs.setdefault(rel_path, self.directory_stats(rel_path))

    def stats_for_path(self, path: str) -> Optional[CoverageStats]:
        if path in self.file_stats:
            return self.file_stats[path]
        if self.resolve_file_source(path) is not None:
            return CoverageStats(None, None, None)
        return None

    @staticmethod
    def is_posix_header(path: str) -> bool:
        norm = path.replace("\\", "/")
        return norm.startswith(f"{POSIX_INCLUDE_REL}/") and norm.endswith(".h")

    def _parse_header_prototypes(self, path: str) -> List[Dict[str, Any]]:
        source_path = self.resolve_file_source(path)
        if source_path is None:
            return []
        lines = source_path.read_text(encoding="utf-8", errors="replace").splitlines()
        protos: List[Dict[str, Any]] = []
        idx = 0
        while idx < len(lines):
            line = lines[idx]
            stripped = line.strip()
            if not stripped or stripped.startswith(("/*", "*", "#", "//")):
                idx += 1
                continue
            if stripped.startswith(("typedef", "struct ", "enum ", "union ")):
                idx += 1
                continue
            match = HEADER_PROTO_START_RE.match(line)
            if not match:
                idx += 1
                continue
            name = match.group(1)
            if name in HEADER_PROTO_SKIP_NAMES:
                idx += 1
                continue
            lineno = idx + 1
            block = line
            while not block.rstrip().endswith(";") and idx + 1 < len(lines):
                idx += 1
                block += " " + lines[idx].strip()
            if "(" in block:
                protos.append({"name": name, "lineno": lineno})
            idx += 1
        return protos

    def prototype_stats(self, path: str, lineno: int, name: str) -> CoverageStats:
        info = self.line_map.get((path, lineno))
        if info and not info.get("gcovr/excluded"):
            count = int(info.get("count") or 0)
            line_pct = 100.0 if count > 0 else 0.0
            branch_hit = branch_total = 0
            for branch in info.get("branches") or []:
                if branch.get("gcovr/excluded"):
                    continue
                branch_total += 1
                if int(branch.get("count") or 0) > 0:
                    branch_hit += 1
            branch_pct = _effective_branch_pct(branch_hit, branch_total)
            return CoverageStats(line_pct, None, branch_pct)

        state = self.symbol_state.get(name, {"status": "unresolved"})
        impl = self.symbol_function_coverage(name, state)
        if impl.line_pct is not None or impl.branch_pct is not None:
            return impl
        return CoverageStats(0.0, None, None)

    def _parse_c_source_functions(self, path: str) -> List[Dict[str, Any]]:
        source_path = self.resolve_file_source(path)
        if source_path is None:
            return []
        try:
            lines = source_path.read_text(
                encoding="utf-8", errors="replace"
            ).splitlines()
        except OSError:
            return []
        funcs: List[Dict[str, Any]] = []
        idx = 0
        while idx < len(lines):
            line = lines[idx]
            stripped = line.strip()
            if (
                not stripped
                or stripped.startswith(("/*", "*", "#", "//", "return "))
                or stripped.startswith(("typedef", "struct ", "enum ", "union "))
                or stripped.startswith(("if ", "for ", "while ", "switch ", "else "))
                or "Copyright" in line
                or "SPDX" in line
            ):
                idx += 1
                continue
            match = C_SOURCE_FUNC_RE.match(line)
            if not match:
                idx += 1
                continue
            if ";" in line:
                idx += 1
                continue
            name = match.group(1)
            if name in HEADER_PROTO_SKIP_NAMES or len(name) < 2:
                idx += 1
                continue
            lineno = idx + 1
            block = line
            while not block.rstrip().endswith("{") and idx + 1 < len(lines):
                idx += 1
                next_line = lines[idx]
                if ";" in next_line:
                    break
                block += " " + next_line.strip()
            if "(" not in block or not block.rstrip().endswith("{"):
                idx += 1
                continue
            funcs.append({"name": name, "lineno": lineno})
            idx += 1
        return funcs

    def _build_implementation_index(self) -> None:
        manifest_symbols = set(self.symbol_option_groups.keys())
        lib_root = self.workspace / "modules/lib/posix/lib"
        if not lib_root.is_dir():
            return
        for c_file in sorted(lib_root.rglob("*.c")):
            try:
                rel = c_file.relative_to(self.workspace).as_posix()
            except ValueError:
                continue
            for func in self._parse_c_source_functions(rel):
                name = func["name"]
                if name not in manifest_symbols:
                    continue
                lineno = int(func["lineno"])
                existing = self.function_index.get(name, [])
                if any(
                    e["path"] == rel and int(e.get("lineno") or 0) == lineno
                    for e in existing
                ):
                    continue
                self.function_index.setdefault(name, []).append(
                    {
                        "path": rel,
                        "count": 0,
                        "name": name,
                        "lineno": lineno,
                    }
                )

    @staticmethod
    def _pick_implementation_candidate(
        candidates: List[Dict[str, Any]],
    ) -> Optional[Dict[str, Any]]:
        impl = [
            c
            for c in candidates
            if not c["path"].endswith(".h")
            and CoverageContainer._is_local_posix(c["path"])
        ]
        libc = [
            c
            for c in candidates
            if not c["path"].endswith(".h")
            and CoverageContainer._is_zephyr_libc_common(c["path"])
        ]
        pool = impl or libc
        if not pool:
            return None

        def rank(entry: Dict[str, Any]) -> Tuple[int, int, int, str]:
            path = entry["path"].replace("\\", "/")
            sym_name = str(entry.get("name") or "")
            return (
                1 if "/lib/posix/" in path and path.endswith(".c") else 0,
                1 if not sym_name.startswith("<unknown") else 0,
                int(entry.get("count") or 0),
                path,
            )

        return max(pool, key=rank)

    def implementation_source_href(self, name: str) -> Optional[str]:
        state = self.symbol_state.get(name, {})
        if state.get("status") == "macro":
            return None
        pick = self._pick_implementation_candidate(self.function_index.get(name, []))
        if not pick:
            return None
        href = f"/source?path={quote(pick['path'], safe='')}"
        lineno = int(pick.get("lineno") or 0)
        if lineno > 0:
            href += f"#L{lineno}"
        return href

    def header_prototypes(self, path: str) -> List[Dict[str, Any]]:
        if not self.is_posix_header(path):
            return []

        rows: List[Dict[str, Any]] = []
        seen: set[Tuple[str, int]] = set()

        for func in self.file_functions.get(path, []):
            name = func["name"]
            lineno = int(func["lineno"] or 0)
            if not name or lineno <= 0:
                continue
            key = (name, lineno)
            if key in seen:
                continue
            seen.add(key)
            stats = self.prototype_stats(path, lineno, name)
            rows.append(
                {
                    "name": name,
                    "lineno": lineno,
                    "stats": stats,
                    "impl_href": self.implementation_source_href(name),
                }
            )

        for proto in self._parse_header_prototypes(path):
            key = (proto["name"], proto["lineno"])
            if key in seen:
                continue
            seen.add(key)
            name = proto["name"]
            stats = self.prototype_stats(path, proto["lineno"], name)
            rows.append(
                {
                    "name": name,
                    "lineno": proto["lineno"],
                    "stats": stats,
                    "impl_href": self.implementation_source_href(name),
                }
            )

        rows.sort(key=lambda row: row["lineno"])
        return rows

    @staticmethod
    def _is_local_posix(path: str) -> bool:
        norm = path.replace("\\", "/")
        return "modules/lib/posix/" in norm

    @staticmethod
    def _is_zephyr_libc_common(path: str) -> bool:
        norm = path.replace("\\", "/")
        return "zephyr/lib/libc/common" in norm

    def _build_macro_index(self) -> None:
        manifest_symbols = set(self.symbol_option_groups.keys())
        search_roots = [
            self.workspace / "modules/lib/posix/include",
            self.workspace / "modules/lib/posix/lib",
        ]
        for root in search_roots:
            if not root.is_dir():
                continue
            for header in root.rglob("*.h"):
                try:
                    rel = header.relative_to(self.workspace).as_posix()
                except ValueError:
                    continue
                try:
                    lines = header.read_text(
                        encoding="utf-8", errors="replace"
                    ).splitlines()
                except OSError:
                    continue
                for lineno, line in enumerate(lines, start=1):
                    match = MACRO_DEFINE_RE.match(line)
                    if not match:
                        continue
                    name = match.group(1)
                    if name not in manifest_symbols:
                        continue
                    tail = line[match.end() :].lstrip()
                    kind = "function" if tail.startswith("(") else "object"
                    self.macro_index.setdefault(name, []).append(
                        {"path": rel, "lineno": lineno, "kind": kind}
                    )

    @staticmethod
    def _symbol_implementation_note(state: Dict[str, Any]) -> str:
        macro_defs = state.get("macro_defs") or []
        if not macro_defs or state.get("status") != "macro":
            return ""
        kind = "Function-like" if macro_defs[0].get("kind") == "function" else "Object"
        return f"{kind} macro — gcov does not attribute coverage to macro expansions."

    def _build_symbol_resolution(self) -> None:
        for symbol, candidates in self.function_index.items():
            candidates.sort(key=lambda c: (c["count"], c["path"]), reverse=True)

        all_symbols: Iterable[str] = self.symbol_option_groups.keys()
        for symbol in all_symbols:
            if not _is_coverable_symbol(symbol):
                continue
            candidates = self.function_index.get(symbol, [])
            local = [c for c in candidates if self._is_local_posix(c["path"])]
            libc = [c for c in candidates if self._is_zephyr_libc_common(c["path"])]

            state: Dict[str, Any] = {
                "name": symbol,
                "status": "unresolved",
                "class": "unknown",
                "path": "-",
                "hit_count": 0,
                "candidates": candidates[:20],
                "groups": sorted(self.symbol_option_groups.get(symbol, [])),
                "macro_defs": self.macro_index.get(symbol, [])[:5],
            }
            if local:
                picked = local[0]
                state["status"] = "live"
                state["path"] = picked["path"]
                line_hit, _line_total, _branch_hit, _branch_total = (
                    self._symbol_line_branch_totals(symbol, picked["path"])
                )
                state["hit_count"] = max(int(picked["count"]), line_hit)
                state["class"] = "green" if state["hit_count"] > 0 else "red"
            elif libc:
                picked = libc[0]
                state["status"] = "libc"
                state["path"] = picked["path"]
                state["hit_count"] = picked["count"]
                state["class"] = "yellow" if picked["count"] > 0 else "unknown"
                if not self.no_warn_libc:
                    self.symbol_resolution_warnings.append(
                        {
                            "symbol": symbol,
                            "message": "resolved from zephyr/lib/libc/common (no local implementation found)",
                        }
                    )
            else:
                is_iso_c = symbol in ISO_C_EXTERNAL_SYMBOLS or any(
                    bool(self.option_group_symbols.get(gid, {}).get("iso_c"))
                    for gid in self.symbol_option_groups.get(symbol, [])
                )
                if is_iso_c:
                    state["status"] = "external"
                    state["class"] = "external"
                    state["path"] = "external (ISO C)"
                    state["hit_count"] = 1
                elif state["macro_defs"]:
                    picked = state["macro_defs"][0]
                    state["status"] = "macro"
                    state["path"] = picked["path"]
                    state["class"] = "yellow"
                else:
                    state["status"] = "unresolved"
            state["implementation_note"] = self._symbol_implementation_note(state)
            self.symbol_state[symbol] = state

    def symbol_function_coverage(
        self, symbol: str, state: Dict[str, Any]
    ) -> CoverageStats:
        if state.get("status") == "external":
            return CoverageStats(100.0, None, 100.0, external=True)

        prefer_path = None
        line_hit, line_total, branch_hit, branch_total = (
            self._symbol_line_branch_totals(symbol, prefer_path)
        )

        if line_total == 0 and state.get("status") == "unresolved":
            return CoverageStats(None, None, None)

        return CoverageStats(
            _to_pct(line_hit, line_total),
            None,
            _effective_branch_pct(branch_hit, branch_total),
        )

    def symbol_source_href(self, symbol: str, state: Dict[str, Any]) -> Optional[str]:
        if state.get("status") == "macro":
            macro_defs = state.get("macro_defs") or []
            if macro_defs:
                picked = macro_defs[0]
                path = picked.get("path")
                lineno = int(picked.get("lineno") or 0)
                if path:
                    href = f"/source?path={quote(path, safe='')}"
                    if lineno > 0:
                        href += f"#L{lineno}"
                    return href

        impl_href = self.implementation_source_href(symbol)
        if impl_href:
            return impl_href

        path = state.get("path") or ""
        if path in ("-", "external (ISO C)"):
            return None
        if path.endswith(".h"):
            if self.is_posix_header(path):
                href = f"/source?path={quote(path, safe='')}"
                candidates = self.function_index.get(symbol, [])
                header_hits = [c for c in candidates if c["path"] == path]
                if header_hits:
                    lineno = int(header_hits[0].get("lineno") or 0)
                    if lineno > 0:
                        href += f"#L{lineno}"
                return href
            return None
        return f"/source?path={quote(path, safe='')}"

    def _enrich_symbol_state(self, sym: str) -> Dict[str, Any]:
        base = self.symbol_state.get(sym)
        if not base:
            base = {
                "name": sym,
                "status": "unresolved",
                "class": "unknown",
                "path": "-",
                "hit_count": 0,
                "candidates": self.function_index.get(sym, [])[:20],
                "groups": sorted(self.symbol_option_groups.get(sym, [])),
                "macro_defs": self.macro_index.get(sym, [])[:5],
            }
        enriched = dict(base)
        enriched["implementation_note"] = self._symbol_implementation_note(enriched)
        enriched["coverage_stats"] = self.symbol_function_coverage(sym, enriched)
        enriched["source_href"] = self.symbol_source_href(sym, enriched)
        return enriched

    def _build_group_state(self) -> None:
        for gid, info in self.option_group_symbols.items():
            manifest_symbols = list(info.get("symbols", []))
            coverable_symbols = [
                sym for sym in manifest_symbols if _is_coverable_symbol(sym)
            ]
            symbol_states = [
                self._enrich_symbol_state(sym) for sym in coverable_symbols
            ]

            line_pcts: List[float] = []
            for s in symbol_states:
                stats = s.get("coverage_stats")
                if s.get("status") == "external":
                    line_pcts.append(100.0)
                    continue
                line_pcts.append(
                    float(stats.line_pct)
                    if stats and stats.line_pct is not None
                    else 0.0
                )

            total = len(coverable_symbols)
            if total == 0:
                func_pct = None
            else:
                func_pct = sum(line_pcts) / float(total)

            line_hit, line_total, branch_hit, branch_total = (
                self._group_line_branch_totals(coverable_symbols)
            )

            stats = CoverageStats(
                line_pct=_to_pct(line_hit, line_total),
                func_pct=func_pct,
                branch_pct=_effective_branch_pct(branch_hit, branch_total),
                external=bool(info.get("iso_c"))
                and all(
                    (s.get("status") in {"external", "unresolved"})
                    for s in symbol_states
                ),
            )
            self.group_state[gid] = {
                "gid": gid,
                "label": info.get("label", gid),
                "kind": info.get("kind", "unknown"),
                "iso_c": bool(info.get("iso_c")),
                "symbols": coverable_symbols,
                "symbol_states": symbol_states,
                "stats": stats,
                "line_cov": line_hit,
                "line_total": line_total,
            }

    def worst_files(self, limit: int = 16) -> List[Dict[str, Any]]:
        rows: List[Dict[str, Any]] = []
        for path, totals in self.file_totals.items():
            missed = max(0, totals["line_total"] - totals["line_cov"])
            if totals["line_total"] <= 0:
                continue
            rows.append(
                {
                    "path": path,
                    "stats": self.file_stats[path],
                    "missed": missed,
                    "reason": f"missed lines: {missed}",
                }
            )
        rows.sort(key=lambda r: (r["missed"], r["path"]), reverse=True)
        return rows[:limit]

    def resolve_file_source(self, path: str) -> Optional[Path]:
        source_raw = self.file_source_paths.get(path, path)
        candidate = self._safe_workspace_path(source_raw)
        if candidate and candidate.exists():
            return candidate
        if Path(source_raw).is_absolute():
            maybe = Path(source_raw)
            if maybe.exists():
                return maybe
        fallback = self._safe_workspace_path(path)
        if fallback and fallback.exists():
            return fallback
        return None

    def render_source_table(
        self, path: str, wheels_html: Optional[Dict[int, str]] = None
    ) -> str:
        source_path = self.resolve_file_source(path)
        if source_path is None:
            raise FileNotFoundError(path)
        wheels_html = wheels_html or {}
        cache_key_suffix = ":wheels" if wheels_html else ""
        if self.cache_dir:
            key = hashlib.sha256(
                f"{source_path}:{source_path.stat().st_mtime_ns}{cache_key_suffix}".encode()
            ).hexdigest()
            cache_file = self.cache_dir / f"{key}.html"
            if cache_file.exists():
                return cache_file.read_text(encoding="utf-8")

        content = source_path.read_text(encoding="utf-8", errors="replace")
        try:
            lexer = get_lexer_for_filename(source_path.name, content)
        except Exception:
            try:
                lexer = guess_lexer(content)
            except Exception:
                lexer = TextLexer()

        formatter = HtmlFormatter(nowrap=True)
        highlighted = highlight(content, lexer, formatter)
        rows = highlighted.splitlines()
        source_lines = content.splitlines()
        if len(rows) < len(source_lines):
            rows.extend([escape(line) for line in source_lines[len(rows) :]])

        show_wheels = bool(wheels_html)
        out: List[str] = [
            f'<table class="source-table{" source-table-proto" if show_wheels else ""}">'
        ]
        for idx, row in enumerate(rows, start=1):
            info = self.line_map.get((path, idx))
            cls = "cov-neutral"
            if info and not info.get("gcovr/excluded"):
                count = int(info.get("count", 0))
                cls = "cov-hit" if count > 0 else "cov-miss"
            cells = [
                f'<tr class="{cls}"><td class="ln"><a id="L{idx}" href="#L{idx}">{idx:>6}</a></td>',
                f'<td class="code">{row or "&nbsp;"}</td>',
            ]
            if show_wheels:
                cells.append(
                    f'<td class="proto-wheels">{wheels_html.get(idx, "")}</td>'
                )
            cells.append("</tr>")
            out.append("".join(cells))
        out.append("</table>")
        html = "\n".join(out)
        if self.cache_dir:
            cache_file.write_text(html, encoding="utf-8")
        return html

    def source_api(self, path: str) -> Dict[str, Any]:
        src = self.resolve_file_source(path)
        if src is None:
            raise FileNotFoundError(path)
        lines = src.read_text(encoding="utf-8", errors="replace").splitlines()
        payload = []
        for idx, text in enumerate(lines, start=1):
            info = self.line_map.get((path, idx), {})
            payload.append(
                {
                    "line": idx,
                    "text": text,
                    "count": int(info.get("count", 0)) if info else None,
                    "excluded": bool(info.get("gcovr/excluded", False)),
                    "branches": info.get("branches", []) if info else [],
                }
            )
        return {"path": path, "line_count": len(lines), "lines": payload}

    def build_treemap_payload(self) -> Dict[str, Any]:
        labels: List[str] = []
        parents: List[str] = []
        percent_values: List[float] = []
        line_values: List[float] = []
        colors: List[str] = []
        customdata: List[List[Any]] = []
        text: List[str] = []
        for gid, group in sorted(self.group_state.items()):
            stats: CoverageStats = group["stats"]
            if stats.external:
                continue
            pct = stats.line_pct if stats.line_pct is not None else 0.0
            if pct <= TREEMAP_MIN_SCORE:
                continue
            cov_class = stats.coverage_class(
                metric="line", green=self.green_threshold, yellow=self.yellow_threshold
            )
            line_total = int(group.get("line_total") or 0)
            labels.append(gid)
            parents.append("")
            percent_values.append(max(pct, 1.0))
            line_values.append(max(float(line_total), 1.0))
            colors.append(
                {
                    "green": "#39d98a",
                    "yellow": "#ffc857",
                    "red": "#ff5d73",
                    "external": "#8793b4",
                    "unknown": "#5e6989",
                }[cov_class]
            )
            customdata.append([gid, pct, line_total])
            text.append(group.get("label", gid))

        fig = go.Figure(
            go.Treemap(
                labels=labels,
                parents=parents,
                values=percent_values,
                marker={"colors": colors, "line": {"color": "#0f1117", "width": 1}},
                customdata=customdata,
                text=text,
                hovertemplate=(
                    "<b>%{label}</b><br>%{text}<br>"
                    "Coverage: %{customdata[1]:.1f}%<br>"
                    "Lines: %{customdata[2]}<extra></extra>"
                ),
            )
        )
        fig.update_layout(
            paper_bgcolor="#131926",
            plot_bgcolor="#131926",
            margin={"l": 10, "r": 10, "t": 16, "b": 10},
            font={"color": "#dbe5ff"},
        )
        payload = fig.to_plotly_json()
        payload["meta"] = {
            "default_size_mode": "percent",
            "size_modes": {
                "percent": percent_values,
                "lines": line_values,
            },
        }
        return payload


class CoverageRequestHandler(BaseHTTPRequestHandler):
    server_version = "coverageui/1.0"

    @property
    def app(self) -> "CoverageApp":
        return self.server.app  # type: ignore[attr-defined]

    def _send_bytes(
        self,
        data: bytes,
        content_type: str = "text/html; charset=utf-8",
        status: int = 200,
    ) -> None:
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(data)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(data)

    def _send_json(self, payload: Any, status: int = 200) -> None:
        self._send_bytes(
            json.dumps(payload, ensure_ascii=False).encode("utf-8"),
            "application/json; charset=utf-8",
            status=status,
        )

    def _redirect(self, location: str) -> None:
        self.send_response(302)
        self.send_header("Location", location)
        self.end_headers()

    def _query(self) -> Dict[str, str]:
        parsed = urlparse(self.path)
        return {k: v[0] for k, v in parse_qs(parsed.query).items() if v}

    def _render(self, template: str, **ctx: Any) -> None:
        html = self.app.render_template(template, **ctx)
        self._send_bytes(html.encode("utf-8"))

    def _not_found(self, detail: str = "not found") -> None:
        self._send_json({"error": detail}, status=404)

    def _bad_request(self, detail: str) -> None:
        self._send_json({"error": detail}, status=400)

    def log_message(self, fmt: str, *args: Any) -> None:
        sys.stderr.write("coverageui: " + (fmt % args) + "\n")

    def do_GET(self) -> None:  # noqa: N802
        parsed = urlparse(self.path)
        route = parsed.path
        q = self._query()
        try:
            if route == "/":
                if self.app.framework == "posix":
                    self._render(
                        "posix.html",
                        title="POSIX",
                        nav="posix",
                        groups=sorted(
                            self.app.coverage.group_state.values(),
                            key=lambda g: g["gid"],
                        ),
                        warnings=self.app.coverage.symbol_resolution_warnings,
                        treemap_min_score=int(TREEMAP_MIN_SCORE),
                    )
                    return
                self._render(
                    "index.html",
                    title="Overview",
                    nav="home",
                    worst_files=self.app.coverage.worst_files(),
                )
                return

            if route == "/tree":
                current = (q.get("path") or "").strip("/")
                resolved = self.app.coverage.resolve_tree_path(current)
                if resolved != current:
                    loc = "/tree"
                    if resolved:
                        loc = f"/tree?path={quote(resolved, safe='')}"
                    self._redirect(loc)
                    return
                data = self.app.coverage.list_tree(current)
                self._render(
                    "tree.html",
                    title="Tree",
                    nav="tree",
                    current_path=current,
                    directories=data["directories"],
                    files=data["files"],
                    breadcrumbs=data["breadcrumbs"],
                )
                return

            if route == "/file":
                path = q.get("path")
                if not path:
                    self._bad_request("missing query parameter: path")
                    return
                path = unquote(path)
                stats = self.app.coverage.stats_for_path(path)
                if stats is None:
                    self._not_found(f"unknown file: {path}")
                    return
                source_path = self.app.coverage.resolve_file_source(path)
                parent = str(Path(path).parent)
                if parent == ".":
                    parent = ""
                prototypes = (
                    self.app.coverage.header_prototypes(path)
                    if self.app.coverage.is_posix_header(path)
                    else []
                )
                self._render(
                    "file.html",
                    title="File",
                    nav="tree",
                    path=path,
                    display_path=format_posix_header_display(path),
                    stats=stats,
                    parent_path=parent,
                    source_path=source_path.as_posix() if source_path else "(missing)",
                    prototypes=prototypes,
                )
                return

            if route == "/source":
                path = q.get("path")
                if not path:
                    self._bad_request("missing query parameter: path")
                    return
                path = unquote(path)
                stats = self.app.coverage.stats_for_path(path)
                if stats is None:
                    self._not_found(f"unknown file: {path}")
                    return
                file_totals = self.app.coverage.file_totals.get(path, {})
                wheels_html = self.app.prototype_wheels_html(path)
                html = self.app.coverage.render_source_table(
                    path, wheels_html=wheels_html
                )
                self._render(
                    "source.html",
                    title="Source",
                    nav="tree",
                    path=path,
                    display_path=format_posix_header_display(path),
                    stats=stats,
                    line_cov=int(file_totals.get("line_cov", 0)),
                    line_total=int(file_totals.get("line_total", 0)),
                    source_table=html,
                    show_prototype_wheels=bool(wheels_html),
                )
                return

            if route == "/posix":
                self._render(
                    "posix.html",
                    title="POSIX",
                    nav="posix",
                    groups=sorted(
                        self.app.coverage.group_state.values(), key=lambda g: g["gid"]
                    ),
                    warnings=self.app.coverage.symbol_resolution_warnings,
                    treemap_min_score=int(TREEMAP_MIN_SCORE),
                )
                return

            if route.startswith("/posix/symbol/"):
                symbol = unquote(route[len("/posix/symbol/") :])
                state = self.app.coverage.symbol_state.get(symbol)
                if not state:
                    self._not_found(f"unknown symbol: {symbol}")
                    return
                self._render(
                    "posix_symbol.html",
                    title=f"Symbol {symbol}",
                    nav="posix",
                    symbol=state,
                )
                return

            if route.startswith("/posix/"):
                gid = unquote(route[len("/posix/") :])
                group = self.app.coverage.group_state.get(gid)
                if not group:
                    self._not_found(f"unknown group: {gid}")
                    return
                self._render(
                    "posix_group.html",
                    title=f"Group {gid}",
                    nav="posix",
                    group=group,
                    symbols=group["symbol_states"],
                )
                return

            if route == "/api/meta":
                self._send_json(
                    {
                        "coverage_file": self.app.coverage.coverage_json_path.as_posix(),
                        "workspace": self.app.coverage.workspace.as_posix(),
                        "framework": self.app.framework,
                        "thresholds": {
                            "green": self.app.coverage.green_threshold,
                            "yellow": self.app.coverage.yellow_threshold,
                        },
                        "gcovr": self.app.coverage.gcovr_meta,
                        "files": len(self.app.coverage.file_stats),
                        "groups": len(self.app.coverage.group_state),
                    }
                )
                return

            if route == "/api/posix/treemap":
                self._send_json(self.app.coverage.build_treemap_payload())
                return

            if route == "/api/source":
                path = q.get("path")
                if not path:
                    self._bad_request("missing query parameter: path")
                    return
                path = unquote(path)
                if self.app.coverage.stats_for_path(path) is None:
                    self._not_found(f"unknown file: {path}")
                    return
                payload = self.app.coverage.source_api(path)
                self._send_json(payload)
                return

            # Serve files if user navigates to static source-like path by mistake.
            candidate = self.app.coverage._safe_workspace_path(route.lstrip("/"))
            if candidate and candidate.exists() and candidate.is_file():
                ctype = mimetypes.guess_type(candidate.name)[0] or "text/plain"
                self._send_bytes(candidate.read_bytes(), f"{ctype}; charset=utf-8")
                return

            self._not_found()
        except FileNotFoundError as exc:
            self._not_found(str(exc))
        except PermissionError as exc:
            self._send_json({"error": str(exc)}, status=403)
        except Exception as exc:  # pragma: no cover - defensive
            traceback.print_exc()
            self._send_json({"error": f"internal error: {exc}"}, status=500)


class CoverageApp:
    def __init__(self, args: argparse.Namespace) -> None:
        self.framework = args.framework
        framework_cfg = (
            Path(args.framework_config).resolve() if args.framework_config else None
        )
        cache_dir = Path(args.cache_dir).resolve() if args.cache_dir else None
        self.coverage = CoverageContainer(
            coverage_json_path=Path(args.coverage_json).resolve(),
            workspace=_resolve_workspace(
                Path(args.coverage_json).resolve(),
                args.workspace,
            ),
            framework=args.framework,
            framework_config_path=framework_cfg,
            green_threshold=float(args.green_threshold),
            yellow_threshold=float(args.yellow_threshold),
            no_warn_libc=bool(args.no_warn_libc),
            cache_dir=cache_dir,
        )
        self.env = Environment(
            loader=DictLoader(TEMPLATES),
            autoescape=select_autoescape(default_for_string=True, default=True),
            trim_blocks=True,
            lstrip_blocks=True,
        )
        self._render_lock = threading.Lock()
        self._inject_globals()

    def _inject_globals(self) -> None:
        totals = self.coverage.overall_totals
        self.env.globals.update(
            cdn=CDN_URLS,
            embed_css=EMBED_CSS,
            embed_js=EMBED_JS,
            pygments_css=build_pygments_css(),
            framework=self.framework,
            coverage_file=self.coverage.coverage_json_path.name,
            coverage_file_href=self.coverage.coverage_json_workspace_path(),
            green_threshold=self.coverage.green_threshold,
            yellow_threshold=self.coverage.yellow_threshold,
            color_for=_color_for_class,
            overall=self.coverage.overall,
            overall_line_cov=totals["line_cov"],
            overall_line_total=totals["line_total"],
        )
        self.env.filters["urlencode"] = lambda s: quote(str(s), safe="")
        self.env.filters["header_display"] = format_posix_header_display

    def render_template(self, template_name: str, **context: Any) -> str:
        with self._render_lock:
            tmpl = self.env.get_template(template_name)
            return tmpl.render(**context)

    def render_inline_wheels(self, stats: CoverageStats) -> str:
        return self.render_template("_inline_wheels.html", stats=stats)

    def prototype_wheels_html(self, path: str) -> Dict[int, str]:
        if not self.coverage.is_posix_header(path):
            return {}
        wheels: Dict[int, str] = {}
        for proto in self.coverage.header_prototypes(path):
            wheels[proto["lineno"]] = self.render_inline_wheels(proto["stats"])
        return wheels


def _workspace_covers_coverage(ws: Path, coverage_json: Path) -> bool:
    try:
        raw = json.loads(coverage_json.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return False
    files = raw.get("files") or []
    if not files:
        return (ws / "modules/lib/posix").is_dir()
    for file_obj in files[:20]:
        rel = str(file_obj.get("file", "")).strip()
        if rel and (ws / rel).exists():
            return True
    return False


def _find_west_workspace(start: Path) -> Optional[Path]:
    cur = start.resolve()
    for _ in range(24):
        if (cur / ".west/config").is_file():
            return cur
        parent = cur.parent
        if parent == cur:
            break
        cur = parent
    return None


def _resolve_workspace(coverage_json: Path, explicit: Optional[str]) -> Path:
    if explicit is not None:
        return Path(explicit).expanduser().resolve()

    candidates: List[Path] = []
    for start in (coverage_json.parent, Path.cwd()):
        found = _find_west_workspace(start)
        if found is not None and found not in candidates:
            candidates.append(found)

    for candidate in candidates:
        if _workspace_covers_coverage(candidate, coverage_json):
            return candidate

    if candidates:
        return candidates[0]

    return Path.cwd().resolve()


DEFAULT_PORT = 8000
TREEMAP_MIN_SCORE = 10.0


def _port_available(bind: str, port: int) -> bool:
    """Return True if *port* can be bound on *bind*."""
    if port == 0:
        return True
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:
            sock.bind((bind, port))
        except OSError as exc:
            if exc.errno in (errno.EADDRINUSE, errno.EACCES):
                return False
            raise
    return True


def _resolve_listen_port(bind: str, port: Optional[int], explicit: bool) -> int:
    """Choose a listen port.

    When *explicit* is false (``-p`` omitted), try ``DEFAULT_PORT`` then ephemeral (0).
    When *explicit* is true, honor *port* and fail if it is unavailable.
    """
    if explicit:
        if port == 0:
            return 0
        if not _port_available(bind, port):
            raise SystemExit(f"coverageui: port {port} is already in use on {bind}")
        return port

    if _port_available(bind, DEFAULT_PORT):
        return DEFAULT_PORT

    sys.stderr.write(
        f"coverageui: port {DEFAULT_PORT} is in use on {bind}, using an ephemeral port\n"
    )
    return 0


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Interactive gcovr coverage viewer",
        allow_abbrev=False,
    )
    parser.add_argument("coverage_json", help="Path to gcovr JSON coverage file")
    parser.add_argument(
        "-d",
        "--workspace",
        default=None,
        help="Workspace root used for secure source resolution (default: auto-detect west workspace)",
    )
    parser.add_argument(
        "-b",
        "--bind",
        default="localhost",
        help="Bind host (default: localhost)",
    )
    parser.add_argument(
        "-p",
        "--port",
        type=int,
        default=None,
        help=f"Bind port (default: {DEFAULT_PORT}, or ephemeral if {DEFAULT_PORT} is taken)",
    )
    parser.add_argument(
        "--framework",
        choices=["default", "posix"],
        default="default",
        help="UI framework landing mode",
    )
    parser.add_argument(
        "--framework-config",
        default=None,
        help="Optional JSON manifest path for framework-specific symbol groups",
    )
    parser.add_argument(
        "--cache-dir",
        default=None,
        help="Optional cache directory for rendered source HTML",
    )
    parser.add_argument(
        "--green-threshold",
        type=float,
        default=70.0,
        help="Coverage threshold for green class",
    )
    parser.add_argument(
        "--yellow-threshold",
        type=float,
        default=50.0,
        help="Coverage threshold for yellow class",
    )
    parser.add_argument(
        "--no-warn-libc",
        action="store_true",
        help="Do not emit warnings when symbol resolves to zephyr libc/common",
    )
    return parser


def main(argv: Optional[List[str]] = None) -> int:
    parser = _build_parser()
    args = parser.parse_args(argv)

    coverage_json = Path(args.coverage_json).resolve()
    if not coverage_json.exists():
        parser.error(f"coverage file does not exist: {coverage_json}")
    if not coverage_json.is_file():
        parser.error(f"coverage path is not a file: {coverage_json}")

    workspace = _resolve_workspace(coverage_json, args.workspace)
    if args.workspace is None:
        sys.stderr.write(f"coverageui: auto-detected workspace {workspace}\n")
    args.workspace = str(workspace)

    port_explicit = args.port is not None
    listen_port = _resolve_listen_port(
        args.bind, args.port if port_explicit else None, port_explicit
    )

    app = CoverageApp(args)
    server = ThreadingHTTPServer((args.bind, listen_port), CoverageRequestHandler)
    server.app = app  # type: ignore[attr-defined]
    host, port = server.server_address[0], server.server_address[1]
    sys.stderr.write(f"coverageui: serving at http://{host}:{port}\n")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
