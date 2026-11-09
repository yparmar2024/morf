# Phase timing and performance comparisons

Build an optimized binary before making performance claims:

```sh
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build build/release --parallel
```

Timing is selected at runtime, including in Release builds. It is not gated by
`DEBUG`: Debug builds are useful for correctness checks, but their timings should
not be compared with Release results.

## Commands

```sh
# Normal application: report startup immediately before the interactive loop.
build/release/cli/morf diff --timings assets/models/cube/base.obj assets/models/cube/target.obj

# Reproducible processing mode: no window and no merged output is written.
build/release/cli/morf diff --benchmark assets/models/cube/base.obj assets/models/cube/target.obj
build/release/cli/morf merge --benchmark assets/models/cube/base.obj assets/models/cube/ours.obj assets/models/cube/target.obj
```

Both flags write a single report to stderr. `--benchmark` implies timing and
processes input through session preparation, including automatic conflict
analysis for merges. A successful benchmark means processing completed; it does
not mean all merge conflicts were resolved. It never enters the save path.
Git LFS expansion still runs normally and may fetch/cache input objects.

Without these flags, phase boundaries perform a pointer check without clock
reads or report collection. There are no timers inside per-line or per-face
loops. Optional timing reports do not replace validation or error exit codes;
failures before startup completes produce a partial report marked failed.

Use `--` before positional arguments if a filename is also an option name.

## Reading the report

Reports show the build configuration, input names and resolved byte sizes, and
these processing phases:

- Input resolution / Git LFS.
- Each input model, with OBJ parsing and conversion shown separately.
- Mapping setup inside parsing: open, file inspection, and mapping creation.
- Model comparison, with canonicalization shown underneath.
- Session preparation and automatic merge analysis.
- Viewer startup when using `--timings` in a normal interactive command.

**Mapping setup is not a physical disk-read benchmark.** Mapped pages are accessed
on demand; page faults and line access are included in parsing. Token conversion
and model construction also remain part of parsing. The current instrumentation
does not isolate all those costs from each other.

Inclusive time includes nested phases. Exclusive time subtracts directly nested
phases; do not add inclusive values together. The startup/processing wall value
also includes uninstrumented work and instrumentation overhead. The normal
startup report excludes user interaction, later mesh rebuilds, and saving.
Viewer startup measures CPU wall time around window/resource setup, not completed
GPU execution or first-frame presentation.

## Performance PRs

Include results for performance-focused PRs, and for other PRs making a concrete
performance claim. Routine documentation and unrelated fixes do not need a
benchmark table. CI checks correctness/build quality; timing percentages should
not be pass/fail gates on shared runners.

For a comparison:

1. Use the same inputs, build type, compiler/options, machine, and timing mode.
2. Use identical instrumentation on both revisions. If the baseline lacks these
   flags, apply the same instrumentation or use a separate equivalent harness.
3. Run a warm-up, then at least seven measured runs per revision. Alternate the
   order when practical and compare medians, including the range of results.
4. State whether filesystem caches were warm or cold. Repeated runs normally
   measure warm-cache performance. Do not describe them as cold-disk throughput.
5. Verify equivalent parsed data and diagnostics, and keep generated fixtures
   stable. Timing improvements alone do not establish correctness.

Example PR section:

| Phase | Before median | After median | Change |
| --- | --- | --- | --- |
| Parse base OBJ | … ms | … ms | …% |
| Processing wall | … ms | … ms | …% |

Record input identity/size, machine/OS, compiler, Release flags, revisions, sample
count, cache state, and observed ranges. Compute time reduction as
`(before - after) / before * 100`; report regressions and inconclusive results too.
Avoid extrapolating one phase's improvement to the entire command.
