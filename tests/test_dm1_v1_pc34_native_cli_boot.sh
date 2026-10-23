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

# A host quit after the first authentic PC-34 I34E title frame should leave
# the launch cleanly instead of reporting corrupt startup media.
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
if ! grep -Fq 'AUTOTEST: DM1 I34E title quit injected after first rendered frame' \
        <<< "$title_quit_output" ||
   grep -qi 'launch failed' <<< "$title_quit_output"; then
    printf '%s\n' "$title_quit_output" >&2
    printf '%s\n' 'FAIL: PC-34 I34E title quit was not handled as a clean game exit' >&2
    exit 1
fi
python3 - "$title_quit_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
if probe.get("launchedEver") != 1 or probe.get("active") != 0:
    raise SystemExit(f"FAIL: PC-34 I34E title quit did not exit cleanly: {probe}")
print("PASS: authentic PC-34 I34E title host quit exits cleanly after its first frame")
PY
rm -f "$title_quit_probe"
rm -rf "$title_quit_home"

# A host quit during the earlier SWSH logo is also a normal exit. This is a
# distinct M12 boundary from the I34E title interruption tested above.
swsh_quit_home="$test_scratch/dm1-pc34-swsh-quit-home-$$"
swsh_quit_probe="$test_scratch/dm1-pc34-swsh-quit-$$.json"
mkdir -p "$swsh_quit_home"
swsh_quit_output=$(HOME="$swsh_quit_home" XDG_CONFIG_HOME="$swsh_quit_home" \
    APPDATA="$swsh_quit_home" FIRESTAFF_CONFIG_PATH="$swsh_quit_home/config.toml" \
    FIRESTAFF_AUTOTEST=1 FIRESTAFF_AUTOTEST_DM1_SWSH_QUIT_AFTER_FRAME=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$swsh_quit_probe" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --debug --menu --game dm1 --platform pc --data-dir "$archive" \
    --script "$menu_original" --duration 10000 2>&1) || {
        printf '%s\n' "$swsh_quit_output" >&2
        exit 1
    }
if ! grep -Fq 'AUTOTEST: DM1 SWSH quit injected after first rendered frame' \
        <<< "$swsh_quit_output" ||
   ! grep -Fq 'startup-source-quit game=dm1 phase=swsh-intro action=exit' \
        <<< "$swsh_quit_output" ||
   grep -qi 'launch failed' <<< "$swsh_quit_output" ||
   grep -Fq 'AUTOTEST: DM1 I34E title quit injected' <<< "$swsh_quit_output"; then
    printf '%s\n' "$swsh_quit_output" >&2
    printf '%s\n' 'FAIL: PC-34 SWSH quit was not handled as a clean startup exit' >&2
    exit 1
fi
python3 - "$swsh_quit_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
if probe.get("active") != 0:
    raise SystemExit(f"FAIL: PC-34 SWSH quit left the game active: {probe}")
print("PASS: authentic PC-34 SWSH host quit exits without entering TITLE")
PY
rm -f "$swsh_quit_probe"
rm -rf "$swsh_quit_home"

# Keep a private copy of the authentic archive with its optional SWSH logo
# removed. The original loader continues to FTL_ANIM if FTL_SWSH fails, so the
# authenticated graphics/dungeon pair must remain playable without the logo.
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
# missing-SWSH case mutates this private copy.
loose_runtime_probe="$test_scratch/dm1-pc34-loose-runtime-$$.json"
loose_runtime_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$loose_runtime_probe" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --debug \
    --menu --game dm1 --platform pc --data-dir "$runtime_data_root" \
    --script "$menu_original" --duration 30000 2>&1)" || {
    printf '%s\n' "$loose_runtime_output" >&2
    exit 1
}
if ! grep -Fq 'DM1 READY: gameId=dm1' <<<"$loose_runtime_output" ||
   ! grep -Fq 'startup-source-complete game=dm1 platform=pc34 phase=anim-title' \
       <<<"$loose_runtime_output" ||
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
        probe["sourceId"] != "dm1"):
    raise SystemExit(
        f"FAIL: authentic DM1 PC-34 I34E menu failed to load DUNGEON.DAT: {probe}")
