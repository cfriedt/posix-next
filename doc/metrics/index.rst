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

- ``coverage-full.json`` — workspace-wide merged trace
- ``coverage-posix.json`` — filtered to ``modules/lib/posix/include`` and ``modules/lib/posix/lib``
- ``coverage-provenance.json`` — commit, run URL, and summary percentages

Use Codecov for browsing source coverage; use the committed JSON for offline
diffs or tooling.

.. raw:: html

   <p>
     <a class="reference external" href="https://app.codecov.io/gh/cfriedt/posix-next">
       View coverage on Codecov
     </a>
   </p>
