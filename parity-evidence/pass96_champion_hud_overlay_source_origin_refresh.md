# Pass 96 — champion HUD overlay source-origin refresh

Scope: V1 DM1 original-faithful champion top row evidence only.

## Finding

`tools/pass83_champion_hud_zone_overlay_probe.py` was still generating champion-HUD overlays at Firestaff's legacy party-panel inset `x=12`, even after pass90 moved the V1 status-box origin to the DM1 PC 3.4 source origin `x=0`.

Source-backed geometry used here:

- layout-696 `C151..C154`: four 67×29 champion status boxes at x offsets `0, 69, 138, 207`
- layout-696 `C159..C166`: name clear/text zones relative to each status box
- layout-696 `C195..C206`: HP/stamina/mana 4×25 vertical bar containers relative to each status box
- layout-696 `C211..C218`: ready/action hand zones relative to each status box
- pass90/code anchor: `M11_V1_PARTY_PANEL_X = 0`; the old `M11_PARTY_PANEL_X = 12` remains V2-only chrome

## Change

Refreshed the pass83 evidence generator and regenerated its JSON/PNG/Markdown evidence so source-zone overlays now match the active V1 source-origin top-row geometry.

This is evidence only. It does not claim pixel-perfect original-runtime parity.

## Gates

```sh
python3 -m py_compile tools/pass83_champion_hud_zone_overlay_probe.py
python3 tools/pass83_champion_hud_zone_overlay_probe.py --self-test
python3 tools/pass83_champion_hud_zone_overlay_probe.py
python3 - <<'PY'
import json
from pathlib import Path
p=Path('parity-evidence/overlays/pass83/pass83_champion_hud_zone_overlay_stats.json')
data=json.loads(p.read_text())
assert data['pass'] is True
assert data['schema'].endswith('.v2')
assert data['zones'][0]['xywh']==[0,0,67,29]
assert data['zones'][24]['xywh']==[207,0,67,29]
print({'json': str(p), 'scenes': len(data['scenes']), 'zones': len(data['zones']), 'pass': data['pass']})
PY
git diff --check
```
