#!/usr/bin/env python3
from __future__ import annotations
import argparse,json,os,re,shutil,subprocess,tempfile,time
from datetime import datetime,timezone
from pathlib import Path
from typing import Any
import pexpect
ROOT=Path(__file__).resolve().parents[1]
PASS='pass328_dm1_v1_direct_pty_postload_route_timing'
OUT=ROOT/'parity-evidence/verification'/PASS
REPORT=ROOT/'parity-evidence'/f'{PASS}.md'
ORIG=Path.home()/'.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34'
SRC=Path.home()/'.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'
F0128='23AD:40FE'; F0097='2809:1EFF'
LOAD_ROUTE='wait:9000 enter wait:2200 one wait:2200 click:276,140 wait:2200 one wait:3500'
MOVE_ROUTE='kp5 wait:900 kp4 wait:900 kp6 wait:900 kp5 wait:900 kp4 wait:900 kp6 wait:900 kp5 wait:900'
ANSI_RE=re.compile(r'\x1b\[[0-9;?]*[ -/]*[@-~]')
CODE_LINE_RE=re.compile(r'\b(?P<addr>[0-9A-F]{4}:[0-9A-F]{4})\s+[0-9A-F]{2,}\s*[a-z][a-z0-9]+',re.I)

def clean(t:str)->str: return re.sub(r'[\x00-\x08\x0b\x0c\x0e-\x1f]','',ANSI_RE.sub('',t).replace('\r','\n'))
def run(cmd:list[str],**kw:Any): return subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,**kw)
def drain(child,seconds:float)->str:
    end=time.time()+seconds; out=''
    while time.time()<end:
        try: out+=child.read_nonblocking(8192,timeout=.05)
        except pexpect.TIMEOUT: pass
        except pexpect.EOF: out+='<EOF>'; break
    return out
def write_conf(path:Path,stage:Path):
    path.write_text('\n'.join(['[sdl]','fullscreen=false','output=surface','usescancodes=false','[dosbox]','machine=svga_paradise','memsize=4','[cpu]','core=normal','cycles=3000','[mixer]','nosound=true','[autoexec]',f'mount c {stage}','c:','DEBUG DM.EXE -vv -sn -pk','']),encoding='utf-8')
def source_audit():
    specs=[('DUNVIEW.C','F0128',['void F0128_DUNGEONVIEW_Draw_CPSF','F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)']),('DRAWVIEW.C','F0097',['void F0097_DUNGEONVIEW_DrawViewport','VIDRV_09_BlitViewPort']),('COMMAND.C','F0380',['void F0380_COMMAND_ProcessQueue_CPSC','F0365_COMMAND_ProcessTypes1To2_TurnParty','F0366_COMMAND_ProcessTypes3To6_MoveParty'])]
    rows=[]
    for fn,ident,needles in specs:
        lines=(SRC/fn).read_text(encoding='latin-1',errors='replace').splitlines(); anchors={}
        for n in needles:
            nn=' '.join(n.split())
            for i,l in enumerate(lines,1):
                if nn in ' '.join(l.split()): anchors[n]=i; break
        rows.append({'id':ident,'file':fn,'ok':len(anchors)==len(needles),'anchors':anchors,'missing':[n for n in needles if n not in anchors]})
    return rows
def xdo(display,args): return run(['xdotool',*args],env={**os.environ,'DISPLAY':display},timeout=10)
def find_win(display):
    ids=[x.strip() for x in xdo(display,['search','--sync','--class','dosbox']).stdout.splitlines() if x.strip()]
    return ids[0] if ids else None
def key_name(tok): return {'enter':'Return','return':'Return','one':'1','1':'1','kp4':'KP_Left','kp5':'KP_Begin','kp6':'KP_Right'}[tok]
def click_at(display,win,x,y):
    ns={}; exec(xdo(display,['getwindowgeometry','--shell',win]).stdout,{},ns)
    gw,gh=float(ns['WIDTH']),float(ns['HEIGHT']); aspect=320/200; cw=gw; ch=cw/aspect
    if ch>gh: ch=gh; cw=ch*aspect
    px=int(round((gw-cw)/2+((x+.5)/320)*cw)); py=int(round((gh-ch)/2+((y+.5)/200)*ch))
    xdo(display,['mousemove','--window',win,str(px),str(py),'click','1']); return [px,py]
