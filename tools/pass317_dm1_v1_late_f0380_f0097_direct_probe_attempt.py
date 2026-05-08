#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, os, re, shlex, shutil, subprocess, tempfile, threading, time
from datetime import datetime, timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'parity-evidence/verification/pass317_dm1_v1_f0380_f0097_direct_probe_attempt'
REPORT=ROOT/'parity-evidence/pass317_dm1_v1_f0380_f0097_direct_probe_attempt.md'
ORIG=Path.home()/'.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34'
SOURCE_ROOT=Path.home()/'.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'
ADDR={'G0432_as_CommandQueue':'2C20:3E7A','G0308_i_PartyDirection':'2C20:3C92','G0306_i_PartyMapX':'2C20:3C94','G0307_i_PartyMapY':'2C20:3CE0','F0380_COMMAND_ProcessQueue_CPSC':'22F4:0699','F0128_DUNGEONVIEW_Draw_CPSF':'23AD:40FE','F0097_DUNGEONVIEW_DrawViewport':'2809:1E31'}
DEFAULT_ROUTE='wait:9000 enter wait:1800 one wait:1800 click:276,140 wait:1800 one wait:1800 kp5 wait:900 kp4 wait:900 kp6 wait:900 kp5 wait:900 kp4 wait:900 kp6 wait:900 kp5 wait:900 kp6 wait:900 kp4 wait:900 kp5 wait:900'
def run(cmd, **kw): return subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,**kw)
def clean(text):
    text=re.sub(r'\x1b\[[0-9;?]*[ -/]*[@-~]','',text).replace('\r','\n')
    return re.sub(r'[\x00-\x08\x0b\x0c\x0e-\x1f]','',text)
def cap(sess,n=240): return clean(run(['tmux','capture-pane','-p','-S',f'-{n}','-t',sess],timeout=5).stdout)
def send(sess,*keys): run(['tmux','send-keys','-t',sess,*keys],timeout=5)
def send_cmd(sess,cmd,events):
    events.append({'t':time.time(),'event':'debugger_cmd','cmd':cmd}); send(sess,cmd,'Enter'); time.sleep(.16)
def source_audit():
    checks=[('COMMAND.C',1,16,['G0432_as_CommandQueue']),('COMMAND.C',1734,1812,['G0443_ps_PrimaryKeyboardInput','G0432_as_CommandQueue']),('COMMAND.C',2045,2156,['F0380_COMMAND_ProcessQueue_CPSC','G0432_as_CommandQueue']),('MOVESENS.C',316,556,['BOOLEAN F0267_MOVE_GetMoveResult_CPSCE','G0306_i_PartyMapX','G0307_i_PartyMapY']),('DUNVIEW.C',8318,8611,['void F0128_DUNGEONVIEW_Draw_CPSF','F0097_DUNGEONVIEW_DrawViewport'])]
    out=[]
    for fn,a,b,needles in checks:
        p=SOURCE_ROOT/fn; lines=p.read_text(encoding='latin-1',errors='replace').splitlines() if p.exists() else []
        block=' '.join('\n'.join(lines[a-1:min(b,len(lines))]).split())
        missing=[n for n in needles if ' '.join(n.split()) not in block]
        out.append({'file':fn,'line_range':[a,b],'ok':p.exists() and not missing,'missing':missing})
    return out
def write_conf(path,stage):
    path.write_text('\n'.join(['[sdl]','fullscreen=false','output=surface','usescancodes=false','[dosbox]','machine=svga_paradise','memsize=4','[cpu]','core=normal','cycles=3000','[mixer]','nosound=true','[autoexec]',f'mount c "{stage}"','c:','DEBUG DM.EXE -vv -sn -pk','']))
def stopped_signature(text):
    hits=[]
    for line in text.splitlines()[-90:]:
        ll=line.lower(); stripped=line.strip()
        if 'breakpoint' in ll or re.search(r'\bBP\s+[0-9A-F]{4}:[0-9A-F]{4}', line): hits.append(stripped)
        elif re.search(r'^[0-9A-F]{4}:[0-9A-F]{4}\s+', stripped): hits.append(stripped)
    return hits[-1] if hits else None
def classify_sig(sig):
    u=sig.upper()
    if ADDR['G0432_as_CommandQueue'] in u and 'MEMORY BREAKPOINT' in u: return 'g0432_bpm'
    if any(ADDR[k] in u for k in ['G0308_i_PartyDirection','G0306_i_PartyMapX','G0307_i_PartyMapY']) and 'MEMORY BREAKPOINT' in u: return 'party_tuple_bpm'
    if ADDR['F0380_COMMAND_ProcessQueue_CPSC'] in u: return 'f0380_bp'
    if ADDR['F0128_DUNGEONVIEW_Draw_CPSF'] in u: return 'f0128_bp'
    if ADDR['F0097_DUNGEONVIEW_DrawViewport'] in u: return 'f0097_bp'
    return 'other'
