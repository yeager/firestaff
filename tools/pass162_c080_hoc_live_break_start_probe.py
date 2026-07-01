#!/usr/bin/env python3
"""Pass162 live HoC C080 probe through DOSBox-X break-start control.

This is the first live stock-original step after the address gate. It mounts a
temporary copy of the staged DM1 PC34 tree, starts DOSBox-X/dosbox-debug with
`-debug -break-start`, arms the pass162 F0359/F0380/F0377/F0275/F0280 BP and
state BPM packet, then drives the Hall-of-Champions portrait route. Promotion is
allowed only if the transcript proves ordered runtime stops. Negative results
are still useful because they name the first missing boundary.
"""
from __future__ import annotations

import argparse
import json
import os
import platform
import re
import shutil
import subprocess
import tempfile
import threading
import time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

import pexpect

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "parity-evidence/verification/pass162_c080_queue_trace/live_hoc_break_start_probe"
REPORT = ROOT / "parity-evidence/pass162_c080_hoc_live_break_start_probe.md"
ORIG = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
COMMANDS_TXT = ROOT / "parity-evidence/verification/pass162_c080_queue_trace/pass162_c080_dosbox_debug_commands.txt"
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"

DEFAULT_ROUTE = "wait:9000 enter wait:2500 click:111,82 wait:1400 click:130,115 wait:1200 enter wait:2200 f1 wait:800 f4"
DEFAULT_LABEL = "hoc"
EXPECTED_ORDER = [
    "F0359_COMMAND_ProcessClick_CPSC",
    "F0380_COMMAND_ProcessQueue_CPSC",
    "F0377_COMMAND_ProcessType80_ClickInDungeonView",
    "F0275_SENSOR_IsTriggeredByClickOnWall",
    "F0280_CHAMPION_AddCandidateChampionToParty",
]
ADDR_TO_SYMBOL = {
    "22F4:030D": "F0359_COMMAND_ProcessClick_CPSC",
    "22F4:0699": "F0380_COMMAND_ProcessQueue_CPSC",
    "1E44:02FE": "F0377_COMMAND_ProcessType80_ClickInDungeonView",
    "1859:1405": "F0275_SENSOR_IsTriggeredByClickOnWall",
    "1782:0031": "F0280_CHAMPION_AddCandidateChampionToParty",
}
SOURCE_CHECKS = [
    ("COMMAND.C", "F0359_COMMAND_ProcessClick_CPSC", ["void F0359_COMMAND_ProcessClick_CPSC", "F0358_COMMAND_GetCommandFromMouseInput_CPSC", "G0432_as_CommandQueue"]),
    ("COMMAND.C", "F0380_COMMAND_ProcessQueue_CPSC", ["void F0380_COMMAND_ProcessQueue_CPSC", "F0377_COMMAND_ProcessType80_ClickInDungeonView"]),
    ("CLIKVIEW.C", "F0377/F0372", ["void F0377_COMMAND_ProcessType80_ClickInDungeonView", "F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor", "F0275_SENSOR_IsTriggeredByClickOnWall"]),
    ("MOVESENS.C", "F0275/C127", ["F0275_SENSOR_IsTriggeredByClickOnWall", "C127_SENSOR_WALL_CHAMPION_PORTRAIT", "F0280_CHAMPION_AddCandidateChampionToParty"]),
    ("REVIVE.C", "F0280", ["void F0280_CHAMPION_AddCandidateChampionToParty", "P0596_ui_ChampionPortraitIndex"]),
]

ANSI_RE = re.compile(r"\x1b\[[0-9;?]*[ -/]*[@-~]")
CONTROL_RE = re.compile(r"[\x00-\x08\x0b\x0c\x0e-\x1f]")
CODE_LINE_RE = re.compile(r"\b(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\s+[0-9A-F]{2,}\s*[a-z][a-z0-9]+", re.I)
BPLIST_RE = re.compile(r"(?:Breakpoint list:|\b\d+\.\s+(?:BP|BPMEM)\s+[0-9A-F]{4}:[0-9A-F]{4}\b|DEBUG: Set (?:memory )?breakpoint at)", re.I)
ENGINE_READY_RE = re.compile(r"\bEXEC:Execute\s+FIRES\b", re.I)
RUNTIME_READY_RE = re.compile(r"file\s+DATA\\DUNGEON\.DAT|file\s+data\\dungeon\.dat", re.I)

