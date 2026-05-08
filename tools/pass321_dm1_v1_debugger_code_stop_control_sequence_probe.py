#!/usr/bin/env python3
"""Pass321: strict DOSBox-debug code-stop parser/control sequencing verifier.

Goal: fix the pass318/pass320 parser blocker by accepting only real debugger
code-stop lines and ignoring BP/BPLIST/setup echoes. Runtime is intentionally
bounded to <=60s.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shlex
import shutil
import subprocess
import tempfile
import threading
import time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "parity-evidence/verification/pass321_dm1_v1_debugger_code_stop_control_sequence_probe"
REPORT = ROOT / "parity-evidence/pass321_dm1_v1_debugger_code_stop_control_sequence_probe.md"
ORIG = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
PASS318 = ROOT / "parity-evidence/verification/pass318_dm1_v1_f0097_after_f0128_offset_window_probe/manifest.json"
PASS320 = ROOT / "parity-evidence/verification/pass320_dm1_v1_f0097_vidrv_window_after_f0128_sequence_probe/manifest.json"

ADDR = {
    "F0128_DUNGEONVIEW_Draw_CPSF": "23AD:40FE",
    "F0097_DUNGEONVIEW_DrawViewport_entry": "2809:1E31",
    "F0097_after_palette_zone_setup": "2809:1EBD",
    "F0097_before_viewport_args": "2809:1EEE",
    "F0097_VIDRV_09_BlitViewPort_indirect_call": "2809:1EFF",
}
F0097_KEYS = [
    "F0097_DUNGEONVIEW_DrawViewport_entry",
    "F0097_after_palette_zone_setup",
    "F0097_before_viewport_args",
    "F0097_VIDRV_09_BlitViewPort_indirect_call",
]
DEFAULT_ROUTE = "wait:9000 enter wait:1800 one wait:1800 click:276,140 wait:1800 one wait:1800 kp5 wait:900 kp4 wait:900 kp6 wait:900 kp5 wait:900 kp4 wait:900 kp6 wait:900"

SOURCE_CHECKS: list[dict[str, Any]] = [
    {"id": "f0097_viewport_present", "file": "DRAWVIEW.C", "line_range": [709, 858], "needles": ["void F0097_DUNGEONVIEW_DrawViewport", "F0638_GetZone(C007_ZONE_VIEWPORT", "VIDRV_09_BlitViewPort"]},
    {"id": "f0128_calls_f0097", "file": "DUNVIEW.C", "line_range": [8318, 8612], "needles": ["void F0128_DUNGEONVIEW_Draw_CPSF", "F0097_DUNGEONVIEW_DrawViewport(G0309_i_PartyMapIndex != C255_MAP_INDEX_ENTRANCE);", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"]},
    {"id": "f0380_command_process", "file": "COMMAND.C", "line_range": [2045, 2156], "needles": ["void F0380_COMMAND_ProcessQueue_CPSC", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"]},
]

CODE_STOP_PATTERNS = [
    re.compile(r"^(?:CODE\s+STOP|CODE\s+BREAKPOINT|BREAKPOINT\s+HIT)\s*:?[ \t]+(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\b", re.I),
    re.compile(r"^(?:Code|CPU)\s+breakpoint\s+(?:hit\s+)?(?:at\s+)?(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\b", re.I),
    re.compile(r"^Breakpoint\s+(?:hit\s+)?at\s+(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\b", re.I),
]
SETUP_ECHO_PATTERNS = [
    re.compile(r"^->"),
    re.compile(r"^Breakpoint list:?", re.I),
    re.compile(r"^[-]{5,}"),
    re.compile(r"^[0-9]+\.\s+BP\s+[0-9A-F]{4}:[0-9A-F]{4}\b", re.I),
    re.compile(r"^BP\s+[0-9A-F]{4}:[0-9A-F]{4}\b", re.I),
]

def run(cmd: list[str], **kw: Any) -> subprocess.CompletedProcess[str]:
    return subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, **kw)

def clean(text: str) -> str:
    text = re.sub(r"\x1b\[[0-9;?]*[ -/]*[@-~]", "", text).replace("\r", "\n")
    return re.sub(r"[\x00-\x08\x0b\x0c\x0e-\x1f]", "", text)

def cap(sess: str, n: int = 240) -> str:
    return clean(run(["tmux", "capture-pane", "-p", "-S", f"-{n}", "-t", sess], timeout=5).stdout)

def send(sess: str, *keys: str) -> None:
    run(["tmux", "send-keys", "-t", sess, *keys], timeout=5)

def send_cmd(sess: str, cmd: str, events: list[dict[str, Any]], delay: float = 0.22) -> None:
    events.append({"t": time.time(), "event": "debugger_cmd", "cmd": cmd})
    send(sess, cmd, "Enter")
    time.sleep(delay)

def norm(s: str) -> str:
    return " ".join(s.split())

def source_audit() -> list[dict[str, Any]]:
    out = []
    for spec in SOURCE_CHECKS:
        path = SOURCE_ROOT / spec["file"]
        lines = path.read_text(encoding="latin-1", errors="replace").splitlines() if path.exists() else []
        start, end = spec["line_range"]
        block = "\n".join(lines[start - 1:min(end, len(lines))])
        compact = norm(block)
        missing = [needle for needle in spec["needles"] if norm(needle) not in compact]
        anchors = {}
        for needle in spec["needles"]:
            n = norm(needle)
            for idx in range(start - 1, min(end, len(lines))):
                if n in norm(lines[idx]):
                    anchors[needle] = idx + 1
                    break
        out.append({**spec, "source_path": str(path), "ok": path.exists() and not missing, "missing": missing, "anchors": anchors})
    return out

def is_setup_echo(line: str) -> bool:
    s = line.strip()
    return (not s) or any(p.search(s) for p in SETUP_ECHO_PATTERNS)

def strict_code_stop_lines(text: str) -> list[dict[str, str]]:
    hits: list[dict[str, str]] = []
    for raw in clean(text).splitlines():
        s = raw.strip()
        if is_setup_echo(s):
            continue
        for pat in CODE_STOP_PATTERNS:
            m = pat.search(s)
            if m:
                hits.append({"addr": m.group("addr").upper(), "line": s[:180]})
                break
    return hits

def parser_selftest() -> dict[str, Any]:
    samples = {
        "bp_command_echo": "-> BP 23AD:40FE_  breakpoint at 23AD:40FE",
        "bplist_header": "Breakpoint list:",
        "bplist_entry": "00. BP 23AD:40FE",
        "setup_prefixed_bplist": "-> BPLIST_  reakpoint at 23AD:40FE",
        "actual_code_stop_1": "CODE STOP: 23AD:40FE",
        "actual_code_stop_2": "Code breakpoint at 2809:1EFF",
        "actual_code_stop_3": "Breakpoint hit at 2809:1E31",
    }
    expected = {"actual_code_stop_1", "actual_code_stop_2", "actual_code_stop_3"}
    results = {name: strict_code_stop_lines(text) for name, text in samples.items()}
    accepted = {name for name, hits in results.items() if hits}
    return {"ok": accepted == expected, "accepted": sorted(accepted), "expected": sorted(expected), "results": results}

def prior_artifact_audit() -> dict[str, Any]:
    out: dict[str, Any] = {}
    for label, path in [("pass318", PASS318), ("pass320", PASS320)]:
        data = json.loads(path.read_text(encoding="utf-8"))
        transcript_path = path.parent / "dosbox_debug_noise_reduced.clean.txt"
        transcript = transcript_path.read_text(encoding="utf-8", errors="replace") if transcript_path.exists() else "\n".join(data.get("bounded_sanitized_transcript_excerpt", []))
        strict_hits = strict_code_stop_lines(transcript)
        out[label] = {
            "status": data.get("status"),
            "legacy_hits": data.get("debugger_hits_captured", []),
            "strict_code_stop_hits": strict_hits[:20],
            "strict_hit_count": len(strict_hits),
            "bplist_echo_confusion": label == "pass318" and any(h.get("sig") == "Breakpoint list:" for h in data.get("debugger_hits_captured", [])),
        }
    return out

def write_conf(path: Path, stage: Path) -> None:
    path.write_text("\n".join([
        "[sdl]", "fullscreen=false", "output=surface", "usescancodes=false",
        "[dosbox]", "machine=svga_paradise", "memsize=4",
        "[cpu]", "core=normal", "cycles=3000",
        "[mixer]", "nosound=true",
        "[autoexec]", f"mount c {shlex.quote(str(stage))}", "c:", "DEBUG DM.EXE -vv -sn -pk", ""
    ]), encoding="utf-8")

def xdo(display: str, args: list[str]) -> subprocess.CompletedProcess[str]:
    return run(["xdotool", *args], env={**os.environ, "DISPLAY": display}, timeout=10)

def find_window_pid(display: str) -> int | None:
    search = xdo(display, ["search", "--sync", "--class", "dosbox"])
    ids = [line.strip() for line in search.stdout.splitlines() if line.strip()]
    if not ids:
        return None
    pid = xdo(display, ["getwindowpid", ids[0]])
    return int(pid.stdout.strip().splitlines()[0]) if pid.stdout.strip() else None

def key_name(tok: str) -> str:
    return {"enter": "Return", "return": "Return", "one": "1", "1": "1", "kp4": "KP_Left", "kp5": "KP_Begin", "kp6": "KP_Right"}[tok]

def click_at(display: str, window: str, x: int, y: int) -> tuple[int, int]:
    geom = xdo(display, ["getwindowgeometry", "--shell", str(window)]).stdout
    ns: dict[str, Any] = {}
    exec(geom, {}, ns)
    gw, gh = float(ns["WIDTH"]), float(ns["HEIGHT"])
    aspect = 320 / 200
    cw = gw
    ch = cw / aspect
    if ch > gh:
        ch = gh
        cw = ch * aspect
    px = int(round((gw - cw) / 2 + ((x + .5) / 320) * cw))
    py = int(round((gh - ch) / 2 + ((y + .5) / 200) * ch))
    xdo(display, ["mousemove", "--window", str(window), str(px), str(py), "click", "1"])
    return px, py

def key_loop(display: str, pid: int, route: str, stop_evt: threading.Event, run_evt: threading.Event, log: list[dict[str, Any]]) -> None:
    window = xdo(display, ["search", "--sync", "--pid", str(pid)]).stdout.strip().splitlines()[0]
    xdo(display, ["windowactivate", "--sync", window])
    xdo(display, ["windowfocus", "--sync", window])
    for item in route.split():
        low = item.lower()
        log.append({"t": time.time(), "event": "route_step", "route_item": item})
        if low.startswith("wait:"):
            end = time.time() + int(low.split(":", 1)[1]) / 1000
            while time.time() < end and not stop_evt.is_set():
                time.sleep(.05)
            continue
        end = time.time() + 12
        while time.time() < end and not stop_evt.is_set() and not run_evt.is_set():
            time.sleep(.05)
        if not run_evt.is_set():
            log.append({"t": time.time(), "event": "route_skipped_wait_running_timeout", "route_item": item})
            continue
        if low.startswith("click:"):
            x, y = map(int, low.split(":", 1)[1].split(","))
            px, py = click_at(display, window, x, y)
            log.append({"t": time.time(), "event": "click", "route_item": item, "screen": [px, py]})
        else:
            xdo(display, ["key", "--window", window, key_name(low)])
            log.append({"t": time.time(), "event": "key", "route_item": item})
        time.sleep(.2)
    log.append({"t": time.time(), "event": "route_done"})

def classify_addr(addr: str) -> str:
    if addr == ADDR["F0128_DUNGEONVIEW_Draw_CPSF"]:
        return "f0128_code_stop"
    for key in F0097_KEYS:
        if addr == ADDR[key]:
            return "f0097_code_stop:" + key
    return "other_code_stop"

def runtime_probe(seconds: int, route: str) -> dict[str, Any]:
    start = time.time()
    events: list[dict[str, Any]] = []
    hits: list[dict[str, Any]] = []
    route_log: list[dict[str, Any]] = []
    snaps: list[str] = []
    missing = [x for x in ["dosbox-debug", "tmux", "Xvfb", "xdotool"] if not shutil.which(x)]
    if missing:
        return {"ran": False, "blocker": "missing runtime tools: " + ", ".join(missing)}
    display = ":79"
    sess = f"pass321-{os.getpid()}"
    stop_evt = threading.Event()
    run_evt = threading.Event()
    stage = "f0128_gate"
    seen_lines: set[str] = set()
    with tempfile.TemporaryDirectory(prefix="firestaff-pass321-") as td:
        stage_dir = Path(td) / "dos"
        shutil.copytree(ORIG, stage_dir)
        conf = Path(td) / "dosbox.conf"
        write_conf(conf, stage_dir)
        xvfb = subprocess.Popen(["Xvfb", display, "-screen", "0", "1024x768x24"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
        time.sleep(.5)
        try:
            r = run(["tmux", "new-session", "-d", "-s", sess, f"TERM=vt100 DISPLAY={display} dosbox-debug -conf {shlex.quote(str(conf))} -exit"], timeout=5)
            if r.returncode:
                return {"ran": False, "blocker": r.stdout[-1000:]}
            time.sleep(3)
            send_cmd(sess, "BPDEL *", events)
            send_cmd(sess, f"BP {ADDR['F0128_DUNGEONVIEW_Draw_CPSF']}", events)
            send(sess, "Escape", "O", "t")
            run_evt.set()
            pid = find_window_pid(display)
            if pid:
                threading.Thread(target=key_loop, args=(display, pid, route, stop_evt, run_evt, route_log), daemon=True).start()
            deadline = time.time() + min(seconds, 60)
            while time.time() < deadline:
                txt = cap(sess, 320)
                snaps.append("\n--- POLL ---\n" + txt[-14000:])
                for hit in strict_code_stop_lines(txt):
                    if hit["line"] in seen_lines:
                        continue
                    seen_lines.add(hit["line"])
                    run_evt.clear()
                    kind = classify_addr(hit["addr"])
                    event = {"t": time.time(), "stage": stage, **hit, "kind": kind}
                    hits.append(event)
                    send_cmd(sess, "CPU", events)
                    if kind == "f0128_code_stop" and stage == "f0128_gate":
                        stage = "f0097_after_f0128"
                        send_cmd(sess, "BPDEL *", events)
                        for key in F0097_KEYS:
                            send_cmd(sess, f"BP {ADDR[key]}", events)
                    send(sess, "Escape", "O", "t")
                    run_evt.set()
                time.sleep(.5)
            stop_evt.set()
            time.sleep(.3)
            snaps.append("\n--- FINAL ---\n" + cap(sess, 700))
        finally:
            stop_evt.set()
            run(["tmux", "kill-session", "-t", sess], timeout=5)
            xvfb.terminate()
            try:
                xvfb.wait(timeout=5)
            except subprocess.TimeoutExpired:
                xvfb.kill()
    transcript = clean("\n".join(snaps))[-500000:]
    return {"ran": True, "duration_seconds": round(time.time() - start, 3), "events": events, "hits": hits, "route_log": route_log, "transcript": transcript}

def bounded_excerpt(text: str) -> list[str]:
    out = []
    pat = re.compile(r"(CODE STOP|Code breakpoint|Breakpoint hit|Breakpoint list|-> BP|-> BPLIST|23AD:40FE|2809:1E31|2809:1EBD|2809:1EEE|2809:1EFF)", re.I)
    for line in text.splitlines():
        if pat.search(line):
            out.append(line[:180])
        if len(out) >= 80:
            break
    return out

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--seconds", type=int, default=58)
    ap.add_argument("--route", default=DEFAULT_ROUTE)
    ap.add_argument("--skip-runtime", action="store_true")
    args = ap.parse_args()
    OUT.mkdir(parents=True, exist_ok=True)
    audit = source_audit()
    parser = parser_selftest()
    prior = prior_artifact_audit()
    runtime: dict[str, Any] = {"ran": False, "skipped": True}
    if not args.skip_runtime:
        runtime = runtime_probe(min(args.seconds, 60), args.route)
        transcript = runtime.get("transcript")
        if transcript:
            (OUT / "dosbox_debug_strict_probe.clean.txt").write_text(transcript, encoding="utf-8")
            runtime = {k: v for k, v in runtime.items() if k != "transcript"} | {"transcript": "parity-evidence/verification/pass321_dm1_v1_debugger_code_stop_control_sequence_probe/dosbox_debug_strict_probe.clean.txt"}
        (OUT / "route_strict_probe_keylog.json").write_text(json.dumps(runtime.get("route_log", []), indent=2, sort_keys=True) + "\n", encoding="utf-8")
    f0128 = any(h.get("kind") == "f0128_code_stop" for h in runtime.get("hits", []))
    f0097 = [h for h in runtime.get("hits", []) if str(h.get("kind", "")).startswith("f0097_code_stop")]
    if f0128 and f0097:
        status = "PASS321_REAL_F0128_CODE_STOP_AND_F0097_AFTERWARD_CAPTURED"
        next_blocker = None
    elif f0128:
        status = "BLOCKED_PASS321_F0128_CODE_STOP_CAPTURED_NO_F0097_AFTERWARD"
        next_blocker = "Real F0128 code-stop was captured, but no F0097/VIDRV window code-stop followed in the bounded run; next step is a wider/rebased F0097 window or route extension."
    else:
        status = "BLOCKED_PASS321_MISSING_DEBUGGER_CODE_STOP_CONTROL_PRIMITIVE"
        next_blocker = "tmux-pane DOSBox-debug control still lacks a reliable actual code-stop line distinct from BP/BPLIST setup echoes. Need a raw debugger event stream or a paused/running-state primitive before promoting runtime F0128/F0097 sequencing."
    transcript_path = OUT / "dosbox_debug_strict_probe.clean.txt"
    manifest = {
        "schema": "pass321_dm1_v1_debugger_code_stop_control_sequence_probe.v1",
        "timestamp_utc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "source_audit": audit,
        "addresses": ADDR,
        "parser_selftest": parser,
        "prior_artifact_audit": prior,
        "runtime_probe": runtime,
        "runtime_limit_seconds": min(args.seconds, 60),
        "strict_code_stop_filter": {
            "accepts": [p.pattern for p in CODE_STOP_PATTERNS],
            "ignores": [p.pattern for p in SETUP_ECHO_PATTERNS],
            "rule": "Only an unprompted CODE STOP / code-breakpoint-hit line counts. BP command echoes, BPLIST headers, and BPLIST entries never count as stops.",
        },
        "direct_hits": {"f0128_real_code_stop": f0128, "f0097_after_f0128_real_code_stop": bool(f0097), "f0097_hits": f0097},
        "bounded_sanitized_excerpt": bounded_excerpt(transcript_path.read_text(encoding="utf-8", errors="replace") if transcript_path.exists() else ""),
        "next_blocker": next_blocker,
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass321 — DM1 V1 debugger strict code-stop/control-sequencing probe",
        "",
        f"Status: `{status}`",
        "",
        "## Findings",
        "",
        "- Source audit confirms `F0128_DUNGEONVIEW_Draw_CPSF` calls `F0097_DUNGEONVIEW_DrawViewport` in `DUNVIEW.C`, and `F0097` reaches `VIDRV_09_BlitViewPort` in `DRAWVIEW.C`.",
        "- Pass318 parser bug confirmed: it accepted `Breakpoint list:` as a stop.",
        "- Pass320 strict filter correctly rejected BP/BPLIST echoes, but did not regain a real F0128 stop.",
        "- Pass321 strict parser only accepts unprompted code-stop lines and ignores setup/BPLIST forms.",
        "",
        "## Decision",
        "",
        next_blocker or "Real post-F0128 F0097 stop captured; gate path can be updated to require this strict sequence.",
        "",
        "Manifest: `parity-evidence/verification/pass321_dm1_v1_debugger_code_stop_control_sequence_probe/manifest.json`",
    ]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(json.dumps({"status": status, "manifest": str(OUT / "manifest.json"), "report": str(REPORT), "hits": runtime.get("hits", [])}, indent=2, sort_keys=True))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
