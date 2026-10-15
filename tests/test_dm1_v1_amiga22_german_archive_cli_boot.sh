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
import subprocess
import sys
import zipfile
member = "Floppy Disks ADF/Dungeon Master for Amiga v2.2 (German) Original (Not working).adf"
data = subprocess.check_output(["7zz", "x", "-so", sys.argv[1], member], stderr=subprocess.PIPE)
if len(data) != 910058 or hashlib.sha256(data).hexdigest() != "5abc4f549941eed4e6af48bd52d348212b38b0a17fd3e83609c8aaabc2e4cd73":
    raise SystemExit("FAIL: original A22G disk identity changed")
with zipfile.ZipFile(sys.argv[2], "w") as transport:
    transport.writestr("original.adf", data)
PYDATA
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS FIRESTAFF_EXIT_AFTER_LAUNCH
for route in direct menu; do
    args=(--game dm1 --platform amiga --data-dir "$scratch/original.zip" --debug)
    if [[ "$route" == direct ]]; then
        args+=(--boot-probe --boot-probe-frames 2 --boot-probe-expect-runtime
            --boot-probe-expect-level-loaded 1
            --boot-probe-expect-asset-md5 0679e39da9dcc2e855cb33c6c64ddcb5 --duration 0)
    else
        export FIRESTAFF_EXIT_AFTER_LAUNCH=1
        args+=(--menu --script 'enter,enter,enter' --duration 10000)
    fi
    FIRESTAFF_CONFIG_PATH="$scratch/$route.toml" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" "${args[@]}" >"$scratch/$route.log" 2>&1 || {
            cat "$scratch/$route.log" >&2; exit 1;
        }
    python3 - "$scratch/$route.log" <<'PYLOG'
from pathlib import Path
import sys
log = Path(sys.argv[1]).read_text(errors="replace")
required = ("tuple=a22g-four-files phase=title", "tuple=a22g-four-files phase=entrance",
            "platform=amiga-v20 phase=title-f0437", "platform=amiga-v20 phase=entrance-f0441",
            "DM1 READY: gameId=dm1")
if any(item not in log for item in required):
    print(log, file=sys.stderr)
    raise SystemExit("FAIL: original A22G source startup/handoff is incomplete")
print("PASS: " + Path(sys.argv[1]).name)
PYLOG
done
echo 'PASS: original A22G CLI and menu startup'
