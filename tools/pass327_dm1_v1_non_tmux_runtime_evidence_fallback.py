#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, os, re, shlex, shutil, subprocess, tempfile, threading, time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass327_dm1_v1_non_tmux_runtime_evidence_fallback"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
OUT_JSON = ROOT / "parity-evidence/verification" / f"{PASS}.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
ORIG = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
SRC = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
PASS320 = ROOT / "parity-evidence/verification/pass320_dm1_v1_f0097_vidrv_window_after_f0128_sequence_probe.json"
PASS321 = ROOT / "parity-evidence/verification/pass321_dm1_v1_debugger_code_stop_control_sequence_probe/manifest.json"
PASS322 = ROOT / "parity-evidence/verification/pass322_dm1_v1_movement_state_binding.json"
ADDR = {"F0380_COMMAND_ProcessQueue_CPSC":"22F4:0699","F0128_DUNGEONVIEW_Draw_CPSF":"23AD:40FE","F0097_DUNGEONVIEW_DrawViewport_entry":"2809:1E31","F0097_after_palette_zone_setup":"2809:1EBD","F0097_before_viewport_args":"2809:1EEE","F0097_VIDRV_09_BlitViewPort_indirect_call":"2809:1EFF"}
MOVEMENT_BPM=["2C20:3E7E","2C20:3E84","2C20:3E8A","2C20:3E90","2C20:3E96","2C20:3EC8","2C20:1F08","2C20:1F0A","2C20:3C92","2C20:3C94","2C20:3CE0","2C20:3C9A","2C20:3D28","2C20:3CE4"]
DEFAULT_ROUTE="wait:9000 enter wait:1800 one wait:1800 click:276,140 wait:1800 one wait:1800 kp5 wait:900 kp4 wait:900 kp6 wait:900 kp5"
CODE_STOP_PATTERNS=[re.compile(r"^(?:CODE\s+STOP|CODE\s+BREAKPOINT|BREAKPOINT\s+HIT)\s*:?[ \t]+(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\b",re.I),re.compile(r"^(?:Code|CPU)\s+breakpoint\s+(?:hit\s+)?(?:at\s+)?(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\b",re.I),re.compile(r"^Breakpoint\s+(?:hit\s+)?at\s+(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\b",re.I)]
SETUP_ECHO_PATTERNS=[re.compile(r"^->"),re.compile(r"^Breakpoint list:?",re.I),re.compile(r"^-{5,}"),re.compile(r"^[0-9]+\.\s+BP\s+[0-9A-F]{4}:[0-9A-F]{4}\b",re.I),re.compile(r"^BP\s+[0-9A-F]{4}:[0-9A-F]{4}\b",re.I),re.compile(r"^\(Running\).*\bbreakpoint\s+at\s+[0-9A-F]{4}:[0-9A-F]{4}\b",re.I)]

def run(cmd:list[str],**kw:Any)->subprocess.CompletedProcess[str]: return subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,**kw)
def clean(text:str)->str:
    text=re.sub(r"\x1b\[[0-9;?]*[ -/]*[@-~]","",text).replace("\r","\n")
    return re.sub(r"[\x00-\x08\x0b\x0c\x0e-\x1f]","",text)
def norm(s:str)->str: return " ".join(s.split())
def source_line(file:str,needle:str,start:int=1,end:int|None=None)->int|None:
    lines=(SRC/file).read_text(encoding="latin-1",errors="replace").splitlines(); n=norm(needle); e=end or len(lines)
    for i in range(start-1,min(e,len(lines))):
        if n in norm(lines[i]): return i+1
    return None

