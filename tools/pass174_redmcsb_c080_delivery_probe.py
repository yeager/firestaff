#!/usr/bin/env python3
"""Pass174: ReDMCSB-source-first C080/F0377 delivery probe.

Goal: after pass173 proved the source chain and still saw no portrait delta, test
whether host mouse delivery to the original DOSBox window can plausibly reach the
source C007/C080 dungeon-view command path. This does not guess gameplay: it uses
ReDMCSB anchors first, then tries multiple mouse delivery methods at the same
source-locked portrait point after a dungeon_gameplay gate.
"""
from __future__ import annotations
import json, shutil, subprocess, sys, time
from pathlib import Path
from typing import Any
from PIL import Image, ImageChops

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO))
from tools.pass118_state_aware_original_route_driver import wait_window, capture_new, classify_file, tap, click_original, xdo, run
from tools.pass80_original_frame_classifier import sha256

STAGE = Path.home()/'.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34'
DOSBOX = '/usr/bin/dosbox'
OUT_ROOT = Path('parity-evidence/verification/pass174_redmcsb_c080_delivery_probe')
RUN_BASE_ROOT = Path.home()/'.openclaw/data/firestaff-n2-runs'
SOURCE_ROOT = '/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'
SOURCE_AUDIT = [
  {'file':'COMMAND.C','lines':'397-403,2322-2323','claim':'C007_ZONE_VIEWPORT left-click maps to C080_COMMAND_CLICK_IN_DUNGEON_VIEW, which calls F0377_COMMAND_ProcessType80_ClickInDungeonView.'},
  {'file':'CLIKVIEW.C','lines':'348-349','claim':'PC build converts screen coordinates to viewport-relative by subtracting G2067_i_ViewportScreenX/G2068_i_ViewportScreenY.'},
  {'file':'CLIKVIEW.C','lines':'407-431','claim':'with empty leader hand, a hit in C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT calls F0372.'},
  {'file':'CLIKVIEW.C','lines':'21-25','claim':'F0372 computes the front square and calls F0275_SENSOR_IsTriggeredByClickOnWall on the opposite side.'},
  {'file':'MOVESENS.C','lines':'1392,1501-1502','claim':'C127_SENSOR_WALL_CHAMPION_PORTRAIT is allowed with no leader and calls F0280_CHAMPION_AddCandidateChampionToParty.'},
  {'file':'DUNVIEW.C','lines':'525,3913-3928','claim':'the portrait box is source-locked to viewport x=96..127 y=35..63; center maps to screen x=111,y=82 for PC y-origin 33.'},
  {'file':'COORD.C','lines':'1693-1698','claim':'G2067_i_ViewportScreenX=0 and F20E G2068_i_ViewportScreenY=33.'},
]
POINTS = [(111,82,'source_screen_portrait_center'), (111,80,'source_center_if_viewport_y31'), (112,83,'source_center_plus_one')]
METHODS = ['helper_scaled_window_relative','absolute_screen_click','window_relative_down_up','window_relative_click_repeat3']

def conf(out:Path)->Path:
    p=out/'dosbox-pass174.conf'
    p.write_text(f'''[sdl]\nfullscreen=false\noutput=opengl\n[dosbox]\nmachine=svga_paradise\nmemsize=4\ncaptures={out}\n[cpu]\ncore=normal\ncputype=386\ncpu_cycles=3000\n[render]\naspect=false\ninteger_scaling=false\n[mixer]\nnosound=true\n[speaker]\npcspeaker=false\ntandy=off\n[capture]\ncapture_dir={out}\ndefault_image_capture_formats=raw\n[autoexec]\nmount c "{STAGE}"\nc:\nDM -vv -sn\n''')
    return p

def shot(out:Path, log:list[str], label:str, idx:int)->dict[str,Any]:
    raw=capture_new(wait_window(log,timeout=5), out, label, log)
    dst=out/f'image{idx:04d}-{label}.png'
    if dst.exists(): dst.unlink()
    shutil.move(str(raw), dst)
    cls,reason=classify_file(dst)
    return {'index':idx,'label':label,'file':dst.name,'path':str(dst),'sha12':sha256(dst)[:12],'class':cls,'reason':reason}

def diff(a:Path,b:Path)->dict[str,Any]:
    ia,ib=Image.open(a).convert('RGB'),Image.open(b).convert('RGB')
    d=ImageChops.difference(ia,ib); bbox=d.getbbox(); nz=sum(1 for px in d.getdata() if px!=(0,0,0))
    return {'bbox':list(bbox) if bbox else None,'changed_pixels':nz,'changed_ratio':round(nz/(320*200),6)}

def geom_map(wid:str, x:int, y:int)->dict[str,int]:
    geom=xdo(['getwindowgeometry','--shell',wid]); vals={}
    for line in geom.splitlines():
        if '=' in line:
            k,v=line.split('=',1); vals[k]=int(v)
    gw,gh=vals.get('WIDTH',640),vals.get('HEIGHT',400)
    wx,wy=vals.get('X',0),vals.get('Y',0)
    cw,ch=gw,gw/(320/200)
    if ch>gh: ch=gh; cw=ch*(320/200)
    left,top=(gw-cw)/2,(gh-ch)/2
    px=round(left+((x+0.5)/320)*cw); py=round(top+((y+0.5)/200)*ch)
    return {'window_x':wx,'window_y':wy,'window_w':gw,'window_h':gh,'rel_x':px,'rel_y':py,'abs_x':wx+px,'abs_y':wy+py}