SWIFT_HELPER = r"""
import Foundation
import CoreGraphics
import ApplicationServices

if CommandLine.arguments.count != 3 {
    fputs("usage: pass162_route_cgevent.swift PID TOKEN\n", stderr)
    exit(2)
}

guard let pid = pid_t(CommandLine.arguments[1]) else {
    fputs("invalid pid\n", stderr)
    exit(2)
}
let token = CommandLine.arguments[2].lowercased()
let source = CGEventSource(stateID: .hidSystemState)
let mousePostMode = ProcessInfo.processInfo.environment["FIRESTAFF_PASS162_MOUSE_POST_MODE"]?.lowercased() ?? "hid"
let cursorWarpMode = ProcessInfo.processInfo.environment["FIRESTAFF_PASS162_MOUSE_WARP"]?.lowercased() ?? "true"

let keycodes: [String: CGKeyCode] = [
    "enter": 36, "return": 36, "space": 49, "esc": 53, "escape": 53,
    "f1": 122, "f2": 120, "f3": 99, "f4": 118, "f10": 109,
    "left": 123, "right": 124, "down": 125, "up": 126,
    "kp1": 83, "kp2": 84, "kp3": 85, "kp4": 86, "kp5": 87, "kp6": 88,
    "kp7": 89, "kp8": 91, "kp9": 92, "kp0": 82, "kpenter": 76,
    "one": 18, "1": 18, "two": 19, "2": 19, "three": 20, "3": 20,
    "four": 21, "4": 21, "five": 23, "5": 23, "six": 22, "6": 22,
    "zero": 29, "0": 29
]

func post(_ key: CGKeyCode, _ down: Bool) {
    guard let event = CGEvent(keyboardEventSource: source, virtualKey: key, keyDown: down) else { return }
    event.postToPid(pid)
}

func tap(_ key: CGKeyCode) {
    post(key, true)
    usleep(25_000)
    post(key, false)
    usleep(120_000)
}

func postKey(_ key: CGKeyCode, _ down: Bool, flags: CGEventFlags = []) {
    guard let event = CGEvent(keyboardEventSource: source, virtualKey: key, keyDown: down) else { return }
    event.flags = flags
    event.postToPid(pid)
}

func tapCombo(_ key: CGKeyCode, flags: CGEventFlags, label: String) {
    postKey(59, true)
    usleep(20_000)
    postKey(key, true, flags: flags)
    usleep(45_000)
    postKey(key, false, flags: flags)
    usleep(20_000)
    postKey(59, false)
    print("key \(label)")
    usleep(180_000)
}

func runProcess(_ path: String, _ args: [String]) -> Int32 {
    let process = Process()
    process.executableURL = URL(fileURLWithPath: path)
    process.arguments = args
    do {
        try process.run()
        process.waitUntilExit()
        return process.terminationStatus
    } catch {
        return 127
    }
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
        fputs("could not find DOSBox window bounds for \(token)\n", stderr)
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
    if cursorWarpMode == "true" || cursorWarpMode == "1" || cursorWarpMode == "yes" {
        CGWarpMouseCursorPosition(point)
        CGAssociateMouseAndMouseCursorPosition(boolean_t(1))
        usleep(90_000)
    }
    if mousePostMode == "cliclick" {
        let tool = "/opt/homebrew/bin/cliclick"
        let clickCommand = (button == "right" ? "rc" : "c") + ":\(Int(px)),\(Int(py))"
        let moveStatus = runProcess(tool, ["m:\(Int(px)),\(Int(py))"])
        usleep(90_000)
        let clickStatus = runProcess(tool, [clickCommand])
        if moveStatus != 0 || clickStatus != 0 {
            fputs("cliclick failed for \(token): move=\(moveStatus) click=\(clickStatus)\n", stderr)
            exit(5)
        }
        print("\(button)-click-mapped \(x),\(y) -> \(Int(px)),\(Int(py)) window=\(Int(bounds.width))x\(Int(bounds.height)) mouse-post=\(mousePostMode) pre-move=true cursor-warp=\(cursorWarpMode) cliclick=true")
        usleep(180_000)
        return
    }
    if mousePostMode == "systemevents" || mousePostMode == "system-events" {
        if button != "left" {
            fputs("systemevents mode only supports left click for \(token)\n", stderr)
            exit(6)
        }
        let script = "tell application \"System Events\" to click at {\(Int(px)), \(Int(py))}"
        let status = runProcess("/usr/bin/osascript", ["-e", script])
        if status != 0 {
            fputs("System Events click failed for \(token): status=\(status)\n", stderr)
            exit(6)
        }
        print("\(button)-click-mapped \(x),\(y) -> \(Int(px)),\(Int(py)) window=\(Int(bounds.width))x\(Int(bounds.height)) mouse-post=\(mousePostMode) pre-move=false cursor-warp=\(cursorWarpMode) system-events=true")
        usleep(180_000)
        return
    }
    guard let move = CGEvent(mouseEventSource: source, mouseType: .mouseMoved, mouseCursorPosition: point, mouseButton: cgButton),
          let down = CGEvent(mouseEventSource: source, mouseType: downType, mouseCursorPosition: point, mouseButton: cgButton),
          let up = CGEvent(mouseEventSource: source, mouseType: upType, mouseCursorPosition: point, mouseButton: cgButton) else { exit(4) }
    if mousePostMode == "pid" {
        move.postToPid(pid)
    } else {
        move.post(tap: .cghidEventTap)
    }
    usleep(80_000)
    if mousePostMode == "pid" {
        down.postToPid(pid)
    } else {
        down.post(tap: .cghidEventTap)
    }
    usleep(55_000)
    if mousePostMode == "pid" {
        up.postToPid(pid)
    } else {
        up.post(tap: .cghidEventTap)
    }
    print("\(button)-click-mapped \(x),\(y) -> \(Int(px)),\(Int(py)) window=\(Int(bounds.width))x\(Int(bounds.height)) mouse-post=\(mousePostMode) pre-move=true cursor-warp=\(cursorWarpMode)")
    usleep(180_000)
}

if token.hasPrefix("click:") || token.hasPrefix("rclick:") {
    let isRightClick = token.hasPrefix("rclick:")
    let prefix = isRightClick ? "rclick:" : "click:"
    let coords = token.dropFirst(prefix.count).split(separator: ",")
    guard coords.count == 2, let x = Int(coords[0]), let y = Int(coords[1]), x >= 0, x < 320, y >= 0, y < 200 else {
        fputs("invalid click token: \(token)\n", stderr)
        exit(2)
    }
    clickOriginalFrame(x: x, y: y, button: isRightClick ? "right" : "left")
} else if token == "ctrl-f10" {
    tapCombo(109, flags: .maskControl, label: token)
} else if let key = keycodes[token] {
    tap(key)
    print("key \(token)")
} else {
    fputs("unknown route token: \(token)\n", stderr)
    exit(2)
}
"""


