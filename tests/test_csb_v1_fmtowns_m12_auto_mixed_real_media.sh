#!/usr/bin/env sh
set -eu

. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_csb_v1_fmtowns_m12_auto_mixed_real_media.sh <firestaff> <csb-data-root>}
collection_root=${2:?usage: test_csb_v1_fmtowns_m12_auto_mixed_real_media.sh <firestaff> <csb-data-root>}
towns_archive="$collection_root/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip"
amiga_archive="$collection_root/Dungeon-Master-Chaos-Strikes-Back---Expansion-Set-1_Amiga_EN.zip"
atari_archive="$collection_root/chaos_strikes_back_ftl.zip"
atari_utility_archive="$collection_root/Game,Chaos_Strikes_Back,Atari_ST,Software.7z"

if [ ! -x "$app" ] || [ ! -f "$towns_archive" ] ||
   [ ! -f "$amiga_archive" ] || [ ! -f "$atari_archive" ] ||
   [ ! -f "$atari_utility_archive" ]; then
    echo 'SKIP: authentic CSB FM Towns, Amiga, Atari ST game, and Atari ST utility archives are required'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/csb-fmtowns-auto-mixed.XXXXXX")
trap 'find "$scratch" -depth -delete' EXIT HUP INT TERM
atari_utility_sha_before=$(firestaff_sha256_file "$atari_utility_archive")

# Run the normal M12 path against a mixed collection. Deliberately omit
# --platform so the production scanner and AUTO edition ranking choose the
# documented FM Towns default while authentic Amiga and Atari ST editions are
# also installed. The scripted title, SWITCHTW Game row, and Prison Enter
# clicks then reach the first real MINI.DAT party without a synthetic save.
FIRESTAFF_CONFIG_PATH="$scratch/config.toml" \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$scratch/runtime.json" \
HOME="$scratch" XDG_CONFIG_HOME="$scratch" APPDATA="$scratch" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --width 320 --height 200 --menu --game csb \
    --data-dir "$collection_root" --debug \
    --script 'enter,enter,enter,wait700,click:52:110,wait10,click:250:50,wait240' \
    --duration 30000 >"$scratch/firestaff.log" 2>&1 || {
        cat "$scratch/firestaff.log" >&2
        exit 1
    }

python3 - "$scratch/firestaff.log" "$scratch/runtime.json" \
    "$towns_archive" <<'PY'
import json
import re
import sys

log_path, probe_path, towns_archive = sys.argv[1:]
with open(log_path, encoding="utf-8") as stream:
    trace = stream.read()
with open(probe_path, encoding="utf-8") as stream:
    probe = json.load(stream)

for platform, edition in (("Amiga", "amiga31-multi"),
                           ("Atari ST", "st20-21-en")):
    if re.search(rf"csb platform={re.escape(platform)} edition={edition} "
                 r"matched source=", trace) is None:
        raise SystemExit(
            f"FAIL: mixed CSB collection did not authenticate {platform} {edition}")

ready = re.search(
    r"CSB READY: gameId=csb dataDir=" + re.escape(towns_archive) +
    r" variant=csb-fmtowns-en route=startup handoff=f31-title-anm "
    r"handoffHash=[0-9a-f]{8}", trace)
launch = re.search(
    r"launch phase=game-handoff mode=menu game=csb platform=FM Towns "
    r"edition=fmtowns-en source=" + re.escape(towns_archive), trace)
if ready is None or launch is None:
    raise SystemExit(
        "FAIL: M12 AUTO did not select and launch the authentic FM Towns edition")

startup = probe["startup"]
party = probe["party"]
if (probe.get("launchedEver") != 1 or probe.get("sourceId") != "csb" or
        startup.get("phase") != "inactive" or
        startup.get("startupActive") != 0 or startup.get("levelLoaded") != 1 or
        probe.get("csbViewportHash", 0) == 0 or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (4, 22, 18, 2, 1)):
    raise SystemExit(
        f"FAIL: CSB M12 AUTO did not reach the authentic MINI.DAT runtime: {probe}")

print("PASS: mixed authentic CSB M12 AUTO selected FM Towns and reached "
      "the original MINI.DAT party")
PY

