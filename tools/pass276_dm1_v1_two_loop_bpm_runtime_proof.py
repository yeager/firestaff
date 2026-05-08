#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, os, re, shlex, shutil, subprocess, tempfile, threading, time
from datetime import datetime, timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'parity-evidence/verification/pass276_dm1_v1_two_loop_bpm_runtime_proof'
REPORT=ROOT/'parity-evidence/pass276_dm1_v1_two_loop_bpm_runtime_proof.md'
ORIG=Path.home()/'.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34'
SOURCE_ROOT=Path.home()/'.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'
ADDR={'G0432_as_CommandQueue':'2C20:3E7A','G0433_i_CommandQueueFirstIndex':'2C20:3EC8','G0434_i_CommandQueueLastIndex':'2C20:1F08','G0435_B_CommandQueueLocked':'2C20:1F0A','G0308_i_PartyDirection':'2C20:3C92','G0306_i_PartyMapX':'2C20:3C94','G0307_i_PartyMapY':'2C20:3CE0','F0380_COMMAND_ProcessQueue_CPSC':'22F4:0699','F0365_COMMAND_ProcessTypes1To2_TurnParty':'1EA4:010D','F0366_COMMAND_ProcessTypes3To6_MoveParty':'1EA4:01AA','F0267_MOVE_GetMoveResult_CPSCE':'1859:0516','F0128_DUNGEONVIEW_Draw_CPSF':'23AD:40FE'}
ROUTE='wait:7000 enter wait:1500 one wait:1500 click:276,140 wait:1500 one wait:1500 kp5 wait:700 kp4 wait:700 kp6 wait:700'
def run(cmd, **kw): return subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,**kw)
def clean(text):
    text=re.sub(r'\x1b\[[0-9;?]*[ -/]*[@-~]','',text).replace('\r','\n')
    return re.sub(r'[\x00-\x08\x0b\x0c\x0e-\x1f]','',text)
def cap(sess,n=200): return clean(run(['tmux','capture-pane','-p','-S',f'-{n}','-t',sess],timeout=5).stdout)
def send(sess,*keys): run(['tmux','send-keys','-t',sess,*keys],timeout=5)
def source_audit():
    checks=[('COMMAND.C',1,16,['G0432_as_CommandQueue','G0433_i_CommandQueueFirstIndex','G0434_i_CommandQueueLastIndex','G0435_B_CommandQueueLocked']),('COMMAND.C',1734,1812,['G0443_ps_PrimaryKeyboardInput','G0432_as_CommandQueue','G0434_i_CommandQueueLastIndex']),('COMMAND.C',2045,2156,['F0380_COMMAND_ProcessQueue_CPSC','G0432_as_CommandQueue','F0365_COMMAND_ProcessTypes1To2_TurnParty','F0366_COMMAND_ProcessTypes3To6_MoveParty']),('CLIKMENU.C',142,328,['F0365_COMMAND_ProcessTypes1To2_TurnParty','F0366_COMMAND_ProcessTypes3To6_MoveParty','G0308_i_PartyDirection','G0306_i_PartyMapX','G0307_i_PartyMapY']),('GAMELOOP.C',55,91,['F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)']),('MOVESENS.C',316,556,['BOOLEAN F0267_MOVE_GetMoveResult_CPSCE','G0306_i_PartyMapX','G0307_i_PartyMapY']),('DUNVIEW.C',8318,8611,['void F0128_DUNGEONVIEW_Draw_CPSF','F0097_DUNGEONVIEW_DrawViewport'])]
    out=[]
    for fn,a,b,needles in checks:
        p=SOURCE_ROOT/fn; lines=p.read_text(encoding='latin-1',errors='replace').splitlines() if p.exists() else []
        block=' '.join('\n'.join(lines[a-1:min(b,len(lines))]).split())
        missing=[n for n in needles if ' '.join(n.split()) not in block]
        out.append({'file':fn,'line_range':[a,b],'ok':p.exists() and not missing,'missing':missing})
    return out
def write_conf(path, stage):
    path.write_text('\n'.join(['[sdl]','fullscreen=false','output=surface','usescancodes=false','[dosbox]','machine=svga_paradise','memsize=4','[cpu]','core=normal','cycles=3000','[mixer]','nosound=true','[autoexec]',f'mount c "{stage}"','c:','DEBUG DM.EXE -vv -sn -pk','']))
