#!/usr/bin/env python3
"""Pass464: DM1 V1 mouse callback event branch before F0359.

Source-locks the ReDMCSB PC mouse callback path and runs the original PC34
runtime with debugger breakpoints at F0781_MouseHandler (IODRV callback) and
F0359_COMMAND_ProcessClick_CPSC (command queue entry). This pass answers whether
xdotool/SDL clicks reach the game callback before F0359, without promoting Hall
movement parity.
"""
from __future__ import annotations

import argparse, json, os, re, shutil, subprocess, tempfile, threading, time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any
import pexpect
import verify_pass385_dm1_v1_corrected_loader_delta_semantic_route as p385

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass464_dm1_v1_mouse_callback_event_branch"
OUT = ROOT / "parity-evidence" / "verification" / PASS
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
ADDR = {
    # FIRES.MAP link 22DD:0020 + corrected loader delta 0736.
    "F0781_MouseHandler": "2A13:0020",
    # IO.C:705 compare site. Break before the cmp so BP has been established by
    # the F0781 prologue and [bp+0A] is the actual MouseEvent argument.
    "F0781_EventCmp": "2A13:002F",
    "F0781_ConditionalAfterCmp": "2A13:0033",
    # FIRES.MAP link 1BC1:030D + corrected loader delta 0736.
    "F0359_COMMAND_ProcessClick_CPSC": "22F7:030D",
}
CLICK_ROUTE = "click:276,135 wait:900 click:248,135 wait:900 click:304,135 wait:900"
CODE_LINE_RE = re.compile(r"\b(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\s+[0-9A-F]{2,}\s*[a-z][a-z0-9]+", re.I)
EVENT_CMP_RE = re.compile(r"(?:cmp\s+word\s+)?\[bp\+0A\][^=]*=([0-9A-F]{4})", re.I)

def run(cmd: list[str], **kw: Any) -> subprocess.CompletedProcess[str]:
    return subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, **kw)

def norm(s: str) -> str:
    return " ".join(s.split())

def find_line(path: Path, needle: str) -> int | None:
    n = norm(needle)
    for i, line in enumerate(path.read_text(encoding="latin-1", errors="replace").splitlines(), 1):
        if n in norm(line):
            return i
    return None

def source_audit() -> list[dict[str, Any]]:
    specs = [
        ("IO.C", "game_callback_to_f0359", [
            "int16_t F0781_MouseHandler",
            "if (P2383_i_MouseEvent < C32_MOUSE_EVENT_CHANGE_SCREEN_REGION)",
            "F0359_COMMAND_ProcessClick_CPSC(P2381_i_X, P2382_i_Y, P2383_i_MouseEvent);",
            "(*(G2161_IODriver->IODRV_02_SetMouseHandler))(F0781_MouseHandler);",
        ]),
        ("IBMIO.C", "pc_driver_status_to_callback", [
            "void F8096_ProcessMouseState(register int16_t P3280_i_MouseX",
            "G8040_CurrentMouseX = P3280_i_MouseX;",
            "(*G8067_MouseHandler)(P3280_i_MouseX, P3281_i_MouseY, (P3282_i_RawButtonStatus & 1) ? C02_MOUSE_EVENT_LEFT_BUTTON_DOWN : C04_MOUSE_EVENT_LEFT_BUTTON_UP);",
            "(*G8067_MouseHandler)(P3280_i_MouseX, P3281_i_MouseY, (P3282_i_RawButtonStatus & 2) ? C01_MOUSE_EVENT_RIGHT_BUTTON_DOWN : C08_MOUSE_EVENT_RIGHT_BUTTON_UP);",
            "void F8101_SetMouseHandler(int16_t (*P3283_pfi_)())",
            "G8067_MouseHandler = P3283_pfi_;",
            "(char*)F8101_SetMouseHandler, /*  2 */",
        ]),
        ("COMMAND.C", "f0359_queue_entry", [
            "void F0359_COMMAND_ProcessClick_CPSC",
            "L1109_i_Command = F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0441_ps_PrimaryMouseInput",
            "L1109_i_Command = F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0442_ps_SecondaryMouseInput",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1108_i_CommandQueueIndex].Command = L1109_i_Command",
        ]),
    ]
    rows=[]
    for fn, section, needles in specs:
        p=RED/fn
        hits={needle: find_line(p, needle) for needle in needles} if p.exists() else {}
        missing=[needle for needle,line in hits.items() if line is None]
        rows.append({"file":fn,"section":section,"path":str(p),"ok":p.exists() and not missing,"lineHits":hits,"missing":missing})
    return rows

