#!/usr/bin/env python3
from __future__ import annotations
import json, os, re, shlex, shutil, subprocess, tempfile, threading, time
from datetime import datetime, timezone
from pathlib import Path
ROOT=Path('<firestaff-worktree>/firestaff-oauth-n2-dm1v1-pass293-direct-f0380-hook-address-window-20260507-0308')
OUT=ROOT/'parity-evidence/verification/pass293_dm1_v1_direct_f0380_hook_address_window'
REPORT=ROOT/'parity-evidence/pass293_dm1_v1_direct_f0380_hook_address_window.md'
ORIG=Path.home()/'.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34'
SRC=Path.home()/'.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'
ADDR={
 'F0359_COMMAND_ProcessClick_CPSC':'22F4:030D',
 'F0361_COMMAND_ProcessKeyPress':'22F4:0407',
 'F0380_COMMAND_ProcessQueue_CPSC':'22F4:0699',
 'F0365_COMMAND_ProcessTypes1To2_TurnParty':'1EA4:010D',
 'F0366_COMMAND_ProcessTypes3To6_MoveParty':'1EA4:01AA',
 'F0128_DUNGEONVIEW_Draw_CPSF':'23AD:40FE',
 'G0432_as_CommandQueue':'2C20:3E7A','G0433_i_CommandQueueFirstIndex':'2C20:3EC8','G0434_i_CommandQueueLastIndex':'2C20:1F08',
 'G0308_i_PartyDirection':'2C20:3C92','G0306_i_PartyMapX':'2C20:3C94','G0307_i_PartyMapY':'2C20:3CE0'}
ROUTE='wait:9000 enter wait:1800 one wait:1800 click:276,140 wait:1800 one wait:1800 kp5 wait:800 kp4 wait:800 kp6 wait:800 kp5 wait:800 kp4 wait:800 kp6 wait:800 kp5 wait:800 kp6 wait:800 kp4 wait:800 kp5 wait:800'
def run(cmd,**kw): return subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,**kw)
def clean(t): return re.sub(r'[\x00-\x08\x0b\x0c\x0e-\x1f]','',re.sub(r'\x1b\[[0-9;?]*[ -/]*[@-~]','',t).replace('\r','\n'))
def cap(sess,n=300): return clean(run(['tmux','capture-pane','-p','-S',f'-{n}','-t',sess],timeout=5).stdout)
def send(sess,*keys): run(['tmux','send-keys','-t',sess,*keys],timeout=5)
def send_cmd(sess,cmd,events,delay=.18): events.append({'t':time.time(),'event':'debugger_cmd','cmd':cmd}); send(sess,cmd,'Enter'); time.sleep(delay)
def write_conf(path,stage): path.write_text('\n'.join(['[sdl]','fullscreen=false','output=surface','usescancodes=false','[dosbox]','machine=svga_paradise','memsize=4','[cpu]','core=normal','cycles=3000','[mixer]','nosound=true','[autoexec]',f'mount c "{stage}"','c:','DEBUG DM.EXE -vv -sn -pk','']))
def source_audit():
 checks=[('COMMAND.C','queue globals/function',1,16,['G0432_as_CommandQueue','G0433_i_CommandQueueFirstIndex','G0434_i_CommandQueueLastIndex']),('COMMAND.C','keyboard scan/write queue',1734,1812,['F0361_COMMAND_ProcessKeyPress','G0432_as_CommandQueue','G0434_i_CommandQueueLastIndex']),('COMMAND.C','F0380 dequeue/dispatch',2045,2156,['void F0380_COMMAND_ProcessQueue_CPSC','G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command','F0365_COMMAND_ProcessTypes1To2_TurnParty','F0366_COMMAND_ProcessTypes3To6_MoveParty']),('GAMELOOP.C','main loop F0361 then F0380',160,216,['F0361_COMMAND_ProcessKeyPress','F0380_COMMAND_ProcessQueue_CPSC();']),('MOVESENS.C','move tuple writes',316,556,['F0267_MOVE_GetMoveResult_CPSCE','G0306_i_PartyMapX','G0307_i_PartyMapY']),('DUNVIEW.C','F0128 consumes args/draws viewport',8318,8611,['void F0128_DUNGEONVIEW_Draw_CPSF','F0097_DUNGEONVIEW_DrawViewport'])]
 out=[]
 for fn,id,a,b,needles in checks:
  p=SRC/fn; lines=p.read_text(encoding='latin-1',errors='replace').splitlines(); block=' '.join('\n'.join(lines[a-1:b]).split()); missing=[n for n in needles if ' '.join(n.split()) not in block]
  out.append({'id':id,'file':fn,'line_range':[a,b],'ok':not missing,'missing':missing})
 return out
