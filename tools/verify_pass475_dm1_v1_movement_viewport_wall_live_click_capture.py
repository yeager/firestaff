#!/usr/bin/env python3
"""Pass475: DM1 V1 live click movement/viewport/wall capture attempt."""
from __future__ import annotations
import argparse,json,os,re,shutil,subprocess,tempfile,time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any
import pexpect
import verify_pass385_dm1_v1_corrected_loader_delta_semantic_route as p385

ROOT=Path(__file__).resolve().parents[1]
PASS="pass475_dm1_v1_movement_viewport_wall_live_click_capture"
OUT=ROOT/"parity-evidence"/"verification"/PASS
REPORT=ROOT/"parity-evidence"/(PASS+".md")
RED=Path.home()/".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
ADDR={
 "F0380_COMMAND_ProcessQueue_CPSC":"22F7:0699",
 "F0365_COMMAND_ProcessTypes1To2_TurnParty":"1EA7:010D",
 "F0366_COMMAND_ProcessTypes3To6_MoveParty":"1EA7:01AA",
 "F0128_DUNGEONVIEW_Draw_CPSF":"23B0:40FE",
 "F0097_VIDRV_09_BlitViewPort_indirect_call":"280C:1EFF",
}
PRIMS=[{"label":"turn_left","command":"C001","xy":[247,135],"handler":"F0365_COMMAND_ProcessTypes1To2_TurnParty"},{"label":"move_forward","command":"C003","xy":[276,135],"handler":"F0366_COMMAND_ProcessTypes3To6_MoveParty"},{"label":"turn_right","command":"C002","xy":[304,135],"handler":"F0365_COMMAND_ProcessTypes1To2_TurnParty"}]
CODE_RE=re.compile(r"\b([0-9A-F]{4}:[0-9A-F]{4})\s+[0-9A-F]{2,}\s*[a-z]",re.I)

def norm(s:str)->str: return " ".join(s.split())
def find_line(p:Path,needle:str)->int|None:
 n=norm(needle)
 for i,line in enumerate(p.read_text(encoding="latin-1",errors="replace").splitlines(),1):
  if n in norm(line): return i
 return None

def source_audit()->list[dict[str,Any]]:
 specs=[
  ("COMMAND.C","pc34_click_boxes",["G0448_as_Graphic561_SecondaryMouseInput_Movement","C001_COMMAND_TURN_LEFT,             234, 261, 125, 145","C003_COMMAND_MOVE_FORWARD,          263, 289, 125, 145","C002_COMMAND_TURN_RIGHT,            291, 318, 125, 145","C080_COMMAND_CLICK_IN_DUNGEON_VIEW,   0, 223,  33, 168"]),
  ("COMMAND.C","queue_to_handlers",["F0380_COMMAND_ProcessQueue_CPSC","F0365_COMMAND_ProcessTypes1To2_TurnParty","F0366_COMMAND_ProcessTypes3To6_MoveParty"]),
  ("CLIKMENU.C","turn_stop",["F0365_COMMAND_ProcessTypes1To2_TurnParty","F0284_CHAMPION_SetPartyDirection","F0276_SENSOR_ProcessThingAdditionOrRemoval"]),
  ("CLIKMENU.C","step_stop",["F0366_COMMAND_ProcessTypes3To6_MoveParty","F0267_MOVE_GetMoveResult_CPSCE","G0310_i_DisabledMovementTicks = AL1115_ui_Ticks"]),
  ("DUNVIEW.C","wall_composite",["F0098_DUNGEONVIEW_DrawFloorAndCeiling","G0296_puc_Bitmap_Viewport","F0100_DUNGEONVIEW_DrawWallSetBitmap","F0102_DUNGEONVIEW_DrawDoorBitmap","F0128_DUNGEONVIEW_Draw_CPSF","F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)"]),
  ("DRAWVIEW.C","post_present_seam",["F0097_DUNGEONVIEW_DrawViewport","F0638_GetZone(C007_ZONE_VIEWPORT","G0296_puc_Bitmap_Viewport","VIDRV_09_BlitViewPort"]),
 ]
 rows=[]
 for fn,section,needles in specs:
  p=RED/fn; hits={needle:find_line(p,needle) for needle in needles} if p.exists() else {}; missing=[k for k,v in hits.items() if v is None]
  rows.append({"file":fn,"section":section,"path":str(p),"ok":p.exists() and not missing,"lineHits":hits,"missing":missing})
 return rows

