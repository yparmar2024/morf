# Development

Morf uses C++17 and CMake. Downloaded dependencies are configured automatically.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build build --parallel
```

`CMAKE_POLICY_VERSION_MINIMUM` allows the existing Raylib dependency to configure
with CMake 4; older CMake versions may report it as unused.

## Formatting

Install clang-format (CI currently uses Homebrew's version, 23). The checked-in
`.clang-format` defines four-space indentation, indented namespaces, attached
braces, and a 120-column limit. Quoted includes precede system includes; both
groups are alphabetically ordered, including the matching source header.

```sh
cmake -S . -B build
cmake --build build --target format
cmake --build build --target format-check
```

Use the same formatter version as CI, which prints its version in the job log.
Reconfigure CMake after installing tools so their targets become available.
Formatting includes Morf's C++ files in `cli/`, `core/`, `gui/`, and `support/`.

## Static analysis

Cppcheck checks Morf's sources for errors, warnings, and portability issues.
Use a Makefile or Ninja generator with `CMAKE_EXPORT_COMPILE_COMMANDS` enabled
to get the `cppcheck` target. The target fails on reported findings and filters
the compilation database to Morf's source directories.

```sh
cmake --build build --target cppcheck
```

On macOS, install the tools with `brew install clang-format cppcheck`. An ordinary
build does not require either tool. No Python scripts or test framework are added.

## Continuous integration

GitHub Actions builds on Ubuntu and macOS and runs formatting and Cppcheck on
macOS for pull requests and pushes to `main`, `refactor/**`, and `chore/**`.
Checks report results without modifying source files, committing fixes, deploying,
or opening a graphical viewer. No automated test suite is configured yet.

Performance/style suggestions are deferred; this first pass gates correctness
and portability without requiring algorithm or optimization changes.

## Phase timing

Use `morf diff --timings <base> <target>` for an interactive startup report or
`morf diff --benchmark <base> <target>` for processing without a viewer.
The same flags work with `merge`; benchmark mode never saves merged output.
Reports are available in Release builds and are disabled by default.
