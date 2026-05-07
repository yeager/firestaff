#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, os, re, shutil, subprocess, tempfile, threading, time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any
import pexpect

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass335_dm1_v1_keyboard_table_route_readiness"
OUT = ROOT / "parity-evidence/verification" / PASS
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
ORIG = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
SRC = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
# I34E MAP segment + runtime load delta 0x0733, same binding family as pass330/pass333.
ADDR = {
    "G0444_ps_SecondaryKeyboardInput": "2C20:3EC0",
    "G0459_as_Graphic561_SecondaryKeyboardInput_Movement": "2C20:26F4",
    "G0458_as_Graphic561_PrimaryKeyboardInput_Interface": "2C20:26D4",
    "G2153_i_QueuedCommandsCount": "2C20:3E78",
    "F0361_COMMAND_ProcessKeyPress": "22F4:0407",
    "F0380_COMMAND_ProcessQueue_CPSC": "22F4:0699",
    "F0365_COMMAND_ProcessTypes1To2_TurnParty": "1EA4:010D",
    "F0366_COMMAND_ProcessTypes3To6_MoveParty": "1EA4:01AA",
    "F0267_MOVE_GetMoveResult_CPSCE": "1859:0516",
    "F0128_DUNGEONVIEW_Draw_CPSF": "23AD:40FE",
    "F0097_DUNGEONVIEW_DrawViewport": "2809:1EFF",
}
LOAD_PREFIX = "wait:9000 enter wait:2200 one wait:2200 click:276,140 wait:2200"
LOAD_SUFFIX = "one wait:3500"
MOVE_ROUTE = "numlock wait:300 kp5 wait:900 kp4 wait:900 kp6 wait:900 kp5 wait:900"
ANSI_RE = re.compile(r"\x1b\[[0-9;?]*[ -/]*[@-~]")
CODE_LINE_RE = re.compile(r"\b(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\s+[0-9A-F]{2,}\s*[a-z][a-z0-9]+", re.I)
DUMP_LINE_RE = re.compile(r"(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\s+(?P<bytes>(?:[0-9A-F]{2}\s+){1,16})", re.I)

def clean(text: str) -> str:
    text = ANSI_RE.sub("", text).replace("\r", "\n")
    return re.sub(r"[\x00-\x08\x0b\x0c\x0e-\x1f]", "", text)

def run(cmd: list[str], **kw: Any):
    return subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, **kw)

def drain(child: pexpect.spawn, seconds: float) -> str:
    end = time.time() + seconds; out = ""
    while time.time() < end:
        try: out += child.read_nonblocking(8192, timeout=0.05)
        except pexpect.TIMEOUT: pass
        except pexpect.EOF: out += "<EOF>"; break
    return out

def write_conf(path: Path, stage: Path) -> None:
    path.write_text("\n".join([
        "[sdl]", "fullscreen=false", "output=surface", "usescancodes=false",
        "[dosbox]", "machine=svga_paradise", "memsize=4",
        "[cpu]", "core=normal", "cycles=3000",
        "[mixer]", "nosound=true",
        "[autoexec]", f"mount c {stage}", "c:", "DEBUG DM.EXE -vv -sn -pk", "",
    ]), encoding="utf-8")