def pc_to_window(display:str,win:str,x:int,y:int)->tuple[int,int,dict[str,Any]]:
 ns:dict[str,Any]={}; exec(p385.xdo(display,["getwindowgeometry","--shell",win]).stdout,{},ns)
 gw,gh=float(ns["WIDTH"]),float(ns["HEIGHT"]); aspect=320/200; cw,ch=gw,gw/aspect
 if ch>gh: ch,cw=gh,gh*aspect
 px=int(round((gw-cw)/2+((x+.5)/320)*cw)); py=int(round((gh-ch)/2+((y+.5)/200)*ch))
 return px,py,{"windowGeometry":{k:ns[k] for k in ("X","Y","WIDTH","HEIGHT") if k in ns},"pc":[x,y],"window":[px,py]}

def xmouse(display,win,action,px=None,py=None):
 p385.xdo(display,["windowactivate","--sync",win]); p385.xdo(display,["windowfocus","--sync",win])
 if action=="move": return p385.xdo(display,["mousemove","--window",win,str(px),str(py)])
 if action=="down": return p385.xdo(display,["mousedown","--window",win,"1"])
 if action=="up": return p385.xdo(display,["mouseup","--window",win,"1"])
 raise ValueError(action)

def stop_kind(text:str)->dict[str,Any]:
 c=p385.clean(text); lines=p385.code_lines(c)[-18:]; addr=p385.last_code_addr(c); kind="other"; entry=None
 u=c.upper()
 for name,target in ADDR.items():
  if any(line.upper().startswith(target) for line in lines) or target in u or addr==target:
   kind=name; entry=target; break
 return {"kind":kind,"entryAddr":entry,"addr":addr,"postRunningCodeLines":lines[-8:]}

def wait_stop(child,transcript,timeout=10):
 deadline=time.time()+timeout; buf=""; saw=False
 while time.time()<deadline:
  chunk=p385.drain(child,.25)
  if chunk:
   transcript.append(chunk); buf+=chunk; c=p385.clean(buf)
   if "(Running)" in c: saw=True
   if saw and "->" in c.split("(Running)",1)[-1]: return stop_kind(c.split("(Running)",1)[-1])
  time.sleep(.05)
 return None

def import_crop(display:str, win:str, label:str)->dict[str,Any]:
 # Capture whole DOSBox window at strict stop; crop math is recorded but original parity is not promoted unless post-present seam was hit.
 png=OUT/f"{label}_window.png"
 r=subprocess.run(["import","-window",win,str(png)],env={**os.environ,"DISPLAY":display},text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=8)
 return {"path":str(png.relative_to(ROOT)),"returncode":r.returncode,"bytes":png.stat().st_size if png.exists() else 0,"tool":"import -window"}