# Select the non-default Atari edition from the same mixed original-media
# collection through M12. There is deliberately no --platform argument: the
# visible platform card must override the FM Towns AUTO choice and keep the
# Atari source owner through its original C200 entrance. Let its source-owned
# startup sequence reach the entrance, click the original button, then wait
# for the runtime receipt instead of treating the launcher handoff as a
# successful game start.
FIRESTAFF_CONFIG_PATH="$scratch/atari-config.toml" \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$scratch/atari-runtime.json" \
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
HOME="$scratch" XDG_CONFIG_HOME="$scratch" APPDATA="$scratch" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --width 960 --height 600 --menu --game csb \
    --data-dir "$collection_root" --debug \
    --script 'wait20,click:586:131,wait20,click:729:202,wait20,click:225:202,wait:3600,click:813:156,wait-game-runtime' \
    --duration 120000 >"$scratch/atari-firestaff.log" 2>&1 || {
        cat "$scratch/atari-firestaff.log" >&2
        exit 1
    }

python3 - "$scratch/atari-firestaff.log" "$scratch/atari-runtime.json" \
    "$atari_archive" <<'PY'
import json
import re
import sys

log_path, probe_path, atari_archive = sys.argv[1:]
with open(log_path, encoding="utf-8") as stream:
    trace = stream.read()
with open(probe_path, encoding="utf-8") as stream:
    probe = json.load(stream)

ready = re.search(
    r"CSB READY: gameId=csb dataDir=" + re.escape(atari_archive) +
    r" variant=csb-st20-21-en route=startup "
    r"handoff=atari-st-animate-ftlcode handoffHash=[0-9a-f]{8}", trace)
launch = re.search(
    r"launch phase=game-handoff mode=menu game=csb platform=Atari ST "
    r"edition=st20-21-en source=" + re.escape(atari_archive), trace)
if ready is None or launch is None:
    raise SystemExit(
        "FAIL: mixed CSB M12 did not hand off to the selected authentic Atari ST edition")

startup = probe["startup"]
party = probe["party"]
if (probe.get("launchedEver") != 1 or probe.get("active") != 1 or
        probe.get("sourceId") != "csb" or
        startup.get("receiptReady") != 1 or startup.get("phase") != "inactive" or
        startup.get("active") != 1 or startup.get("startupActive") != 0 or
        startup.get("levelLoaded") != 1 or
        probe.get("csbViewportHash", 0) == 0 or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (0, 9, 0, 2, 0)):
    raise SystemExit(
        f"FAIL: CSB M12 Atari selection did not reach the authentic C200 runtime: {probe}")

print("PASS: mixed authentic CSB M12 menu selected Atari ST without --platform "
      "and reached the original C200 runtime")
PY

# Select the other non-default native platform from the same mixed install.
# Isolate this one M12 session with links to the three original archives so
# unrelated authentic variants elsewhere under the user's data directory
# cannot change which Amiga release owns the launch. The links leave every
# source archive in place; no game bytes or saves are generated or copied.
# The direct CLI checks later reuse the same isolated collection.
amiga_media_root="$scratch/amiga-platform-selection"
amiga_home="$scratch/amiga-home"
mkdir -p "$amiga_media_root" "$amiga_home"
# Atari ANIMATE.SCR/DAT lives on the original ST utility disk, separate from
# the game disk. Keep both original Atari archives in this isolated install so
# platform selection tests exercise the complete source-owned startup chain.
for archive in "$towns_archive" "$amiga_archive" "$atari_archive" \
               "$atari_utility_archive"; do
    ln -s "$archive" "$amiga_media_root/$(basename "$archive")"
done

FIRESTAFF_CONFIG_PATH="$amiga_home/config.toml" \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$scratch/amiga-runtime.json" \
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
HOME="$amiga_home" XDG_CONFIG_HOME="$amiga_home" APPDATA="$amiga_home" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --width 320 --height 200 --menu --game csb \
    --data-dir "$amiga_media_root" --debug \
    --script 'enter,right,enter,enter,wait:1000,click:100:100,key:enter' \
    --duration 30000 >"$scratch/amiga-firestaff.log" 2>&1 || {
        cat "$scratch/amiga-firestaff.log" >&2
        exit 1
    }

python3 - "$scratch/amiga-firestaff.log" "$scratch/amiga-runtime.json" \
    "$amiga_media_root/$(basename "$amiga_archive")" <<'PY'
import json
import re
import sys

log_path, probe_path, amiga_archive = sys.argv[1:]
with open(log_path, encoding="utf-8") as stream:
    trace = stream.read()
