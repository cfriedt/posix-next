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

.. raw:: html

   <p>
     <a class="reference external" href="https://app.codecov.io/gh/cfriedt/posix-next">
       View coverage on Codecov
     </a>
   </p>