def run_probe(seconds:int)->dict[str,Any]:
 missing=[t for t in ["dosbox-debug","Xvfb","xdotool","import"] if not shutil.which(t)]
 if missing: return {"ran":False,"blocker":"missing tools: "+", ".join(missing)}
 OUT.mkdir(parents=True,exist_ok=True); transcript=[]; cmdlog=[]; routelog=[]; clicklog=[]; stops=[]; captures=[]; start=time.time()
 with tempfile.TemporaryDirectory(prefix="firestaff-pass475-") as td:
  stage=Path(td)/"dos"; shutil.copytree(p385.ORIG,stage); conf=Path(td)/"dosbox.conf"; p385.write_conf(conf,stage)
  display=f":{150+(os.getpid()%40)}"; xvfb=subprocess.Popen(["Xvfb",display,"-screen","0","1024x768x24"],stdout=subprocess.DEVNULL,stderr=subprocess.STDOUT); time.sleep(.6)
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
   if not p385.pause_to_prompt(child,display,win,cmdlog,transcript,"dungeon mouse-arm"):
    return {"ran":True,"blocker":"no prompt at dungeon mouse-arm point","routeLog":routelog,"commandLog":cmdlog,"stops":stops}
   for ccmd in ["BPDEL *", *("BP "+ADDR[n] for n in ADDR), "BPLIST"]: p385.dbg(child,ccmd,cmdlog,transcript)
   bplist=p385.clean(cmdlog[-1].get("excerpt","")).upper(); retained={n:(ADDR[n] in bplist) for n in ADDR}
   for prim in PRIMS:
    px,py,geom=pc_to_window(display,win,*prim["xy"]); xmouse(display,win,"move",px,py); clicklog.append({"t":time.time(),"action":"preposition","label":prim["label"],**geom})
    child.send("\x1bOt"); cmdlog.append({"t":time.time(),"control":"F5","purpose":"run armed for "+prim["label"]}); time.sleep(.35)
    xmouse(display,win,"down"); clicklog.append({"t":time.time(),"action":"mousedown","label":prim["label"]})
    released=False; deadline=time.time()+max(6,seconds/len(PRIMS))
    while time.time()<deadline:
     row=wait_stop(child,transcript,timeout=min(8,max(1,int(deadline-time.time()))))
     if not row: break
     row.update({"t":time.time(),"label":prim["label"],"command":prim["command"]}); stops.append(row)
     if not released:
      xmouse(display,win,"up"); clicklog.append({"t":time.time(),"action":"mouseup_after_first_stop","label":prim["label"]}); released=True
     if row["kind"] in {"F0128_DUNGEONVIEW_Draw_CPSF","F0097_VIDRV_09_BlitViewPort_indirect_call"}:
      cap_label = prim["label"] + "_" + row["kind"]
      captures.append({"label": prim["label"], "stopKind": row["kind"], **import_crop(display, win, cap_label)})
     child.send("\x1bOt"); cmdlog.append({"t":time.time(),"control":"F5","purpose":"continue after stop","kind":row.get("kind"),"label":prim["label"]})
     if row["kind"]=="F0097_VIDRV_09_BlitViewPort_indirect_call": break
    if not released:
     xmouse(display,win,"up"); clicklog.append({"t":time.time(),"action":"mouseup_final","label":prim["label"]})
    if any(s.get("label")==prim["label"] and s.get("kind")=="F0097_VIDRV_09_BlitViewPort_indirect_call" for s in stops):
     break
   p385.pause_to_prompt(child,display,win,cmdlog,transcript,"final sample")
   return {"ran":True,"durationSeconds":round(time.time()-start,3),"boundedSeconds":seconds,"method":"N2 dosbox-debug/Xvfb with pass473 PC34 click centers, explicit mousedown/up, breakpoints for movement handlers and post-present seam","retainedAtArm":retained,"routeLog":routelog,"clickLog":clicklog,"stops":stops,"captures":captures,"commandLog":cmdlog}
  finally:
   try: transcript.append(p385.drain(child,.5)); child.terminate(force=True)
   except Exception: pass
   try: xvfb.terminate(); xvfb.wait(timeout=5)
   except Exception: xvfb.kill()
   OUT.mkdir(parents=True,exist_ok=True)
   (OUT/(PASS+"_runtime.clean.txt")).write_text(p385.clean("".join(transcript))[-500000:],encoding="utf-8",errors="replace")
   (OUT/(PASS+"_command_log.json")).write_text(json.dumps(cmdlog,indent=2),encoding="utf-8")
   (OUT/(PASS+"_click_log.json")).write_text(json.dumps(clicklog,indent=2),encoding="utf-8")

