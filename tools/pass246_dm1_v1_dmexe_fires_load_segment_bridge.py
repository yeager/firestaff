#!/usr/bin/env python3
from __future__ import annotations
import hashlib, json, os, re, shutil, struct, subprocess, tempfile, threading, time
from pathlib import Path
from typing import Any
import pexpect
ROOT=Path(__file__).resolve().parents[1]
ORIG=Path.home()/'.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34'
UNLZEXE=Path.home()/'.openclaw/data/firestaff-redmcsb-source/Toolchains/Common/Base/UNLZEXE/unlzexe.exe'
OUT=ROOT/'parity-evidence/verification/pass246_dm1_v1_dmexe_fires_load_segment_bridge'
REPORT=ROOT/'parity-evidence/pass246_dm1_v1_dmexe_fires_load_segment_bridge.md'
EXPECTED='fc79ac65046e3d96c189ac3dd20ad40bacb8debee2cd1c7d2c33ca2d8f82fe94'
def sha256(p:Path)->str:
    h=hashlib.sha256()
    with p.open('rb') as f:
        for b in iter(lambda:f.read(1048576),b''): h.update(b)
    return h.hexdigest()
def mz(p:Path)->dict[str,Any]:
    b=p.read_bytes()[:64]; vals=struct.unpack_from('<HHHHHHHHHHHH',b,2)
    return {'header_bytes':vals[3]*16,'e_ip':vals[9],'e_cs':vals[10],'entry_cs_ip':f'{vals[10]:04X}:{vals[9]:04X}'}
def make_exenew(stage:Path):
    work=stage/'FIRES.EXE'; shutil.copy2(ORIG/'FIRES',work)
    proc=subprocess.run(['wine',str(UNLZEXE),f'Z:{work}'],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=60)
    ex=stage/'FIRES.EXENEW'; info={'path_policy':'temporary only; not committed','returncode':proc.returncode,'sha256':sha256(ex),'size':ex.stat().st_size,'mz':mz(ex)}
    if info['sha256']!=EXPECTED: raise RuntimeError(info)
    return ex,info
def clean(s:str)->str:
    s=s.replace('\r','\n')
    s=re.sub(r'\x1b\[[0-9;?]*[ -/]*[@-~]','',s)
    s=re.sub(r'/tmp/firestaff-pass246-[^\s"]+','<TMP>',s)
    s=re.sub(r'(?m)^(\s*)\d{5,}:',r'\1<TICKS>:',s)
    s=re.sub(r'\(B\d{5,}(?=b_)','(B<TICKS>',s)
    return '\n'.join(line.rstrip() for line in s.split('\n'))
def run_route(display:str):
    env=os.environ.copy(); env['DISPLAY']=display; time.sleep(12)
    try: w=subprocess.check_output(['xdotool','search','--name','DOSBox'],text=True,env=env).split()[0]
    except Exception: return
    def key(k): subprocess.run(['xdotool','key','--window',w,k],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL); time.sleep(.15)
    def wait(ms): time.sleep(ms/1000)
    wait(9000); key('Return'); wait(1800); key('1'); wait(1800)
    geom=subprocess.check_output(['xdotool','getwindowgeometry','--shell',w],text=True,env=env); d={}
    for line in geom.splitlines():
        if '=' in line:
            a,b=line.split('=',1); d[a]=int(b)
    gw,gh=d['WIDTH'],d['HEIGHT']; x,y=276,140; aspect=320/200; cw=gw; ch=cw/aspect
    if ch>gh: ch=gh; cw=ch*aspect
    px=round((gw-cw)/2+((x+.5)/320)*cw); py=round((gh-ch)/2+((y+.5)/200)*ch)
    subprocess.run(['xdotool','mousemove','--window',w,str(px),str(py),'click','1'],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL); wait(2200); key('1'); wait(2500)