def robust_find_win(display: str) -> str | None:
    out = p385.xdo(display, ["search", "--sync", "--class", "dosbox"]).stdout
    ids=[x.strip() for x in out.splitlines() if x.strip().isdigit()]
    return ids[0] if ids else None

def click_at_logged(display: str, win: str, x: int, y: int) -> dict[str, Any]:
    ns: dict[str, Any] = {}
    exec(p385.xdo(display, ["getwindowgeometry", "--shell", win]).stdout, {}, ns)
    gw, gh = float(ns["WIDTH"]), float(ns["HEIGHT"])
    aspect=320/200; cw,ch=gw,gw/aspect
    if ch>gh: ch,cw=gh,gh*aspect
    px=int(round((gw-cw)/2+((x+.5)/320)*cw)); py=int(round((gh-ch)/2+((y+.5)/200)*ch))
    r=p385.xdo(display, ["mousemove", "--window", win, str(px), str(py), "click", "1"])
    return {"client":[x,y],"screen":[px,py],"rc":r.returncode,"out":r.stdout[-160:]}

def drive(display: str, win: str, route: str, log: list[dict[str, Any]], running: threading.Event, stop: threading.Event) -> None:
    p385.xdo(display, ["windowactivate", "--sync", win]); p385.xdo(display, ["windowfocus", "--sync", win])
    for item in route.split():
        if stop.is_set(): break
        row={"t":time.time(),"route_item":item}; low=item.lower()
        if low.startswith("wait:"):
            end=time.time()+int(low.split(":",1)[1])/1000
            while time.time()<end and not stop.is_set(): time.sleep(.05)
            row["slept"]=True
        elif low.startswith("click:"):
            deadline=time.time()+20
            while time.time()<deadline and not running.is_set() and not stop.is_set(): time.sleep(.05)
            row["running_guard"]=running.is_set()
            x,y=map(int,low.split(":",1)[1].split(",")); row.update(click_at_logged(display,win,x,y))
        else:
            r=p385.xdo(display,["key","--window",win,p385.key_name(low)]); row.update({"rc":r.returncode,"out":r.stdout[-160:]})
        log.append(row)

def classify(post: str) -> dict[str, Any]:
    c=p385.clean(post); lines=p385.code_lines(c)[-18:]
    kind="other"; entry=None
    last_addr=p385.last_code_addr(c)
    for name,target in ADDR.items():
        if last_addr == target:
            kind=name; entry=target; break
    if kind == "other":
        for name,target in ADDR.items():
            if any(line.upper().startswith(target) for line in lines) or target in c.upper():
                kind=name; entry=target; break
    return {"kind":kind,"entryAddr":entry,"addr":last_addr,"postRunningCodeLines":lines,"postRunningExcerpt":c[-2200:]}

def parse_event_from_cmp_text(text: str) -> int | None:
    m = EVENT_CMP_RE.search(text or "")
    return int(m.group(1), 16) if m else None

def annotate_event(row: dict[str, Any]) -> None:
    event = parse_event_from_cmp_text(row.get("postRunningExcerpt", ""))
    if event is None:
        event = parse_event_from_cmp_text(row.get("stepTail", ""))
    if event is not None:
        row["eventValueBeforeIOConditional"] = event
        row["eventValueHexBeforeIOConditional"] = f"0x{event:04X}"
        row["ioConditionalAllowsF0359"] = event < 0x20