def source_audit()->list[dict[str,Any]]:
    specs=[("DRAWVIEW.C","f0097_entry","void F0097_DUNGEONVIEW_DrawViewport",709,730),("DRAWVIEW.C","f0097_sets_draw_request","G0324_B_DrawViewportRequested = C1_TRUE",709,730),("DRAWVIEW.C","f0097_blits_to_screen","F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport",830,865),("DRAWVIEW.C","f0097_vidrv_slot9_call","(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);",830,865),("DUNVIEW.C","f0128_entry","void F0128_DUNGEONVIEW_Draw_CPSF",8318,8335),("DUNVIEW.C","f0128_ceiling_viewport_bitmap","F0674_F0128_sub(G2109_Ceiling, G0296_puc_Bitmap_Viewport);",8360,8375),("DUNVIEW.C","f0128_calls_f0097_not_entrance","F0097_DUNGEONVIEW_DrawViewport(G0309_i_PartyMapIndex != C255_MAP_INDEX_ENTRANCE);",8600,8612),("DUNVIEW.C","f0128_calls_f0097_dungeon_view","F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",8600,8612),("COMMAND.C","f0380_entry","void F0380_COMMAND_ProcessQueue_CPSC",2045,2065),("COMMAND.C","f0380_dequeues_command","G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command",2045,2125),("COMMAND.C","f0380_dispatches_move","F0366_COMMAND_ProcessTypes3To6_MoveParty",2045,2156),("CLIKMENU.C","move_party_entry","F0366_COMMAND_ProcessTypes3To6_MoveParty",180,205),("MOVESENS.C","move_result_commit_x","G0306_i_PartyMapX = P0560_i_DestinationMapX",316,506),("MOVESENS.C","move_draw_while_falling","F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, P0560_i_DestinationMapX, P0561_i_DestinationMapY)",550,556)]
    rows=[]
    for file,ident,needle,start,end in specs:
        line=source_line(file,needle,start,end); rows.append({"id":ident,"file":file,"function_or_anchor":needle,"lineRange":[start,end],"line":line,"ok":line is not None})
    return rows

def strict_code_stop_lines(text:str)->list[dict[str,str]]:
    out=[]
    for raw in clean(text).splitlines():
        s=raw.strip()
        if not s or any(p.search(s) for p in SETUP_ECHO_PATTERNS): continue
        for pat in CODE_STOP_PATTERNS:
            m=pat.search(s)
            if m: out.append({"addr":m.group("addr").upper(),"line":s[:180]}); break
    return out

def parser_selftest()->dict[str,Any]:
    samples={"bp_command_echo":"-> BP 23AD:40FE_  breakpoint at 23AD:40FE","bplist_header":"Breakpoint list:","bplist_entry":"00. BP 2809:1EFF","running_echo":"(Running)t breakpoint at 23AD:40FE","actual_f0128":"CODE STOP: 23AD:40FE","actual_vidrv":"Code breakpoint at 2809:1EFF","actual_f0380":"Breakpoint hit at 22F4:0699"}
    accepted={k for k,v in samples.items() if strict_code_stop_lines(v)}; expected={"actual_f0128","actual_vidrv","actual_f0380"}
    return {"ok":accepted==expected,"accepted":sorted(accepted),"expected":sorted(expected)}

def write_conf(path:Path,stage:Path)->None:
    path.write_text("\n".join(["[sdl]","fullscreen=false","output=surface","usescancodes=false","[dosbox]","machine=svga_paradise","memsize=4","[cpu]","core=normal","cycles=3000","[mixer]","nosound=true","[autoexec]",f"mount c {shlex.quote(str(stage))}","c:","DEBUG DM.EXE -vv -sn -pk",""]),encoding="utf-8")
def xdo(display:str,args:list[str])->subprocess.CompletedProcess[str]: return run(["xdotool",*args],env={**os.environ,"DISPLAY":display},timeout=10)
def key_name(tok:str)->str: return {"enter":"Return","return":"Return","one":"1","1":"1","kp4":"KP_Left","kp5":"KP_Begin","kp6":"KP_Right"}[tok]
def drive_route(display:str,pid:int,route:str,stop:threading.Event,log:list[dict[str,Any]])->None:
    try:
        wins=xdo(display,["search","--sync","--pid",str(pid)]).stdout.strip().splitlines()
        if not wins: log.append({"event":"no_window_for_pid","pid":pid}); return
        window=wins[0]; xdo(display,["windowactivate","--sync",window]); xdo(display,["windowfocus","--sync",window])
        for item in route.split():
            if stop.is_set(): break
            low=item.lower(); log.append({"t":time.time(),"event":"route_step","route_item":item})
            if low.startswith("wait:"):
                end=time.time()+int(low.split(":",1)[1])/1000
                while time.time()<end and not stop.is_set(): time.sleep(.05)
            elif low.startswith("click:"):
                x,y=map(int,low.split(":",1)[1].split(",")); xdo(display,["mousemove","--window",window,str(x*2),str(y*2),"click","1"]); log.append({"t":time.time(),"event":"click","route_item":item})
            else:
                xdo(display,["key","--window",window,key_name(low)]); log.append({"t":time.time(),"event":"key","route_item":item}); time.sleep(.2)
        log.append({"t":time.time(),"event":"route_done"})
    except Exception as e: log.append({"event":"route_error","error":repr(e)})
