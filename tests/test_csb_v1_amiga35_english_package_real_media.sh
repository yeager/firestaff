#!/usr/bin/env bash
set -euo pipefail
app=${1:?Firestaff required}
original=${2:-}
if [[ ! -x "$app" || ! -d "$original" ]] || ! command -v python3 >/dev/null; then
    echo 'SKIP: complete original A35E loose media unavailable'; exit 77
fi
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/csb-a35e-package.XXXXXX")
trap 'rc=$?; if ((rc)); then cat "$scratch"/*.log >&2; fi; if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then echo "TEST SCRATCH PRESERVED: $scratch"; else rm -rf "$scratch"; fi' EXIT
: > "$scratch/Case"
if [[ -e "$scratch/case" ]]; then echo 'SKIP: ambiguity needs case-sensitive scratch'; exit 77; fi
python3 - "$original" "$scratch/media" <<'PY'
import hashlib,pathlib,shutil,sys
source,target=map(pathlib.Path,sys.argv[1:])
expected={'graphics.dat':'291e1bc6803e3dc4b974c60117ca5d68',
          'dungeon.dat':'6695d2acebce49f95db1d8f3a5c733de',
          'appb.ftl':'11d8d059cd8f241d6f68ec09c5c8b66d',
          'bjeload_r':'6aec320606f014a149548e664719cf5b'}
for name,digest in expected.items():
    files=[p for p in source.iterdir() if p.is_file() and p.name.lower()==name]
    assert len(files)==1 and hashlib.md5(files[0].read_bytes()).hexdigest()==digest,name
shutil.copytree(source,target)
for name in ('appb.ftl','bjeload_r'):
    p=next(p for p in target.iterdir() if p.name.lower()==name)
    p.rename(target / ('Appb.ftl' if name=='appb.ftl' else 'Bjeload_R'))
PY
mkdir -p "$scratch/home" "$scratch/neighbor"
export HOME="$scratch/home" FIRESTAFF_CONFIG_PATH="$scratch/home/config.toml"
export SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy
unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR FIRESTAFF_EXIT_AFTER_LAUNCH
"$app" --width 320 --height 200 --game csb --platform amiga --data-dir "$scratch/media" \
    --boot-probe --boot-probe-frames 800 --script 'enter,up' --duration 0 \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
    --boot-probe-expect-runtime-tick-min 1 > "$scratch/positive-cli.log" 2>&1
grep -Fq 'assetMd5=291e1bc6803e3dc4b974c60117ca5d68' "$scratch/positive-cli.log"
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$scratch/menu.json" \
"$app" --width 320 --height 200 --menu --game csb --platform amiga --data-dir "$scratch/media" \
    --script 'enter,enter,enter,wait:1200,enter,wait:100,up' --duration 35000 \
    > "$scratch/positive-menu.log" 2>&1
python3 - "$scratch/menu.json" <<'PY'
import json,sys
p=json.load(open(sys.argv[1]));s=p['startup']
assert p['sourceId']=='csb' and p['bootAssetMd5']=='291e1bc6803e3dc4b974c60117ca5d68',p
assert p['launchedEver']==1 and s['levelLoaded']==1 and s['startupActive']==0,p
assert p['csbViewportHash']!=0,p
PY
reject() {
    local label=$1
    "$app" --scan-data --game csb --platform amiga --data-dir "$scratch/media" > "$scratch/$label-scan.log" 2>&1
    if grep -Eq 'Chaos Strikes Back +READY|Amiga 3.5 English +FOUND' "$scratch/$label-scan.log"; then
        echo "FAIL: incomplete $label remains selectable" >&2; exit 1
    fi
    if "$app" --game csb --platform amiga --data-dir "$scratch/media" --boot-probe --duration 0 \
        > "$scratch/$label-cli.log" 2>&1; then
        echo "FAIL: incomplete $label CLI accepted" >&2; exit 1
    fi
    if grep -Eq 'CSB READY:|FIRESTAFF BOOT PROBE READY:' "$scratch/$label-cli.log"; then
        echo "FAIL: incomplete $label published launch receipt" >&2; exit 1
    fi
}
# Keep authentic APPB in a neighboring directory: it must not satisfy this owner.
mv "$scratch/media/Appb.ftl" "$scratch/neighbor/Appb.ftl"
reject missing-appb
cp "$scratch/neighbor/Appb.ftl" "$scratch/media/Appb.ftl"
cp "$scratch/media/Appb.ftl" "$scratch/media/APPB.FTL"
reject ambiguous-appb
rm "$scratch/media/APPB.FTL"
# Wrong authentic edition, not synthetic executable bytes.
cp "$scratch/media/Bjeload_R" "$scratch/media/Appb.ftl"
reject wrong-hash-appb
cp "$scratch/neighbor/Appb.ftl" "$scratch/media/Appb.ftl"
mv "$scratch/media/Bjeload_R" "$scratch/neighbor/Bjeload_R"
reject missing-loader
cp "$scratch/neighbor/Bjeload_R" "$scratch/media/Bjeload_R"
python3 - "$original" "$scratch/media" <<'PY'
import hashlib,pathlib,sys
expected={'graphics.dat':'291e1bc6803e3dc4b974c60117ca5d68','dungeon.dat':'6695d2acebce49f95db1d8f3a5c733de',
          'appb.ftl':'11d8d059cd8f241d6f68ec09c5c8b66d','bjeload_r':'6aec320606f014a149548e664719cf5b'}
for directory in map(pathlib.Path,sys.argv[1:]):
    for name,digest in expected.items():
        files=[p for p in directory.iterdir() if p.is_file() and p.name.lower()==name]
        assert len(files)==1 and hashlib.md5(files[0].read_bytes()).hexdigest()==digest,(directory,name)
PY
echo 'PASS: original A35E CLI/M12 runtime and required same-owner program admission'
