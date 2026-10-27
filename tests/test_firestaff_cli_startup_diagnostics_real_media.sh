#!/bin/sh
set -eu

firestaff_cli=${1:?Firestaff executable is required}
mac_archive=${FIRESTAFF_DM2_MAC_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_Mac_EN (1).zip"}
towns_archive=${FIRESTAFF_DM2_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip"}
dos_archive=${FIRESTAFF_DM2_DOS_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_DOS_EN.zip"}
amiga_archive=${FIRESTAFF_DM2_AMIGA_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_Amiga_EN.zip"}
dm1_towns_archive=${FIRESTAFF_DM1_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_FM-Towns_JA-EN.zip"}
dm1_pc_archive=${FIRESTAFF_DM1_PC_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_EN.zip"}
dm1_pc98_archive=${FIRESTAFF_DM1_PC98_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_PC-98_EN.zip"}
csb_towns_archive=${FIRESTAFF_CSB_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip"}
csb_amiga_archive=${FIRESTAFF_CSB_AMIGA_ARCHIVE:-"$HOME/.firestaff/data/csb/Dungeon-Master-Chaos-Strikes-Back---Expansion-Set-1_Amiga_EN.zip"}
csb_atari_archive=${FIRESTAFF_CSB_ATARI_ARCHIVE:-"$HOME/.firestaff/data/csb/chaos_strikes_back_ftl.zip"}
csb_towns_loose_root=${FIRESTAFF_CSB_FMTOWNS_LOOSE_ROOT:-"$HOME/.firestaff/data/csb/fmtowns_iso"}
csb_amiga33_loose_root=${FIRESTAFF_CSB_AMIGA33_LOOSE_ROOT:-"$HOME/.firestaff/data/csb/amiga33-original"}
nexus_iso=${FIRESTAFF_NEXUS_ISO:-"$HOME/.firestaff/data/nexus/Dungeon Master Nexus (English).iso"}
theron_track02=${FIRESTAFF_THERON_TRACK02:-"$HOME/.firestaff/data/theron/TQUS02.bin"}

if [ ! -x "$firestaff_cli" ] || [ ! -f "$towns_archive" ]; then
    echo "SKIP: Firestaff executable and original DM2 FM Towns archive are required"
    exit 77
fi

if [ -f "$mac_archive" ]; then
    mac_debug=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game dm2 --platform auto --data-dir "$mac_archive" \
        --debug --boot-probe --boot-probe-frames 0 2>&1) || {
        printf '%s\n' "$mac_debug" >&2
        exit 1
    }
    if ! printf '%s\n' "$mac_debug" | \
            grep -F 'task=search root' | grep -F "path=$mac_archive" >/dev/null; then
        echo "FAIL: Mac debug trace omitted the searched archive path on its task record" >&2
        exit 1
    fi
    case "$mac_debug" in
        *"candidate-file=GRAPHICS.DAT"*) ;;
        *) echo "FAIL: Mac debug trace omitted the catalogued graphics candidate" >&2; exit 1 ;;
    esac
    case "$mac_debug" in
        *"launch phase=game-handoff mode=direct game=dm2 platform=Macintosh edition=mac-en-retail source="*) ;;
        *) echo "FAIL: Mac debug trace omitted the selected edition" >&2; exit 1 ;;
    esac
else
    echo 'SKIP: DM2 Macintosh debug diagnostics (authentic Mac archive unavailable)'
fi

towns_verbose=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game dm2 --platform auto --data-dir "$towns_archive" \
    --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
    printf '%s\n' "$towns_verbose" >&2
    exit 1
}
case "$towns_verbose" in
    *"selected game=dm2 platform=FM Towns edition=fmtowns-ja source="*) ;;
    *) echo "FAIL: FM Towns verbose trace omitted selected edition" >&2; exit 1 ;;
esac
case "$towns_verbose" in
    *"candidate-file="*"edition-result="*) ;;
    *) echo "FAIL: verbose omitted catalogued candidate filenames" >&2; exit 1 ;;
esac

# --debug must expose every original FM Towns TWANIM frame on the ordinary
# no-platform direct CLI route, including source completion time. Boot probes
# fast-forward source waits and cannot verify this timing contract.
towns_debug=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game dm2 --data-dir "$towns_archive" --debug --duration 40000 2>&1) || {
    printf '%s\n' "$towns_debug" >&2
    exit 1
}
printf '%s\n' "$towns_debug" | python3 -c '
import re
import sys

trace = sys.stdin.read()
frames = set()
finished = []
rejected = False
pattern = re.compile(
    r"startup-frame game=dm2 elapsed-ms=(\d+).*?"
    r"dm2-fmtowns-title=\{bound:(\d+) finished:(\d+) "
    r"rejected:(\d+) swoosh:(\d+) frame:(\d+)/(\d+) ticks-remaining:\d+\}"
)
for match in pattern.finditer(trace):
    elapsed, bound, done, reject, swoosh, frame, maximum = map(int, match.groups())
    rejected |= reject != 0
    if bound and not done and not swoosh and maximum == 225 and 0 < frame < 225:
        frames.add(frame)
    if done and not reject:
        finished.append((int(elapsed), int(frame), int(maximum)))
if (frames != set(range(1, 225)) or not finished or
        finished[-1][1:] != (225, 0) or
        not 24000 <= finished[-1][0] < 40000 or rejected or
        "phase=dm2-startup-menu" not in trace):
    raise SystemExit(
        "FAIL: DM2 --debug did not record every timed TWANIM frame and source completion timing"
    )
print(
    f"PASS: DM2 --debug recorded all timed TWANIM frames and finished in "
    f"{finished[-1][0]} ms"
)
' || {
    printf '%s\n' "$towns_debug" >&2
    exit 1
}

combined_root=$(mktemp -d "${PWD}/.firestaff-cli-diagnostics.XXXXXX")
csb_mixed_root=
csb_combined_root=
dm1_combined_root=
default_home=
uppercase_game_root=
offline_home=
cache_replacement_root=
trap 'rm -rf "$combined_root" "$csb_mixed_root" "$csb_combined_root" "$dm1_combined_root" "$default_home" "$uppercase_game_root" "$offline_home" "$cache_replacement_root"' EXIT