def prior_inspection():
 items={}
 for p in ['pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof','pass284_dm1_v1_f0380_dequeue_ordering_proof','pass289_dm1_v1_f0380_dispatch_equivalent_proof']:
  mf=ROOT/'parity-evidence/verification'/p/'manifest.json'; data=json.loads(mf.read_text())
  items[p]={'status':data.get('status'),'path':str(mf.relative_to(ROOT)),'f0380_seen':data.get('proof_predicates',{}).get('f0380_dequeue_hit_seen',data.get('direct_f0380_body_bp_seen',data.get('f0380_runtime_seen'))),'blocker':data.get('blocker') or data.get('exact_remaining_blocker') or data.get('exact_smaller_blocker')}
 return items
def xdo(display,args): return run(['xdotool',*args],env={**os.environ,'DISPLAY':display},timeout=10)
def find_window_pid(display):
 p=run(['bash','-lc',f'export DISPLAY={display}; xdotool search --sync --class dosbox | head -n 1 | xargs -r xdotool getwindowpid'],timeout=10); s=p.stdout.strip(); return int(s.splitlines()[0]) if s else None
def key_name(tok): return {'enter':'Return','return':'Return','one':'1','1':'1','kp4':'KP_Left','kp5':'KP_Begin','kp6':'KP_Right'}[tok]
def click_at(display,window,x,y):
 ns={}; exec(xdo(display,['getwindowgeometry','--shell',str(window)]).stdout,{},ns); gw,gh=float(ns['WIDTH']),float(ns['HEIGHT']); cw=gw; ch=cw/(320/200)
 if ch>gh: ch=gh; cw=ch*(320/200)
 px=int(round((gw-cw)/2+((x+.5)/320)*cw)); py=int(round((gh-ch)/2+((y+.5)/200)*ch)); xdo(display,['mousemove','--window',str(window),str(px),str(py),'click','1']); return [px,py]
def key_loop(display,pid,route,stop_evt,run_evt,log):
 win=xdo(display,['search','--sync','--pid',str(pid)]).stdout.strip().splitlines()[0]; xdo(display,['windowactivate','--sync',win]); xdo(display,['windowfocus','--sync',win])
 for item in route.split():
  low=item.lower(); log.append({'t':time.time(),'event':'route_step','route_item':item})
  if low.startswith('wait:'):
   end=time.time()+int(low.split(':',1)[1])/1000
   while time.time()<end and not stop_evt.is_set(): time.sleep(.05)
   continue
  deadline=time.time()+35
  while not run_evt.is_set() and time.time()<deadline and not stop_evt.is_set(): time.sleep(.05)
  if not run_evt.is_set(): log.append({'t':time.time(),'event':'guard_timeout','route_item':item}); continue
  if low.startswith('click:'):
   x,y=map(int,low.split(':',1)[1].split(',')); log.append({'t':time.time(),'event':'click','route_item':item,'screen':click_at(display,win,x,y)})
  else:
   xdo(display,['key','--window',win,key_name(low)]); log.append({'t':time.time(),'event':'key','route_item':item})
  time.sleep(.18)
 log.append({'t':time.time(),'event':'route_done'})
def sig_from(txt):
 lines=txt.splitlines()[-120:]
 for line in reversed(lines):
  if 'memory breakpoint' in line.lower(): return line.strip()
  if re.search(r'^[0-9A-F]{4}:[0-9A-F]{4}\s+',line.strip()): return line.strip()
 return None
def classify(sig):
 u=sig.upper()
 for k,a in ADDR.items():
  if a in u: return k
 if 'MEMORY BREAKPOINT' in u: return 'memory_breakpoint_other'
 return 'other'