print("PASS: authentic DM1 PC-34 I34E title and loose-file menu reach the loaded runtime")
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
    wrong_only_root="$test_scratch/dm1-pc34-wrong-dungeon-only-$$"
    mkdir -p "$wrong_only_root"
    cp "$runtime_data_root/GRAPHICS.DAT" "$wrong_only_root/GRAPHICS.DAT"
    cp "$wrong_dungeon" "$wrong_only_root/DUNGEON.DAT"

    # The menu must not mark an authenticated PC graphics file ready when
    # the only adjacent dungeon belongs to another original edition.
    wrong_only_menu_output="$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --menu --data-dir "$wrong_only_root" \
        --script wait-scan-complete --duration 8000 --debug --verbose 2>&1)" || {
        printf '%s\n' "$wrong_only_menu_output" >&2
        exit 1
    }
    if ! grep -Fq 'scan complete elapsed-ms=' <<<"$wrong_only_menu_output" ||
       ! grep -Fq 'games=dm1:no,csb:no,dm2:no,nexus:no,theron:no' <<<"$wrong_only_menu_output" ||
       ! grep -Fq "firestaff: dm1 available=no scan-owner=$wrong_only_root" <<<"$wrong_only_menu_output" ||
       ! grep -Fq 'platform=PC edition=pc34-en not matched' <<<"$wrong_only_menu_output" ||
       grep -Fq 'launch phase=game-handoff' <<<"$wrong_only_menu_output"; then
        printf '%s\n' "$wrong_only_menu_output" >&2
        echo 'FAIL: startup menu admitted PC34 graphics with only an original Atari dungeon' >&2
        exit 1
    fi
    echo 'PASS: menu rejects original PC34 graphics paired only with an original Atari dungeon'

    wrong_only_cli_status=0
    wrong_only_cli_output="$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --game dm1 --platform pc --data-dir "$wrong_only_root" \
        --debug --verbose --boot-probe --boot-probe-frames 0 2>&1)" ||
        wrong_only_cli_status=$?
    if (( wrong_only_cli_status == 0 )) ||
       ! grep -Fq 'firestaff: dm1 available=no' <<<"$wrong_only_cli_output" ||
       ! grep -Fq 'game unavailable for --game: dm1' <<<"$wrong_only_cli_output" ||
       grep -Fq 'DM1 READY: gameId=dm1' <<<"$wrong_only_cli_output"; then
        printf '%s\n' "$wrong_only_cli_output" >&2
        echo 'FAIL: direct DM1 PC CLI admitted an original dungeon from another edition' >&2
        exit 1
    fi
    echo 'PASS: direct PC34 CLI rejects an original Atari dungeon without a matching PC dungeon'
    rm -rf "$wrong_only_root"

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
    if ! grep -Fq "dungeonPath=$expected_pc_dungeon" <<<"$mixed_output" ||
       ! grep -Fq "DM1 READY: gameId=dm1" <<<"$mixed_output" ||
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
if ! grep -Fq 'handoff=pc-img3' <<<"$default_loose_output" ||
   ! grep -Fq 'startup-source-complete game=dm1 platform=pc34 phase=anim-title frames=53' \
       <<<"$default_loose_output" ||
   ! grep -Fq 'startup-source-complete game=dm1 platform=pc34 phase=selector' \
       <<<"$default_loose_output"; then
    printf '%s\n' "$default_loose_output" >&2
    echo 'FAIL: DM1 AUTO menu did not launch loose PC-34 media from ~/.firestaff/data/dm1 without --data-dir' >&2
    exit 1
fi
python3 - "$default_loose_output" <<'PY'
import re
import sys

match = re.search(
    r"startup-source-complete game=dm1 platform=pc34 phase=anim-title "
    r"frames=(\d+) source-counter-units=(\d+) elapsed-ms=(\d+)",
    sys.argv[1])
if not match:
    raise SystemExit("FAIL: authentic PC-34 ANIM/TITLE completion receipt was missing")
frames, source_counter_units, elapsed_ms = map(int, match.groups())
if frames != 53 or source_counter_units != 53:
    raise SystemExit(
        f"FAIL: authentic PC-34 ANIM/TITLE frame count/cadence mismatch: {match.groups()}")
# Every assignment must cross a new default BIOS/PIT callback. The first
# callback phase is not recovered from the historical machine.
expected_ms = (frames - 1) * 65536000 // 1193182
if elapsed_ms + 20 < expected_ms:
    raise SystemExit(
        f"FAIL: PC-34 ANIM/TITLE ended before its default IBMIO PIT cadence: "
        f"{elapsed_ms} ms for {source_counter_units} raw counter units (expected "
        f"at least {expected_ms} ms)")
print(
    f"PASS: DM1 PC-34 ANIM/TITLE logged {frames} original frames and "
    f"{source_counter_units} raw counter units in {elapsed_ms} ms "
    "(default IBMIO PIT cadence; original initial phase/CPU remain unmeasured)")
selector = re.search(
    r"startup-source-complete game=dm1 platform=pc34 phase=selector .*"
    r"frames=(\d+) source-vblanks=(\d+) elapsed-ms=(\d+) "
    r"door-animation-ms=(\d+) post-animation-audio-wait-ms=(\d+) "
    r"modeled-vblank-hz=(\d+)",
    sys.argv[1])
if not selector:
    raise SystemExit("FAIL: authentic PC-34 selector Enter timing receipt was missing")
door_frames, door_vblanks, total_ms, animation_ms, audio_wait_ms, vblank_hz = \
    map(int, selector.groups())
expected_ms = door_frames * 1000 / vblank_hz
if (door_frames <= 1 or door_vblanks != door_frames or vblank_hz != 70 or
        animation_ms + 20 < expected_ms or animation_ms > expected_ms + 250 or
        total_ms != animation_ms + audio_wait_ms):
    raise SystemExit(
        f"FAIL: PC-34 selector door cadence diverged from 70 Hz VGA waits: "
        f"{selector.groups()}")
print(
    f"PASS: I34E selector door used {door_frames} source-sized steps at "
    f"{vblank_hz} Hz in {animation_ms} ms; post-animation audio wait "
    f"was {audio_wait_ms} ms (total selector handoff {total_ms} ms)")
sound = re.search(
    r"startup-source-complete game=dm1 platform=pc34 phase=selector .*"
    r"switch-event-queued=(\d+) door-events-queued=(\d+) snd3-events-loaded=(\d+)",
    sys.argv[1])
if not sound:
    raise SystemExit("FAIL: authentic PC-34 selector SND3 audio receipt was missing")
switch_queued, door_queued, snd3_loaded = map(int, sound.groups())
if switch_queued != 1 or door_queued < 1 or snd3_loaded < 3:
    raise SystemExit(
        f"FAIL: authentic PC-34 selector did not queue original switch/door SND3: "
        f"{sound.groups()}")
