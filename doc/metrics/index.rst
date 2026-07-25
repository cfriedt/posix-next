.. _metrics:

Metrics
#######

This section will grow to include longitudinal charts for project health and
quality signals.

Test coverage
-------------

Interactive coverage reports (line and diff coverage, PR comments, and history)
are published on `Codecov <https://app.codecov.io/gh/cfriedt/posix-next>`_.

The `Coverage <https://github.com/cfriedt/posix-next/actions/workflows/coverage.yml>`__
workflow on ``main`` also commits gcovr JSON snapshots under ``doc/metrics/`` when
nightly results change:

- ``coverage-full.json`` — gcovr merge of per-shard ``coverage-full.json`` artifacts (zephyr + ``modules/lib/posix``)
- ``coverage-posix.json`` — gcovr merge of per-shard ``coverage-posix.json`` artifacts (``modules/lib/posix/include`` and ``modules/lib/posix/lib/posix``; same file uploaded to Codecov per shard)
- ``coverage-provenance.json`` — ``main`` commit (full and short SHA), run URL, and summary percentages

Scheduled nightly runs compare ``origin/main`` to the ``commit`` field in
``coverage-provenance.json`` (on ``main``, or on the open ``automation/coverage-json``
bot PR when snapshots have not merged yet). When ``main`` is unchanged, the
nightly coverage job is skipped.

Use Codecov for browsing source coverage; use the committed JSON for offline
diffs or tooling.

Twister results
---------------

The `Twister <https://github.com/cfriedt/posix-next/actions/workflows/twister.yml>`__
workflow's scheduled runs commit an abridged results summary when outcomes
change (via the ``automation/twister-summary`` bot PR):

- ``twister-summary.json`` — per Option Group, per twister scenario variant
  (``base``, ``minimal``, ``linux_compat``, ...): aggregate status, instance
  counts, and platforms, plus provenance (commit, run URL, timestamp). It is
  produced by ``scripts/ci/twister-summarize.py`` from the per-shard
  ``twister.json`` reports.

At documentation build time, ``scripts/doc/posix_metrics.py`` combines this
summary with ``coverage-posix.json``, the Option Group tables, and the curated
``doc/*.yaml`` metadata to render the badges on the
:ref:`Option Group pages <posix_option_groups>` (via the ``posix_badges``
Sphinx extension). Missing or stale snapshots degrade to absent badges; doc
builds never require fresh CI data.

.. raw:: html

   <p>
     <a class="reference external" href="https://app.codecov.io/gh/cfriedt/posix-next">
       View coverage on Codecov
     </a>
   </p>
