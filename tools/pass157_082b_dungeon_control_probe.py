#!/usr/bin/env python3
from __future__ import annotations
import json, shutil, subprocess, sys, time
from pathlib import Path
from PIL import Image, ImageChops, ImageDraw

REPO=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(REPO))
from tools.pass118_state_aware_original_route_driver import wait_window,capture_new,classify_file,tap,click_original
from tools.pass80_original_frame_classifier import sha256

STAGE=Path.home()/'.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34'
DOSBOX='/usr/bin/dosbox'
PROGRAM='DM -vv -sn'
OUT_ROOT=Path('parity-evidence/verification/pass157_082b_dungeon_control_probe')
RUN_BASE=Path.home()/'.openclaw/data/firestaff-n2-runs'

SEED_ROUTE=[
  ('key','Return'),('wait',1.0),
  ('click',45,35),('wait',.7),
  ('click',95,35),('wait',.7),
  ('click',150,35),('wait',.7),
  ('click',220,35),('wait',.7),
  ('click',280,35),('wait',.7),
]

PROBES=[
  ('settle_baseline',('wait',.95)),
  ('key_up',('key','Up')),
  ('key_down',('key','Down')),
  ('key_left',('key','Left')),
  ('key_right',('key','Right')),
  ('key_w',('key','w')),
  ('key_a',('key','a')),
  ('key_s',('key','s')),
  ('key_d',('key','d')),
  ('key_space_use',('key','Space')),
  ('key_return_action',('key','Return')),
  ('click_forward_panel',('click',272,124)),
  ('click_left_panel',('click',236,148)),
  ('click_right_panel',('click',304,148)),
  ('click_back_panel',('click',272,164)),
]

CROPS={
  'viewport':(0,0,224,136),
  'right_panel':(224,0,320,136),
  'lower_panel':(0,136,320,200),
  'top_strip':(0,0,320,48),
  'movement_panel':(224,112,320,180),
}

def slug(s:str)->str:
  return ''.join(c.lower() if c.isalnum() else '_' for c in s).strip('_')

def conf(out:Path):
  p=out/'dosbox-pass157.conf'
  p.write_text(f'''[sdl]\nfullscreen=false\noutput=opengl\n[dosbox]\nmachine=svga_paradise\nmemsize=4\ncaptures={out}\n[cpu]\ncore=normal\ncputype=386\ncpu_cycles=3000\n[render]\naspect=false\ninteger_scaling=false\n[mixer]\nnosound=true\n[speaker]\npcspeaker=false\ntandy=off\n[capture]\ncapture_dir={out}\ndefault_image_capture_formats=raw\n[autoexec]\nmount c "{STAGE}"\nc:\n{PROGRAM}\n''')
  return p

def safe(log,label,fn):
  last=None
  for i in range(3):
    try: return fn()
    except Exception as e:
      last=e; log.append(f'retry {label} attempt={i+1}: {e}'); time.sleep(.25)
  raise RuntimeError(f'failed {label}: {last}')

def shot(out:Path,log,label:str,idx:int):
  raw=safe(log,f'capture-{label}',lambda:capture_new(wait_window(log,timeout=5.0),out,label,log))
  dst=out/f'image{idx:04d}-{label}.png'
  if dst.exists(): dst.unlink()
  shutil.move(str(raw),dst)
  cls,reason=classify_file(dst)
  return {'label':label,'file':dst.name,'path':str(dst),'sha12':sha256(dst)[:12],'class':cls,'reason':reason}

def do_action(out:Path,log,a,delay=.75):
  if a[0]=='wait': time.sleep(float(a[1])); return
  if a[0]=='key': safe(log,f'key-{a[1]}',lambda:tap(wait_window(log,timeout=5.0),a[1],log,delay=delay)); return
  _,x,y=a; safe(log,f'click-{x}-{y}',lambda:click_original(wait_window(log,timeout=5.0),x,y,log,delay=delay))

def annotate_and_crops(src:Path,dst_dir:Path,prefix:str):
  im=Image.open(src).convert('RGB')
  for name,box in CROPS.items(): im.crop(box).save(dst_dir/f'{prefix}_{name}.png')
  ann=im.copy(); dr=ImageDraw.Draw(ann)
  colors={'viewport':'red','right_panel':'yellow','lower_panel':'cyan','top_strip':'magenta','movement_panel':'lime'}
  for name,box in CROPS.items():
    dr.rectangle(box,outline=colors[name],width=2); dr.text((box[0]+2,box[1]+2),name,fill=colors[name])
  ann.save(dst_dir/f'{prefix}_annotated_bbox.png')

def diff_stats(before:Path,after:Path,dst:Path):
  a=Image.open(before).convert('RGB'); b=Image.open(after).convert('RGB')
  d=ImageChops.difference(a,b)
  bbox=d.getbbox(); nonzero=0
  if bbox:
    pix=d.load(); w,h=d.size
    for y in range(h):
      for x in range(w):
        if pix[x,y]!=(0,0,0): nonzero+=1
    d.save(dst/'diff.png')
    ann=b.copy(); dr=ImageDraw.Draw(ann); dr.rectangle(bbox,outline='white',width=2); ann.save(dst/'after_diff_bbox.png')
  return {'bbox':bbox,'changed_pixels':nonzero,'changed_ratio':round(nonzero/(320*200),6)}

