#!/usr/bin/env bash
# bench/run_bench.sh — sysctrace benchmark runner
#
# Compares wall-clock time for three conditions × three workloads.
# Each (condition, workload) is run N times; reports median.

set -euo pipefail

# Force C locale so 'time' uses '.' as decimal separator
export LC_ALL=C
export LANG=C

# Run from project root
cd "$(dirname "$0")/.."

if [ ! -x ./sysctrace ]; then
    echo "error: ./sysctrace not built. run 'make' first." >&2
    exit 1
fi

ITERATIONS=${ITERATIONS:-10}
TMPFILE=$(mktemp -d)
SRC_FILE="/usr/bin/bash"
DST_FILE="$TMPFILE/bash.bak"

cleanup() { rm -rf "$TMPFILE"; }
trap cleanup EXIT

# Time a command in seconds (high precision) using bash's built-in.
# Returns a single float number on stdout.
time_one() {
    local cmd="$1"
    # Use bash's TIMEFORMAT to get just the real time in seconds
    local out
    out=$( { TIMEFORMAT='%R'; time bash -c "$cmd" >/dev/null 2>/dev/null; } 2>&1 )
    echo "$out"
}

median_time() {
    local cmd="$1"
    local n="$ITERATIONS"
    local times=()
    for ((i=0; i<n; i++)); do
        local t
        t=$(time_one "$cmd")
        times+=("$t")
    done
    printf '%s\n' "${times[@]}" | sort -n | awk '
        { a[NR]=$1+0 }
        END {
            n=NR
            if (n==0) { print "0"; exit }
            if (n%2==1) printf "%.4f\n", a[(n+1)/2]
            else        printf "%.4f\n", (a[n/2]+a[n/2+1])/2
        }'
}

run_workload() {
    local name="$1"
    local cmd="$2"

    echo ""
    echo "Workload: $name"
    echo "──────────────────────────────────────────────"

    local t_untraced t_sysctrace t_strace
    t_untraced=$(median_time "$cmd")
    t_sysctrace=$(median_time "./sysctrace -q -- $cmd")
    t_strace=$(median_time "strace -o /dev/null $cmd")

    local oh_sys oh_str
    if awk -v a="$t_untraced" 'BEGIN { exit !(a > 0) }'; then
        oh_sys=$(awk -v a="$t_untraced" -v b="$t_sysctrace" \
            'BEGIN { printf "%.1f", (b-a)/a*100 }')
        oh_str=$(awk -v a="$t_untraced" -v b="$t_strace" \
            'BEGIN { printf "%.1f", (b-a)/a*100 }')
    else
        oh_sys="?"
        oh_str="?"
    fi

    printf "  untraced  : %s s\n" "$t_untraced"
    printf "  sysctrace : %s s   (+%s%%)\n" "$t_sysctrace" "$oh_sys"
    printf "  strace    : %s s   (+%s%%)\n" "$t_strace"   "$oh_str"
}

echo "═══════════════════════════════════════════════"
echo " sysctrace benchmark — $ITERATIONS iterations each"
echo "═══════════════════════════════════════════════"

# Syscall-bound workloads: many short syscalls, where sysctrace's
# deferred-formatting design wins over strace's per-call formatting.

run_workload "find / (5 levels deep)" \
  "find / -maxdepth 5 -type f 2>/dev/null | head -2000 > /dev/null"

run_workload "tar /usr/include" \
  "tar -cf $TMPFILE/out.tar /usr/include 2>/dev/null"

run_workload "stat 2000 files" \
  "find /usr/lib -maxdepth 2 -type f 2>/dev/null | head -10000 | xargs -r stat > /dev/null"

echo ""
echo "═══════════════════════════════════════════════"