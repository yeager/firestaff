#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"
app=${1:?Firestaff executable is required}
archive=${FIRESTAFF_DM1_IIGS_SOFTWARE_ARCHIVE:-"$HOME/.firestaff/data/dm1/Game,Dungeon_Master,Apple_IIGS,Software.7z"}
if [[ ! -f "$archive" ]] || ! command -v 7zz >/dev/null 2>&1; then
    echo 'SKIP: original IIGS archive or 7zz unavailable'; exit 77
fi
mkdir -p "$PWD/.codex-scratch"
scratch=$(mktemp -d "$PWD/.codex-scratch/iigs-cli.XXXXXX")
cleanup() {
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else rm -rf "$scratch"; fi
}
trap cleanup EXIT
assert_trace() {
    if ! grep -Fq "$1" "$2"; then
        echo "FAIL: expected trace $1" >&2
        cat "$2" >&2
        exit 1
    fi
}
before=$(firestaff_sha256_file "$archive")
for version in 20 21; do
    member="Dungeon Master for Apple IIGS 2.${version:1:1} (English) Original (Not working).po"
    # Development-only original transport staging; runtime reads in memory.
    7zz x -so "$archive" "$member" >"$scratch/g$version.po"
    case "$version" in
        20) expected=0579ef4e8bada108e85f022a2f2cf1f2e273a7b1b447b6dcc3de316c99dfd5bd ;;
        21) expected=823ee053e95bbf093af51fb88eed534467103914bb5a0c4bd889d87cd9017087 ;;
    esac
    [[ $(firestaff_sha256_file "$scratch/g$version.po") == "$expected" ]]
    FIRESTAFF_CONFIG_PATH="$scratch/g$version-cli.toml" SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --game dm1 --platform apple-iigs --data-dir "$scratch/g$version.po" \
        --debug --boot-probe --boot-probe-frames 4 --script turn-right \
        --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
        --boot-probe-expect-asset-md5 1045a36952e64eb2bc7b4c7b2965b112 >"$scratch/g$version-cli.log" 2>&1 || {
            cat "$scratch/g$version-cli.log"; exit 1;
        }
    assert_trace "platformHandoff=iigs-g$version-native" "$scratch/g$version-cli.log"
    assert_trace 'party=1,3,3' "$scratch/g$version-cli.log"
    FIRESTAFF_CONFIG_PATH="$scratch/g$version-direct.toml" SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
        "$app" --game dm1 --platform apple-iigs --data-dir "$scratch/g$version.po" \
        --debug --duration 3000 >"$scratch/g$version-direct.log" 2>&1 || {
            cat "$scratch/g$version-direct.log"; exit 1;
        }
    assert_trace 'startup-source-frame game=dm1 platform=apple-iigs phase=title-f0437 frame=18/18 input=ignored' "$scratch/g$version-direct.log"
    assert_trace 'startup-source-complete game=dm1 platform=apple-iigs phase=title-f0437 frames=18 cadence-hz=60 sampling=nearest-neighbour-approx' "$scratch/g$version-direct.log"
    assert_trace 'DM1 IIGS READY:' "$scratch/g$version-direct.log"
    title_line=$(grep -n -F 'startup-source-complete game=dm1 platform=apple-iigs phase=title-f0437' "$scratch/g$version-direct.log" | cut -d: -f1)
    world_line=$(grep -n -F 'DM1 IIGS READY:' "$scratch/g$version-direct.log" | cut -d: -f1)
    [[ $title_line -lt $world_line ]] || { echo 'FAIL: IIGS title did not precede world handoff' >&2; exit 1; }
    FIRESTAFF_CONFIG_PATH="$scratch/g$version-entrance.toml" SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        FIRESTAFF_AUTOTEST=1 FIRESTAFF_AUTOTEST_ENTRANCE_INPUT=mouse:enter \
        "$app" --game dm1 --platform apple-iigs --data-dir "$scratch/g$version.po" \
        --debug --duration 3000 >"$scratch/g$version-entrance.log" 2>&1 || {
            cat "$scratch/g$version-entrance.log"; exit 1;
        }
    assert_trace 'phase=entrance-f0441 state=closed input=mouse-only' "$scratch/g$version-entrance.log"
    assert_trace 'viewport=f0439-c255 source=F0441+C004+C002+C003' "$scratch/g$version-entrance.log"
    assert_trace 'startup-input-source mouse x=777 y=140 command=1' "$scratch/g$version-entrance.log"
    assert_trace 'phase=entrance-door frame=31/31 source-vblanks=1' "$scratch/g$version-entrance.log"
    assert_trace "startup-f0441-authenticated-handoff game=dm1 platform=apple-iigs version=$version" "$scratch/g$version-entrance.log"
    (cd "$scratch" && 7zz a -tzip -mx=0 "g$version-original.zip" "g$version.po" >/dev/null)
    FIRESTAFF_CONFIG_PATH="$scratch/g$version-zip.toml" SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --game dm1 --platform apple-iigs --data-dir "$scratch/g$version-original.zip" \
        --debug --boot-probe --boot-probe-frames 4 --script turn-right \
        --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
        --boot-probe-expect-asset-md5 1045a36952e64eb2bc7b4c7b2965b112 \
        >"$scratch/g$version-zip.log" 2>&1 || {
            cat "$scratch/g$version-zip.log"; exit 1;
        }
    assert_trace 'DM1 IIGS READY:' "$scratch/g$version-zip.log"
    assert_trace "platformHandoff=iigs-g$version-native" "$scratch/g$version-zip.log"
    assert_trace 'runtime game=dm1 startup-phase=dm1-iigs-runtime' "$scratch/g$version-zip.log"
    FIRESTAFF_CONFIG_PATH="$scratch/g$version-menu.toml" SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_AUTOTEST=1 \
        FIRESTAFF_AUTOTEST_ENTRANCE_INPUT=mouse:enter \
        FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$scratch/g$version-menu-runtime.json" \
        "$app" --menu --game dm1 --platform iigs --data-dir "$scratch/g$version.po" \
        --debug --script enter,enter,enter --duration 6000 >"$scratch/g$version-menu.log" 2>&1 || {
            cat "$scratch/g$version-menu.log"; exit 1;
        }
    assert_trace 'startup-source-frame game=dm1 platform=apple-iigs phase=title-f0437 frame=18/18 input=ignored' "$scratch/g$version-menu.log"
    assert_trace 'startup-source-complete game=dm1 platform=apple-iigs phase=title-f0437 frames=18 cadence-hz=60 sampling=nearest-neighbour-approx' "$scratch/g$version-menu.log"
    assert_trace "DM1 IIGS READY: version=$version graphicsPath=$scratch/g$version.po::GRAPHICS.GAME" "$scratch/g$version-menu.log"
    assert_trace "startup-f0441-authenticated-handoff game=dm1 platform=apple-iigs version=$version" "$scratch/g$version-menu.log"
    title_line=$(grep -n -F 'startup-source-complete game=dm1 platform=apple-iigs phase=title-f0437' "$scratch/g$version-menu.log" | cut -d: -f1)
    world_line=$(grep -n -F 'DM1 IIGS READY:' "$scratch/g$version-menu.log" | cut -d: -f1)
    [[ $title_line -lt $world_line ]] || { echo 'FAIL: M12 IIGS title did not precede world handoff' >&2; exit 1; }
    python3 - "$version" "$scratch/g$version-menu-runtime.json" <<'PY'