def wait_running(run_evt,stop_evt,log,timeout=35):
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
def key_loop(display,pid,route,stop_evt,run_evt,log):
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
            xdo(display,['key','--window',window,key_name(low)]); log.append({'t':time.time(),'event':'key','route_item':item,'running_guard':True})
        time.sleep(.20)
    log.append({'t':time.time(),'event':'route_done'})
def set_stage(sess,stage,events):
    send_cmd(sess,'BPDEL *',events)
    if stage=='queue_watch':
        cmds=[f"BP {ADDR['F0380_COMMAND_ProcessQueue_CPSC']}", f"BPM {ADDR['G0432_as_CommandQueue']}", f"BPM {ADDR['G0308_i_PartyDirection']}", f"BPM {ADDR['G0306_i_PartyMapX']}", f"BPM {ADDR['G0307_i_PartyMapY']}"]
    elif stage=='queue_to_tuple':
        cmds=[f"BP {ADDR['F0380_COMMAND_ProcessQueue_CPSC']}", f"BP {ADDR['F0128_DUNGEONVIEW_Draw_CPSF']}", f"BPM {ADDR['G0308_i_PartyDirection']}", f"BPM {ADDR['G0306_i_PartyMapX']}", f"BPM {ADDR['G0307_i_PartyMapY']}"]
    elif stage=='code_only':
        cmds=[f"BP {ADDR['F0097_DUNGEONVIEW_DrawViewport']}", f"BP {ADDR['F0128_DUNGEONVIEW_Draw_CPSF']}", f"BP {ADDR['F0380_COMMAND_ProcessQueue_CPSC']}"]
    else:
        cmds=[]
    for c in cmds: send_cmd(sess,c,events)
    send_cmd(sess,'BPLIST',events)
def debugger_loop(sess,stop_evt,run_evt,snaps,hits,events):
    seen=set(); stage='code_only'; states={'g0432_seen':True,'tuple_seen':True}; set_stage(sess,stage,events); send(sess,'Escape','O','t'); run_evt.set()
    last_continue=0.0
    while not stop_evt.is_set():
        txt=cap(sess,300); sig=stopped_signature(txt)
        if sig and sig not in seen:
            run_evt.clear(); seen.add(sig); kind=classify_sig(sig); hit={'t':time.time(),'sig':sig,'kind':kind,'stage':stage}; hits.append(hit); snaps.append(f"\n--- STOP {len(hits)} {kind} stage={stage} ---\n"+txt[-16000:])
            for c in ['CPU','MEMDUMP 2C20:3E7A 80','MEMDUMP 2C20:3C88 80']:
                send_cmd(sess,c,events)
            post=cap(sess,300); snaps.append(f"\n--- AFTER CAPTURE {len(hits)} ---\n"+post[-16000:])
            # pass317: keep direct code breakpoints armed; F0380 is armed last to avoid setup-time collision
            send(sess,'Escape','O','t'); last_continue=time.time(); run_evt.set()
        elif sig and time.time()-last_continue>1.2:
            for c in ['CPU','MEMDUMP 2C20:3E7A 80','MEMDUMP 2C20:3C88 80']: send_cmd(sess,c,events)
            send(sess,'Escape','O','t'); last_continue=time.time(); run_evt.set()
        time.sleep(.12)
def classify(transcript,route_log,hits):
    s=transcript; memchg=re.findall(r'memory breakpoint\s*:\s*([0-9A-F]{4}:[0-9A-F]{4})\s*-\s*([0-9A-F]{2})\s*->\s*([0-9A-F]{2})',s,re.I)
    key_items=[r.get('route_item') for r in route_log if r.get('event')=='key']
    required={'route_posted_controlled_keys':all(k in key_items for k in ['kp5','kp4','kp6']),'g0432_write_seen':any(a.upper()==ADDR['G0432_as_CommandQueue'] for a,_,__ in memchg),'f0380_dequeue_hit_seen':any(h.get('kind')=='f0380_bp' for h in hits),'party_tuple_mutation_seen':any(a.upper() in {ADDR['G0308_i_PartyDirection'],ADDR['G0306_i_PartyMapX'],ADDR['G0307_i_PartyMapY']} for a,_,__ in memchg),'f0128_draw_hit_seen':any(h.get('kind')=='f0128_bp' for h in hits),'cpu_memdump_after_stops':len(hits)>0 and 'MEMDUMP 2C20:3E7A 80' in s and 'MEMDUMP 2C20:3C88 80' in s}
    return ('PROVEN_RUNTIME_HOOK' if all(required.values()) else 'BLOCKED_NO_PROVEN_RUNTIME_HOOK'), memchg, required
