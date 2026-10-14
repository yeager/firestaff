#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 || $# -gt 2 ]]; then
    printf 'usage: %s <firestaff-binary> [atari-stx-extractor-test]\n' "$0" >&2
    exit 2
fi

app=$1
atari_stx_test=${2:-}
archive=${FIRESTAFF_DM1_PC34_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"}
# Card startup flow: game card -> verified PC card -> Original card. Space
# scripted input across M12 frames so slow authenticated source intro work
# cannot consume the entire menu selection sequence in one host frame.
menu_original='wait20,enter,wait20,enter,wait20,enter'
hoc_route='wait5,key:kp5,key:kp5,key:kp5,key:kp5,key:kp5,key:kp1,key:kp1,key:kp1,key:kp2,key:kp2,key:kp2,key:kp2,key:kp2,key:kp1,key:kp1,key:kp5,key:kp1,key:kp1,key:kp5,key:kp1,key:kp1,key:kp1,key:kp1,key:kp1,key:kp2,key:kp1,key:kp6,key:kp6,wait5,click:112:83,wait5'

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic DM1 PC-34 archive is not staged'
    exit 77
fi

test_scratch=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$test_scratch"
menu_probe_json="$test_scratch/dm1-menu-runtime-$$.json"

# A host quit after the first authentic PC-34 C001 frame should leave the
# launched game cleanly instead of reporting corrupt startup media.
title_quit_home="$test_scratch/dm1-pc34-title-quit-home-$$"
title_quit_probe="$test_scratch/dm1-pc34-title-quit-$$.json"
mkdir -p "$title_quit_home"
title_quit_output=$(HOME="$title_quit_home" XDG_CONFIG_HOME="$title_quit_home" \
    APPDATA="$title_quit_home" FIRESTAFF_CONFIG_PATH="$title_quit_home/config.toml" \
    FIRESTAFF_AUTOTEST=1 FIRESTAFF_AUTOTEST_DM1_TITLE_QUIT_AFTER_FRAME=1 \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$title_quit_probe" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform pc --data-dir "$archive" \
    --script "$menu_original" --duration 10000 2>&1) || {
        printf '%s\n' "$title_quit_output" >&2
        exit 1
    }
if ! grep -Fq 'AUTOTEST: DM1 C001 quit injected after first rendered frame' \
        <<< "$title_quit_output" ||
   grep -qi 'launch failed' <<< "$title_quit_output"; then
    printf '%s\n' "$title_quit_output" >&2
    printf '%s\n' 'FAIL: PC-34 C001 quit was not handled as a clean game exit' >&2
    exit 1
fi
python3 - "$title_quit_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
if probe.get("launchedEver") != 1 or probe.get("active") != 0:
    raise SystemExit(f"FAIL: PC-34 C001 quit did not exit the game cleanly: {probe}")
print("PASS: authentic PC-34 C001 host quit exits cleanly after its first frame")
PY
rm -f "$title_quit_probe"
rm -rf "$title_quit_home"

# The PC34 launch plan requires the original SWSH prelude. Keep all other
# files from the authentic archive, remove only this source asset in a private
# test copy, and verify the CLI does not claim a completed launch without it.
missing_swsh_root="$test_scratch/dm1-missing-swsh-$$"
missing_swsh_home="$missing_swsh_root/home"
mkdir -p "$missing_swsh_root/data" "$missing_swsh_home"
unzip_log="$missing_swsh_root/unzip.log"
unzip_status=0
unzip -q "$archive" -d "$missing_swsh_root/data" 2>"$unzip_log" || unzip_status=$?
if (( unzip_status > 1 )); then
    cat "$unzip_log" >&2
    echo "FAIL: could not extract the authentic DM1 test archive (unzip status $unzip_status)" >&2
    exit 1
fi
runtime_data_root=$(python3 - "$missing_swsh_root/data" <<'PY'
from pathlib import Path
import sys

root = Path(sys.argv[1])
matches = [path.parent for path in root.rglob("GRAPHICS.DAT")
           if path.is_file() and path.name.casefold() == "graphics.dat" and
           (path.parent / "DUNGEON.DAT").is_file()]
if len(matches) != 1:
    raise SystemExit(
        f"FAIL: expected one original DM1 GRAPHICS.DAT/DUNGEON.DAT pair; found {matches}")
print(matches[0])
PY
)

# Issue #13 reported that DM1 PC 3.4 could be marked verified while the
# extracted DUNGEON.DAT failed to load. Exercise the installed loose-file
# layout using bytes extracted from the authentic retail ZIP, before the
# negative missing-SWSH case mutates this private copy.
loose_runtime_probe="$test_scratch/dm1-pc34-loose-runtime-$$.json"
loose_runtime_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$loose_runtime_probe" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform pc --data-dir "$runtime_data_root" \
    --script "$menu_original" --duration 30000 2>&1)" || {
    printf '%s\n' "$loose_runtime_output" >&2
    exit 1
}
if ! grep -Fq 'DM1 READY: gameId=dm1' <<<"$loose_runtime_output" ||
   ! grep -Fq "dataDir=$runtime_data_root" <<<"$loose_runtime_output" ||
   ! grep -Fq "graphicsPath=$runtime_data_root/GRAPHICS.DAT" <<<"$loose_runtime_output"; then
    printf '%s\n' "$loose_runtime_output" >&2
    printf '%s\n' 'FAIL: DM1 PC-34 loose-file menu did not bind the original GRAPHICS.DAT/DUNGEON.DAT pair' >&2
    exit 1
fi
python3 - "$loose_runtime_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or startup["receiptReady"] != 1 or
        startup["phase"] != "dm1-runtime" or startup["active"] != 1 or
        startup["startupActive"] != 0 or startup["levelLoaded"] != 1 or
        startup["dm1StartupHandoffExecuted"] != 1 or
        startup["dm1StartupHoCFirstFrameReady"] != 1):
    raise SystemExit(
        f"FAIL: authentic DM1 PC-34 loose-file menu failed to load DUNGEON.DAT: {probe}")
print("PASS: authentic DM1 PC-34 loose GRAPHICS.DAT/DUNGEON.DAT reach the first menu runtime frame")
PY
rm -f "$loose_runtime_probe"