import json
import sys

version, probe_path = sys.argv[1:]
with open(probe_path, encoding="utf-8") as source:
    probe = json.load(source)
startup = probe["startup"]
if (probe.get("launchedEver") != 1 or probe.get("active") != 1 or
        probe.get("sourceId") != "dm1" or startup.get("receiptReady") != 1 or
        startup.get("startupActive") != 0 or startup.get("levelLoaded") != 1 or
        startup.get("phase") != "dm1-iigs-runtime"):
    raise SystemExit(f"FAIL: IIGS G{version} menu handoff missed active dungeon runtime: {probe}")
print(f"PASS: IIGS G{version} M12 menu reached active dungeon runtime")
PY

    # Also cover the first-run route with no game preselected. The extracted
    # original disk image is the only available game source, so the game,
    # Apple IIGS platform, and matching edition must all be discovered and
    # selected through the startup menu before the entrance handoff.
    FIRESTAFF_CONFIG_PATH="$scratch/g$version-no-game.toml" SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_AUTOTEST=1 \
        FIRESTAFF_AUTOTEST_ENTRANCE_INPUT=mouse:enter \
        FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$scratch/g$version-no-game-runtime.json" \
        "$app" --data-dir "$scratch/g$version.po" --debug \
        --script wait-scan-complete,wait20,key:enter,wait20,key:enter,wait20,key:enter \
        --duration 6000 >"$scratch/g$version-no-game.log" 2>&1 || {
            cat "$scratch/g$version-no-game.log"; exit 1;
        }
    assert_trace "launch phase=game-handoff mode=menu game=dm1 platform=Apple IIGS edition=iigs$version-en source=$scratch/g$version.po::GRAPHICS.GAME" \
        "$scratch/g$version-no-game.log"
    assert_trace "startup-f0441-authenticated-handoff game=dm1 platform=apple-iigs version=$version" \
        "$scratch/g$version-no-game.log"
    python3 - "$version" "$scratch/g$version-no-game-runtime.json" <<'PY'