# A cached physical-file hash must not survive replacing an authentic game
# file with another original edition of the same size while preserving its
# modification timestamp. CSB's Towns and Amiga 3.3 DUNGEON.DAT files are both
# 2098 bytes but have distinct known hashes. Keep the source archives pristine
# and put the two original files at one temporary path to exercise the real
# persistent scanner cache.
csb_towns_graphics="$csb_towns_loose_root/CDATA/GRAPHICS.DAT"
csb_towns_dungeon="$csb_towns_loose_root/CDATA/DUNGEON.DAT"
csb_amiga33_dungeon="$csb_amiga33_loose_root/Dungeon.DAT"
if [ -f "$csb_towns_graphics" ] && [ -f "$csb_towns_dungeon" ] &&
   [ -f "$csb_amiga33_dungeon" ] &&
   [ "$(wc -c < "$csb_towns_dungeon" | tr -d ' ')" = \
     "$(wc -c < "$csb_amiga33_dungeon" | tr -d ' ')" ]; then
    cache_replacement_root=$(mktemp -d "${PWD}/.firestaff-cache-replacement.XXXXXX")
    cache_replacement_data="$cache_replacement_root/data"
    cache_replacement_home="$cache_replacement_root/home"
    mkdir -p "$cache_replacement_data" "$cache_replacement_home"
    cp "$csb_towns_graphics" "$cache_replacement_data/GRAPHICS.DAT"
    cp "$csb_towns_dungeon" "$cache_replacement_data/DUNGEON.DAT"
    replacement_scan=$(HOME="$cache_replacement_home" \
        FIRESTAFF_CONFIG_PATH="$cache_replacement_home/config.toml" \
        "$firestaff_cli" --scan-data --data-dir "$cache_replacement_data" \
        --verbose 2>&1) || {
        printf '%s\n' "$replacement_scan" >&2
        exit 1
    }
    printf '%s\n' "$replacement_scan" | python3 -c '
import re
import sys
trace = sys.stdin.read()
summary = trace.split("Data dir:", 1)[1]
block = summary.split("Chaos Strikes Back", 1)[1].split("Dungeon Master II", 1)[0]
match = re.search(r"DUNGEON\.DAT\s+FOUND[^\n]*\n\s+md5: ([0-9a-f]{32})", block)
if match is None or match.group(1) != "83c56cf1b779e7460a55c9299ebeb04b":
    raise SystemExit("FAIL: authentic CSB FM Towns dungeon hash was not scanned")
'
    replacement_scan=$(HOME="$cache_replacement_home" \
        FIRESTAFF_CONFIG_PATH="$cache_replacement_home/config.toml" \
        "$firestaff_cli" --scan-data --data-dir "$cache_replacement_data" \
        --verbose 2>&1) || {
        printf '%s\n' "$replacement_scan" >&2
        exit 1
    }
    printf '%s\n' "$replacement_scan" | python3 -c '
import re
import sys
trace = sys.stdin.read()
summary = trace.split("Data dir:", 1)[1]
block = summary.split("Chaos Strikes Back", 1)[1].split("Dungeon Master II", 1)[0]
match = re.search(r"DUNGEON\.DAT\s+FOUND[^\n]*\n\s+md5: ([0-9a-f]{32})", block)
if match is None or match.group(1) != "83c56cf1b779e7460a55c9299ebeb04b":
    raise SystemExit("FAIL: authentic CSB hash changed after loading a persisted scan cache")
'
    cat "$csb_amiga33_dungeon" > "$cache_replacement_data/DUNGEON.DAT"
    touch -r "$csb_towns_dungeon" "$cache_replacement_data/DUNGEON.DAT"
    replacement_scan=$(HOME="$cache_replacement_home" \
        FIRESTAFF_CONFIG_PATH="$cache_replacement_home/config.toml" \
        "$firestaff_cli" --scan-data --data-dir "$cache_replacement_data" \
        --verbose 2>&1) || {
        printf '%s\n' "$replacement_scan" >&2
        exit 1
    }
    printf '%s\n' "$replacement_scan" | python3 -c '
import re
import sys
trace = sys.stdin.read()
summary = trace.split("Data dir:", 1)[1]
block = summary.split("Chaos Strikes Back", 1)[1].split("Dungeon Master II", 1)[0]
match = re.search(r"DUNGEON\.DAT\s+FOUND[^\n]*\n\s+md5: ([0-9a-f]{32})", block)
if match is None or match.group(1) != "6695d2acebce49f95db1d8f3a5c733de":
    raise SystemExit(
        "FAIL: scanner reused the old CSB Towns dungeon digest after authentic same-size replacement"
    )
'
    echo 'PASS: authentic CSB scan cache invalidates same-size replacement with preserved mtime'
fi

# Exercise the real default path layout without --data-dir and without
# inheriting a developer's saved config. Stage only links to the original
# archives already selected above; no game data is copied or synthesized.
default_home=$(mktemp -d "${PWD}/.firestaff-default-data-home.XXXXXX")
default_data_root="$default_home/.firestaff/data"
link_default_game_archive() {
    game_id=$1
    archive_path=$2
    if [ -f "$archive_path" ]; then
        mkdir -p "$default_data_root/$game_id"
        ln -s "$archive_path" \
            "$default_data_root/$game_id/$(basename "$archive_path")"
    fi
}
link_default_game_archive dm2 "$mac_archive"
link_default_game_archive dm2 "$towns_archive"
link_default_game_archive dm1 "$dm1_towns_archive"
link_default_game_archive csb "$csb_towns_archive"
link_default_game_archive nexus "$nexus_iso"
link_default_game_archive theron "$theron_track02"

