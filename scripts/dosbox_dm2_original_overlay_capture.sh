#!/usr/bin/env bash
# Pass H2313 — DM2 original-overlay reference capture scaffold.
#
# Honest scope:
#   This script is the bounded source/runtime scaffold for future original-vs-
#   Firestaff DM2 overlay evidence. It stages the DM2 PC 1.0 EN canonical
#   archive (Dungeon-Master-II-Skullkeep_DOS_EN.zip) into a scratch tree,
#   generates a deterministic DOSBox config (DOS4GW/4DOS, fixed cycles, no
#   dynamic core), wires an explicit route-injection helper (Swift on macOS,
#   xdotool on X11/Linux), and normalizes 320x200 raw screenshots into
#   224x136 DM2 viewport crops plus 320x200 full-frame receipts.
#
#   It deliberately refuses to inject an unverified DM2 route: the script
#   only emits shots/keystrokes that the operator has explicitly validated.
#   The pass80-style classifier, the semantic route audit, and the source-lock
#   verifier gate are layered on top, not duplicated here.
#
# Why this is a scaffold and not a claim:
#   - DM2's DOS binary is a DOS4GW protected-mode executable (skull.exe +
#     dos4gw.exe), so the DOSBox config must reserve enough base memory and
#     keep the cycles deterministic.
#   - DM2's PC 1.0 EN first launch enters the Interplay splash, the press-any-
#     key intro, and then the main menu. There is no proven DM2 "dungeon_gameplay"
#     route yet on this host. This script records labeled shots and writes
#     honest rawshot health + classifier input without inventing routes.
#   - The status remains OPEN-BOUNDED in FIRESTAFF_GAP_LIST.md until paired
#     dungeon_gameplay viewport rows from both original and Firestaff are
#     promoted with hashes that match the documented route.
#
# DM2_BLIT_SPECIALEFFECTS_HONEST_BOUNDARY
#   This script preserves the SKULLWIN c_gui_vp.cpp DM2_QUERY_BLIT_RECT and
#   DM2_blit_specialeffects region semantics by keeping the 224x136 viewport
#   crop coordinates (x=0..223, y=33..169) and the original 320x200 frame
#   receipt. It does NOT claim overlay parity with Firestaff: see
#   docs/FIRESTAFF_GAP_LIST.md (DM2 original-overlay evidence is OPEN-BOUNDED).
#
# Reference materials the verifier ties this script to:
#   - docs/dm2_viewport.md (RG1R/RG2R/RG3R region queries, 320x136 viewport,
#     224x136 backbuffer).
#   - docs/dm2_hud.md (right-panel system, three-stat panel, portrait classes).
#   - docs/dm2_input.md (SDL mouse + keyboard path; original SKULLWIN routes
#     use c_Tmouse + IBMIO_USER_INPUT_CHECK).
#   - docs/dm2_source_lock.md (archive/member identities for the DM2 PC EN
#     canonical assets).
#   - tools/verify_dm2_v1_original_overlay_capture_source_lock.py
#     (source-lock verifier; this script satisfies the route_tool_* rows).
#
# Usage:
#   scripts/dosbox_dm2_original_overlay_capture.sh --prepare      # write helpers
#   scripts/dosbox_dm2_original_overlay_capture.sh --dry-run       # show plan
#   scripts/dosbox_dm2_original_overlay_capture.sh --preflight-route
#                                                            # validate route shape
#   scripts/dosbox_dm2_original_overlay_capture.sh --run          # launch + capture
#   scripts/dosbox_dm2_original_overlay_capture.sh --normalize-only
#                                                            # crop existing rawshots
#   scripts/dosbox_dm2_original_overlay_capture.sh --print-route-template
#                                                            # emit a route skeleton
#
# Environment overrides:
#   DM2_ORIGINAL_STAGE_DIR=/path/to/staged/Skullkeep
#        override the default stage derived from
#        ~/.openclaw/data/firestaff-original-games/DM/Dungeon-Master-II-Skullkeep_DOS_EN.zip
#   DM2_ORIGINAL_ARCHIVE=/path/to/Skullkeep.zip
#        override the canonical archive path.
#   DM2_ORIGINAL_PROGRAM='SKULL.EXE'
#        override the DOSBox autoexec launch command.
#   DM2_ORIGINAL_ROUTE_EVENTS='wait:9000 enter wait:1500 shot:title ...'
#        operator-validated DM2 keystroke route. Required for --run.
#   DM2_ORIGINAL_EXPECTED_SHOTS=6
#        required rawshot count (default 6).
#   DOSBOX=/Applications/DOSBox\ Staging.app/Contents/MacOS/dosbox
#        override the DOSBox binary path.
#   WAIT_BEFORE_INPUT_MS=3000  DOSBox warm-up before route injection.
#   NEW_FILE_TIMEOUT_MS=2500    how long to wait for new raw screenshots.

set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ARCHIVE_DEFAULT="${HOME}/.openclaw/data/firestaff-original-games/DM/Dungeon-Master-II-Skullkeep_DOS_EN.zip"
ARCHIVE="${DM2_ORIGINAL_ARCHIVE:-${ARCHIVE_DEFAULT}}"
STAGE_DEFAULT_DEFAULT="${REPO}/verification-screens/dm2-dosbox-capture/SkullkeepPC10EN"
if [[ -z "${DM2_ORIGINAL_STAGE_DIR:-}" && -d "${HOME}/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34" ]]; then
    # Fall back to the legacy extracted tree only if an operator explicitly
    # created it; the canonical Skullkeep tree is staged below.
    STAGE_DEFAULT="${STAGE_DEFAULT_DEFAULT}"