def clean(text: str) -> str:
    text = ANSI_RE.sub("", text).replace("\r", "\n")
    return CONTROL_RE.sub("", text)


def drain(child: pexpect.spawn, seconds: float, stop: threading.Event | None = None) -> str:
    out = ""
    deadline = time.time() + seconds
    while time.time() < deadline and not (stop and stop.is_set()):
        try:
            out += child.read_nonblocking(size=8192, timeout=0.05)
        except pexpect.TIMEOUT:
            pass
        except pexpect.EOF:
            out += "<EOF>"
            break
    return out


def source_audit() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for file_name, symbol, needles in SOURCE_CHECKS:
        path = SOURCE_ROOT / file_name
        text = path.read_text(encoding="latin-1", errors="replace") if path.exists() else ""
        compact = " ".join(text.split())
        missing = [needle for needle in needles if " ".join(needle.split()) not in compact]
        rows.append({"file": file_name, "symbol": symbol, "path": str(path), "ok": path.exists() and not missing, "missing": missing})
    return rows


def write_conf(path: Path, stage: Path, log_path: Path) -> None:
    autolock = os.environ.get("FIRESTAFF_PASS162_DOSBOX_AUTOLOCK", "false")
    output = os.environ.get("FIRESTAFF_PASS162_DOSBOX_OUTPUT", "surface")
    mouse_emulation = os.environ.get("FIRESTAFF_PASS162_DOSBOX_MOUSE_EMULATION", "locked")
    usesystemcursor = os.environ.get("FIRESTAFF_PASS162_DOSBOX_USESYSTEMCURSOR", "false")
    clip_mouse_button = os.environ.get("FIRESTAFF_PASS162_DOSBOX_CLIP_MOUSE_BUTTON", "right")
    mouse_log = os.environ.get("FIRESTAFF_PASS162_DOSBOX_MOUSE_LOG", "false")
    path.write_text(
        "\n".join(
            [
                "[sdl]",
                "fullscreen=false",
                f"output={output}",
                f"autolock={autolock}",
                "autolock_feedback=none",
                "middle_unlock=auto",
                f"clip_mouse_button={clip_mouse_button}",
                f"usesystemcursor={usesystemcursor}",
                f"mouse_emulation={mouse_emulation}",
                "usescancodes=false",
                "[log]",
                f"logfile={log_path}",
                f"mouse={mouse_log}",
                "[dosbox]",
                "machine=svga_paradise",
                "memsize=4",
                "[cpu]",
                "core=normal",
                "cycles=3000",
                "[mixer]",
                "nosound=true",
                "[autoexec]",
                f"mount c {stage}",
                "c:",
                "DM -vv -sn -pk",
                "",
            ]
        ),
        encoding="utf-8",
    )


def label_slug(label: str) -> str:
    slug = re.sub(r"[^a-z0-9_]+", "_", label.lower()).strip("_")
    return slug or DEFAULT_LABEL


def output_paths(label: str) -> tuple[Path, Path, str]:
    slug = label_slug(label)
    if slug == DEFAULT_LABEL:
        return OUT, REPORT, slug
    return (
        ROOT / f"parity-evidence/verification/pass162_c080_queue_trace/live_{slug}_break_start_probe",
        ROOT / f"parity-evidence/pass162_c080_{slug}_live_break_start_probe.md",
        slug,
    )


def start_xvfb(out_dir: Path) -> tuple[subprocess.Popen[bytes], str]:
    out_dir.mkdir(parents=True, exist_ok=True)
    log_path = out_dir / "xvfb_startup.log"
    attempts: list[dict[str, Any]] = []
    candidates = list(range(180, 220)) + list(range(88, 100))
    with log_path.open("w", encoding="utf-8") as log:
        for display_no in candidates[:16]:
            socket = Path(f"/tmp/.X11-unix/X{display_no}")
            display = f":{display_no}"
            if socket.exists():
                attempts.append({"display": display, "skipped": "socket exists"})
                continue
            proc = subprocess.Popen(
                ["Xvfb", display, "-screen", "0", "1024x768x24", "-nolisten", "tcp"],
                stdout=log,
                stderr=subprocess.STDOUT,
            )
            for _ in range(20):
                if proc.poll() is not None:
                    break
                if socket.exists():
                    attempts.append({"display": display, "started": True})
                    return proc, display
                time.sleep(0.05)
            attempts.append({"display": display, "poll": proc.poll(), "socket_exists": socket.exists()})
            if proc.poll() is None:
                proc.terminate()
                try:
                    proc.wait(timeout=2)
                except subprocess.TimeoutExpired:
                    proc.kill()
        raise RuntimeError(f"no Xvfb display started; attempts={attempts}; log={log_path.relative_to(ROOT)}")