# The default no-game start now overlaps collection discovery with the branded
# intro. Close during the real scan to prove that the window remains responsive
# and that the worker observes cancellation before the launcher can publish
# unauthenticated availability or enter a game.
if [ -f "$dm1_towns_archive" ] && [ -f "$csb_towns_archive" ] &&
   [ -f "$towns_archive" ]; then
    set +e
    intro_scan_output=$(python3 - "$firestaff_cli" "$default_home" \
        "$default_data_root" <<'PYINTRO'
import os, subprocess, sys
app, home, data_root = sys.argv[1:]
env = os.environ.copy()
env.update({
    'HOME': home,
    'USERPROFILE': home,
    'XDG_CONFIG_HOME': os.path.join(home, '.config'),
    'APPDATA': os.path.join(home, 'AppData'),
    'FIRESTAFF_CONFIG_PATH': os.path.join(home, 'intro-scan-config.toml'),
    'FIRESTAFF_DATA': data_root,
    'FIRESTAFF_AUTOTEST': '1',
    'FIRESTAFF_AUTOTEST_QUIT_DURING_SCAN': '1',
    'SDL_VIDEODRIVER': 'dummy',
    'SDL_AUDIODRIVER': 'dummy',
})
try:
    result = subprocess.run([app, '--debug'], env=env, capture_output=True,
                            text=True, timeout=90)
except subprocess.TimeoutExpired as error:
    for chunk in (error.stdout, error.stderr):
        if chunk:
            sys.stdout.write(chunk.decode(errors='replace') if isinstance(chunk, bytes)
                             else chunk)
    sys.exit(124)
sys.stdout.write(result.stdout)
sys.stdout.write(result.stderr)
sys.exit(result.returncode)
PYINTRO
    ) || intro_scan_status=$?
    set -e
    if [ "${intro_scan_status:-0}" -ne 0 ] ||
       ! printf '%s\n' "$intro_scan_output" | \
            grep -Fq 'autotest injected window-close during startup intro asset scan' ||
       ! printf '%s\n' "$intro_scan_output" | \
            grep -Fq 'firestaff: scan elapsed-ms=' ||
       printf '%s\n' "$intro_scan_output" | \
            grep -Fq 'launch phase=game-handoff'; then
        printf 'FAIL: no-game intro scan did not remain cancellable and game-free (status %s)\n%s\n' \
            "${intro_scan_status:-0}" "$intro_scan_output" >&2
        exit 1
    fi
    echo 'PASS: no-game startup scans during the intro and cancels cleanly on window close'
fi

# Make sure a close event is still serviced while the large original DM2 CUE
# image is inflating, not only between archive candidates. The image scan runs
# on the intro worker while the renderer keeps pumping native window events.
if [ -f "$dm1_towns_archive" ] && [ -f "$csb_towns_archive" ] &&
   [ -f "$towns_archive" ] && [ -f "$mac_archive" ]; then
    set +e
    dm2_inflate_close_output=$(python3 - "$firestaff_cli" "$default_home" \
        "$default_data_root" <<'PYDM2INFLATE'
import os, subprocess, sys
app, home, data_root = sys.argv[1:]
env = os.environ.copy()
env.update({
    'HOME': home,
    'USERPROFILE': home,
    'XDG_CONFIG_HOME': os.path.join(home, '.config'),
    'APPDATA': os.path.join(home, 'AppData'),
    'FIRESTAFF_CONFIG_PATH': os.path.join(home, 'dm2-inflate-close.toml'),
    'FIRESTAFF_DATA': data_root,
    'FIRESTAFF_AUTOTEST': '1',
    'FIRESTAFF_AUTOTEST_QUIT_DURING_DM2_IMAGE_INFLATE': '1',
    'SDL_VIDEODRIVER': 'dummy',
    'SDL_AUDIODRIVER': 'dummy',
})
try:
    result = subprocess.run([app, '--debug'], env=env, capture_output=True,
                            text=True, timeout=90)
except subprocess.TimeoutExpired as error:
    for chunk in (error.stdout, error.stderr):
        if chunk:
            sys.stdout.write(chunk.decode(errors='replace') if isinstance(chunk, bytes)
                             else chunk)
    sys.exit(124)
sys.stdout.write(result.stdout)
sys.stdout.write(result.stderr)
sys.exit(result.returncode)
PYDM2INFLATE
    ) || dm2_inflate_close_status=$?
    set -e
    if [ "${dm2_inflate_close_status:-0}" -ne 0 ] ||
       ! printf '%s\n' "$dm2_inflate_close_output" | \
            grep -F 'task=reading original DM2 disc image' | \
            grep -F "path=$default_data_root/dm2/" >/dev/null ||
       ! printf '%s\n' "$dm2_inflate_close_output" | \
            grep -Fq 'autotest injected window-close during DM2 disc image inflate' ||
       printf '%s\n' "$dm2_inflate_close_output" | \
            grep -Fq 'launch phase=game-handoff'; then
        printf 'FAIL: DM2 image-inflate window close did not cancel startup (status %s)\n%s\n' \
            "${dm2_inflate_close_status:-0}" "$dm2_inflate_close_output" >&2
        exit 1
    fi
    echo 'PASS: no-game startup cancels DM2 disc-image inflation on window close'
fi

