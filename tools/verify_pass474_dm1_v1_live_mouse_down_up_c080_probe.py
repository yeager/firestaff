#!/usr/bin/env python3
"""Pass474: DM1 V1 live mouse down/up primitive and C080 probe.

Prepositions the DOSBox mouse before arming original-runtime breakpoints, then
uses explicit XTest mousedown/mouseup instead of xdotool click. This avoids the
pass464 failure mode where a motion/change-region callback stops the debugger
while the host click has already completed.
"""
from __future__ import annotations
import argparse, json, os, re, shutil, subprocess, tempfile, time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any
import pexpect
import verify_pass385_dm1_v1_corrected_loader_delta_semantic_route as p385

ROOT=Path(__file__).resolve().parents[1]
PASS="pass474_dm1_v1_live_mouse_down_up_c080_probe"
OUT=ROOT/"parity-evidence"/"verification"/PASS
REPORT=ROOT/"parity-evidence"/(PASS+".md")
RED=Path.home()/".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
MAP=Path.home()/".openclaw/data/redmcsb-n2-build-probe/ibm-pc-i34e-fires/HARDDISK/BUILD/I34E/FIRES.MAP"
ADDR={
 "F0781_EventCmp":"2A13:002F",
 "F0359_COMMAND_ProcessClick_CPSC":"22F7:030D",
 "F0380_COMMAND_ProcessQueue_CPSC":"22F7:0699",
 "F0377_COMMAND_ProcessType80_ClickInDungeonView":"1E47:02FE",
 "F0280_CHAMPION_AddCandidateChampionToParty":"1785:0031",
}
EVENT_NAMES={1:"C01_MOUSE_EVENT_RIGHT_BUTTON_DOWN",2:"C02_MOUSE_EVENT_LEFT_BUTTON_DOWN",4:"C04_MOUSE_EVENT_LEFT_BUTTON_UP",8:"C08_MOUSE_EVENT_RIGHT_BUTTON_UP",32:"C32_MOUSE_EVENT_CHANGE_SCREEN_REGION",33:"C33_MOUSE_EVENT_LEAVE_CHAMPION_ICON_REGION"}
EVENT_RE=re.compile(r"\[bp\+0A\].*?=([0-9A-F]{4})",re.I)

def norm(s:str)->str: return " ".join(s.split())
def find_line(p:Path, needle:str)->int|None:
 n=norm(needle)
 for i,line in enumerate(p.read_text(encoding="latin-1",errors="replace").splitlines(),1):
  if n in norm(line): return i
 return None

def source_audit()->list[dict[str,Any]]:
 specs=[
  ("IO.C","callback_gate",["int16_t F0781_MouseHandler","if (P2383_i_MouseEvent < C32_MOUSE_EVENT_CHANGE_SCREEN_REGION)","F0359_COMMAND_ProcessClick_CPSC(P2381_i_X, P2382_i_Y, P2383_i_MouseEvent);"]),
  ("IBMIO.C","driver_down_up",["void F8096_ProcessMouseState(register int16_t P3280_i_MouseX","(*G8067_MouseHandler)(P3280_i_MouseX, P3281_i_MouseY, (P3282_i_RawButtonStatus & 1) ? C02_MOUSE_EVENT_LEFT_BUTTON_DOWN : C04_MOUSE_EVENT_LEFT_BUTTON_UP);","G8091_PreviousRawButtonStatus = P3282_i_RawButtonStatus;"]),
  ("COMMAND.C","c080_queue_dispatch",["{ C080_COMMAND_CLICK_IN_DUNGEON_VIEW,   CM1_SCREEN_RELATIVE, C007_ZONE_VIEWPORT","void F0359_COMMAND_ProcessClick_CPSC","L1109_i_Command = F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0442_ps_SecondaryMouseInput","G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1108_i_CommandQueueIndex].Command = L1109_i_Command","void F0380_COMMAND_ProcessQueue_CPSC","F0377_COMMAND_ProcessType80_ClickInDungeonView(L1161_i_CommandX, L1162_i_CommandY);"]),
  ("CLIKVIEW.C","f0377_front_sensor",["void F0377_COMMAND_ProcessType80_ClickInDungeonView","P0752_i_X -= G2067_i_ViewportScreenX","C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT","F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor"]),
  ("MOVESENS.C","c127_to_f0280",["case C127_SENSOR_WALL_CHAMPION_PORTRAIT","F0280_CHAMPION_AddCandidateChampionToParty(L0758_ui_SensorData)"]),
  ("REVIVE.C","f0280_candidate",["void F0280_CHAMPION_AddCandidateChampionToParty","G0299_ui_CandidateChampionOrdinal","G0305_ui_PartyChampionCount"]),
 ]
 rows=[]
 for fn,section,needles in specs:
  p=RED/fn; hits={needle:find_line(p,needle) for needle in needles} if p.exists() else {}
  miss=[k for k,v in hits.items() if v is None]
  rows.append({"file":fn,"section":section,"path":str(p),"ok":p.exists() and not miss,"lineHits":hits,"missing":miss})
 return rows