with open(probe_path, encoding="utf-8") as stream:
    probe = json.load(stream)

ready = re.search(
    r"CSB READY: gameId=csb dataDir=" + re.escape(amiga_archive) +
    r" variant=csb-amiga-a31m route=startup "
    r"handoff=a31m-titl-dat handoffHash=2601fbe2", trace)
launch = re.search(
    r"launch phase=game-handoff mode=menu game=csb platform=Amiga "
    r"edition=amiga31-multi source=" + re.escape(amiga_archive) + r"::", trace)
if ready is None or launch is None:
    raise SystemExit(
        "FAIL: unforced M12 platform-card selection did not retain original Amiga media")

startup = probe["startup"]
party = probe["party"]
if (probe.get("launchedEver") != 1 or probe.get("active") != 1 or
        probe.get("sourceId") != "csb" or startup.get("receiptReady") != 1 or
        startup.get("phase") != "inactive" or
        startup.get("startupActive") != 0 or startup.get("levelLoaded") != 1 or
        probe.get("csbViewportHash", 0) == 0 or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (0, 9, 0, 2, 0)):
    raise SystemExit(
        f"FAIL: unforced M12 Amiga card selection did not reach the authentic A31M runtime: {probe}")

print("PASS: mixed authentic CSB M12 selected Amiga from the platform cards and "
      "reached the original A31M first runtime frame")
PY

# The same mixed-media selection must work with a mouse-only platform choice.
# The CSB cover and Amiga platform card are clicked; the remaining keyboard
# inputs accept the Original presentation and advance the original A31M APPB
# route. This guards against a platform card that works only through keyboard
# navigation.
FIRESTAFF_CONFIG_PATH="$amiga_home/mouse-config.toml" \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$scratch/amiga-mouse-runtime.json" \
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
HOME="$amiga_home" XDG_CONFIG_HOME="$amiga_home" APPDATA="$amiga_home" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --width 1920 --height 1080 --menu --game csb \
    --data-dir "$amiga_media_root" --debug \
    --script 'click:1159:262,click:934:405,enter,enter,wait:1000,click:600:540,key:enter' \
    --duration 30000 >"$scratch/amiga-mouse-firestaff.log" 2>&1 || {
        cat "$scratch/amiga-mouse-firestaff.log" >&2
        exit 1
    }

python3 - "$scratch/amiga-mouse-firestaff.log" \
    "$scratch/amiga-mouse-runtime.json" \
    "$amiga_media_root/$(basename "$amiga_archive")" <<'PY'
import json
import re
import sys

log_path, probe_path, amiga_archive = sys.argv[1:]
with open(log_path, encoding="utf-8") as stream:
    trace = stream.read()
with open(probe_path, encoding="utf-8") as stream:
    probe = json.load(stream)

ready = re.search(
    r"CSB READY: gameId=csb dataDir=" + re.escape(amiga_archive) +
    r" variant=csb-amiga-a31m route=startup "
    r"handoff=a31m-titl-dat handoffHash=2601fbe2", trace)
launch = re.search(
    r"launch phase=game-handoff mode=menu game=csb platform=Amiga "
    r"edition=amiga31-multi source=" + re.escape(amiga_archive) + r"::", trace)
if ready is None or launch is None:
    raise SystemExit(
        "FAIL: mouse-selected M12 Amiga card did not retain original A31M media")

startup = probe["startup"]
party = probe["party"]
if (probe.get("launchedEver") != 1 or probe.get("active") != 1 or
        probe.get("sourceId") != "csb" or startup.get("receiptReady") != 1 or
        startup.get("phase") != "inactive" or
        startup.get("startupActive") != 0 or startup.get("levelLoaded") != 1 or
        probe.get("csbViewportHash", 0) == 0 or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (0, 9, 0, 2, 0)):
    raise SystemExit(
        f"FAIL: mouse-selected M12 Amiga card did not reach authentic A31M: {probe}")

print("PASS: mouse-selected mixed authentic CSB M12 Amiga card reached the "
      "original A31M first runtime frame")
PY