def classify(addr:str)->str:
    if addr==ADDR["F0128_DUNGEONVIEW_Draw_CPSF"]: return "f0128_code_stop"
    if addr==ADDR["F0380_COMMAND_ProcessQueue_CPSC"]: return "f0380_code_stop"
    for k in ["F0097_DUNGEONVIEW_DrawViewport_entry","F0097_after_palette_zone_setup","F0097_before_viewport_args","F0097_VIDRV_09_BlitViewPort_indirect_call"]:
        if addr==ADDR[k]: return "f0097_code_stop:"+k
    if addr in MOVEMENT_BPM: return "movement_memory_stop"
    return "other_code_stop"

def runtime_probe(seconds:int,route:str)->dict[str,Any]:
    try: import pexpect
    except Exception as e: return {"ran":False,"blocker":f"pexpect unavailable: {e}"}
    missing=[x for x in ["dosbox-debug","Xvfb","xdotool"] if not shutil.which(x)]
    if missing: return {"ran":False,"blocker":"missing runtime tools: "+", ".join(missing)}
    display=f":{80+(os.getpid()%10)}"; events=[]; hits=[]; route_log=[]; transcript=[]; stop=threading.Event(); start=time.time()
    with tempfile.TemporaryDirectory(prefix="firestaff-pass327-") as td:
        stage=Path(td)/"dos"; shutil.copytree(ORIG,stage); conf=Path(td)/"dosbox.conf"; write_conf(conf,stage)
        xvfb=subprocess.Popen(["Xvfb",display,"-screen","0","1024x768x24"],stdout=subprocess.DEVNULL,stderr=subprocess.STDOUT); time.sleep(.5); child=None
        try:
            child=pexpect.spawn(f"dosbox-debug -conf {shlex.quote(str(conf))} -exit",env={**os.environ,"DISPLAY":display,"TERM":"vt100"},encoding="utf-8",timeout=1,maxread=8192)
            time.sleep(3)
            def send(cmd:str)->None:
                events.append({"t":time.time(),"event":"debugger_cmd","cmd":cmd}); child.sendline(cmd); time.sleep(.25)
            send("BPDEL *"); send(f"BP {ADDR['F0128_DUNGEONVIEW_Draw_CPSF']}")
            for addr in MOVEMENT_BPM[:5]: send(f"BPM {addr}")
            child.send("\x1bOt"); threading.Thread(target=drive_route,args=(display,child.pid,route,stop,route_log),daemon=True).start()
            deadline=time.time()+seconds; seen=set(); f0128_seen=False
            while time.time()<deadline:
                try:
                    child.expect([r"\r?\n",pexpect.TIMEOUT,pexpect.EOF],timeout=.5); chunk=child.before or ""
                    if chunk:
                        transcript.append(chunk)
                        for hit in strict_code_stop_lines(chunk):
                            key=(hit["addr"],hit["line"])
                            if key in seen: continue
                            seen.add(key); row={"t":time.time(),**hit,"kind":classify(hit["addr"])}; hits.append(row)
                            if row["kind"]=="f0128_code_stop" and not f0128_seen:
                                f0128_seen=True; send("BPDEL *")
                                for a in [ADDR[k] for k in ["F0097_DUNGEONVIEW_DrawViewport_entry","F0097_after_palette_zone_setup","F0097_before_viewport_args","F0097_VIDRV_09_BlitViewPort_indirect_call","F0380_COMMAND_ProcessQueue_CPSC"]]: send(f"BP {a}")
                                for addr in MOVEMENT_BPM: send(f"BPM {addr}")
                                child.send("\x1bOt")
                except Exception as e: transcript.append(f"\n[pexpect_error {e!r}]\n"); break
        finally:
            stop.set()
            if child is not None:
                try: child.close(force=True)
                except Exception: pass
            xvfb.terminate()
            try: xvfb.wait(timeout=5)
            except subprocess.TimeoutExpired: xvfb.kill()
    text=clean("\n".join(transcript)); text="\n".join(line.rstrip() for line in text.splitlines())+"\n"; OUT_DIR.mkdir(parents=True,exist_ok=True)
    (OUT_DIR/"non_tmux_pexpect_stream.clean.txt").write_text(text[-400000:],encoding="utf-8"); (OUT_DIR/"non_tmux_route_keylog.json").write_text(json.dumps(route_log,indent=2,sort_keys=True)+"\n",encoding="utf-8")
    f0128=any(h["kind"]=="f0128_code_stop" for h in hits); f0097=[h for h in hits if h["kind"].startswith("f0097_code_stop")]; movement=[h for h in hits if h["kind"] in {"f0380_code_stop","movement_memory_stop"}]
    return {"ran":True,"durationSeconds":round(time.time()-start,3),"method":"pexpect_raw_pty_no_tmux","usedTmux":False,"events":events[:120],"hits":hits[:80],"directHits":{"f0128":f0128,"f0097AfterF0128":bool(f0128 and f0097),"movementOrF0380":bool(movement)},"routeLogCount":len(route_log),"streamExcerpt":[l[:180] for l in text.splitlines() if re.search(r"BP|Breakpoint|CODE|Running|23AD|2809|22F4|2C20",l,re.I)][:80]}