def pc_to_window(display:str, win:str, x:int,y:int)->tuple[int,int,dict[str,Any]]:
 ns:dict[str,Any]={}; exec(p385.xdo(display,["getwindowgeometry","--shell",win]).stdout,{},ns)
 gw,gh=float(ns["WIDTH"]),float(ns["HEIGHT"]); aspect=320/200; cw,ch=gw,gw/aspect
 if ch>gh: ch,cw=gh,gh*aspect
 px=int(round((gw-cw)/2+((x+.5)/320)*cw)); py=int(round((gh-ch)/2+((y+.5)/200)*ch))
 return px,py,{"windowGeometry":{k:ns[k] for k in ("X","Y","WIDTH","HEIGHT") if k in ns},"pc":[x,y],"window":[px,py]}

def xmouse(display, win, action, px=None, py=None):
 args=["windowactivate","--sync",win]; p385.xdo(display,args)
 if action=="move": return p385.xdo(display,["mousemove","--window",win,str(px),str(py)])
 if action=="down": return p385.xdo(display,["mousedown","--window",win,"1"])
 if action=="up": return p385.xdo(display,["mouseup","--window",win,"1"])
 raise ValueError(action)

def classify(post:str)->dict[str,Any]:
 c=p385.clean(post); lines=p385.code_lines(c)[-18:]; addr=p385.last_code_addr(c); kind="other"; entry=None
 for name,target in ADDR.items():
  if any(line.upper().startswith(target) for line in lines) or target in c.upper() or addr==target:
   kind=name; entry=target; break
 row={"kind":kind,"entryAddr":entry,"addr":addr,"postRunningCodeLines":lines,"postRunningExcerpt":c[-2600:]}
 if kind=="F0781_EventCmp":
  vals=[int(m.group(1),16) for m in EVENT_RE.finditer(c)]
  if vals:
   ev=vals[-1]; row.update({"eventValue":ev,"eventHex":f"0x{ev:04X}","eventName":EVENT_NAMES.get(ev,"unknown"),"allowsF0359":ev<32})
 return row

def wait_stop(child, transcript, timeout=12):
 deadline=time.time()+timeout; buf=""; saw=False
 while time.time()<deadline:
  chunk=p385.drain(child,.25)
  if chunk:
   transcript.append(chunk); buf+=chunk; c=p385.clean(buf)
   if "(Running)" in c: saw=True
   if saw and "->" in c.split("(Running)",1)[-1]: return classify(c.split("(Running)",1)[-1])
  time.sleep(.04)
 return None

