#!/usr/bin/env bash
# Pass 47 — smoke test for the reference / overlay tooling.
#
# Regenerates the reference pack, runs one overlay-diff against the
# existing Firestaff start-of-game capture, and checks the expected
# invariants hold.  Exit code 0 means the tooling is healthy.
#
# This is a tooling-health check, NOT a parity check: the diff is
# expected to have a large delta (reference viewport is a placeholder
# grid until ZONES.H / DOSBox capture lands).

set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO"

echo "[pass-47-verify] regenerating reference pack"
python3 tools/redmcsb_reference_compose.py >/dev/null

test -f reference-artifacts/redmcsb_reference_320x200.png
test -f reference-artifacts/provenance.json
test -d reference-artifacts/anchors
test "$(ls reference-artifacts/anchors/*.png 2>/dev/null | wc -l | tr -d ' ')" = "10"

echo "[pass-47-verify] running example overlay-diff"
python3 tools/redmcsb_overlay_diff.py \
    --firestaff verification-screens/01_ingame_start_latest.png \
    --reference reference-artifacts/redmcsb_reference_320x200.png \
    --region viewport \
    --out parity-evidence/overlays/pass47_example_viewport >/dev/null

test -f parity-evidence/overlays/pass47_example_viewport.mask.png
test -f parity-evidence/overlays/pass47_example_viewport.stats.json

echo "[pass-47-verify] validating anchor size-match invariant (10/10 expected)"
python3 -c "
import json, sys
prov = json.load(open('reference-artifacts/provenance.json'))
anchors = prov['outputs']['anchors']
ok = sum(1 for a in anchors if a.get('size_match'))
total = len(anchors)
print(f'  anchors matching DEFS.H: {ok}/{total}')
sys.exit(0 if ok == total else 1)
"

echo "[pass-47-verify] validating DOSBox harness stage (no emulator required)"
bash scripts/dosbox_dm1_capture.sh >/tmp/pass47_dosbox_stage.log 2>&1 || {
    echo "stage log:"; cat /tmp/pass47_dosbox_stage.log; exit 1
}
test -f verification-screens/dm1-dosbox-capture/dosbox.conf
test -d verification-screens/dm1-dosbox-capture/DungeonMasterPC34
test -f verification-screens/dm1-dosbox-capture/DungeonMasterPC34/DM.EXE

echo "[pass-47-verify] OK"