# Exercise explicit CLI platform constraints against the same mixed set of
# original archives. These are separate from the card-selection checks above:
# --platform must constrain source selection before the ordinary M12 flow, and
# that source must remain the owner through its first authentic runtime frame.
explicit_platform_runtime() {
    platform=$1
    display_platform=$2
    edition=$3
    variant=$4
    handoff=$5
    archive=$6
    script=$7
    expected_party=$8
    width=$9
    height=${10}
    duration=${11}
    key=$(printf '%s' "$platform" | tr '[:upper:]' '[:lower:]' | tr -cd 'a-z0-9-')

    FIRESTAFF_CONFIG_PATH="$scratch/$key-explicit-config.toml" \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$scratch/$key-explicit-runtime.json" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    HOME="$scratch" XDG_CONFIG_HOME="$scratch" APPDATA="$scratch" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --width "$width" --height "$height" --menu --game csb \
        --data-dir "$amiga_media_root" --platform "$platform" --debug \
        --script "$script" --duration "$duration" >"$scratch/$key-explicit.log" 2>&1 || {
            cat "$scratch/$key-explicit.log" >&2
            return 1
        }

    python3 - "$scratch/$key-explicit.log" \
        "$scratch/$key-explicit-runtime.json" "$archive" \
        "$display_platform" "$edition" "$variant" "$handoff" \
        "$expected_party" <<'PY'
import json
import re
import sys

log_path, probe_path, archive, platform, edition, variant, handoff, party_text = sys.argv[1:]
with open(log_path, encoding="utf-8") as stream:
    trace = stream.read()
with open(probe_path, encoding="utf-8") as stream:
    probe = json.load(stream)

ready = re.search(
    r"CSB READY: gameId=csb dataDir=" + re.escape(archive) +
    r" variant=" + re.escape(variant) + r" route=startup handoff=" +
    re.escape(handoff) + r" handoffHash=[0-9a-f]{8}", trace)
launch = re.search(
    r"launch phase=game-handoff mode=menu game=csb platform=" +
    re.escape(platform) + r" edition=" + re.escape(edition) +
    r" source=" + re.escape(archive), trace)
if ready is None or launch is None:
    raise SystemExit(
        f"FAIL: explicit {platform} M12 launch did not retain expected original source")

startup = probe["startup"]
party = probe["party"]
actual_party = (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
                party.get("direction"), party.get("championCount"))
expected_party = tuple(map(int, party_text.split(",")))
if (probe.get("launchedEver") != 1 or probe.get("sourceId") != "csb" or
        startup.get("phase") != "inactive" or
        startup.get("startupActive") != 0 or startup.get("levelLoaded") != 1 or
        probe.get("csbViewportHash", 0) == 0 or actual_party != expected_party):
    raise SystemExit(
        f"FAIL: explicit {platform} M12 did not reach its authentic first runtime: {probe}")

print(f"PASS: mixed authentic CSB M12 --platform {platform} retained {edition} "
      "and reached its source-owned first runtime")
PY
}

explicit_platform_runtime fm-towns "FM Towns" fmtowns-en csb-fmtowns-en \
    f31-title-anm "$amiga_media_root/$(basename "$towns_archive")" \
    'enter,enter,enter,wait700,click:52:110,wait10,click:250:50,wait240' \
    '4,22,18,2,1' 320 200 30000
explicit_platform_runtime atari-st "Atari ST" st20-21-en csb-st20-21-en \
    atari-st-animate-ftlcode "$amiga_media_root/$(basename "$atari_archive")" \
    'enter,enter,enter,wait:3600,click:813:156' '0,9,0,2,0' 960 600 120000
explicit_platform_runtime amiga "Amiga" amiga31-multi csb-amiga-a31m \
    a31m-titl-dat "$amiga_media_root/$(basename "$amiga_archive")" \
    'enter,enter,enter,wait:1000,click:100:100,key:enter,up' '0,9,0,2,0' 320 200 30000