# A normal app start with no game selection must open the launcher and discover
# every installed game from the default ~/.firestaff/data tree. Use only links
# to authentic archives and an isolated home/config root.
if [ -f "$dm1_towns_archive" ] && [ -f "$csb_towns_archive" ] &&
   [ -f "$towns_archive" ]; then
    set +e
    no_game_output=$(HOME="$default_home" USERPROFILE="$default_home" \
        XDG_CONFIG_HOME="$default_home/.config" \
        APPDATA="$default_home/AppData" \
        FIRESTAFF_CONFIG_PATH="$default_home/no-game-config.toml" \
        FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --verbose --debug --duration 1000 2>&1) || {
        no_game_status=$?
    }
    set -e
    if [ "${no_game_status:-0}" -ne 1 ]; then
        printf 'FAIL: no-game startup did not run the launcher without launching a game (status %s)\n%s\n' \
            "${no_game_status:-0}" "$no_game_output" >&2
        exit 1
    fi
    printf '%s\n' "$no_game_output" | grep -Fq 'startup game=none mode=menu' || {
        printf 'FAIL: no-game startup did not stay in the launcher\n%s\n' \
            "$no_game_output" >&2
        exit 1
    }
    for game_id in dm1 csb dm2; do
        printf '%s\n' "$no_game_output" | grep -Fq "firestaff: $game_id available=yes" || {
            printf 'FAIL: no-game startup did not discover authentic %s media\n%s\n' \
                "$game_id" "$no_game_output" >&2
            exit 1
        }
    done
    printf '%s\n' "$no_game_output" | \
        grep -Fq 'firestaff: scan complete elapsed-ms=' || {
        printf 'FAIL: --debug did not report the deferred startup scan completion\n%s\n' \
            "$no_game_output" >&2
        exit 1
    }
    printf '%s\n' "$no_game_output" | \
        grep -Fq 'firestaff: scan partial-ready games=dm1:yes,csb:yes,dm2:' || {
        printf 'FAIL: deferred scan did not publish the authenticated DM1/CSB snapshot before completion\n%s\n' \
            "$no_game_output" >&2
        exit 1
    }
    expected_scan_games='dm1:yes,csb:yes,dm2:yes'
    if [ -f "$nexus_iso" ]; then
        expected_scan_games="$expected_scan_games,nexus:yes"
    fi
    if [ -f "$theron_track02" ]; then
        expected_scan_games="$expected_scan_games,theron:yes"
    fi
    printf '%s\n' "$no_game_output" | \
        grep -Fq "result=completed games=$expected_scan_games" || {
        printf 'FAIL: deferred startup scan summary did not report original game availability\n%s\n' \
            "$no_game_output" >&2
        exit 1
    }
    if [ -f "$nexus_iso" ]; then
        printf '%s\n' "$no_game_output" | grep -Fq 'firestaff: nexus available=yes' || {
            printf 'FAIL: no-game startup did not discover authentic Nexus media\n%s\n' \
                "$no_game_output" >&2
            exit 1
        }
    fi
    if [ -f "$theron_track02" ]; then
        printf '%s\n' "$no_game_output" | grep -Fq 'firestaff: theron available=yes' || {
            printf 'FAIL: no-game startup did not discover authentic Theron media\n%s\n' \
                "$no_game_output" >&2
            exit 1
        }
    fi
    if printf '%s\n' "$no_game_output" | grep -Fq 'launch phase=game-handoff'; then
        printf 'FAIL: no-game startup entered a game handoff\n%s\n' \
            "$no_game_output" >&2
        exit 1
    fi
    printf '%s\n' "$no_game_output" | \
        grep -Fq 'launch smoke failed: no launch reached before exit' || {
        printf 'FAIL: no-game startup did not finish the menu loop\n%s\n' \
            "$no_game_output" >&2
        exit 1
    }
    echo 'PASS: no-game startup opens the launcher and discovers authentic DM1, CSB, DM2, Nexus and Theron media when installed'

    # A single-file preference must be promoted to its shared collection root
    # by the ordinary no-game launcher. This uses an actual original archive;
    # explicit --data-dir file requests remain scoped to their selected source.
    single_file_config="$default_home/single-file-menu-config.toml"
    printf 'data_dir = "%s"\n' "$dm1_towns_archive" > "$single_file_config"
    set +e
    single_file_output=$(HOME="$default_home" USERPROFILE="$default_home" \
        XDG_CONFIG_HOME="$default_home/.config" \
        APPDATA="$default_home/AppData" \
        FIRESTAFF_CONFIG_PATH="$single_file_config" \
        FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --verbose --debug --duration 1000 2>&1) || {
        single_file_status=$?
    }
    set -e
    if [ "${single_file_status:-0}" -ne 1 ]; then
        printf 'FAIL: persisted single-file startup did not remain in the launcher (status %s)\n%s\n' \
            "${single_file_status:-0}" "$single_file_output" >&2
        exit 1
    fi
    printf '%s\n' "$single_file_output" | grep -Fq 'startup game=none mode=menu' || {
        printf 'FAIL: persisted single-file startup did not stay in the launcher\n%s\n' \
            "$single_file_output" >&2
        exit 1
    }
    for game_id in dm1 csb dm2; do
        printf '%s\n' "$single_file_output" | \
            grep -Fq "firestaff: $game_id available=yes" || {
            printf 'FAIL: persisted single-file startup did not discover authentic %s media\n%s\n' \
                "$game_id" "$single_file_output" >&2
            exit 1
        }
    done
    if printf '%s\n' "$single_file_output" | \
        grep -Fq 'launch phase=game-handoff'; then
        printf 'FAIL: persisted single-file startup entered a game handoff\n%s\n' \
            "$single_file_output" >&2
        exit 1
    fi
    printf 'PASS: persisted original media file still discovers DM1, CSB and DM2 from the collection menu\n'
fi