else
    STAGE_DEFAULT="${DM2_ORIGINAL_STAGE_DIR:-${STAGE_DEFAULT_DEFAULT}}"
fi
OUT_DIR="${OUT_DIR:-${REPO}/verification-screens/passH2313-dm2-original-overlays}"
DOSBOX="${DOSBOX:-/Applications/DOSBox Staging.app/Contents/MacOS/dosbox}"
WAIT_BEFORE_INPUT_MS="${WAIT_BEFORE_INPUT_MS:-3000}"
NEW_FILE_TIMEOUT_MS="${NEW_FILE_TIMEOUT_MS:-2500}"
ROUTE_EVENTS="${DM2_ORIGINAL_ROUTE_EVENTS:-}"
EXPECTED_SHOTS="${DM2_ORIGINAL_EXPECTED_SHOTS:-6}"
case "${EXPECTED_SHOTS}" in
    ''|*[!0-9]*) echo "ERROR: DM2_ORIGINAL_EXPECTED_SHOTS must be a positive integer" >&2; exit 2 ;;
    *) EXPECTED_SHOTS_COUNT="${EXPECTED_SHOTS}"
       if [[ "${EXPECTED_SHOTS_COUNT}" -le 0 ]]; then
           echo "ERROR: DM2_ORIGINAL_EXPECTED_SHOTS must be positive" >&2
           exit 2
       fi
       ;;
esac
SKIP_INTRO_SELECTOR="${DM2_ROUTE_SKIP_INTRO:-0}"
ORIGINAL_PROGRAM="${DM2_ORIGINAL_PROGRAM:-SKULL.EXE}"
CONF="${OUT_DIR}/dosbox-dm2-original.conf"
LOG="${OUT_DIR}/dosbox-dm2-original.log"
PID_FILE="${OUT_DIR}/dosbox.pid"
KEY_HELPER="${OUT_DIR}/dm2_original_overlay_route_keys.swift"
KEY_HELPER_XDOTOOL="${OUT_DIR}/dm2_original_overlay_route_keys_xdotool.sh"
KEY_LOG="${OUT_DIR}/dm2-overlay-route-keys.log"
SHOT_LABEL_MANIFEST="${OUT_DIR}/dm2_original_overlay_shot_labels.tsv"
RAW_MANIFEST="${OUT_DIR}/dm2_raw_manifest.tsv"
RAW_HEALTH_MANIFEST="${OUT_DIR}/dm2_raw_frame_health.json"
CROP_MANIFEST="${OUT_DIR}/dm2_viewport_224x136_manifest.tsv"
CROP_DIR="${OUT_DIR}/viewport_224x136"
SCREENSHOT_DIR="${OUT_DIR}/screenshots_320x200"
SIZE_LOG="${OUT_DIR}/dm2_artifact_sizes.txt"
STAGE_LOCK="${OUT_DIR}/stage.sha256"

usage() {
    sed -n '2,80p' "$0"
}

mode="prepare"
while [[ $# -gt 0 ]]; do
    case "$1" in
        --prepare) mode="prepare"; shift ;;
        --dry-run) mode="dry-run"; shift ;;
        --preflight-route) mode="preflight-route"; shift ;;
        --run) mode="run"; shift ;;
        --normalize-only) mode="normalize-only"; shift ;;
        --print-route-template) mode="route-template"; shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "unknown arg: $1" >&2; usage >&2; exit 2 ;;
    esac
done

print_route_template() {
    cat <<'EOF'
# Pass H2313 — DM2 original-overlay route template.
#
# DM2 PC 1.0 EN first-launch sequence (operator-validated only; not promised
# by this scaffold):
#   1. Interplay splash screen
#   2. Press-any-key intro
#   3. Main menu (PRESS FIRE TO START or SELECT AN OPTION)
#   4. Optional save slot selection
#   5. Dungeon gameplay (viewport 224x136 at y=33..169 on 320x200 screen)
#
# Operator-validated routes MUST replace the example tokens below before
# --run. The scaffold refuses to inject unvalidated routes.
#
# Example template (NOT validated; do not commit as evidence):
#   DM2_ORIGINAL_ROUTE_EVENTS='
#       wait:7000 shot:interplay_splash
#       enter wait:2500 shot:press_any_key
#       enter wait:2500 shot:main_menu
#       enter wait:4000 shot:dungeon_entry
#       kp5 wait:1500 shot:dungeon_forward_1
#       kp5 wait:1500 shot:dungeon_forward_2
#   '
#
# Supported route tokens (mirroring scripts/dosbox_dm1_original_viewport_reference_capture.sh):
#   shot, shot:<label>, wait:<ms>, click:<x>,<y>, rclick:<x>,<y>,
#   enter, return, esc, escape, space, up, down, left, right,
#   one, two, three, four, five, six, f1-f4,
#   kp0-kp9, kpenter, a-z, 0-9
#
# shot labels: lowercase route semantics such as shot:interplay_splash,
# shot:press_any_key, shot:main_menu, shot:dungeon_entry,
# shot:dungeon_forward_1, shot:dungeon_forward_2.
EOF
}

need_archive() {
    if [[ ! -f "${ARCHIVE}" ]]; then
        echo "ERROR: DM2 canonical archive not found: ${ARCHIVE}" >&2
        echo "       Set DM2_ORIGINAL_ARCHIVE to override, or extract the" >&2
        echo "       Dungeon-Master-II-Skullkeep_DOS_EN.zip from the local" >&2
        echo "       firestaff-original-games cache." >&2
        exit 3
    fi
}

