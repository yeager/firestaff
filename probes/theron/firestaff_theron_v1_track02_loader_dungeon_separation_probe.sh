#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
verifier="$repo/scripts/verify_theron_track02_loader_dungeon_separation.sh"
tmpdir=$(mktemp -d "${TMPDIR:-/tmp}/theron-loader-dungeon.XXXXXX")
trap 'rm -rf "$tmpdir"' EXIT

cat >"$tmpdir/us.trace" <<'EOF'
source=mednafen-pce-instrumented
dynamic_cd_read_transaction pc=4090 return_pc=4093 sector_count=01 destination=3800 record_register_mask=07 record_cl=e0 record_dl=04 record_ch=00 variant=us_bin record=0004e0
EOF

"$verifier" "$tmpdir/us.trace" >/dev/null

sed 's/record=0004e0/record=000b52/' "$tmpdir/us.trace" >"$tmpdir/collapsed.trace"
if "$verifier" "$tmpdir/collapsed.trace" >/dev/null 2>&1; then
    printf 'FAIL: level record cannot pass as a stage-two loader transaction\n' >&2
    exit 1
fi

sed 's/destination=3800/destination=3000/' "$tmpdir/us.trace" >"$tmpdir/wrong-destination.trace"
if "$verifier" "$tmpdir/wrong-destination.trace" >/dev/null 2>&1; then
    printf 'FAIL: non-$3800 transfer cannot pass the stage-two loader gate\n' >&2
    exit 1
fi

printf 'theron Track02 loader/dungeon separation probe: PASS\n'
