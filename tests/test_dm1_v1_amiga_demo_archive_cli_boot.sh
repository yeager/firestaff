#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_dm1_v1_amiga_demo_archive_cli_boot.sh <firestaff>}
archive=${FIRESTAFF_DM1_AMIGA_DEMO_ARCHIVE:-"$HOME/.firestaff/data/dm1/dm-amiga-demo-en.zip"}
expected_graphics_md5=491ca939f9abb33ceeb26619b841fe91
expected_dungeon_md5=78848e1a2d3d5a11e5954deb8c7b772b

unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic DM1 Amiga demo archive is not staged'
    exit 77
fi

archive_hash_before=$(firestaff_sha256_file "$archive")
python3 - "$archive" "$expected_graphics_md5" "$expected_dungeon_md5" <<'PY'
import hashlib
import sys
import zipfile

archive, expected_graphics, expected_dungeon = sys.argv[1:]
with zipfile.ZipFile(archive) as package:
    graphics = hashlib.md5(package.read("graphics.dat")).hexdigest()
    dungeon = hashlib.md5(package.read("DemoDun.dat")).hexdigest()
if (graphics, dungeon) != (expected_graphics, expected_dungeon):
    raise SystemExit(
        f"FAIL: demo package has unexpected original media hashes: "
        f"graphics={graphics} dungeon={dungeon}")
PY

direct_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform amiga --data-dir "$archive" --debug \
    --boot-probe --boot-probe-frames 120 --duration 0 2>&1) || {
    printf '%s\n' "$direct_output" >&2
    exit 1
}
grep -Fq "edition=amiga-demo-en matched source=$archive::graphics.dat" <<<"$direct_output"
grep -Fq "inventory-file=graphics matched source=$archive::graphics.dat" <<<"$direct_output"
grep -Fq "inventory-file=dungeon matched source=$archive::DemoDun.dat" <<<"$direct_output"
grep -Fq "FIRESTAFF BOOT PROBE READY: gameId=dm1 sourceKind=0 sourceId=dm1 assetMd5=$expected_graphics_md5" <<<"$direct_output"
grep -Fq 'phase=dm1-runtime' <<<"$direct_output"
grep -Fq 'levelLoaded=1' <<<"$direct_output"

menu_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform amiga --data-dir "$archive" --debug --verbose \
    --script enter,enter,enter --duration 1000 2>&1) || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
grep -Fq 'edition=amiga-demo-en' <<<"$menu_output"
grep -Fq "DM1 READY: gameId=dm1 dataDir=$archive" <<<"$menu_output"
grep -Fq "graphicsPath=$archive::graphics.dat" <<<"$menu_output"

archive_hash_after=$(firestaff_sha256_file "$archive")
if [[ "$archive_hash_before" != "$archive_hash_after" ]]; then
    printf '%s\n' 'FAIL: Firestaff modified the original DM1 Amiga demo archive' >&2
    exit 1
fi

printf 'PASS: authentic DM1 Amiga demo starts from CLI and M12 menu; graphics=%s dungeon=%s archive=read-only\n' \
    "$expected_graphics_md5" "$expected_dungeon_md5"