stage_archive() {
    if [[ ! -d "${STAGE_DEFAULT}" || ! -f "${STAGE_DEFAULT}/SKULL.EXE" ]]; then
        echo "[pass-H2313] staging ${ARCHIVE} -> ${STAGE_DEFAULT}"
        mkdir -p "${STAGE_DEFAULT}"
        # 7zz is the macOS Homebrew 7-Zip CLI; fall back to unzip if missing.
        if command -v 7zz >/dev/null 2>&1; then
            7zz x -y -o"${STAGE_DEFAULT}" "${ARCHIVE}" >/dev/null
        elif command -v unzip >/dev/null 2>&1; then
            unzip -q -o "${ARCHIVE}" -d "${STAGE_DEFAULT}"
        else
            echo "ERROR: neither 7zz nor unzip is available; cannot stage archive" >&2
            exit 4
        fi
    fi
    if [[ ! -f "${STAGE_DEFAULT}/SKULL.EXE" ]]; then
        echo "ERROR: stage tree missing SKULL.EXE after extraction: ${STAGE_DEFAULT}" >&2
        exit 5
    fi
    # Record a stage lock next to the staged bytes so downstream verifiers can
    # prove the staged bytes came from the canonical archive rather than an
    # arbitrary local copy. The lock lives in the stage tree, not the OUT_DIR.
    if command -v shasum >/dev/null 2>&1; then
        local stage_lock="${STAGE_DEFAULT}/stage.sha256"
        ( cd "${STAGE_DEFAULT}" && shasum -a 256 SKULL.EXE skull.cfg data/dungeon.dat data/graphics.dat data/songlist.dat 2>/dev/null ) > "${stage_lock}" || true
    fi
}

need_image_tool() {
    if python3 - <<PY >/dev/null 2>&1
from PIL import Image
PY
    then
        echo pillow
    elif command -v magick >/dev/null 2>&1; then
        echo magick
    elif command -v convert >/dev/null 2>&1; then
        echo convert
    else
        echo "ERROR: ImageMagick (magick/convert) or Python Pillow is required for PNG->PPM viewport crop normalization." >&2
        exit 6
    fi
}

validate_route_shape() {
    if [[ -z "${ROUTE_EVENTS}" ]]; then
        return 0
    fi
    python3 - "${ROUTE_EVENTS}" "${EXPECTED_SHOTS}" <<'PY'
import re
import sys

route = sys.argv[1].split()
expected = int(sys.argv[2])
if expected <= 0:
    raise SystemExit("ERROR: DM2_ORIGINAL_EXPECTED_SHOTS must be positive")

allowed = set("""
shot capture screenshot enter return esc escape space up down left right
one two three four five six zero
""".split())
allowed |= set("abcdefghijklmnopqrstuvwxyz")
allowed |= set("0123456789")
allowed |= {f"kp{i}" for i in range(10)}
allowed |= {f"f{i}" for i in range(1, 5)}
allowed |= {"kpenter"}

shots = 0
labels = []
for token in route:
    low = token.lower()
    if low in {"shot", "capture", "screenshot"}:
        shots += 1
        labels.append("")
        continue
    if low.startswith("shot:"):
        label = low.split(":", 1)[1]
        if not re.fullmatch(r"[a-z0-9][a-z0-9_-]*", label):
            raise SystemExit(f"ERROR: invalid shot label: {token}")
        shots += 1
        labels.append(label)
        continue
    if low.startswith("wait:"):
        if not re.fullmatch(r"wait:[0-9]+", low):
            raise SystemExit(f"ERROR: invalid wait token: {token}")
        continue
    if low.startswith("click:") or low.startswith("rclick:"):
        m = re.fullmatch(r"(?:r?click):([0-9]{1,3}),([0-9]{1-3})", low)
        # tolerate regex-escape quirks by using a simpler parse below
        m2 = re.match(r"^(r?click):([0-9]{1,3}),([0-9]{1,3})$", low)
        if not m2:
            raise SystemExit(f"ERROR: invalid click token: {token}")
        x = int(m2.group(2)); y = int(m2.group(3))
        if not (0 <= x < 320 and 0 <= y < 200):
            raise SystemExit(f"ERROR: click token outside original 320x200 frame: {token}")
        continue
    if low not in allowed:
        raise SystemExit(f"ERROR: unknown route token: {token}")

if shots != expected:
    raise SystemExit(
        f"ERROR: DM2_ORIGINAL_ROUTE_EVENTS must contain exactly {expected} shot or shot:<label> tokens, found {shots}"
    )
pretty = ", ".join(f"{idx + 1:02d}:{label or '(unlabeled)'}" for idx, label in enumerate(labels))
print(f"[pass-H2313] route shape OK: {len(route)} tokens, {shots} shots")
print(f"[pass-H2313] shot label plan: {pretty}")
PY
}

select_route_injector() {
    local uname_s
    local swift_path
    uname_s="$(uname -s 2>/dev/null || true)"
    swift_path="$(command -v swift 2>/dev/null || true)"
    if [[ "${uname_s}" == "Darwin" && -n "${swift_path}" && -x "${swift_path}" ]]; then
        echo "swift"
        return 0
    fi
    if command -v xdotool >/dev/null 2>&1; then
        echo "xdotool"
        return 0
    fi
    return 1
}

preflight_route() {
    if [[ -z "${ROUTE_EVENTS}" ]]; then
        echo "ERROR: DM2_ORIGINAL_ROUTE_EVENTS is required for --preflight-route" >&2
        return 5
    fi
    write_helpers
    validate_route_shape
    local injector
    if ! injector="$(select_route_injector)"; then
        echo "ERROR: no supported route injector found; install Swift on macOS or xdotool on X11/Linux" >&2
        return 7
    fi
    if [[ "${injector}" == "xdotool" && -z "${DISPLAY:-}" ]]; then
        echo "ERROR: xdotool route injector selected but DISPLAY is not set; run under Xvfb or an X server" >&2
        return 7
    fi
    echo "[pass-H2313] selected route injector: ${injector}"
    echo "[pass-H2313] route preflight OK"
}

