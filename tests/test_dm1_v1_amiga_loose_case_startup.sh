#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"
if [[ $# -ne 2 ]]; then
    printf 'usage: %s <firestaff> <original-Amiga-demo.zip>\n' "$0" >&2
    exit 2
fi
app=$1
archive=$2
if [[ ! -x "$app" || ! -f "$archive" ]] ||
    ! command -v python3 >/dev/null 2>&1; then
    echo 'SKIP: Firestaff, original Amiga demo ZIP, or Python is unavailable'
    exit 77
fi
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/dm1-amiga-loose-case.XXXXXX")
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
: >"$scratch/Case"
if [[ -e "$scratch/case" ]]; then
    rm "$scratch/Case"
    echo 'SKIP: original-media filename ambiguity requires case-sensitive scratch storage'
    exit 77
fi
rm "$scratch/Case"
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS FIRESTAFF_EXIT_AFTER_LAUNCH
archive_hash_before=$(firestaff_sha256_file "$archive")
mkdir -p "$scratch/media" "$scratch/positive-home" "$scratch/ambiguous-home"
python3 - "$archive" "$scratch/media" <<'PY'
import hashlib
import pathlib
import sys
import zipfile

expected = {
    "graphics.dat": "491ca939f9abb33ceeb26619b841fe91",
    "DemoDun.dat": "78848e1a2d3d5a11e5954deb8c7b772b",
}
with zipfile.ZipFile(sys.argv[1]) as package:
    for name in ("graphics.dat", "DemoDun.dat", "swoosh"):
        payload = package.read(name)
        if name in expected and hashlib.md5(payload).hexdigest() != expected[name]:
            raise SystemExit(f"FAIL: unexpected authentic demo identity: {name}")
        target = pathlib.Path(sys.argv[2], "Swoosh" if name == "swoosh" else name)
        target.write_bytes(payload)
        if target.read_bytes() != payload:
            raise SystemExit(f"FAIL: loose staging changed original bytes: {name}")
PY
run_menu() {
    local route=$1
    HOME="$scratch/$route-home" XDG_CONFIG_HOME="$scratch/$route-home" \
    APPDATA="$scratch/$route-home" FIRESTAFF_CONFIG_PATH="$scratch/$route-home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --width 320 --height 200 --menu --game dm1 --platform amiga \
        --data-dir "$scratch/media" --debug --verbose \
        --script enter,enter,enter --duration 10000 >"$scratch/$route.log" 2>&1
}
if ! run_menu positive; then
    echo 'FAIL: mixed-case original Amiga startup companion did not launch' >&2
    exit 1
fi
for required in 'phase=amiga-swsh' 'phase=title-f0437' 'phase=entrance-f0441' \
    'DM1 READY: gameId=dm1' "graphicsPath=$scratch/media/graphics.dat"; do
    if ! grep -Fq "$required" "$scratch/positive.log"; then
        printf 'FAIL: mixed-case original startup lacks %s\n' "$required" >&2
        exit 1
    fi
done
# Both names contain the same original bytes. Filename ambiguity must still
# reject ownership rather than silently preferring one directory entry.
cp "$scratch/media/Swoosh" "$scratch/media/swoosh"
if run_menu ambiguous; then
    echo 'FAIL: ambiguous original startup companions were accepted' >&2
    exit 1
fi
# Complete-installation admission may reject the duplicate before native
# startup binds it; either boundary must leave the edition unavailable.
if ! grep -Fq 'platform=Amiga edition=amiga-demo-en not matched' "$scratch/ambiguous.log" &&
    ! { grep -Fq 'startup-sibling-rejected game=dm1 platform=amiga wanted=swoosh ' "$scratch/ambiguous.log" &&
        grep -Fq 'reason=ambiguous' "$scratch/ambiguous.log"; }; then
    echo 'FAIL: ambiguous original companion was not rejected by inventory or startup' >&2
    exit 1
fi
if grep -Eq 'startup-source-complete .*phase=(amiga-swsh|title-f0437|entrance-f0441)|launch phase=runtime-entered' \
    "$scratch/ambiguous.log"; then
    echo 'FAIL: ambiguous companions bound a native startup owner' >&2
    exit 1
fi
if [[ "$archive_hash_before" != "$(firestaff_sha256_file "$archive")" ]]; then
    echo 'FAIL: original Amiga demo ZIP changed during filename tests' >&2
    exit 1
fi
echo 'PASS: original loose Amiga companions support unique mixed case and reject case ambiguity'