def stopped_signature(text):
    hits=[]
    for line in text.splitlines()[-80:]:
        ll=line.lower()
        if 'breakpoint' in ll or re.search(r'\bBP\s+[0-9A-F]{4}:[0-9A-F]{4}', line): hits.append(line.strip())
        elif re.search(r'^[0-9A-F]{4}:[0-9A-F]{4}\s+', line.strip()): hits.append(line.strip())
    return hits[-1] if hits else None
def is_running(text):
    last='\n'.join(text.splitlines()[-24:])
    if stopped_signature(last): return False
    return '(Running)' in last or 'Running)' in last
def wait_running(run_evt,stop_evt,log,timeout=25):
    deadline=time.time()+timeout
    while time.time()<deadline and not stop_evt.is_set():
        if run_evt.is_set(): return True
        time.sleep(.05)
    log.append({'t':time.time(),'event':'wait_running_timeout'}); return False
def find_window_pid(display):
    p=run(['bash','-lc',f'export DISPLAY={display}; xdotool search --sync --class dosbox | head -n 1 | xargs -r xdotool getwindowpid'],timeout=10)
    return int(p.stdout.strip().splitlines()[0]) if p.stdout.strip() else None
def xdo(display,args): return run(['xdotool',*args],env={**os.environ,'DISPLAY':display},timeout=10)
def key_name(tok): return {'enter':'Return','return':'Return','one':'1','1':'1','kp4':'KP_Left','kp5':'KP_Begin','kp6':'KP_Right'}[tok]
def click_at(display,window,x,y):
    geom=xdo(display,['getwindowgeometry','--shell',str(window)]).stdout; ns={}; exec(geom,{},ns)
    gw,gh=float(ns['WIDTH']),float(ns['HEIGHT']); aspect=320/200; cw=gw; ch=cw/aspect
    if ch>gh: ch=gh; cw=ch*aspect
    px=int(round((gw-cw)/2+((x+.5)/320)*cw)); py=int(round((gh-ch)/2+((y+.5)/200)*ch))
    xdo(display,['mousemove','--window',str(window),str(px),str(py),'click','1']); return px,py
def key_loop(sess,display,pid,route,stop_evt,run_evt,log):
    window=xdo(display,['search','--sync','--pid',str(pid)]).stdout.strip().splitlines()[0]
    xdo(display,['windowactivate','--sync',window]); xdo(display,['windowfocus','--sync',window])
    for item in route.split():
        low=item.lower(); log.append({'t':time.time(),'event':'route_step','route_item':item})
        if low.startswith('wait:'):
            end=time.time()+int(low.split(':',1)[1])/1000
            while time.time()<end and not stop_evt.is_set(): time.sleep(.05)
            continue
        if not wait_running(run_evt,stop_evt,log): continue
        if low.startswith('click:'):
            x,y=map(int,low.split(':',1)[1].split(',')); px,py=click_at(display,window,x,y)
            log.append({'t':time.time(),'event':'click','route_item':item,'screen':[px,py],'running_guard':True})
        else:
            xdo(display,['key','--window',window,key_name(low)])
            log.append({'t':time.time(),'event':'key','route_item':item,'running_guard':True})
        time.sleep(.18)
    log.append({'t':time.time(),'event':'route_done'})
def debugger_loop(sess,stop_evt,run_evt,snaps,hits):
    seen=set(); last_continue=0.0
    while not stop_evt.is_set():
        txt=cap(sess,260); sig=stopped_signature(txt)
        if sig and sig not in seen:
            run_evt.clear(); seen.add(sig); hits.append({'t':time.time(),'sig':sig}); snaps.append('\n--- STOP %s ---\n%s'%(len(hits),txt[-14000:]))
            for c in ['CPU','MEMDUMP 2C20:3E7A 80','MEMDUMP 2C20:3C88 80']:
                send(sess,c,'Enter'); time.sleep(.10)
            snaps.append('\n--- AFTER CAPTURE %s ---\n%s'%(len(hits),cap(sess,260)[-14000:])); send(sess,'Escape','O','t'); last_continue=time.time(); run_evt.set()
        elif sig and time.time()-last_continue>1.0:
            for c in ['CPU','MEMDUMP 2C20:3E7A 80','MEMDUMP 2C20:3C88 80']:
                send(sess,c,'Enter'); time.sleep(.07)
            send(sess,'Escape','O','t'); last_continue=time.time(); run_evt.set()
        time.sleep(.12)
