#!/usr/bin/env python3
"""Pass326: direct PTY DOSBox-debug proof harness."""
from __future__ import annotations
import argparse, json, os, re, shutil, subprocess, tempfile, threading, time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any
import pexpect
ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "parity-evidence/verification/pass326_dm1_v1_direct_pty_f0128_code_stop"
REPORT = ROOT / "parity-evidence/pass326_dm1_v1_direct_pty_f0128_code_stop.md"
ORIG = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
F0128, F0097, F0380 = "23AD:40FE", "2809:1EFF", "22F4:0699"
DEFAULT_ROUTE = "wait:4000 enter wait:1200 one wait:1200 click:276,140 wait:1200 one wait:1200 kp5 wait:600 kp4 wait:600 kp6 wait:600 kp5 wait:600"
ANSI_RE = re.compile(r"\x1b\[[0-9;?]*[ -/]*[@-~]")
CODE_LINE_RE = re.compile(r"\b(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\s+[0-9A-F]{2,}\s*[a-z][a-z0-9]+", re.I)
BPLIST_RE = re.compile(r"(?:Breakpoint list:|\b\d+\.\s+BP\s+[0-9A-F]{4}:[0-9A-F]{4}\b|DEBUG: Set breakpoint at|^BP\s+[0-9A-F]{4}:[0-9A-F]{4}\b)", re.I | re.M)
SOURCE_CHECKS = [
    {"id":"DUNVIEW.C F0128","file":"DUNVIEW.C","needles":["void F0128_DUNGEONVIEW_Draw_CPSF","F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)"]},
    {"id":"DRAWVIEW.C F0097","file":"DRAWVIEW.C","needles":["void F0097_DUNGEONVIEW_DrawViewport","VIDRV_09_BlitViewPort"]},
    {"id":"COMMAND.C F0380","file":"COMMAND.C","needles":["void F0380_COMMAND_ProcessQueue_CPSC","F0365_COMMAND_ProcessTypes1To2_TurnParty","F0366_COMMAND_ProcessTypes3To6_MoveParty"]},
]
def clean(text: str) -> str:
    return re.sub(r"[\x00-\x08\x0b\x0c\x0e-\x1f]", "", ANSI_RE.sub("", text).replace("\r", "\n"))
def run(cmd: list[str], **kw: Any) -> subprocess.CompletedProcess[str]:
    return subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, **kw)
def drain(child: pexpect.spawn, seconds: float, stop: threading.Event | None = None) -> str:
    end=time.time()+seconds; out=""
    while time.time()<end and not (stop and stop.is_set()):
        try: out += child.read_nonblocking(size=8192, timeout=0.05)
        except pexpect.TIMEOUT: pass
        except pexpect.EOF: out += "<EOF>"; break
    return out
def write_conf(path: Path, stage: Path) -> None:
    path.write_text("\n".join(["[sdl]","fullscreen=false","output=surface","usescancodes=false","[dosbox]","machine=svga_paradise","memsize=4","[cpu]","core=normal","cycles=3000","[mixer]","nosound=true","[autoexec]",f"mount c {stage}","c:","DEBUG DM.EXE -vv -sn -pk",""]), encoding="utf-8")
def source_audit() -> list[dict[str, Any]]:
    out=[]
    for spec in SOURCE_CHECKS:
        path=SOURCE_ROOT/spec["file"]; lines=path.read_text(encoding="latin-1",errors="replace").splitlines() if path.exists() else []
        anchors={}
        for needle in spec["needles"]:
            compact=" ".join(needle.split())
            for idx,line in enumerate(lines, start=1):
                if compact in " ".join(line.split()): anchors[needle]=idx; break
        out.append({"id":spec["id"],"file":spec["file"],"source_path":str(path),"ok":path.exists() and len(anchors)==len(spec["needles"]),"anchors":anchors,"missing":[n for n in spec["needles"] if n not in anchors]})
    return out
def code_lines(text: str) -> list[str]: return [m.group(0)[:160] for m in CODE_LINE_RE.finditer(clean(text))]
def last_code_addr(text: str) -> str | None:
    matches=[m.group("addr").upper() for m in CODE_LINE_RE.finditer(clean(text))]
    return matches[-1] if matches else None
def xdo(display: str, args: list[str]) -> subprocess.CompletedProcess[str]: return run(["xdotool",*args], env={**os.environ,"DISPLAY":display}, timeout=10)
def find_window(display: str) -> str | None:
    ids=[x.strip() for x in xdo(display,["search","--sync","--class","dosbox"]).stdout.splitlines() if x.strip()]
    return ids[0] if ids else None
