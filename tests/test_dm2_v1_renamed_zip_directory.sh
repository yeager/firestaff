#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"
app=${1:?usage: test_dm2_v1_renamed_zip_directory.sh <firestaff> <data-root>}
data_root=${2:?missing data root}
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
scratch=$(mktemp -d "$scratch_root/dm2-renamed-directory.XXXXXX")
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
        echo "FAIL: renamed-directory $label $route" >&2; exit 1
    fi
    grep -Fq "source=$alias_path::" "$log"
    # The selected spelling must survive discovery, not canonicalize to the
    # physical original or borrow a similarly named neighboring archive.
    for file in "${files[@]}"; do
        if grep -Fq "source=$data_root/dm2/$file::" "$log"; then
            echo 'FAIL: renamed directory lost its selected source owner' >&2; exit 1
        fi
    done
    if [[ "$route" = cli ]]; then
        grep -Fq "assetMd5=$hash" "$log"
        grep -Fq 'phase=dm2-runtime' "$log"
        grep -Fq 'dm2RealAssets=1' "$log"
        grep -Fq 'dm2NoCoreFallbacks=1' "$log"
        grep -Fq 'dm2FallbackDraws=0' "$log"
    else
        grep -Fq 'launch phase=game-handoff mode=menu game=dm2' "$log"
    fi
    echo "PASS: renamed directory $label $route retains authentic alias ownership"
}

for i in 0 1 2 3; do
    platform=${platforms[$i]}
    selected_root="$scratch/$platform/data"
    mkdir -p "$selected_root/dm2"
    alias_path="$selected_root/dm2/Arbitrary-$platform.ZIP"
    ln -s "$data_root/dm2/${files[$i]}" "$alias_path"
    for route in cli menu unscoped-menu; do
        run_route "$platform" "$platform" "$selected_root" "$alias_path" "${hashes[$i]}" "$route"
    done
done
selected_root="$scratch/mixed/data"
mkdir -p "$selected_root/dm2"
for i in 0 1 2 3; do
    ln -s "$data_root/dm2/${files[$i]}" "$selected_root/dm2/Arbitrary-${platforms[$i]}.ZIP"
done
# With four authenticated editions and no platform override, FM Towns owns
# AUTO launch from CLI, scoped M12, and the unscoped start menu.
for route in cli menu unscoped-menu; do
    run_route mixed fm-towns "$selected_root" "$selected_root/dm2/Arbitrary-fm-towns.ZIP" "${hashes[2]}" "$route"
done
for i in 0 1 2 3; do
    [[ "${before[$i]}" = "$(firestaff_sha256_file "$data_root/dm2/${files[$i]}")" ]]
done
unset FIRESTAFF_EXIT_AFTER_LAUNCH
echo 'PASS: original DM2 renamed ZIP directory ownership and FM Towns AUTO preference'