for game_id in dm1 csb dm2 nexus theron; do
    if [ -d "$default_data_root/$game_id" ]; then
        expected_launch_phase='runtime-entered'
        case "$game_id" in
            dm1|csb|dm2) expected_platform='FM Towns' ;;
            nexus)
                expected_platform='Saturn'
                expected_launch_phase='title-entered'
                ;;
            theron) expected_platform='PC Engine' ;;
        esac
        default_output=$(HOME="$default_home" USERPROFILE="$default_home" \
            XDG_CONFIG_HOME="$default_home/.config" \
            APPDATA="$default_home/AppData" \
            FIRESTAFF_CONFIG_PATH="$default_home/config.toml" \
            SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
            --game "$game_id" --verbose --debug --boot-probe \
            --boot-probe-frames 0 2>&1) || {
            printf '%s\n' "$default_output" >&2
            exit 1
        }
        if ! printf '%s\n' "$default_output" | \
                grep -Fq "startup game=$game_id mode=direct platform=auto data=default search roots" ||
           ! printf '%s\n' "$default_output" | \
                grep -Fq "launch phase=game-handoff mode=direct game=$game_id platform=$expected_platform edition=" ||
           ! printf '%s\n' "$default_output" | \
                grep -Fq "$default_data_root/$game_id/" ||
           ! printf '%s\n' "$default_output" | \
                grep -Fq "launch phase=$expected_launch_phase game=$game_id" ||
           ! printf '%s\n' "$default_output" | \
                grep -Fq "startup-frame game=$game_id " ||
           ! printf '%s\n' "$default_output" | \
                grep -Fq "FIRESTAFF BOOT PROBE READY: gameId=$game_id "; then
            printf '%s\n' "$default_output" >&2
            printf 'FAIL: bare --game %s did not report the selected original media and reach its runtime handoff from the default per-game data directory\n' \
                "$game_id" >&2
            exit 1
        fi
        if [ "$game_id" = nexus ] && {
            ! printf '%s\n' "$default_output" | \
                grep -Fq 'NEXUS STARTUP RECEIPT: status=blocked gameId=nexus';
            ! printf '%s\n' "$default_output" | \
                grep -Fq 'title=1 levelLoaded=0 menu=0 blocker=menu-bpk-prs3-capture-required';
            printf '%s\n' "$default_output" | \
                grep -Fq 'launch phase=runtime-entered game=nexus';
        }; then
            printf '%s\n' "$default_output" >&2
            printf 'FAIL: bare --game nexus reported playable runtime without a loaded level or admitted menu\n' >&2
            exit 1
        fi
        printf 'PASS: bare --game %s resolves original %s media and reports its source-bound startup trace from ~/.firestaff/data/%s\n' \
            "$game_id" "$expected_platform" "$game_id"
    fi
done

if [ -f "$mac_archive" ]; then
    ln -s "$mac_archive" "$combined_root/$(basename "$mac_archive")"
fi
ln -s "$towns_archive" "$combined_root/$(basename "$towns_archive")"
if [ -f "$dos_archive" ]; then
    ln -s "$dos_archive" "$combined_root/$(basename "$dos_archive")"
fi
if [ -f "$amiga_archive" ]; then
    ln -s "$amiga_archive" "$combined_root/$(basename "$amiga_archive")"
fi
combined_auto=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game dm2 --data-dir "$combined_root" \
    --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
    printf '%s\n' "$combined_auto" >&2
    exit 1
}
if [ -f "$mac_archive" ]; then
    case "$combined_auto" in
        *"startup game=dm2 mode=direct platform=auto"*"selected game=dm2 platform=FM Towns edition=fmtowns-ja source="*"platform=FM Towns edition=fmtowns-ja matched source="*"platform=Macintosh edition=mac-en-retail matched source="*) ;;
        *) echo "FAIL: DM2 AUTO did not prefer FM Towns when Mac retail was also present" >&2; exit 1 ;;
    esac
    combined_mac=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game dm2 --platform mac --data-dir "$combined_root" \
        --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
        printf '%s\n' "$combined_mac" >&2
        exit 1
    }
    case "$combined_mac" in
        *"selected game=dm2 platform=Macintosh edition=mac-en-retail source="*) ;;
        *) echo "FAIL: explicit DM2 Macintosh choice was overridden by FM Towns default" >&2; exit 1 ;;
    esac
else
    case "$combined_auto" in
        *"startup game=dm2 mode=direct platform=auto"*"selected game=dm2 platform=FM Towns edition=fmtowns-ja source="*) ;;
        *) echo "FAIL: DM2 AUTO did not select FM Towns without Mac retail installed" >&2; exit 1 ;;
    esac
fi

verify_mixed_dm2_cli_platform() {
    platform_arg=$1
    platform_name=$2
    edition_id=$3
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game dm2 --platform "$platform_arg" --data-dir "$combined_root" \
        --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
        printf '%s\n' "$output" >&2
        exit 1
    }
    case "$output" in
        *"selected game=dm2 platform=$platform_name edition=$edition_id source=$combined_root/"*) ;;
        *)
            printf 'FAIL: explicit DM2 %s choice was not retained from mixed media root\n' \
                "$platform_name" >&2
            printf '%s\n' "$output" >&2
            exit 1
            ;;
    esac
    printf 'PASS: mixed-root direct CLI selects DM2 %s %s\n' \
        "$platform_name" "$edition_id"
}

if [ -f "$dos_archive" ]; then
    verify_mixed_dm2_cli_platform pc PC pc-en
fi
verify_mixed_dm2_cli_platform fm-towns 'FM Towns' fmtowns-ja
if [ -f "$amiga_archive" ]; then
    verify_mixed_dm2_cli_platform amiga Amiga amiga-en
fi

# A bare game selection must not inherit an obsolete platform from saved
# launcher settings. Check the other two original FM Towns archives when
# they are staged, using the same direct CLI path as the DM2 check above.
if [ -f "$dm1_towns_archive" ]; then
    dm1_auto=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game dm1 --data-dir "$dm1_towns_archive" \
        --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
        printf '%s\n' "$dm1_auto" >&2
        exit 1
    }
    case "$dm1_auto" in
        *"startup game=dm1 mode=direct platform=auto"*"selected game=dm1 platform=FM Towns edition="*) ;;
        *) echo "FAIL: bare DM1 did not select original FM Towns media" >&2; exit 1 ;;
    esac
    dm1_japanese_verbose=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game dm1 --platform fm-towns --dm1-fmtowns-ja \
        --data-dir "$dm1_towns_archive" --verbose \
        --boot-probe --boot-probe-frames 0 2>&1) || {
        printf '%s\n' "$dm1_japanese_verbose" >&2
        exit 1
    }
    case "$dm1_japanese_verbose" in
        *"selected game=dm1 platform=FM Towns edition=fmtowns-ja source="*) ;;
        *) echo "FAIL: DM1 Japanese verbose diagnostics did not report the overridden edition" >&2; exit 1 ;;
    esac
    dm1_natural_debug=$(FIRESTAFF_CONFIG_PATH="$combined_root/dm1-towns-debug.toml" \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game dm1 --data-dir "$dm1_towns_archive" --debug --duration 8000 2>&1) || {
        printf '%s\n' "$dm1_natural_debug" >&2
        exit 1
    }
    printf '%s\n' "$dm1_natural_debug" | python3 -c '