write_helpers() {
    mkdir -p "${OUT_DIR}" "${CROP_DIR}" "${SCREENSHOT_DIR}"
    # ── DOSBox config ──────────────────────────────────────────────
    # DM2 PC 1.0 EN uses DOS4GW protected mode; reserve enough base memory
    # and pin cycles so screenshots stay deterministic across runs.
    cat > "${CONF}" <<EOF
[sdl]
fullscreen=false
output=opengl

[dosbox]
machine=svga_paradise
memsize=63
captures=${OUT_DIR}

[cpu]
core=normal
cputype=386
cpu_cycles=3000
cycleup=0
cycledown=0

[render]
aspect=false
integer_scaling=false

[mixer]
nosound=true

[speaker]
pcspeaker=false
tandy=off

[capture]
capture_dir=${OUT_DIR}
default_image_capture_formats=raw

[autoexec]
mount c "${STAGE_DEFAULT}"
c:
${ORIGINAL_PROGRAM}
EOF

    # ── Swift route injector (macOS) ────────────────────────────────
    cat > "${KEY_HELPER}" <<'SWIFT'
import Foundation
import CoreGraphics
import ApplicationServices

if CommandLine.arguments.count != 4 {
    fputs("usage: dm2_original_overlay_route_keys.swift PID ROUTE_EVENTS SKIP_INTRO\n", stderr)
    exit(2)
}

guard let pid = pid_t(CommandLine.arguments[1]) else {
    fputs("invalid pid\n", stderr)
    exit(2)
}
let route = CommandLine.arguments[2].split(separator: " ").map(String.init)
let skipIntro = CommandLine.arguments[3] == "1"
let source = CGEventSource(stateID: .hidSystemState)

let keycodes: [String: CGKeyCode] = [
    "a": 0, "s": 1, "d": 2, "f": 3, "h": 4, "g": 5, "z": 6, "x": 7, "c": 8, "v": 9,
    "b": 11, "q": 12, "w": 13, "e": 14, "r": 15, "y": 16, "t": 17,
    "one": 18, "1": 18, "two": 19, "2": 19, "three": 20, "3": 20, "four": 21, "4": 21,
    "six": 22, "6": 22, "five": 23, "5": 23, "zero": 29, "0": 29,
    "o": 31, "u": 32, "i": 34, "p": 35, "l": 37, "j": 38, "k": 40,
    "n": 45, "m": 46,
    "enter": 36, "return": 36, "space": 49, "esc": 53, "escape": 53,
    "f1": 122, "f2": 120, "f3": 99, "f4": 118,
    "left": 123, "right": 124, "down": 125, "up": 126,
    "kp1": 83, "kp2": 84, "kp3": 85, "kp4": 86, "kp5": 87, "kp6": 88,
    "kp7": 89, "kp8": 91, "kp9": 92, "kp0": 82, "kpenter": 76
]

func post(_ key: CGKeyCode, _ down: Bool, flags: CGEventFlags = []) {
    guard let event = CGEvent(keyboardEventSource: source, virtualKey: key, keyDown: down) else { return }
    event.flags = flags
    event.postToPid(pid)
}
func tap(_ key: CGKeyCode, _ delayUs: useconds_t = 120_000) {
    post(key, true)
    usleep(20_000)
    post(key, false)
    usleep(delayUs)
}
func cmdF5() {
    post(55, true, flags: .maskCommand)
    usleep(20_000)
    post(96, true, flags: .maskCommand)
    usleep(20_000)
    post(96, false, flags: .maskCommand)
    usleep(20_000)
    post(55, false)
    usleep(180_000)
}

func dosboxWindowBounds() -> CGRect? {
    let opts: CGWindowListOption = [.optionOnScreenOnly, .excludeDesktopElements]
    guard let windows = CGWindowListCopyWindowInfo(opts, kCGNullWindowID) as? [[String: Any]] else { return nil }
    for window in windows {
        guard let ownerPid = window[kCGWindowOwnerPID as String] as? pid_t, ownerPid == pid else { continue }
        guard let boundsDict = window[kCGWindowBounds as String] as? [String: Any] else { continue }
        guard
            let x = boundsDict["X"] as? CGFloat,
            let y = boundsDict["Y"] as? CGFloat,
            let w = boundsDict["Width"] as? CGFloat,
            let h = boundsDict["Height"] as? CGFloat,
            w > 0, h > 0
        else { continue }
        return CGRect(x: x, y: y, width: w, height: h)
    }
    return nil
}

func clickOriginalFrame(x: Int, y: Int, button: String = "left") {
    guard let bounds = dosboxWindowBounds() else {
        fputs("could not find DOSBox window bounds for click:\(x),\(y)\n", stderr)
        exit(3)
    }
    let contentAspect = 320.0 / 200.0
    var contentW = Double(bounds.width)
    var contentH = contentW / contentAspect
    if contentH > Double(bounds.height) {
        contentH = Double(bounds.height)
        contentW = contentH * contentAspect
    }
    let left = Double(bounds.minX) + (Double(bounds.width) - contentW) / 2.0
    let top = Double(bounds.minY) + (Double(bounds.height) - contentH) / 2.0
    let px = left + ((Double(x) + 0.5) / 320.0) * contentW
    let py = top + ((Double(y) + 0.5) / 200.0) * contentH
    let point = CGPoint(x: px, y: py)
    let cgButton: CGMouseButton = (button == "right") ? .right : .left
    let downType: CGEventType = (button == "right") ? .rightMouseDown : .leftMouseDown
    let upType: CGEventType = (button == "right") ? .rightMouseUp : .leftMouseUp
    guard let down = CGEvent(mouseEventSource: source, mouseType: downType, mouseCursorPosition: point, mouseButton: cgButton),
          let up = CGEvent(mouseEventSource: source, mouseType: upType, mouseCursorPosition: point, mouseButton: cgButton) else { return }
    down.postToPid(pid)
    usleep(45_000)
    up.postToPid(pid)
    print("\(button)-click-mapped \(x),\(y) -> \(Int(px)),\(Int(py)) window=\(Int(bounds.width))x\(Int(bounds.height))")
    usleep(180_000)
}

// DM2's startup does not expose a numbered selector like DM1's; the Interplay
// splash and press-any-key intro absorb the first 1-3 Enter presses depending
// on whether the operator has bound SKIP_INTRO=1 via DM2_ROUTE_SKIP_INTRO.
if !skipIntro {
    // Best-effort intro advance; harmless if intro is already past.
    tap(36) // Enter
}

for token in route {
    let lowerToken = token.lowercased()
    print("route-token \(token)")
    if lowerToken == "shot" || lowerToken == "capture" || lowerToken == "screenshot" || lowerToken.hasPrefix("shot:") {
        cmdF5()
    } else if lowerToken.hasPrefix("wait:") {
        let msText = String(lowerToken.dropFirst("wait:".count))
        guard let ms = UInt32(msText) else {
            fputs("invalid wait token: \(token)\n", stderr)
            exit(2)
        }
        usleep(ms * 1000)
    } else if lowerToken.hasPrefix("click:") || lowerToken.hasPrefix("rclick:") {
        let isRightClick = lowerToken.hasPrefix("rclick:")
        let prefix = isRightClick ? "rclick:" : "click:"
        let coords = lowerToken.dropFirst(prefix.count).split(separator: ",")
        guard coords.count == 2, let x = Int(coords[0]), let y = Int(coords[1]), x >= 0, x < 320, y >= 0, y < 200 else {
            fputs("invalid click token: \(token)\n", stderr)
            exit(2)
        }
        clickOriginalFrame(x: x, y: y, button: isRightClick ? "right" : "left")
    } else if let key = keycodes[lowerToken] {
        tap(key)
    } else {
        fputs("unknown route token: \(token)\n", stderr)
        exit(2)
    }
}
SWIFT

    # ── xdotool route injector (X11/Linux) ──────────────────────────
    cat > "${KEY_HELPER_XDOTOOL}" <<'SH'
#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 3 ]]; then
    echo "usage: dm2_original_overlay_route_keys_xdotool.sh PID ROUTE_EVENTS SKIP_INTRO" >&2
    exit 2
fi

pid="$1"
route_events="$2"
skip_intro="$3"

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
    xdotool key --window "$window" ctrl+F5
    sleep 0.18
}