def run_probe(seconds:int, x:int,y:int)->dict[str,Any]:
 missing=[t for t in ["dosbox-debug","Xvfb","xdotool"] if not shutil.which(t)]
 if missing: return {"ran":False,"blocker":"missing tools: "+", ".join(missing)}
 OUT.mkdir(parents=True,exist_ok=True); transcript=[]; cmdlog=[]; routelog=[]; stops=[]; start=time.time(); clicklog=[]
 with tempfile.TemporaryDirectory(prefix="firestaff-pass474-") as td:
  stage=Path(td)/"dos"; shutil.copytree(p385.ORIG,stage); conf=Path(td)/"dosbox.conf"; p385.write_conf(conf,stage)
  display=f":{120+(os.getpid()%40)}"; xvfb=subprocess.Popen(["Xvfb",display,"-screen","0","1024x768x24"],stdout=subprocess.DEVNULL,stderr=subprocess.STDOUT); time.sleep(.5)
  child=pexpect.spawn("dosbox-debug",["-conf",str(conf),"-exit"],env={**os.environ,"DISPLAY":display,"TERM":"vt100"},encoding="utf-8",timeout=2,echo=False); child.delaybeforesend=.05
  try:
   time.sleep(3); transcript.append(p385.drain(child,1)); win=p385.find_win(display)
   if not win: return {"ran":True,"blocker":"dosbox window not found"}
   child.send("\x1bOt"); cmdlog.append({"t":time.time(),"control":"F5","purpose":"run through load prefix unarmed"})
   p385.drive(display,win,p385.LOAD_PREFIX,routelog)
   if not p385.pause_to_prompt(child,display,win,cmdlog,transcript,"post load-prefix"):
    return {"ran":True,"blocker":"no prompt after load prefix","routeLog":routelog,"commandLog":cmdlog,"stops":stops}
   child.send("\x1bOt"); cmdlog.append({"t":time.time(),"control":"F5","purpose":"enter dungeon before mouse-arm"})
   p385.drive(display,win,p385.LOAD_SUFFIX,routelog)
   px,py,geom=pc_to_window(display,win,x,y); r=xmouse(display,win,"move",px,py); clicklog.append({"t":time.time(),"action":"preposition","rc":r.returncode,**geom})
   time.sleep(.4)
   if not p385.pause_to_prompt(child,display,win,cmdlog,transcript,"prepositioned mouse-arm"):
    return {"ran":True,"blocker":"no prompt at prepositioned arm point","routeLog":routelog,"clickLog":clicklog,"commandLog":cmdlog,"stops":stops}
   initial_bp_names=["F0781_EventCmp","F0359_COMMAND_ProcessClick_CPSC"]
   for cmd in ["BPDEL *", *("BP "+ADDR[n] for n in initial_bp_names), "BPLIST"]: p385.dbg(child,cmd,cmdlog,transcript)
   bplist=p385.clean(cmdlog[-1].get("excerpt","")).upper(); retained={n:(ADDR[n] in bplist) for n in ADDR}
   child.send("\x1bOt"); cmdlog.append({"t":time.time(),"control":"F5","purpose":"run armed for explicit mousedown"}); time.sleep(.4)
   r=xmouse(display,win,"down"); clicklog.append({"t":time.time(),"action":"mousedown","rc":r.returncode})
   released=False; downstream_armed=False; deadline=time.time()+seconds
   while time.time()<deadline:
    row=wait_stop(child,transcript,timeout=min(8,max(1,int(deadline-time.time()))))
    if not row: break
    row["t"]=time.time(); stops.append(row)
    for ccmd in ["CPU","BPLIST"]: p385.dbg(child,ccmd,cmdlog,transcript)
    if row.get("kind")=="F0359_COMMAND_ProcessClick_CPSC":
     if not downstream_armed:
      for ccmd in ["BP "+ADDR["F0380_COMMAND_ProcessQueue_CPSC"], "BP "+ADDR["F0377_COMMAND_ProcessType80_ClickInDungeonView"], "BP "+ADDR["F0280_CHAMPION_AddCandidateChampionToParty"], "BPLIST"]: p385.dbg(child,ccmd,cmdlog,transcript)
      downstream_armed=True
     if not released:
      r=xmouse(display,win,"up"); clicklog.append({"t":time.time(),"action":"mouseup_after_f0359_stop","rc":r.returncode}); released=True
    child.send("\x1bOt"); cmdlog.append({"t":time.time(),"control":"F5","purpose":"continue after stop","kind":row.get("kind")})
    if any(s.get("kind")=="F0377_COMMAND_ProcessType80_ClickInDungeonView" for s in stops) or any(s.get("kind")=="F0280_CHAMPION_AddCandidateChampionToParty" for s in stops):
     if released: pass
   if not released:
    r=xmouse(display,win,"up"); clicklog.append({"t":time.time(),"action":"mouseup_final","rc":r.returncode})
   p385.pause_to_prompt(child,display,win,cmdlog,transcript,"final sample"); p385.dbg(child,"BPLIST",cmdlog,transcript)
   return {"ran":True,"durationSeconds":round(time.time()-start,3),"boundedSeconds":seconds,"method":"unarmed entrance-to-dungeon and mouse preposition, then explicit xdotool mousedown/mouseup with original-runtime BPs","clickPc":[x,y],"retainedAtArm":retained,"routeLog":routelog,"clickLog":clicklog,"stops":stops,"commandLog":cmdlog}
  finally:
   try: transcript.append(p385.drain(child,.5)); child.terminate(force=True)
   except Exception: pass
   xvfb.terminate();
   try: xvfb.wait(timeout=5)
   except Exception: xvfb.kill()
   OUT.mkdir(parents=True,exist_ok=True); (OUT/(PASS+"_runtime.clean.txt")).write_text(p385.clean("".join(transcript))[-500000:],encoding="utf-8",errors="replace")
   (OUT/(PASS+"_command_log.json")).write_text(json.dumps(cmdlog,indent=2),encoding="utf-8")
   (OUT/(PASS+"_click_log.json")).write_text(json.dumps(clicklog,indent=2),encoding="utf-8")