def drive(display,win,route,log):
    xdo(display,['windowactivate','--sync',win]); xdo(display,['windowfocus','--sync',win])
    for item in route.split():
        low=item.lower(); log.append({'t':time.time(),'route_item':item})
        if low.startswith('wait:'): time.sleep(int(low.split(':',1)[1])/1000); continue
        if low.startswith('click:'):
            x,y=map(int,low.split(':',1)[1].split(',')); log[-1]['screen']=click_at(display,win,x,y)
        else: xdo(display,['key','--window',win,key_name(low)])
        time.sleep(.2)
def dbg(child,cmd,log):
    child.sendline(cmd); time.sleep(.2); out=drain(child,.55); log.append({'t':time.time(),'cmd':cmd,'excerpt':clean(out)[-500:]}); return out
def last_code_addr(t):
    m=[x.group('addr').upper() for x in CODE_LINE_RE.finditer(clean(t))]
    return m[-1] if m else None
def run_probe(seconds:int,load_route:str,move_route:str):
    missing=[x for x in ['dosbox-debug','Xvfb','xdotool'] if not shutil.which(x)]
    if missing: return {'ran':False,'stage':'tooling','blocker':'missing '+','.join(missing)}
    OUT.mkdir(parents=True,exist_ok=True); start=time.time(); transcript=''; cmdlog=[]; routelog=[]; stops=[]
    with tempfile.TemporaryDirectory(prefix='firestaff-pass328-') as td:
        stage=Path(td)/'dos'; shutil.copytree(ORIG,stage); conf=Path(td)/'dosbox.conf'; write_conf(conf,stage); display=f':{70+(os.getpid()%20)}'
        xvfb=subprocess.Popen(['Xvfb',display,'-screen','0','1024x768x24'],stdout=subprocess.DEVNULL,stderr=subprocess.STDOUT); time.sleep(.5)
        child=pexpect.spawn('dosbox-debug',['-conf',str(conf),'-exit'],env={**os.environ,'DISPLAY':display,'TERM':'vt100'},encoding='utf-8',timeout=2,echo=False); child.delaybeforesend=.05
        try:
            time.sleep(3); transcript+=drain(child,1)
            win=find_win(display)
            if not win: return {'ran':True,'stage':'load-ready detection','blocker':'dosbox window not found'}
            child.send('\x1bOt'); cmdlog.append({'t':time.time(),'control':'F5','purpose':'run unarmed to load route'})
            transcript+=drain(child,.5)
            drive(display,win,load_route,routelog)
            xdo(display,['key','--window',win,'Alt+Pause']); cmdlog.append({'t':time.time(),'control':'Alt+Pause','purpose':'post-load debugger break'})
            post=drain(child,5); transcript+=post
            if '->' not in clean(post):
                xdo(display,['key','--window',win,'Pause']); cmdlog.append({'t':time.time(),'control':'Pause','purpose':'post-load debugger break fallback'})
                post=drain(child,5); transcript+=post
            if '->' not in clean(post):
                return {'ran':True,'stage':'load-ready detection','blocker':'post-load debugger prompt not reached via Alt+Pause/Pause','routeLog':routelog,'commandLog':cmdlog}
            for c in ['BPDEL *',f'BP {F0128}','BPLIST']: transcript+=dbg(child,c,cmdlog)
            child.send('\x1bOt'); cmdlog.append({'t':time.time(),'control':'F5','purpose':'run after post-load F0128 arm'})
            deadline=time.time()+seconds; buf=''
            # movement route starts after arm; no separate thread needed because debugger will stop if BP fires
            drive(display,win,move_route,routelog)
            while time.time()<deadline:
                chunk=drain(child,.25)
                if chunk:
                    transcript+=chunk; buf+=chunk; c=clean(buf)
                    if '(Running)' in c and '->' in c.split('(Running)',1)[-1]:
                        addr=last_code_addr(c.split('(Running)',1)[-1]); row={'t':time.time(),'addr':addr,'postRunningCodeLines':[m.group(0)[:160] for m in CODE_LINE_RE.finditer(c.split('(Running)',1)[-1])][-8:]}; stops.append(row)
                        if addr==F0128:
                            transcript+=dbg(child,f'BP {F0097}',cmdlog); child.send('\x1bOt'); cmdlog.append({'t':time.time(),'control':'F5','purpose':'run after F0097 candidate arm'}); break
                        buf=''; child.send('\x1bOt'); cmdlog.append({'t':time.time(),'control':'F5','purpose':'continue non-target'})
                time.sleep(.05)
            if stops and stops[-1].get('addr')==F0128:
                buf=''; end=time.time()+min(10, max(1, deadline-time.time()))
                while time.time()<end:
                    chunk=drain(child,.25)
                    if chunk:
                        transcript+=chunk; buf+=chunk; c=clean(buf)
                        if '(Running)' in c and '->' in c.split('(Running)',1)[-1]:
                            addr=last_code_addr(c.split('(Running)',1)[-1]); stops.append({'t':time.time(),'addr':addr,'afterF0128':True,'postRunningCodeLines':[m.group(0)[:160] for m in CODE_LINE_RE.finditer(c.split('(Running)',1)[-1])][-8:]}); break
                    time.sleep(.05)
        finally:
            try: transcript+=drain(child,.5); child.terminate(force=True)
            except Exception: pass
            xvfb.terminate();
            try: xvfb.wait(timeout=5)
            except subprocess.TimeoutExpired: xvfb.kill()
    clean_text=clean(transcript)[-400000:]; (OUT/'direct_pty_postload.clean.txt').write_text(clean_text+'\n',encoding='utf-8'); (OUT/'route_keylog.json').write_text(json.dumps(routelog,indent=2,sort_keys=True)+'\n',encoding='utf-8')
    f0128=any(s.get('addr')==F0128 for s in stops); f0097=any(s.get('afterF0128') and s.get('addr')==F0097 for s in stops)
    stage=None if f0128 else ('breakpoint arming timing' if any('BP 23AD:40FE' in e.get('excerpt','') for e in cmdlog) else 'breakpoint arming timing')
    return {'ran':True,'durationSeconds':round(time.time()-start,3),'method':'direct PTY; run unarmed through load route; Alt+Pause post-load prompt; arm F0128 only after prompt','loadRoute':load_route,'moveRoute':move_route,'routeLog':routelog,'commandLog':cmdlog,'stops':stops,'directHits':{'f0128_23AD_40FE':f0128,'f0097_2809_1EFF_after_f0128':f0097},'stage':stage,'transcript':str(OUT/'direct_pty_postload.clean.txt')}
