#!/usr/bin/env bash
set -eu

firestaff_bin=$1
data_root=$2
scratch_root=${FIRESTAFF_TEST_SCRATCH:-$(dirname "$firestaff_bin")/.codex-scratch}
mkdir -p "$scratch_root"
log_file=$scratch_root/dm1-i34e-selector-direct-launch.$$
wait_log=$scratch_root/dm1-i34e-selector-wait.$$
config_file=$scratch_root/dm1-i34e-selector-wait-config.$$.toml
trap 'rm -f "$log_file" "$wait_log" "$config_file"' EXIT

show_relevant_log() {
    grep -E '^firestaff: startup-source-|^DM1 READY:|^firestaff: launch phase=runtime-entered|^firestaff: game unavailable' \
        "$log_file" || true
}

if ! SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    FIRESTAFF_EXIT_AFTER_LAUNCH=1 "$firestaff_bin" \
    --game dm1 --platform pc --debug --data-dir "$data_root" \
    >"$log_file" 2>&1; then
    show_relevant_log
    echo "FAIL: authenticated I34E direct launch returned nonzero" >&2
    exit 1
fi

require_once() {
    count=$(grep -c "$1" "$log_file" || true)
    if [ "$count" -ne 1 ]; then
        show_relevant_log
        echo "FAIL: expected one trace matching: $1 (found $count)" >&2
        exit 1
    fi
}

require_once 'phase=swsh-intro frames=17 source-vblanks=30'
require_once 'phase=anim-title frames=53 source-counter-units=53'
require_once 'phase=anim-title-erase pixels=64000 frames=64 .*modeled-vblank-hz=70'
require_once 'phase=selector .*entrance=320x200 left-door=105x161 right-door=128x161 selector-view=224x136 frames=32 source-vblanks=32 .*modeled-vblank-hz=70 .*handoff=fires command=enter'
require_once 'DM1 READY: .*handoff=pc-img3'
require_once 'launch phase=runtime-entered game=dm1'

erase_elapsed=$(sed -n 's/.*phase=anim-title-erase pixels=64000 frames=64 elapsed-ms=\([0-9][0-9]*\) modeled-vblank-hz=70.*/\1/p' "$log_file")
if [ -z "$erase_elapsed" ] || [ "$erase_elapsed" -lt 850 ]; then
    show_relevant_log
    echo "FAIL: I34E source-ordered title erase completed too quickly (${erase_elapsed:-missing} ms)" >&2
    exit 1
fi

swsh_line=$(grep -n 'phase=swsh-intro frames=17' "$log_file" | cut -d: -f1)
title_line=$(grep -n 'phase=anim-title frames=53' "$log_file" | cut -d: -f1)
erase_line=$(grep -n 'phase=anim-title-erase pixels=64000 frames=64' "$log_file" | cut -d: -f1)
selector_line=$(grep -n 'phase=selector graphics=' "$log_file" | cut -d: -f1)
fires_line=$(grep -n 'DM1 READY: .*handoff=pc-img3' "$log_file" | cut -d: -f1)
runtime_line=$(grep -n 'launch phase=runtime-entered game=dm1' "$log_file" | cut -d: -f1)
if [ "$swsh_line" -ge "$erase_line" ] ||
   [ "$erase_line" -ge "$title_line" ] ||
   [ "$title_line" -ge "$selector_line" ] ||
   [ "$selector_line" -ge "$fires_line" ] ||
   [ "$fires_line" -ge "$runtime_line" ]; then
    show_relevant_log
    echo "FAIL: I34E startup stages are out of source order" >&2
    exit 1
fi

if grep -E 'phase=(redmcsb-title|shared-entrance)|callback=(F0437|F0441)' \
    "$log_file" >/dev/null; then
    cat "$log_file"
    echo "FAIL: I34E direct launch entered the shared title/entrance path" >&2
    exit 1
fi

# A direct CLI selection is not an Enter command in the original I34E
# selector. Let the finite run expire while the selector is closed and ensure
# the game never starts without a key or click.
if ! FIRESTAFF_CONFIG_PATH="$config_file" \
     SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
     "$firestaff_bin" --game dm1 --platform pc --debug \
         --data-dir "$data_root" --duration 12000 >"$wait_log" 2>&1; then
    cat "$wait_log" >&2
    echo "FAIL: direct I34E selector wait returned nonzero" >&2
    exit 1
fi
if ! grep -Fq 'phase=selector action=await-enter-or-click' "$wait_log" ||
   grep -Eq 'phase=selector-door|DM1 READY:|launch phase=runtime-entered' "$wait_log"; then
    cat "$wait_log" >&2
    echo "FAIL: direct I34E launch opened or entered without user input" >&2
    exit 1
fi

echo "PASS: authenticated I34E SWOOSH -> TITLE -> SELECTOR/F8360 -> FIRES direct launch"
echo "PASS: direct I34E selector remains closed until user input"
