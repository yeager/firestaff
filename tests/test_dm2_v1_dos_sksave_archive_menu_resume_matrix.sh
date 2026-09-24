#!/usr/bin/env sh
set -eu

app=${1:?usage: test_dm2_v1_dos_sksave_archive_menu_resume_matrix.sh <firestaff>}
archive=${FIRESTAFF_DM2_DOS_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_DOS_EN.zip"}

# This is a production-native archive route.  A developer's optional archive
# diagnostic must not turn the all-slot regression into an extractor wrapper.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [ ! -x "$app" ] || [ ! -f "$archive" ]; then
    echo 'SKIP: authentic DM2 DOS archive is not staged'
    exit 77
fi

# Every direct and M12 Quick Resume GAME_LOAD below reads archive::SKSAVE into
# process memory. Preserve the outer original hash across the complete
# primary/backup matrix so a resume path cannot write or unpack game media.
archive_hash_before=$(sha256sum "$archive")
case "$app" in
    */*) app_dir=${app%/*} ;;
    *) app_dir=. ;;
esac

# Original slots and backups are intentionally distinct evidence.  The values
# below are observed source positions from the mounted retail archive, not a
# generated save fixture.  Each route gives archive::member to the source
# GAME_LOAD owner directly and through the normal start menu.
while IFS='|' read -r member map party; do
    [ -n "$member" ] || continue
    save_path="$archive::$member"
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
            --game dm2 --platform pc --data-dir "$archive" --save "$save_path" \
            --boot-probe --boot-probe-frames 5000 \
            --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
            --boot-probe-expect-map "$map" --boot-probe-expect-party "$party" \
            --duration 0 2>&1) || { printf '%s\n' "$output" >&2; exit 1; }
    case "$output" in
    # A direct resume receipt proves source GAME_LOAD restored this exact
    # original slot. Some save poses need a later viewport transaction before
    # their first scene frame; this state check does not claim frame parity.
    *'assetMd5=25247ede4dabb6a71e5dabdfbcd5907d'*'phase=dm2-runtime'*"map=$map"*"party=$party"*'dm2RealAssets=1'*'dm2NoCoreFallbacks=1'*'dm2FallbackDraws=0'*) ;;
        *) printf '%s\n' "$output" >&2; exit 1 ;;
    esac

    # Verify each original primary/backup through ordinary M12 Quick Resume.
    probe_file="$app_dir/dm2-sksave-menu-${member##*/}-$$.json"
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe_file" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --menu --game dm2 --platform pc --data-dir "$archive" --save "$save_path" \
        --script enter --duration 3000 >/dev/null 2>&1
    python3 - "$probe_file" "$map" "$party" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
expected_map, expected_pose = int(sys.argv[2]), tuple(
    int(value) for value in sys.argv[3].split(","))
actual_pose = (party["mapX"], party["mapY"], party["direction"])
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm2" or startup["receiptReady"] != 1 or
        startup["startupActive"] != 0 or startup["levelLoaded"] != 1 or
        startup["phase"] != "dm2-runtime" or
        party["mapIndex"] != expected_map or actual_pose != expected_pose):
    raise SystemExit(f"FAIL: DM2 M12 Quick Resume state mismatch: {probe}")
PY
    rm -f "$probe_file"
done <<'EOF'
data/sksave0.dat|11|15,2,3
data/sksave0.bak|11|15,3,0
data/sksave1.dat|11|15,10,2
data/sksave1.bak|11|15,10,2
data/sksave2.dat|24|4,3,1
data/sksave2.bak|8|13,10,1
data/sksave3.dat|8|8,21,0
data/sksave3.bak|8|8,21,0
EOF

if [ "$archive_hash_before" != "$(sha256sum "$archive")" ]; then
    echo 'FAIL: DM2 DOS archive changed during native SKSAVE resume matrix' >&2
    exit 1
fi

echo 'PASS: native DM2 DOS ZIP CLI and start menu resume every archive::SKSAVE slot in memory'