def monitor(child: pexpect.spawn, seconds:int, transcript:list[str], cmdlog:list[dict[str,Any]], stops:list[dict[str,Any]], running:threading.Event, stop:threading.Event)->bool:
    deadline=time.time()+seconds; buf=""; saw=False
    while time.time()<deadline and not stop.is_set():
        chunk=p385.drain(child,.25)
        if chunk:
            transcript.append(chunk); buf+=chunk; c=p385.clean(buf)
            if "(Running)" in c: saw=True; running.set()
            if "(Running)" in c and "->" in c.split("(Running)",1)[-1]:
                running.clear(); post=c.split("(Running)",1)[-1]
                row={"t":time.time(),"runningMarkerSeen":True,"promptReappearedAfterRunning":True,**classify(post)}; stops.append(row)
                if row.get("kind") == "F0781_MouseHandler":
                    # Entry is before the prologue, so do not sample [bp+0A] here.
                    # Drop this entry breakpoint and continue to the cmp breakpoint.
                    for cmd in ["CPU", "BPDEL " + ADDR["F0781_MouseHandler"], "BPLIST"]:
                        p385.dbg(child, cmd, cmdlog, transcript)
                elif row.get("kind") in ("F0781_EventCmp", "F0781_ConditionalAfterCmp"):
                    # At the cmp site the F0781 prologue has established BP; this
                    # is the first reliable MouseEvent argument sample.
                    for cmd in ["CPU", "T", "CPU", "BPLIST"]:
                        p385.dbg(child, cmd, cmdlog, transcript)
                    tail = p385.clean("\n".join(transcript))[-5000:]
                    row["singleStepCommands"] = ["CPU", "T", "CPU", "BPLIST"]
                    row["stepTail"] = tail
                    annotate_event(row)
                    row["steppedToAddr"] = p385.last_code_addr(tail)
                else:
                    for cmd in ["CPU", "BPLIST"]:
                        p385.dbg(child, cmd, cmdlog, transcript)
                child.send("\x1bOt"); cmdlog.append({"t":time.time(),"control":"F5","purpose":"continue after pass464 stop","kind":row.get("kind")})
                buf=""; running.set()
        time.sleep(.05)
    return saw

def runtime_probe(seconds:int, route:str)->dict[str,Any]:
    missing=[x for x in ["dosbox-debug","Xvfb","xdotool"] if not shutil.which(x)]
    if missing: return {"ran":False,"missingTools":missing,"blocker":"missing tools: "+", ".join(missing)}
    OUT.mkdir(parents=True,exist_ok=True); transcript=[]; cmdlog=[]; routelog=[]; stops=[]; start=time.time(); arm_time=0.0
    with tempfile.TemporaryDirectory(prefix="firestaff-pass464-") as td:
        stage=Path(td)/"dos"; shutil.copytree(p385.ORIG,stage)
        conf=Path(td)/"dosbox.conf"; p385.write_conf(conf,stage)
        display=f":{245+(os.getpid()%10)}"; xvfb=subprocess.Popen(["Xvfb",display,"-screen","0","1024x768x24"],stdout=subprocess.DEVNULL,stderr=subprocess.STDOUT)
        time.sleep(.5)
        child=pexpect.spawn("dosbox-debug",["-conf",str(conf),"-exit"],env={**os.environ,"DISPLAY":display,"TERM":"vt100"},encoding="utf-8",timeout=2,echo=False); child.delaybeforesend=.05
        try:
            time.sleep(3); transcript.append(p385.drain(child,1)); win=robust_find_win(display)
            if not win: return {"ran":True,"stage":"window","blocker":"dosbox window not found"}
            child.send("\x1bOt"); cmdlog.append({"t":time.time(),"control":"F5","purpose":"run unarmed through fresh-entry load prefix"})
            p385.drive(display,win,p385.LOAD_PREFIX,routelog)
            if not p385.pause_to_prompt(child,display,win,cmdlog,transcript,"post-load mouse seam arm point"):
                return {"ran":True,"durationSeconds":round(time.time()-start,3),"stage":"post-load arm","blocker":"no debugger prompt at arm point","routeLog":routelog,"commandLog":cmdlog,"stops":stops}
            for cmd in ["BPDEL *", "BP " + ADDR["F0781_MouseHandler"], "BP " + ADDR["F0781_EventCmp"], "BP " + ADDR["F0359_COMMAND_ProcessClick_CPSC"], "BPLIST"]:
                p385.dbg(child,cmd,cmdlog,transcript)
            bplist=p385.clean(cmdlog[-1].get("excerpt","")).upper(); retained_at_arm={n:(a in bplist) for n,a in ADDR.items()}
            arm_time=time.time(); running=threading.Event(); stop=threading.Event(); child.send("\x1bOt"); cmdlog.append({"t":time.time(),"control":"F5","purpose":"run after pass464 mouse seam arm"})
            t=threading.Thread(target=drive,args=(display,win,p385.LOAD_SUFFIX+" "+route,routelog,running,stop),daemon=True); t.start()
            saw=monitor(child,seconds,transcript,cmdlog,stops,running,stop); stop.set(); t.join(timeout=1)
            p385.pause_to_prompt(child,display,win,cmdlog,transcript,"final retention sample"); p385.dbg(child,"BPLIST",cmdlog,transcript)
            final=p385.clean(cmdlog[-1].get("excerpt","")).upper(); retained_final={n:(a in final) for n,a in ADDR.items()}
            route_after=any(r.get("t",0)>arm_time and str(r.get("route_item","")).startswith("click:") for r in routelog)
            return {"ran":True,"durationSeconds":round(time.time()-start,3),"boundedSeconds":seconds,"method":"fresh-entry original runtime; arm F0781 mouse callback and F0359 click queue entry; drive client-relative movement-panel clicks; strict stops require (Running)->prompt","route":p385.LOAD_SUFFIX+" "+route,"routeInputAfterArming":route_after,"sawRunning":saw,"retainedAtArm":retained_at_arm,"retainedFinal":retained_final,"stops":stops,"routeLog":routelog,"commandLog":cmdlog}
        finally:
            try: transcript.append(p385.drain(child,.5)); child.terminate(force=True)
            except Exception: pass
            xvfb.terminate();
            try: xvfb.wait(timeout=5)
            except Exception: xvfb.kill()
            OUT.mkdir(parents=True, exist_ok=True)
            (OUT/(PASS+"_runtime.clean.txt")).write_text(p385.clean("\n".join(transcript)),encoding="utf-8",errors="replace")
            (OUT/(PASS+"_route_keylog.json")).write_text(json.dumps(routelog,indent=2),encoding="utf-8")
            (OUT/(PASS+"_command_log.json")).write_text(json.dumps(cmdlog,indent=2),encoding="utf-8")