def parse_match(stop_text:str, exenew:Path)->dict[str,Any]:
    text=clean(stop_text); code=[]
    for line in text.splitlines():
        m=re.search(r'([0-9A-F]{4}):([0-9A-F]{4})\s+((?:[0-9A-F]{2}){1,8})',line)
        if m: code.append((int(m.group(1),16),int(m.group(2),16),bytes.fromhex(m.group(3)),line.strip()))
    int_rows=[r for r in code if r[2].startswith(b'\xcd\x21')]
    row=int_rows[-1] if int_rows else (code[-1] if code else None)
    if not row: return {'ok':False,'reason':'no_code_row_parsed','stop_excerpt':text[-3000:]}
    rcs,rip,rb,line=row; seq=b''; started=False
    for cs,ip,b,line2 in code:
        if cs==rcs and ip==rip: started=True
        if started and cs==rcs:
            seq+=b
            if len(seq)>=24: break
    body=exenew.read_bytes()[mz(exenew)['header_bytes']:]; candidates=[]
    for n in range(min(len(seq),24),1,-1):
        offs=[m.start() for m in re.finditer(re.escape(seq[:n]),body)]
        if offs:
            candidates=[{'linear':o,'static_cs':o//16,'static_ip':o%16,'matched_len':n} for o in offs[:20]]; break
    if not candidates: return {'ok':False,'reason':'byte_sequence_not_found','runtime_cs_ip':f'{rcs:04X}:{rip:04X}','line':line,'seq_hex':seq.hex(),'stop_excerpt':text[-3000:]}
    for c in candidates:
        c['runtime_load_segment']=f"{(rcs-c['static_cs'])&0xffff:04X}"; c['runtime_cs_ip']=f'{rcs:04X}:{rip:04X}'; c['static_cs_ip']=f"{c['static_cs']:04X}:{c['static_ip']:04X}"
    return {'ok':len(candidates)==1,'runtime_cs_ip':f'{rcs:04X}:{rip:04X}','selected_line':line,'sequence_hex':seq.hex(),'candidates':candidates,'stop_excerpt':text[-3000:]}
def main()->int:
    OUT.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='firestaff-pass246-') as td:
        stage=Path(td); shutil.copytree(ORIG,stage,dirs_exist_ok=True); ex,exinfo=make_exenew(stage)
        (stage/'dosbox.conf').write_text('\n'.join(['[sdl]','fullscreen=false','output=surface','[dosbox]','machine=svga_paradise','memsize=4','[cpu]','core=normal','cycles=3000','[mixer]','nosound=true','[autoexec]',f'mount c "{stage}"','c:','DEBUG DM.EXE -vv -sn -pk','']))
        display=':88'; xvfb=subprocess.Popen(['Xvfb',display,'-screen','0','1024x768x16'],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
        os.environ['DISPLAY']=display; os.environ['TERM']='xterm'; log=[]; target_stop=''
        try:
            threading.Thread(target=run_route,args=(display,),daemon=True).start()
            child=pexpect.spawn('dosbox-debug',['-conf',str(stage/'dosbox.conf'),'-exit'],encoding='latin-1',timeout=35,dimensions=(50,80)); child.delaybeforesend=.03
            child.expect('TYPE HELP',timeout=35); log.append(child.before+child.after)
            child.sendline('bpint 21 4b'); child.expect('DEBUG: Set interrupt breakpoint',timeout=5); log.append(child.before+child.after); child.send('\x1b[15~')
            for i in range(8):
                child.expect('->',timeout=45); chunk=child.before+child.after; log.append(chunk)
                if i==5:
                    child.sendline('bpint 21 3d'); child.expect('DEBUG: Set interrupt breakpoint',timeout=5); log.append(child.before+child.after)
                if i==6: target_stop=chunk; break
                child.send('\x1b[15~')
            child.close(force=True)
        finally: xvfb.terminate()
        transcript=clean(''.join(log)); (OUT/'dmexe_fires_bridge_transcript.clean.txt').write_text(transcript[-20000:],encoding='latin-1')
        match=parse_match(target_stop,ex)
    status='PASS_DMEXE_FIRES_LOAD_SEGMENT_BRIDGED_NO_RUNTIME_HIT_PROMOTION' if match.get('ok') else 'BLOCKED_DMEXE_FIRES_LOAD_SEGMENT_AMBIGUOUS'
    manifest={'schema':'pass246_dm1_v1_dmexe_fires_load_segment_bridge.v1','status':status,'fires_exenew':exinfo,'dm_route':'DEBUG DM.EXE -vv -sn -pk; BPINT 21 AH=4B through launcher EXEC chain; BPINT 21 AH=3D for first FIRES-side file open','match':match,'non_claims':['does not promote verified_runtime_hit','does not prove command/movement/viewport chain','temporary original-derived FIRES.EXENEW not committed'],'transcript_excerpt':'parity-evidence/verification/pass246_dm1_v1_dmexe_fires_load_segment_bridge/dmexe_fires_bridge_transcript.clean.txt'}
    (OUT/'manifest.json').write_text(json.dumps(manifest,indent=2,sort_keys=True)+'\n')
    lines=['# Pass246 — DM.EXE FIRES load-segment bridge','',f'Status: `{status}`','','## Result']
    if match.get('candidates'):
        c=match['candidates'][0]; lines += ['',f"- Actual DM.EXE-loader FIRES-side stop: `{match.get('runtime_cs_ip')}`.",f"- Matched FIRES.EXENEW static CS:IP: `{c['static_cs_ip']}` (`{c['matched_len']}` bytes).",f"- Derived actual FIRES load segment under `DM.EXE`: `{c['runtime_load_segment']}`."]
    lines += ['','## Guardrail','','This is a loader-segment bridge only. No `verified_runtime_hit` promotion and no command/movement/viewport chain proof.','',f"Manifest: `{OUT/'manifest.json'}`"]
    REPORT.write_text('\n'.join(lines)+'\n')
    print(json.dumps({'status':status,'manifest':str(OUT/'manifest.json'),'report':str(REPORT),'load_segment':(match.get('candidates') or [{}])[0].get('runtime_load_segment')},indent=2,sort_keys=True))
    return 0 if status.startswith('PASS_') else 1
if __name__=='__main__': raise SystemExit(main())
