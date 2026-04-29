#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path
REPO=Path(__file__).resolve().parent.parent
P168=REPO/'parity-evidence/verification/pass168_redmcsb_pass_source_map/manifest.json'
P169=REPO/'parity-evidence/verification/pass169_redmcsb_anchor_gap_resolution/manifest.json'
P170=REPO/'parity-evidence/verification/pass170_source_mentioned_unresolved_batch/manifest.json'
OUT=REPO/'parity-evidence/verification/pass171_redmcsb_source_lock_closure'
def main():
 p168=json.load(open(P168)); p169=json.load(open(P169)); p170=json.load(open(P170))
 counts=p168.get('status_counts',{})
 direct=counts.get('source-locked',0)
 gap=p169['resolved'] if 'resolved' in p169 else p169.get('source_locked',0)
 mentioned=p170['source_locked']
 total=p168.get('passes') or sum(counts.values())
 closed=direct+gap+mentioned
 status='closed' if closed==total else 'open'
 OUT.mkdir(parents=True,exist_ok=True)
 summary={'schema':'pass171_redmcsb_source_lock_closure.v1','status':status,'pass168_total':total,'pass168_direct_source_locked':direct,'pass169_needs_anchor_resolved':gap,'pass170_source_mentioned_resolved':mentioned,'closed':closed,'remaining':total-closed,'inputs':[str(P168),str(P169),str(P170)]}
 (OUT/'manifest.json').write_text(json.dumps(summary,indent=2)+'\n')
 lines=['# Pass171 — ReDMCSB source-lock closure','','## Result','',f"- status: **{status}**",f"- pass168 total pass groups: **{total}**",f"- pass168 directly source-locked: **{direct}**",f"- pass169 resolved `needs-redmcsb-anchor`: **{gap}**",f"- pass170 resolved `source-mentioned-unresolved`: **{mentioned}**",f"- closed: **{closed}/{total}**",f"- remaining: **{total-closed}**",'','## Inputs','',f'- `{P168.relative_to(REPO)}`',f'- `{P169.relative_to(REPO)}`',f'- `{P170.relative_to(REPO)}`','','## Interpretation','', f'The full historical pass corpus covered by pass168 now has a ReDMCSB source-lock path: direct anchors for {direct} groups, explicit addendum anchors for the {gap} weak/missing groups, and lane-level source anchors for the {mentioned} source-mentioned unresolved groups.']
 (OUT/'README.md').write_text('\n'.join(lines)+'\n')
 print(json.dumps(summary,indent=2))
 return 0 if status=='closed' else 1
if __name__=='__main__': raise SystemExit(main())
