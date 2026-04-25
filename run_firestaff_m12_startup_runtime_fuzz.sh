#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m12/startup-runtime-fuzz}
PRESENT_DATA=${FIRESTAFF_DATA:-${2:-$HOME/.firestaff/data}}
mkdir -p "$OUT_DIR"

# Build the real launcher/runtime binary through the normal M11 path.
"$HERE/run_firestaff_m11_phase_a_probe.sh" "$OUT_DIR/build" >/dev/null

MISSING_DATA="$OUT_DIR/missing-data"
mkdir -p "$MISSING_DATA"

run_case() {
    name=$1
    data_dir=$2
    script=$3
    duration=${4:-140}
    log="$OUT_DIR/$name.log"
    SDL_AUDIODRIVER=${SDL_AUDIODRIVER:-dummy} \
    SDL_VIDEODRIVER=${SDL_VIDEODRIVER:-dummy} \
        "$HERE/firestaff" \
        --data-dir "$data_dir" \
        --script "$script" \
        --duration "$duration" >"$log" 2>&1
    echo "PASS $name"
}

# Script tokens prefixed with key:/click:/move: are pushed as SDL events,
# so these runs exercise the launcher event pump. Plain legacy tokens are
# kept where we need to cover the direct script-to-menu launch path too.
run_case rapid-key-repeat-events "$MISSING_DATA" \
    "key:down,key:down,key:down,key:down,key:up,key:up,key:invalid,key:f10,key:f11,key:f11,key:escape" 160
run_case nested-settings-back-events "$MISSING_DATA" \
    "key:down,key:down,key:down,key:enter,key:right,key:right,key:invalid,key:escape,key:escape" 180
run_case missing-enter-launch-safe "$MISSING_DATA" \
    "key:enter,key:escape" 120
run_case missing-space-launch-safe "$MISSING_DATA" \
    "space,esc" 120
run_case present-enter-launch "$PRESENT_DATA" \
    "key:enter,key:escape" 160
run_case present-space-launch "$PRESENT_DATA" \
    "space,esc" 160
# Window coords for the default 960x540 window; they map to modern menu
# framebuffer coords near DM1 card center and the game-options Launch button.
run_case present-mouse-click-launch "$PRESENT_DATA" \
    "move:480:270,click:140:300,click:771:460,key:escape" 220

echo "M12 startup runtime fuzz: PASS"
echo "Logs: $OUT_DIR"
