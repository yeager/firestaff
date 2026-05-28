#!/bin/sh
# run_firestaff_dm1_v1_capture_reference_check_probe.sh
#
# Headless deterministic probe: checks canonical DM1 PC 3.4 game data SHA256 hashes,
# verifies existence of original DOSBox capture directories, and reports their
# fingerprint status.
#
# No build step required - this is a standalone C probe compiled with stock -std=c99.
# No game files needed to run - only checks paths and computes fast fingerprints.
#
# Usage:
#   ./run_firestaff_dm1_v1_capture_reference_check_probe.sh
#   ./run_firestaff_dm1_v1_capture_reference_check_probe.sh /path/to/output_dir
#
# Output:
#   Prints probe results to stdout.
#   Exit code 0 = all canonical data present, 1 = missing or impaired references.
#
# See:
#   docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md
#   docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md
#   probes/dm1/firestaff_dm1_v1_capture_reference_check_probe.c

set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
REPO_ROOT="$(cd -- "$HERE/../.." >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-"$REPO_ROOT/verification-m11/dm1v1-capture-ref-check-$(date +%Y%m%d-%H%M%S)"}

mkdir -p "$OUT_DIR"

# Compile and run the probe
cc -std=c99 -Wall -Wextra -pedantic \
    -I"$REPO_ROOT" \
    "$REPO_ROOT/probes/dm1/firestaff_dm1_v1_capture_reference_check_probe.c" \
    -o "$OUT_DIR/capture_ref_check"

"$OUT_DIR/capture_ref_check" > "$OUT_DIR/probe.log" 2>&1 || probe_exit=$?
probe_exit=${probe_exit:-0}
cat "$OUT_DIR/probe.log"

echo ""
echo "Output saved to: $OUT_DIR/probe.log"
echo "Result: $(test $probe_exit -eq 0 && echo PASS || echo FAIL)"

exit $probe_exit