import re
import sys

trace = sys.stdin.read()
frames = [
    (int(step), int(elapsed), int(vblanks))
    for step, elapsed, vblanks in re.findall(
        r"startup-source-frame game=dm1 (?:platform=fm-towns )?phase=title "
        r"frame=(\d+)/20 (?:palette=[^ ]+ )?elapsed-ms=(\d+) "
        r"source-wait-vblanks=(\d+)",
        trace,
    )
]
complete = re.search(
    r"startup-source-complete game=dm1 (?:platform=fm-towns )?phase=title "
    r"frames=20 source-vblanks=21 .*?elapsed-ms=(\d+)",
    trace,
)
swsh = re.search(
    r"startup-source-complete game=dm1 (?:platform=[^ ]+ )?phase=swsh-intro "
    r"frames=(\d+) source-vblanks=(\d+) elapsed-ms=(\d+)",
    trace,
)
if ([step for step, _, _ in frames] != list(range(20)) or
        sum(vblanks for _, _, vblanks in frames) != 18 or
        complete is None or int(complete.group(1)) < 350 or
        (swsh is not None and
         (int(swsh.group(1)) != 17 or int(swsh.group(2)) != 30 or
          int(swsh.group(3)) < 3000))):
    raise SystemExit("FAIL: DM1 debug trace did not preserve FM Towns title cadence")
print("PASS: DM1 debug trace records the FM Towns title source cadence")
if swsh is not None:
    print("PASS: DM1 debug trace records the SWSH source intro cadence")
' || {
        printf '%s\n' "$dm1_natural_debug" >&2
        exit 1
    }
fi
if [ -f "$dm1_pc_archive" ]; then
    dm1_pc_debug=$(FIRESTAFF_CONFIG_PATH="$combined_root/dm1-pc-debug.toml" \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game dm1 --platform pc --data-dir "$dm1_pc_archive" \
        --debug --duration 8000 2>&1) || {
        printf '%s\n' "$dm1_pc_debug" >&2
        exit 1
    }
    printf '%s\n' "$dm1_pc_debug" | python3 -c '
import re
import sys

trace = sys.stdin.read()
if not re.search(
    r"renderer backend=(software|unavailable) vsync=(-?\d+|unavailable) "
    r"drawable-scale=1\.000x1\.000", trace
):
    raise SystemExit("FAIL: DM1 PC debug omitted actual renderer, VSync or drawable scale")
swsh = re.search(
    r"startup-source-complete game=dm1 phase=swsh-intro "
    r"frames=(\d+) source-vblanks=(\d+) elapsed-ms=(\d+)",
    trace,
)
if swsh is None or int(swsh.group(1)) != 17 or int(swsh.group(2)) != 30 or int(swsh.group(3)) < 3000:
    raise SystemExit("FAIL: DM1 PC original-media debug trace did not preserve SWSH intro source cadence")
print("PASS: DM1 PC original-media debug trace records SWSH intro source cadence")
' || {
        printf '%s\n' "$dm1_pc_debug" >&2
        exit 1
    }
fi
if [ -f "$csb_towns_archive" ]; then
    csb_auto=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game csb --data-dir "$csb_towns_archive" \
        --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
        printf '%s\n' "$csb_auto" >&2
        exit 1
    }
    case "$csb_auto" in
        *"startup game=csb mode=direct platform=auto"*"selected game=csb platform=FM Towns edition="*) ;;
        *) echo "FAIL: bare CSB did not select original FM Towns media" >&2; exit 1 ;;
    esac
    csb_japanese_verbose=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game csb --platform fm-towns --csb-fmtowns-ja \
        --data-dir "$csb_towns_archive" --verbose \
        --boot-probe --boot-probe-frames 0 2>&1) || {
        printf '%s\n' "$csb_japanese_verbose" >&2
        exit 1
    }
    case "$csb_japanese_verbose" in
        *"selected game=csb platform=FM Towns edition=fmtowns-ja source="*) ;;
        *) echo "FAIL: CSB Japanese verbose diagnostics did not report the overridden edition" >&2; exit 1 ;;
    esac
    if [ -d "$csb_towns_loose_root/CDATA" ] &&
       [ -d "$csb_towns_loose_root/CJDATA" ]; then
        csb_mixed_root=$(mktemp -d "${PWD}/.firestaff-csb-mixed-media.XXXXXX")
        mkdir "$csb_mixed_root/csb"
        ln -s "$csb_towns_archive" "$csb_mixed_root/csb/$(basename "$csb_towns_archive")"
        ln -s "$csb_towns_loose_root" "$csb_mixed_root/csb/fmtowns_iso"
        csb_mixed=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
            --game csb --data-dir "$csb_mixed_root" \
            --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
            printf '%s\n' "$csb_mixed" >&2
            exit 1
        }
        case "$csb_mixed" in
            *"selected game=csb platform=FM Towns edition=fmtowns-en source="*"$(basename "$csb_towns_archive")::CDATA/GRAPHICS.DAT"*) ;;
            *) echo "FAIL: CSB selected the loose tree over its complete original CD image" >&2; exit 1 ;;
        esac
        rm -rf "$csb_mixed_root"
    fi
fi

