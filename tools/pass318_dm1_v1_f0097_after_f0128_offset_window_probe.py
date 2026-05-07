#!/usr/bin/env python3
from __future__ import annotations
import argparse,json,os,re,shlex,shutil,subprocess,tempfile,threading,time
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'parity-evidence/verification/pass318_dm1_v1_f0097_after_f0128_offset_window_probe'
REPORT=ROOT/'parity-evidence/pass318_dm1_v1_f0097_after_f0128_offset_window_probe.md'
ORIG=Path.home()/'.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34'
SOURCE_ROOT=Path.home()/'.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'
ADDR={'F0128_DUNGEONVIEW_Draw_CPSF':'23AD:40FE','F0097_DUNGEONVIEW_DrawViewport_entry':'2809:1E31','F0097_after_palette_zone_setup':'2809:1EBD','F0097_before_viewport_args':'2809:1EEE','F0097_VIDRV_09_BlitViewPort_indirect_call':'2809:1EFF'}
DEFAULT_ROUTE='wait:9000 enter wait:1800 one wait:1800 click:276,140 wait:1800 one wait:1800 kp5 wait:900 kp4 wait:900 kp6 wait:900 kp5 wait:900 kp4 wait:900 kp6 wait:900 kp5 wait:900 kp6 wait:900 kp4 wait:900 kp5 wait:900'
def run(cmd,**kw): return subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,**kw)
def clean(text):
    text=re.sub(r'\x1b\[[0-9;?]*[ -/]*[@-~]','',text).replace('\r','\n')
    return re.sub(r'[\x00-\x08\x0b\x0c\x0e-\x1f]','',text)
def cap(sess,n=240): return clean(run(['tmux','capture-pane','-p','-S',f'-{n}','-t',sess],timeout=5).stdout)
def send(sess,*keys): run(['tmux','send-keys','-t',sess,*keys],timeout=5)
def send_cmd(sess,cmd,events): events.append({'t':time.time(),'event':'debugger_cmd','cmd':cmd}); send(sess,cmd,'Enter'); time.sleep(.18)
def source_audit():
    checks=[('DRAWVIEW.C',[709,858],['void F0097_DUNGEONVIEW_DrawViewport','F0638_GetZone(C007_ZONE_VIEWPORT','VIDRV_09_BlitViewPort']),('DUNVIEW.C',[8604,8611],['F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)'])]
    out=[]
    for fn,(a,b),needles in checks:
        p=SOURCE_ROOT/fn; lines=p.read_text(encoding='latin-1',errors='replace').splitlines() if p.exists() else []
        block=' '.join('\n'.join(lines[a-1:min(b,len(lines))]).split())
        missing=[n for n in needles if ' '.join(n.split()) not in block]
        found={}
        for n in needles:
            for i in range(a-1,min(b,len(lines))):
                if ' '.join(n.split()) in ' '.join(lines[i].split()): found[n]=i+1; break
        out.append({'file':fn,'line_range':[a,b],'ok':p.exists() and not missing,'found':found,'missing':missing})
    return out
def write_conf(path,stage):
    path.write_text('\n'.join(['[sdl]','fullscreen=false','output=surface','usescancodes=false','[dosbox]','machine=svga_paradise','memsize=4','[cpu]','core=normal','cycles=3000','[mixer]','nosound=true','[autoexec]',f'mount c "{stage}"','c:','DEBUG DM.EXE -vv -sn -pk','']))
def stopped_signature(text):
    hits=[]
    for line in text.splitlines()[-90:]:
        stripped=line.strip()
        if 'breakpoint' in stripped.lower() or re.search(r'\bBP\s+[0-9A-F]{4}:[0-9A-F]{4}',line,re.I): hits.append(stripped)
        elif re.search(r'^[0-9A-F]{4}:[0-9A-F]{4}\s+',stripped): hits.append(stripped)
    return hits[-1] if hits else None
def classify_sig(sig):
    u=sig.upper()
    if ADDR['F0128_DUNGEONVIEW_Draw_CPSF'] in u: return 'f0128_bp'
    for k in ['F0097_DUNGEONVIEW_DrawViewport_entry','F0097_after_palette_zone_setup','F0097_before_viewport_args','F0097_VIDRV_09_BlitViewPort_indirect_call']:
        if ADDR[k] in u: return 'f0097_window_bp:'+k
    return 'other'
