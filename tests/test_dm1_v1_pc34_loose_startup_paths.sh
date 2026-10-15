#!/usr/bin/env bash
set -euo pipefail
if [[ $# -lt 1 || $# -gt 2 ]]; then
    printf 'usage: %s <firestaff> [original-PC34-install-directory]\n' "$0" >&2
    exit 2
fi
app=$1
original=${2:-${FIRESTAFF_DM1_PC34_LOOSE_INSTALL:-}}
if [[ ! -x "$app" || ! -d "$original" ]] || ! command -v python3 >/dev/null 2>&1; then
    echo 'SKIP: Firestaff or original PC34 loose installation is unavailable'
    exit 77
fi
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/dm1-pc34-loose-startup.XXXXXX")
cleanup() {
    rc=$?
    if [[ $rc != 0 ]]; then
        for log in "$scratch"/*.log; do [[ ! -f "$log" ]] || cat "$log" >&2; done
    fi
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
}
trap cleanup EXIT
unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR FIRESTAFF_EXIT_AFTER_LAUNCH FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS
python3 - "$original" "$scratch" <<'PYDATA'
import hashlib
import json
import pathlib
import shutil
import sys

original, root = map(pathlib.Path, sys.argv[1:])
expected = {
    "DATA/GRAPHICS.DAT": (363417, "fa6b1aa29e191418713bf2cda93d962e"),
    "DATA/DUNGEON.DAT": (33357, "766450c940651fc021c92fe5d0d0b3a6"),
    "SELECTOR": (15474, "165ae5438ce2686ae62f2fbb61f3e831"),
}
for name, (size, digest) in expected.items():
    data = (original / name).read_bytes()
    if len(data) != size or hashlib.md5(data).hexdigest() != digest:
        raise SystemExit(f"FAIL: authentic PC34 identity mismatch: {name}")
title = (original / "TITLE").read_bytes()
if len(title) != 12002 or hashlib.sha256(title).hexdigest() != "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745":
    raise SystemExit("FAIL: authentic PC34 TITLE identity mismatch")
manifest = {str(p.relative_to(original)): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in original.rglob("*") if p.is_file()}
(root / "original-hashes.json").write_text(json.dumps(manifest), encoding="utf-8")
for fixture in ("canonical", "lower", "bad-title", "neighbor-selector"):
    shutil.copytree(original, root / fixture)
(root / "lower/DATA").rename(root / "lower/data")
(root / "lower/TITLE").rename(root / "lower/Title")
(root / "lower/SELECTOR").rename(root / "lower/selector")
# An original SELECTOR is not a TITLE: its failed fingerprint must not borrow
# the valid original TITLE one level above the selected DATA directory.
shutil.copyfile(original / "SELECTOR", root / "bad-title/DATA/TITLE")
# An arbitrary graphics folder is not a DOS DATA layout. A valid neighboring
# SELECTOR must not replace its missing local companion after TITLE succeeds.
(root / "neighbor-selector/DATA").rename(root / "neighbor-selector/assets")
shutil.copyfile(original / "TITLE", root / "neighbor-selector/assets/TITLE")
PYDATA
run_route() {
    local label=$1 media=$2 route=$3
    local home="$scratch/$label-home"
    local args=(--game dm1 --platform pc --data-dir "$media" --debug)
    mkdir -p "$home"
    if [[ "$route" = menu ]]; then
        args+=(--menu --script wait20,enter,wait20,enter,wait20,enter)
    fi
    HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
    FIRESTAFF_CONFIG_PATH="$home/config.toml" FIRESTAFF_AUTOTEST=1 \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" "${args[@]}" --duration 12000 >"$scratch/$label.log" 2>&1
}
require_startup() {
    local label=$1 graphics=$2
    for expected in \
        'startup-source-complete game=dm1 platform=pc34 phase=anim-title frames=53' \
        'startup-source-complete game=dm1 platform=pc34 phase=selector ' \
        'frames=32 source-vblanks=32' 'handoff=fires command=enter' \
        'DM1 READY: gameId=dm1' 'level-loaded=1' "graphicsPath=$graphics"; do
        if ! grep -Fq "$expected" "$scratch/$label.log"; then
            printf 'FAIL: original PC34 %s lacks %s\n' "$label" "$expected" >&2
            exit 1
        fi
    done
}
for route in cli menu; do
    run_route "canonical-$route" "$scratch/canonical/DATA" "$route"
    require_startup "canonical-$route" "$scratch/canonical/DATA/GRAPHICS.DAT"
    run_route "lower-$route" "$scratch/lower/data" "$route"
    require_startup "lower-$route" "$scratch/lower/data/GRAPHICS.DAT"
    run_route "selected-file-$route" "$scratch/lower/data/GRAPHICS.DAT" "$route"
    require_startup "selected-file-$route" "$scratch/lower/data/GRAPHICS.DAT"
done
run_route bad-title "$scratch/bad-title/DATA" cli
if grep -Fq 'phase=anim-title frames=53' "$scratch/bad-title.log"; then
    echo 'FAIL: invalid local TITLE borrowed a neighboring original' >&2
    exit 1
fi
grep -Fq 'reason=fingerprint-mismatch' "$scratch/bad-title.log"
run_route neighbor-selector "$scratch/neighbor-selector/assets" cli
grep -Fq 'phase=anim-title frames=53' "$scratch/neighbor-selector.log"
if grep -Fq 'startup-source-complete game=dm1 platform=pc34 phase=selector ' "$scratch/neighbor-selector.log"; then
    echo 'FAIL: missing SELECTOR borrowed outside its selected installation' >&2
    exit 1
fi
grep -Fq 'phase=selector reason=missing-or-unverified-source-media' "$scratch/neighbor-selector.log"
: >"$scratch/Case"
if [[ ! -e "$scratch/case" ]]; then
    rm "$scratch/Case"
    cp "$scratch/canonical/TITLE" "$scratch/canonical/DATA/TITLE"
    cp "$scratch/canonical/TITLE" "$scratch/canonical/DATA/title"
    run_route ambiguous-title "$scratch/canonical/DATA" cli
    grep -Eq 'startup-sibling-rejected .*wanted=TITLE .*reason=ambiguous' "$scratch/ambiguous-title.log"
    if grep -Fq 'phase=anim-title frames=53' "$scratch/ambiguous-title.log"; then
        echo 'FAIL: ambiguous TITLE borrowed its valid parent companion' >&2
        exit 1
    fi
    rm "$scratch/canonical/DATA/TITLE" "$scratch/canonical/DATA/title"
    cp "$scratch/canonical/SELECTOR" "$scratch/canonical/DATA/SELECTOR"
    cp "$scratch/canonical/SELECTOR" "$scratch/canonical/DATA/selector"
    run_route ambiguous-selector "$scratch/canonical/DATA" cli
    grep -Eq 'startup-sibling-rejected .*wanted=SELECTOR .*reason=ambiguous' "$scratch/ambiguous-selector.log"
    if grep -Fq 'startup-source-complete game=dm1 platform=pc34 phase=selector ' "$scratch/ambiguous-selector.log"; then
        echo 'FAIL: ambiguous SELECTOR borrowed its valid parent companion' >&2
        exit 1
    fi
    echo 'PASS: original PC34 ambiguous companions reject parent fallback'
else
    rm "$scratch/Case"
    echo 'SKIP: case-sensitive storage required for PC34 ambiguity subcases'
fi
python3 - "$original" "$scratch/original-hashes.json" <<'PYHASH'
import hashlib
import json
import pathlib
import sys
original = pathlib.Path(sys.argv[1])
with open(sys.argv[2], encoding="utf-8") as stream:
    expected = json.load(stream)
actual = {str(p.relative_to(original)): hashlib.sha256(p.read_bytes()).hexdigest()
          for p in original.rglob("*") if p.is_file()}
if actual != expected:
    raise SystemExit("FAIL: original PC34 installation changed during startup checks")
PYHASH
echo 'PASS: original PC34 loose startup path ownership'
