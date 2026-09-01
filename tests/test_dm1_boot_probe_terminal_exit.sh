#!/bin/sh
set -eu

firestaff_bin="${1:-build/ninja-dm2/firestaff}"
data_source="${FIRESTAFF_DM1_BOOT_PROBE_DATA:-$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip}"
runtime_root="${FIRESTAFF_TEST_RUNTIME_DIR:-$(pwd)/test-runtime}"
timeout_bin="${TIMEOUT_BIN:-}"

if [ -z "$timeout_bin" ]; then
    if command -v gtimeout >/dev/null 2>&1; then
        timeout_bin="gtimeout"
    elif command -v timeout >/dev/null 2>&1; then
        timeout_bin="timeout"
    else
        echo "skip: no gtimeout/timeout available" >&2
        exit 77
    fi
fi

if [ ! -x "$firestaff_bin" ]; then
    echo "skip: firestaff binary not executable: $firestaff_bin" >&2
    exit 77
fi

if [ ! -f "$data_source" ]; then
    echo "skip: authentic DM1 PC 3.4 ZIP not found: $data_source" >&2
    exit 77
fi

mkdir -p "$runtime_root"
out_file="$runtime_root/firestaff_dm1_boot_probe_terminal_exit.$$"
trap 'rm -f "$out_file"' EXIT

set +e
SDL_VIDEODRIVER=dummy "$timeout_bin" 20s "$firestaff_bin" \
    --game dm1 \
    --platform pc \
    --data-dir "$data_source" \
    --boot-probe \
    --boot-probe-frames 90 \
    --duration 0 >"$out_file" 2>&1
rc=$?
set -e

if [ "$rc" -ne 0 ]; then
    sed -n '1,120p' "$out_file" >&2
    echo "fail: DM1 boot-probe exited with $rc" >&2
    exit 1
fi

if ! grep -q "FIRESTAFF BOOT PROBE READY" "$out_file"; then
    sed -n '1,120p' "$out_file" >&2
    echo "fail: DM1 boot-probe did not print READY receipt" >&2
    exit 1
fi

if ! grep -q "phase=dm1-runtime" "$out_file"; then
    sed -n '1,120p' "$out_file" >&2
    echo "fail: DM1 boot-probe did not reach dm1-runtime" >&2
    exit 1
fi

if grep -q "boot-probe expected selected-entry source" "$out_file"; then
    sed -n '1,120p' "$out_file" >&2
    echo "fail: DM1 boot-probe regressed selected-entry receipt validation" >&2
    exit 1
fi

if grep -q "phase-a run failed" "$out_file"; then
    sed -n '1,120p' "$out_file" >&2
    echo "fail: DM1 boot-probe regressed phase-a completion" >&2
    exit 1
fi

echo "ok: DM1 boot-probe terminal exit returned rc=0 with runtime READY receipt"
