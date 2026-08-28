# POSIX Testsuites

Each directory here is a testsuite for one POSIX Subprofiling Option Group (or a select
group-like option), as defined in the
[POSIX Subprofiling Considerations](https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_subprofiles.html)
of IEEE Std 1003.1-2017 (POSIX Issue 7). A function's tests live in the suite of the Option
Group that owns the function — e.g. `nanosleep()` is tested in `timers`, while
`clock_nanosleep()` is tested in `clock_selection`. Implementations follow the same layout
under [lib/posix](../../lib/posix).

The `shared` directory holds common test helpers; it is not itself a testsuite.

## Suite and test naming

Testsuites are usually named after the Option Group under test, as in
`ZTEST_SUITE(<option group>, ...)` (e.g. `posix_timers`, `xsi_streams`). Tests are centered
around a unit — i.e. a function — typically one test per unit:

```c
ZTEST(<option group>, test_<unit>)
/* or, when the test runs in user mode as well */
ZTEST_USER(<option group>, test_<unit>)
```

For example, `ZTEST(posix_timers, test_timer_create)`.

## Test configurations

Each testsuite is evaluated in several test configurations (see each suite's
`testcase.yaml`):

- **default** — the baseline scenario, e.g. `portability.posix.timers`
- **nominal builds per officially supported libc** — e.g. `.minimal`, `.newlib`, `.picolibc`
- **userspace** — with `CONFIG_USERSPACE=y`, exercising tests as unprivileged threads
- **linux_compat** — on `native_sim` with the host C library (`CONFIG_NATIVE_LIBC`),
  verifying Linux-compatible behavior
