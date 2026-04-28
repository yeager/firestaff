#!/usr/bin/env python3
from __future__ import annotations
import json, shutil, subprocess, sys, time
from pathlib import Path
REPO=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(REPO))
from tools.pass118_state_aware_original_route_driver import wait_window,capture_new,classify_file,tap,click_original
from tools.pass80_original_frame_classifier import sha256
STAGE=Path.home()/'.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34'
DOSBOX='/usr/bin/dosbox'
PROGRAMS=['DM -vv -sn -pm','DM -vv -sn','DM -vv -pm','DM']
# Deliberately probes menus/title/roster-like keys/clicks BEFORE accepting 48ed as route end.
SCENARIOS={
 'pm_enter_f1_then_function_keys':[('key','Return'),('wait',1.0),('key','F1'),('wait',.8),('key','F2'),('wait',.8),('key','F3'),('wait',.8),('key','F4'),('wait',.8),('key','F5'),('wait',.8),('key','F6'),('wait',.8)],
 'pm_enter_f2_function_keys':[('key','Return'),('wait',1.0),('key','F2'),('wait',.8),('key','F1'),('wait',.8),('key','F3'),('wait',.8),('key','F4'),('wait',.8)],
 'menu_numeric_roster':[('key','Return'),('wait',1.0),('key','1'),('wait',.7),('key','2'),('wait',.7),('key','3'),('wait',.7),('key','4'),('wait',.7),('key','5'),('wait',.7),('key','6'),('wait',.7)],
 'pre_dungeon_click_grid':[('key','Return'),('wait',1.0),('click',45,35),('wait',.7),('click',95,35),('wait',.7),('click',150,35),('wait',.7),('click',220,35),('wait',.7),('click',280,35),('wait',.7),('click',90,100),('wait',.7),('click',220,120),('wait',.7)],
 'champion_name_keys':[('key','Return'),('wait',1.0),('key','c'),('wait',.7),('key','C'),('wait',.7),('key','p'),('wait',.7),('key','i'),('wait',.7),('key','Escape'),('wait',.7),('key','Return'),('wait',.7)],
}
def conf(out:Path,program:str):
 p=out/'dosbox-pass155.conf'; p.write_text(f'''[sdl]\nfullscreen=false\noutput=opengl\n[dosbox]\nmachine=svga_paradise\nmemsize=4\ncaptures={out}\n[cpu]\ncore=normal\ncputype=386\ncpu_cycles=3000\n[render]\naspect=false\ninteger_scaling=false\n[mixer]\nnosound=true\n[speaker]\npcspeaker=false\ntandy=off\n[capture]\ncapture_dir={out}\ndefault_image_capture_formats=raw\n[autoexec]\nmount c "{STAGE}"\nc:\n{program}\n'''); return p
def run(cmd,log,check=True):
 log.append('$ '+' '.join(map(str,cmd)))
 cp=subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
 log.append(cp.stdout.strip())
 if check and cp.returncode: raise RuntimeError(cp.stdout[-600:])
 return cp.stdout
def shot(out,log,label,idx):
 raw=capture_new(wait_window(log,timeout=5.0),out,label,log)
 dst=out/f'image{idx:04d}-{label}.png'
 if dst.exists(): dst.unlink()
 shutil.move(str(raw),dst)
 cls,reason=classify_file(dst)
 return {'label':label,'file':dst.name,'sha12':sha256(dst)[:12],'class':cls,'reason':reason}
def act(out,log,a,idx):
 if a[0]=='wait': time.sleep(float(a[1])); return {'phase':'wait',**shot(out,log,f'wait_{idx}',idx)}
 w=wait_window(log,timeout=5.0)
 if a[0]=='key': tap(w,a[1],log,delay=.45); return {'phase':'key','value':a[1],**shot(out,log,f'key_{a[1]}_{idx}',idx)}
 _,x,y=a; click_original(w,x,y,log,delay=.45); return {'phase':'click','x':x,'y':y,**shot(out,log,f'click_{x}_{y}_{idx}',idx)}
