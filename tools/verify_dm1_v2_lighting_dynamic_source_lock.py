#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
EVIDENCE = ROOT / 'parity-evidence/verification/dm1_v2_lighting_dynamic_source_lock.json'

REQUIRED_SOURCE = [
    (SOURCE / 'PANEL.C', 'F0337_INVENTORY_SetDungeonViewPalette', 336),
    (SOURCE / 'PANEL.C', 'Get torch light power from both hands of each champion in the party', 370),
    (SOURCE / 'PANEL.C', 'G0039_ai_Graphic562_LightPowerToLightAmount[*AL1040_pi_TorchLightPower]', 412),
    (SOURCE / 'PANEL.C', 'G0407_s_Party.MagicalLightAmount', 417),
    (SOURCE / 'PANEL.C', 'G0304_i_DungeonViewPaletteIndex = AL1039_ui_PaletteIndex', 428),
    (SOURCE / 'DATA.C', 'G0039_ai_Graphic562_LightPowerToLightAmount[16] = { 0, 5, 12, 24, 33, 40, 46, 51, 59, 68, 76, 82, 89, 94, 97, 100 }', 359),
]

REQUIRED_FIRESTAFF = [
    (ROOT / 'src/dm1v2/dm1_v2_lighting_dynamic_pc34.c', 'presentation-only'),
    (ROOT / 'src/dm1v2/dm1_v2_lighting_dynamic_pc34.c', 'PANEL.C:F0337_INVENTORY_SetDungeonViewPalette'),
    (ROOT / 'src/dm1v2/dm1_v2_lighting_dynamic_pc34.c', 'additive light map/fog overlay'),
    (ROOT / 'tests/test_dm1_v2_lighting_dynamic_pc34.c', 'half radius => squared falloff'),
    (ROOT / 'tests/test_dm1_v2_lighting_dynamic_pc34.c', 'additive overlay clamps'),
]

errors = []
anchors = []
for path, needle, line in REQUIRED_SOURCE:
    text = path.read_text(encoding='utf-8', errors='replace')
    lines = text.splitlines()
    if needle not in text:
        errors.append(f'missing {needle} in {path.name}')
    if not (1 <= line <= len(lines)):
        errors.append(f'line out of range {path.name}:{line}')
    else:
        anchors.append({'file': path.name, 'line': line, 'needle': needle, 'text': lines[line - 1].strip()})

for path, needle in REQUIRED_FIRESTAFF:
    text = path.read_text(encoding='utf-8', errors='replace')
    if needle not in text:
        errors.append(f'missing Firestaff lighting source-lock text {needle} in {path.name}')

result = {
    'status': 'failed' if errors else 'passed',
    'scope': 'dm1_v2_lighting_dynamic_pc34 presentation-only source-lock',
    'evidenceImpact': {
        'completionMatrixGap': 'Phase 3 lighting/fog gates were incomplete or unproven; this adds a first deterministic lighting CTest plus ReDMCSB source-reference gate, but does not prove the full Phase 3 viewport lane complete.',
        'verifiedCompletionPercent': None,
    },
    'anchors': anchors,
    'errors': errors,
}
EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + '\n', encoding='utf-8')
if errors:
    for e in errors:
        print('error:', e)
    raise SystemExit(1)
print(f'dm1_v2_lighting_dynamic_source_lock: ok evidence={EVIDENCE.relative_to(ROOT)}')
