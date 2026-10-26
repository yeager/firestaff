#!/usr/bin/env bash
set -euo pipefail
app=${1:?Firestaff executable required}
original=${2:-}
if [[ ! -x "$app" || ! -d "$original" ]] || ! command -v python3 >/dev/null; then
    echo 'SKIP: executable or complete original CSB Amiga loose media unavailable'
    exit 77
fi
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/csb-amiga-loose-case.XXXXXX")
trap 'rc=$?; if ((rc)); then cat "$scratch"/*.log >&2; fi; if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then echo "TEST SCRATCH PRESERVED: $scratch"; else rm -rf "$scratch"; fi' EXIT
: > "$scratch/Case"
if [[ -e "$scratch/case" ]]; then
    echo 'SKIP: filename ambiguity needs case-sensitive scratch storage'
    exit 77
fi
python3 - "$original" "$scratch/media" <<'PY'
import hashlib, pathlib, shutil, sys
source, target = map(pathlib.Path, sys.argv[1:])
expected = {'graphics.dat':'61fbfd56887c94adc26888a9491c6611',
            'dungeon.dat':'6695d2acebce49f95db1d8f3a5c733de',
            'titl.dat':'5b590ea3a6f5eed513b5678b01468ee4',
            'appb.ftl':'35987d3f0278c6036fcc24786d4a75d7',
            'swsh.ftl':'ff3872baaed8ee4e83ee3c0684b2eeec'}
for name, digest in expected.items():
    matches = [p for p in source.iterdir() if p.is_file() and p.name.lower()==name]
    if len(matches)!=1 or hashlib.md5(matches[0].read_bytes()).hexdigest()!=digest:
        raise SystemExit(f'FAIL: original identity mismatch: {name}')
shutil.copytree(source, target)
for name in ('titl.dat','appb.ftl'):
    p = next(p for p in target.iterdir() if p.name.lower()==name)
    p.rename(target / name.upper())
PY
unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR FIRESTAFF_EXIT_AFTER_LAUNCH
run_cli() {
    local label=$1
    mkdir -p "$scratch/$label-home"
    HOME="$scratch/$label-home" FIRESTAFF_CONFIG_PATH="$scratch/$label-home/config.toml" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --width 320 --height 200 --game csb --platform amiga \
        --data-dir "$scratch/media" --debug --boot-probe --boot-probe-frames 800 \
        --script 'wait700,click:100:100,key:enter,up' --boot-probe-expect-runtime \
        --boot-probe-expect-startup-active 0 --boot-probe-expect-level-loaded 1 \
        --boot-probe-expect-runtime-tick-min 1 > "$scratch/$label.log" 2>&1
}
run_cli baseline
grep -Fq 'game=csb phase=amiga-swsh-ftl' "$scratch/baseline.log"
grep -Fq 'source-vblanks=30' "$scratch/baseline.log"
mv "$scratch/media/SWSH.FTL" "$scratch/media/SWSH.FTL.disabled"
run_cli missing-swsh
grep -Fq 'phase=amiga-swsh-ftl reason=missing-or-invalid-source' "$scratch/missing-swsh.log"
mv "$scratch/media/SWSH.FTL.disabled" "$scratch/media/SWSH.FTL"
mv "$scratch/media/TITL.DAT" "$scratch/media/Titl.dat"
mv "$scratch/media/APPB.FTL" "$scratch/media/Appb.ftl"
run_cli mixed
mkdir -p "$scratch/menu-home"
HOME="$scratch/menu-home" FIRESTAFF_CONFIG_PATH="$scratch/menu-home/config.toml" \
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$scratch/menu.json" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --menu --game csb --platform amiga \
    --data-dir "$scratch/media" --width 320 --height 200 \
    --script 'enter,enter,enter,wait:1000,click:100:100,key:enter,up' --duration 30000 \
    > "$scratch/menu.log" 2>&1
python3 - "$scratch/menu.json" <<'PY'
import json, sys
p=json.load(open(sys.argv[1]))
s=p['startup']
assert p['sourceId']=='csb' and p['bootAssetMd5']=='61fbfd56887c94adc26888a9491c6611', p
assert p['launchedEver']==1 and s['levelLoaded']==1 and s['startupActive']==0, p
assert p.get('csbViewportHash',0)!=0, p
PY
grep -Fq 'variant=csb-amiga-a31m route=startup handoff=a31m-titl-dat' "$scratch/menu.log"
for name in TITL.DAT APPB.FTL; do
    case "$name" in TITL.DAT) mixed=Titl.dat;; *) mixed=Appb.ftl;; esac
    cp "$scratch/media/$mixed" "$scratch/media/$name"
    if run_cli "ambiguous-$name"; then
        echo "FAIL: ambiguous $name accepted" >&2
        exit 1
    fi
    rm "$scratch/media/$name"
done
python3 - "$original" "$scratch/media" <<'PY'
import hashlib, pathlib, sys
expected={'graphics.dat':'61fbfd56887c94adc26888a9491c6611',
          'dungeon.dat':'6695d2acebce49f95db1d8f3a5c733de',
          'titl.dat':'5b590ea3a6f5eed513b5678b01468ee4',
          'appb.ftl':'35987d3f0278c6036fcc24786d4a75d7',
          'swsh.ftl':'ff3872baaed8ee4e83ee3c0684b2eeec'}
for directory in map(pathlib.Path,sys.argv[1:]):
    for name,digest in expected.items():
        matches=[p for p in directory.iterdir() if p.is_file() and p.name.lower()==name]
        assert len(matches)==1 and hashlib.md5(matches[0].read_bytes()).hexdigest()==digest, (directory,name)
PY
echo 'PASS: authentic CSB Amiga FTL prelude, missing-SWSH fallback, mixed-case CLI/menu runtime; ambiguous companions rejected'
