# sysctrace

A lightweight Linux syscall tracer built from scratch in C using the `ptrace` API.
Computer Architecture course project, 2026.

## Features

- Intercepts every syscall issued by a target process
- Decodes 110+ x86-64 Linux syscalls by name
- Measures per-call latency with nanosecond precision (`CLOCK_MONOTONIC`)
- Classifies syscalls into 5 semantic categories (I/O, Memory, Network, Process, Other)
- Renders a structured terminal report on tracee exit
- Optional CSV export for external analysis
- Lower overhead than `strace` (~13% vs ~30% on I/O-heavy workloads)

## Build

```bash
make          # release build
make debug    # debug build (-g -O0)
make clean    # remove artifacts
```

## Usage

```bash
./sysctrace -- <program> [args...]
```

### Options

| Flag             | Description                                             |
|------------------|---------------------------------------------------------|
| `-h`, `--help`   | show help                                               |
| `-v`, `--version`| show version                                            |
| `-V`, `--verbose`| print every syscall (entry/exit) to stderr              |
| `-q`, `--quiet`  | suppress the summary report                             |
| `-c`, `--csv P`  | write `P_syscalls.csv` and `P_categories.csv`           |

### Examples

```bash
# Basic trace + summary
./sysctrace -- ls -la /tmp

# Verbose: print every syscall as it happens
./sysctrace -V -- curl -s https://example.com

# Export CSV for external analysis
./sysctrace --csv out -- cp /etc/hosts /tmp/hosts.bak
```

## Benchmarks

Run the benchmark suite (3 workloads × 3 conditions):

```bash
./bench/run_bench.sh
```

## Architecture


## Project Status

- [x] Phase 1: Project skeleton
- [x] Phase 2: ptrace core loop, syscall counting
- [x] Phase 3: Register reader, syscall name decoder, verbose mode
- [x] Phase 4: Timer, classifier, category report
- [x] Phase 5: CSV export, benchmark script
- [ ] Phase 6: Live in-class demo

## Authors

Denizhan Gepdiremen + Barış Birdal