print(
    f"PASS: I34E selector queued switch/door SND3 events from its "
    f"{snd3_loaded}-event GRAPHICS.DAT bank")
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
        startup["active"] != 1):
    raise SystemExit(
        f"FAIL: default-root loose PC-34 media did not reach runtime: {probe}")
print("PASS: authentic PC-34 I34E title, selector Enter and loose media reach runtime without --data-dir")
PY
rm -rf "$default_loose_home"

# Issue #13's reported install root contains several editions side by side.
# Keep the authenticated loose PC-34 pair in ~/.firestaff/data/dm1 and expose
# the original ZIPs from its source directory so scanning sees real mixed
# media while `--game dm1 --platform pc` still has to bind the PC dungeon.
mixed_media_home="$test_scratch/dm1-pc34-mixed-default-$$"
mixed_media_data="$mixed_media_home/.firestaff/data/dm1"
mixed_media_pc34="$mixed_media_data/dos_extract/Dungeon-Master_DOS_EN_Version-34"
mixed_media_probe="$mixed_media_home/runtime.json"
mkdir -p "$mixed_media_pc34"
cp -R "$missing_swsh_root/data/." "$mixed_media_pc34/"
mixed_media_pc34_data=$(python3 - "$mixed_media_pc34" <<'PY'
from hashlib import md5
from pathlib import Path
import sys

root = Path(sys.argv[1])
graphics_hash = "fa6b1aa29e191418713bf2cda93d962e"
dungeon_hash = "766450c940651fc021c92fe5d0d0b3a6"
pairs = []
for graphics in root.rglob("GRAPHICS.DAT"):
    dungeon = graphics.parent / "DUNGEON.DAT"
    if (graphics.is_file() and dungeon.is_file() and
            md5(graphics.read_bytes()).hexdigest() == graphics_hash and
            md5(dungeon.read_bytes()).hexdigest() == dungeon_hash):
        pairs.append(graphics.parent)
if len(pairs) != 1:
    raise SystemExit(f"FAIL: expected one authentic PC-34 pair, found {pairs}")
print(pairs[0])
PY
)
archive_source_dir=$(cd "$(dirname "$archive")" && pwd)
for sibling_archive in "$archive_source_dir"/*.zip; do
    [[ -f "$sibling_archive" ]] || continue
    ln -s "$sibling_archive" \
        "$mixed_media_data/$(basename "$sibling_archive")"
done
mixed_media_output="$(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR
    HOME="$mixed_media_home" XDG_CONFIG_HOME="$mixed_media_home/.config" \
    APPDATA="$mixed_media_home" \
    FIRESTAFF_CONFIG_PATH="$mixed_media_home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$mixed_media_probe" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --game dm1 --platform pc --debug --script "$menu_original" \
        --boot-probe --boot-probe-frames 120 --duration 0 2>&1
)" || {
    printf '%s\n' "$mixed_media_output" >&2
    exit 1
}
if ! grep -Fq \
        "dm1 platform=PC edition=pc34-en matched source=$mixed_media_pc34_data/GRAPHICS.DAT" \
        <<< "$mixed_media_output" ||
   ! grep -Fq \
        "dungeonPath=$mixed_media_pc34_data/DUNGEON.DAT" \
        <<< "$mixed_media_output" ||
   ! grep -Fq "dataDir=$mixed_media_pc34_data" <<< "$mixed_media_output" ||
   ! grep -Fq 'handoff=pc-img3' <<< "$mixed_media_output" ||
   ! grep -Fq 'phase=dm1-runtime' <<< "$mixed_media_output" ||
   ! grep -Fq 'levelLoaded=1' <<< "$mixed_media_output"; then
    printf '%s\n' "$mixed_media_output" >&2
    echo 'FAIL: mixed installed DM1 media did not bind PC-34 English graphics to its matching DUNGEON.DAT' >&2
    exit 1
fi
python3 - "$mixed_media_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
if (probe["sourceId"] != "dm1" or startup["phase"] != "dm1-runtime" or
        startup["levelLoaded"] != 1):
    raise SystemExit(f"FAIL: issue #13 mixed-media route did not load PC-34: {probe}")
print("PASS: issue #13 mixed installed data root selects the matching PC-34 DUNGEON.DAT")
PY
rm -rf "$mixed_media_home"

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
pair_only_data_root="$missing_swsh_root/pc34-pair"
mkdir -p "$pair_only_data_root"
title_path=$(python3 - "$missing_swsh_root/data" TITLE <<'PY'
from pathlib import Path
import sys

root = Path(sys.argv[1])
matches = [path for path in root.rglob("*")
           if path.is_file() and path.name.casefold() == sys.argv[2].casefold()]
if len(matches) != 1:
    raise SystemExit(f"FAIL: expected one authentic {sys.argv[2]} asset; found {matches}")
print(matches[0])
PY
)
selector_path=$(python3 - "$missing_swsh_root/data" SELECTOR <<'PY'
from pathlib import Path
import sys

root = Path(sys.argv[1])
matches = [path for path in root.rglob("*")
           if path.is_file() and path.name.casefold() == sys.argv[2].casefold()]
if len(matches) != 1:
    raise SystemExit(f"FAIL: expected one authentic {sys.argv[2]} asset; found {matches}")
print(matches[0])
PY
)
cp "$runtime_data_root/GRAPHICS.DAT" "$runtime_data_root/DUNGEON.DAT" \
   "$title_path" "$selector_path" \
   "$pair_only_data_root/"
missing_swsh_output="$(FIRESTAFF_AUTOTEST=1 HOME="$missing_swsh_home" XDG_CONFIG_HOME="$missing_swsh_home" \
   APPDATA="$missing_swsh_home" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
   SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
   --game dm1 --platform pc --data-dir "$pair_only_data_root" \
   --debug --verbose --duration 12000 \
   2>&1)" || {
    printf '%s\n' "$missing_swsh_output" >&2
    echo "FAIL: DM1 did not continue to the dungeon runtime without optional SWSH" >&2
    exit 1
}
if ! grep -Fq "dm1 platform=PC edition=pc34-en matched source=$pair_only_data_root/GRAPHICS.DAT" \
       <<<"$missing_swsh_output" ||
   ! grep -Fq 'startup-source-skip game=dm1 phase=swsh-intro reason=not-found' \
       <<<"$missing_swsh_output" ||
   ! grep -Fq 'startup-source-complete game=dm1 platform=pc34 phase=anim-title frames=53' \
       <<<"$missing_swsh_output" ||
   ! grep -Fq 'startup-source-complete game=dm1 platform=pc34 phase=selector' \
       <<<"$missing_swsh_output" ||
   ! grep -Fq 'DM1 READY: gameId=dm1' <<<"$missing_swsh_output" ||
   ! grep -Fq 'launch phase=runtime-entered game=dm1' \
       <<<"$missing_swsh_output"; then
    printf '%s\n' "$missing_swsh_output" >&2
    echo "FAIL: absent optional SWSH prevented the authentic PC 3.4 dungeon from reaching runtime" >&2
    exit 1
fi
echo "PASS: DM1 skips absent optional SWSH, runs I34E title and selector, then reaches PC 3.4 runtime"
repeat_title_output="$(FIRESTAFF_AUTOTEST=1 \
   FIRESTAFF_AUTOTEST_DM1_TITLE_KEY_AFTER_FRAME=1 \
   HOME="$missing_swsh_home" XDG_CONFIG_HOME="$missing_swsh_home" \
   APPDATA="$missing_swsh_home" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
   SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
   --game dm1 --platform pc --data-dir "$pair_only_data_root" \
   --debug --duration 12000 2>&1)" || {
    printf '%s\n' "$repeat_title_output" >&2
    echo "FAIL: DM1 startup key regression process failed" >&2
    exit 1
}
if ! grep -Fq \
       'AUTOTEST: DM1 I34E title key injected after first rendered frame' \
       <<<"$repeat_title_output" ||
   ! grep -Fq \
       'startup-source-abort game=dm1 platform=pc34 phase=anim-title' \
       <<<"$repeat_title_output" ||
   grep -Fq \
       'startup-source-complete game=dm1 platform=pc34 phase=anim-title frames=53' \
       <<<"$repeat_title_output"; then
    printf '%s\n' "$repeat_title_output" >&2
    echo "FAIL: DM1 title input did not follow the authentic BR interrupt flag" >&2
    exit 1
fi
echo "PASS: PC34 title animation accepts keydown because its authentic BR record allows interruption"
mouse_title_output="$(FIRESTAFF_AUTOTEST=1 \
   FIRESTAFF_AUTOTEST_DM1_TITLE_MOUSE_AFTER_FRAME=1 \
   HOME="$missing_swsh_home" XDG_CONFIG_HOME="$missing_swsh_home" \
   APPDATA="$missing_swsh_home" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
   SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
   --game dm1 --platform pc --data-dir "$pair_only_data_root" \
   --debug --duration 12000 2>&1)" || {
    printf '%s\n' "$mouse_title_output" >&2
    echo "FAIL: DM1 startup mouse regression process failed" >&2
    exit 1
}
if ! grep -Fq \
       'AUTOTEST: DM1 I34E title mouse click injected after first rendered frame' \
       <<<"$mouse_title_output" ||
   ! grep -Fq \
       'startup-input-ignore game=dm1 phase=anim-title reason=source-keyboard-only button=1 break-allowed=1 frame=1' \
       <<<"$mouse_title_output" ||
   ! grep -Fq \
       'startup-source-complete game=dm1 platform=pc34 phase=anim-title frames=53' \
       <<<"$mouse_title_output" ||
   grep -Fq \
       'startup-source-abort game=dm1 platform=pc34 phase=anim-title' \
       <<<"$mouse_title_output"; then
    printf '%s\n' "$mouse_title_output" >&2
    echo "FAIL: PC34 title mouse click incorrectly interrupted keyboard-only ANIM input" >&2
    exit 1
fi
echo "PASS: PC34 title ignores mouse clicks even when authentic BR permits keyboard interruption"
repeat_selector_output="$(FIRESTAFF_AUTOTEST_SELECTOR_INPUT=key:return-repeat \
   HOME="$missing_swsh_home" XDG_CONFIG_HOME="$missing_swsh_home" \
   APPDATA="$missing_swsh_home" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
   SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
   --game dm1 --platform pc --data-dir "$pair_only_data_root" \
   --debug --duration 12000 2>&1)" || {
    printf '%s\n' "$repeat_selector_output" >&2
    echo "FAIL: repeat-key selector regression process failed" >&2
    exit 1
}
if ! grep -Fq \
       'startup-input-ignore game=dm1 phase=selector reason=key-repeat' \
       <<<"$repeat_selector_output" ||
   grep -Fq 'startup-source-input game=dm1 platform=pc34 phase=selector command=enter' \
       <<<"$repeat_selector_output"; then
    printf '%s\n' "$repeat_selector_output" >&2
    echo "FAIL: keyboard autorepeat incorrectly opened the DM1 selector entrance" >&2
    exit 1
fi
echo "PASS: PC34 selector entrance ignores keyboard autorepeat and waits for a fresh command"
resume_status=0
resume_output="$(FIRESTAFF_AUTOTEST_SELECTOR_INPUT=key:r \
   HOME="$missing_swsh_home" XDG_CONFIG_HOME="$missing_swsh_home" \
   APPDATA="$missing_swsh_home" FIRESTAFF_CONFIG_PATH="$missing_swsh_home/config.toml" \
   SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
   --game dm1 --platform pc --data-dir "$archive" \
   --save "$missing_swsh_root/no-save.DAT" --debug --duration 12000 2>&1)" || resume_status=$?
if ! grep -Fq \
       'startup-source-input game=dm1 platform=pc34 phase=selector command=resume' \
       <<<"$resume_output" ||
   grep -Fq \
       'startup-source-input game=dm1 platform=pc34 phase=selector command=enter' \
       <<<"$resume_output"; then
    printf '%s\n' "$resume_output" >&2
    echo "FAIL: PC34 selector Resume did not choose Firestaff's DM1 Quick Resume route (exit $resume_status)" >&2
    exit 1
fi
echo "PASS: PC34 selector R selects the existing DM1 Quick Resume launch intent"
resume_click_status=0
resume_click_output="$(FIRESTAFF_AUTOTEST_SELECTOR_INPUT=click:resume \
   HOME="$missing_swsh_home" XDG_CONFIG_HOME="$missing_swsh_home" \
   APPDATA="$missing_swsh_home" FIRESTAFF_CONFIG_PATH="$missing_swsh_home/config.toml" \
   SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
   --game dm1 --platform pc --data-dir "$archive" \
   --save "$missing_swsh_root/no-save.DAT" --debug --duration 12000 2>&1)" || resume_click_status=$?
if ! grep -Fq \
       'startup-source-input game=dm1 platform=pc34 phase=selector command=resume' \
       <<<"$resume_click_output" ||
   grep -Fq \
       'startup-source-input game=dm1 platform=pc34 phase=selector command=enter' \
       <<<"$resume_click_output"; then
    printf '%s\n' "$resume_click_output" >&2
    echo "FAIL: PC34 selector Resume hitbox did not choose Quick Resume (exit $resume_click_status)" >&2
    exit 1
fi
echo "PASS: PC34 selector Resume hitbox selects the existing DM1 Quick Resume launch intent"

# Real PC34 saves are operator-staged outside the repository. When available,
# verify both the direct CLI `--save` contract and the selector's Resume input
# against the same original bytes; never manufacture a save to satisfy this
# integration route.
authentic_pc34_save=${FIRESTAFF_DM1_PC34_AUTHENTIC_SAVE:-}
if [[ -n "$authentic_pc34_save" ]]; then
    if [[ ! -f "$authentic_pc34_save" ]]; then
        printf 'FAIL: configured original PC34 save is missing: %s\n' \
            "$authentic_pc34_save" >&2
        exit 1
    fi
    authentic_pc34_save_hash=$(shasum -a 256 "$authentic_pc34_save")
    direct_save_probe="$test_scratch/dm1-pc34-authentic-save-direct-$$.json"
    direct_save_output=$(FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$direct_save_probe" \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --game dm1 --platform pc --data-dir "$archive" \
        --save "$authentic_pc34_save" --debug --boot-probe \
        --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
        --duration 0 2>&1) || {
            printf '%s\n' "$direct_save_output" >&2
            exit 1
        }
    if ! grep -Fq "RESUME: loaded save from $authentic_pc34_save" \
            <<<"$direct_save_output"; then
        printf '%s\n' "$direct_save_output" >&2
        echo 'FAIL: direct DM1 --save did not load the original PC34 save' >&2
        exit 1
    fi
    selector_save_probe="$test_scratch/dm1-pc34-authentic-save-selector-$$.json"
    selector_save_output=$(FIRESTAFF_AUTOTEST_SELECTOR_INPUT=key:r \
        FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$selector_save_probe" \
        HOME="$missing_swsh_home" XDG_CONFIG_HOME="$missing_swsh_home" \
        APPDATA="$missing_swsh_home" \
        FIRESTAFF_CONFIG_PATH="$missing_swsh_home/config.toml" \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --game dm1 --platform pc --data-dir "$archive" \
        --save "$authentic_pc34_save" --debug --duration 15000 2>&1) || {
            printf '%s\n' "$selector_save_output" >&2
            exit 1
        }
    if ! grep -Fq \
            'startup-source-input game=dm1 platform=pc34 phase=selector command=resume' \
            <<<"$selector_save_output" ||
       ! grep -Fq "RESUME: loaded save from $authentic_pc34_save" \
            <<<"$selector_save_output"; then
        printf '%s\n' "$selector_save_output" >&2
        echo 'FAIL: PC34 selector Resume did not restore the original save' >&2
        exit 1
    fi
    python3 - "$direct_save_probe" "$selector_save_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    direct = json.load(probe_file)
with open(sys.argv[2], encoding="utf-8") as probe_file:
    selector = json.load(probe_file)
for name, probe in (("direct", direct), ("selector", selector)):
    startup = probe["startup"]
    if (probe["launchedEver"] != 1 or probe["sourceId"] != "dm1" or
            startup["phase"] != "dm1-runtime" or startup["levelLoaded"] != 1):
        raise SystemExit(f"FAIL: authentic PC34 {name} Resume missed runtime: {probe}")
direct_party = direct["party"]
selector_party = selector["party"]
for key in ("mapIndex", "mapX", "mapY", "direction", "championCount"):
    if direct_party.get(key) != selector_party.get(key):
        raise SystemExit(
            "FAIL: direct and selector Resume restored different original PC34 "
            f"pose at {key}: direct={direct_party} selector={selector_party}"
        )
# The selector takes time to animate before the caller enters F0435, and the
# live timer advances after restoration. Compare a bounded interval rather
# than requiring an impossible identical observation tick.
if not (direct["gameTick"] <= selector["gameTick"] <= direct["gameTick"] + 120):
    raise SystemExit(
        "FAIL: selector Resume did not retain the direct original PC34 save tick "
        f"within startup elapsed time: direct={direct['gameTick']} "
        f"selector={selector['gameTick']}"
    )
print("PASS: direct CLI and I34E selector Resume restore the same original PC34 pose")
PY
    rm -f "$direct_save_probe" "$selector_save_probe"
    if [[ "$authentic_pc34_save_hash" != "$(shasum -a 256 "$authentic_pc34_save")" ]]; then
        echo 'FAIL: original PC34 save changed during Resume verification' >&2
        exit 1
    fi
fi

credits_status=0
credits_output="$(FIRESTAFF_AUTOTEST_SELECTOR_INPUT=credits-then-enter \
   HOME="$missing_swsh_home" XDG_CONFIG_HOME="$missing_swsh_home" \
   APPDATA="$missing_swsh_home" FIRESTAFF_CONFIG_PATH="$missing_swsh_home/config.toml" \
   SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
   --game dm1 --platform pc --data-dir "$archive" --debug --duration 12000 2>&1)" || credits_status=$?
if ! grep -Fq \
       'startup-source-input game=dm1 platform=pc34 phase=selector command=credits-return' \
       <<<"$credits_output" ||
   ! grep -Fq \
       'startup-source-input game=dm1 platform=pc34 phase=selector command=enter' \
       <<<"$credits_output"; then
    printf '%s\n' "$credits_output" >&2
    echo "FAIL: PC34 selector Credits did not return to the closed selector before Enter (exit $credits_status)" >&2
    exit 1
fi
echo "PASS: PC34 selector Credits shows the original page, returns to the selector and then enters"
for selector_quit in key:q key:escape click:quit; do
    selector_quit_output="$(FIRESTAFF_AUTOTEST_SELECTOR_INPUT="$selector_quit" \
       HOME="$missing_swsh_home" XDG_CONFIG_HOME="$missing_swsh_home" \
       APPDATA="$missing_swsh_home" FIRESTAFF_CONFIG_PATH="$missing_swsh_home/config.toml" \
       SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
       --game dm1 --platform pc --data-dir "$archive" \
       --debug --duration 12000 2>&1)" || {
        printf '%s\n' "$selector_quit_output" >&2
        echo "FAIL: PC34 selector Quit route failed for $selector_quit" >&2
        exit 1
    }
    if ! grep -Fq \
           "startup-source-input game=dm1 platform=pc34 phase=selector command=quit" \
           <<<"$selector_quit_output" ||
       grep -Fq 'startup-source-input game=dm1 platform=pc34 phase=selector command=enter' \
           <<<"$selector_quit_output" ||
       grep -Fq 'launch phase=runtime-entered game=dm1' <<<"$selector_quit_output"; then
        printf '%s\n' "$selector_quit_output" >&2
        echo "FAIL: PC34 selector Quit did not exit before dungeon runtime for $selector_quit" >&2
        exit 1
    fi
    echo "PASS: authentic PC34 selector Quit route exits before runtime via $selector_quit"
done
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

# The public --game CLI boot probe checks edition resolution and a loaded
# runtime. Boot-probe mode intentionally skips the visible I34E animation,
# which is verified by the non-probe startup checks above.
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
        startup["levelLoaded"] != 1):
    raise SystemExit(f"FAIL: DM1 --game CLI boot probe missed the loaded runtime: {probe}")
print("PASS: DM1 --game CLI boot probe reaches the loaded runtime")
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
    --game dm1 --data-dir "$archive" --debug --verbose --boot-probe \
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
        startup["levelLoaded"] != 1):
    raise SystemExit(f"FAIL: bare DM1 CLI missed its PC-34 runtime: {probe}")
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
    --debug --menu --game dm1 --data-dir "$menu_auto_data_root" \
    --script "$menu_original" --duration 30000 2>&1)" || {
    printf '%s\n' "$menu_auto_output" >&2
    exit 1
}
if ! grep -Fq 'handoff=pc-img3' <<<"$menu_auto_output" ||
   ! grep -Fq 'startup-source-complete game=dm1 platform=pc34 phase=anim-title frames=53' \
       <<<"$menu_auto_output" ||
   ! grep -Fq 'startup-source-complete game=dm1 platform=pc34 phase=selector' \
       <<<"$menu_auto_output"; then
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
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 3, 2, 0)):
    raise SystemExit(f"FAIL: authentic DM1 AUTO menu did not reach the PC-34 runtime: {probe}")
print("PASS: clean-config DM1 AUTO menu ran I34E ANIM/TITLE and SELECTOR Enter before runtime")
PY
python3 - "$startup_capture_root" "$menu_auto_output" <<'PY'
from pathlib import Path
import hashlib
import struct
import sys

root = Path(sys.argv[1])
log = sys.argv[2]
markers = ("phase=anim-title-erase pixels=64000", "phase=anim-title frames=53",
           "startup-source-held game=dm1 platform=pc34 phase=anim-title flags=-ah,-as terminal-erase=0",
           "phase=selector graphics=")
positions = [log.find(marker) for marker in markers]
if any(position < 0 for position in positions) or positions != sorted(positions):
    raise SystemExit(f"FAIL: original -ah -as startup phase order differs: {positions}")
frames = {}
geometry = None
for phase in ("i34e-title-first", "i34e-title-mid", "i34e-title-final",
              "i34e-title-held",
              "i34e-selector-closed", "i34e-selector-door-step-1",
              "i34e-selector-door-mid", "i34e-selector-door-open"):
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
if (frames["i34e-title-first"] == frames["i34e-title-mid"] or
        frames["i34e-title-mid"] == frames["i34e-title-final"]):
    raise SystemExit("FAIL: authentic I34E ANIM/TITLE frames did not change")
print("PASS: authentic I34E ANIM/TITLE first, middle and final frames differ")
if frames["i34e-title-final"] != frames["i34e-title-held"]:
    raise SystemExit("FAIL: -ah -as modified the final TITLE pixels/palette before SELECTOR")
print("PASS: -ah -as retained the actual final TITLE presentation until SELECTOR")
if (frames["i34e-selector-closed"] != frames["i34e-selector-door-step-1"] or
        frames["i34e-selector-door-step-1"] == frames["i34e-selector-door-mid"] or
        frames["i34e-selector-door-mid"] == frames["i34e-selector-door-open"]):
    raise SystemExit("FAIL: authentic I34E selector door animation did not change")
print("PASS: authentic I34E selector preserved its closed first step and opened progressively")
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
movement_home="$test_scratch/dm1-pc34-movement-home-$$"
mkdir -p "$movement_home"
gameplay_output=$(HOME="$movement_home" XDG_CONFIG_HOME="$movement_home" \
    APPDATA="$movement_home" FIRESTAFF_CONFIG_PATH="$movement_home/config.toml" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform pc --data-dir "$archive" --debug \
    --boot-probe --boot-probe-frames 0 \
    --script up --duration 0 2>&1) || {
    printf '%s\n' "$gameplay_output" >&2
    exit 1
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$gameplay_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$gameplay_output" ||
   ! grep -Fq 'map=0 party=1,4,2' <<<"$gameplay_output" ||
   ! grep -Fq 'game-action game=dm1 tick=1 input=forward result=1' <<<"$gameplay_output" ||
   ! grep -Fq 'action="FRAMÅT" outcome="GRUPP FLYTTAD" party=1,4' <<<"$gameplay_output"; then
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
    output=$(HOME="$movement_home" XDG_CONFIG_HOME="$movement_home" \
        APPDATA="$movement_home" FIRESTAFF_CONFIG_PATH="$movement_home/config.toml" \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --game dm1 --platform pc --data-dir "$archive" \
        --boot-probe \
        --boot-probe-frames 0 --script "$input" --duration 0 2>&1) || {
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
mouse_move_output=$(HOME="$movement_home" XDG_CONFIG_HOME="$movement_home" \
    APPDATA="$movement_home" FIRESTAFF_CONFIG_PATH="$movement_home/config.toml" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform pc --data-dir "$archive" --width 320 --height 200 \
    --boot-probe --boot-probe-frames 0 \
    --script 'click:275:135' \
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
mouse_hoc_output=$(HOME="$movement_home" XDG_CONFIG_HOME="$movement_home" \
    APPDATA="$movement_home" FIRESTAFF_CONFIG_PATH="$movement_home/config.toml" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform pc --data-dir "$archive" --width 320 --height 200 \
    --boot-probe --boot-probe-frames 0 \
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
hoc_output=$(HOME="$movement_home" XDG_CONFIG_HOME="$movement_home" \
    APPDATA="$movement_home" FIRESTAFF_CONFIG_PATH="$movement_home/config.toml" \
    FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$hoc_capture_dir" \
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
hoc_scaled_output=$(HOME="$movement_home" XDG_CONFIG_HOME="$movement_home" \
    APPDATA="$movement_home" FIRESTAFF_CONFIG_PATH="$movement_home/config.toml" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
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
hoc_modern_output=$(HOME="$movement_home" XDG_CONFIG_HOME="$movement_home" \
    APPDATA="$movement_home" FIRESTAFF_CONFIG_PATH="$movement_home/config.toml" \
    FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$hoc_modern_capture_dir" \
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
hoc_confirm_output=$(HOME="$movement_home" XDG_CONFIG_HOME="$movement_home" \
    APPDATA="$movement_home" FIRESTAFF_CONFIG_PATH="$movement_home/config.toml" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
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
hoc_source_coordinate_output=$(HOME="$movement_home" XDG_CONFIG_HOME="$movement_home" \
    APPDATA="$movement_home" FIRESTAFF_CONFIG_PATH="$movement_home/config.toml" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
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
hoc_inventory_output=$(HOME="$movement_home" XDG_CONFIG_HOME="$movement_home" \
    APPDATA="$movement_home" FIRESTAFF_CONFIG_PATH="$movement_home/config.toml" \
    FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$hoc_inventory_capture_dir" \
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

# Exercise the same SDL event-queue route used by an ordinary desktop mouse.
# The C-level HoC test above proves source geometry, while this direct CLI run
# proves event polling, window-to-source mapping, and consecutive champion
# changes after two original C127 mirrors have been recruited. SELECTOR.C is
# synchronously waiting before the main loop can dispatch scripted events, so
# use its narrow test hook for the one Enter that leaves the selector; all
# later movement and clicks go through SDL_PushEvent and the normal event pump.
native_hoc_keys='key:kp5,key:kp5,key:kp5,key:kp5,key:kp5,key:kp1,key:kp1,key:kp1,key:kp2,key:kp2,key:kp2,key:kp2,key:kp2,key:kp1,key:kp1,key:kp5,key:kp1,key:kp1,key:kp5,key:kp1,key:kp1,key:kp1,key:kp1,key:kp1,key:kp2,key:kp1,key:kp6,key:kp6,click:112:83'
native_hoc_route=$(python3 -c 'import sys; tokens=sys.argv[1].split(","); print(",".join(token+(",wait:30" if token.startswith("key:") else ",wait:5" if token.startswith("click:") else "") for token in tokens))' "$native_hoc_keys")
native_hoc_route+=',click:130:115,wait:5,key:kp2,wait:30,key:kp1,wait:30,key:kp1,wait:30,key:kp1,wait:30,key:kp1,wait:30,click:112:82,wait:5,click:130:115,wait:5,click:80:3,wait:5,click:10:3,wait:5'
native_hoc_dispatch_log="$test_scratch/dm1-pc34-native-hoc-dispatch-$$.log"
native_hoc_home="$test_scratch/dm1-pc34-native-hoc-home-$$"
mkdir -p "$native_hoc_home"
native_hoc_dispatch_output=$(HOME="$native_hoc_home" \
    XDG_CONFIG_HOME="$native_hoc_home" APPDATA="$native_hoc_home" \
    FIRESTAFF_CONFIG_PATH="$native_hoc_home/config.toml" \
    FIRESTAFF_AUTOTEST_SELECTOR_INPUT=key:return \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --presentation-mode v1 --width 320 --height 200 \
    --game dm1 --platform pc --data-dir "$archive" --debug \
    --script "wait-game-runtime,$native_hoc_route" --duration 30000 \
    2>&1) || {
        printf '%s\n' "$native_hoc_dispatch_output" >&2
        exit 1
    }
printf '%s\n' "$native_hoc_dispatch_output" > "$native_hoc_dispatch_log"
if ! grep -Fq \
    'dm1-pointer event=down-left window=80,3 source=80,3 button-mask=0x2 active-champion=0->1 champions=2 result=1' \
    "$native_hoc_dispatch_log" ||
   ! grep -Fq \
    'dm1-pointer event=down-left window=10,3 source=10,3 button-mask=0x2 active-champion=1->0 champions=2 result=1' \
    "$native_hoc_dispatch_log" ||
   ! grep -Fq 'game-action game=dm1 tick=' "$native_hoc_dispatch_log" ||
   ! grep -Fq 'input=pointer-left' "$native_hoc_dispatch_log" ||
   ! grep -Fq 'game-health game=dm1 tick=' "$native_hoc_dispatch_log" ||
   ! grep -Fq 'target=champion' "$native_hoc_dispatch_log"; then
    printf '%s\n' "$native_hoc_dispatch_output" >&2
    printf '%s\n' 'FAIL: normal DM1 PC SDL debug loop missed champion switching, pointer actions, or source health changes' >&2
    exit 1
fi
echo 'PASS: authentic PC-34 SDL debug loop logs pointer actions, champion switching, and source health changes'

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
    FIRESTAFF_AUTOTEST=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform pc --data-dir "$archive" --debug-full-redraw \
    --duration 30000 \
    2>&1) || {
        printf '%s\n' "$perf_output" >&2
        exit 1
    }
if ! grep -Eq 'firestaff: perf game=dm1 active=1 .*process-cpu-of-one-core-percent=([0-9]+\.[0-9]+|unavailable)' \
        <<< "$perf_output" ||
   ! grep -Eq 'process-rss-bytes=[1-9][0-9]*' <<< "$perf_output" ||
   ! grep -Eq 'process-virtual-bytes=[1-9][0-9]*' <<< "$perf_output" ||
   ! grep -Fq 'game-frames-rendered-per-sec=' <<< "$perf_output" ||
   ! grep -Fq 'target-loop-fps=' <<< "$perf_output" ||
   ! grep -Fq 'debug-full-redraw=1' <<< "$perf_output" ||
   ! grep -Fq 'DM1 READY: gameId=dm1' <<< "$perf_output" ||
   ! grep -Fq 'runtime game=dm1 startup-phase=dm1-runtime' <<< "$perf_output" ||
   ! grep -Fq 'frame-compose-present-call-ms-max=' <<< "$perf_output" ||
   ! grep -Fq 'gpu-utilization=unavailable' <<< "$perf_output"; then
    printf '%s\n' "$perf_output" >&2
    printf '%s\n' 'FAIL: --debug-full-redraw did not emit complete DM1 performance telemetry' >&2
    exit 1
fi
# Actual source Draw calls must accompany forced presentations. A repeated
# retained framebuffer is not evidence of full-render throughput.
python3 - "$perf_output" <<'PYDRAWS'
import re
import sys
samples = [line for line in sys.argv[1].splitlines()
           if "firestaff: perf game=dm1 active=1 " in line]
if not samples:
    raise SystemExit("FAIL: no active original-media Draw sample")
line = samples[-1]
draw = re.search(r"source-draw-count=(\d+)", line)
present = re.search(r"presented-frame-count=(\d+)", line)
render = re.search(r"game-frames-rendered-per-sec=([0-9.]+)", line)
ticks = re.search(r"game-ticks-per-sec=([0-9.]+)", line)
if (not draw or not present or not render or not ticks or
        int(present[1]) <= 0 or int(draw[1]) < int(present[1]) or
        float(render[1]) <= float(ticks[1])):
    raise SystemExit("FAIL: forced presentations do not contain actual per-frame source Draw calls: " + line)
print("PASS: original full-redraw sample counts actual source Draw calls separately from presents")
PYDRAWS
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