click_original_frame() {
    local x="$1" y="$2" button="${3:-1}"
    local geom gx gy gw gh px py
    geom="$(xdotool getwindowgeometry --shell "$window")"
    eval "$geom"
    gx="$X"; gy="$Y"; gw="$WIDTH"; gh="$HEIGHT"
    read -r px py < <(python3 - "$gw" "$gh" "$x" "$y" <<'PY'
import sys
gw, gh, x, y = map(float, sys.argv[1:])
content_aspect = 320.0 / 200.0
content_w = gw
content_h = content_w / content_aspect
if content_h > gh:
    content_h = gh
    content_w = content_h * content_aspect
left = (gw - content_w) / 2.0
top = (gh - content_h) / 2.0
px = left + ((x + 0.5) / 320.0) * content_w
py = top + ((y + 0.5) / 200.0) * content_h
print(int(round(px)), int(round(py)))
PY
)
    xdotool mousemove --window "$window" "$px" "$py" click "$button"
    local button_name=left
    if [[ "$button" == "3" ]]; then button_name=right; fi
    echo "${button_name}-click-mapped ${x},${y} -> window-relative ${px},${py} window=${gw}x${gh} origin=${gx},${gy}"
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

if [[ "$skip_intro" != "1" ]]; then
    tap_key Return
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
        click:*|rclick:*)
            if [[ "$low" == rclick:* ]]; then
                coords="${low#rclick:}"
                click_original_frame "${coords%,*}" "${coords#*,}" 3
            else
                coords="${low#click:}"
                click_original_frame "${coords%,*}" "${coords#*,}" 1
            fi
            ;;
        *)
            key="$(key_for_token "$low")" || { echo "unknown route token: $token" >&2; exit 2; }
            tap_key "$key"
            ;;
    esac
done
SH
    chmod +x "${KEY_HELPER_XDOTOOL}"
    echo "[pass-H2313] wrote ${CONF}"
    echo "[pass-H2313] wrote ${KEY_HELPER}"
    echo "[pass-H2313] wrote ${KEY_HELPER_XDOTOOL}"
}