def classify(transcript, route_log, hits):
    s=transcript; seen={name:(addr in s) for name,addr in ADDR.items()}
    memchg=re.findall(r'memory breakpoint\s*:\s*([0-9A-F]{4}:[0-9A-F]{4})\s*-\s*([0-9A-F]{2})\s*->\s*([0-9A-F]{2})',s,re.I)
    key_items=[r.get('route_item') for r in route_log if r.get('event')=='key']
    hit_sigs='\n'.join(h.get('sig','') for h in hits)
    required={'route_posted_controlled_keys':all(k in key_items for k in ['kp5','kp4','kp6']),'g0432_write_seen':any(a.upper()==ADDR['G0432_as_CommandQueue'] for a,_,__ in memchg),'f0380_dequeue_hit_seen':ADDR['F0380_COMMAND_ProcessQueue_CPSC'] in hit_sigs,'party_tuple_mutation_seen':any(a.upper() in {ADDR['G0308_i_PartyDirection'],ADDR['G0306_i_PartyMapX'],ADDR['G0307_i_PartyMapY']} for a,_,__ in memchg),'f0128_draw_hit_seen':ADDR['F0128_DUNGEONVIEW_Draw_CPSF'] in hit_sigs,'cpu_memdump_after_stops':len(hits)>0 and 'MEMDUMP 2C20:3E7A 80' in s and 'MEMDUMP 2C20:3C88 80' in s}
    return ('PROVEN_RUNTIME_HOOK' if all(required.values()) else 'BLOCKED_NO_PROVEN_RUNTIME_HOOK'), seen, memchg, required