def build(args):
    audit=source_audit(); rt={'ran':False,'blocker':'--no-runtime'} if args.no_runtime else run_probe(args.seconds,args.load_route,args.move_route)
    if rt.get('directHits',{}).get('f0097_2809_1EFF_after_f0128'): status='PASS_DIRECT_PTY_F0128_F0097_SEQUENCE_PROVEN'
    elif rt.get('directHits',{}).get('f0128_23AD_40FE'): status='PASS_DIRECT_PTY_F0128_CODE_STOP_PROVEN'
    elif rt.get('stage')=='route input': status='BLOCKED_PASS328_POSTLOAD_ROUTE_INPUT_MISSING'
    else: status='BLOCKED_PASS328_BREAKPOINT_ARMING_TIMING'
    return {'schema':PASS+'.v1','timestampUtc':datetime.now(timezone.utc).isoformat(),'status':status,'sourceAudit':audit,'addresses':{'F0128_DUNGEONVIEW_Draw_CPSF':F0128,'F0097_VIDRV_09_BlitViewPort_indirect_call':F0097},'runtimeProbe':rt,'blocker':None if status.startswith('PASS') else rt.get('blocker') or rt.get('stage'),'notPromotedBy':['BPLIST','BP command echo','tmux/capture-pane']}
def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--seconds',type=int,default=75); ap.add_argument('--load-route',default=LOAD_ROUTE); ap.add_argument('--move-route',default=MOVE_ROUTE); ap.add_argument('--no-runtime',action='store_true'); args=ap.parse_args(); args.seconds=max(10,min(75,args.seconds))
    m=build(args); OUT.mkdir(parents=True,exist_ok=True); (OUT/'manifest.json').write_text(json.dumps(m,indent=2,sort_keys=True)+'\n',encoding='utf-8')
    REPORT.write_text('\n'.join(['# Pass328 — DM1 V1 direct-PTY post-load route/timing probe','',f"Status: `{m['status']}`",'', '## ReDMCSB anchors','']+[f"- `{r['file']}` {r['id']}: `{r['anchors']}`" for r in m['sourceAudit']]+['','## Runtime decision','',f"- Method: `{m['runtimeProbe'].get('method')}`",f"- Direct hits: `{m['runtimeProbe'].get('directHits')}`",f"- Blocker: `{m.get('blocker')}`",'', 'Manifest: `parity-evidence/verification/pass328_dm1_v1_direct_pty_postload_route_timing/manifest.json`'])+'\n',encoding='utf-8')
    print(json.dumps({'status':m['status'],'manifest':str(OUT/'manifest.json'),'directHits':m['runtimeProbe'].get('directHits'),'blocker':m.get('blocker')},indent=2,sort_keys=True)); return 0
if __name__=='__main__': raise SystemExit(main())
