#!/usr/bin/env bash
set -euo pipefail
app=${1:?Firestaff required}
multi=${2:-}
adf=${3:-}
if [[ ! -x "$app" || ! -d "$multi" || ! -f "$adf" ]] || ! command -v python3 >/dev/null; then
    echo 'SKIP: original CSB A33M files and ADF are required'; exit 77
fi
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/csb-amiga-title-candidates.XXXXXX")
trap 'rc=$?; if ((rc)); then cat "$scratch"/*.log >&2; fi; if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then echo "TEST SCRATCH PRESERVED: $scratch"; else rm -rf "$scratch"; fi' EXIT
archive="$scratch/title-candidate-cap.zip"
python3 - "$multi" "$adf" "$archive" <<'PY'
import hashlib,pathlib,sys,zipfile
multi,adf,out=map(pathlib.Path,sys.argv[1:])
title=next(p for p in multi.iterdir() if p.is_file() and p.name.lower()=='titl.dat')
assert hashlib.md5(title.read_bytes()).hexdigest()=='5b590ea3a6f5eed513b5678b01468ee4'
with zipfile.ZipFile(out,'w',compression=zipfile.ZIP_STORED) as z:
    for i in range(9): z.writestr(f'orphan-{i:02d}/TITL.DAT',title.read_bytes())
    z.write(adf,'zz-complete.adf')
PY
"$app" --scan-data --game csb --data-dir "$archive" > "$scratch/title-candidate.log" 2>&1
grep -Eq 'Amiga 3\.1/3\.3 Multilanguage +FOUND .*::zz-complete\.adf::graphics\.dat' "$scratch/title-candidate.log"
echo 'PASS: authentic CSB A33M archive title after nine orphan title files is discovered'