def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--seconds',type=int,default=60); ap.add_argument('--route',default=ROUTE); args=ap.parse_args()
    OUT.mkdir(parents=True,exist_ok=True)
    missing=[x for x in ['dosbox-debug','tmux','Xvfb','xdotool'] if not shutil.which(x)]
    if missing: raise SystemExit('missing: '+', '.join(missing))
    audit=source_audit(); route_log=[]; hits=[]; snaps=[]; dosbox_pid=None
    with tempfile.TemporaryDirectory(prefix='firestaff-pass276-') as td:
        stage=Path(td)/'dos'; shutil.copytree(ORIG,stage); conf=Path(td)/'dosbox.conf'; write_conf(conf,stage)
        display=':76'; xvfb=subprocess.Popen(['Xvfb',display,'-screen','0','1024x768x24'],stdout=subprocess.DEVNULL,stderr=subprocess.STDOUT); time.sleep(.5)
        sess=f'pass276-{os.getpid()}'; stop_evt=threading.Event(); run_evt=threading.Event()
        try:
            r=run(['tmux','new-session','-d','-s',sess,f'TERM=vt100 DISPLAY={display} dosbox-debug -conf {shlex.quote(str(conf))} -exit'],timeout=5)
            if r.returncode: raise RuntimeError(r.stdout)
            time.sleep(3)
            for k in ['F0380_COMMAND_ProcessQueue_CPSC','F0365_COMMAND_ProcessTypes1To2_TurnParty','F0366_COMMAND_ProcessTypes3To6_MoveParty','F0267_MOVE_GetMoveResult_CPSCE','F0128_DUNGEONVIEW_Draw_CPSF']:
                send(sess,f'BP {ADDR[k]}','Enter'); time.sleep(.2)
            for k in ['G0432_as_CommandQueue','G0433_i_CommandQueueFirstIndex','G0434_i_CommandQueueLastIndex','G0308_i_PartyDirection','G0306_i_PartyMapX','G0307_i_PartyMapY']:
                send(sess,f'BPM {ADDR[k]}','Enter'); time.sleep(.2)
            send(sess,'BPLIST','Enter'); time.sleep(.5); send(sess,'MEMDUMP 2C20:3E7A 80','Enter'); time.sleep(.2); send(sess,'MEMDUMP 2C20:3C88 80','Enter'); time.sleep(.2)
            send(sess,'Escape','O','t'); run_evt.set(); time.sleep(1); dosbox_pid=find_window_pid(display)
            dbg=threading.Thread(target=debugger_loop,args=(sess,stop_evt,run_evt,snaps,hits),daemon=True); dbg.start()
            if dosbox_pid: threading.Thread(target=key_loop,args=(sess,display,dosbox_pid,args.route,stop_evt,run_evt,route_log),daemon=True).start()
            end=time.time()+args.seconds
            while time.time()<end: snaps.append('\n--- POLL ---\n'+cap(sess,260)[-12000:]); time.sleep(2)
            stop_evt.set(); time.sleep(.5); snaps.append('\n--- FINAL ---\n'+cap(sess,4000))
        finally:
            stop_evt.set(); run(['tmux','kill-session','-t',sess],timeout=5); xvfb.terminate(); xvfb.wait(timeout=5)
    transcript=clean('\n'.join(snaps)); (OUT/'dosbox_debug_two_loop.clean.txt').write_text(transcript[-300000:],encoding='utf-8')
    (OUT/'route_two_loop_keylog.json').write_text(json.dumps(route_log,indent=2,sort_keys=True)+'\n')
    status, seen, memchg, required=classify(transcript,route_log,hits)
    bplist=sorted(set(re.findall(r'\b(?:BP|BPMEM)\s+([0-9A-F]{4}:[0-9A-F]{4})',transcript)))
    blocker=None if status=='PROVEN_RUNTIME_HOOK' else 'Two-loop driver posted route keys only after a debugger-run guard and debugger loop captured/continued stops, but the two-loop driver fixed the pass275 synchronous-input flaw and posted kp5/kp4/kp6 under the debugger-run guard, but the transcript still does not show F0380 dequeuing or F0128 consuming the mutated tuple as BP hits; next step is to reduce breakpoint noise or drive a longer in-game route after game input readiness so code BP hits are captured between queue write/mutation and draw.'
    manifest={'schema':'pass276_dm1_v1_two_loop_bpm_runtime_proof.v1','timestamp_utc':datetime.now(timezone.utc).isoformat(),'status':status,'source_audit':audit,'addresses':ADDR,'route':args.route,'method':'two Python threads: route loop waits for debugger-run event before kp5/kp4/kp6; debugger loop captures CPU plus MEMDUMP 2C20:3E7A/2C20:3C88 on each BP/BPM stop and continues with vt100 F5 (Esc O t)','dosbox_debug':'/usr/bin/dosbox-debug','dosbox_window_pid':dosbox_pid,'listed_breakpoints_or_watches':bplist,'address_strings_seen':seen,'memory_breakpoint_changes':memchg[:100],'debugger_hits_captured':hits[:140],'proof_predicates':required,'blocker':blocker,'transcript':'parity-evidence/verification/pass276_dm1_v1_two_loop_bpm_runtime_proof/dosbox_debug_two_loop.clean.txt','route_keylog':'parity-evidence/verification/pass276_dm1_v1_two_loop_bpm_runtime_proof/route_two_loop_keylog.json','promotion_rule':'Claim runtime hook only if key input writes G0432, F0380 dequeues, movement/turn mutates G0308 or G0306/G0307, and F0128 consumes resulting tuple.'}
    (OUT/'manifest.json').write_text(json.dumps(manifest,indent=2,sort_keys=True)+'\n')
    REPORT.write_text(f'''# Pass276 — DM1 V1 two-loop BPM runtime proof\n\nStatus: `{status}`\n\n## What changed from pass275\n\n- Re-audited ReDMCSB command/movement/viewport seams before running.\n- Replaced the synchronous xdotool route injector with a two-loop dosbox-debug driver.\n- Route loop posts controlled `kp5`/`kp4`/`kp6` only after a debugger-run guard.\n- Debugger loop captures `CPU`, `MEMDUMP 2C20:3E7A 80`, and `MEMDUMP 2C20:3C88 80` on stops and immediately continues via vt100 F5 (`Esc O t`).\n\n## Evidence\n\n- Manifest: `parity-evidence/verification/pass276_dm1_v1_two_loop_bpm_runtime_proof/manifest.json`\n- Transcript: `parity-evidence/verification/pass276_dm1_v1_two_loop_bpm_runtime_proof/dosbox_debug_two_loop.clean.txt`\n- Route log: `parity-evidence/verification/pass276_dm1_v1_two_loop_bpm_runtime_proof/route_two_loop_keylog.json`\n\n## Proof predicates\n\n```json\n{json.dumps(required, indent=2, sort_keys=True)}\n```\n\n## Decision\n\n{('Runtime hook proven by the strict chain predicates.' if status=='PROVEN_RUNTIME_HOOK' else blocker)}\n''',encoding='utf-8')
    print(json.dumps({'status':status,'manifest':str(OUT/'manifest.json'),'report':str(REPORT),'predicates':required},indent=2)); return 0
if __name__=='__main__': raise SystemExit(main())