def key_name(tok: str) -> str: return {"enter":"Return","return":"Return","one":"1","1":"1","kp4":"KP_Left","kp5":"KP_Begin","kp6":"KP_Right"}[tok]
def click_at(display: str, window: str, x: int, y: int) -> tuple[int,int]:
    ns:dict[str,Any]={}; exec(xdo(display,["getwindowgeometry","--shell",window]).stdout,{},ns)
    gw,gh=float(ns["WIDTH"]),float(ns["HEIGHT"]); aspect=320/200; cw=gw; ch=cw/aspect
    if ch>gh: ch=gh; cw=ch*aspect
    px=int(round((gw-cw)/2+((x+.5)/320)*cw)); py=int(round((gh-ch)/2+((y+.5)/200)*ch))
    xdo(display,["mousemove","--window",window,str(px),str(py),"click","1"]); return px,py
def route_worker(display: str, route: str, running: threading.Event, stop: threading.Event, log: list[dict[str,Any]]) -> None:
    window=find_window(display)
    if not window: log.append({"event":"no_dosbox_window","t":time.time()}); return
    xdo(display,["windowactivate","--sync",window]); xdo(display,["windowfocus","--sync",window])
    for item in route.split():
        low=item.lower(); log.append({"event":"route_step","route_item":item,"t":time.time()})
        if low.startswith("wait:"):
            end=time.time()+int(low.split(":",1)[1])/1000
            while time.time()<end and not stop.is_set(): time.sleep(.05)
            continue
        end=time.time()+20
        while time.time()<end and not stop.is_set() and not running.is_set(): time.sleep(.05)
        if stop.is_set(): break
        if not running.is_set(): log.append({"event":"route_skipped_not_running","route_item":item,"t":time.time()}); continue
        if low.startswith("click:"):
            x,y=map(int,low.split(":",1)[1].split(",")); px,py=click_at(display,window,x,y); log.append({"event":"click","route_item":item,"screen":[px,py],"t":time.time()})
        else:
            xdo(display,["key","--window",window,key_name(low)]); log.append({"event":"key","route_item":item,"t":time.time()})
        time.sleep(.2)
    log.append({"event":"route_done","t":time.time()})
def classify_run_transition(text: str) -> dict[str,Any] | None:
    c=clean(text)
    if "(Running)" not in c or "->" not in c.split("(Running)",1)[-1]: return None
    post=c.split("(Running)",1)[-1]; addr=last_code_addr(post)
    return {"running_marker_seen":True,"prompt_reappeared_after_running":True,"stop_code_addr_after_running":addr,"target_f0128":addr==F0128,"bplist_text_after_running":bool(BPLIST_RE.search(post)),"post_running_code_lines":code_lines(post)[-12:],"post_running_excerpt":clean(post)[-3000:]}
def debugger_command(child: pexpect.spawn, cmd: str, log: list[dict[str,Any]]) -> str:
    child.sendline(cmd); time.sleep(.2); out=drain(child,.5); log.append({"t":time.time(),"cmd":cmd,"excerpt":clean(out)[-600:]}); return out
def runtime_probe(seconds: int, route: str) -> dict[str,Any]:
    missing=[x for x in ["dosbox-debug","Xvfb","xdotool"] if not shutil.which(x)]
    if missing: return {"ran":False,"blocker":"missing tools: "+", ".join(missing)}
    OUT.mkdir(parents=True,exist_ok=True); transcript=""; command_log=[]; route_log=[]; stops=[]; running=threading.Event(); stop=threading.Event(); start=time.time()
    with tempfile.TemporaryDirectory(prefix="firestaff-pass326-") as td:
        stage=Path(td)/"dos"; shutil.copytree(ORIG,stage); conf=Path(td)/"dosbox.conf"; write_conf(conf,stage); display=f":{90+(os.getpid()%7)}"
        xvfb=subprocess.Popen(["Xvfb",display,"-screen","0","1024x768x24"],stdout=subprocess.DEVNULL,stderr=subprocess.STDOUT); time.sleep(.5)
        child=pexpect.spawn("dosbox-debug",["-conf",str(conf),"-exit"],env={**os.environ,"DISPLAY":display,"TERM":"vt100"},encoding="utf-8",timeout=2,echo=False); child.delaybeforesend=.05
        try:
            time.sleep(3); transcript += drain(child,1.0)
            for cmd in ["BPDEL *",f"BP {F0128}","BPLIST"]: transcript += debugger_command(child,cmd,command_log)
            bplist_negative_control=any(e.get("cmd")=="BPLIST" and "Breakpoint list:" in e.get("excerpt","") for e in command_log)
            child.send("\x1bOt"); command_log.append({"t":time.time(),"control":"F5","bytes":"ESC O t","purpose":"run after arming F0128"})
            threading.Thread(target=route_worker,args=(display,route,running,stop,route_log),daemon=True).start()
            buffer=""; deadline=time.time()+seconds
            while time.time()<deadline and not stop.is_set():
                chunk=drain(child,.25)
                if chunk:
                    transcript+=chunk; buffer+=chunk
                    if "(Running)" in clean(buffer): running.set()
                    transition=classify_run_transition(buffer)
                    if transition:
                        running.clear(); stops.append({"t":time.time(),**transition}); transcript += debugger_command(child,"CPU",command_log); transcript += debugger_command(child,f"D {F0128}",command_log); transcript += debugger_command(child,"BPLIST",command_log)
                        if transition.get("target_f0128"): break
                        buffer=""; child.send("\x1bOt"); command_log.append({"t":time.time(),"control":"F5","bytes":"ESC O t","purpose":"continue after non-target stop"})
                time.sleep(.05)
        finally:
            stop.set()
            try: transcript += drain(child,.5); child.terminate(force=True)
            except Exception: pass
            xvfb.terminate()
            try: xvfb.wait(timeout=5)
            except subprocess.TimeoutExpired: xvfb.kill()
    OUT.mkdir(parents=True,exist_ok=True); (OUT/"dosbox_debug_pty.clean.txt").write_text(clean(transcript)[-300000:]+"\n",encoding="utf-8"); (OUT/"route_keylog.json").write_text(json.dumps(route_log,indent=2,sort_keys=True)+"\n",encoding="utf-8")
    f0128_stops=[s for s in stops if s.get("target_f0128") and s.get("stop_code_addr_after_running")==F0128]
    return {"ran":True,"bounded_seconds":seconds,"duration_seconds":round(time.time()-start,3),"method":"pexpect-owned PTY to dosbox-debug under Xvfb; no tmux/capture-pane","route":route,"bplist_negative_control_before_running":bplist_negative_control,"strict_stop_detection_rule":"accept stop only after F5/run, observed (Running), observed prompt -> after running, and post-running disassembly address equals target; reject BPLIST/setup echoes","command_log":command_log,"route_log_path":str(OUT/"route_keylog.json"),"stops":stops,"f0128_target_stops":f0128_stops,"transcript_path":str(OUT/"dosbox_debug_pty.clean.txt")}
