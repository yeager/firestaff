#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"
app=${1:?usage: test_dm1_v1_amiga_loose_graphics_alias_real_media.sh <firestaff> [original-demo.zip]}
archive=${2:-${FIRESTAFF_DM1_AMIGA_DEMO_ARCHIVE:-"$HOME/.firestaff/data/dm1/dm-amiga-demo-en.zip"}}
if [[ ! -x "$app" || ! -f "$archive" ]] || ! command -v python3 >/dev/null 2>&1; then
    echo 'SKIP: Firestaff, original Amiga demo ZIP, or Python is unavailable'; exit 77
fi
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/dm1-amiga-graphics-alias.XXXXXX")
cleanup() {
    rc=$?
    if [[ $rc != 0 ]]; then
        for log in "$scratch"/*.log; do [[ ! -f "$log" ]] || cat "$log" >&2; done
    fi
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then echo "TEST SCRATCH PRESERVED: $scratch"; else rm -rf "$scratch"; fi
}
trap cleanup EXIT
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS FIRESTAFF_EXIT_AFTER_LAUNCH
# Explicit selections must work without borrowing the developer's data roots.
unset FIRESTAFF_DATA FIRESTAFF_DATA_DIR FIRESTAFF_DATA_ROOT FIRESTAFF_ORIGINALS FIRESTAFF_ORIGINAL_DATA_DIR
archive_hash_before=$(firestaff_sha256_file "$archive")
mkdir -p "$scratch/media"
python3 - "$archive" "$scratch/media" <<'PY'
import hashlib,pathlib,sys,zipfile
files={
 'graphics.dat':('Maze-art.dat',192421,'491ca939f9abb33ceeb26619b841fe91'),
 'DemoDun.dat':('DemoDun.dat',1209,'78848e1a2d3d5a11e5954deb8c7b772b'),
 'swoosh':('swoosh',78068,'28d406007e99b0ae8da0c6f7fc7f183b')}
with zipfile.ZipFile(sys.argv[1]) as package:
 for source,(name,size,md5) in files.items():
  data=package.read(source)
  if len(data)!=size or hashlib.md5(data).hexdigest()!=md5:
   raise SystemExit('FAIL: original demo identity changed: '+source)
  target=pathlib.Path(sys.argv[2],name)
  with target.open('xb') as out:out.write(data)
  if target.read_bytes()!=data:raise SystemExit('FAIL: original staging altered bytes')
PY
for selection in directory file; do
    selected="$scratch/media"
    if [[ "$selection" = file ]]; then selected="$scratch/media/Maze-art.dat"; fi
    for route in cli menu; do
        home="$scratch/$selection-$route-home"
        mkdir -p "$home"
        log="$scratch/$selection-$route.log"
        args=(--game dm1 --platform amiga --data-dir "$selected" --width 320 --height 200 --debug --verbose)
        if [[ "$route" = cli ]]; then
            unset FIRESTAFF_EXIT_AFTER_LAUNCH
            args+=(--boot-probe --boot-probe-frames 120 --duration 0)
        else
            export FIRESTAFF_EXIT_AFTER_LAUNCH=1
            args+=(--menu --script enter,enter,enter --duration 10000)
        fi
        if ! HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
            FIRESTAFF_CONFIG_PATH="$home/config.toml" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
            SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "${args[@]}" >"$log" 2>&1; then
            echo "FAIL: original loose graphics alias $selection $route" >&2; exit 1
        fi
        grep -Fq 'edition=amiga-demo-en' "$log"
        if [[ "$route" = cli ]]; then
            grep -Fq "inventory-file=graphics matched source=$scratch/media/Maze-art.dat" "$log"
            grep -Fq 'assetMd5=491ca939f9abb33ceeb26619b841fe91' "$log"
            grep -Fq 'phase=dm1-runtime' "$log"
            grep -Fq 'levelLoaded=1' "$log"
        else
            for phase in amiga-swsh title-f0437 entrance-f0441; do
                grep -Eq "startup-source-complete .*phase=$phase([[:space:]]|$)" "$log"
            done
            grep -Fq 'DM1 READY: gameId=dm1' "$log"
            grep -Fq "graphicsPath=$scratch/media/Maze-art.dat" "$log"
        fi
        echo "PASS: original loose graphics alias $selection $route retains Maze-art.dat ownership"
    done
done
[[ "$archive_hash_before" = "$(firestaff_sha256_file "$archive")" ]]
python3 - "$archive" "$scratch/media" <<'PY'
import pathlib,sys,zipfile
with zipfile.ZipFile(sys.argv[1]) as package:
 for source,target in [('graphics.dat','Maze-art.dat'),('DemoDun.dat','DemoDun.dat'),('swoosh','swoosh')]:
  if package.read(source)!=pathlib.Path(sys.argv[2],target).read_bytes():
   raise SystemExit('FAIL: staged original bytes changed during launch: '+target)
PY
unset FIRESTAFF_EXIT_AFTER_LAUNCH
echo 'PASS: original Amiga demo graphics alias CLI/M12 ownership matrix'
