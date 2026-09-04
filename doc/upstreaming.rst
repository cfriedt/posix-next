.. _posix_next_upstreaming:

Upstreaming Zephyr patches
##########################

posix-next carries its Zephyr tree changes as plain unified diffs under
``zephyr/patches/zephyr/``, registered in ``zephyr/patches.yml`` and applied with
``west patch apply``. The ``deps``, ``status``, ``sync`` and ``submit`` sub-commands of
``west patch`` (added by ``west-patch-upstream.patch``, itself part of the series) keep the
upstreaming bookkeeping in that file current and open draft pull requests against
`zephyrproject-rtos/zephyr <https://github.com/zephyrproject-rtos/zephyr>`_.

Prerequisites
*************

- ``gh`` (GitHub CLI) authenticated with an account that has a fork of Zephyr
- git remotes in ``zephyr/`` for the upstream repository and for your fork
- ``ruamel.yaml`` in the Zephyr Python environment (a dependency of ``pykwalify``, so
  already present); ``patches.yml`` is edited in place and left untouched unless it
  round-trips byte-for-byte

Bookkeeping lives on ``main``, so run the commands against a worktree of this module that is
branched from ``main`` rather than against a feature checkout, for example:

.. code-block:: bash

   git -C modules/lib/posix worktree add -b upstream-sync ../../../worktrees/posix/upstream main

Daily workflow
**************

.. code-block:: bash

   P="-l worktrees/posix/upstream/zephyr/patches.yml -b worktrees/posix/upstream/zephyr/patches"
   west patch $P sync --commit          # record merged / closed PRs from GitHub
   west patch $P deps --write --commit  # refresh detected dependencies (cached; fast)
   west patch $P -dm zephyr status      # what is ready, blocked, or needs a rebase
   west patch $P -dm zephyr submit -n 2 # open up to two draft PRs for ready patches

``submit`` commits each chosen patch on a branch from upstream ``main`` (built in a temporary
index, no checkout of ``zephyr/`` is needed), pushes it as ``patches/<patch-name>`` to your
fork, opens a **draft** PR and records the URL as ``merge-pr``. Only patches whose
dependencies are already merged upstream are submitted, one PR per patch unless ``--group``
is given. Splitting the commit into implementation, tests and documentation, editing the
generated message, and taking the PR out of draft are left to the human. ``--dry-run`` builds
the branch locally and prints the PR title and body without pushing.

The statuses, the ``custom.upstream`` keys (``ignore``, ``depends-on``,
``manual-depends-on``, ``title``, ``branch``) and the ``patch.*`` west config keys are
described in the Zephyr documentation of ``west patch`` carried by the patch
(``doc/develop/west/zephyr-cmds.rst``).