def debugger(sess,stop_evt,run_evt,snaps,hits,events):
 # broad candidate window: source caller/seam, direct F0380, dispatch targets, F0128, and state/data watchpoints.
 cmds=['BPDEL *','BP '+ADDR['F0361_COMMAND_ProcessKeyPress'],'BP '+ADDR['F0380_COMMAND_ProcessQueue_CPSC'],'BP '+ADDR['F0365_COMMAND_ProcessTypes1To2_TurnParty'],'BP '+ADDR['F0366_COMMAND_ProcessTypes3To6_MoveParty'],'BP '+ADDR['F0128_DUNGEONVIEW_Draw_CPSF'],'BPM '+ADDR['G0432_as_CommandQueue'],'BPM '+ADDR['G0433_i_CommandQueueFirstIndex'],'BPM '+ADDR['G0434_i_CommandQueueLastIndex'],'BPM '+ADDR['G0308_i_PartyDirection'],'BPM '+ADDR['G0306_i_PartyMapX'],'BPM '+ADDR['G0307_i_PartyMapY'],'BPLIST']
 for c in cmds: send_cmd(sess,c,events)
 send(sess,'Escape','O','t'); run_evt.set(); seen=set()
 while not stop_evt.is_set():
  txt=cap(sess,350); sig=sig_from(txt)
  if sig and sig not in seen:
   seen.add(sig); run_evt.clear(); kind=classify(sig); hits.append({'t':time.time(),'sig':sig,'kind':kind}); snaps.append(f'\n--- STOP {len(hits)} {kind} ---\n'+txt[-18000:])
   for c in ['CPU','MEMDUMP 2C20:3E7A 96','MEMDUMP 2C20:3C88 96','BPLIST']: send_cmd(sess,c,events)
   snaps.append(f'\n--- AFTER STOP {len(hits)} ---\n'+cap(sess,420)[-18000:])
   send(sess,'Escape','O','t'); run_evt.set()
  time.sleep(.12)
def runtime_run(seconds=105):
 for x in ['dosbox-debug','tmux','Xvfb','xdotool']:
  if not shutil.which(x): raise SystemExit('missing '+x)
 OUT.mkdir(parents=True,exist_ok=True); events=[]; route=[]; hits=[]; snaps=[]; pid=None
 with tempfile.TemporaryDirectory(prefix='firestaff-pass293-') as td:
  stage=Path(td)/'dos'; shutil.copytree(ORIG,stage); conf=Path(td)/'dosbox.conf'; write_conf(conf,stage); display=':79'
  xvfb=subprocess.Popen(['Xvfb',display,'-screen','0','1024x768x24'],stdout=subprocess.DEVNULL,stderr=subprocess.STDOUT); time.sleep(.5)
  sess=f'pass293-{os.getpid()}'; stop_evt=threading.Event(); run_evt=threading.Event()
  try:
   r=run(['tmux','new-session','-d','-s',sess,f'TERM=vt100 DISPLAY={display} dosbox-debug -conf {shlex.quote(str(conf))} -exit'],timeout=5)
   if r.returncode: raise RuntimeError(r.stdout)
   time.sleep(3); threading.Thread(target=debugger,args=(sess,stop_evt,run_evt,snaps,hits,events),daemon=True).start(); time.sleep(1); pid=find_window_pid(display)
   if pid: threading.Thread(target=key_loop,args=(display,pid,ROUTE,stop_evt,run_evt,route),daemon=True).start()
   end=time.time()+seconds
   while time.time()<end: snaps.append('\n--- POLL ---\n'+cap(sess,350)[-14000:]); time.sleep(2)
   stop_evt.set(); time.sleep(.5); snaps.append('\n--- FINAL ---\n'+cap(sess,5000))
  finally:
   stop_evt.set(); run(['tmux','kill-session','-t',sess],timeout=5); xvfb.terminate(); xvfb.wait(timeout=5)
 transcript=clean('\n'.join(snaps)); (OUT/'dosbox_debug_candidate_window.clean.txt').write_text(transcript[-700000:],encoding='utf-8'); (OUT/'route_candidate_window_keylog.json').write_text(json.dumps(route,indent=2,sort_keys=True)+'\n')
 return {'dosbox_window_pid':pid,'route_keylog':'parity-evidence/verification/pass293_dm1_v1_direct_f0380_hook_address_window/route_candidate_window_keylog.json','transcript':'parity-evidence/verification/pass293_dm1_v1_direct_f0380_hook_address_window/dosbox_debug_candidate_window.clean.txt','debugger_events':events[:260],'debugger_hits_captured':hits[:220]}