def xdo(display: str, args: list[str], timeout: int = 10) -> subprocess.CompletedProcess[str]:
    return subprocess.run(["xdotool", *args], env={**os.environ, "DISPLAY": display}, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)


def route_driver() -> str:
    if platform.system() == "Darwin" and shutil.which("swift"):
        return "swift-cgevent"
    return "xdotool-x11"


def write_swift_helper(path: Path) -> None:
    path.write_text(SWIFT_HELPER, encoding="utf-8")


def compile_swift_helper(source: Path, output: Path) -> dict[str, Any]:
    proc = subprocess.run(
        ["swiftc", str(source), "-o", str(output)],
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        timeout=45,
        check=False,
    )
    return {"returncode": proc.returncode, "output": proc.stdout.strip()[-4000:], "executable": str(output)}


def focus_macos_process(pid: int, log: list[dict[str, Any]]) -> None:
    if platform.system() != "Darwin" or not shutil.which("osascript"):
        return
    script = f'tell application "System Events" to set frontmost of first process whose unix id is {pid} to true'
    proc = subprocess.run(["osascript", "-e", script], text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=5, check=False)
    log.append({"event": "macos_focus", "pid": pid, "returncode": proc.returncode, "output": proc.stdout.strip()[-500:], "t": time.time()})


def find_window(display: str, log: list[dict[str, Any]]) -> str | None:
    searches = [
        ["search", "--class", "dosbox"],
        ["search", "--class", "DOSBox"],
        ["search", "--name", "DOSBox"],
        ["search", "--onlyvisible", "--name", "."],
    ]
    deadline = time.time() + 24
    while time.time() < deadline:
        for args in searches:
            try:
                proc = xdo(display, args, timeout=2)
            except subprocess.TimeoutExpired:
                log.append({"event": "window_search_timeout", "args": args, "t": time.time()})
                continue
            ids = [line.strip() for line in proc.stdout.splitlines() if line.strip()]
            details = []
            for window_id in ids[:8]:
                name = xdo(display, ["getwindowname", window_id], timeout=2).stdout.strip()
                klass = xdo(display, ["getwindowclassname", window_id], timeout=2).stdout.strip()
                details.append({"id": window_id, "name": name, "class": klass})
                if "dosbox" in name.lower() or "dosbox" in klass.lower():
                    log.append({"event": "dosbox_window_found", "args": args, "window": window_id, "details": details, "t": time.time()})
                    return window_id
            if details:
                log.append({"event": "window_candidates", "args": args, "details": details, "t": time.time()})
        time.sleep(0.25)
    return None


def pc_to_window(display: str, window: str, x: int, y: int) -> tuple[int, int, dict[str, Any]]:
    ns: dict[str, Any] = {}
    exec(xdo(display, ["getwindowgeometry", "--shell", window]).stdout, {}, ns)
    gw, gh = float(ns["WIDTH"]), float(ns["HEIGHT"])
    aspect = 320 / 200
    cw, ch = gw, gw / aspect
    if ch > gh:
        ch, cw = gh, gh * aspect
    px = int(round((gw - cw) / 2 + ((x + 0.5) / 320) * cw))
    py = int(round((gh - ch) / 2 + ((y + 0.5) / 200) * ch))
    return px, py, {"windowGeometry": {key: ns[key] for key in ("X", "Y", "WIDTH", "HEIGHT") if key in ns}, "pc": [x, y], "window": [px, py]}


def key_name(token: str) -> str:
    return {
        "enter": "Return",
        "return": "Return",
        "f1": "F1",
        "f2": "F2",
        "f3": "F3",
        "f4": "F4",
        "kp4": "KP_Left",
        "kp5": "KP_Begin",
        "kp6": "KP_Right",
        "one": "1",
        "1": "1",
    }[token]


def wait_runtime_ready(runtime_ready: threading.Event, stop: threading.Event, log: list[dict[str, Any]], driver: str) -> bool:
    log.append({"event": "route_wait_runtime_ready", "driver": driver, "t": time.time()})
    deadline = time.time() + 300
    while time.time() < deadline and not stop.is_set():
        if runtime_ready.is_set():
            log.append({"event": "route_runtime_ready", "driver": driver, "t": time.time()})
            return True
        time.sleep(0.05)
    log.append({"event": "route_runtime_ready_timeout", "driver": driver, "t": time.time()})
    return False


def wait_route_ms(ms: int, running: threading.Event, stop: threading.Event, log: list[dict[str, Any]], driver: str, route_item: str) -> None:
    remaining = ms / 1000
    last = time.time()
    log.append({"event": "route_wait_start", "driver": driver, "route_item": route_item, "remaining_seconds": remaining, "t": last})
    while remaining > 0 and not stop.is_set():
        now = time.time()
        if running.is_set():
            remaining -= max(0.0, now - last)
        last = now
        time.sleep(0.05)
    log.append({"event": "route_wait_done", "driver": driver, "route_item": route_item, "t": time.time()})