# Direct CLI selection must keep the requested source edition when other
# authentic CSB platforms are installed beside it. The platform-specific
# tests above use --menu; this small boot-probe matrix covers the separate
# no-launcher `--game csb --platform ...` route without fabricating media or
# advancing any title with synthetic input.
mixed_direct_cli_platform() {
    platform=$1
    display_platform=$2
    edition=$3
    variant=$4
    handoff=$5
    handoff_hash=$6
    animation=$7
    asset_md5=$8
    archive=$9
    source_path=${10}
    key=$(printf '%s' "$platform" | tr '[:upper:]' '[:lower:]' | tr -cd 'a-z0-9-')
    runtime_log="$scratch/$key-direct-cli.log"
    case "$platform" in
        fm-towns) startup_phase=csb-fmtowns-title ;;
        atari-st) startup_phase=csb-atari-st-animation ;;
        amiga) startup_phase=csb-amiga-a31-titl ;;
        *) echo "FAIL: unsupported CSB direct CLI platform: $platform" >&2; return 1 ;;
    esac

    FIRESTAFF_CONFIG_PATH="$scratch/$key-direct-cli-config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    HOME="$scratch" XDG_CONFIG_HOME="$scratch" APPDATA="$scratch" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --width 320 --height 200 --game csb \
        --data-dir "$amiga_media_root" --platform "$platform" --debug \
        --boot-probe --boot-probe-frames 2 \
        --boot-probe-expect-startup-active 1 --duration 0 \
        >"$runtime_log" 2>&1 || {
            cat "$runtime_log" >&2
            return 1
        }

    python3 - "$runtime_log" "$display_platform" "$edition" "$variant" \
        "$handoff" "$handoff_hash" "$startup_phase" "$animation" "$asset_md5" \
        "$archive" "$source_path" <<'PY'
import sys

(log_path, platform, edition, variant, handoff, handoff_hash, startup_phase,
 animation, asset_md5, archive, source_path) = sys.argv[1:]
with open(log_path, encoding="utf-8") as stream:
    trace = stream.read()

checks = (
    (f"CSB READY: gameId=csb dataDir={archive} variant={variant} "
     f"route=startup handoff={handoff} handoffHash={handoff_hash}" in trace,
     "authenticated source handoff"),
    (f"launch phase=game-handoff mode=direct game=csb "
     f"platform={platform} edition={edition} source={source_path}" in trace,
     "direct CLI source selection"),
    (f"phase={startup_phase}" in trace, "source-owned startup phase"),
    (f"startupAnimation={animation} startupAnimationActive=1" in trace,
     f"{animation} startup animation"),
    (f"assetMd5={asset_md5}", "authentic asset identity"),
    ("startupActive=1", "active original startup"),
    ("levelLoaded=0", "no premature campaign load"),
)
for result, label in checks:
    if not result:
        raise SystemExit(f"FAIL: CSB {platform} direct CLI lost {label}:\n{trace}")

print(f"PASS: mixed authentic CSB direct CLI selected {edition} on {platform} "
      "and reached its source-owned startup phase")
PY
}

towns_archive_link="$amiga_media_root/$(basename "$towns_archive")"
atari_archive_link="$amiga_media_root/$(basename "$atari_archive")"
amiga_archive_link="$amiga_media_root/$(basename "$amiga_archive")"
towns_source="$towns_archive_link::CDATA/GRAPHICS.DAT"
atari_source="$atari_archive_link::Chaos Strikes Back for Atari ST v2.1 (English).zip::Chaos Strikes Back for Atari ST Game Disk v2.1 (English).stx::GRAPHICS.DAT"
amiga_source="$amiga_archive_link::Graphics.DAT"
mixed_direct_cli_platform fm-towns "FM Towns" fmtowns-en csb-fmtowns-en \
    f31-title-anm 6fc7d5b9 title-anm 405b757038eea3c263e60f240854d6de \
    "$towns_archive_link" "$towns_source"
mixed_direct_cli_platform atari-st "Atari ST" st20-21-en csb-st20-21-en \
    atari-st-animate-ftlcode d2818858 animate-scr ebf6a57af3f27782e358c0490bfd2f2e \
    "$atari_archive_link" "$atari_source"
mixed_direct_cli_platform amiga "Amiga" amiga31-multi csb-amiga-a31m \
    a31m-titl-dat 2601fbe2 titl-dat 61fbfd56887c94adc26888a9491c6611 \
    "$amiga_archive_link" "$amiga_source"

# Bare direct launch must apply the same FM Towns AUTO preference as the
# launcher when competing authentic editions are installed. Keep this
# separate from the explicit platform cases above so a CLI default regression
# cannot be hidden by a caller-supplied --platform.
auto_cli_log="$scratch/auto-direct-cli.log"
FIRESTAFF_CONFIG_PATH="$scratch/auto-direct-cli-config.toml" \
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
HOME="$scratch" XDG_CONFIG_HOME="$scratch" APPDATA="$scratch" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --width 320 --height 200 --game csb \
    --data-dir "$amiga_media_root" --debug \
    --boot-probe --boot-probe-frames 2 \
    --boot-probe-expect-startup-active 1 --duration 0 \
    >"$auto_cli_log" 2>&1 || {
        cat "$auto_cli_log" >&2
        exit 1
    }