def run_one(base,program,scenario,actions):
 name=(program.replace(' ','_').replace('-','').replace('/','_')+'__'+scenario)[:110]
 out=base/name; out.mkdir(parents=True,exist_ok=True); log=[]; rows=[]; idx=1
 proc=subprocess.Popen([DOSBOX,'-conf',str(conf(out,program))],stdout=(out/'dosbox.log').open('w'),stderr=subprocess.STDOUT,text=True)
 try:
  wait_window(log,timeout=6.0); time.sleep(6.5); rows.append({'phase':'initial',**shot(out,log,'initial',idx)}); idx+=1
  for a in actions: rows.append(act(out,log,a,idx)); idx+=1
 finally:
  try: proc.terminate(); proc.wait(timeout=2)
  except Exception: proc.kill()
 (out/'pass155_driver.log').write_text('\n'.join(log)+'\n')
 (out/'pass155_rows.json').write_text(json.dumps(rows,indent=2)+'\n')
 return {'program':program,'scenario':scenario,'rows':rows}
def score(row):
 cls=row.get('class'); h=row.get('sha12')
 if h=='48ed3743ab6a': return 0
 if cls in {'spell_panel','inventory'}: return 100
 if cls=='graphics_320x200_unclassified': return 40
 if cls=='entrance_menu': return 15
 if cls=='dungeon_gameplay': return 20
 return 10
def main():
 base=Path(sys.argv[1]); base.mkdir(parents=True,exist_ok=True); results=[]; errors=[]
 for program in PROGRAMS:
  for scenario,actions in SCENARIOS.items():
   try: results.append(run_one(base,program,scenario,actions))
   except Exception as e: errors.append({'program':program,'scenario':scenario,'error':str(e)})
 rows=[]
 for r in results:
  for row in r['rows']:
   if row.get('sha12')!='48ed3743ab6a': rows.append({'score':score(row),'program':r['program'],'scenario':r['scenario'],**row})
 rows=sorted(rows,key=lambda x:(-x['score'],x.get('class',''),x.get('sha12','')))[:80]
 summary=[]
 for r in results:
  hashes=[x.get('sha12') for x in r['rows']]
  non48=[x for x in r['rows'] if x.get('sha12')!='48ed3743ab6a']
  summary.append({'program':r['program'],'scenario':r['scenario'],'frames':len(r['rows']),'unique_hashes':len(set(hashes)),'non48ed_frames':len(non48),'classes':sorted(set(x.get('class') for x in r['rows']))})
 (base/'pass155_results.json').write_text(json.dumps({'summary':summary,'top_rows':rows,'errors':errors},indent=2)+'\n')
 md=['# Pass 155 — champion/control route seed finder','','- purpose: pivot away from static 48ed and identify non-48ed route seeds for champion/party-control capture.',f'- run base: `{base}`',f'- completed scenarios: {len(results)}',f'- errors: {len(errors)}','','## Top non-48ed seeds','']
 for r in rows[:30]: md.append(f"- score={r['score']} `{r['program']}` `{r['scenario']}` {r.get('phase')} {r.get('value', '') or (str((r.get('x'),r.get('y'))) if 'x' in r else '')}: {r.get('sha12')} `{r.get('class')}` — {r.get('reason')}")
 md += ['','## Scenario summary','']
 for s in summary: md.append(f"- `{s['program']}` `{s['scenario']}`: unique_hashes={s['unique_hashes']} non48ed={s['non48ed_frames']} classes={','.join(s['classes'])}")
 if errors: md += ['','## Errors']+[f"- `{e['program']}` `{e['scenario']}`: {e['error']}" for e in errors]
 Path('parity-evidence/pass155_champion_route_seed_finder.md').write_text('\n'.join(md)+'\n')
 if errors and len(results)==0: raise SystemExit(1)
 if not rows: raise SystemExit('no non-48ed seeds found')
if __name__=='__main__': main()
