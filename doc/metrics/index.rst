.. _metrics:

Metrics
#######

This section will grow to include longitudinal charts for project health and
quality signals.

Badge data flow
---------------

Every metrics producer publishes its summary as a workflow artifact; nothing
is committed to git. At documentation build time the latest completed run of
each producer on ``main`` is downloaded into the gitignored
``doc/metrics/*.json``, and the docs redeploy automatically whenever a
producer or a push to ``main`` completes. Local builds via
``scripts/ci/docs.sh`` fetch the same artifacts (see ``docs.sh --help``;
the token defaults to ``~/.ghtoken``). Missing or stale summaries degrade
to absent badges; doc builds never require fresh CI data.

Test coverage
-------------

Interactive coverage reports (line and diff coverage, PR comments, and history)
are published on `Codecov <https://app.codecov.io/gh/cfriedt/posix-next>`_.

The `Coverage <https://github.com/cfriedt/posix-next/actions/workflows/coverage.yml>`__
workflow uploads the merged gcovr JSON as the ``coverage-json-snapshot``
artifact:

- ``coverage-full.json`` — gcovr merge of per-shard ``coverage-full.json`` artifacts (zephyr + ``modules/lib/posix``)
- ``coverage-posix.json`` — gcovr merge of per-shard ``coverage-posix.json`` artifacts (``modules/lib/posix/include`` and ``modules/lib/posix/lib/posix``; same file uploaded to Codecov per shard)
- ``coverage-provenance.json`` — ``main`` commit (full and short SHA), run URL, and summary percentages

Scheduled nightly runs are skipped when ``main`` is unchanged since the last
Codecov upload.

Use Codecov for browsing source coverage; use the artifact JSON for offline
diffs or tooling.

Twister results
---------------

The `Twister <https://github.com/cfriedt/posix-next/actions/workflows/twister.yml>`__
workflow's scheduled runs upload an abridged results summary as the
``twister-summary`` artifact:

- ``twister-summary.json`` — per Option Group, per twister scenario
  variant (``base``, ``minimal``, ``linux_compat``, ``userspace``, ...):
  aggregate status, instance counts, and platforms, plus provenance
  (commit, run URL, timestamp). Produced by
  ``scripts/ci/twister-summarize.py`` from the per-shard ``twister.json``
  reports.
- ``asan-summary.json`` / ``ubsan-summary.json`` — the same summary
  shape for the nightly AddressSanitizer / UndefinedBehaviorSanitizer runs
  (``native_sim`` only, ``--enable-asan`` / ``--enable-ubsan`` with
  ``CONFIG_NO_OPTIMIZATIONS=y``), collapsed into a single ``asan`` /
  ``ubsan`` variant per group, uploaded as the ``asan-summary`` /
  ``ubsan-summary`` artifacts by the dedicated
  `ASAN <https://github.com/cfriedt/posix-next/actions/workflows/asan.yml>`__
  and `UBSAN <https://github.com/cfriedt/posix-next/actions/workflows/ubsan.yml>`__
  workflows. Each scenario also records
  ``failed_functions``: the implementation functions implicated by
  sanitizer reports (extracted from twister logs by
  ``scripts/ci/sanitizer-findings.py``). Sanitizers only report what
  fails, so a function absent from every report renders as passing.
- ``static-analysis.json`` — clang static analyzer (scan-build /
  ``analyze-build``) findings from a ``--build-only`` twister run,
  produced by ``scripts/ci/static-analysis.py`` in the dedicated
  `Scan-Build <https://github.com/cfriedt/posix-next/actions/workflows/scan-build.yml>`__
  workflow (artifact ``static-analysis-summary``): analyzed files plus
  per-finding file, line, enclosing function, checker, and description.
  Coverage spans the module implementation and public headers as well as
  the Zephyr-tree sources introduced by the module's patch series, with
  every distinct Kconfig variant of each file analyzed (deduplicated by
  preprocessed content), cross-translation-unit analysis, and Z3
  refutation of findings. A group whose files were analyzed shows a
  passing badge unless a finding lands in them.

At documentation build time, ``scripts/doc/posix_metrics.py`` combines this
summary with ``coverage-posix.json``, the Option Group tables, and the curated
``doc/*.yaml`` metadata to render the badges on the
:ref:`Option Group pages <posix_option_groups>` (via the ``posix_badges``
Sphinx extension).

.. raw:: html

   <p>
     <a class="reference external" href="https://app.codecov.io/gh/cfriedt/posix-next">
       View coverage on Codecov
     </a>
   </p>
