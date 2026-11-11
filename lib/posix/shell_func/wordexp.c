/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "shell_internal.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wordexp.h>

#include <zephyr/sys/zvfs.h>
#include <zephyr/zvfs/pipe.h>

#define WRDE_FLAGS (WRDE_APPEND | WRDE_DOOFFS | WRDE_NOCMD | WRDE_REUSE | WRDE_SHOWERR | WRDE_UNDEF)

/*
 * The shell does the expanding: printf writes each expanded word
 * NUL-terminated through a pipe, behind a sentinel that keeps a lone
 * "%s" from standing in for words that expanded to nothing.
 */
static const char wordexp_prologue[] = "printf '%s\\000' _ ";

/*
 * A look over the words before the shell sees them: an unbalanced quote is a
 * syntax error the shell need not be asked about, and command substitution
 * is what WRDE_NOCMD forbids.
 */
static int wordexp_scan(const char *words, int flags)
{
	char quote = '\0';
	bool cmdsub = false;

	for (const char *p = words; *p != '\0'; p++) {
		if (*p == '\\' && quote != '\'' && p[1] != '\0') {
			p++;
		} else if (quote != '\0') {
			if (*p == quote) {
				quote = '\0';
			} else if (quote == '"' && (*p == '`' || (*p == '$' && p[1] == '('))) {
				cmdsub = true;
			}
		} else if (*p == '\'' || *p == '"') {
			quote = *p;
		} else if (*p == '`' || (*p == '$' && p[1] == '(')) {
			cmdsub = true;
		}
	}

	if (quote != '\0') {
		return WRDE_SYNTAX;
	}
	if (cmdsub && (flags & WRDE_NOCMD) != 0) {
		return WRDE_CMDSUB;
	}

	return 0;
}

/* collect everything the shell writes, growing the buffer as it comes */
static int wordexp_collect(int fd, char **out, size_t *len)
{
	size_t cap = 256;
	char *buf = malloc(cap);

	*len = 0;
	if (buf == NULL) {
		return WRDE_NOSPACE;
	}

	while (true) {
		ssize_t n;

		if (*len == cap) {
			char *more = realloc(buf, cap * 2);

			if (more == NULL) {
				free(buf);
				return WRDE_NOSPACE;
			}
			buf = more;
			cap *= 2;
		}
		n = zvfs_read(fd, buf + *len, cap - *len);
		if (n < 0) {
			free(buf);
			return WRDE_SYNTAX;
		}
		if (n == 0) {
			break;
		}
		*len += (size_t)n;
	}

	*out = buf;

	return 0;
}

static int wordexp_store(const char *buf, size_t len, wordexp_t *we, int flags)
{
	size_t offs = ((flags & WRDE_DOOFFS) != 0) ? we->we_offs : 0;
	size_t have = ((flags & WRDE_APPEND) != 0) ? we->we_wordc : 0;
	size_t count;
	size_t total;
	char **vec;
	const char *p;
	const char *end = buf + len;

	/* the sentinel comes first; the words are what follows it */
	if (len < 2 || buf[0] != '_' || buf[1] != '\0') {
		return WRDE_SYNTAX;
	}
	p = buf + 2;
	count = 0;
	for (const char *q = p; q < end; q++) {
		if (*q == '\0') {
			count++;
		}
	}

	total = offs + have + count;
	vec = realloc(((flags & WRDE_APPEND) != 0) ? we->we_wordv : NULL,
		      (total + 1) * sizeof(char *));
	if (vec == NULL) {
		return WRDE_NOSPACE;
	}
	if ((flags & WRDE_APPEND) == 0) {
		for (size_t i = 0; i < offs; i++) {
			vec[i] = NULL;
		}
	}
	we->we_wordv = vec;
	we->we_wordc = have;
	vec[offs + have] = NULL;

	for (size_t i = 0; i < count; i++) {
		const char *nul = memchr(p, '\0', end - p);
		char *word;

		if (nul == NULL) {
			return WRDE_SYNTAX;
		}
		word = malloc(nul - p + 1);
		if (word == NULL) {
			return WRDE_NOSPACE;
		}
		memcpy(word, p, nul - p + 1);
		vec[offs + have + i] = word;
		vec[offs + have + i + 1] = NULL;
		we->we_wordc++;
		p = nul + 1;
	}

	return 0;
}

int wordexp(const char *ZRESTRICT words, wordexp_t *ZRESTRICT pwordexp, int flags)
{
	int ret;
	int status;
	int fds[2];
	size_t len;
	char *out;
	char *command;
	k_pid_t child;
	struct sys_clone_fd_action acts[3];
	const char *undef = ((flags & WRDE_UNDEF) != 0) ? "set -u; " : "";

	if ((flags & ~WRDE_FLAGS) != 0) {
		return WRDE_BADVAL;
	}
	ret = wordexp_scan(words, flags);
	if (ret != 0) {
		return ret;
	}
	if ((flags & WRDE_REUSE) != 0 && (flags & WRDE_APPEND) == 0) {
		wordfree(pwordexp);
	}
	if ((flags & WRDE_APPEND) == 0) {
		pwordexp->we_wordc = 0;
		pwordexp->we_wordv = NULL;
		if ((flags & WRDE_DOOFFS) == 0) {
			pwordexp->we_offs = 0;
		}
	}

	command = malloc(strlen(undef) + sizeof(wordexp_prologue) + strlen(words));
	if (command == NULL) {
		return WRDE_NOSPACE;
	}
	strcpy(command, undef);
	strcat(command, wordexp_prologue);
	strcat(command, words);

	if (zvfs_pipe(fds, 0) < 0) {
		free(command);
		return WRDE_NOSPACE;
	}
	acts[0] = (struct sys_clone_fd_action){.op = SYS_CLONE_FD_DUP2, .fd = fds[1],
					       .newfd = STDOUT_FILENO};
	acts[1] = (struct sys_clone_fd_action){.op = SYS_CLONE_FD_CLOSE, .fd = fds[0]};
	acts[2] = (struct sys_clone_fd_action){.op = SYS_CLONE_FD_CLOSE, .fd = fds[1]};

	ret = posix_shell_spawn(command, acts, ARRAY_SIZE(acts), &child);
	free(command);
	(void)zvfs_close(fds[1]);
	if (ret < 0) {
		(void)zvfs_close(fds[0]);
		return WRDE_NOSPACE;
	}

	ret = wordexp_collect(fds[0], &out, &len);
	(void)zvfs_close(fds[0]);
	status = posix_shell_wait(child);
	if (ret != 0) {
		return ret;
	}
	if (status != 0) {
		/* the shell refused the words: a syntax error, or an unset variable */
		free(out);
		return WRDE_SYNTAX;
	}

	ret = wordexp_store(out, len, pwordexp, flags);
	free(out);

	return ret;
}

void wordfree(wordexp_t *pwordexp)
{
	if (pwordexp->we_wordv == NULL) {
		return;
	}
	for (size_t i = 0; i < pwordexp->we_wordc; i++) {
		free(pwordexp->we_wordv[pwordexp->we_offs + i]);
	}
	free(pwordexp->we_wordv);
	pwordexp->we_wordv = NULL;
	pwordexp->we_wordc = 0;
}