# When authentic Atari STX media is available, decode its original dungeon
# with the production STX reader, then place it beside
# the selected PC 3.4 graphics while keeping the matching PC dungeon below
# the original extraction tree. The selected PC graphics identity must bind
# the corresponding PC DUNGEON.DAT, even when an Atari dungeon is found first.
wrong_dungeon=${FIRESTAFF_DM1_WRONG_DUNGEON_PATH:-}
wrong_dungeon_archive=${FIRESTAFF_DM1_WRONG_DUNGEON_ARCHIVE:-}
if [[ -z "$wrong_dungeon" && -n "$wrong_dungeon_archive" &&
      -f "$wrong_dungeon_archive" && -x "$atari_stx_test" ]]; then
    wrong_dungeon_root="$test_scratch/dm1-wrong-edition-$$"
    mkdir -p "$wrong_dungeon_root"
    wrong_dungeon="$wrong_dungeon_root/DUNGEON.DAT"
    FIRESTAFF_DM1_ATARI_ARCHIVE="$wrong_dungeon_archive" \
        FIRESTAFF_DM1_ATARI_DUNGEON_OUTPUT="$wrong_dungeon" \
        "$atari_stx_test"
    python3 - "$wrong_dungeon" <<'PY'
from hashlib import md5
from pathlib import Path
import sys

path = Path(sys.argv[1])
actual = md5(path.read_bytes()).hexdigest()
expected = "050fb2cfded1b502ec2c53956b94c5bd"
if actual != expected:
    raise SystemExit(
        f"FAIL: original Atari STX DUNGEON.DAT identity {actual}, expected {expected}")
print(f"PASS: decoded authentic Atari STX DUNGEON.DAT {actual}")
PY
fi
if [[ -n "$wrong_dungeon" && -f "$wrong_dungeon" ]]; then
    mixed_root="$test_scratch/dm1-pc34-mixed-dungeon-$$"
    mixed_pc_root="$mixed_root/dos_extract/Dungeon-Master_DOS_EN_Version-34"
    mkdir -p "$mixed_pc_root"
    cp -R "$missing_swsh_root/data/." "$mixed_pc_root/"
    pc_graphics=$(python3 - "$mixed_pc_root" <<'PY'
from pathlib import Path
import sys

root = Path(sys.argv[1])
matches = [path for path in root.rglob("GRAPHICS.DAT")
           if path.is_file() and (path.parent / "DUNGEON.DAT").is_file()]
if len(matches) != 1:
    raise SystemExit(f"FAIL: expected one matching PC34 graphics file, found {matches}")
print(matches[0])
PY
)
    expected_pc_dungeon="${pc_graphics%/*}/DUNGEON.DAT"
    rm "$pc_graphics"
    cp "$runtime_data_root/GRAPHICS.DAT" "$mixed_root/GRAPHICS.DAT"
    cp "$wrong_dungeon" "$mixed_root/DUNGEON.DAT"
    mixed_probe="$mixed_root/runtime.json"
    mixed_output="$(FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$mixed_probe" \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --game dm1 --platform pc --data-dir "$mixed_root" --debug \
        --boot-probe --boot-probe-frames 120 --duration 0 2>&1)" || {
        printf '%s\n' "$mixed_output" >&2
        exit 1
    }
    if ! grep -Fq "LOADING DUNGEON: [$expected_pc_dungeon]" <<<"$mixed_output" ||
       ! grep -Fq "graphicsPath=$mixed_root/GRAPHICS.DAT" <<<"$mixed_output" ||
       ! grep -Fq 'phase=dm1-runtime' <<<"$mixed_output" ||
       ! grep -Fq 'levelLoaded=1' <<<"$mixed_output"; then
        printf '%s\n' "$mixed_output" >&2
        echo 'FAIL: PC 3.4 English graphics loaded a different edition DUNGEON.DAT' >&2
        exit 1
    fi
    echo 'PASS: PC 3.4 English graphics selected their matching DUNGEON.DAT from mixed media'
    rm -rf "$mixed_root"
fi
if [[ -n "${wrong_dungeon_root:-}" ]]; then
    rm -rf "$wrong_dungeon_root"
fi

# Verify the same original loose-file pair from the documented per-user
# installation root.  This combines the loose DUNGEON.DAT runtime path with
# M12's default-root discovery; neither --data-dir nor FIRESTAFF_DATA may
# hide a mismatch between the source that the launcher authenticates and the
# source that M11 opens.
default_loose_home="$test_scratch/dm1-pc34-default-loose-$$"
default_loose_data="$default_loose_home/.firestaff/data/dm1"
default_loose_probe="$default_loose_home/runtime.json"
mkdir -p "$default_loose_data"
cp -R "$missing_swsh_root/data/." "$default_loose_data/"
# Default-root discovery scans the installed tree before it can play the
# source-owned SWSH and PC-34 title. Give the complete authentic startup route
# enough run budget even on hosts with slower storage.
default_loose_output="$(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR
    HOME="$default_loose_home" XDG_CONFIG_HOME="$default_loose_home/.config" \
    APPDATA="$default_loose_home" \
    FIRESTAFF_CONFIG_PATH="$default_loose_home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$default_loose_probe" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --menu --game dm1 --debug \
        --script 'wait20,enter,wait20,enter,wait20,enter' --duration 30000 2>&1
)" || {
    printf '%s\n' "$default_loose_output" >&2
    exit 1
}
if ! grep -Fq 'handoff=pc-img3' <<<"$default_loose_output"; then
    printf '%s\n' "$default_loose_output" >&2
    echo 'FAIL: DM1 AUTO menu did not launch loose PC-34 media from ~/.firestaff/data/dm1 without --data-dir' >&2
    exit 1
fi
python3 - "$default_loose_output" <<'PY'
import re
import sys

pattern = re.compile(r"startup-source-frame game=dm1 platform=pc phase=title-f0437 (.*)")
rows = []
for line in sys.argv[1].splitlines():
    match = pattern.search(line)
    if match:
        fields = dict(part.split("=", 1) for part in match.group(1).split())
        rows.append(fields)

if len(rows) != 23:
    raise SystemExit(f"FAIL: expected 23 timed DM1 PC34 C001 events, found {len(rows)}")
