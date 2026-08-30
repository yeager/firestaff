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

# Every direct/menu GAME_LOAD below reads archive::SKSAVE into process memory.
# Preserve the outer original hash across the complete primary/backup matrix
# so a future resume path cannot silently write, unpack, or replace media.
archive_hash_before=$(sha256sum "$archive")

# Original slots and backups are intentionally distinct evidence.  The values
# below are observed source positions from the mounted retail archive, not a
# generated save fixture.  Each route gives archive::member to the source
# GAME_LOAD owner directly and through the normal start menu.
while IFS='|' read -r member map party; do
    [ -n "$member" ] || continue
    save_path="$archive::$member"
    for launch_route in direct menu; do
        if [ "$launch_route" = menu ]; then
            menu_arg=--menu
        else
            menu_arg=
        fi
        output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
            $menu_arg --game dm2 --platform pc --data-dir "$archive" --save "$save_path" \
            --boot-probe --boot-probe-frames 5000 \
            --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
            --boot-probe-expect-map "$map" --boot-probe-expect-party "$party" \
            --duration 0 2>&1) || { printf '%s\n' "$output" >&2; exit 1; }
        case "$output" in
        # A resume receipt proves source GAME_LOAD restored this exact original
        # slot.  Do not require a frame-acceptance bit here: some authentic save
        # poses need a later source-owned viewport transaction before their first
        # scene frame can be admitted, and treating that presentation boundary as
        # a failed load would erase valid read-only archive-save coverage.
        *'assetMd5=25247ede4dabb6a71e5dabdfbcd5907d'*'phase=dm2-runtime'*"map=$map"*"party=$party"*'dm2RealAssets=1'*'dm2NoCoreFallbacks=1'*'dm2FallbackDraws=0'*'startedFromLauncher=1'*) ;;
            *) printf '%s\n' "$output" >&2; exit 1 ;;
        esac
    done
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
