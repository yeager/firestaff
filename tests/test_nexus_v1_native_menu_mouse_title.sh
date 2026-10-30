#!/usr/bin/env bash
# Verify the visible Nexus card flow using only pointer input.  The retail
# Saturn CUE/BIN remains in place; a hash-verified extracted corpus takes
# precedence while the CUE is admitted as supplemental media.  The title
# route remains capture-gated, so this test must not claim a usable menu.
set -eu

firestaff_cli="${1:?Firestaff executable is required}"
build_root="${2:?CTest build directory is required}"
data_root="${FIRESTAFF_NEXUS_DATA_DIR:-$HOME/.firestaff/data/nexus}"
extracted_region="$(python3 - "$data_root/RLOWFIX.BIN" <<'PY'
import hashlib
import pathlib
import sys

try:
    digest = hashlib.md5(pathlib.Path(sys.argv[1]).read_bytes()).hexdigest()
except OSError:
    digest = ""
regions = {
    "bb650a4e6f7b6374ba8aa86a61f8f523": "jp",
    "14c3a7e6fed2dc9e53a727640d4c9348": "en",
    "ecbecff383d6ee8330e68e38417be9c8": "fr",
}
print(regions.get(digest, "unknown"))
PY
)"
case "$extracted_region" in
    jp)
        expected_edition=nexus-saturn-jp
        regional_cue="$data_root/Dungeon Master Nexus (Japan).cue"
        regional_iso="$data_root/Dungeon Master Nexus (Japan).iso"
        ;;
    en)
        expected_edition=nexus-saturn-en-extracted
        regional_cue="$data_root/Dungeon Master Nexus (English).cue"
        regional_iso="$data_root/Dungeon Master Nexus (English).iso"
        ;;
    fr)
        expected_edition=nexus-saturn-fr-extracted
        regional_cue="$data_root/Dungeon Master Nexus (French).cue"
        regional_iso="$data_root/Dungeon Master Nexus (French).iso"
        ;;
    *) expected_edition=; regional_cue=; regional_iso= ;;
esac
expected_source=extracted
expected_iso_edition=
if [ -f "$regional_iso" ]; then
    expected_iso_edition="$(python3 - "$regional_iso" <<'PY'
import hashlib
import pathlib
import sys

digest = hashlib.md5(pathlib.Path(sys.argv[1]).read_bytes()).hexdigest()
images = {
    "96e511c8d36ccbe30a48ba36c59df194": "nexus",
    "cf158b32f342c168fc570d36a0f1c637": "nexus-saturn-en-fan-v2",
    "2efb0e8c41f01dea3faa41328ce87f46": "nexus-saturn-fr-fan",
}
print(images.get(digest, ""))
PY
)"
    if [ -n "$expected_iso_edition" ]; then
        expected_source=ISO
    fi
fi
if [ -n "${FIRESTAFF_NEXUS_CUE:-}" ]; then
    cue=$FIRESTAFF_NEXUS_CUE
else
    cue=$regional_cue
fi

if [ ! -x "$firestaff_cli" ] || [ ! -f "$cue" ]; then
    echo "SKIP: authentic Nexus Saturn CUE or Firestaff executable is unavailable"
    exit 77
fi
if [ -z "$expected_edition" ]; then
    echo "FAIL: authentic extracted RLOWFIX.BIN does not identify a known Nexus region" >&2
    exit 1
fi

# Do not inherit an operator's online credentials or launcher preferences.
# Keep the throwaway HOME inside the build tree, never /tmp or the game-data
# tree, and remove it once the test completes.
test_home="$(mktemp -d "$build_root/firestaff-nexus-menu-mouse.XXXXXX")"
cleanup() { rm -rf "$test_home"; }
trap cleanup EXIT HUP INT TERM

unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS
output="$(HOME="$test_home" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_EXIT_AFTER_LAUNCH=1 SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$firestaff_cli" --debug --width 1920 --height 1080 --menu --game nexus \
    --platform saturn --data-dir "$data_root" \
    --script 'wait20,click:700:728,wait20,click:410:405,wait20,click:450:405,wait20' \
    --duration 12000 2>&1)" || {
    printf '%s\n' "$output" >&2
    exit 1
}

expected_handoff_edition=$expected_edition
if [ "$expected_source" = ISO ]; then
    expected_handoff_edition=$expected_iso_edition
fi
if ! grep -Fq 'NEXUS STARTUP RECEIPT: status=blocked gameId=nexus' <<< "$output" ||
   ! grep -Fq 'blocker=title-vdp-owner-compositor-capture-required' <<< "$output" ||
   ! grep -Fq "launch phase=game-handoff mode=menu game=nexus platform=Saturn edition=$expected_handoff_edition" <<< "$output" ||
   grep -Fq 'launch phase=runtime-entered game=nexus' <<< "$output" ||
   ! grep -Fq "Nexus V1 engine initialized (source: $expected_source)" <<< "$output" ||
   ! grep -Fq 'TITLE.CG/4bpp-atlas' <<< "$output"; then
    echo "FAIL: pointer-only Nexus card flow did not reach the authentic capture-gated title route" >&2
    printf '%s\n' "$output" >&2
    exit 1
