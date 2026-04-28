#!/usr/bin/env python3
from __future__ import annotations
import json, os, shutil, subprocess, sys, time
from pathlib import Path
REPO=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(REPO))
from tools.pass118_state_aware_original_route_driver import wait_window, capture_new, classify_file, tap, click_original
from tools.pass80_original_frame_classifier import sha256
STAGE=Path.home()/'.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34'
DOSBOX='/usr/bin/dosbox'; PROGRAM='DM -vv -sn -pm'
BASE_ROUTE=[('key','Return'),('wait',1.4),('key','F1'),('wait',1.2)]
VARIANTS={
 'helper_tap_arrows':[('key','Up'),('wait',1.2),('key','Right'),('wait',1.2),('key','Left'),('wait',1.2)],
 'xdotool_window_key':[('xkey','Up'),('wait',1.2),('xkey','Right'),('wait',1.2),('xkey','Left'),('wait',1.2)],
 'xdotool_type_digits':[('xtype','4'),('wait',1.2),('xtype','8'),('wait',1.2),('xtype','6'),('wait',1.2)],
 'helper_click_panel':[('click',235,52),('wait',1.2),('click',276,158),('wait',1.2),('click',190,8),('wait',1.2)],
}
def conf(out:Path):
 p=out/'dosbox-pass153.conf'; p.write_text(f'''[sdl]\nfullscreen=false\noutput=opengl\n[dosbox]\nmachine=svga_paradise\nmemsize=4\ncaptures={out}\n[cpu]\ncore=normal\ncputype=386\ncpu_cycles=3000\n[render]\naspect=false\ninteger_scaling=false\n[mixer]\nnosound=true\n[speaker]\npcspeaker=false\ntandy=off\n[capture]\ncapture_dir={out}\ndefault_image_capture_formats=raw\n[autoexec]\nmount c "{STAGE}"\nc:\n{PROGRAM}\n'''); return p
def run(cmd, log, check=True):
 log.append('$ '+' '.join(map(str,cmd)))
 cp=subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
 log.append(cp.stdout.strip())
 if check and cp.returncode: raise RuntimeError(f'{cmd} exited {cp.returncode}: {cp.stdout[-400:]}')
 return cp.stdout
def win(log):
 w=wait_window(log,timeout=6.0)
 info=run(['xdotool','getwindowgeometry','--shell',str(w)],log,check=False)
 focus=run(['xdotool','getwindowfocus'],log,check=False)
 return str(w),info.strip(),focus.strip()
def shot(out,log,label,idx):
 raw=capture_new(wait_window(log,timeout=5.0),out,label,log)
 dst=out/f'image{idx:04d}-{label}.png'
 if dst.exists(): dst.unlink()
 shutil.move(str(raw),dst); cls,reason=classify_file(dst)
 return {'label':label,'file':dst.name,'sha12':sha256(dst)[:12],'class':cls,'reason':reason}
def do(out,log,a,idx):
 if a[0]=='wait': time.sleep(float(a[1])); return {'phase':'wait','seconds':a[1],**shot(out,log,f'wait_{idx}',idx)}
 w,geom,focus=win(log)
 if a[0]=='key': tap(w,a[1],log,delay=.9); return {'phase':'helper_key','value':a[1],'window':w,'focus':focus,**shot(out,log,f'helper_key_{a[1]}_{idx}',idx)}
 if a[0]=='xkey': run(['xdotool','windowactivate','--sync',w],log,check=False); run(['xdotool','key','--window',w,a[1]],log,check=False); time.sleep(.9); return {'phase':'xdotool_key','value':a[1],'window':w,'focus':focus,**shot(out,log,f'xdotool_key_{a[1]}_{idx}',idx)}
 if a[0]=='xtype': run(['xdotool','windowactivate','--sync',w],log,check=False); run(['xdotool','type','--window',w,a[1]],log,check=False); time.sleep(.9); return {'phase':'xdotool_type','value':a[1],'window':w,'focus':focus,**shot(out,log,f'xdotool_type_{a[1]}_{idx}',idx)}
 _,x,y=a; click_original(w,x,y,log,delay=.9); return {'phase':'helper_click','x':x,'y':y,'window':w,'focus':focus,**shot(out,log,f'click_{x}_{y}_{idx}',idx)}
def run_one(base,name,actions):
 out=base/name; out.mkdir(parents=True,exist_ok=True); log=[]; rows=[]; idx=1
 proc=subprocess.Popen([DOSBOX,'-conf',str(conf(out))],stdout=(out/'dosbox.log').open('w'),stderr=subprocess.STDOUT,text=True)
 try:
  win(log); time.sleep(7.0); rows.append({'phase':'initial',**shot(out,log,'initial',idx)}); idx+=1
  for a in BASE_ROUTE+actions:
   rows.append(do(out,log,a,idx)); idx+=1
 finally:
  try: proc.terminate(); proc.wait(timeout=2)
  except Exception: proc.kill()
 (out/'pass153_driver.log').write_text('\n'.join(log)+'\n')
 (out/'pass153_rows.json').write_text(json.dumps(rows,indent=2)+'\n')
 return {'variant':name,'rows':rows}
def main():
 base=Path(sys.argv[1]); base.mkdir(parents=True,exist_ok=True); results=[]; errors=[]
 for n,a in VARIANTS.items():
  try: results.append(run_one(base,n,a))
  except Exception as e: errors.append({'variant':n,'error':str(e)})
 summary=[]
 for r in results:
  ds=[x for x in r['rows'] if x.get('class')=='dungeon_gameplay']
  hashes=[x.get('sha12') for x in ds]
  summary.append({'variant':r['variant'],'dungeon_frames':len(ds),'unique_dungeon_hashes':len(set(hashes)),'hashes':hashes})
 (base/'pass153_results.json').write_text(json.dumps({'results':results,'errors':errors,'summary':summary},indent=2)+'\n')
 md=['# Pass 153 — Xvfb input delivery matrix for 48ed blocker','',f'- run base: `{base}`',f'- completed: {len(results)}',f'- errors: {len(errors)}','','## Summary','']
 for s in summary: md.append(f"- `{s['variant']}`: dungeon_frames={s['dungeon_frames']} unique_dungeon_hashes={s['unique_dungeon_hashes']} hashes={','.join(s['hashes'])}")
 if errors: md += ['', '## Errors']+[f"- `{e['variant']}`: {e['error']}" for e in errors]
 Path('parity-evidence/pass153_xvfb_input_delivery_matrix.md').write_text('\n'.join(md)+'\n')
 if errors: raise SystemExit(1)
 if not summary or not any(s['dungeon_frames'] for s in summary): raise SystemExit('no dungeon gameplay frames captured')
if __name__=='__main__': main()
