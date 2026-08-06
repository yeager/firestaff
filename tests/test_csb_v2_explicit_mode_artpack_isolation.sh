#!/bin/sh
set -eu

firestaff_bin="${1:?firestaff executable path is required}"
repo_root="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
data_dir="${FIRESTAFF_CSB_PC_DATA:-$HOME/.firestaff/data/csb}"

if [ ! -x "$firestaff_bin" ]; then
    echo "SKIP: firestaff executable is unavailable: $firestaff_bin"
    exit 77
fi
if [ ! -f "$data_dir/GRAPHICS.DAT" ] || [ ! -f "$data_dir/DUNGEON.DAT" ]; then
    echo "SKIP: verified PC CSB data is unavailable: $data_dir"
    exit 77
fi
if ! python3 -c 'import PIL' >/dev/null 2>&1; then
    echo "SKIP: Pillow is unavailable for the source-derived CSB artpack fixture"
    exit 77
fi

test_root="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-explicit-mode-XXXXXX")"
test_home="$test_root/home"
pack_path="$test_root/csb-source.fsart"
config_dir="$test_home/Library/Application Support/Firestaff"
trap 'rm -rf "$test_root"' EXIT HUP INT TERM
mkdir -p "$config_dir"

python3 "$repo_root/scripts/build_csb_v22_source_fsart.py" \
    --graphics-dat "$data_dir/GRAPHICS.DAT" \
    --output "$pack_path" >"$test_root/pack.log" 2>&1

if [ ! -s "$pack_path" ]; then
    cat "$test_root/pack.log" >&2
    echo "FAIL: source-derived CSB V2.2 artpack fixture was not created" >&2
    exit 1
fi

printf '%s\n' \
    'artpack_path = "'"$pack_path"'"' \
    'v22_modern_assets_installed = 1' \
    >"$config_dir/startup-menu.toml"

run_case() {
    mode="$1"
    expected="$2"
    log="$test_root/$mode.log"

    HOME="$test_home" SDL_VIDEODRIVER=dummy "$firestaff_bin" \
        --game csb --data-dir "$data_dir" --presentation-mode "$mode" \
        --boot-probe --width 960 --height 600 --scale-mode 4 \
        --script 'wait120,key:enter,wait200' >"$log" 2>&1

    # The fixed 320-frame boot script reaches tick 149 after the current
    # source-owned CSB startup handoff. Keep this exact value so a timing
    # regression cannot be hidden by the presentation-mode assertion.
    if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=csb' "$log" ||
       ! grep -Fq "presentationMode=$expected" "$log" ||
       ! grep -Fq 'phase=inactive startupActive=0' "$log" ||
       ! grep -Fq 'runtimeTick=149' "$log"; then
        cat "$log" >&2
        echo "FAIL: explicit CSB $mode launch did not retain presentation mode $expected" >&2
        exit 1
    fi
}

# The source-derived fixture deliberately carries unbound F0128 routes. V2.1
# must retain its source-owned 320x200 page and EPX presentation; an explicit
# V2.2 request must resolve back to V2.1 until a package proves every route.
run_case v21 2
run_case v22 2

echo "PASS: an unbound CSB .fsart cannot override V2.1 or falsely admit V2.2"
