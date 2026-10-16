#!/usr/bin/env bash
set -euo pipefail
app=${1:?usage: test_dm1_v1_amiga22_german_archive_cli_boot.sh firestaff}
source_archive=${FIRESTAFF_DM1_AMIGA_SOFTWARE_ARCHIVE:-"$HOME/.firestaff/data/dm1/Game,Dungeon_Master,Amiga,Software.7z"}
if [[ ! -x "$app" || ! -f "$source_archive" ]] || ! command -v 7zz >/dev/null; then
    echo 'SKIP: original Amiga archive or development repacker is unavailable'
    exit 77
fi
scratch=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}/dm1-amiga22-de-$$
mkdir -p "$scratch"
trap 'rm -rf "$scratch"' EXIT
# Preserve the entire original disk, including its incomplete raw track.
# Only the transport changes; runtime must decode it without external tools.
python3 - "$source_archive" "$scratch/original.zip" <<'PYDATA'
import hashlib
from pathlib import Path
import subprocess
import sys
import zipfile
member = "Floppy Disks ADF/Dungeon Master for Amiga v2.2 (German) Original (Not working).adf"
data = subprocess.check_output(["7zz", "x", "-so", sys.argv[1], member], stderr=subprocess.PIPE)
if len(data) != 910058 or hashlib.sha256(data).hexdigest() != "5abc4f549941eed4e6af48bd52d348212b38b0a17fd3e83609c8aaabc2e4cd73":
    raise SystemExit("FAIL: original A22G disk identity changed")
Path(sys.argv[2]).with_name("original.adf").write_bytes(data)
with zipfile.ZipFile(sys.argv[2], "w") as transport:
    transport.writestr("original.adf", data)
PYDATA
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS FIRESTAFF_EXIT_AFTER_LAUNCH
for route in direct menu disk-menu; do
    media="$scratch/original.zip"
    if [[ "$route" == disk-menu ]]; then media="$scratch/original.adf"; fi
    mkdir -p "$scratch/$route-home"
    args=(--game dm1 --platform amiga --data-dir "$media" --debug)
    if [[ "$route" == direct ]]; then
        args+=(--boot-probe --boot-probe-frames 2 --boot-probe-expect-runtime
            --boot-probe-expect-level-loaded 1
            --boot-probe-expect-asset-md5 0679e39da9dcc2e855cb33c6c64ddcb5 --duration 0)
    else
        args+=(--menu --script 'enter,enter,enter' --duration 15000)
    fi
    HOME="$scratch/$route-home" FIRESTAFF_CONFIG_PATH="$scratch/$route.toml" \
        FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$scratch/$route.json" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" "${args[@]}" >"$scratch/$route.log" 2>&1 || {
            cat "$scratch/$route.log" >&2; exit 1;
        }
    python3 - "$scratch/$route.log" "$media" <<'PYLOG'
from pathlib import Path
import sys
log = Path(sys.argv[1]).read_text(errors="replace")
owner = sys.argv[2] + ("::original.adf" if sys.argv[2].endswith(".zip") else "")
required = ("tuple=a22g-four-files phase=title", "tuple=a22g-four-files phase=entrance",
            "platform=amiga-v20 phase=title-f0437", "platform=amiga-v20 phase=entrance-f0441",
            "DM1 READY: gameId=dm1", "graphicsPath=" + owner + "::graphics.dat",
            "dungeonPath=" + owner + "::DUNGEON.DAT")
if any(item not in log for item in required):
    print(log, file=sys.stderr)
    raise SystemExit("FAIL: original A22G source startup/handoff is incomplete")
print("PASS: " + Path(sys.argv[1]).name)
PYLOG
    if [[ "$route" != direct ]]; then
        python3 - "$scratch/$route.json" <<'PYRUNTIME'
import json
import sys
with open(sys.argv[1], encoding="utf-8") as stream:
    probe = json.load(stream)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or
        probe["bootAssetMd5"] != "0679e39da9dcc2e855cb33c6c64ddcb5" or
        startup["receiptReady"] != 1 or startup["active"] != 1 or
        startup["phase"] != "dm1-runtime" or startup["levelLoaded"] != 1 or
        startup["startupActive"] != 0 or
        startup["dm1StartupHandoffExecuted"] != 1 or
        startup["dm1StartupHoCFirstFrameReady"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 3, 2, 0)):
    raise SystemExit(f"FAIL: original A22G menu did not reach an active first Hall: {probe}")
print("PASS: original A22G actual menu reaches active Hall with authenticated original graphics")
PYRUNTIME
    fi
done
python3 - "$scratch/original.adf" "$scratch/original.zip" <<'PYUNCHANGED'
import hashlib
from pathlib import Path
import sys
import zipfile
expected = "5abc4f549941eed4e6af48bd52d348212b38b0a17fd3e83609c8aaabc2e4cd73"
with zipfile.ZipFile(sys.argv[2]) as transport:
    wrapped = transport.read("original.adf")
if any(hashlib.sha256(data).hexdigest() != expected
       for data in (Path(sys.argv[1]).read_bytes(), wrapped)):
    raise SystemExit("FAIL: original A22G disk bytes changed during startup")
PYUNCHANGED
echo 'PASS: original A22G CLI and menu startup'