steps = [int(row["step"].split("/", 1)[0]) for row in rows]
if steps != list(range(1, 24)):
    raise SystemExit(f"FAIL: DM1 PC34 title events were reordered or skipped: {steps}")
if any(row["fast-forward"] != "0" for row in rows):
    raise SystemExit("FAIL: ordinary M12 title run unexpectedly fast-forwarded its source waits")
total_ms = int(rows[-1]["total-elapsed-ms"])
if total_ms < 3900:
    raise SystemExit(
        f"FAIL: DM1 PC34 C001 title duration was shorter than its source-bound cadence: {total_ms} ms")
if rows[0]["event"] != "0" or int(rows[0]["requested-wait-ms"]) < 1600:
    raise SystemExit(f"FAIL: DM1 PRESENTS hold was missing from its timing receipt: {rows[0]}")
print(
    f"PASS: DM1 PC34 title timing logged 23 ordered source events in {total_ms} ms; "
    f"max present={max(int(row['present-ms']) for row in rows)} ms")
PY
python3 - "$default_loose_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or
        probe["bootAssetMd5"] != "fa6b1aa29e191418713bf2cda93d962e" or
        startup["phase"] != "dm1-runtime" or startup["levelLoaded"] != 1 or
        startup["dm1StartupHandoffExecuted"] != 1 or
        startup["dm1StartupHoCFirstFrameReady"] != 1):
    raise SystemExit(
        f"FAIL: default-root loose PC-34 media did not reach runtime: {probe}")
print("PASS: authentic DM1 PC-34 loose media in ~/.firestaff/data/dm1 reaches runtime without --data-dir")
PY
rm -rf "$default_loose_home"
python3 - "$missing_swsh_root/data" <<'PY'
from pathlib import Path
import sys

root = Path(sys.argv[1])
matches = [path for path in root.rglob("*")
           if path.is_file() and path.name.casefold() in {"swoosh", "swoosh.dat"}]
if len(matches) != 1:
    raise SystemExit(f"FAIL: expected one authentic SWSH file in test copy; found {matches}")
matches[0].unlink()
print("PASS: prepared an isolated missing-SWSH test copy from the original archive")
PY
missing_swsh_output_file="$missing_swsh_root/output.txt"
if HOME="$missing_swsh_home" XDG_CONFIG_HOME="$missing_swsh_home" \
   APPDATA="$missing_swsh_home" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
   SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
   --game dm1 --platform pc --data-dir "$runtime_data_root" \
   --debug --verbose --boot-probe --boot-probe-frames 120 --duration 0 \
   >"$missing_swsh_output_file" 2>&1; then
    cat "$missing_swsh_output_file" >&2
    echo "FAIL: DM1 reported a successful runtime handoff without the required SWSH asset" >&2
    exit 1
fi
if grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' "$missing_swsh_output_file" ||
   { ! grep -Fq 'game unavailable for --game: dm1' "$missing_swsh_output_file" &&
     ! grep -Fq 'direct launch failed for --game dm1' "$missing_swsh_output_file"; } ||
   ! grep -Fq 'dm1 platform=PC edition=pc34-en not matched' \
       "$missing_swsh_output_file" ||
   grep -Fq 'dm1 platform=PC edition=pc34-en matched source=' \
       "$missing_swsh_output_file"; then
    cat "$missing_swsh_output_file" >&2
    echo "FAIL: missing SWSH did not remove PC 3.4 from startup matching before runtime" >&2
    exit 1
fi
echo "PASS: DM1 hides PC 3.4 and rejects the launch when its source SWSH prelude is missing"
python3 - "$missing_swsh_root" <<'PY'
from pathlib import Path
import shutil
import sys

shutil.rmtree(Path(sys.argv[1]))
PY

probe() {
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" ||
       ! grep -Fq 'assetMd5=fa6b1aa29e191418713bf2cda93d962e' <<<"$output" ||
       ! grep -Fq 'phase=dm1-runtime' <<<"$output" ||
       ! grep -Fq 'levelLoaded=1' <<<"$output"; then
        printf '%s\n' "$output" >&2
        return 1
    fi
}

probe --game dm1 --platform pc --data-dir "$archive" --boot-probe --boot-probe-frames 120 \
    --duration 0
probe --game dm1 --platform pc --data-dir "$archive" --script "$menu_original" \
    --boot-probe --boot-probe-frames 120 --duration 0

# The public --game CLI route still enters through M12. Its boot probe must
# retain the source-owned M12-to-M11 startup receipts instead of passing on a
# runtime-only state.
direct_probe_json="$test_scratch/dm1-pc34-direct-probe-$$.json"
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$direct_probe_json" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform pc --data-dir "$archive" --boot-probe \
    --boot-probe-frames 120 --duration 0 >/dev/null 2>&1
python3 - "$direct_probe_json" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
if (probe["sourceId"] != "dm1" or startup["phase"] != "dm1-runtime" or
        startup["dm1StartupTitleRuntimeSource"] != 1 or
        startup["dm1StartupTitleCompletedSteps"] != 23 or
        startup["dm1StartupSwshConsumed"] != 1 or
        startup["dm1StartupTitleConsumed"] != 1 or
        startup["dm1StartupEntranceConsumed"] != 1 or
        startup["dm1StartupFullGraphicsConsumed"] != 1 or
        startup["dm1StartupHoCFirstFrameReady"] != 1):
    raise SystemExit(f"FAIL: DM1 --game CLI boot probe missed a source startup phase: {probe}")
print("PASS: DM1 --game CLI boot probe confirms SWSH, C001, ENTRANCE and the HoC handoff")
PY
rm -f "$direct_probe_json"

# Issue #13 used the ordinary `--game dm1` CLI route on Linux. Keep this
# platform-auto case separate from the explicit PC route above: with only the
# authentic PC 3.4 archive installed and no saved launcher preference, AUTO
# must resolve the PC release and load its original DUNGEON.DAT.
auto_cli_home="$test_scratch/dm1-cli-auto-home-$$"
mkdir -p "$auto_cli_home"
auto_cli_probe_json="$auto_cli_home/runtime.json"
auto_cli_output="$(HOME="$auto_cli_home" XDG_CONFIG_HOME="$auto_cli_home" \
    APPDATA="$auto_cli_home" FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$auto_cli_probe_json" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --data-dir "$archive" --verbose --boot-probe \
    --boot-probe-frames 120 --duration 0 2>&1)" || {
    printf '%s\n' "$auto_cli_output" >&2
    exit 1
}
if ! grep -Fq 'selected game=dm1 platform=PC edition=pc34-en' <<<"$auto_cli_output" ||
   ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$auto_cli_output" ||
   ! grep -Fq 'phase=dm1-runtime' <<<"$auto_cli_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$auto_cli_output"; then
    printf '%s\n' "$auto_cli_output" >&2
    printf '%s\n' 'FAIL: bare DM1 CLI did not select and load authentic PC 3.4 data' >&2
    exit 1
