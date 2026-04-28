#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path
REPO=Path(__file__).resolve().parent.parent
atlas=REPO/'parity-evidence/pass143_frame48ed_control_delta_atlas.json'
out=REPO/'parity-evidence/pass145_follow_frame48ed_next_route.md'
if not atlas.exists():
    out.write_text('# Pass 145 — follow frame48ed next route\n\nBlocked: pass143 atlas is not present on this branch. Land pass143 first.\n')
    raise SystemExit(0)
data=json.loads(atlas.read_text())
focus=data.get('focus',[])
nonstatic=[]
static=[]
for r in focus:
    d=r.get('delta_from_prev') or {}
    ratio=float(d.get('changed_ratio') or 0)
    if r.get('sha12')=='48ed3743ab6a':
        (nonstatic if ratio>0.0005 else static).append((ratio,r))
lines=['# Pass 145 — follow frame48ed next route','',f"- source atlas: `{atlas}`",f"- focus rows: {len(focus)}",f"- 48ed static/near-static rows: {len(static)}",f"- 48ed non-static rows: {len(nonstatic)}",'','## Candidate follow-ups','']
if nonstatic:
    lines.append('Non-static 48ed rows exist; next dynamic route target should replay these exact scenarios with longer waits and movement/action sequences, capturing before/after bounding boxes.')
    for ratio,r in sorted(nonstatic, key=lambda x: x[0], reverse=True)[:20]:
        lines.append(f"- ratio={ratio:.6f} `{r.get('scenario')}` `{r.get('phase')} {r.get('value')}` file `{r.get('file')}` bbox `{(r.get('delta_from_prev') or {}).get('bbox')}`")
else:
    lines.append('All repeated 48ed rows are static/near-static. Treat 48ed as no-party/static placeholder, not party-control. Next route should pivot to pre-48ed title/menu/inventory frames or source-backed champion selection rather than more movement from 48ed.')
lines += ['', '## Specific next pass recommendation', '', '- If non-static rows exist: pass146 should replay the top non-static scenarios with longer waits and more movement/action inputs.', '- If all static: pass146 should pivot to source/asset-backed champion-selection/Hall-of-Champions route instead of direct dungeon movement.', '']
out.write_text('\n'.join(lines))