def main():
 audit=source_audit(); prior=prior_inspection(); rt=runtime_run()
 hits=rt['debugger_hits_captured']; kinds=[h['kind'] for h in hits]
 direct='F0380_COMMAND_ProcessQueue_CPSC' in kinds
 dispatch=any(k in kinds for k in ['F0365_COMMAND_ProcessTypes1To2_TurnParty','F0366_COMMAND_ProcessTypes3To6_MoveParty'])
 f0361='F0361_COMMAND_ProcessKeyPress' in kinds; f0128='F0128_DUNGEONVIEW_Draw_CPSF' in kinds
 memchg=re.findall(r'memory breakpoint\s*:\s*([0-9A-F]{4}:[0-9A-F]{4})\s*-\s*([0-9A-F]{2})\s*->\s*([0-9A-F]{2})',(OUT/'dosbox_debug_candidate_window.clean.txt').read_text(errors='ignore'),re.I)
 route_items=[r.get('route_item') for r in json.loads((OUT/'route_candidate_window_keylog.json').read_text()) if r.get('event')=='key']
 preds={'controlled_keys_posted':all(k in route_items for k in ['kp5','kp4','kp6']),'direct_f0380_entry_bp_hit':direct,'f0361_keypress_bp_hit':f0361,'dispatch_target_bp_hit':dispatch,'f0128_draw_bp_hit':f0128,'queue_or_index_bpm_seen':any(a.upper() in {ADDR['G0432_as_CommandQueue'],ADDR['G0433_i_CommandQueueFirstIndex'],ADDR['G0434_i_CommandQueueLastIndex']} for a,_,__ in memchg),'party_tuple_bpm_seen':any(a.upper() in {ADDR['G0308_i_PartyDirection'],ADDR['G0306_i_PartyMapX'],ADDR['G0307_i_PartyMapY']} for a,_,__ in memchg)}
 if direct: status='DIRECT_F0380_BP_PROVEN'
 else: status='BLOCKED_DIRECT_F0380_ENTRY_BP_STILL_MISSES'
 blocker=None if direct else 'Candidate-window run set BP 22F4:0699 together with caller/source-adjacent and dispatch/draw/data probes. Direct F0380 entry BP was still not captured; next candidates are to verify the 22F4:0699 public-symbol binding against a live disassembly/MEMDUMP window and set BPs on instruction offsets inside COMMAND.C:F0380 after the prologue/dequeue reads, not just the public entry.'
 manifest={'schema':'pass293_dm1_v1_direct_f0380_hook_address_window.v1','timestamp_utc':datetime.now(timezone.utc).isoformat(timespec='seconds'),'status':status,'source_root':str(SRC),'addresses':ADDR,'source_audit':audit,'prior_inspection':prior,'runtime':rt,'proof_predicates':preds,'memory_breakpoint_changes':memchg[:180],'direct_f0380_claim':bool(direct),'exact_blocker':blocker,'next_address_candidates':['22F4:0699 entry/public symbol (reconfirm with live disassembly bytes)','22F4:06A0..22F4:06F0 inside F0380 prologue/dequeue window','22F4:0407 F0361 caller-side source seam','1EA4:010D F0365 turn dispatch','1EA4:01AA F0366 move dispatch'],'reproduction_command':'python3 tools/pass293_dm1_v1_direct_f0380_hook_address_window.py'}
 (OUT/'manifest.json').write_text(json.dumps(manifest,indent=2,sort_keys=True)+'\n')
 REPORT.write_text(f"""# Pass293 — DM1 V1 direct F0380 hook address/window probe\n\nStatus: `{status}`\n\n## Verdict\n\n- Direct F0380 entry BP `22F4:0699`: `{'seen' if direct else 'not seen'}`.\n- Controlled keys posted: `{preds['controlled_keys_posted']}`.\n- Queue/index watchpoint seen: `{preds['queue_or_index_bpm_seen']}`.\n- Party tuple watchpoint seen: `{preds['party_tuple_bpm_seen']}`.\n- F0128 draw BP seen: `{preds['f0128_draw_bp_hit']}`.\n\n## Source audit\n\n- `GAMELOOP.C:160-216` calls `F0361_COMMAND_ProcessKeyPress` before `F0380_COMMAND_ProcessQueue_CPSC()`.\n- `COMMAND.C:2045-2156` is the F0380 queue/dequeue/dispatch body.\n- `MOVESENS.C:316-556` covers movement tuple writes.\n- `DUNVIEW.C:8318-8611` covers F0128 draw argument consumption and viewport draw request.\n\n## Decision\n\n{blocker or 'Direct F0380 entry breakpoint proof captured; see manifest/transcript.'}\n\n## Artifacts\n\n- Manifest: `parity-evidence/verification/pass293_dm1_v1_direct_f0380_hook_address_window/manifest.json`\n- Transcript: `{rt['transcript']}`\n- Route log: `{rt['route_keylog']}`\n""",encoding='utf-8')
 print(json.dumps({'status':status,'predicates':preds,'manifest':str(OUT/'manifest.json')},indent=2))
if __name__=='__main__': main()