def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--seconds',type=int,default=95); ap.add_argument('--route',default=DEFAULT_ROUTE); args=ap.parse_args()
    OUT.mkdir(parents=True,exist_ok=True); missing=[x for x in ['dosbox-debug','tmux','Xvfb','xdotool'] if not shutil.which(x)]
    if missing: raise SystemExit('missing: '+', '.join(missing))
    audit=source_audit(); route_log=[]; hits=[]; snaps=[]; events=[]; dosbox_pid=None
    with tempfile.TemporaryDirectory(prefix='firestaff-pass317-') as td:
        stage_dir=Path(td)/'dos'; shutil.copytree(ORIG,stage_dir); conf=Path(td)/'dosbox.conf'; write_conf(conf,stage_dir)
        display=':78'; xvfb=subprocess.Popen(['Xvfb',display,'-screen','0','1024x768x24'],stdout=subprocess.DEVNULL,stderr=subprocess.STDOUT); time.sleep(.5)
        sess=f'pass317-{os.getpid()}'; stop_evt=threading.Event(); run_evt=threading.Event()
        try:
            r=run(['tmux','new-session','-d','-s',sess,f'TERM=vt100 DISPLAY={display} dosbox-debug -conf {shlex.quote(str(conf))} -exit'],timeout=5)
            if r.returncode: raise RuntimeError(r.stdout)
            time.sleep(3)
            dbg=threading.Thread(target=debugger_loop,args=(sess,stop_evt,run_evt,snaps,hits,events),daemon=True); dbg.start()
            time.sleep(1); dosbox_pid=find_window_pid(display)
            if dosbox_pid: threading.Thread(target=key_loop,args=(display,dosbox_pid,args.route,stop_evt,run_evt,route_log),daemon=True).start()
            end=time.time()+args.seconds
            while time.time()<end: snaps.append('\n--- POLL ---\n'+cap(sess,300)[-12000:]); time.sleep(2)
            stop_evt.set(); time.sleep(.5); snaps.append('\n--- FINAL ---\n'+cap(sess,5000))
        finally:
            stop_evt.set(); run(['tmux','kill-session','-t',sess],timeout=5); xvfb.terminate(); xvfb.wait(timeout=5)
    transcript=clean('\n'.join(snaps)); (OUT/'dosbox_debug_noise_reduced.clean.txt').write_text(transcript[-500000:],encoding='utf-8')
    (OUT/'route_noise_reduced_keylog.json').write_text(json.dumps(route_log,indent=2,sort_keys=True)+'\n')
    status,memchg,required=classify(transcript,route_log,hits)
    blocker=None if status=='PROVEN_RUNTIME_HOOK' else 'Late-F0380 probe attempted to avoid pass316 setup collision by arming F0380 last after F0097/F0128. Promote only if explicit F0380 or F0097 stop appears; otherwise this remains a blocked attempt, not a runtime-hook promotion.'
    manifest={'schema':'pass317_dm1_v1_f0380_f0097_direct_probe_attempt.raw_probe.v1','timestamp_utc':datetime.now(timezone.utc).isoformat(),'status':status,'source_audit':audit,'addresses':ADDR,'route':args.route,'method':'pass317 late-F0380 debugger window after pass316: arm F0097 2809:1E31 and F0128 23AD:40FE first, then F0380 22F4:0699 last, then drive the pass278 guarded xdotool route','dosbox_debug':'/usr/bin/dosbox-debug','dosbox_window_pid':dosbox_pid,'debugger_events':events[:220],'memory_breakpoint_changes':memchg[:140],'debugger_hits_captured':hits[:180],'proof_predicates':required,'blocker':blocker,'transcript':'parity-evidence/verification/pass317_dm1_v1_f0380_f0097_direct_probe_attempt/dosbox_debug_noise_reduced.clean.txt','route_keylog':'parity-evidence/verification/pass317_dm1_v1_f0380_f0097_direct_probe_attempt/route_noise_reduced_keylog.json','promotion_rule':'Promote only an explicit debugger stop for F0380 at 22F4:0699 or F0097 at 2809:1E31; do not reinterpret setup echo/collision text as a direct runtime hit.'}
    (OUT/'manifest.json').write_text(json.dumps(manifest,indent=2,sort_keys=True)+'\n')
    REPORT.write_text(f'''# Pass317 — DM1 V1 F0380/F0097 direct runtime probe attempt

Status: `{status}`

## What this pass tried

- Started with a ReDMCSB source audit before runtime probing.
- Armed code breakpoints in late-F0380 order: F0097 (`2809:1E31`), F0128 (`23AD:40FE`), then F0380 (`22F4:0699`).
- Drove the existing pass278 guarded DOSBox-debug/tmux/Xvfb/xdotool route.

## Evidence

- Manifest: `parity-evidence/verification/pass317_dm1_v1_f0380_f0097_direct_probe_attempt/manifest.json`
- Transcript: `parity-evidence/verification/pass317_dm1_v1_f0380_f0097_direct_probe_attempt/dosbox_debug_noise_reduced.clean.txt`
- Route log: `parity-evidence/verification/pass317_dm1_v1_f0380_f0097_direct_probe_attempt/route_noise_reduced_keylog.json`

## Proof predicates

```json
{json.dumps(required, indent=2, sort_keys=True)}
```

## Decision

{('Direct runtime hit proven by strict breakpoint predicates.' if status=='PROVEN_RUNTIME_HOOK' else blocker)}
''',encoding='utf-8')
    print(json.dumps({'status':status,'manifest':str(OUT/'manifest.json'),'report':str(REPORT),'predicates':required},indent=2)); return 0
if __name__=='__main__': raise SystemExit(main())