def source_audit() -> list[dict[str, Any]]:
    specs = [
        ("COMMAND.C", "movement table definition and I34E key-code mapping", [
            "KEYBOARD_INPUT* G0444_ps_SecondaryKeyboardInput;",
            "KEYBOARD_INPUT G0459_as_Graphic561_SecondaryKeyboardInput_Movement[7]",
            "MEDIA707_I34E_I34M", "C001_COMMAND_TURN_LEFT,     0x004B",
            "C003_COMMAND_MOVE_FORWARD,  0x004C", "C002_COMMAND_TURN_RIGHT,    0x004D",
            "C006_COMMAND_MOVE_LEFT,     0x004F", "C005_COMMAND_MOVE_BACKWARD, 0x0050",
            "C004_COMMAND_MOVE_RIGHT,    0x0051",
        ]),
        ("STARTUP2.C", "new-game dungeon input install", [
            "G0443_ps_PrimaryKeyboardInput = G0458_as_Graphic561_PrimaryKeyboardInput_Interface;",
            "G0444_ps_SecondaryKeyboardInput = G0459_as_Graphic561_SecondaryKeyboardInput_Movement;",
            "F0003_MAIN_ProcessNewPartyMap_CPSE(G0309_i_PartyMapIndex);",
        ]),
        ("PANEL.C", "panel close returns to dungeon movement input", [
            "G0442_ps_SecondaryMouseInput = G0448_as_Graphic561_SecondaryMouseInput_Movement;",
            "G0444_ps_SecondaryKeyboardInput = G0459_as_Graphic561_SecondaryKeyboardInput_Movement;",
            "F0357_COMMAND_DiscardAllInput();",
        ]),
        ("COMMAND.C", "keyboard processing path into F0361/F0380", [
            "void F0361_COMMAND_ProcessKeyPress", "G0443_ps_PrimaryKeyboardInput",
            "G0444_ps_SecondaryKeyboardInput", "G2153_i_QueuedCommandsCount++",
            "void F0380_COMMAND_ProcessQueue_CPSC", "G2153_i_QueuedCommandsCount == 0",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty",
        ]),
        ("GAMELOOP.C", "runtime key poll and queue process loop", [
            "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());",
            "F0380_COMMAND_ProcessQueue_CPSC();",
        ]),
    ]
    rows=[]
    for fn, ident, needles in specs:
        lines=(SRC/fn).read_text(encoding="latin-1", errors="replace").splitlines(); anchors={}
        for needle in needles:
            compact=" ".join(needle.split())
            for idx,line in enumerate(lines,1):
                if compact in " ".join(line.split()): anchors[needle]=idx; break
        rows.append({"id": ident, "file": fn, "ok": len(anchors)==len(needles), "anchors": anchors, "missing": [n for n in needles if n not in anchors]})
    return rows

def xdo(display: str, args: list[str]):
    return run(["xdotool", *args], env={**os.environ, "DISPLAY": display}, timeout=10)

def find_win(display: str) -> str | None:
    res=xdo(display,["search","--sync","--class","dosbox"])
    ids=[x.strip() for x in res.stdout.splitlines() if x.strip().isdigit()]
    return ids[0] if ids else None

def key_name(tok: str) -> str:
    return {"enter":"Return","return":"Return","one":"1","1":"1","numlock":"Num_Lock","kp4":"KP_4","kp5":"KP_5","kp6":"KP_6"}[tok]

def click_at(display: str, win: str, x: int, y: int) -> dict[str, Any]:
    ns: dict[str, Any] = {}; geom=xdo(display,["getwindowgeometry","--shell",win])
    try:
        exec(geom.stdout, {}, ns); gw, gh = float(ns["WIDTH"]), float(ns["HEIGHT"])
    except Exception as exc: return {"rc":geom.returncode or 1,"out":geom.stdout[-240:],"error":"geometry failed: "+type(exc).__name__}
    aspect=320/200; cw=gw; ch=cw/aspect
    if ch>gh: ch=gh; cw=ch*aspect
    px=int(round((gw-cw)/2+((x+.5)/320)*cw)); py=int(round((gh-ch)/2+((y+.5)/200)*ch))
    r=xdo(display,["mousemove","--window",win,str(px),str(py),"click","1"])
    return {"screen":[px,py],"rc":r.returncode,"out":r.stdout[-160:]}

def drive(display: str, win: str, route: str, log: list[dict[str, Any]], stop_event: threading.Event | None=None) -> None:
    xdo(display,["windowactivate","--sync",win]); xdo(display,["windowfocus","--sync",win])
    for item in route.split():
        if stop_event and stop_event.is_set(): break
        low=item.lower(); row={"t":time.time(),"route_item":item}
        if low.startswith("wait:"):
            rem=int(low.split(":",1)[1])/1000
            while rem>0:
                sl=min(.1,rem); time.sleep(sl); rem-=sl
                if stop_event and stop_event.is_set(): break
            row["slept"]=True
        elif low.startswith("click:"):
            x,y=map(int,low.split(":",1)[1].split(",")); row.update(click_at(display,win,x,y))
        else:
            r=xdo(display,["key","--window",win,key_name(low)]); row.update({"rc":r.returncode,"out":r.stdout[-160:]}); time.sleep(.2)
        log.append(row)

