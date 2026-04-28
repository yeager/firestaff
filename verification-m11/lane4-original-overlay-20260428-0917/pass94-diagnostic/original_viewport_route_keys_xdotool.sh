#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 3 ]]; then
    echo "usage: original_viewport_route_keys_xdotool.sh PID ROUTE_EVENTS SKIP_STARTUP_SELECTOR" >&2
    exit 2
fi

pid="$1"
route_events="$2"
skip_startup_selector="$3"

if [[ -z "${DISPLAY:-}" ]]; then
    echo "ERROR: DISPLAY is not set; run DOSBox under an X server, e.g. xvfb-run -a ... --run" >&2
    exit 6
fi

window="$(xdotool search --sync --pid "$pid" | head -n 1 || true)"
if [[ -z "$window" ]]; then
    echo "ERROR: could not find DOSBox X window for pid $pid" >&2
    exit 3
fi
xdotool windowactivate --sync "$window" >/dev/null 2>&1 || true
xdotool windowfocus --sync "$window" >/dev/null 2>&1 || true

tap_key() {
    local key="$1"
    xdotool key --window "$window" "$key"
    sleep 0.12
}

shot() {
    # DOSBox 0.74 on Linux uses Ctrl+F5 for screenshots. DOSBox Staging accepts
    # the same accelerator, and writes to the configured capture directory.
    xdotool key --window "$window" ctrl+F5
    sleep 0.18
}

click_original_frame() {
    local x="$1" y="$2"
    local geom gx gy gw gh px py
    geom="$(xdotool getwindowgeometry --shell "$window")"
    eval "$geom"
    gx="$X"; gy="$Y"; gw="$WIDTH"; gh="$HEIGHT"
    read -r px py < <(python3 - "$gx" "$gy" "$gw" "$gh" "$x" "$y" <<'PY'
import sys
gx, gy, gw, gh, x, y = map(float, sys.argv[1:])
content_aspect = 320.0 / 200.0
content_w = gw
content_h = content_w / content_aspect
if content_h > gh:
    content_h = gh
    content_w = content_h * content_aspect
left = gx + (gw - content_w) / 2.0
top = gy + (gh - content_h) / 2.0
px = left + ((x + 0.5) / 320.0) * content_w
py = top + ((y + 0.5) / 200.0) * content_h
print(int(round(px)), int(round(py)))
PY
)
    xdotool mousemove --window "$window" "$px" "$py" click 1
    echo "click-mapped ${x},${y} -> ${px},${py} window=${gw}x${gh}"
    sleep 0.18
}

key_for_token() {
    case "$1" in
        enter|return) echo Return ;;
        esc|escape) echo Escape ;;
        space) echo space ;;
        up) echo Up ;;
        down) echo Down ;;
        left) echo Left ;;
        right) echo Right ;;
        one|1) echo 1 ;;
        two|2) echo 2 ;;
        three|3) echo 3 ;;
        four|4) echo 4 ;;
        five|5) echo 5 ;;
        six|6) echo 6 ;;
        zero|0) echo 0 ;;
        f1) echo F1 ;;
        f2) echo F2 ;;
        f3) echo F3 ;;
        f4) echo F4 ;;
        kp0) echo KP_Insert ;;
        kp1) echo KP_End ;;
        kp2) echo KP_Down ;;
        kp3) echo KP_Next ;;
        kp4) echo KP_Left ;;
        kp5) echo KP_Begin ;;
        kp6) echo KP_Right ;;
        kp7) echo KP_Home ;;
        kp8) echo KP_Up ;;
        kp9) echo KP_Prior ;;
        kpenter) echo KP_Enter ;;
        [a-z]) echo "$1" ;;
        *) return 1 ;;
    esac
}

if [[ "$skip_startup_selector" != "1" ]]; then
    for _ in 1 2 3; do
        tap_key 1
        tap_key Return
    done
fi

for token in $route_events; do
    low="${token,,}"
    echo "route-token $token"
    case "$low" in
        shot|capture|screenshot|shot:*) shot ;;
        wait:*) sleep "$(python3 - "$low" <<'PY'
import sys
t = sys.argv[1]
print(int(t.split(':', 1)[1]) / 1000.0)
PY
)" ;;
        click:*)
            coords="${low#click:}"
            click_original_frame "${coords%,*}" "${coords#*,}"
            ;;
        *)
            key="$(key_for_token "$low")" || { echo "unknown route token: $token" >&2; exit 2; }
            tap_key "$key"
            ;;
    esac
done