def wait_running(run_evt,stop_evt,log,timeout=35):
    end=time.time()+timeout
    while time.time()<end and not stop_evt.is_set():
        if run_evt.is_set(): return True
        time.sleep(.05)
    log.append({'t':time.time(),'event':'wait_running_timeout'}); return False
def xdo(display,args): return run(['xdotool',*args],env={**os.environ,'DISPLAY':display},timeout=10)
def find_window_pid(display):
    search=run(['xdotool','search','--sync','--class','dosbox'],env={**os.environ,'DISPLAY':display},timeout=10)
    ids=[line.strip() for line in search.stdout.splitlines() if line.strip()]
    if not ids:
        return None
    pid=run(['xdotool','getwindowpid',ids[0]],env={**os.environ,'DISPLAY':display},timeout=10)
    return int(pid.stdout.strip().splitlines()[0]) if pid.stdout.strip() else None
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
            x,y=map(int,low.split(':',1)[1].split(',')); px,py=click_at(display,window,x,y); log.append({'t':time.time(),'event':'click','route_item':item,'screen':[px,py],'running_guard':True})
        else:
            xdo(display,['key','--window',window,key_name(low)]); log.append({'t':time.time(),'event':'key','route_item':item,'running_guard':True})
        time.sleep(.2)
    log.append({'t':time.time(),'event':'route_done'})
def set_stage(sess,stage,events):
    send_cmd(sess,'BPDEL *',events)
    cmds=[f"BP {ADDR['F0128_DUNGEONVIEW_Draw_CPSF']}"] if stage=='f0128_gate' else [f"BP {ADDR[k]}" for k in ['F0097_DUNGEONVIEW_DrawViewport_entry','F0097_after_palette_zone_setup','F0097_before_viewport_args','F0097_VIDRV_09_BlitViewPort_indirect_call']]
    for c in cmds: send_cmd(sess,c,events)
    send_cmd(sess,'BPLIST',events)
def debugger_loop(sess,stop_evt,run_evt,snaps,hits,events):
    seen=set(); stage='f0128_gate'; f0128_seen=False; set_stage(sess,stage,events); send(sess,'Escape','O','t'); run_evt.set(); last_continue=0.0
    while not stop_evt.is_set():
        txt=cap(sess,300); sig=stopped_signature(txt)
        if sig and sig not in seen:
            run_evt.clear(); seen.add(sig); kind=classify_sig(sig); hit={'t':time.time(),'sig':sig,'kind':kind,'stage':stage}; hits.append(hit); snaps.append(f"\n--- STOP {len(hits)} {kind} stage={stage} ---\n"+txt[-16000:])
            for c in ['CPU','BPLIST']: send_cmd(sess,c,events)
            snaps.append(f"\n--- AFTER CAPTURE {len(hits)} ---\n"+cap(sess,300)[-16000:])
            if kind=='f0128_bp' and not f0128_seen:
                f0128_seen=True; stage='f0097_after_f0128_window'; set_stage(sess,stage,events)
            send(sess,'Escape','O','t'); last_continue=time.time(); run_evt.set()
        elif sig and time.time()-last_continue>1.2:
            send_cmd(sess,'CPU',events); send(sess,'Escape','O','t'); last_continue=time.time(); run_evt.set()
        time.sleep(.12)
def bounded_excerpt(text):
    pat=re.compile(r'(STOP|BP 23AD:40FE|BP 2809:1E31|BP 2809:1EBD|BP 2809:1EEE|BP 2809:1EFF|Breakpoint list)',re.I); out=[]
    for line in text.splitlines():
        if pat.search(line): out.append(line[:180])
        if len(out)>=60: break
    return out