def classify_result(before,after,diff):
  if after.get('class') in {'title_logo','startup_menu','entrance_menu'}:
    return 'menu/title/overlay'
  if after.get('class') in {'non_graphics_blocker'} or after.get('class') is None:
    return 'blocker/unknown'
  if before['sha12']==after['sha12'] or diff.get('changed_pixels',0)==0:
    return 'ignored/no-op'
  if before.get('class')=='dungeon_gameplay' and after.get('class')=='dungeon_gameplay':
    return 'accepted movement/control'
  if after.get('class') in {'dungeon_gameplay','graphics_320x200_unclassified','inventory','spell_panel'}:
    return 'accepted movement/control'
  return 'blocker/unknown'

def run_probe(run_base:Path,label:str,action):
  out=run_base/slug(label); out.mkdir(parents=True,exist_ok=True)
  log=[]; idx=1
  proc=subprocess.Popen([DOSBOX,'-conf',str(conf(out))],stdout=(out/'dosbox.log').open('w'),stderr=subprocess.STDOUT,text=True)
  try:
    wait_window(log,timeout=6.0); time.sleep(6.5)
    rows=[{'phase':'initial',**shot(out,log,'initial',idx)}]; idx+=1
    for a in SEED_ROUTE:
      do_action(out,log,a,delay=.45)
      rows.append({'phase':a[0],'value':a[1:] if len(a)>1 else None,**shot(out,log,f'seed_{idx}',idx)})
      idx+=1
    before=rows[-1].copy(); before['label']='seed_wait_13_before_control'
    do_action(out,log,action,delay=.95)
    after=shot(out,log,'after_control',idx); idx+=1
    time.sleep(.7)
    settled=shot(out,log,'after_settle',idx)
    rows += [{'phase':'before_control',**before},{'phase':'probe','action':action,**after},{'phase':'after_settle',**settled}]
  finally:
    try: proc.terminate(); proc.wait(timeout=2)
    except Exception: proc.kill()
    (out/'pass157_driver.log').write_text('\n'.join(log)+'\n')
  (out/'pass157_rows.json').write_text(json.dumps(rows,indent=2)+'\n')
  result=settled if settled['sha12']!=before['sha12'] else after
  d=diff_stats(Path(before['path']),Path(result['path']),out)
  annotate_and_crops(Path(before['path']),out,'before')
  annotate_and_crops(Path(result['path']),out,'result')
  summary={'label':label,'action':action,'before':before,'after':after,'settled':settled,'result':result,'diff':d}
  summary['classification']=classify_result(before,result,d)
  summary['evidence_dir']=str(out)
  (out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
  return summary

def main():
  OUT_ROOT.mkdir(parents=True,exist_ok=True)
  run_base=RUN_BASE/(time.strftime('%Y%m%d-%H%M%S')+'-pass157-082b-dungeon-control-probe')
  run_base.mkdir(parents=True,exist_ok=True)
  results=[]; errors=[]
  for label,action in PROBES:
    try:
      res=run_probe(run_base,label,action)
      ev=OUT_ROOT/slug(label)
      if ev.exists(): shutil.rmtree(ev)
      shutil.copytree(Path(res['evidence_dir']),ev)
      res['evidence_dir']=str(ev)
      results.append(res)
    except Exception as e:
      errors.append({'label':label,'action':action,'error':str(e)})
  baseline=next((r for r in results if r['label']=='settle_baseline'),None)
  baseline_sha=baseline['result']['sha12'] if baseline else None
  if baseline_sha:
    for r in results:
      if r['label']!='settle_baseline' and r['result']['sha12']==baseline_sha:
        r['classification']='ignored/no-op'
        Path(r['evidence_dir'],'summary.json').write_text(json.dumps(r,indent=2)+'\n')
  manifest={'program':PROGRAM,'seed':'pass155/pass156 DM -vv -sn pre_dungeon_click_grid wait_13 / 082b4d249740','run_base':str(run_base),'evidence_root':str(OUT_ROOT),'results':results,'errors':errors}
  (OUT_ROOT/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
  buckets={}
  for r in results: buckets[r['classification']]=buckets.get(r['classification'],0)+1
  md=['# Pass 157 — 082b dungeon control probe','',f'- program: `{PROGRAM}`','- seed route: `pre_dungeon_click_grid` through `wait_13` (pass155 hash `082b4d249740`)',f'- run base: `{run_base}`',f'- evidence root: `{OUT_ROOT}`',f'- probes: {len(PROBES)} (includes settle_baseline)',f'- completed: {len(results)}',f'- errors: {len(errors)}',f"- buckets: {', '.join(f'{k}={v}' for k,v in sorted(buckets.items()))}",'','## Action matrix','']
  for r in results:
    b=r['before']; res=r['result']; diff=r['diff']; action=' '.join(map(str,r['action']))
    md.append(f"- `{r['label']}` `{action}`: **{r['classification']}** `{b['sha12']}`/{b['class']} -> `{res['sha12']}`/{res['class']} changed_pixels={diff['changed_pixels']} bbox={diff['bbox']} — `{r['evidence_dir']}`")
  if errors:
    md += ['', '## Errors', '']
    for e in errors: md.append(f"- `{e['label']}` `{e['action']}`: {e['error']}")
  md += ['', '## Evidence contents', '', 'Each action directory contains raw route/control frames, `summary.json`, `pass157_rows.json`, `before_*.png`/`result_*.png` crops, annotated bboxes, and `diff.png`/`after_diff_bbox.png` when pixels changed.']
  (OUT_ROOT/'README.md').write_text('\n'.join(md)+'\n')
  print(f'wrote {OUT_ROOT}/README.md')
  print(f'run_base={run_base}')
  print(f'completed={len(results)} errors={len(errors)}')
  return 1 if errors else 0
if __name__=='__main__': raise SystemExit(main())
