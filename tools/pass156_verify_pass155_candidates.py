#!/usr/bin/env python3
from __future__ import annotations
import json, shutil, sys, time
from pathlib import Path
from PIL import Image, ImageDraw
REPO=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(REPO))
from tools.pass155_champion_route_seed_finder import run_one, SCENARIOS
from tools.pass80_original_frame_classifier import sha256
from tools.pass118_state_aware_original_route_driver import classify_file

PASS155_JSON=Path('<N2_RUNS>/20260428-205557-pass155-champion-route-seed-finder/pass155_results.json')
OUT_ROOT=Path('parity-evidence/verification/pass156_pass155_candidate_replay')
CROPS={
  'viewport': (0,0,224,136),
  'right_panel': (224,0,320,136),
  'lower_panel': (0,136,320,200),
  'top_strip': (0,0,320,48),
}
FOCUS_HASHES={'082b4d249740'}

def load_candidates():
    d=json.loads(PASS155_JSON.read_text())
    out=[]; seen=set(); unclassified=0
    for r in d['top_rows']:
        keep = r.get('sha12') in FOCUS_HASHES or r.get('class')=='graphics_320x200_unclassified' or (r.get('scenario')=='pre_dungeon_click_grid' and r.get('program')=='DM -vv -sn' and r.get('sha12') in {'014ed52c71a0','17bd7e878157'})
        if not keep: continue
        key=(r['program'],r['scenario'],r['label'],r['sha12'])
        if key in seen: continue
        seen.add(key); out.append(r)
        if r.get('class')=='graphics_320x200_unclassified': unclassified+=1
        if unclassified>=24 and '082b4d249740' in {x.get('sha12') for x in out}: break
    return out

def slug(s): return ''.join(c.lower() if c.isalnum() else '_' for c in s).strip('_')

def annotate_and_crop(src:Path, dst_dir:Path):
    im=Image.open(src).convert('RGB')
    for name,box in CROPS.items(): im.crop(box).save(dst_dir/f'{name}.png')
    ann=im.copy(); dr=ImageDraw.Draw(ann)
    colors={'viewport':'red','right_panel':'yellow','lower_panel':'cyan','top_strip':'magenta'}
    for name,box in CROPS.items():
        dr.rectangle(box, outline=colors[name], width=2)
        dr.text((box[0]+2,box[1]+2), name, fill=colors[name])
    ann.save(dst_dir/'annotated_bbox.png')

def manifest_row(candidate, ev_dir):
    frame=ev_dir/'frame.png'; cls,reason=classify_file(frame)
    return {'program':candidate['program'],'scenario':candidate['scenario'],'phase':candidate.get('phase'),'value':candidate.get('value'),'x':candidate.get('x'),'y':candidate.get('y'),'label':candidate['label'],'pass155_sha12':candidate['sha12'],'replay_sha12':sha256(frame)[:12],'pass155_class':candidate.get('class'),'replay_class':cls,'reason':reason,'evidence_dir':str(ev_dir),'frame':str(frame),'crops':{k:str(ev_dir/f'{k}.png') for k in CROPS},'annotated_bbox':str(ev_dir/'annotated_bbox.png')}

def main():
    candidates=load_candidates(); OUT_ROOT.mkdir(parents=True, exist_ok=True)
    (OUT_ROOT/'candidate_input.json').write_text(json.dumps(candidates,indent=2)+'\n')
    grouped={}
    for c in candidates: grouped.setdefault((c['program'],c['scenario']),[]).append(c)
    manifest=[]; errors=[]
    run_base=Path('<N2_RUNS>')/(time.strftime('%Y%m%d-%H%M%S')+'-pass156-pass155-candidate-replay')
    for (program,scenario), cs in grouped.items():
        try:
            rr=run_one(run_base,program,scenario,SCENARIOS[scenario])
            rows_by_label={r['label']:r for r in rr['rows']}
            replay_dir=run_base/(program.replace(' ','_').replace('-','').replace('/','_')+'__'+scenario)[:110]
            for c in cs:
                row=rows_by_label.get(c['label'])
                if not row: errors.append({'candidate':c,'error':'label not replayed'}); continue
                src=replay_dir/row['file']
                ev_dir=OUT_ROOT/f"{slug(program)}__{scenario}__{c['label']}__{c['sha12']}"
                ev_dir.mkdir(parents=True, exist_ok=True)
                shutil.copy2(src, ev_dir/'frame.png')
                (ev_dir/'source.txt').write_text(f"replay_run={run_base}\nreplay_source={src}\npass155_file={c.get('file')}\n")
                annotate_and_crop(ev_dir/'frame.png', ev_dir)
                mr=manifest_row(c,ev_dir); mr['replay_source']=str(src); mr['replay_run']=str(run_base)
                manifest.append(mr)
        except Exception as e:
            errors.append({'program':program,'scenario':scenario,'error':str(e)})
    (OUT_ROOT/'manifest.json').write_text(json.dumps({'run_base':str(run_base),'rows':manifest,'errors':errors},indent=2)+'\n')
    md=['# Pass 156 — pass155 candidate replay/crop verification','',f'- pass155 source: `{PASS155_JSON}`',f'- replay run base: `{run_base}`',f'- evidence root: `{OUT_ROOT}`',f'- candidates: {len(candidates)}',f'- captured: {len(manifest)}',f'- errors: {len(errors)}','','## Replayed candidates','']
    for m in manifest:
        status='MATCH' if m['pass155_sha12']==m['replay_sha12'] else 'DRIFT'
        coord=(m.get('x'),m.get('y')) if m.get('x') is not None else ''
        md.append(f"- {status} `{m['program']}` `{m['scenario']}` {m['phase']} {m.get('value') or coord}: pass155={m['pass155_sha12']} replay={m['replay_sha12']} classifier=`{m['replay_class']}` — `{m['evidence_dir']}`")
    if errors: md += ['','## Errors']+[f"- `{e}`" for e in errors]
    (OUT_ROOT/'README.md').write_text('\n'.join(md)+'\n')
    print(f'wrote {OUT_ROOT}/README.md'); print(f'run_base={run_base}'); print(f'captured={len(manifest)} errors={len(errors)}')
    return 1 if errors else 0
if __name__=='__main__': raise SystemExit(main())
