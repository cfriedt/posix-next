#!/usr/bin/env python3
#
# Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
#
# SPDX-License-Identifier: Apache-2.0
"""
Draw the system calls a Linux process made on Zephyr as a graph.

Reads a console log holding the lines the Linux hello sample's trace hook
prints ('linux-syscall: pid=<n> <name>'), one per call, and writes a Graphviz
graph: a node per system call carrying its count, an edge per transition from
one call to the next carrying how often it happened, one cluster per process
(a fork() child's calls are its own sequence) unless --merge is given.

  west build -t run | tee hello.log
  scripts/linux_syscall_graph.py hello.log --svg hello.svg
"""

import argparse
import re
import subprocess
import sys
from collections import Counter

LINE = re.compile(r"linux-syscall: pid=(\d+) (\w+)")


def read_calls(lines, pid=None):
    """[(pid, name)] in log order."""
    calls = []
    for line in lines:
        m = LINE.search(line)
        if m and (pid is None or int(m.group(1)) == pid):
            calls.append((int(m.group(1)), m.group(2)))
    return calls


def chain_dot(names, prefix, indent, out):
    """Nodes with counts and transition edges of one sequence of calls."""
    counts = Counter(names)
    edges = Counter(zip(names, names[1:]))

    for name, n in counts.most_common():
        out.append(f'{indent}"{prefix}{name}" [label="{name}\\n{n}"];')
    out.append(f'{indent}"{prefix}start" [shape=point]; '
               f'"{prefix}end" [shape=doublecircle, label="", width=0.2];')
    out.append(f'{indent}"{prefix}start" -> "{prefix}{names[0]}";')
    out.append(f'{indent}"{prefix}{names[-1]}" -> "{prefix}end";')
    for (a, b), n in sorted(edges.items(), key=lambda kv: -kv[1]):
        label = f' [label="{n}"]' if n > 1 else ""
        out.append(f'{indent}"{prefix}{a}" -> "{prefix}{b}"{label};')


def to_dot(calls, title, merge):
    """One chain, or one cluster per process: a fork() child's calls are its own."""
    pids = sorted({pid for pid, _ in calls}, key=lambda p: next(i for i, c in enumerate(calls)
                                                                 if c[0] == p))
    out = [
        f'digraph "{title}" {{',
        "\trankdir=LR;",
        '\tfontname="Helvetica"; node [shape=box, style=rounded, fontname="Helvetica"];',
        '\tedge [fontname="Helvetica", fontsize=10];',
        f'\tlabel="{title}: {len(calls)} system calls, {len({n for _, n in calls})} distinct, '
        f'{len(pids)} process(es)";',
    ]
    if merge or len(pids) == 1:
        chain_dot([name for _, name in calls], "", "\t", out)
    else:
        for pid in pids:
            names = [name for p, name in calls if p == pid]
            out.append(f"\tsubgraph cluster_{pid} {{")
            out.append(f'\t\tlabel="pid {pid}: {len(names)} calls"; style=rounded; color=gray;')
            chain_dot(names, f"{pid}/", "\t\t", out)
            out.append("\t}")
    out.append("}")
    return "\n".join(out) + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("log", nargs="?", type=argparse.FileType("r", errors="replace"),
                        default=sys.stdin, help="console log (default: stdin)")
    parser.add_argument("--pid", type=int, help="only this process's calls")
    parser.add_argument("--title", default="linux syscalls")
    parser.add_argument("--merge", action="store_true",
                        help="one chain for all processes instead of a cluster per pid")
    parser.add_argument("-o", "--dot", help="write the Graphviz source here (default: stdout)")
    parser.add_argument("--svg", help="also render with dot(1) to this file")
    args = parser.parse_args()

    calls = read_calls(args.log, args.pid)
    if not calls:
        sys.exit("no 'linux-syscall:' lines found")
    dot = to_dot(calls, args.title, args.merge)
    if args.dot:
        with open(args.dot, "w") as fp:
            fp.write(dot)
    else:
        sys.stdout.write(dot)
    if args.svg:
        subprocess.run(["dot", "-Tsvg", "-o", args.svg], input=dot.encode(), check=True)
    names = [name for _, name in calls]
    print(f"{len(calls)} calls, {len(set(names))} distinct: "
          + ", ".join(f"{n} {c}" for c, n in Counter(names).most_common(8)), file=sys.stderr)


if __name__ == "__main__":
    main()