def deliver(wid:str, method:str, x:int, y:int, log:list[str])->dict[str,Any]:
    g=geom_map(wid,x,y)
    xdo(['windowactivate','--sync',wid],check=False)
    if method=='helper_scaled_window_relative':
        click_original(wid,x,y,log,delay=.65)
    elif method=='absolute_screen_click':
        xdo(['mousemove','--sync',str(g['abs_x']),str(g['abs_y'])],check=False); xdo(['click','1'],check=False); time.sleep(.65)
        log.append(f'absolute_click {x},{y} abs={g["abs_x"]},{g["abs_y"]} rel={g["rel_x"]},{g["rel_y"]}')
    elif method=='window_relative_down_up':
        xdo(['mousemove','--sync','--window',wid,str(g['rel_x']),str(g['rel_y'])],check=False); xdo(['mousedown','1'],check=False); time.sleep(.08); xdo(['mouseup','1'],check=False); time.sleep(.65)
        log.append(f'window_down_up {x},{y} rel={g["rel_x"]},{g["rel_y"]}')
    elif method=='window_relative_click_repeat3':
        xdo(['mousemove','--sync','--window',wid,str(g['rel_x']),str(g['rel_y'])],check=False); xdo(['click','--repeat','3','--delay','90','1'],check=False); time.sleep(.8)
        log.append(f'window_repeat3 {x},{y} rel={g["rel_x"]},{g["rel_y"]}')
    return g

def gate(out:Path, log:list[str], rows:list[dict[str,Any]], idx:int)->int:
    wid=wait_window(log,timeout=8); deadline=time.time()+20; attempt=0
    while time.time()<deadline:
        attempt+=1; r=shot(out,log,f'gate{attempt:02d}',idx); r['phase']='gate'; rows.append(r); idx+=1
        if r['class']=='dungeon_gameplay': return idx
        if r['class']=='entrance_menu': tap(wid,'Return',log,delay=1.0)
        else: time.sleep(.8)
    raise RuntimeError('no dungeon_gameplay gate')

def run_one(method:str,x:int,y:int,name:str)->dict[str,Any]:
    out=RUN_BASE/name; out.mkdir(parents=True,exist_ok=True); log=[]; rows=[]
    proc=subprocess.Popen([DOSBOX,'-conf',str(conf(out))],stdout=(out/'dosbox.log').open('w'),stderr=subprocess.STDOUT,text=True)
    try:
        wait_window(log,timeout=8); time.sleep(7); idx=1
        rows.append({'phase':'initial',**shot(out,log,'initial',idx)}); idx+=1
        idx=gate(out,log,rows,idx)
        before=rows[-1]
        wid=wait_window(log,timeout=5); mapping=deliver(wid,method,x,y,log)
        after={'phase':'click','method':method,'x':x,'y':y,'mapping':mapping,**shot(out,log,'after_click',idx)}; rows.append(after); idx+=1
        rows.append({'phase':'key_control','key':'F1',**(tap(wid,'F1',log,delay=.8) or shot(out,log,'after_f1',idx))}); idx+=1
    finally:
        try: proc.terminate(); proc.wait(timeout=2)
        except Exception: proc.kill()
    delta=diff(Path(before['path']),Path(after['path']))
    (out/'pass174_driver.log').write_text('\n'.join(log)+'\n')
    result={'name':name,'method':method,'point':{'x':x,'y':y,'label':name},'classification':'click-visible-delta' if delta['changed_pixels'] else 'click-no-visible-delta','delta':delta,'source_root':SOURCE_ROOT,'source_audit':SOURCE_AUDIT,'rows':rows,'evidence_dir':str(out)}
    (out/'summary.json').write_text(json.dumps(result,indent=2)+'\n')
    return result

def main()->int:
    global RUN_BASE
    OUT_ROOT.mkdir(parents=True,exist_ok=True)
    RUN_BASE=RUN_BASE_ROOT/(time.strftime('%Y%m%d-%H%M%S')+'-pass174-redmcsb-c080-delivery-probe')
    RUN_BASE.mkdir(parents=True,exist_ok=True)
    results=[]; errors=[]
    for method in METHODS:
        for x,y,plabel in POINTS:
            name=f'{method}_{plabel}'
            try: results.append(run_one(method,x,y,name))
            except Exception as e: errors.append({'name':name,'error':str(e)})
    if OUT_ROOT.exists(): shutil.rmtree(OUT_ROOT)
    shutil.copytree(RUN_BASE,OUT_ROOT)
    buckets={}
    for r in results: buckets[r['classification']]=buckets.get(r['classification'],0)+1
    manifest={'schema':'pass174_redmcsb_c080_delivery_probe.v1','run_base':str(RUN_BASE),'evidence_root':str(OUT_ROOT),'source_root':SOURCE_ROOT,'source_audit':SOURCE_AUDIT,'points':POINTS,'methods':METHODS,'completed':len(results),'errors':errors,'buckets':buckets,'results':results}
    (OUT_ROOT/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    lines=['# Pass174 — ReDMCSB C080/F0377 delivery probe','',f'- run base: `{RUN_BASE}`',f'- evidence root: `{OUT_ROOT}`',f'- completed: {len(results)}',f'- errors: {len(errors)}',f'- buckets: {buckets}',f'- ReDMCSB source root: `{SOURCE_ROOT}`','','## ReDMCSB source audit','']
    for s in SOURCE_AUDIT: lines.append(f"- `{s['file']}:{s['lines']}` — {s['claim']}")
    lines += ['','## Result','']
    for r in results: lines.append(f"- `{r['name']}`: **{r['classification']}** delta={r['delta']}")
    if errors: lines += ['','## Errors','']+[f"- `{e['name']}`: {e['error']}" for e in errors]
    (OUT_ROOT/'README.md').write_text('\n'.join(lines)+'\n')
    print(f'wrote {OUT_ROOT}/README.md'); print(f'completed={len(results)} errors={len(errors)} buckets={buckets}')
    return 1 if errors else 0
if __name__=='__main__': raise SystemExit(main())