# When both original CSB FM Towns and Amiga packages are installed, AUTO must
# retain the FM Towns default independently of catalogue order. Both archives
# remain at their authentic filenames because CD admission also validates the
# documented original package path; the links keep this check read-only.
if [ -f "$csb_towns_archive" ] && [ -f "$csb_amiga_archive" ]; then
    csb_combined_root=$(mktemp -d "${PWD}/.firestaff-csb-platforms.XXXXXX")
    ln -s "$csb_towns_archive" \
        "$csb_combined_root/$(basename "$csb_towns_archive")"
    ln -s "$csb_amiga_archive" \
        "$csb_combined_root/$(basename "$csb_amiga_archive")"
    if [ -f "$csb_atari_archive" ]; then
        ln -s "$csb_atari_archive" \
            "$csb_combined_root/$(basename "$csb_atari_archive")"
    fi
    csb_auto_multi=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game csb --data-dir "$csb_combined_root" \
        --debug --boot-probe --boot-probe-frames 0 2>&1) || {
        printf '%s\n' "$csb_auto_multi" >&2
        exit 1
    }
    case "$csb_auto_multi" in
        *"csb platform=FM Towns edition=fmtowns-en matched source=$csb_combined_root/$(basename "$csb_towns_archive")::CDATA/GRAPHICS.DAT"*"csb platform=Amiga edition=amiga31-multi matched source="*"launch phase=game-handoff mode=direct game=csb platform=FM Towns edition=fmtowns-en source=$csb_combined_root/$(basename "$csb_towns_archive")::CDATA/GRAPHICS.DAT"*) ;;
        *)
            echo "FAIL: CSB AUTO did not prefer FM Towns with authentic Amiga media also installed" >&2
            printf '%s\n' "$csb_auto_multi" >&2
            exit 1
            ;;
    esac
    echo "PASS: CSB AUTO prefers FM Towns when original Amiga media is also installed"

    csb_explicit_amiga=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game csb --platform amiga --data-dir "$csb_combined_root" \
        --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
        printf '%s\n' "$csb_explicit_amiga" >&2
        exit 1
    }
    case "$csb_explicit_amiga" in
        *"selected game=csb platform=Amiga edition=amiga31-multi source=$csb_combined_root/$(basename "$csb_amiga_archive")"*) ;;
        *) echo "FAIL: explicit CSB Amiga selection was not retained with FM Towns installed" >&2; exit 1 ;;
    esac

    csb_explicit_towns=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game csb --platform fm-towns --data-dir "$csb_combined_root" \
        --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
        printf '%s\n' "$csb_explicit_towns" >&2
        exit 1
    }
    case "$csb_explicit_towns" in
        *"selected game=csb platform=FM Towns edition=fmtowns-en source=$csb_combined_root/$(basename "$csb_towns_archive")::CDATA/GRAPHICS.DAT"*) ;;
        *) echo "FAIL: explicit CSB FM Towns selection was not retained with Amiga installed" >&2; exit 1 ;;
    esac

    if [ -f "$csb_atari_archive" ]; then
        csb_explicit_atari=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
            --game csb --platform atari-st --data-dir "$csb_combined_root" \
            --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
            printf '%s\n' "$csb_explicit_atari" >&2
            exit 1
        }
        case "$csb_explicit_atari" in
            *"selected game=csb platform=Atari ST edition=st20-21-en source=$csb_combined_root/$(basename "$csb_atari_archive")::"*) ;;
            *) echo "FAIL: explicit CSB Atari ST selection was not retained with FM Towns and Amiga installed" >&2; exit 1 ;;
        esac
    fi
fi

# When both authentic DM1 PC and FM Towns releases are installed, a bare
# --game request must retain FM Towns as the default. Keep the unsupported
# PC-98 archive in the same root when available to ensure discovery never
# promotes it into a selectable or launchable platform.
if [ -f "$dm1_towns_archive" ] && [ -f "$dm1_pc_archive" ]; then
    dm1_combined_root=$(mktemp -d "${PWD}/.firestaff-dm1-platforms.XXXXXX")
    ln -s "$dm1_towns_archive" "$dm1_combined_root/$(basename "$dm1_towns_archive")"
    ln -s "$dm1_pc_archive" "$dm1_combined_root/$(basename "$dm1_pc_archive")"
    if [ -f "$dm1_pc98_archive" ]; then
        ln -s "$dm1_pc98_archive" "$dm1_combined_root/$(basename "$dm1_pc98_archive")"
    fi
    dm1_auto_multi=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game dm1 --data-dir "$dm1_combined_root" \
        --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
        printf '%s\n' "$dm1_auto_multi" >&2
        exit 1
    }
    case "$dm1_auto_multi" in
        *"selected game=dm1 platform=FM Towns edition="*) ;;
        *) echo "FAIL: DM1 AUTO did not prefer FM Towns when PC media was also present" >&2; exit 1 ;;
    esac
    case "$dm1_auto_multi" in
        *"platform=PC-98"*|*"PC-98 edition="*)
            echo "FAIL: unsupported DM1 PC-98 media was surfaced by the launcher" >&2
            exit 1
            ;;
    esac
    dm1_pc_multi=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game dm1 --platform pc --data-dir "$dm1_combined_root" \
        --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
        printf '%s\n' "$dm1_pc_multi" >&2
        exit 1
    }
    case "$dm1_pc_multi" in
        *"selected game=dm1 platform=PC edition="*) ;;
        *) echo "FAIL: explicit DM1 PC selection was overridden by FM Towns AUTO" >&2; exit 1 ;;
    esac
    echo "PASS: DM1 AUTO prefers FM Towns over PC and ignores unsupported PC-98 media"
fi

# macOS data folders selected in Finder may use an uppercase game name even
# though Firestaff's installed folders are documented in lowercase. When an
# interactive menu starts from that selected leaf, it must scan the parent
# collection so sibling games remain available. Stage only the selected
# authentic archives: scanning unrelated installed editions makes this path
# regression depend on the size of a developer's complete media collection.
# No game data is copied or synthesized.
uppercase_game_root=$(mktemp -d "${PWD}/.firestaff-uppercase-game-leaf.XXXXXX")
uppercase_data_root="$uppercase_game_root/data"
mkdir -p "$uppercase_data_root/DM1"
uppercase_expected_games=
for game_id in dm1 csb dm2; do
    case "$game_id" in
        dm1) leaf_name=DM1; selected_archive=$dm1_towns_archive ;;
        csb) leaf_name=csb; selected_archive=$csb_towns_archive ;;
        dm2) leaf_name=dm2; selected_archive=$towns_archive ;;
    esac
    if [ -f "$selected_archive" ]; then
        mkdir -p "$uppercase_data_root/$leaf_name"
        ln -s "$selected_archive" \
            "$uppercase_data_root/$leaf_name/$(basename "$selected_archive")"
        uppercase_expected_games="$uppercase_expected_games $game_id"
    fi
