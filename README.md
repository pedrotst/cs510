# CS 510 — Program Analysis (Purdue, 2023)

Two projects building program-analysis tooling: one **static**, as an LLVM pass over LLVM
IR; one **dynamic**, as a Valgrind tool over VEX IR.

## `proj2` — a static leak detector, written as an LLVM pass

[`proj2/LeakDetector.cpp`](proj2/LeakDetector.cpp)

An LLVM analysis pass in C++. It enumerates control-flow paths across the basic blocks of a
function, tracks heap allocation and release along each path, and reports leaks mapped back
to source lines through `getDebugLoc()`. Built against the legacy pass manager
(`llvm/Pass.h`, `LegacyPassManager`, `PassManagerBuilder`).

Test inputs live in [`proj2/testcases`](proj2/testcases).

## `proj1` — a dynamic dependency tracker, written as a Valgrind tool

[`proj1/valgrind-3.18.1/depAnalysis/depAnalysis.c`](proj1/valgrind-3.18.1/depAnalysis/depAnalysis.c)

A custom Valgrind tool in C that instruments **VEX IR** to trace data dependencies through a
running binary. For each superblock it deep-copies the `IRSB`, walks the `IRStmt` and
`IRExpr` nodes, and inserts dirty-call instrumentation on loads, stores and temporary
assignments, emitting a dependency trace as the program executes.

Test programs and expected traces are in [`proj1/testcases`](proj1/testcases).

### Building it

`proj1` vendors the complete Valgrind 3.18.1 tree, so `depAnalysis` builds the way any
Valgrind tool does — configure the tree, `make install` into a local prefix, then run it
with `--tool=depAnalysis`.

## Attribution and licence

`proj1/valgrind-3.18.1` is [Valgrind](https://valgrind.org/) 3.18.1, GPLv2, unmodified
except for the added `depAnalysis` tool. `depAnalysis.c` derives from **Nulgrind** (Nicholas
Nethercote) and retains its copyright header.

This is coursework rather than a maintained project, and is offered as-is.
