#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"
app=${1:?usage: test_dm2_v1_loose_owner_real_media.sh <firestaff> <stage-probe> <data-root>}
stage_probe=${2:?missing native staging probe}
data_root=${3:?missing data root}
[[ -x "$app" ]] || exit 77
[[ -d "$data_root" ]] || { echo "SKIP: original game-data root is unavailable"; exit 77; }
data_root=$(cd "$data_root" && pwd)
platforms=(pc mac fm-towns amiga)
files=('Dungeon-Master-II-Skullkeep_DOS_EN.zip' 'Dungeon-Master-II-Skullkeep_Mac_EN (1).zip' 'Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip' 'Dungeon-Master-II-Skullkeep_Amiga_EN.zip')
hashes=(25247ede4dabb6a71e5dabdfbcd5907d 5cab25f6b975957eae4a203174e7f2a6 027ff3b8ddc2c4c4cdda7ada0b0bc46c 1c940ea95703eaea0ecdf84d17e954b9)
for file in "${files[@]}"; do
    [[ -f "$data_root/dm2/$file" ]] || { echo "SKIP: missing original $file"; exit 77; }
done
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/dm2-loose-owner.XXXXXX")
cleanup() {
    rc=$?
    if [[ $rc != 0 ]]; then
        for log in "$scratch"/*.log; do [[ ! -f "$log" ]] || cat "$log" >&2; done
    fi
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else rm -rf "$scratch"; fi
}
trap cleanup EXIT
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS FIRESTAFF_EXIT_AFTER_LAUNCH FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR
before=()
for i in 0 1 2 3; do before[$i]=$(firestaff_sha256_file "$data_root/dm2/${files[$i]}"); done

run_route() {
    local label=$1 platform=$2 selected_root=$3 alias_path=$4 hash=$5 route=$6
    local frames inputs menu_inputs
    case "$platform" in
        pc) frames=100; inputs=key:enter,key:enter,key:enter ;;
        mac) frames=500; inputs=key:enter,key:enter,click:100:60,up ;;
        fm-towns) frames=0; inputs=wait:8000,click:115:65,click:100:60 ;;
        amiga) frames=2000; inputs=key:enter,key:enter,key:enter ;;
    esac
    local home="$scratch/$label-$route-home" log="$scratch/$label-$route.log"
    mkdir -p "$home"
    local args=(--data-dir "$selected_root" --width 320 --height 200 --debug)
    if [[ "$route" != unscoped-menu ]]; then args+=(--game dm2); fi
    if [[ "$label" != mixed && "$route" != unscoped-menu ]]; then args+=(--platform "$platform"); fi
    if [[ "$route" = cli ]]; then
        unset FIRESTAFF_EXIT_AFTER_LAUNCH
        args+=(--boot-probe --boot-probe-frames "$frames" --script "$inputs" --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 --duration 0)
    else
        export FIRESTAFF_EXIT_AFTER_LAUNCH=1
        menu_inputs=key:enter,key:enter,key:enter
        if [[ "$route" = unscoped-menu ]]; then menu_inputs=key:down,key:down,$menu_inputs; fi
        args+=(--menu --script "$menu_inputs" --duration 10000)
    fi
    if ! HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
        FIRESTAFF_CONFIG_PATH="$home/config.toml" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "${args[@]}" >"$log" 2>&1; then
        echo "FAIL: loose-owner $label $route" >&2; exit 1
    fi
    grep -Fq "source=$alias_path" "$log"
    grep -Fq "inventory-file=dungeon matched source=$(dirname "$alias_path")/Level-layout.dat" "$log"
    if [[ "$route" = cli ]]; then
        grep -Fq "assetMd5=$hash" "$log"
        grep -Fq 'phase=dm2-runtime' "$log"
        grep -Fq 'dm2RealAssets=1' "$log"
        grep -Fq 'dm2NoCoreFallbacks=1' "$log"
        grep -Fq 'dm2FallbackDraws=0' "$log"
    else
        grep -Fq 'launch phase=game-handoff mode=menu game=dm2' "$log"
    fi
    echo "PASS: loose owner $label $route retains authentic alias ownership"
}

for i in 0 1 2 3; do
    platform=${platforms[$i]}
    selected_root="$scratch/$platform/data"
    mkdir -p "$selected_root"
    "$stage_probe" "$data_root/dm2/${files[$i]}" "$selected_root" "$i"
    alias_path="$selected_root/Maze-art.dat"
    for selected in "$selected_root" "$alias_path"; do
        if [[ "$selected" = "$selected_root" ]]; then suffix=directory; else suffix=file; fi
        for route in cli menu unscoped-menu; do
            run_route "$platform-$suffix" "$platform" "$selected" "$alias_path" "${hashes[$i]}" "$route"
        done
    done
    # A genuine dungeon in a child directory is a different install owner.
    # Moving unchanged original bytes must reject, even when generic recursive
    # discovery can still find both hashes below the selected root.
    mkdir -p "$selected_root/neighbour"
    mv "$selected_root/Level-layout.dat" "$selected_root/neighbour/Level-layout.dat"
    for selected in "$selected_root" "$alias_path"; do
        home="$scratch/$platform-negative-$(basename "$selected")-home"
        mkdir -p "$home"
        log="$scratch/$platform-negative-$(basename "$selected").log"
        unset FIRESTAFF_EXIT_AFTER_LAUNCH
        if HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
            FIRESTAFF_CONFIG_PATH="$home/config.toml" \
            SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
            "$app" --game dm2 --platform "$platform" --data-dir "$selected" \
            --boot-probe --boot-probe-frames 0 --duration 0 --debug >"$log" 2>&1; then
            echo "FAIL: $platform admitted a dungeon from a child install" >&2; exit 1
        fi
        if grep -Fq 'phase=dm2-runtime' "$log"; then
            echo "FAIL: $platform mixed loose owners" >&2; exit 1
        fi
    done
    cp -p "$selected_root/neighbour/Level-layout.dat" "$selected_root/Level-layout.dat"
    # The child directory was created before this restored parent payload.
    # Generic recursive inventory can prime the child's hash first; a depth0
    # companion lookup must bypass that cache hit and find the actual sibling.
    run_route "$platform-cache-recovery" "$platform" "$selected_root" \
        "$alias_path" "${hashes[$i]}" cli
done
# A mixed collection still defaults to Towns, while an explicitly selected
# graphics payload retains its own edition despite the neighboring installs.
mkdir -p "$scratch/mixed/data"
for i in 0 1 2 3; do
    platform=${platforms[$i]}
    mkdir -p "$scratch/mixed/data/$platform"
    cp -p "$scratch/$platform/data/Maze-art.dat" "$scratch/mixed/data/$platform/Maze-art.dat"
    cp -p "$scratch/$platform/data/Level-layout.dat" "$scratch/mixed/data/$platform/Level-layout.dat"
done
for route in cli menu unscoped-menu; do
    run_route mixed fm-towns "$scratch/mixed/data" \
        "$scratch/mixed/data/fm-towns/Maze-art.dat" "${hashes[2]}" "$route"
done
for i in 1 3; do
    platform=${platforms[$i]}
    alias_path="$scratch/mixed/data/$platform/Maze-art.dat"
    for route in cli menu unscoped-menu; do
        run_route "$platform-selected-mixed" "$platform" "$alias_path" \
            "$alias_path" "${hashes[$i]}" "$route"
    done
done
for i in 0 1 2 3; do
    [[ "${before[$i]}" = "$(firestaff_sha256_file "$data_root/dm2/${files[$i]}")" ]]
done
unset FIRESTAFF_EXIT_AFTER_LAUNCH
echo 'PASS: original DM2 loose pair directory/file ownership across four platforms'