def route_worker(display: str, route: str, running: threading.Event, runtime_ready: threading.Event, stop: threading.Event, log: list[dict[str, Any]]) -> None:
    try:
        window = find_window(display, log)
        if not window:
            log.append({"event": "no_dosbox_window", "t": time.time()})
            return
    except Exception as exc:
        log.append({"event": "window_search_exception", "error": repr(exc), "t": time.time()})
        return
    xdo(display, ["windowactivate", "--sync", window])
    xdo(display, ["windowfocus", "--sync", window])
    for item in route.split():
        low = item.lower()
        log.append({"event": "route_step", "route_item": item, "t": time.time()})
        if low.startswith("wait:"):
            wait_route_ms(int(low.split(":", 1)[1]), running, stop, log, "xdotool-x11", item)
            continue
        deadline = time.time() + 20
        while time.time() < deadline and not stop.is_set() and not running.is_set():
            time.sleep(0.05)
        if stop.is_set():
            break
        if not running.is_set():
            log.append({"event": "route_skipped_not_running", "route_item": item, "t": time.time()})
            continue
        if low.startswith("click:"):
            if not runtime_ready.is_set() and not wait_runtime_ready(runtime_ready, stop, log, "xdotool-x11"):
                break
            x, y = [int(part) for part in low.split(":", 1)[1].split(",", 1)]
            px, py, geom = pc_to_window(display, window, x, y)
            xdo(display, ["mousemove", "--window", window, str(px), str(py)])
            xdo(display, ["mousedown", "--window", window, "1"])
            time.sleep(0.12)
            xdo(display, ["mouseup", "--window", window, "1"])
            log.append({"event": "click", "route_item": item, "t": time.time(), **geom})
        else:
            xdo(display, ["key", "--window", window, key_name(low)])
            log.append({"event": "key", "route_item": item, "t": time.time()})
        time.sleep(0.2)
    log.append({"event": "route_done", "t": time.time()})


def route_worker_swift(pid: int, helper: Path, route: str, running: threading.Event, runtime_ready: threading.Event, stop: threading.Event, log: list[dict[str, Any]]) -> None:
    focus_macos_process(pid, log)
    log.append({"event": "swift_route_start", "pid": pid, "helper": str(helper), "t": time.time()})
    for item in route.split():
        low = item.lower()
        log.append({"event": "route_step", "driver": "swift-cgevent", "route_item": item, "t": time.time()})
        if low.startswith("wait:"):
            wait_route_ms(int(low.split(":", 1)[1]), running, stop, log, "swift-cgevent", item)
            continue
        deadline = time.time() + 20
        while time.time() < deadline and not stop.is_set() and not running.is_set():
            time.sleep(0.05)
        if stop.is_set():
            break
        if not running.is_set():
            log.append({"event": "route_skipped_not_running", "driver": "swift-cgevent", "route_item": item, "t": time.time()})
            continue
        if low.startswith(("click:", "rclick:")) and not runtime_ready.is_set():
            if not wait_runtime_ready(runtime_ready, stop, log, "swift-cgevent"):
                break
        try:
            proc = subprocess.run(
                [str(helper), str(pid), item],
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                timeout=8,
                check=False,
            )
            event = "click" if low.startswith(("click:", "rclick:")) else "key"
            log.append({
                "event": event,
                "driver": "swift-cgevent",
                "route_item": item,
                "returncode": proc.returncode,
                "output": proc.stdout.strip()[-1000:],
                "t": time.time(),
            })
            if proc.returncode != 0:
                log.append({"event": "route_driver_failed", "driver": "swift-cgevent", "route_item": item, "t": time.time()})
                break
        except Exception as exc:
            log.append({"event": "route_driver_exception", "driver": "swift-cgevent", "route_item": item, "error": repr(exc), "t": time.time()})
            break
        time.sleep(0.2)
    log.append({"event": "route_done", "driver": "swift-cgevent", "t": time.time()})


def load_command_packet() -> list[str]:
    out: list[str] = []
    for raw in COMMANDS_TXT.read_text(encoding="utf-8").splitlines():
        line = raw.split(";", 1)[0].strip()
        if line and line.split()[0].upper() in {"BPDEL", "BP", "BPM", "BPLIST", "CPU"}:
            out.append(line)
    return out


def debugger_command(child: pexpect.spawn, command: str, command_log: list[dict[str, Any]]) -> str:
    child.sendline(command)
    chunk = drain(child, 0.55)
    command_log.append({"t": time.time(), "cmd": command, "excerpt": clean(chunk)[-800:]})
    return chunk


def code_lines(text: str) -> list[str]:
    return [match.group(0)[:160] for match in CODE_LINE_RE.finditer(clean(text))]


def last_code_addr(text: str) -> str | None:
    matches = [match.group("addr").upper() for match in CODE_LINE_RE.finditer(clean(text))]
    return matches[-1] if matches else None


def classify_transition(text: str) -> dict[str, Any] | None:
    cleaned = clean(text)
    if "(Running)" not in cleaned or "->" not in cleaned.split("(Running)", 1)[-1]:
        return None
    post = cleaned.split("(Running)", 1)[-1]
    addr = last_code_addr(post)
    symbol = ADDR_TO_SYMBOL.get(addr or "")
    return {
        "running_marker_seen": True,
        "prompt_reappeared_after_running": True,
        "stop_code_addr_after_running": addr,
        "symbol": symbol,
        "bplist_text_after_running": bool(BPLIST_RE.search(post)),
        "post_running_code_lines": code_lines(post)[-12:],
        "post_running_excerpt": post[-3000:],
    }