def main() -> int:
    ap=argparse.ArgumentParser(); ap.add_argument("--seconds",type=int,default=45); ap.add_argument("--route",default=DEFAULT_ROUTE); args=ap.parse_args(); OUT.mkdir(parents=True,exist_ok=True)
    audit=source_audit(); runtime=runtime_probe(max(10,min(args.seconds,105)),args.route); proven=bool(runtime.get("ran") and runtime.get("f0128_target_stops") and runtime.get("bplist_negative_control_before_running"))
    status="PASS326_DIRECT_PTY_STRICT_F0128_CODE_STOP_PROVEN" if proven else "BLOCKED_PASS326_DIRECT_PTY_F0128_CODE_STOP_NOT_PROVEN"
    manifest={"schema":"pass326_dm1_v1_direct_pty_f0128_code_stop.v1","timestamp_utc":datetime.now(timezone.utc).isoformat(),"status":status,"addresses":{"F0128_DUNGEONVIEW_Draw_CPSF":F0128,"F0097_VIDRV_candidate":F0097,"F0380_COMMAND_ProcessQueue_CPSC":F0380},"source_audit":audit,"runtime_probe":runtime,"decision":{"separable_from_tmux_echo":proven,"tmux_echo_confuser":"BPLIST/setup text is present before any run marker, but pass326 does not count it. A stop requires (Running)->-> transition in an owned PTY.","next_blocker":"Promote this direct-PTY primitive into the F0097/F0380 runtime-evidence probes: after strict F0128 stop, arm 2809:1EFF/2809:1E31 and 22F4:0699 without tmux scraping."}}
    (OUT/"manifest.json").write_text(json.dumps(manifest,indent=2,sort_keys=True)+"\n",encoding="utf-8")
    REPORT.write_text(f"""# Pass326 — direct PTY strict F0128 code-stop proof

Status: `{status}`

## ReDMCSB anchors audited first

- DUNVIEW.C / F0128: `{audit[0]['anchors']}`
- DRAWVIEW.C / F0097: `{audit[1]['anchors']}`
- COMMAND.C / F0380: `{audit[2]['anchors']}`

## Proof harness

`tools/run_dosbox_debug_pty.py` owns `dosbox-debug` with pexpect under Xvfb. It arms `BP 23AD:40FE`, sends F5 as `ESC O t`, executes a bounded DM1 V1 in-game route, and records a real stop only when the PTY transcript shows `(Running)` followed by debugger prompt `->` and post-running code-view address `23AD:40FE`.

BPLIST/setup echoes are recorded as a negative control, not a stop source.

## Next blocker

{manifest['decision']['next_blocker']}

Manifest: `parity-evidence/verification/pass326_dm1_v1_direct_pty_f0128_code_stop/manifest.json`
Transcript: `parity-evidence/verification/pass326_dm1_v1_direct_pty_f0128_code_stop/dosbox_debug_pty.clean.txt`
""", encoding="utf-8")
    print(json.dumps({"status":status,"manifest":str(OUT/"manifest.json"),"f0128_stops":runtime.get("f0128_target_stops",[])},indent=2,sort_keys=True)); return 0 if proven else 2
if __name__ == "__main__": raise SystemExit(main())