fi
for diagnostic in \
    'Nexus UI:' \
    'Nexus sound:' \
    'Nexus font:' \
    'Nexus V1 engine initialized' \
    'Nexus launcher:' \
    'NEXUS STARTUP RECEIPT:'; do
    if ! grep -Fq "$diagnostic" <<< "$output"; then
        printf 'FAIL: --debug Nexus launch omitted diagnostic family: %s\n' \
            "$diagnostic" >&2
        printf '%s\n' "$output" >&2
        exit 1
    fi
done
if [ "$expected_source" = ISO ] &&
   ! grep -Fq 'Nexus: opened disc image' <<< "$output"; then
    echo 'FAIL: --debug Nexus ISO launch omitted its source diagnostic' >&2
    printf '%s\n' "$output" >&2
    exit 1
fi
if [ "$expected_source" = extracted ] &&
   ! grep -Fq 'Nexus: using extracted files' <<< "$output"; then
    echo 'FAIL: --debug Nexus extracted owner receipt was omitted' >&2
    printf '%s\n' "$output" >&2
    exit 1
fi
if [ "$expected_source" = extracted ] && {
    ! grep -Fq "Nexus: supplemental retail ISO $regional_cue " <<< "$output" ||
    ! grep -Fq "Nexus: using extracted files from $data_root" <<< "$output";
}; then
    echo "FAIL: extracted Nexus menu did not keep its matching supplemental CUE" >&2
    printf '%s\n' "$output" >&2
    exit 1
fi

echo "PASS: Nexus mouse route reaches the authentic Saturn title capture gate"

mkdir -p "$test_home/quiet"
quiet_output="$(HOME="$test_home/quiet" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_EXIT_AFTER_LAUNCH=1 SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$firestaff_cli" --width 1920 --height 1080 --menu --game nexus \
    --platform saturn --data-dir "$data_root" \
    --script 'wait20,click:700:728,wait20,click:410:405,wait20,click:450:405,wait20' \
    --duration 12000 2>&1)" || {
    printf '%s\n' "$quiet_output" >&2
    exit 1
}
for diagnostic in \
    'Nexus UI:' \
    'Nexus sound:' \
    'Nexus font:' \
    'Nexus V1 engine' \
    'Nexus launcher:' \
    'Nexus: opened disc image' \
    'Nexus: using extracted files' \
    'Nexus palette:' \
    'Nexus texture:' \
    'NEXUS STARTUP RECEIPT:'; do
    if grep -Fq "$diagnostic" <<< "$quiet_output"; then
        printf 'FAIL: non-debug Nexus launch emitted diagnostics: %s\n' \
            "$diagnostic" >&2
        printf '%s\n' "$quiet_output" >&2
        exit 1
    fi
done
if grep -Fq 'launch failed' <<< "$quiet_output" ||
   grep -Fq 'game unavailable for --game' <<< "$quiet_output"; then
    echo 'FAIL: non-debug Nexus launch emitted startup diagnostics or failed' >&2
    printf '%s\n' "$quiet_output" >&2
    exit 1
fi
echo 'PASS: authentic Nexus menu launch keeps startup receipt quiet without --debug'