python3 - "$auto_cli_log" "$towns_archive_link" "$towns_source" <<'PY'
import sys

log_path, archive, source_path = sys.argv[1:]
with open(log_path, encoding="utf-8") as stream:
    trace = stream.read()

checks = (
    (f"launch phase=game-handoff mode=direct game=csb "
     f"platform=FM Towns edition=fmtowns-en source={source_path}" in trace,
     "FM Towns AUTO selection"),
    (f"CSB READY: gameId=csb dataDir={archive} variant=csb-fmtowns-en "
     "route=startup handoff=f31-title-anm handoffHash=6fc7d5b9" in trace,
     "authentic F31 startup handoff"),
    ("assetMd5=405b757038eea3c263e60f240854d6de" in trace,
     "original FM Towns graphics identity"),
    ("phase=csb-fmtowns-title" in trace and "startupActive=1" in trace and
     "levelLoaded=0" in trace,
     "source-owned F31 title startup state"),
)
for result, label in checks:
    if not result:
        raise SystemExit(f"FAIL: bare CSB direct CLI lost {label}:\n{trace}")

print("PASS: bare mixed-media CSB direct CLI selected authentic FM Towns and reached F31 title")
PY

# A normal installation stores each game's originals under the per-user
# .firestaff/data tree. Verify direct CLI startup discovers that root without
# either --data-dir or --platform, even when several real CSB editions exist.
default_home="$scratch/default-cli-home"
default_game_root="$default_home/.firestaff/data/csb"
mkdir -p "$default_home/.firestaff/data"
ln -s "$collection_root" "$default_game_root"
default_cli_log="$scratch/default-root-direct-cli.log"
archive_sha_before=$(firestaff_sha256_file "$towns_archive")
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR
    FIRESTAFF_CONFIG_PATH="$default_home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    HOME="$default_home" XDG_CONFIG_HOME="$default_home" APPDATA="$default_home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --width 320 --height 200 --game csb --debug --verbose \
        --boot-probe --boot-probe-frames 2 \
        --boot-probe-expect-startup-active 1 --duration 0 \
        >"$default_cli_log" 2>&1 || {
            cat "$default_cli_log" >&2
            exit 1
        }
)
python3 - "$default_cli_log" "$default_game_root" "$(basename "$towns_archive")" <<'PY'
import sys

log_path, game_root, archive_name = sys.argv[1:]
with open(log_path, encoding="utf-8") as stream:
    trace = stream.read()

archive = f"{game_root}/{archive_name}"
source_path = f"{archive}::CDATA/GRAPHICS.DAT"
checks = (
    (f"launch phase=game-handoff mode=direct game=csb "
     f"platform=FM Towns edition=fmtowns-en source={source_path}" in trace,
     "default per-user data root and FM Towns AUTO selection"),
    (f"CSB READY: gameId=csb dataDir={archive} variant=csb-fmtowns-en "
     "route=startup handoff=f31-title-anm handoffHash=6fc7d5b9" in trace,
     "authentic F31 startup handoff"),
    ("assetMd5=405b757038eea3c263e60f240854d6de" in trace,
     "original FM Towns graphics identity"),
    ("phase=csb-fmtowns-title" in trace and "startupActive=1" in trace and
     "levelLoaded=0" in trace,
     "source-owned F31 title startup state"),
)
for result, label in checks:
    if not result:
        raise SystemExit(f"FAIL: default-root CSB direct CLI lost {label}:\n{trace}")

print("PASS: CSB direct CLI discovered authentic FM Towns in the default per-user data root")
PY
archive_sha_after=$(firestaff_sha256_file "$towns_archive")
if [ "$archive_sha_before" != "$archive_sha_after" ]; then
    echo 'FAIL: CSB direct CLI modified the original FM Towns archive' >&2
    exit 1
fi
atari_utility_sha_after=$(firestaff_sha256_file "$atari_utility_archive")
if [ "$atari_utility_sha_before" != "$atari_utility_sha_after" ]; then
    echo 'FAIL: CSB runtime modified the original Atari ST utility archive' >&2
    exit 1
fi
echo 'PASS: CSB default-root direct CLI left original game data unchanged'