def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--seconds',type=int,default=75); ap.add_argument('--route',default=DEFAULT_ROUTE); args=ap.parse_args()
    OUT.mkdir(parents=True,exist_ok=True); missing=[x for x in ['dosbox-debug','tmux','Xvfb','xdotool'] if not shutil.which(x)]
    if missing: raise SystemExit('missing: '+', '.join(missing))
    audit=source_audit(); route_log=[]; hits=[]; snaps=[]; events=[]; dosbox_pid=None
    with tempfile.TemporaryDirectory(prefix='firestaff-pass318-') as td:
        stage_dir=Path(td)/'dos'; shutil.copytree(ORIG,stage_dir); conf=Path(td)/'dosbox.conf'; write_conf(conf,stage_dir)
        display=':78'; xvfb=subprocess.Popen(['Xvfb',display,'-screen','0','1024x768x24'],stdout=subprocess.DEVNULL,stderr=subprocess.STDOUT); time.sleep(.5)
        sess=f'pass318-{os.getpid()}'; stop_evt=threading.Event(); run_evt=threading.Event()
        try:
            r=run(['tmux','new-session','-d','-s',sess,f'TERM=vt100 DISPLAY={display} dosbox-debug -conf {shlex.quote(str(conf))} -exit'],timeout=5)
            if r.returncode: raise RuntimeError(r.stdout)
            time.sleep(3); threading.Thread(target=debugger_loop,args=(sess,stop_evt,run_evt,snaps,hits,events),daemon=True).start(); time.sleep(1)
            dosbox_pid=find_window_pid(display)
            if dosbox_pid: threading.Thread(target=key_loop,args=(display,dosbox_pid,args.route,stop_evt,run_evt,route_log),daemon=True).start()
            end=time.time()+args.seconds
            while time.time()<end: snaps.append('\n--- POLL ---\n'+cap(sess,300)[-12000:]); time.sleep(2)
            stop_evt.set(); time.sleep(.5); snaps.append('\n--- FINAL ---\n'+cap(sess,5000))
        finally:
            stop_evt.set(); run(['tmux','kill-session','-t',sess],timeout=5); xvfb.terminate(); xvfb.wait(timeout=5)
    transcript=clean('\n'.join(snaps)); (OUT/'dosbox_debug_noise_reduced.clean.txt').write_text(transcript[-500000:],encoding='utf-8'); (OUT/'route_noise_reduced_keylog.json').write_text(json.dumps(route_log,indent=2,sort_keys=True)+'\n')
    f0128=any(h.get('kind')=='f0128_bp' for h in hits); f0097=[h for h in hits if str(h.get('kind','')).startswith('f0097_window_bp')]
    status='F0097_OFFSET_WINDOW_HIT_VERIFIED' if f0128 and f0097 else 'BLOCKED_F0097_AFTER_F0128_WINDOW_NO_HIT'
    manifest={'schema':'pass318_dm1_v1_f0097_after_f0128_offset_window_probe.raw_probe.v1','timestamp_utc':datetime.now(timezone.utc).isoformat(),'status':status,'source_audit':audit,'addresses':ADDR,'route':args.route,'method':'F0128-gated F0097 offset window: arm only F0128 first; after first F0128 stop, arm entry/post-zone/arg/VIDRV-call candidates and continue guarded route.','debugger_events':events[:240],'debugger_hits_captured':hits[:80],'direct_hits':{'f0128_23AD_40FE':f0128,'f0097_window':bool(f0097),'f0097_hit_kinds':[h.get('kind') for h in f0097]},'bounded_sanitized_transcript_excerpt':bounded_excerpt(transcript),'blocker':None if status.endswith('VERIFIED') else 'No explicit F0097-window stop appeared after the F0128-gated arm. Need either reliable post-stop debugger command sequencing/retained BPLIST, or different static offsets/segment rebasing before claiming viewport_present.','promotion_rule':'Promote viewport_present only for an explicit F0097-window debugger stop after prior F0128 stop.'}
    (OUT/'manifest.json').write_text(json.dumps(manifest,indent=2,sort_keys=True)+'\n')
    REPORT.write_text(f'''# Pass318 — DM1 V1 F0097 after-F0128 offset-window runtime probe\n\nStatus: `{status}`\n\n## What this pass tried\n\n- Started with ReDMCSB source audit: `DRAWVIEW.C:709-858` and `DUNVIEW.C:8604-8611`.\n- Armed only F0128 (`23AD:40FE`) first.\n- After F0128, armed F0097 candidates: entry `2809:1E31`, post-palette/zone setup `2809:1EBD`, viewport-argument push `2809:1EEE`, and `VIDRV_09_BlitViewPort` indirect call `2809:1EFF`.\n\n## Decision\n\n{('F0097 viewport-present window hit proven after F0128.' if status=='F0097_OFFSET_WINDOW_HIT_VERIFIED' else manifest['blocker'])}\n\nManifest: `parity-evidence/verification/pass318_dm1_v1_f0097_after_f0128_offset_window_probe/manifest.json`\n''',encoding='utf-8')
    print(json.dumps({'status':status,'hits':hits,'manifest':str(OUT/'manifest.json')},indent=2)); return 0
if __name__=='__main__': raise SystemExit(main())