# Explicit DM.BIN selection can resolve through the extracted tree while
# several regional discs sit beside it. Bind any supplemental disc to the
# extracted locale, not to the first CUE returned by directory iteration.
if [ -f "$data_root/DM.BIN" ] && [ -f "$data_root/RLOWFIX.BIN" ]; then
    direct_home="$test_home/direct-dm-bin"
    mkdir -p "$direct_home"
    direct_output="$(HOME="$direct_home" FIRESTAFF_CONFIG_PATH="$direct_home/config.toml" \
        FIRESTAFF_DATA="$data_root" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$firestaff_cli" --debug --game nexus --platform saturn \
        --data-dir "$data_root/DM.BIN" --boot-probe --boot-probe-frames 0 2>&1)" || {
        printf '%s\n' "$direct_output" >&2
        exit 1
    }
    case "$extracted_region" in
        jp) regional_cue="$data_root/Dungeon Master Nexus (Japan).cue" ;;
        en) regional_cue="$data_root/Dungeon Master Nexus (English).cue" ;;
        fr) regional_cue="$data_root/Dungeon Master Nexus (French).cue" ;;
        *) regional_cue='' ;;
    esac
    if ! grep -Fq 'Nexus V1 engine initialized (source: extracted)' <<< "$direct_output" ||
       ! grep -Fq 'blocker=title-vdp-owner-compositor-capture-required' <<< "$direct_output"; then
        echo "FAIL: direct authentic DM.BIN launch did not retain its extracted title gate" >&2
        printf '%s\n' "$direct_output" >&2
        exit 1
    fi
    if [ -n "$regional_cue" ] && [ -f "$regional_cue" ]; then
        if ! grep -Fq "Nexus: supplemental retail ISO $regional_cue " <<< "$direct_output"; then
            echo "FAIL: Nexus did not bind its supplemental CUE to extracted region $extracted_region" >&2
            printf '%s\n' "$direct_output" >&2
            exit 1
        fi
        echo "PASS: extracted Nexus $extracted_region data selected its matching supplemental CUE"
    elif grep -Fq 'Nexus: supplemental retail ISO ' <<< "$direct_output"; then
        echo "FAIL: Nexus attached supplemental media without an authenticated matching region" >&2
        printf '%s\n' "$direct_output" >&2
        exit 1
    fi

    case "$extracted_region" in
        jp)
            other_cue="$data_root/Dungeon Master Nexus (English).cue"
            other_iso="$data_root/Dungeon Master Nexus (English).iso"
            ;;
        en)
            other_cue="$data_root/Dungeon Master Nexus (Japan).cue"
            other_iso="$data_root/Dungeon Master Nexus (Japan).iso"
            ;;
        fr)
            other_cue="$data_root/Dungeon Master Nexus (Japan).cue"
            other_iso="$data_root/Dungeon Master Nexus (Japan).iso"
            ;;
    esac
    if [ ! -f "$other_cue" ] && [ ! -f "$other_iso" ]; then
        case "$extracted_region" in
            jp)
                other_cue="$data_root/Dungeon Master Nexus (French).cue"
                other_iso="$data_root/Dungeon Master Nexus (French).iso"
                ;;
            en)
                other_cue="$data_root/Dungeon Master Nexus (French).cue"
                other_iso="$data_root/Dungeon Master Nexus (French).iso"
                ;;
            fr)
                other_cue="$data_root/Dungeon Master Nexus (English).cue"
                other_iso="$data_root/Dungeon Master Nexus (English).iso"
                ;;
        esac
    fi
    other_media=
    if [ -f "$other_cue" ]; then
        other_media=$other_cue
    elif [ -f "$other_iso" ]; then
        other_media=$other_iso
    fi
    if [ -n "$other_media" ]; then
        mismatch_root="$test_home/extracted-with-wrong-region-disc"
        mismatch_home="$test_home/wrong-region-home"
        mkdir -p "$mismatch_root" "$mismatch_home"
        for name in DM.BIN LEV01.DGN RLOWFIX.BIN RHIFIX.BIN TITLE.BIN \
                    MENU.BPK FONT256.S2D LOGOBG.DG2; do
            if [ -f "$data_root/$name" ]; then
                ln -s "$data_root/$name" "$mismatch_root/$name"
            fi
        done
        if [ -f "$other_cue" ]; then
            python3 - "$other_cue" "$mismatch_root" <<'PY'
import pathlib
import re
import sys

cue = pathlib.Path(sys.argv[1])
destinations = [pathlib.Path(value) for value in sys.argv[2:]]
for line in cue.read_text(errors="strict").splitlines():
    match = re.match(r'\s*FILE\s+"([^"]+)"', line, re.IGNORECASE)
    if not match:
        continue
    source = cue.parent / match.group(1)
    if not source.is_file():
        raise SystemExit(f"Missing authentic cue track: {source}")
    for destination in destinations:
        target = destination / source.name
        if not target.exists():
            target.symlink_to(source)
PY
            ln -s "$other_cue" "$mismatch_root/$(basename "$other_cue")"
        else
            ln -s "$other_iso" "$mismatch_root/$(basename "$other_iso")"
        fi
        mismatch_output="$(HOME="$mismatch_home" \
            FIRESTAFF_CONFIG_PATH="$mismatch_home/config.toml" \
            FIRESTAFF_DATA="$mismatch_root" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
            SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
            "$firestaff_cli" --debug --game nexus --platform saturn \
            --data-dir "$mismatch_root" --boot-probe --boot-probe-frames 0 2>&1)" || {
            printf '%s\n' "$mismatch_output" >&2
            exit 1
        }
        if grep -Fq 'Nexus: supplemental retail ISO ' <<< "$mismatch_output" ||
           ! grep -Fq 'Nexus: no verified supplemental ISO matches the extracted localized resources' <<< "$mismatch_output"; then
            echo "FAIL: Nexus attached the available CUE despite its mismatched extracted resources" >&2
            printf '%s\n' "$mismatch_output" >&2
            exit 1
        fi
        echo "PASS: mismatched authentic regional media remained unattached"
    fi
else
    echo "SKIP: extracted Nexus media is unavailable for supplemental-region binding"
fi
