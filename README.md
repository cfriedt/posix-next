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
  <a href="https://app.codecov.io/gh/cfriedt/posix-next">
    <img src="https://codecov.io/gh/cfriedt/posix-next/branch/main/graph/badge.svg" alt="Coverage">
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
| Getting started | [Getting Started](https://cfriedt.github.io/posix-next/getting_started/index.html) |
| API reference (Doxygen) | [Doxygen HTML](https://cfriedt.github.io/posix-next/doxygen/html/index.html) |
| POSIX symbol search | [Symbol search](https://cfriedt.github.io/posix-next/posix/symbols.html) |
| Option groups | [Option groups](https://cfriedt.github.io/posix-next/posix/option_groups/index.html) |
| Conformance status | [Conformance](https://cfriedt.github.io/posix-next/posix/conformance/index.html) |

## Quick start

See the full
[Getting Started](https://cfriedt.github.io/posix-next/getting_started/index.html)
guide for workspace setup, SDK installation, and sample applications.

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