import json
import sys

version, probe_path = sys.argv[1:]
with open(probe_path, encoding="utf-8") as source:
    probe = json.load(source)
startup = probe["startup"]
if (probe.get("launchedEver") != 1 or probe.get("active") != 1 or
        probe.get("sourceId") != "dm1" or startup.get("receiptReady") != 1 or
        startup.get("startupActive") != 0 or startup.get("levelLoaded") != 1 or
        startup.get("phase") != "dm1-iigs-runtime"):
    raise SystemExit(f"FAIL: IIGS G{version} no-game menu missed active dungeon runtime: {probe}")
print(f"PASS: IIGS G{version} no-game menu selected the authentic edition and reached active dungeon runtime")
PY
    [[ $(firestaff_sha256_file "$scratch/g$version.po") == "$expected" ]]
done
towns=${FIRESTAFF_DM1_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_FM-Towns_JA-EN.zip"}
if [[ -f "$towns" ]]; then
    mkdir "$scratch/mixed"
    ln -s "$scratch/g20.po" "$scratch/mixed/iigs.po"
    ln -s "$towns" "$scratch/mixed/towns.zip"
    FIRESTAFF_CONFIG_PATH="$scratch/mixed.toml" SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --game dm1 --data-dir "$scratch/mixed" --debug \
        --boot-probe --boot-probe-frames 2 --boot-probe-expect-runtime \
        --boot-probe-expect-level-loaded 1 \
        --boot-probe-expect-asset-md5 c10c512f63461ebe79b5ac365115b61b >"$scratch/mixed.log" 2>&1 || {
            cat "$scratch/mixed.log"; exit 1;
        }
    assert_trace 'platformHandoff=fmtowns-tmenu-edm' "$scratch/mixed.log"
else
    echo 'SKIP: optional original FM Towns AUTO-priority comparison unavailable'
fi
[[ $(firestaff_sha256_file "$archive") == "$before" ]]
echo 'PASS: original G20/G21 title, ZIP-wrapped CLI startup, no-game and selected M12 menus, F0439 C255 entrance viewport and F0441 mouse-gated door startup reach runtime; GS F0128 pixel parity and GS F0129 remain unverified'