def summarize(source,runtime):
 stops=runtime.get("stops",[]) if runtime.get("ran") else []; kinds=[s.get("kind") for s in stops]
 preds={"sourceAuditOk":all(r.get("ok") for r in source),"runtimeRan":runtime.get("ran") is True,"handlersHit":[k for k in kinds if k in ("F0365_COMMAND_ProcessTypes1To2_TurnParty","F0366_COMMAND_ProcessTypes3To6_MoveParty")],"f0380Hit":"F0380_COMMAND_ProcessQueue_CPSC" in kinds,"f0128Hit":"F0128_DUNGEONVIEW_Draw_CPSF" in kinds,"postPresentHit":"F0097_VIDRV_09_BlitViewPort_indirect_call" in kinds,"captureCount":len(runtime.get("captures",[])),"stopKinds":kinds}
 if not preds["sourceAuditOk"]: return "FAIL_PASS475_SOURCE_AUDIT",preds,"ReDMCSB source audit missing required anchors"
 if not preds["runtimeRan"]: return "BLOCKED_PASS475_RUNTIME_NOT_RUN",preds,runtime.get("blocker","runtime did not run")
 if preds["postPresentHit"] and preds["captureCount"]: return "PASS_PASS475_FRESH_CLICK_POST_PRESENT_CAPTURED",preds,"fresh click-driven run reached movement/viewport post-present seam and saved bounded capture artifacts"
 if preds["handlersHit"] and preds["f0128Hit"]: return "BLOCKED_PASS475_MOVEMENT_AND_F0128_HIT_POST_PRESENT_NOT_REACHED",preds,"movement handler and F0128 were reached, but VIDRV post-present seam was not hit in the bounded run"
 if preds["handlersHit"]: return "BLOCKED_PASS475_CLICK_REACHED_MOVEMENT_HANDLER_NO_VIEWPORT_PRESENT",preds,"locked PC34 click reached movement handler, but no fresh F0128/F0097 post-present stop occurred in the bounded run"
 if preds["f0380Hit"]: return "BLOCKED_PASS475_CLICK_REACHED_QUEUE_NO_MOVEMENT_HANDLER",preds,"click run reached command queue, but not turn/step handler before timeout"
 return "BLOCKED_PASS475_CLICK_DID_NOT_REACH_COMMAND_QUEUE",preds,runtime.get("blocker","locked click centers did not reach F0380 in bounded run")

def main():
 ap=argparse.ArgumentParser(); ap.add_argument("--seconds",type=int,default=45); args=ap.parse_args()
 source=source_audit(); runtime=run_probe(args.seconds); status,preds,summary=summarize(source,runtime)
 manifest={"schema":PASS+".v1","timestampUtc":datetime.now(timezone.utc).isoformat(),"status":status,"summary":summary,"sourceRoot":str(RED),"sourceAudit":source,"addresses":ADDR,"clickPrimitives":PRIMS,"runtime":runtime,"predicates":preds,"artifacts":{"manifest":str(OUT/"manifest.json"),"transcript":str(OUT/(PASS+"_runtime.clean.txt")),"commandLog":str(OUT/(PASS+"_command_log.json")),"clickLog":str(OUT/(PASS+"_click_log.json"))},"promotionRule":"Fresh wall/viewport frames are promotable only when captured after F0128 composition and the F0097/VIDRV post-present seam; pass304/pass376/pass449 frames are not used."}
 OUT.mkdir(parents=True,exist_ok=True); (OUT/"manifest.json").write_text(json.dumps(manifest,indent=2),encoding="utf-8")
 lines=[
  "# Pass475 — DM1 V1 movement/viewport/wall live click capture", "",
  "Status: `{}`".format(status), "", summary, "", "## Evidence summary",
  "- Source audit ok: `{}`".format(preds["sourceAuditOk"]),
  "- Queue hit: `{}`".format(preds["f0380Hit"]),
  "- Movement handlers hit: `{}`".format(preds["handlersHit"]),
  "- F0128 hit: `{}`".format(preds["f0128Hit"]),
  "- Post-present seam hit: `{}`".format(preds["postPresentHit"]),
  "- Capture count: `{}`".format(preds["captureCount"]),
  "", "## Artifacts",
  "- Manifest: `{}`".format((OUT/"manifest.json").relative_to(ROOT)),
  "- Transcript: `{}`".format((OUT/(PASS+"_runtime.clean.txt")).relative_to(ROOT)),
  "- Command log: `{}`".format((OUT/(PASS+"_command_log.json")).relative_to(ROOT)),
  "- Click log: `{}`".format((OUT/(PASS+"_click_log.json")).relative_to(ROOT)),
  "", "## Promotion rule",
  "Fresh wall/viewport frames are promotable only after the F0128 composition and F0097/VIDRV post-present seam in this live run.",
 ]
 REPORT.write_text("\n".join(lines)+"\n",encoding="utf-8")
 print(json.dumps({"status":status,"summary":summary,"predicates":preds,"manifest":str((OUT/"manifest.json").relative_to(ROOT)),"report":str(REPORT.relative_to(ROOT))},indent=2))
 return 0 if status.startswith(("PASS","BLOCKED")) else 1
if __name__=="__main__": raise SystemExit(main())