normalize_existing() {
    local image_tool
    image_tool="$(need_image_tool)"
    mkdir -p "${CROP_DIR}" "${SCREENSHOT_DIR}"
    rm -f "${CROP_DIR}"/*.ppm "${CROP_DIR}"/*.png "${SCREENSHOT_DIR}"/*.png \
          "${RAW_MANIFEST}" "${RAW_HEALTH_MANIFEST}" "${CROP_MANIFEST}" \
          "${SHOT_LABEL_MANIFEST}"

    python3 - "${OUT_DIR}" "${RAW_MANIFEST}" "${EXPECTED_SHOTS}" "${SCREENSHOT_DIR}" <<'PY'
from __future__ import annotations
from pathlib import Path
from datetime import datetime, timezone
import hashlib
import struct
import sys

out = Path(sys.argv[1])
manifest = Path(sys.argv[2])
expected = int(sys.argv[3])
shot_dir = Path(sys.argv[4])
if expected <= 0:
    raise SystemExit("ERROR: DM2_ORIGINAL_EXPECTED_SHOTS must be positive")
paths = sorted(out.glob("image*.png"))
if not paths:
    # DOSBox 0.74 / Staging may name screenshots after the active program
    # (skull_NNN.png) instead of imageNNNN.png. Normalize those into stable
    # image000N-raw.png names expected by the downstream DM2 verifier.
    candidates = sorted(
        [p for p in out.glob("*.png") if p.parent == out and not p.name.startswith("image")],
        key=lambda p: (p.stat().st_mtime_ns, p.name),
    )
    if len(candidates) == expected:
        normalized = []
        for idx, src in enumerate(candidates, 1):
            dst = out / f"image{idx:04d}-raw.png"
            dst.write_bytes(src.read_bytes())
            normalized.append(dst)
        paths = normalized
if not paths:
    raise SystemExit(f"ERROR: no DOSBox raw screenshots found under {out}/image*.png")
if len(paths) != expected:
    raise SystemExit(
        f"ERROR: expected exactly {expected} DOSBox raw screenshots under {out}/image*.png, found {len(paths)}"
    )
shot_dir.mkdir(parents=True, exist_ok=True)
with manifest.open("w") as f:
    f.write("index\tpath\tmtime_epoch_ns\tmtime_iso\tsha256\tsize_bytes\twidth\theight\n")
    for i, path in enumerate(paths):
        data = path.read_bytes()
        if data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR":
            raise SystemExit(f"ERROR: not a PNG with IHDR: {path}")
        w, h = struct.unpack(">II", data[16:24])
        if (w, h) == (640, 400):
            try:
                from PIL import Image
            except Exception as exc:
                raise SystemExit(
                    f"ERROR: {path} is a 640x400 DOSBox 2x capture; Python Pillow is required to normalize: {exc}"
                )
            im = Image.open(path).convert("RGB")
            resample = getattr(getattr(Image, "Resampling", Image), "NEAREST")
            im.resize((320, 200), resample).save(path)
            data = path.read_bytes()
            if data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR":
                raise SystemExit(f"ERROR: normalized 2x capture is not a PNG with IHDR: {path}")
            w, h = struct.unpack(">II", data[16:24])
        if (w, h) != (320, 200):
            raise SystemExit(f"ERROR: expected raw screenshot 320x200, got {w}x{h} for {path}")
        st = path.stat()
        iso = datetime.fromtimestamp(st.st_mtime_ns / 1_000_000_000, timezone.utc).isoformat(timespec="microseconds").replace("+00:00", "Z")
        f.write(f"{i:02d}\t{path}\t{st.st_mtime_ns}\t{iso}\t{hashlib.sha256(data).hexdigest()}\t{st.st_size}\t{w}\t{h}\n")
        # Mirror to screenshots_320x200 for stable downstream consumption.
        (shot_dir / path.name).write_bytes(data)
PY

    python3 - "${OUT_DIR}" "${RAW_HEALTH_MANIFEST}" "${image_tool}" "${EXPECTED_SHOTS}" <<'PY'
from __future__ import annotations
from pathlib import Path
import hashlib
import json
import subprocess
import sys

out = Path(sys.argv[1])
manifest = Path(sys.argv[2])
image_tool = sys.argv[3]
expected = int(sys.argv[4])
if expected <= 0:
    raise SystemExit("ERROR: DM2_ORIGINAL_EXPECTED_SHOTS must be positive")

def ppm_pixels(data: bytes, path: Path) -> tuple[tuple[int, int], list[tuple[int, int, int]]]:
    tokens: list[bytes] = []
    i = 0
    n = len(data)
    while len(tokens) < 4 and i < n:
        while i < n and data[i] in b" \t\r\n":
            i += 1
        if i < n and data[i] == ord("#"):
            while i < n and data[i] not in b"\r\n":
                i += 1
            continue
        start = i
        while i < n and data[i] not in b" \t\r\n":
            i += 1
        if start < i:
            tokens.append(data[start:i])
    if len(tokens) < 4 or tokens[0] != b"P6" or tokens[3] != b"255":
        raise SystemExit(f"ERROR: ImageMagick did not produce binary PPM for {path}")
    while i < n and data[i] in b" \t\r\n":
        i += 1
    width = int(tokens[1])
    height = int(tokens[2])
    raw = data[i:]
    if len(raw) != width * height * 3:
        raise SystemExit(f"ERROR: PPM pixel payload size mismatch for {path}")
    pixels = [(raw[j], raw[j + 1], raw[j + 2]) for j in range(0, len(raw), 3)]
    return (width, height), pixels

def load_pixels(path: Path) -> tuple[tuple[int, int], list[tuple[int, int, int]]]:
    if image_tool == "pillow":
        from PIL import Image
        im = Image.open(path).convert("RGB")
        return im.size, list(im.getdata())
    data = subprocess.check_output([image_tool, str(path), "ppm:-"])
    return ppm_pixels(data, path)

paths = sorted(out.glob("image*.png"))
rows = []
problems = []
for idx, path in enumerate(paths, 1):
    dims, pixels = load_pixels(path)
    total = len(pixels)
    nonblack = sum(1 for rgb in pixels if rgb != (0, 0, 0))
    unique = len(set(pixels))
    data = path.read_bytes()
    row = {
        "index": idx,
        "path": str(path),
        "width": dims[0],
        "height": dims[1],
        "sizeBytes": path.stat().st_size,
        "sha256": hashlib.sha256(data).hexdigest(),
        "nonblackRatio": round(nonblack / total, 6),
        "uniqueColors": unique,
    }
    if dims != (320, 200):
        problems.append(f"{path.name}: rawshot dimensions are {dims[0]}x{dims[1]}, expected 320x200")
    if row["nonblackRatio"] <= 0.005 or unique <= 1:
        problems.append(f"{path.name}: black/blank rawshot candidate nonblack={row['nonblackRatio']} uniqueColors={unique}")
    rows.append(row)
payload = {
    "schema": "dm2_original_raw_frame_health.v1",
    "attemptDir": str(out),
    "expectedCaptureCount": expected,
    "captureCount": len(rows),
    "honesty": "Rawshot health gate only. Passing this gate does not claim route semantics or pixel parity.",
    "captures": rows,
    "problems": problems,
    "pass": len(rows) == expected and not problems,
}
manifest.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n")
if not payload["pass"]:
    print(f"ERROR: raw screenshot health gate failed; see {manifest}", file=sys.stderr)
    for problem in problems:
        print(f"ERROR: {problem}", file=sys.stderr)
    raise SystemExit(9)
print(f"[pass-H2313] raw screenshot health OK: {manifest}")
PY

    local route_shot_labels=()
    if [[ -n "${ROUTE_EVENTS}" ]]; then
        local route_labels_tmp
        route_labels_tmp="$(mktemp "${OUT_DIR}/dm2-route-shot-labels.XXXXXX")"
        python3 - "${ROUTE_EVENTS}" > "${route_labels_tmp}" <<'PY'
import sys
for token in sys.argv[1].split():
    low = token.lower()
    if low in {"shot", "capture", "screenshot"}:
        print("")
    elif low.startswith("shot:"):
        print(low.split(":", 1)[1])
PY
        while IFS= read -r route_label || [[ -n "$route_label" ]]; do
            route_shot_labels+=("$route_label")
        done < "${route_labels_tmp}"
        rm -f "${route_labels_tmp}"
        if [[ ${#route_shot_labels[@]} -ne ${EXPECTED_SHOTS_COUNT} ]]; then
            echo "ERROR: expected exactly ${EXPECTED_SHOTS_COUNT} route shot labels, found ${#route_shot_labels[@]}" >&2
            exit 8
        fi
    fi
    printf 'index\tfilename\troute_label\troute_token\n' > "${SHOT_LABEL_MANIFEST}"
    local i=0 src route_label route_token label ppm png
    while IFS= read -r src; do
        route_label="${route_shot_labels[$i]:-}"
        if [[ -n "$route_label" ]]; then
            route_token="shot:${route_label}"
            label="$(python3 - "$((i + 1))" "$route_label" <<'PY'
import re
import sys
idx = int(sys.argv[1])
route_label = sys.argv[2]
stem = re.sub(r"[^a-z0-9_-]+", "_", route_label.lower()).strip("_")
if not stem:
    raise SystemExit("ERROR: empty normalized route label")
print(f"{idx:02d}_{stem}_dm2_original_viewport_224x136")
PY
)"
        else
            route_token="shot"
            label="$(printf '%02d_unlabeled_dm2_original_viewport_224x136' "$((i + 1))")"
        fi
        ppm="${CROP_DIR}/${label}.ppm"
        png="${CROP_DIR}/${label}.png"
        if [[ "${image_tool}" == "pillow" ]]; then
            python3 - "$src" "$ppm" "$png" <<'PY'
from pathlib import Path
from PIL import Image
import sys
src, ppm, png = map(Path, sys.argv[1:4])
im = Image.open(src).convert("RGB")
if im.size != (320, 200):
    raise SystemExit(f"ERROR: expected raw screenshot 320x200, got {im.size[0]}x{im.size[1]} for {src}")
# DM2 viewport occupies x=0..223, y=33..169 (224x136) on the 320x200 frame.
# Same backbuffer geometry as DM1 (c_gfx_main.cpp backbuffer_w=0xe0,
# backbuffer_h=0x88), but the right HUD/right-panel system is larger and
# starts at x>=224, so the crop stays in the viewport zone only.
crop = im.crop((0, 33, 224, 169))
crop.save(ppm)
crop.save(png)
PY
        else
            "${image_tool}" "$src" -crop 224x136+0+33 +repage "$ppm"
            "${image_tool}" "$ppm" "$png" 2>/dev/null || true
        fi
        printf '%02d\t%s\t%s\t%s\n' "$((i + 1))" "${label}.ppm" "$route_label" "$route_token" >> "${SHOT_LABEL_MANIFEST}"
        i=$((i + 1))
    done < <(find "${OUT_DIR}" -maxdepth 1 -type f -name 'image*.png' | sort)

    python3 - "${CROP_DIR}" "${CROP_MANIFEST}" "${EXPECTED_SHOTS}" <<'PY'
from __future__ import annotations
from pathlib import Path
import hashlib
import sys

def ppm_dims(data: bytes, path: Path) -> tuple[int, int]:
    tokens: list[bytes] = []
    i = 0
    n = len(data)
    while len(tokens) < 4 and i < n:
        while i < n and data[i] in b" \t\r\n":
            i += 1
        if i < n and data[i] == ord('#'):
            while i < n and data[i] not in b"\r\n":
                i += 1
            continue
        start = i
        while i < n and data[i] not in b" \t\r\n":
            i += 1
        if start < i:
            tokens.append(data[start:i])
    if len(tokens) < 4 or tokens[0] != b"P6" or tokens[3] != b"255":
        raise SystemExit(f"ERROR: not a binary PPM with maxval 255: {path}")
    return int(tokens[1]), int(tokens[2])

crop_dir = Path(sys.argv[1])
manifest = Path(sys.argv[2])
expected = int(sys.argv[3])
if expected <= 0:
    raise SystemExit("ERROR: DM2_ORIGINAL_EXPECTED_SHOTS must be positive")
paths = sorted(crop_dir.glob("*.ppm"))
if len(paths) != expected:
    raise SystemExit(
        f"ERROR: expected exactly {expected} normalized viewport PPM crops, found {len(paths)} in {crop_dir}"
    )
with manifest.open("w") as f:
    f.write("kind\tfilename\twidth\theight\tbytes\tsha256\n")
    for path in paths:
        data = path.read_bytes()
        width, height = ppm_dims(data, path)
        if (width, height) != (224, 136):
            raise SystemExit(f"ERROR: wrong crop geometry for {path}: {width}x{height}")
        f.write(f"dm2_original_viewport_224x136\t{path.name}\t{width}\t{height}\t{len(data)}\t{hashlib.sha256(data).hexdigest()}\n")
PY
    ls -lh "${RAW_MANIFEST}" "${RAW_HEALTH_MANIFEST}" "${CROP_MANIFEST}" "${SHOT_LABEL_MANIFEST}" "${SCREENSHOT_DIR}"/* "${CROP_DIR}"/* 2>/dev/null | tee "${SIZE_LOG}" || true
    echo "[pass-H2313] normalized DM2 viewport crops: ${CROP_MANIFEST}"
}

case "${mode}" in
    route-template)
        print_route_template
        exit 0
        ;;
    prepare)
        mkdir -p "${OUT_DIR}"
        need_archive
        stage_archive
        write_helpers
        exit 0
        ;;
    dry-run)
        mkdir -p "${OUT_DIR}"
        need_archive
        stage_archive
        write_helpers
        if [[ -z "${ROUTE_EVENTS}" ]]; then
            echo "[blocked] DM2_ORIGINAL_ROUTE_EVENTS is not set. Do not guess the DM2 route; validate before --run."
            echo "          Use --print-route-template for a labelled skeleton."
        else
            echo "[pass-H2313] route events: ${ROUTE_EVENTS}"
            validate_route_shape
        fi
        echo "[pass-H2313] normalize command after raw screenshots exist: scripts/dosbox_dm2_original_overlay_capture.sh --normalize-only"
        exit 0
        ;;
    preflight-route)
        preflight_route
        exit $?
        ;;
    normalize-only)
        normalize_existing
        exit 0
        ;;
    run)
        mkdir -p "${OUT_DIR}"
        need_archive
        stage_archive
        if [[ -z "${ROUTE_EVENTS}" ]]; then
            echo "ERROR: DM2_ORIGINAL_ROUTE_EVENTS is required for --run; refusing to guess the DM2 route." >&2
            echo "        Use --print-route-template for a labelled skeleton and validate against the SKULLWIN source." >&2
            exit 5
        fi
        validate_route_shape
        if [[ ! -x "${DOSBOX}" ]]; then
            echo "ERROR: DOSBox binary not executable: ${DOSBOX}" >&2
            exit 7
        fi
        write_helpers
        injector="$(select_route_injector || true)"
        if [[ "${injector}" == "swift" ]]; then
            route_injector=(swift "${KEY_HELPER}")
        elif [[ "${injector}" == "xdotool" ]]; then
            if [[ -z "${DISPLAY:-}" ]]; then
                echo "ERROR: xdotool route injector selected but DISPLAY is not set; run under Xvfb" >&2
                exit 6
            fi
            route_injector=("${KEY_HELPER_XDOTOOL}")
        else
            echo "ERROR: no supported route injector found; install Swift on macOS or xdotool on X11/Linux" >&2
            exit 6
        fi
        rm -f "${LOG}" "${PID_FILE}" "${KEY_LOG}" "${RAW_MANIFEST}" "${RAW_HEALTH_MANIFEST}" \
              "${CROP_MANIFEST}" "${SIZE_LOG}"
        rm -f "${OUT_DIR}"/*.png "${CROP_DIR}"/*.ppm "${CROP_DIR}"/*.png
        "${DOSBOX}" -conf "${CONF}" >"${LOG}" 2>&1 &
        pid=$!
        echo "${pid}" > "${PID_FILE}"
        cleanup() {
            osascript -e 'tell application "DOSBox Staging" to quit' >/dev/null 2>&1 || true
            kill "${pid}" >/dev/null 2>&1 || true
        }
        trap cleanup EXIT
        sleep "$(python3 - <<PY
print(${WAIT_BEFORE_INPUT_MS}/1000)
PY
)"
        "${route_injector[@]}" "${pid}" "${ROUTE_EVENTS}" "${SKIP_INTRO_SELECTOR}" >"${KEY_LOG}" 2>&1
        python3 - "${OUT_DIR}" "${NEW_FILE_TIMEOUT_MS}" "${EXPECTED_SHOTS_COUNT}" <<'PY'
from pathlib import Path
import sys
import time
out = Path(sys.argv[1])
timeout = int(sys.argv[2]) / 1000.0
expected = int(sys.argv[3])
start = time.monotonic()
while time.monotonic() - start < timeout:
    if len(list(out.glob("image*.png"))) >= expected:
        break
    time.sleep(0.025)
PY
        normalize_existing
        ;;
esac