fi
python3 - "$auto_cli_probe_json" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
if (probe["sourceId"] != "dm1" or startup["phase"] != "dm1-runtime" or
        startup["levelLoaded"] != 1 or startup["dm1StartupHandoffExecuted"] != 1 or
        startup["dm1StartupSwshConsumed"] != 1 or
        startup["dm1StartupTitleConsumed"] != 1 or
        startup["dm1StartupEntranceConsumed"] != 1):
    raise SystemExit(f"FAIL: bare DM1 CLI missed its PC-34 runtime handoff: {probe}")
print("PASS: bare --game dm1 CLI selected PC 3.4 and reached runtime with original data")
PY
rm -f "$auto_cli_probe_json"

menu_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$menu_probe_json" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --menu --game dm1 --platform pc \
    --data-dir "$archive" --script "$menu_original" --duration 30000 2>&1)" || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
if ! grep -Fq 'DM1 READY: gameId=dm1' <<<"$menu_output" ||
   ! grep -Fq "dataDir=$archive" <<<"$menu_output" ||
   ! grep -Fq 'handoff=pc-img3' <<<"$menu_output"; then
    printf '%s\n' "$menu_output" >&2
    printf '%s\n' 'FAIL: authentic DM1 PC-34 start menu did not bind IMG3 source media' >&2
    exit 1
fi
python3 - "$menu_probe_json" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        startup["receiptReady"] != 1 or startup["active"] != 1 or
        startup["startupActive"] != 0 or startup["phase"] != "dm1-runtime" or
        startup["levelLoaded"] != 1):
    raise SystemExit(f"FAIL: authentic DM1 menu did not reach its first runtime frame: {probe}")
print("PASS: authentic DM1 PC-34 start menu reached the source-owned runtime frame")
PY

# Verify the PC fallback of the ordinary AUTO route in a clean config. Keep
# only this authenticated PC archive in an installed dm1/ data-root layout:
# AUTO correctly prefers FM Towns when both editions are available.
menu_auto_home="$test_scratch/dm1-menu-auto-home-$$"
mkdir -p "$menu_auto_home"
menu_auto_data_root="$menu_auto_home/data"
mkdir -p "$menu_auto_data_root/dm1"
cp "$archive" "$menu_auto_data_root/dm1/$(basename "$archive")"
startup_capture_root=$(mktemp -d "$test_scratch/dm1-pc34-startup-$$.XXXXXX")
menu_auto_probe_json="$menu_auto_home/runtime.json"
menu_auto_output="$(HOME="$menu_auto_home" XDG_CONFIG_HOME="$menu_auto_home" \
    APPDATA="$menu_auto_home" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$menu_auto_probe_json" \
    FIRESTAFF_DM1_STARTUP_CAPTURE_DIR="$startup_capture_root" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --data-dir "$menu_auto_data_root" \
    --script "$menu_original" --duration 30000 2>&1)" || {
    printf '%s\n' "$menu_auto_output" >&2
    exit 1
}
if ! grep -Fq 'handoff=pc-img3' <<<"$menu_auto_output"; then
    printf '%s\n' "$menu_auto_output" >&2
    printf '%s\n' 'FAIL: DM1 AUTO route did not discover PC-34 from the installed data root' >&2
    exit 1
fi
python3 - "$menu_auto_probe_json" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or
        probe["bootAssetMd5"] != "fa6b1aa29e191418713bf2cda93d962e" or
        startup["receiptReady"] != 1 or
        startup["active"] != 1 or startup["startupActive"] != 0 or
        startup["phase"] != "dm1-runtime" or startup["levelLoaded"] != 1 or
        startup["dm1StartupHandoffExecuted"] != 1 or
        startup["dm1StartupTitleRuntimeSource"] != 1 or
        startup["dm1StartupTitleCompletedSteps"] != 23 or
        startup["dm1StartupSwshConsumed"] != 1 or
        startup["dm1StartupTitleConsumed"] != 1 or
        startup["dm1StartupEntranceConsumed"] != 1 or
        startup["dm1StartupFullGraphicsConsumed"] != 1 or
        startup["dm1StartupHoCFirstFrameReady"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 3, 2, 0)):
    raise SystemExit(f"FAIL: authentic DM1 AUTO menu did not reach the PC-34 runtime: {probe}")
print("PASS: clean-config DM1 AUTO menu used all 23 GRAPHICS.DAT C001 title steps and reached runtime")
PY
python3 - "$startup_capture_root" <<'PY'
from pathlib import Path
import hashlib
import struct
import sys

