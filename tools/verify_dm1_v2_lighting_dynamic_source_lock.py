#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
EVIDENCE = ROOT / 'parity-evidence/verification/dm1_v2_lighting_dynamic_source_lock.json'

REQUIRED_SOURCE = [
    (SOURCE / 'PANEL.C', 'F0337_INVENTORY_SetDungeonViewPalette', (329, 432)),
    (SOURCE / 'PANEL.C', 'Get torch light power from both hands of each champion in the party', (370, 387)),
    (SOURCE / 'PANEL.C', 'G0039_ai_Graphic562_LightPowerToLightAmount[*AL1040_pi_TorchLightPower]', (405, 417)),
    (SOURCE / 'PANEL.C', 'G0407_s_Party.MagicalLightAmount', (417, 417)),
    (SOURCE / 'PANEL.C', 'G0304_i_DungeonViewPaletteIndex = AL1039_ui_PaletteIndex', (418, 428)),
    (SOURCE / 'DATA.C', 'G0039_ai_Graphic562_LightPowerToLightAmount[16] = { 0, 5, 12, 24, 33, 40, 46, 51, 59, 68, 76, 82, 89, 94, 97, 100 }', (359, 360)),
]

REQUIRED_FIRESTAFF = [
    (ROOT / 'include/dm1_v2_lighting_dynamic_pc34.h', 'M11_V2_SourcePaletteLighting'),
    (ROOT / 'src/dm1v2/dm1_v2_lighting_dynamic_pc34.c', 'presentation-only'),
    (ROOT / 'src/dm1v2/dm1_v2_lighting_dynamic_pc34.c', 'PANEL.C:F0337_INVENTORY_SetDungeonViewPalette'),
    (ROOT / 'src/dm1v2/dm1_v2_lighting_dynamic_pc34.c', 'additive light map/fog overlay'),
    (ROOT / 'tests/test_dm1_v2_lighting_dynamic_pc34.c', 'half radius => squared falloff'),
    (ROOT / 'tests/test_dm1_v2_lighting_dynamic_pc34.c', 'additive overlay clamps'),
]

REQUIRED_EVIDENCE = [
    (ROOT / 'parity-evidence/verification/dm1_v2_phase4_lighting_palette_gate.md', 'PANEL.C:329-432'),
    (ROOT / 'parity-evidence/verification/dm1_v2_phase4_lighting_palette_gate.md', 'DATA.C:359-360'),
]

errors = []
anchors = []
for path, needle, line_range in REQUIRED_SOURCE:
    text = path.read_text(encoding='utf-8', errors='replace')
    lines = text.splitlines()
    if needle not in text:
        errors.append(f'missing {needle} in {path.name}')
    start, end = line_range
    if not (1 <= start <= end <= len(lines)):
        errors.append(f'line range out of range {path.name}:{start}-{end}')
    else:
        anchors.append({
            'file': path.name,
            'lineRange': f'{start}-{end}',
            'needle': needle,
            'text': lines[start - 1].strip(),
        })

for path, needle in REQUIRED_FIRESTAFF:
    text = path.read_text(encoding='utf-8', errors='replace')
    if needle not in text:
        errors.append(f'missing Firestaff lighting source-lock text {needle} in {path.name}')

for path, needle in REQUIRED_EVIDENCE:
    text = path.read_text(encoding='utf-8', errors='replace')
    if needle not in text:
        errors.append(f'missing evidence citation {needle} in {path.name}')

result = {
    'status': 'failed' if errors else 'passed',
    'scope': 'dm1_v2_lighting_dynamic_pc34 presentation-only source-lock',
    'evidenceImpact': {
        'completionMatrixGap': 'Phase 4 lighting/effects remains incomplete; this locks the V2 lighting palette presentation gate to ReDMCSB-selected DM1 V1 palette indices and deterministic invalid-input fallback only.',
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