def summarize(source,runtime):
 stops=runtime.get("stops",[]) if runtime.get("ran") else []
 kinds=[s.get("kind") for s in stops]
 events=[s for s in stops if s.get("kind")=="F0781_EventCmp"]
 event_values=[s.get("eventValue") for s in events if isinstance(s.get("eventValue"),int)]
 preds={"sourceAuditOk":all(r.get("ok") for r in source),"runtimeRan":runtime.get("ran") is True,"sampledEventValues":event_values,"sampledEventNames":[EVENT_NAMES.get(v,"unknown") for v in event_values],"hasLeftDown":2 in event_values,"hasLeftUp":4 in event_values,"f0359Hit":"F0359_COMMAND_ProcessClick_CPSC" in kinds,"f0380Hit":"F0380_COMMAND_ProcessQueue_CPSC" in kinds,"f0377Hit":"F0377_COMMAND_ProcessType80_ClickInDungeonView" in kinds,"f0280Hit":"F0280_CHAMPION_AddCandidateChampionToParty" in kinds,"kinds":kinds}
 if not preds["sourceAuditOk"]: return "FAIL_PASS474_SOURCE_AUDIT",preds,"ReDMCSB source audit missing required anchors"
 if not preds["runtimeRan"]: return "BLOCKED_PASS474_RUNTIME_NOT_RUN",preds,runtime.get("blocker","runtime did not run")
 if preds["hasLeftDown"] and preds["f0359Hit"] and (preds["f0377Hit"] or preds["f0280Hit"]): return "PASS_PASS474_MOUSE_DOWN_F0359_C080_PATH_REACHED",preds,"explicit mousedown sampled C02 and reached F0359 plus C080 downstream path"
 if preds["hasLeftDown"] and preds["f0359Hit"]: return "PASS_PASS474_MOUSE_DOWN_REACHES_F0359",preds,"explicit mousedown sampled C02 and reached F0359; downstream C080/F0377 was not reached in bounded run"
 if event_values: return "BLOCKED_PASS474_MOUSE_EVENT_SAMPLED_NO_F0359",preds,"mouse callback sampled events but did not reach F0359 in bounded run"
 return "BLOCKED_PASS474_NO_MOUSE_EVENT_SAMPLE",preds,runtime.get("blocker","no strict mouse event sample")

def main():
 ap=argparse.ArgumentParser(); ap.add_argument("--seconds",type=int,default=35); ap.add_argument("--x",type=int,default=111); ap.add_argument("--y",type=int,default=82); args=ap.parse_args()
 source=source_audit(); runtime=run_probe(args.seconds,args.x,args.y); status,preds,summary=summarize(source,runtime)
 manifest={"schema":PASS+".v1","timestampUtc":datetime.now(timezone.utc).isoformat(),"status":status,"summary":summary,"addresses":ADDR,"map":str(MAP),"sourceAudit":source,"runtime":runtime,"predicates":preds,"artifacts":{"manifest":str(OUT/"manifest.json"),"transcript":str(OUT/(PASS+"_runtime.clean.txt")),"commandLog":str(OUT/(PASS+"_command_log.json")),"clickLog":str(OUT/(PASS+"_click_log.json"))}}
 OUT.mkdir(parents=True,exist_ok=True); (OUT/"manifest.json").write_text(json.dumps(manifest,indent=2),encoding="utf-8")
 lines=["# Pass474 — DM1 V1 live mouse down/up C080 probe","",f"Status: `{status}`","",summary,"","## ReDMCSB source audit"]
 for r in source:
  lines.append(f"- `{r['file']}` `{r['section']}` ok=`{r['ok']}`")
  for n,l in r.get("lineHits",{}).items(): lines.append(f"  - line {l}: `{n}`")
 lines += ["","## Runtime predicates",f"- sampled events: `{preds['sampledEventNames']}` / `{[hex(v) for v in preds['sampledEventValues']]}`",f"- stops: `{preds['kinds']}`",f"- F0359 hit: `{preds['f0359Hit']}`; F0380 hit: `{preds['f0380Hit']}`; F0377 hit: `{preds['f0377Hit']}`; F0280 hit: `{preds['f0280Hit']}`","","## Artifacts",f"- Manifest: `{(OUT/'manifest.json').relative_to(ROOT)}`",f"- Transcript: `{(OUT/(PASS+'_runtime.clean.txt')).relative_to(ROOT)}`",f"- Command log: `{(OUT/(PASS+'_command_log.json')).relative_to(ROOT)}`",f"- Click log: `{(OUT/(PASS+'_click_log.json')).relative_to(ROOT)}`",""]
 REPORT.write_text("\n".join(lines),encoding="utf-8")
 print(json.dumps({"status":status,"summary":summary,"predicates":preds,"report":str(REPORT)},indent=2))
 raise SystemExit(0 if status.startswith(("PASS","BLOCKED")) else 1)
if __name__=="__main__": main()