def classify_result(source,runtime):
    stops = runtime.get("stops", []) if runtime.get("ran") else []
    kinds=[st.get("kind") for st in stops]
    f0781=[st for st in stops if st.get("kind") in ("F0781_MouseHandler", "F0781_EventCmp", "F0781_ConditionalAfterCmp") or str(st.get("addr", "")).startswith("2A13:003")]
    event_rows=[st for st in stops if st.get("kind") in ("F0781_EventCmp", "F0781_ConditionalAfterCmp") or str(st.get("addr", "")).startswith("2A13:003")]
    first=event_rows[0] if event_rows else (f0781[0] if f0781 else {})
    sampled_events=[st.get("eventValueBeforeIOConditional") for st in event_rows if isinstance(st.get("eventValueBeforeIOConditional"), int)]
    f0359_allowed_events=[event for event in sampled_events if event < 0x20]
    change_region_events=[event for event in sampled_events if event >= 0x20]
    event=first.get("eventValueBeforeIOConditional")
    preds={
        "sourceAuditOk":all(r.get("ok") for r in source),
        "runtimeRan":runtime.get("ran") is True,
        "routeInputAfterArming":runtime.get("routeInputAfterArming") is True,
        "f0781Hit":bool(f0781),
        "f0781EventCmpHit":bool(event_rows),
        "f0359Hit":"F0359_COMMAND_ProcessClick_CPSC" in kinds,
        "eventValueBeforeIOConditional":event,
        "sampledEventsBeforeIOConditional":sampled_events,
        "f0359AllowedEventCount":len(f0359_allowed_events),
        "changeRegionOrPointerEventCount":len(change_region_events),
        "ioConditionalAllowsF0359": first.get("ioConditionalAllowsF0359"),
        "steppedToAddr": first.get("steppedToAddr"),
    }
    if not preds["sourceAuditOk"]: return "FAIL_PASS464_SOURCE_AUDIT", preds, "source audit missing required ReDMCSB anchors"
    if not preds["runtimeRan"]: return "BLOCKED_PASS464_RUNTIME_NOT_RUN", preds, runtime.get("blocker","runtime did not run")
    if preds["f0359Hit"]: return "PASS_PASS464_F0359_REACHED", preds, "F0359_COMMAND_ProcessClick_CPSC stopped after F0781"
    if preds["f0781Hit"] and sampled_events and not f0359_allowed_events:
        return "BLOCKED_PASS464_ONLY_CHANGE_SCREEN_REGION_CALLBACK", preds, "F0781 was reached, but sampled MouseEvent values were all >= C32_MOUSE_EVENT_CHANGE_SCREEN_REGION, so IO.C:705 intentionally skips F0359"
    if preds["f0781Hit"]:
        return "BLOCKED_PASS464_F0781_EVENT_SAMPLE_UNSTABLE", preds, "F0781 branch-region was reached, but the debugger event sample is not stable enough to prove either the change-screen-region skip or the F0359 call path"
    return "BLOCKED_PASS464_CLICK_NOT_REACHING_F0781", preds, "client-relative clicks were posted after arming, but F0781 did not hit"

