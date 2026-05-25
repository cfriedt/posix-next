<p align="center">
  <img src="doc/_static/posix-next-logo.png" alt="posix-next" width="256">
</p>

<h1 align="center">posix-next</h1>

<p align="center">
  <em>The next-generation of Zephyr's POSIX reference design.</em>
</p>

<p align="center">
  <a href="https://github.com/cfriedt/posix-next/actions/workflows/twister.yml">
    <img src="https://github.com/cfriedt/posix-next/actions/workflows/twister.yml/badge.svg" alt="Twister">
  </a>
  <a href="https://github.com/cfriedt/posix-next/actions/workflows/docs.yml">
    <img src="https://github.com/cfriedt/posix-next/actions/workflows/docs.yml/badge.svg" alt="Documentation">
  </a>
  <a href="https://github.com/cfriedt/posix-next/blob/main/LICENSE">
    <img src="https://img.shields.io/badge/license-Apache--2.0-blue.svg" alt="License">
  </a>
</p>

---

## What is posix-next?

**posix-next** is an out-of-tree [Zephyr RTOS](https://zephyrproject.org/)
module that serves as a staging tree for upstream POSIX API features. It tracks ahead of what is upstream and aims to improve the cadence of feature development and testing. In terms of confromance, **posix-next** targets the [POSIX.1-2018](https://pubs.opengroup.org/onlinepubs/9699919799/) release (Issue 7), although there is some support for [POSIX.1-2024](https://pubs.opengroup.org/onlinepubs/9799919799/) (Issue 8).

Key goals:

- **Portability** — run POSIX applications on Zephyr with minimal changes.
- **Conformance** — track the full POSIX option groups and work toward PSE51/52/53 profiles.
- **Linux parity** — validate behavior against Linux using `native_sim` and real hardware.

## Documentation

| Resource | Link |
|----------|------|
| Guides & overview | [cfriedt.github.io/posix-next](https://cfriedt.github.io/posix-next/) |
| API reference (Doxygen) | [Doxygen HTML](https://cfriedt.github.io/posix-next/doxygen/html/index.html) |
| POSIX symbol search | [Symbol search](https://cfriedt.github.io/posix-next/posix/symbols.html) |
| Option groups | [Option groups](https://cfriedt.github.io/posix-next/posix/option_groups/index.html) |
| Conformance status | [Conformance](https://cfriedt.github.io/posix-next/posix/conformance/index.html) |

## Quick start

### Prerequisites

- [Zephyr dependencies](https://docs.zephyrproject.org/latest/develop/getting_started/index.html#install-dependencies)
  installed (CMake, Python 3, device-tree compiler, etc.)

### Set up the workspace

```bash
# Create and activate a Python virtual environment
python3 -m venv ~/posix-next/.venv
source ~/posix-next/.venv/bin/activate
pip install west

# Create a new west workspace with posix-next as the manifest repo
west init -m https://github.com/cfriedt/posix-next --mr main ~/posix-next
cd ~/posix-next
west update

# Re-home the manifest repo to modules/lib/posix (to workaround a bug in west)
mv posix-next modules/lib/posix
sed -i 's|manifest/path:.*|manifest/path = modules/lib/posix|' .west/config
```

### Install the Zephyr SDK

If you already have a Zephyr SDK installed, feel free to point the appropriate Zephyr environment
variables to it.

```bash
export ZEPHYR_BASE=$HOME/posix-next/zephyr
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
export ZEPHYR_SDK_INSTALL_DIR=/path/to/zephyr-sdk
```

Otherwise, install the SDK using
```bash
west sdk install
```

### Hello, POSIX world!

A primary goal for Zephyr's POSIX immplementation is to ensure that POSIX APIs can be used
portably, with as few source modifications as possible. This sample uses automatic thread
allocation as well as automatic thread stack allocation to conform to standard POSIX API
usage.

Below is an exerpt from the [samples/posix/hello_world](samples/posix/hello_world) app.

```c
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>

static void *worker(void *arg)
{
	printf("Hello World! %s from pthread_t %p\n", CONFIG_BOARD_TARGET,
		(void *)(uintptr_t)pthread_self());

	return (void *)(intptr_t)42;
}

int main(void)
{
	void *res;
	pthread_t tid;

	pthread_create(&tid, NULL, worker, NULL);
	pthread_join(tid, &res);

	printf("pthread_t returned %d\n", (int)(intptr_t)res);

	return 0;
}
```

### Build and run

To build and run on a simulator such as `native_sim` or `qemu_riscv64`, use the following command.

```bash
west build -b native_sim -t run samples/posix/hello_world
```

To build and run on real hardware, first build the application, and then flash it.

```bash
west build -b <your_board> samples/posix/hello_world
west flash
```

Output should appear approximately as shown below.

```bash
*** Booting Zephyr OS build v4.3.0 ***
Hello World! native_sim/native from pthread_t 0x8058400
pthread_t returned 42
```

### Hello, Zephyr world!

Some Zephyr apps may not use automatic thread / thread stack allocation since it can be viewed as
a form of dynamic memory allocation. Dynamic memory allocation is prohibited by some industry
standards for safety reasons. Supporting those applications, with as few pain points as possible,
is of paramount importance for both The Zephyr Project and Zephyr's POSIX reference implementation.

Although the POSIX API does not directly support operations on pre-initialized threads or
threading primatives, `posix-next` has moved to a model where there is a 1:1 correlation between
Zephyr and POSIX threading primitives.

- `k_condvar` <=> `pthread_cond_t`
- `k_mutex` <=> `pthread_mutex_t`
- `k_thread` <=> `pthread_t`

Below is an exerpt from the [samples/posix/hello_zephyr](samples/posix/hello_zephyr) app.

```c
#include <pthread.h>
#include <stdio.h>

#include <zephyr/kernel.h>

void worker_entry(void *p1, void *p2, void *p3)
{
	  printf("Hello World! %s from k_thread %p\n", CONFIG_BOARD_TARGET, k_current_get());

    /* Zephyr's native threading API still does not support a direct return value */
    k_thread_result_set(INT_TO_POINTER(42));
}

K_THREAD_DEFINE(kth, STACK_SIZE,
                worker_entry, NULL, NULL, NULL,
                K_LOWEST_APPLICATION_THREAD_PRIO, 0, SYS_FOREVER_MS);

int main(void)
{
    int ret;
    void *res;
    pthread_t th = (pthread_t)(uintptr_t)kth;

    k_thread_start(kth);
    pthread_join(th, &res);

    printf("k_thread returned %d\n", POINTER_TO_INT(res));

    return 0;
}
```

### Build and run

```bash
west build -b native_sim -t run samples/posix/hello_zephyr
```

To build for real hardware, substitute the board name:

```bash
west build -b <your_board> samples/posix/hello_zephyr
west flash
```

Output should appear approximately as shown below.

```bash
*** Booting Zephyr OS build v4.3.0 ***
Hello World! native_sim/native from k_thread 0x8057420
k_thread returned 42
```

### Build documentation locally

Building documentation for `posix-next` follows the same process for Zephyr. Before continuing,
please ensure that all [Zephyr documentation](https://docs.zephyrproject.org/latest/contribute/documentation/generation.html)
build requirements are met.

```bash
cmake -GNinja -Bdoc/_build doc
ninja -C doc/_build html
python3 -m http.server -d doc/_build/html --bind 127.0.0.1
```

## Repository layout

```
modules/lib/posix/
├── doc/                    # Sphinx documentation sources
│   ├── posix/              # Vendored POSIX guides (from Zephyr v4.3.0)
│   └── _static/            # Logo, CSS
├── include/zephyr/posix/   # Public POSIX headers (Doxygen documented)
├── lib/posix/options/      # Implementation sources
├── samples/posix/          # Example applications
├── tests/                  # Test suites (Twister)
└── west.yml                # West manifest (pins Zephyr v4.3.0)
```

## Contributing

Contributions are welcome! Please open an issue or pull request on
[GitHub](https://github.com/cfriedt/posix-next).

## License

This project is licensed under the [Apache License 2.0](LICENSE).
