# POSIX Option Groups

Each directory here implements one POSIX Subprofiling Option Group (or a select group-like
option, such as the XSI-related options), as defined in the
[POSIX Subprofiling Considerations](https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html)
of IEEE Std 1003.1-2017 (POSIX Issue 7).

A function's implementation lives in the directory of the Option Group that owns it — for
example, `nanosleep()` belongs to `timers`, while `clock_nanosleep()` belongs to
`clock_selection`.

The `shared` directory contains internal helpers used by multiple Option Groups; it is not
itself an Option Group.

Tests are organized the same way, under [tests/posix](../../../tests/posix).
