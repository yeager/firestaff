#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"
app=${1:?usage: test_dm2_v1_explicit_zip_ownership.sh <firestaff> <data-root>}
data_root=${2:?missing data root}
[[ -x "$app" ]] || exit 77
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/dm2-zip-owner.XXXXXX")
trap 'rc=$?; if [[ $rc != 0 && -f $scratch/output.log ]]; then cat "$scratch/output.log" >&2; fi; rm -rf "$scratch"' EXIT
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS
platforms=(pc mac fm-towns amiga)
files=('Dungeon-Master-II-Skullkeep_DOS_EN.zip' 'Dungeon-Master-II-Skullkeep_Mac_EN (1).zip' 'Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip' 'Dungeon-Master-II-Skullkeep_Amiga_EN.zip')
hashes=(25247ede4dabb6a71e5dabdfbcd5907d 5cab25f6b975957eae4a203174e7f2a6 027ff3b8ddc2c4c4cdda7ada0b0bc46c 1c940ea95703eaea0ecdf84d17e954b9)
for file in "${files[@]}"; do
    [[ -f "$data_root/dm2/$file" ]] || { echo "SKIP: missing original $file"; exit 77; }
done
for i in 0 1 2 3; do
    platform=${platforms[$i]}
    original="$data_root/dm2/${files[$i]}"
    before=$(firestaff_sha256_file "$original")
    alias_path="$scratch/Selected-$platform.ZIP"
    ln -s "$original" "$alias_path"
    case "$platform" in
        pc) frames=100; inputs=key:enter,key:enter,key:enter ;;
        mac) frames=500; inputs=key:enter,key:enter,click:100:60,up ;;
        fm-towns) frames=0; inputs=wait:8000,click:115:65,click:100:60 ;;
        amiga) frames=2000; inputs=key:enter,key:enter,key:enter ;;
    esac
    for source in "$original" "$alias_path"; do
        for route in cli menu unscoped-menu; do
            config="$scratch/$i-$route.toml"
            args=(--data-dir "$source" --width 320 --height 200 --debug)
            if [[ "$route" != unscoped-menu ]]; then args+=(--game dm2 --platform "$platform"); fi
            if [[ "$route" = cli ]]; then
                args+=(--boot-probe --boot-probe-frames "$frames" --script "$inputs" --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 --duration 0)
            else
                menu_inputs=key:enter,key:enter,key:enter
                if [[ "$route" = unscoped-menu ]]; then menu_inputs=key:down,key:down,$menu_inputs; fi
                args+=(--menu --script "$menu_inputs" --duration 1000)
            fi
            log="$scratch/output.log"
            # Only the menu case exits at M12's successful runtime handoff.
            if [[ "$route" != cli ]]; then export FIRESTAFF_EXIT_AFTER_LAUNCH=1; else unset FIRESTAFF_EXIT_AFTER_LAUNCH; fi
            if ! FIRESTAFF_CONFIG_PATH="$config" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "${args[@]}" >"$log" 2>&1; then cat "$log" >&2; exit 1; fi
            grep -Fq "source=$source::" "$log"
            if [[ "$route" = cli ]]; then
                grep -Fq "assetMd5=${hashes[$i]}" "$log"
                grep -Fq 'phase=dm2-runtime' "$log"
                grep -Fq 'dm2FallbackDraws=0' "$log"
            fi
        done
    done
    [[ "$before" = "$(firestaff_sha256_file "$original")" ]]
    echo "PASS: original $platform ZIP and renamed alias retain CLI/runtime and M12 ownership"
done
unset FIRESTAFF_EXIT_AFTER_LAUNCH
# An unreadable selection must not borrow this authentic neighboring Towns disc.
ln -s "$data_root/dm2/${files[2]}" "$scratch/${files[2]}"
for rejected in "$data_root/dm1/Dungeon-Master_DOS_EN_Version-34.zip" "$data_root/dm2/Dungeon-Master-II-Skullkeep_PC-9821_JA.zip"; do
    [[ -f "$rejected" ]] || continue
    ln -sf "$rejected" "$scratch/Rejected.ZIP"
    if FIRESTAFF_CONFIG_PATH="$scratch/rejected.toml" SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --game dm2 --data-dir "$scratch/Rejected.ZIP" --debug --boot-probe --boot-probe-frames 1 --duration 0 >"$scratch/rejected.log" 2>&1; then cat "$scratch/rejected.log" >&2; exit 1; fi
    if grep -Fq 'launch phase=runtime-entered' "$scratch/rejected.log"; then cat "$scratch/rejected.log" >&2; exit 1; fi
    if FIRESTAFF_CONFIG_PATH="$scratch/rejected-menu.toml" \
        FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --menu --data-dir "$scratch/Rejected.ZIP" --debug \
        --script key:down,key:down,key:enter,key:enter,key:enter --duration 1000 \
        >"$scratch/rejected-menu.log" 2>&1; then
        cat "$scratch/rejected-menu.log" >&2; exit 1
    fi
    echo "PASS: unrelated/unsupported original ZIP rejected beside authentic Towns media"
done
echo 'PASS: explicit DM2 ZIP ownership matrix'