def main():
    ap=argparse.ArgumentParser(); ap.add_argument("--seconds",type=int,default=45); ap.add_argument("--route",default=CLICK_ROUTE); args=ap.parse_args()
    source=source_audit(); runtime=runtime_probe(args.seconds,args.route); status,preds,summary=classify_result(source,runtime)
    manifest={"schema":PASS+".v1","timestampUtc":datetime.now(timezone.utc).isoformat(),"status":status,"summary":summary,"addresses":ADDR,"sourceAudit":source,"runtime":runtime,"predicates":preds,"artifacts":{"transcript":str(OUT/(PASS+"_runtime.clean.txt")),"routeKeylog":str(OUT/(PASS+"_route_keylog.json")),"commandLog":str(OUT/(PASS+"_command_log.json"))}}
    OUT.mkdir(parents=True,exist_ok=True); (OUT/"manifest.json").write_text(json.dumps(manifest,indent=2),encoding="utf-8")
    lines=[f"# Pass464 — DM1 V1 mouse callback event branch", "", f"Status: `{status}`", "", summary, "", "## ReDMCSB source audit", ""]
    for row in source:
        lines.append(f"- `{row['file']}` `{row['section']}` ok=`{row['ok']}` missing=`{row['missing']}`")
        for needle,line in row.get("lineHits",{}).items(): lines.append(f"  - line {line}: `{needle}`")
    rel_out = OUT.relative_to(ROOT)
    lines += ["", "## Runtime", "", f"- Method: {runtime.get('method')}", f"- Route input after arming: `{runtime.get('routeInputAfterArming')}`", f"- Stops: `{[stop.get('kind') for stop in runtime.get('stops', [])]}`", f"- Retained at arm: `{runtime.get('retainedAtArm')}`", f"- Retained final: `{runtime.get('retainedFinal')}`", "", "## Conclusion", ""]
    if status == "BLOCKED_PASS464_ONLY_CHANGE_SCREEN_REGION_CALLBACK":
        lines += [
            "- xdotool/SDL delivery reached the original game mouse callback seam (`F0781_MouseHandler`) after arming.",
            "- The sampled callback values were change-screen-region/pointer events (`MouseEvent >= 0x20`), so the source branch at `IO.C:705` correctly skipped `F0359_COMMAND_ProcessClick_CPSC`.",
            "- Next action: fix the live N2 click primitive so a bounded run samples `C02_MOUSE_EVENT_LEFT_BUTTON_DOWN`/`C04_MOUSE_EVENT_LEFT_BUTTON_UP` before claiming Hall candidate transition parity.",
        ]
    elif status == "BLOCKED_PASS464_F0781_REACHED_F0359_NOT_PROVEN":
        lines += ["- xdotool/SDL click delivery reached the original game mouse callback seam (`F0781_MouseHandler`) after arming.", "- The bounded run did not prove transition into `F0359_COMMAND_ProcessClick_CPSC`; next action is to single-step from `2A13:0020`/`2A13:0033` and inspect the callback parameters/event value before the conditional at `IO.C:705`."]
    else:
        lines.append(f"- {summary}")
    lines += ["", "## Artifacts", "", f"- Manifest: `{rel_out/'manifest.json'}`", f"- Transcript: `{rel_out/(PASS+'_runtime.clean.txt')}`", f"- Route keylog: `{rel_out/(PASS+'_route_keylog.json')}`", f"- Command log: `{rel_out/(PASS+'_command_log.json')}`", ""]
    REPORT.write_text("\n".join(lines),encoding="utf-8")
    print(json.dumps({"status":status,"summary":summary,"predicates":preds,"report":str(REPORT)},indent=2))
    raise SystemExit(0 if status.startswith("PASS") or status.startswith("BLOCKED") else 1)
if __name__ == "__main__": main()