def dbg(child: pexpect.spawn, cmd: str, log: list[dict[str, Any]], transcript: list[str], wait: float=.55) -> str:
    child.sendline(cmd); time.sleep(.22); out=drain(child,wait); transcript.append(out)
    log.append({"t":time.time(),"cmd":cmd,"excerpt":clean(out)[-500:]}); return out

def parse_near_ptr(dump: str, expected_addr: str) -> dict[str, Any]:
    text=clean(dump); exp=expected_addr.upper()
    for m in DUMP_LINE_RE.finditer(text):
        if m.group("addr").upper() == exp:
            bs=[int(x,16) for x in m.group("bytes").split()[:2]]
            if len(bs) >= 2:
                off=bs[0] | (bs[1]<<8)
                return {"ok": True, "rawBytes": [f"{b:02X}" for b in bs], "nearOffsetHex": f"{off:04X}"}
    return {"ok": False, "rawTextTail": text[-300:]}

def last_code_addr(text: str) -> str | None:
    m=[x.group("addr").upper() for x in CODE_LINE_RE.finditer(clean(text))]
    return m[-1] if m else None

def pause_prompt(child: pexpect.spawn, display: str, win: str, cmdlog: list[dict[str, Any]], transcript: list[str], purpose: str) -> bool:
    for key in ["Alt+Pause", "Pause"]:
        xdo(display,["key","--window",win,key]); cmdlog.append({"t":time.time(),"control":key,"purpose":purpose})
        out=drain(child,5); transcript.append(out)
        if "->" in clean(out): return True
    return False

def monitor(child: pexpect.spawn, seconds: int, transcript: list[str], cmdlog: list[dict[str, Any]], stops: list[dict[str, Any]], stop_event: threading.Event) -> bool:
    deadline=time.time()+seconds; buf=""; saw_running=False
    while time.time()<deadline:
        chunk=drain(child,.25)
        if chunk:
            transcript.append(chunk); buf+=chunk; c=clean(buf)
            if "(Running)" in c: saw_running=True
            if "(Running)" in c and "->" in c.split("(Running)",1)[-1]:
                post=c.split("(Running)",1)[-1]; addr=last_code_addr(post)
                stops.append({"t":time.time(),"addr":addr,"label": next((k for k,v in ADDR.items() if v==addr), None)})
                if addr == ADDR["F0128_DUNGEONVIEW_Draw_CPSF"]:
                    dbg(child, "BP " + ADDR["F0097_DUNGEONVIEW_DrawViewport"], cmdlog, transcript)
                if addr == ADDR["F0097_DUNGEONVIEW_DrawViewport"]:
                    stop_event.set(); break
                child.send("\x1bOt"); cmdlog.append({"t":time.time(),"control":"F5","purpose":"continue stop","addr":addr}); buf=""
        time.sleep(.05)
    return saw_running

