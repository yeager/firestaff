#!/usr/bin/env bash
set -euo pipefail

app=${1:?usage: test_dm1_v1_pc34_selector_archive_paths.sh firestaff}
archive=${FIRESTAFF_DM1_PC34_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"}
if [[ ! -x "$app" || ! -f "$archive" ]]; then
    echo 'SKIP: authentic DM1 PC34 archive is unavailable'
    exit 77
fi
scratch=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}/dm1-selector-paths-$$
mkdir -p "$scratch"
trap 'rm -rf "$scratch"' EXIT
# Transport changes preserve every original member byte; no game payload is
# generated. A directory prefix exercises the authenticated graphics locator.
python3 - "$archive" "$scratch" <<'PYDATA'
from pathlib import Path
import sys
import struct
import copy
import zipfile
source = Path(sys.argv[1]).resolve()
root = Path(sys.argv[2]).resolve()
(root / "FLAT.ZIP").symlink_to(source)
raw = source.read_bytes()
with zipfile.ZipFile(source) as original, zipfile.ZipFile(root / "wrapped.zip", "w") as wrapped:
    for item in original.infolist():
        # This retail ZIP uses backslashes in local headers but slashes in
        # its central directory. Accept only that documented separator alias;
        # ZipFile still checks decompression and payload CRC.
        offset = item.header_offset
        length = struct.unpack_from("<H", raw, offset + 26)[0]
        local_name = raw[offset + 30:offset + 30 + length].decode(
            "utf-8" if item.flag_bits & 0x800 else "cp437")
        if local_name.replace("\\", "/") != item.filename.replace("\\", "/"):
            raise SystemExit("FAIL: inconsistent original ZIP member identity")
        readable = copy.copy(item)
        readable.orig_filename = local_name
        wrapped.writestr("package/" + item.filename.replace("\\", "/"), original.read(readable))
(root / "PACKED.ZIP").symlink_to(root / "wrapped.zip")
PYDATA

probe() {
    local media=$1 mode=$2 log="$scratch/$2-$(basename "$1").log"
    local home="$scratch/home-$2-$(basename "$1")"
    local args=(--game dm1)
    mkdir -p "$home"
    if [[ "$mode" == menu ]]; then
        args+=(--menu --script 'wait20,enter,wait20,enter,wait20,enter')
    fi
    FIRESTAFF_CONFIG_PATH="$home/config.toml" FIRESTAFF_AUTOTEST=1 \
        FIRESTAFF_FAIL_IF_NO_LAUNCH=1 SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --platform pc --data-dir "$media" \
        "${args[@]}" --duration 12000 >"$log" 2>&1 || {
            cat "$log" >&2; return 1;
        }
    python3 - "$log" <<'PYLOG'
from pathlib import Path
import re
import sys
log = Path(sys.argv[1]).read_text(errors="replace")
selector = re.search(r"startup-source-complete game=dm1 platform=pc34 phase=selector .*frames=32 .*command=enter", log)
if not selector or "level-loaded=1" not in log or "DM1 READY: gameId=dm1" not in log:
    print(log, file=sys.stderr)
    raise SystemExit("FAIL: authentic selector and runtime handoff missing")
if "phase=selector reason=missing-or-unverified-source-media" in log:
    raise SystemExit("FAIL: authenticated selector was skipped")
print("PASS: " + str(Path(sys.argv[1]).name))
PYLOG
}
probe "$scratch/FLAT.ZIP" direct
probe "$scratch/FLAT.ZIP" menu
probe "$scratch/wrapped.zip" direct
probe "$scratch/PACKED.ZIP" menu
echo "PASS: all PC34 selector archive paths"