root = Path(sys.argv[1])
frames = {}
geometry = None
for phase in ("c001-first-frame", "c001-zoom-mid", "c001-zoom-full",
              "c004-closed", "c004-door-step-1"):
    captures = list((root / phase).glob("*.bmp"))
    if len(captures) != 1:
        raise SystemExit(f"FAIL: expected one presented authentic {phase} frame; found {captures}")
    blob = captures[0].read_bytes()
    if len(blob) < 54 or blob[:2] != b"BM":
        raise SystemExit(f"FAIL: {phase} capture is not a BMP")
    offset = struct.unpack_from("<I", blob, 10)[0]
    width, signed_height = struct.unpack_from("<Ii", blob, 18)
    height = abs(signed_height)
    bits = struct.unpack_from("<H", blob, 28)[0]
    if width < 320 or height < 200 or bits != 24:
        raise SystemExit(f"FAIL: unexpected {phase} presented geometry {width}x{height}x{bits}")
    if geometry is None:
        geometry = (width, height)
    elif geometry != (width, height):
        raise SystemExit(f"FAIL: startup phases changed presented geometry: {geometry} vs {(width, height)}")
    stride = ((width * bits + 31) // 32) * 4
    if offset + stride * height > len(blob):
        raise SystemExit(f"FAIL: truncated {phase} frame")
    colors = set()
    nonblack = 0
    for y in range(height):
        row = offset + y * stride
        for x in range(width):
            pixel = tuple(blob[row + x * 3:row + x * 3 + 3])
            colors.add(pixel)
            nonblack += pixel != (0, 0, 0)
    if nonblack == 0 or len(colors) < 2:
        raise SystemExit(f"FAIL: {phase} is blank/flat (nonblack={nonblack}, colours={len(colors)})")
    frames[phase] = hashlib.sha256(blob).hexdigest()
    print(f"PASS: authentic {phase} was presented ({nonblack} nonblack pixels, {len(colors)} colours)")
if (frames["c001-first-frame"] == frames["c001-zoom-mid"] or
        frames["c001-zoom-mid"] == frames["c001-zoom-full"]):
    raise SystemExit("FAIL: authentic C001 zoom frames did not change")
print("PASS: authentic C001 Presents, mid-zoom and full-zoom frames differ")
if frames["c004-closed"] == frames["c004-door-step-1"]:
    raise SystemExit("FAIL: Entrance closed-door and first door-animation frames did not change")
print("PASS: authentic Entrance presentation advanced from closed C004 to door step 1")
print(f"PASS: authentic startup captures share presented geometry {geometry[0]}x{geometry[1]}")
PY
rm -rf "$menu_auto_home"
rm -rf "$startup_capture_root"

# Exercise the complete authentic Hall route through the real M12->M11
# handoff. Scripted key events carry SDL scancodes just like host key presses;
# paced input preserves the PC-34 command cadence while the selected original
# archive supplies the Hall, portrait and candidate data.
menu_hoc_probe_json="$test_scratch/dm1-menu-hoc-$$.json"
menu_hoc_route=$(python3 -c 'import sys; tokens=sys.argv[1].split(","); print(",".join(token+(",wait:30" if token.startswith("key:") else "") for token in tokens))' \
    "$hoc_route")
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$menu_hoc_probe_json" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --menu --game dm1 --platform pc --data-dir "$archive" \
    --presentation-mode v1 --script "enter,enter,enter,$menu_hoc_route,click:130:115,wait:30,key:kp6,wait:60" \
    --duration 60000 >/dev/null
python3 - "$menu_hoc_probe_json" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
party = probe["party"]
candidate = probe["dm1HoC"]
if (probe["sourceId"] != "dm1" or probe["active"] != 1 or
        party["mapIndex"] != 0 or party["mapX"] != 14 or
        party["mapY"] != 3 or party["direction"] != 1 or
        party["championCount"] != 1 or candidate != {
            "candidatePanel": 0,
            "candidateOrdinal": -1,
            "candidatePartyIndex": -1,
        }):
    raise SystemExit(f"FAIL: authentic DM1 M12 Hall route did not confirm C127 ordinal 5: {probe}")
print("PASS: authentic DM1 M12 Hall route confirms C127 ordinal 5")
PY

# Physical card coordinates use the explicit launcher canvas, making this a
# real mouse-only game -> PC -> Original launch rather than a keyboard alias.
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --menu --game dm1 --platform pc \
    --data-dir "$archive" \
    --script 'wait20,click:700:262,wait20,click:934:405,wait20,click:450:405,wait20' \
    --duration 3000 >/dev/null 2>&1

# The authentic PC-3.4 dungeon starts at (map=0,x=1,y=3,dir=2).  A native
# `up` input advances to y=4; this proves the selected archive has reached
# the actual M11 movement route rather than only a title/startup receipt.
gameplay_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform pc --data-dir "$archive" --boot-probe --boot-probe-frames 500 \
    --script up --duration 0 2>&1) || {
    printf '%s\n' "$gameplay_output" >&2
    exit 1
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$gameplay_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$gameplay_output" ||
   ! grep -Fq 'map=0 party=1,4,2' <<<"$gameplay_output"; then
    printf '%s\n' "$gameplay_output" >&2
    printf '%s\n' 'FAIL: authentic DM1 PC-34 up input did not reach native movement' >&2
    exit 1
fi

# Every row begins from a fresh native archive session.  The original PC 3.4
# ZIP supplies the title, dungeon and party state throughout; do not replace
# it with a generated save, an extracted fixture, or a synthetic map.
probe_runtime_input() {
    local input=$1
    local expected_party=$2
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --game dm1 --platform pc --data-dir "$archive" \
        --boot-probe --boot-probe-frames 500 --script "$input" --duration 0 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    grep -Fq 'phase=dm1-runtime' <<<"$output" &&
    grep -Fq 'levelLoaded=1' <<<"$output" &&
    grep -Fq "map=0 party=$expected_party" <<<"$output"
}

# The authentic PC-3.4 spawn is (1,3), and the south neighbor (1,2) is a
# closed door in DUNGEON.DAT. DOWN must leave the party in place; moving north
# and then backing up verifies the same control can return from an open tile.
probe_runtime_input down 1,3,2
probe_runtime_input up,down 1,3,2
probe_runtime_input left 1,3,1
probe_runtime_input right 1,3,3
probe_runtime_input strafe-left 1,3,2
probe_runtime_input strafe-right 1,3,2
probe_runtime_input action 1,3,2

# ReDMCSB COMMAND.C:107-112 G0448 and 323-328 G0463 map C070's
# x=263..289/y=125..145 arrow to C003 forward movement. Exercise that
# source mouse route with the authenticated PC 3.4 ZIP at native 320x200.
mouse_move_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform pc --data-dir "$archive" --width 320 --height 200 \
    --boot-probe --boot-probe-frames 500 --script 'click:275:135' \
    --duration 0 2>&1) || {
    printf '%s\n' "$mouse_move_output" >&2
    exit 1
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$mouse_move_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$mouse_move_output" ||
   ! grep -Fq 'map=0 party=1,4,2' <<<"$mouse_move_output"; then
    printf '%s\n' "$mouse_move_output" >&2
    printf '%s\n' 'FAIL: authentic DM1 PC-34 C070 mouse arrow did not move forward' >&2
    exit 1
fi

# ReDMCSB COMMAND.C:107-112 G0448 maps all six movement boxes to C001..C006.
# Replay the Hall path with source-space mouse clicks only, then use the
# portrait's C080 viewport route and C040 RESURRECT button. The fresh PC 3.4
# archive supplies every tile, mirror, and champion throughout this route.
mouse_hoc_route=${hoc_route//key:kp5/click:275:135,wait5}
mouse_hoc_route=${mouse_hoc_route//key:kp1/click:247:157,wait5}
mouse_hoc_route=${mouse_hoc_route//key:kp2/click:275:157,wait5}
mouse_hoc_route=${mouse_hoc_route//key:kp6/click:305:135,wait5}
if [[ "$mouse_hoc_route" == *key:* ]]; then
    printf '%s\n' 'FAIL: PC-34 Hall mouse route still contains keyboard input' >&2
    exit 1
fi
mouse_hoc_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform pc --data-dir "$archive" --width 320 --height 200 \
    --boot-probe --boot-probe-frames 500 \
    --script "$mouse_hoc_route,click:130:115,wait5" --duration 0 2>&1) || {
    printf '%s\n' "$mouse_hoc_output" >&2
    exit 1
}
for receipt in 'phase=dm1-runtime' 'levelLoaded=1' \
    'map=0 party=14,3,0' 'champions=1' \
    'dm1HocCandidatePanel=0' 'dm1HocCandidateOrdinal=-1'; do
    if ! grep -Fq "$receipt" <<<"$mouse_hoc_output"; then
        printf '%s\n' "$mouse_hoc_output" >&2
        printf 'FAIL: authentic PC-34 mouse-only Hall route missed %s\n' "$receipt" >&2
        exit 1
    fi
done

# Authentic Hall of Champions route, derived from the mounted PC 3.4
# DUNGEON.DAT rather than from a save or a coordinate fixture.  The terminal
# C127 portrait is ordinal 5: after the source movement sequence, its C026
# portrait occupies screen x=96..127/y=68..96 (viewport-local 96..127/35..63).
# A click at its source centre must run REVIVE.C F0280 and append exactly one
# pending candidate.  Keeping this at CLI level catches presentation/input
# scaling regressions that a direct M11-state probe cannot see.
hoc_capture_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$hoc_capture_root"
hoc_capture_dir=$(mktemp -d "$hoc_capture_root/dm1-hoc-c040.XXXXXX")
hoc_inventory_capture_dir=$(mktemp -d "$hoc_capture_root/dm1-hoc-c007.XXXXXX")
hoc_modern_capture_dir=$(mktemp -d "$hoc_capture_root/dm1-hoc-c040-modern.XXXXXX")
cleanup_hoc_capture() {
    find "$hoc_capture_dir" -depth -delete
    find "$hoc_inventory_capture_dir" -depth -delete
    find "$hoc_modern_capture_dir" -depth -delete
}
trap cleanup_hoc_capture EXIT HUP INT TERM
hoc_output=$(FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$hoc_capture_dir" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --presentation-mode v1 --width 320 --height 200 \
    --game dm1 --platform pc --data-dir "$archive" \
    --boot-probe --boot-probe-frames 720 --script "$hoc_route" --duration 0 2>&1) || {
    printf '%s\n' "$hoc_output" >&2
    exit 1
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$hoc_output" ||
   ! grep -Fq 'map=0 party=14,3,0 champions=1' <<<"$hoc_output" ||
   ! grep -Fq 'dm1HocCandidatePanel=1' <<<"$hoc_output" ||
   ! grep -Fq 'dm1HocCandidateOrdinal=5' <<<"$hoc_output" ||
   ! grep -Fq 'dm1HocCandidatePartyIndex=0' <<<"$hoc_output"; then
    printf '%s\n' "$hoc_output" >&2
    printf '%s\n' 'FAIL: authentic PC-34 Hall C127 portrait click did not open C040 for ordinal 5' >&2
    exit 1
fi

# State receipts are not presentation proof. C040 is a source-sized 144x73
# overlay at screen (80,85), whose original PC3.4 raster is materially dense.
# A cleared/stale admission leaves this rectangle black even while the input
# state says candidatePanel=1, which is the reported invisible HoC panel bug.
python3 - "$hoc_capture_dir" <<'PY'
import pathlib
import struct
import sys

files = list(pathlib.Path(sys.argv[1]).glob("*.bmp"))
if len(files) != 1:
    raise SystemExit("FAIL: expected one native DM1 C040 presentation capture")
blob = files[0].read_bytes()
if len(blob) < 54 or blob[:2] != b"BM":
    raise SystemExit("FAIL: native DM1 C040 capture is not BMP")
offset = struct.unpack_from("<I", blob, 10)[0]
width, signed_height = struct.unpack_from("<Ii", blob, 18)
height = abs(signed_height)
bits = struct.unpack_from("<H", blob, 28)[0]
stride = ((width * bits + 31) // 32) * 4
if width != 320 or height != 200 or bits != 24 or offset + stride * height > len(blob):
    raise SystemExit("FAIL: invalid native DM1 C040 capture geometry")

# The BMP is BGR. The C040 rectangle is deliberately measured, not compared
# to copyrighted pixels; its source graphic has a stable dense non-black body
# and more than the black/cyan control-strip colours.
pixels = []
for y in range(85, 158):
    row = offset + y * stride
    pixels.extend(tuple(blob[row + x * 3:row + x * 3 + 3]) for x in range(80, 224))
nonblack = sum(pixel != (0, 0, 0) for pixel in pixels)
colours = len(set(pixels))
if nonblack < 5000 or colours < 10:
    raise SystemExit(
        "FAIL: authentic PC-34 C040 was not visibly presented "
        f"(nonblack={nonblack}, colours={colours})")
print(f"PASS: authentic PC-34 C040 visible nonblack={nonblack} colours={colours}")
PY

# The Mac regression was not a source-coordinate failure: a 16:9 window
# letterboxes the 4:3 original page. Exercise the same portrait through the
# physical 1920x1080 point (744,464), which maps to source (112,83) inside
# the 1600x1000 presentation rectangle. This protects the normal SDL pointer
# transform, rather than merely the boot probe's 320x200 convenience route.
hoc_scaled_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --presentation-mode v1 --width 1920 --height 1080 --scale-mode 4 \
    --game dm1 --platform pc --data-dir "$archive" \
    --boot-probe --boot-probe-frames 720 \
    --script "${hoc_route%click:112:83,wait5}click:744:464,wait5" --duration 0 2>&1) || {
    printf '%s\n' "$hoc_scaled_output" >&2
    exit 1
}
if ! grep -Fq 'window=1920x1080' <<<"$hoc_scaled_output" ||
   ! grep -Fq 'dm1HocCandidatePanel=1' <<<"$hoc_scaled_output" ||
   ! grep -Fq 'dm1HocCandidateOrdinal=5' <<<"$hoc_scaled_output"; then
    printf '%s\n' "$hoc_scaled_output" >&2
    printf '%s\n' 'FAIL: scaled authentic PC-34 Hall portrait click missed C040' >&2
    exit 1
fi

# Modern changes the render target, but it must not change the original
# source-coordinate hit route.  Use the same real PC3.4 archive and physical
# 16:9 click; this caught variants where C040 worked only in Original mode.
hoc_modern_output=$(FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$hoc_modern_capture_dir" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --presentation-mode v20 --width 1920 --height 1080 --scale-mode 4 \
    --game dm1 --platform pc --data-dir "$archive" \
    --boot-probe --boot-probe-frames 720 \
    --script "${hoc_route%click:112:83,wait5}click:744:464,wait5" --duration 0 2>&1) || {
    printf '%s\n' "$hoc_modern_output" >&2
    exit 1
}
if ! grep -Fq 'window=1920x1080' <<<"$hoc_modern_output" ||
   ! grep -Fq 'presentationMode=1' <<<"$hoc_modern_output" ||
   ! grep -Fq 'dm1HocCandidatePanel=1' <<<"$hoc_modern_output" ||
   ! grep -Fq 'dm1HocCandidateOrdinal=5' <<<"$hoc_modern_output"; then
    printf '%s\n' "$hoc_modern_output" >&2
    printf '%s\n' 'FAIL: Modern scaled authentic PC-34 Hall portrait click missed C040' >&2
    exit 1
fi

# Modern may change the target resolution but must not accidentally retain a
# state-only C040 modal while omitting its source-backed pixels.  This is the
# same real PC3.4 click as above at a physical 16:9 coordinate; inspect the
# original source rectangle after presentation rather than relying on the
# modal receipt alone.
python3 - "$hoc_modern_capture_dir" <<'PY'
import pathlib
import struct
import sys

files = list(pathlib.Path(sys.argv[1]).glob("*.bmp"))
if len(files) != 1:
    raise SystemExit("FAIL: expected one Modern native DM1 C040 presentation capture")
blob = files[0].read_bytes()
if len(blob) < 54 or blob[:2] != b"BM":
    raise SystemExit("FAIL: Modern native DM1 C040 capture is not BMP")
offset = struct.unpack_from("<I", blob, 10)[0]
width, signed_height = struct.unpack_from("<Ii", blob, 18)
height = abs(signed_height)
bits = struct.unpack_from("<H", blob, 28)[0]
stride = ((width * bits + 31) // 32) * 4
# The screenshot hook captures the actual presentation target, not the host
# window.  Modern V2.0's default internal target is 640x400 (2x the original
# source page); 1920x1080 is only the letterboxed host presentation.  Requiring
# host-window geometry here hid a valid high-resolution C040 behind a stale
# test assumption.
if width != 640 or height != 400 or bits != 24 or offset + stride * height > len(blob):
    raise SystemExit("FAIL: invalid Modern native DM1 C040 capture geometry")

# The 640x400 target preserves the source geometry at exactly 2x.  C040's
# source rectangle (80,85)-(224,158) maps to (160,170)-(448,316). Test
# density/diversity only, never source pixels.
pixels = []
for y in range(170, 316):
    row = offset + y * stride
    pixels.extend(tuple(blob[row + x * 3:row + x * 3 + 3]) for x in range(160, 448))
nonblack = sum(pixel != (0, 0, 0) for pixel in pixels)
colours = len(set(pixels))
if nonblack < 20000 or colours < 10:
    raise SystemExit(
        "FAIL: Modern authentic PC-34 C040 was not visibly presented "
        f"(nonblack={nonblack}, colours={colours})")
print(f"PASS: Modern authentic PC-34 C040 visible nonblack={nonblack} colours={colours}")
PY

# C040 is interactive, not merely a painted modal.  Its RESURRECT control is
# centred at (130,115) in the same source-sized PC viewport.  It must consume
# the pending C127 candidate and close the panel through the normal REVIVE.C
# path.  Keep this separate from the open assertion above so a broken click
# transform cannot be hidden by the first portrait dispatch.
hoc_confirm_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --presentation-mode v1 --width 320 --height 200 \
    --game dm1 --platform pc --data-dir "$archive" \
    --boot-probe --boot-probe-frames 720 \
    --script "${hoc_route},click:130:115,wait5" --duration 0 2>&1) || {
    printf '%s\n' "$hoc_confirm_output" >&2
    exit 1
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$hoc_confirm_output" ||
   ! grep -Fq 'map=0 party=14,3,0 champions=1' <<<"$hoc_confirm_output" ||
   ! grep -Fq 'dm1HocCandidatePanel=0' <<<"$hoc_confirm_output" ||
   ! grep -Fq 'dm1HocCandidateOrdinal=-1' <<<"$hoc_confirm_output" ||
   ! grep -Fq 'dm1HocCandidatePartyIndex=-1' <<<"$hoc_confirm_output"; then
    printf '%s\n' "$hoc_confirm_output" >&2
    printf '%s\n' 'FAIL: authentic PC-34 Hall C040 RESURRECT click did not consume ordinal 5' >&2
    exit 1
fi

# C040 must remain modal: a source-space top-row click cannot dismiss the
# live revive panel.  C040 intentionally retains C017's inventory backing
# state, so that state alone is not a dispatch observation; the visible C040
# ownership is the source-relevant assertion.
hoc_source_coordinate_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --presentation-mode v1 --width 320 --height 200 \
    --game dm1 --platform pc --data-dir "$archive" \
    --boot-probe --boot-probe-frames 720 \
    --script "${hoc_route},click:22:14,wait5" --duration 0 2>&1) || {
    printf '%s\n' "$hoc_source_coordinate_output" >&2
    exit 1
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$hoc_source_coordinate_output" ||
   ! grep -Fq 'dm1HocCandidatePanel=1' <<<"$hoc_source_coordinate_output"; then
    printf '%s\n' "$hoc_source_coordinate_output" >&2
    printf '%s\n' 'FAIL: authentic PC-34 HoC C040 did not remain modal at (22,14)' >&2
    exit 1
fi

# C007's independently verified inventory control is centred at (54,14)
# after C160 has recruited the mirror candidate. Keep this a live PC3.4 CLI
# route: it guards the reported HoC state where panel clicks looked accepted
# but never opened the champion inventory.
hoc_inventory_output=$(FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$hoc_inventory_capture_dir" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --presentation-mode v1 --width 320 --height 200 \
    --game dm1 --platform pc --data-dir "$archive" \
    --boot-probe --boot-probe-frames 720 \
    --script "${hoc_route},click:130:115,wait5,click:54:14,wait5" --duration 0 2>&1) || {
    printf '%s\n' "$hoc_inventory_output" >&2
    exit 1
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$hoc_inventory_output" ||
   ! grep -Fq 'dm1HocCandidatePanel=0' <<<"$hoc_inventory_output" ||
   ! grep -Fq 'dm1InventoryPanel=1' <<<"$hoc_inventory_output" ||
   ! grep -Fq 'dm1FoodWaterPanel=1' <<<"$hoc_inventory_output"; then
    printf '%s\n' "$hoc_inventory_output" >&2
    printf '%s\n' 'FAIL: authentic PC-34 Hall C007 did not open source FOOD/WATER inventory after C160' >&2
    exit 1
fi

# C007 is a visible source page, not merely an input state.  A stale panel
# admission used to report InventoryPanel=1 while leaving the player with a
# black/right-hand dungeon pane.  Verify that the real PC3.4 route presents a
# dense multi-colour panel across the original left-hand 220-pixel UI region.
# This deliberately asserts only observable geometry and colour diversity,
# rather than encoding copyrighted source pixels.
python3 - "$hoc_inventory_capture_dir" <<'PY'
import pathlib
import struct
import sys

files = list(pathlib.Path(sys.argv[1]).glob("*.bmp"))
if len(files) != 1:
    raise SystemExit("FAIL: expected one native DM1 C007 presentation capture")
blob = files[0].read_bytes()
if len(blob) < 54 or blob[:2] != b"BM":
    raise SystemExit("FAIL: native DM1 C007 capture is not BMP")
offset = struct.unpack_from("<I", blob, 10)[0]
width, signed_height = struct.unpack_from("<Ii", blob, 18)
height = abs(signed_height)
bits = struct.unpack_from("<H", blob, 28)[0]
stride = ((width * bits + 31) // 32) * 4
if width != 320 or height != 200 or bits != 24 or offset + stride * height > len(blob):
    raise SystemExit("FAIL: invalid native DM1 C007 capture geometry")

pixels = []
for y in range(0, 200):
    row = offset + y * stride
    pixels.extend(tuple(blob[row + x * 3:row + x * 3 + 3]) for x in range(0, 220))
nonblack = sum(pixel != (0, 0, 0) for pixel in pixels)
colours = len(set(pixels))
if nonblack < 25000 or colours < 10:
    raise SystemExit(
        "FAIL: authentic PC-34 C007 was not visibly presented "
        f"(nonblack={nonblack}, colours={colours})")
print(f"PASS: authentic PC-34 C007 visible nonblack={nonblack} colours={colours}")
PY

# Keep debug performance telemetry parseable on a real-media gameplay path.
# SDL's dummy backend validates instrumentation only; it is not evidence of
# desktop/GPU frame rate.
perf_home="$test_scratch/dm1-perf-home-$$"
mkdir -p "$perf_home"
# The debug test build can spend several seconds opening the original archive
# and materializing the first runtime state before the profiling window starts.
# Keep a full sample window after that setup so telemetry remains deterministic.
perf_output=$(HOME="$perf_home" XDG_CONFIG_HOME="$perf_home" \
    APPDATA="$perf_home" FIRESTAFF_CONFIG_PATH="$perf_home/config.toml" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform pc --data-dir "$archive" --debug-full-redraw \
    --duration 30000 \
    2>&1) || {
        printf '%s\n' "$perf_output" >&2
        exit 1
    }
if ! grep -Eq 'firestaff: perf game=dm1 active=1 .*process-cpu-of-one-core-percent=([0-9]+\.[0-9]+|unavailable)' \
        <<< "$perf_output" ||
   ! grep -Fq 'game-frames-rendered-per-sec=' <<< "$perf_output" ||
   ! grep -Fq 'target-loop-fps=' <<< "$perf_output" ||
   ! grep -Fq 'debug-full-redraw=1' <<< "$perf_output" ||
   ! grep -Fq 'frame-compose-present-call-ms-max=' <<< "$perf_output" ||
   ! grep -Fq 'gpu-utilization=unavailable' <<< "$perf_output"; then
    printf '%s\n' "$perf_output" >&2
    printf '%s\n' 'FAIL: --debug-full-redraw did not emit complete DM1 performance telemetry' >&2
    exit 1
fi
render_fps=$(sed -n 's/.*game-frames-rendered-per-sec=\([0-9.]*\).*/\1/p' \
    <<< "$perf_output" | tail -n 1)
if [[ -z "$render_fps" ]] || ! awk -v fps="$render_fps" 'BEGIN { exit !(fps > 0.0) }'; then
    printf '%s\n' "$perf_output" >&2
    printf 'FAIL: dummy full-redraw FPS was not a positive measurement (got %s)\n' \
        "${render_fps:-missing}" >&2
    exit 1
fi
rm -rf "$perf_home"
printf '%s\n' 'PASS: authentic DM1 PC-34 --debug-full-redraw measures full-redraw FPS, CPU, frame-time, and explicit GPU availability'

trap - EXIT HUP INT TERM
cleanup_hoc_capture

printf '%s\n' 'PASS: authentic DM1 PC-34 archive reaches CLI, menu, and complete native input matrix'