def run_probe(seconds: int) -> dict[str, Any]:
    transcript=[]; cmdlog=[]; routelog=[]; stops=[]; start=time.time(); OUT.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="firestaff-pass335-") as td:
        stage=Path(td)/"dos"; shutil.copytree(ORIG, stage); conf=Path(td)/"dosbox.conf"; write_conf(conf, stage)
        display=f":{80+(os.getpid()%15)}"; xvfb=subprocess.Popen(["Xvfb",display,"-ac","-screen","0","1024x768x24"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
        time.sleep(.5)
        child=pexpect.spawn("dosbox-debug", ["-conf",str(conf),"-exit"], env={**os.environ,"DISPLAY":display,"TERM":"vt100"}, encoding="utf-8", timeout=2, echo=False)
        child.delaybeforesend=.05
        try:
            time.sleep(3); transcript.append(drain(child,1)); win=find_win(display)
            if not win: return {"ran": True, "stage":"load-ready marker", "status":"BLOCKED_PASS335_KEYBOARD_TABLE_ADDRESS_REBIND_REQUIRED", "blocker":"dosbox window not found"}
            child.send("\x1bOt"); cmdlog.append({"t":time.time(),"control":"F5","purpose":"run unarmed to load/menu prefix"})
            drive(display, win, LOAD_PREFIX, routelog)
            if not pause_prompt(child, display, win, cmdlog, transcript, "pre-load-suffix debugger pause"):
                return {"ran": True, "durationSeconds":round(time.time()-start,3), "routeLog":routelog, "commandLog":cmdlog, "stops":stops, "status":"BLOCKED_PASS335_KEYBOARD_TABLE_ADDRESS_REBIND_REQUIRED", "blocker":"no debugger prompt after load prefix"}
            child.send("\x1bOt"); cmdlog.append({"t":time.time(),"control":"F5","purpose":"run load suffix toward dungeon"})
            drive(display, win, LOAD_SUFFIX, routelog)
            if not pause_prompt(child, display, win, cmdlog, transcript, "route-readiness pointer check before movement keys"):
                return {"ran": True, "durationSeconds":round(time.time()-start,3), "routeLog":routelog, "commandLog":cmdlog, "stops":stops, "status":"BLOCKED_PASS335_KEYBOARD_TABLE_ADDRESS_REBIND_REQUIRED", "blocker":"no debugger prompt before movement route"}
            dump=dbg(child, "D " + ADDR["G0444_ps_SecondaryKeyboardInput"] + " L 4", cmdlog, transcript)
            ptr=parse_near_ptr(dump, ADDR["G0444_ps_SecondaryKeyboardInput"])
            table_off=ADDR["G0459_as_Graphic561_SecondaryKeyboardInput_Movement"].split(":",1)[1]
            if not ptr.get("ok"):
                status="BLOCKED_PASS335_KEYBOARD_TABLE_ADDRESS_REBIND_REQUIRED"
            elif ptr.get("nearOffsetHex") != table_off:
                status="BLOCKED_PASS335_ROUTE_BEFORE_DUNGEON_KEY_TABLE"
            else:
                for cmd in (["BPDEL *"] + ["BP " + ADDR[k] for k in ["F0361_COMMAND_ProcessKeyPress", "F0380_COMMAND_ProcessQueue_CPSC", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0267_MOVE_GetMoveResult_CPSCE", "F0128_DUNGEONVIEW_Draw_CPSF"]] + ["BPLIST"]):
                    dbg(child, cmd, cmdlog, transcript)
                child.send("\x1bOt"); cmdlog.append({"t":time.time(),"control":"F5","purpose":"run after route-readiness arms"})
                stop_event=threading.Event(); t=threading.Thread(target=drive, args=(display,win,MOVE_ROUTE,routelog,stop_event), daemon=True); t.start()
                saw_running=monitor(child, seconds, transcript, cmdlog, stops, stop_event); t.join(timeout=1)
                labels=[s.get("label") for s in stops]
                if "F0097_DUNGEONVIEW_DrawViewport" in labels or "F0128_DUNGEONVIEW_Draw_CPSF" in labels:
                    status="PASS_ROUTE_REACHES_COMMAND_QUEUE_AFTER_TABLE_READY"
                elif set(labels) & {"F0380_COMMAND_ProcessQueue_CPSC","F0365_COMMAND_ProcessTypes1To2_TurnParty","F0366_COMMAND_ProcessTypes3To6_MoveParty","F0267_MOVE_GetMoveResult_CPSCE"}:
                    status="PASS_ROUTE_REACHES_COMMAND_QUEUE_AFTER_TABLE_READY"
                elif ptr.get("nearOffsetHex") == table_off:
                    status="BLOCKED_PASS335_TABLE_READY_BUT_KEYPAD_CODES_WRONG"
                else:
                    status="BLOCKED_PASS335_KEYBOARD_TABLE_ADDRESS_REBIND_REQUIRED"
                return {"ran": True, "durationSeconds":round(time.time()-start,3), "boundedSeconds":seconds, "routeLog":routelog, "commandLog":cmdlog, "stops":stops, "sawRunning":saw_running, "pointerCheck":ptr, "expectedMovementTableOffset":table_off, "status":status, "stage":None if status.startswith("PASS") else "route-readiness after dungeon load", "blocker":None if status.startswith("PASS") else status}
            return {"ran": True, "durationSeconds":round(time.time()-start,3), "boundedSeconds":seconds, "routeLog":routelog, "commandLog":cmdlog, "stops":stops, "pointerCheck":ptr, "expectedMovementTableOffset":table_off, "status":status, "stage":"route-readiness after dungeon load", "blocker":status}
        finally:
            try: transcript.append(drain(child,.5)); child.terminate(force=True)
            except Exception: pass
            xvfb.terminate()
            try: xvfb.wait(timeout=5)
            except subprocess.TimeoutExpired: xvfb.kill()
            OUT.mkdir(parents=True, exist_ok=True)
            (OUT/"probe.clean.txt").write_text(clean("".join(transcript))[-300000:]+"\n", encoding="utf-8")

def build(args: argparse.Namespace) -> dict[str, Any]:
    audit=source_audit(); missing=[x for x in ["dosbox-debug","Xvfb","xdotool"] if not shutil.which(x)]
    probe=None
    if not missing and not args.no_runtime: probe=run_probe(args.seconds)
    if missing: status="BLOCKED_PASS335_KEYBOARD_TABLE_ADDRESS_REBIND_REQUIRED"
    elif args.no_runtime: status="BLOCKED_PASS335_KEYBOARD_TABLE_ADDRESS_REBIND_REQUIRED"
    else: status=probe.get("status", "BLOCKED_PASS335_KEYBOARD_TABLE_ADDRESS_REBIND_REQUIRED")
    return {"schema":PASS+".v1", "timestampUtc":datetime.now(timezone.utc).isoformat(), "status":status, "hypothesesDistinguished":["ROUTE_BEFORE_DUNGEON_KEY_TABLE","TABLE_READY_BUT_KEYPAD_CODES_WRONG","ADDRESS_BINDING_REQUIRED"], "sourceAudit":audit, "addresses":ADDR, "runtimeProbe":{"ran": bool(probe), "missingTools":missing, "boundedSecondsPerProbe":args.seconds, "probes":[probe] if probe else []}, "blocker": None if status.startswith("PASS") else (probe or {}).get("blocker", "runtime unavailable"), "notPromotedBy":["BPLIST","BP command echo","tmux/capture-pane"]}

def main() -> int:
    ap=argparse.ArgumentParser(); ap.add_argument("--seconds", type=int, default=35); ap.add_argument("--no-runtime", action="store_true")
    args=ap.parse_args(); args.seconds=max(10,min(75,args.seconds))
    manifest=build(args); OUT.mkdir(parents=True, exist_ok=True)
    (OUT/"manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True)+"\n", encoding="utf-8")
    for p in manifest.get("runtimeProbe",{}).get("probes",[]):
        if p: (OUT/"probe_route_keylog.json").write_text(json.dumps(p.get("routeLog",[]), indent=2, sort_keys=True)+"\n", encoding="utf-8")
    stop_summary=[]
    for probe in manifest.get("runtimeProbe", {}).get("probes", []):
        if probe: stop_summary.extend((s.get("addr"), s.get("label")) for s in probe.get("stops", []))
    ptr=(manifest.get("runtimeProbe",{}).get("probes") or [{}])[0].get("pointerCheck", {}) if manifest.get("runtimeProbe",{}).get("probes") else {}
    report_lines=["# Pass335 — DM1 V1 keyboard-table route readiness", "", "Status: `" + manifest["status"] + "`", "", "## ReDMCSB anchors"]
    report_lines += ["- `" + r["file"] + "` " + r["id"] + ": `" + str(r["anchors"]) + "`" for r in manifest["sourceAudit"]]
    report_lines += ["", "## Runtime decision", "", "- Pointer check before route keys: `" + str(ptr) + "`", "- Expected movement table offset: `" + ADDR["G0459_as_Graphic561_SecondaryKeyboardInput_Movement"].split(":",1)[1] + "`", "- Stops: `" + str(stop_summary) + "`", "- Blocker: `" + str(manifest.get("blocker")) + "`", "", "Completion matrix: unchanged; evidence narrows the existing DM1 V1 primary blocker but does not change the 52% score.", "", "Manifest: `parity-evidence/verification/pass335_dm1_v1_keyboard_table_route_readiness/manifest.json`"]
    REPORT.write_text("\n".join(report_lines)+"\n", encoding="utf-8")
    return 0
if __name__ == "__main__": raise SystemExit(main())
