#!/usr/bin/env python3
from __future__ import annotations
import json, sys
from pathlib import Path
from PIL import Image, ImageChops, ImageStat
REPO=Path(__file__).resolve().parent.parent
RUN141=Path.home()/".openclaw/data/firestaff-n2-runs/20260428-192537-pass141-pm-f1-dungeon-control-readiness"

def stats(img):
    rgb=img.convert('RGB')
    st=ImageStat.Stat(rgb)
    px=list(rgb.getdata())
    nb=sum(1 for p in px if sum(p)>12)/len(px)
    return {"mean":[round(x,2) for x in st.mean],"nonblack":round(nb,5)}

def diffstats(a,b):
    d=ImageChops.difference(a.convert('RGB'),b.convert('RGB'))
    px=list(d.getdata())
    changed=sum(1 for p in px if sum(p)>10)
    bbox=d.getbbox()
    return {"changed_pixels":changed,"changed_ratio":round(changed/len(px),6),"bbox":bbox,"mean_delta":[round(x,3) for x in ImageStat.Stat(d).mean]}

def main():
    run=Path(sys.argv[1]) if len(sys.argv)>1 else RUN141
    rows=[]; bysha={}
    for jf in sorted(run.glob('*/pass141_rows.json')):
        scenario=jf.parent.name
        data=json.loads(jf.read_text())
        prev=None
        for r in data:
            img=Image.open(jf.parent/r['file'])
            rec={"scenario":scenario,"phase":r.get('phase'),"value":r.get('value') or r.get('label') or (f"{r.get('x')},{r.get('y')}" if r.get('x') is not None else ''),"sha12":r.get('sha12'),"class":r.get('class'),"file":str((jf.parent/r['file']).relative_to(run)),"stats":stats(img)}
            if prev:
                rec['delta_from_prev']=diffstats(prev[1],img)
            rows.append(rec)
            bysha.setdefault(r.get('sha12'),[]).append(rec)
            prev=(r,img)
    # Summarize all 48ed and adjacent movement rows; 48ed is the repeated dungeon hash from pass139/141.
    focus=[]
    for i,r in enumerate(rows):
        if r['sha12']=='48ed3743ab6a' or r['value'] in {'Up','Down','Left','Right'} or r['class']=='dungeon_gameplay':
            focus.append(r)
    out_json=REPO/'parity-evidence/pass143_frame48ed_control_delta_atlas.json'
    out_md=REPO/'parity-evidence/pass143_frame48ed_control_delta_atlas.md'
    out_json.write_text(json.dumps({"schema":"pass143_frame48ed_control_delta_atlas.v1","run":str(run),"rows":rows,"focus":focus,"hash_counts":{k:len(v) for k,v in sorted(bysha.items())}},indent=2)+"\n")
    md=["# Pass 143 — frame 48ed control delta atlas","",f"- source run: `{run}`","- purpose: test whether the repeated `48ed3743ab6a` dungeon frame hides real control changes or is a static no-party/start frame.",f"- total rows: {len(rows)}",f"- unique hashes: {len(bysha)}", "", "## Hash counts", ""]
    for sha,items in sorted(bysha.items(), key=lambda kv:(-len(kv[1]),kv[0])):
        md.append(f"- `{sha}`: {len(items)} rows; classes={','.join(sorted({str(x['class']) for x in items}))}")
    md += ["", "## Focus rows", "", "| scenario | input | sha12 | class | changed_ratio | bbox | file |", "|---|---|---|---|---:|---|---|"]
    for r in focus:
        d=r.get('delta_from_prev',{})
        md.append(f"| `{r['scenario']}` | `{r['phase']} {r['value']}` | `{r['sha12']}` | `{r['class']}` | {d.get('changed_ratio','')} | `{d.get('bbox','')}` | `{r['file']}` |")
    md += ["", "## Interpretation", "", "A route is only a party-control candidate if movement inputs produce distinct dungeon frames or a non-trivial visual delta. Repeated `48ed3743ab6a` with zero/near-zero deltas means this is likely a static no-party/dungeon placeholder rather than real champion control; non-zero deltas or new dungeon hashes should become the next route target.", ""]
    out_md.write_text("\n".join(md))
if __name__=='__main__': main()
