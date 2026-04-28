#!/usr/bin/env python3
from __future__ import annotations
import json, shutil, subprocess, sys, time
from pathlib import Path
REPO=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(REPO))
from tools.pass118_state_aware_original_route_driver import wait_window,capture_new, classify_file, tap, click_original
from tools.pass80_original_frame_classifier import sha256
STAGE=Path.home()/'.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34'
DOSBOX='/usr/bin/dosbox'; PROGRAM='DM -vv -sn -pm'
BASE_ROUTE=[('key','Return'),('wait',1.4),('key','F1'),('wait',0.9)]
SCENARIOS={
 'key4_probe_actions': [('key','4'),('wait',1.2),('key','Up'),('wait',1.0),('key','Right'),('wait',1.0),('click',275,47),('wait',1.0),('key','Up'),('wait',1.0),('key','Left'),('wait',1.0),('click',235,52),('wait',1.0)],
 'return_probe_actions': [('key','Return'),('wait',1.2),('key','Up'),('wait',1.0),('key','Right'),('wait',1.0),('click',276,158),('wait',1.0),('key','Up'),('wait',1.0),('key','Down'),('wait',1.0)],
 'panel_probe_actions': [('click',235,52),('click',276,158),('wait',1.2),('key','Up'),('wait',1.0),('key','Right'),('wait',1.0),('click',190,8),('wait',1.0),('key','Left'),('wait',1.0)],
}
def conf(out:Path):
 p=out/'dosbox-pass151.conf'; p.write_text(f'''[sdl]\nfullscreen=false\noutput=opengl\n[dosbox]\nmachine=svga_paradise\nmemsize=4\ncaptures={out}\n[cpu]\ncore=normal\ncputype=386\ncpu_cycles=3000\n[render]\naspect=false\ninteger_scaling=false\n[mixer]\nnosound=true\n[speaker]\npcspeaker=false\ntandy=off\n[capture]\ncapture_dir={out}\ndefault_image_capture_formats=raw\n[autoexec]\nmount c "{STAGE}"\nc:\n{PROGRAM}\n'''); return p
def safe(log,label,fn):
 last=None
 for i in range(3):
  try: return fn()
  except Exception as e: last=e; log.append(f'retry {label} {i+1}: {e}'); time.sleep(.25)
 raise RuntimeError(f'failed {label}: {last}')
def bbox_png(path:Path):
 try:
  from PIL import Image
  im=Image.open(path).convert('RGB'); pix=im.load(); w,h=im.size
  bg=pix[0,0]; xs=[]; ys=[]
  for y in range(h):
   for x in range(w):
    r,g,b=pix[x,y]
    if abs(r-bg[0])+abs(g-bg[1])+abs(b-bg[2])>18:
     xs.append(x); ys.append(y)
  if not xs: return None
  return [min(xs),min(ys),max(xs),max(ys)]
 except Exception as e: return {'error':str(e)}
def shot(out,log,label,idx):
 raw=safe(log,f'capture-{label}',lambda:capture_new(wait_window(log,timeout=5.0),out,label,log))
 dst=out/f'image{idx:04d}-{label}.png'
 if dst.exists(): dst.unlink()
 shutil.move(str(raw),dst); cls,reason=classify_file(dst)
 return {'label':label,'file':dst.name,'sha12':sha256(dst)[:12],'class':cls,'reason':reason,'bbox':bbox_png(dst)}
def do(out,log,a,idx):
 if a[0]=='wait': time.sleep(float(a[1])); return {'phase':'wait','seconds':a[1],**shot(out,log,f'wait_{idx}',idx)}
 if a[0]=='key': safe(log,f'key-{a[1]}',lambda:tap(wait_window(log),a[1],log,delay=.9)); return {'phase':'key','value':a[1],**shot(out,log,f'key_{a[1]}_{idx}',idx)}
 _,x,y=a; safe(log,f'click-{x}-{y}',lambda:click_original(wait_window(log),x,y,log,delay=.9)); return {'phase':'click','x':x,'y':y,**shot(out,log,f'click_{x}_{y}_{idx}',idx)}
def run_one(base,name,actions):
 out=base/name; out.mkdir(parents=True,exist_ok=True); rows=[]; log=[]; idx=1
 proc=subprocess.Popen([DOSBOX,'-conf',str(conf(out))],stdout=(out/'dosbox.log').open('w'),stderr=subprocess.STDOUT,text=True)
 try:
  wait_window(log); time.sleep(7.0); rows.append({'phase':'initial',**shot(out,log,'initial',idx)}); idx+=1
  for a in BASE_ROUTE+actions: rows.append(do(out,log,a,idx)); idx+=1
 finally:
  try: proc.terminate(); proc.wait(timeout=2)
  except Exception: proc.kill()
 (out/'pass151_driver.log').write_text('\n'.join(log)+'\n'); (out/'pass151_rows.json').write_text(json.dumps(rows,indent=2)+'\n')
 return {'scenario':name,'rows':rows}
def main():
 base=Path(sys.argv[1]); base.mkdir(parents=True,exist_ok=True); results=[]; errors=[]
 for n,a in SCENARIOS.items():
  try: results.append(run_one(base,n,a))
  except Exception as e: errors.append({'scenario':n,'error':str(e)})
 summary=[]
 for r in results:
  rows=[x for x in r['rows'] if x.get('class')=='dungeon_gameplay']
  summary.append({'scenario':r['scenario'],'dungeon_frames':len(rows),'unique_dungeon_hashes':len(set(x.get('sha12') for x in rows)),'hashes':[x.get('sha12') for x in rows],'bboxes':[x.get('bbox') for x in rows]})
 (base/'pass151_results.json').write_text(json.dumps({'results':results,'errors':errors,'summary':summary},indent=2)+'\n')
 md=['# Pass 151 — 48ed extended control bbox probe','',f'- run base: `{base}`',f'- completed: {len(results)}',f'- errors: {len(errors)}','','## Dungeon-frame summary','']
 for s in summary: md.append(f"- `{s['scenario']}`: dungeon_frames={s['dungeon_frames']} unique_dungeon_hashes={s['unique_dungeon_hashes']} hashes={','.join(s['hashes'])} bboxes={s['bboxes']}")
 if errors: md += ['', '## Errors']+[f"- `{e['scenario']}`: {e['error']}" for e in errors]
 Path('parity-evidence/pass151_48ed_extended_control_bbox.md').write_text('\n'.join(md)+'\n')
if __name__=='__main__': main()