done
uppercase_leaf_menu=$(FIRESTAFF_CONFIG_PATH="$uppercase_game_root/menu.toml" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --menu --game dm1 --data-dir "$uppercase_data_root/DM1/" --debug --verbose \
    --duration 1000 2>&1) || {
    printf '%s\n' "$uppercase_leaf_menu" >&2
    exit 1
}
for game_id in $uppercase_expected_games; do
    if ! printf '%s\n' "$uppercase_leaf_menu" | \
           grep -Fq "firestaff: $game_id available=yes"; then
        printf '%s\n' "$uppercase_leaf_menu" >&2
        printf 'FAIL: uppercase DM1 data leaf hid sibling %s media from the start menu\n' \
            "$game_id" >&2
        exit 1
    fi
done
echo "PASS: uppercase DM1 menu selection reports authentic availability:$uppercase_expected_games"

# An offline remembered root must not block authenticated fallback media or
# be replaced by that fallback in the user's saved selection.
if [ -f "$dm1_pc_archive" ]; then
    for offline_leaf in external-originals collection/dm1; do
        offline_home=$(mktemp -d "${PWD}/.firestaff-offline-root.XXXXXX")
        offline_default="$offline_home/.firestaff/data"
        offline_selected="$offline_home/$offline_leaf"
        mkdir -p "$offline_default/dm1" "$offline_selected"
        ln -s "$dm1_pc_archive" "$offline_default/dm1/$(basename "$dm1_pc_archive")"
        run_offline_profile() {
            HOME="$offline_home" USERPROFILE="$offline_home" \
            XDG_CONFIG_HOME="$offline_home" APPDATA="$offline_home" \
            FIRESTAFF_CONFIG_PATH="$offline_home/config.toml" \
            FIRESTAFF_ORIGINALS_DIR="$offline_default" FIRESTAFF_DATA= \
            FIRESTAFF_AUTOTEST_ENTRANCE_INPUT=key:return \
            FIRESTAFF_AUTOTEST_SELECTOR_INPUT=key:return \
            FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$offline_home/runtime.json" \
            SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
                "$firestaff_cli" "$@"
        }
        run_offline_profile --menu --game dm1 --data-dir "$offline_selected" \
            --duration 1000 --verbose >"$offline_home/selected.log" 2>&1
        offline_selection=$(grep '^data_dir = ' "$offline_home/config.toml")
        case "$offline_selection" in
            *"$offline_leaf"*) ;;
            *) echo "FAIL: original fallback setup lost selected root" >&2; exit 1 ;;
        esac
        rmdir "$offline_selected"
        run_offline_profile --menu --duration 1000 --verbose \
            >"$offline_home/offline-menu.log" 2>&1
        grep -Fq 'firestaff: dm1 available=yes' "$offline_home/offline-menu.log" || {
            cat "$offline_home/offline-menu.log" >&2
            echo "FAIL: offline remembered root hid authentic default DM1 media" >&2
            exit 1
        }
        # Diagnostic startup must admit the same authenticated ZIP fallback
        # as ordinary startup, even without an explicit --data-dir.
        run_offline_profile --game dm1 --presentation-mode v1 --verbose --debug \
            --boot-probe --boot-probe-frames 0 \
            >"$offline_home/offline-probe.log" 2>&1 || {
            cat "$offline_home/offline-probe.log" >&2
            echo "FAIL: diagnostic startup rejected authentic fallback ZIP media" >&2
            exit 1
        }
        grep -F 'FIRESTAFF BOOT PROBE READY: gameId=dm1 ' "$offline_home/offline-probe.log" |
            grep -F 'assetMd5=fa6b1aa29e191418713bf2cda93d962e ' |
            grep -F "dataDir=$offline_default/dm1/$(basename "$dm1_pc_archive")::" |
            grep -E '(::|/)DATA/GRAPHICS[.]DAT ' |
            grep -Fq 'levelLoaded=1 ' || {
            cat "$offline_home/offline-probe.log" >&2
            echo "FAIL: diagnostic startup selected a different DM1 source" >&2
            exit 1
        }
        run_offline_profile --game dm1 --presentation-mode v1 --verbose --debug --duration 20000 \
            --script wait-game-runtime,up \
            >"$offline_home/offline-runtime.log" 2>&1 || {
            cat "$offline_home/offline-runtime.log" >&2
            exit 1
        }
        grep -Fq 'startup-source-frame game=dm1 platform=pc34' "$offline_home/offline-runtime.log" || {
            cat "$offline_home/offline-runtime.log" >&2
            echo "FAIL: authentic fallback did not select PC media" >&2
            exit 1
        }
        grep -Eq 'dm1-motion-present .*moved=1' "$offline_home/offline-runtime.log" || {
            cat "$offline_home/offline-runtime.log" >&2
            if [ -f "$offline_home/runtime.json" ]; then cat "$offline_home/runtime.json" >&2; fi
            echo "FAIL: authentic fallback did not present an accepted DM1 movement" >&2
            exit 1
        }
        if [ "$offline_selection" != "$(grep '^data_dir = ' "$offline_home/config.toml")" ] ||
           [ -e "$offline_selected" ]; then
            echo "FAIL: authentic default fallback forgot or recreated the offline selection" >&2
            exit 1
        fi
        echo "PASS: authentic DM1 PC menu/CLI fallback preserves offline selection ($offline_leaf)"
        rm -rf "$offline_home"
        offline_home=
    done
fi

echo "PASS: original-media startup diagnostics and FM Towns defaults"