def expected_prefix(observed: list[str]) -> list[str]:
    out: list[str] = []
    for symbol in EXPECTED_ORDER:
        if symbol in observed:
            out.append(symbol)
        else:
            break
    return out


def run_probe(seconds: int, route: str, out_dir: Path, route_label: str) -> dict[str, Any]:
    driver = route_driver()
    required_tools = ["dosbox-debug", "swiftc"] if driver == "swift-cgevent" else ["dosbox-debug", "Xvfb", "xdotool"]
    missing = [tool for tool in required_tools if not shutil.which(tool)]
    if missing:
        return {"ran": False, "blocker": "missing tools: " + ", ".join(missing)}
    if not ORIG.exists():
        return {"ran": False, "blocker": f"missing staged original: {ORIG}"}
    if not COMMANDS_TXT.exists():
        return {"ran": False, "blocker": f"missing command packet: {COMMANDS_TXT}"}

    out_dir.mkdir(parents=True, exist_ok=True)
    transcript = ""
    command_log: list[dict[str, Any]] = []
    route_log: list[dict[str, Any]] = []
    stops: list[dict[str, Any]] = []
    running = threading.Event()
    engine_ready = threading.Event()
    runtime_ready = threading.Event()
    stop = threading.Event()
    start = time.time()
    with tempfile.TemporaryDirectory(prefix="firestaff-pass162-live-hoc-") as td:
        stage = Path(td) / "dos"
        shutil.copytree(ORIG, stage)
        conf = Path(td) / "dosbox.conf"
        dosbox_log_path = out_dir / "dosbox_runtime.log"
        try:
            dosbox_log_path.unlink()
        except FileNotFoundError:
            pass
        write_conf(conf, stage, dosbox_log_path)
        display = ""
        xvfb: subprocess.Popen[bytes] | None = None
        swift_source = Path(td) / "pass162_route_cgevent.swift"
        swift_helper = Path(td) / "pass162_route_cgevent"
        if driver == "swift-cgevent":
            write_swift_helper(swift_source)
            compile_result = compile_swift_helper(swift_source, swift_helper)
            command_log.append({"t": time.time(), "tool": "swiftc", **compile_result})
            if compile_result.get("returncode") != 0:
                return {"ran": False, "blocker": "swiftc helper compile failed", "route_driver": driver, "swiftc": compile_result}
        else:
            try:
                xvfb, display = start_xvfb(out_dir)
            except Exception as exc:
                return {"ran": False, "blocker": repr(exc), "route_driver": driver}
        child: pexpect.spawn | None = None
        try:
            child_env = {**os.environ, "TERM": "vt100"}
            if display:
                child_env["DISPLAY"] = display
            else:
                child_env.pop("DISPLAY", None)
            child = pexpect.spawn(
                "dosbox-debug",
                ["-debug", "-break-start", "-conf", str(conf), "-exit", "-o", "quit warning=false"],
                env=child_env,
                encoding="utf-8",
                timeout=2,
                echo=False,
                maxread=8192,
            )
            child.delaybeforesend = 0.05
            time.sleep(2.5)
            transcript += drain(child, 1.0)
            for command in load_command_packet():
                transcript += debugger_command(child, command, command_log)
            bplist_ok = any("Breakpoint list:" in item.get("excerpt", "") for item in command_log if item.get("cmd") == "BPLIST")
            child.send("\x1bOt")
            command_log.append({"t": time.time(), "control": "F5", "bytes": "ESC O t", "purpose": "run after arming pass162 C080 BP/BPM packet"})
            running.set()
            if driver == "swift-cgevent":
                worker = threading.Thread(target=route_worker_swift, args=(child.pid, swift_helper, route, running, runtime_ready, stop, route_log), daemon=True)
            else:
                worker = threading.Thread(target=route_worker, args=(display, route, running, runtime_ready, stop, route_log), daemon=True)
            worker.start()
            buffer = ""
            deadline = time.time() + seconds
            while time.time() < deadline and not stop.is_set():
                chunk = drain(child, 0.25, stop)
                if chunk:
                    transcript += chunk
                    buffer += chunk
                    cleaned = clean(buffer)
                    if "(Running)" in cleaned:
                        running.set()
                    if not engine_ready.is_set() and ENGINE_READY_RE.search(cleaned):
                        engine_ready.set()
                        route_log.append({"event": "engine_ready_signal", "source": "EXEC_FIRES", "t": time.time()})
                    if not runtime_ready.is_set() and RUNTIME_READY_RE.search(cleaned):
                        runtime_ready.set()
                        route_log.append({"event": "runtime_ready_signal", "source": "DATA_DUNGEON_DAT", "t": time.time()})
                    transition = classify_transition(buffer)
                    if transition:
                        running.clear()
                        transition["t"] = time.time()
                        stops.append(transition)
                        transcript += debugger_command(child, "CPU", command_log)
                        transcript += debugger_command(child, "MEMDUMP 2C20:3E7A 96", command_log)
                        if transition.get("symbol") == "F0280_CHAMPION_AddCandidateChampionToParty":
                            break
                        buffer = ""
                        child.send("\x1bOt")
                        command_log.append({"t": time.time(), "control": "F5", "bytes": "ESC O t", "purpose": "continue after stop", "symbol": transition.get("symbol"), "addr": transition.get("stop_code_addr_after_running")})
                        running.set()
                if route_log and route_log[-1].get("event") == "route_done" and not running.is_set():
                    break
                time.sleep(0.05)
            stop.set()
            worker.join(timeout=2)
            duration = round(time.time() - start, 3)
        finally:
            stop.set()
            if child is not None:
                try:
                    transcript += drain(child, 0.5)
                    child.terminate(force=True)
                except Exception:
                    pass
            if xvfb is not None:
                xvfb.terminate()
                try:
                    xvfb.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    xvfb.kill()

    cleaned_transcript = clean(transcript)
    transcript_path = out_dir / f"live_{route_label}_break_start.clean.txt"
    route_path = out_dir / "route_log.json"
    command_path = out_dir / "command_log.json"
    transcript_path.write_text(cleaned_transcript[-500000:] + "\n", encoding="utf-8")
    route_path.write_text(json.dumps(route_log, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    command_path.write_text(json.dumps(command_log, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    observed = [stop_row["symbol"] for stop_row in stops if stop_row.get("symbol")]
    prefix = expected_prefix(observed)
    first_missing = EXPECTED_ORDER[len(prefix)] if len(prefix) < len(EXPECTED_ORDER) else None
    route_window_found = any(row.get("event") == "dosbox_window_found" for row in route_log)
    if driver == "swift-cgevent":
        route_window_found = any(
            row.get("event") == "click" and row.get("returncode") == 0 and "click-mapped" in row.get("output", "")
            for row in route_log
        )
    route_control_ok = any(row.get("event") in {"click", "key", "route_done"} for row in route_log)
    if driver == "swift-cgevent":
        route_control_ok = any(
            row.get("event") in {"click", "key"} and row.get("returncode") == 0
            for row in route_log
        )
    memory_stop_count = sum(1 for stop_row in stops if "Memory breakpoint" in stop_row.get("post_running_excerpt", ""))
    return {
        "ran": True,
        "duration_seconds": duration,
        "bounded_seconds": seconds,
        "method": "DOSBox-X/dosbox-debug -debug -break-start with owned PTY and platform route driver",
        "route_driver": driver,
        "route_label": route_label,
        "route": route,
        "mouse_post_mode": os.environ.get("FIRESTAFF_PASS162_MOUSE_POST_MODE", "hid"),
        "mouse_warp": os.environ.get("FIRESTAFF_PASS162_MOUSE_WARP", "true"),
        "dosbox_mouse_config": {
            "autolock": os.environ.get("FIRESTAFF_PASS162_DOSBOX_AUTOLOCK", "false"),
            "output": os.environ.get("FIRESTAFF_PASS162_DOSBOX_OUTPUT", "surface"),
            "mouse_emulation": os.environ.get("FIRESTAFF_PASS162_DOSBOX_MOUSE_EMULATION", "locked"),
            "usesystemcursor": os.environ.get("FIRESTAFF_PASS162_DOSBOX_USESYSTEMCURSOR", "false"),
            "clip_mouse_button": os.environ.get("FIRESTAFF_PASS162_DOSBOX_CLIP_MOUSE_BUTTON", "right"),
            "mouse_log": os.environ.get("FIRESTAFF_PASS162_DOSBOX_MOUSE_LOG", "false"),
        },
        "bplist_ok": bplist_ok,
        "stops": stops,
        "observed_symbols": observed,
        "ordered_prefix": prefix,
        "first_missing_expected_symbol": first_missing,
        "reached_f0280": "F0280_CHAMPION_AddCandidateChampionToParty" in observed,
        "route_window_found": route_window_found,
        "route_control_ok": route_control_ok,
        "engine_ready_seen": engine_ready.is_set(),
        "runtime_ready_seen": runtime_ready.is_set(),
        "memory_stop_count": memory_stop_count,
        "route_log": str(route_path.relative_to(ROOT)),
        "command_log": str(command_path.relative_to(ROOT)),
        "transcript": str(transcript_path.relative_to(ROOT)),
        "dosbox_log": str(dosbox_log_path.relative_to(ROOT)) if dosbox_log_path.exists() else None,
    }


def classify(runtime: dict[str, Any], source: list[dict[str, Any]], route_label: str) -> tuple[str, str]:
    is_hoc = route_label == DEFAULT_LABEL
    route_name = "HoC" if is_hoc else route_label.replace("_", " ")
    status_prefix = "PASS162_LIVE_HOC" if is_hoc else f"PASS162_LIVE_{route_label.upper()}"
    blocked_prefix = "BLOCKED_PASS162_LIVE_HOC" if is_hoc else f"BLOCKED_PASS162_LIVE_{route_label.upper()}"
    if not all(row.get("ok") for row in source):
        return f"FAIL_{status_prefix}_SOURCE_AUDIT", f"ReDMCSB source audit failed for the C080/{route_name} chain."
    if not runtime.get("ran"):
        return f"{blocked_prefix}_NOT_RUN", runtime.get("blocker", "runtime did not run")
    if runtime.get("bplist_ok") and runtime.get("engine_ready_seen") and not runtime.get("runtime_ready_seen"):
        return (
            f"{blocked_prefix}_DUNGEON_READY_NOT_REACHED",
            f"Debugger accepted the pass162 BP/BPM packet and FIRES started, but DATA\\DUNGEON.DAT was not observed before the bounded {route_name} click gate.",
        )
    if runtime.get("bplist_ok") and not runtime.get("engine_ready_seen") and not runtime.get("route_window_found"):
        return (
            f"{blocked_prefix}_ENGINE_READY_NOT_REACHED",
            f"Debugger accepted the pass162 BP/BPM packet, but the prelude did not reach FIRES before the bounded {route_name} click gate.",
        )
    if runtime.get("bplist_ok") and runtime.get("memory_stop_count", 0) > 0 and not runtime.get("route_window_found"):
        return (
            f"{blocked_prefix}_WINDOW_CONTROL_NOT_AVAILABLE",
            f"Debugger accepted the pass162 BP/BPM packet and stopped in stock original, but the {runtime.get('route_driver')} route driver could not prove mapped {route_name} window input.",
        )
    if runtime.get("bplist_ok") and runtime.get("memory_stop_count", 0) > 0 and not runtime.get("route_control_ok"):
        return (
            f"{blocked_prefix}_ROUTE_CONTROL_NOT_PROVEN",
            f"Debugger accepted the pass162 BP/BPM packet and stopped in stock original, but the {route_name} route was not driven.",
        )
    if runtime.get("reached_f0280"):
        return f"{status_prefix}_REACHED_F0280", f"Stock original reached F0280 under the pass162 {route_name} C080 break-start probe."
    observed = runtime.get("observed_symbols") or []
    first_missing = runtime.get("first_missing_expected_symbol")
    if observed:
        return f"{blocked_prefix}_PARTIAL_CHAIN", f"Observed {observed}; first missing expected boundary is {first_missing}."
    if runtime.get("bplist_ok"):
        return f"{blocked_prefix}_NO_C080_STOPS", f"Debugger accepted the pass162 BP/BPM packet, but the bounded {route_name} route produced no C080-chain stops."
    return f"{blocked_prefix}_DEBUGGER_PACKET_NOT_RETAINED", "Debugger did not prove the pass162 BP/BPM packet was retained before route execution."


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--seconds", type=int, default=60)
    parser.add_argument("--route", default=DEFAULT_ROUTE)
    parser.add_argument("--label", default=DEFAULT_LABEL, help="artifact label; default keeps legacy HoC paths")
    args = parser.parse_args()
    out_dir, report_path, route_label = output_paths(args.label)
    out_dir.mkdir(parents=True, exist_ok=True)
    source = source_audit()
    runtime = run_probe(max(15, min(args.seconds, 180)), args.route, out_dir, route_label)
    status, summary = classify(runtime, source, route_label)
    manifest = {
        "schema": "pass162_c080_live_break_start_probe.v2",
        "timestamp_utc": datetime.now(timezone.utc).isoformat(),
        "route_label": route_label,
        "status": status,
        "summary": summary,
        "source_audit": source,
        "runtime": runtime,
        "expected_order": EXPECTED_ORDER,
        "non_claims": [
            "does not promote pass435 unless F0280 is reached and later semantic screenshots are captured",
            "does not claim pixel parity",
            "does not use Firestaff renderer output",
        ],
    }
    manifest_path = out_dir / "manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    report_path.write_text(
        "\n".join(
            [
                f"# Pass162 C080 {route_label} live break-start probe",
                "",
                f"Status: `{status}`",
                "",
                summary,
                "",
                "## Runtime",
                "",
                f"- observed symbols: `{runtime.get('observed_symbols')}`",
                f"- ordered prefix: `{runtime.get('ordered_prefix')}`",
                f"- first missing expected symbol: `{runtime.get('first_missing_expected_symbol')}`",
                f"- reached F0280: `{runtime.get('reached_f0280')}`",
                f"- mouse post mode: `{runtime.get('mouse_post_mode')}`",
                f"- mouse warp: `{runtime.get('mouse_warp')}`",
                f"- route window found: `{runtime.get('route_window_found')}`",
                f"- route control ok: `{runtime.get('route_control_ok')}`",
                f"- engine ready seen: `{runtime.get('engine_ready_seen')}`",
                f"- runtime ready seen: `{runtime.get('runtime_ready_seen')}`",
                f"- memory stop count: `{runtime.get('memory_stop_count')}`",
                "",
                "## Artifacts",
                "",
                f"- Manifest: `{manifest_path.relative_to(ROOT)}`",
                f"- Transcript: `{runtime.get('transcript')}`",
                f"- Route log: `{runtime.get('route_log')}`",
                f"- Command log: `{runtime.get('command_log')}`",
                "",
            ]
        ),
        encoding="utf-8",
    )
    print(json.dumps({"status": status, "summary": summary, "manifest": str(manifest_path.relative_to(ROOT)), "runtime": {"observed_symbols": runtime.get("observed_symbols"), "first_missing": runtime.get("first_missing_expected_symbol"), "reached_f0280": runtime.get("reached_f0280"), "route_window_found": runtime.get("route_window_found"), "route_control_ok": runtime.get("route_control_ok"), "engine_ready_seen": runtime.get("engine_ready_seen"), "runtime_ready_seen": runtime.get("runtime_ready_seen"), "memory_stop_count": runtime.get("memory_stop_count")}}, indent=2, sort_keys=True))
    return 0 if status.startswith(("PASS", "BLOCKED")) else 1


if __name__ == "__main__":
    raise SystemExit(main())