def load_json(path:Path)->dict[str,Any]: return json.loads(path.read_text(encoding="utf-8")) if path.exists() else {}
def build(seconds:int,route:str,no_runtime:bool)->dict[str,Any]:
    src=source_audit(); parser=parser_selftest(); prior={"pass320Status":load_json(PASS320).get("status"),"pass321Status":load_json(PASS321).get("status"),"pass322Status":load_json(PASS322).get("status")}
    runtime={"ran":False,"blocker":"--no-runtime requested"} if no_runtime else runtime_probe(seconds,route)
    if runtime.get("directHits",{}).get("f0097AfterF0128") and runtime.get("directHits",{}).get("movementOrF0380"): status="PASS327_NON_TMUX_RUNTIME_EVIDENCE_CAPTURED"
    elif runtime.get("ran"): status="BLOCKED_PASS327_NON_TMUX_RAW_STREAM_NO_STRICT_CODE_STOP_PROMOTION"
    else: status="BLOCKED_PASS327_NON_TMUX_RUNTIME_NOT_AVAILABLE"
    return {"schema":f"{PASS}.v1","timestampUtc":datetime.now(timezone.utc).isoformat(),"status":status,"ok":True,"sourceRoot":str(SRC),"sourceAudit":src,"parserSelftest":parser,"priorArtifacts":prior,"addresses":{**ADDR,"movementBpm":MOVEMENT_BPM},"fallbackPath":{"control":"pexpect raw PTY","tmuxRequired":False,"xDisplay":"Xvfb isolated","routeDriver":"xdotool window by child pid","boundedSeconds":seconds},"promotionRules":{"sourceLockPreserved":all(r["ok"] for r in src) and parser["ok"],"allowedPromotionOnlyIf":["strict F0128 code stop","later strict F0097/VIDRV code stop","movement/F0380 or movement memory stop from pass322 addresses","order is observed in one bounded run"],"notPromotedBy":["BP command echoes","BPLIST entries","tmux pane noise","source-only address binding"]},"runtimeProbe":runtime}
def render_md(m:dict[str,Any])->str:
    lines=["# Pass327 — DM1 V1 non-tmux runtime evidence fallback","",f"Status: `{m['status']}`","","## ReDMCSB source audit",""]
    for r in m["sourceAudit"]: lines.append(f"- {'PASS' if r['ok'] else 'FAIL'} `{r['file']}:{r['line']}` — {r['id']} / `{r['function_or_anchor']}`")
    rt=m["runtimeProbe"]
    lines += ["","## Fallback path","","- Uses `pexpect` raw PTY control of `dosbox-debug`; `tmuxRequired=false`.","- Uses strict pass321 parser rules: setup echoes/BPLIST do not count as runtime stops.","- Pre-arms source/public-symbol-locked F0128 plus movement BPM fallback addresses from pass322; arms F0097/VIDRV only after a real F0128 stop.","","## Runtime result","",f"- ran: `{rt.get('ran')}`; method: `{rt.get('method')}`; usedTmux: `{rt.get('usedTmux')}`",f"- direct hits: `{rt.get('directHits')}`","","## Decision","","No runtime promotion unless strict F0128 → F0097/VIDRV → movement/F0380 evidence appears in order. This pass creates/verifies the non-tmux fallback path; current run remains blocked if the raw stream still lacks strict code-stop lines.",""]
    return "\n".join(lines)
def main()->int:
    ap=argparse.ArgumentParser(); ap.add_argument("--seconds",type=int,default=35); ap.add_argument("--route",default=DEFAULT_ROUTE); ap.add_argument("--no-runtime",action="store_true"); args=ap.parse_args()
    m=build(args.seconds,args.route,args.no_runtime); OUT_DIR.mkdir(parents=True,exist_ok=True); OUT_JSON.write_text(json.dumps(m,indent=2,sort_keys=True)+"\n",encoding="utf-8"); REPORT.write_text(render_md(m),encoding="utf-8")
    print(json.dumps({"status":m["status"],"runtimeRan":m["runtimeProbe"].get("ran"),"usedTmux":m["runtimeProbe"].get("usedTmux"),"out":str(OUT_JSON)},indent=2,sort_keys=True)); return 0
if __name__=="__main__": raise SystemExit(main())
