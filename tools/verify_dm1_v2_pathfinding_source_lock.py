#!/usr/bin/env python3
from pathlib import Path
import json

root = Path(__file__).resolve().parents[1]
source = Path.home() / '.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/GROUP.C'
reference = source.read_text(encoding='utf-8', errors='replace')
implementation = (root / 'src/dm1v2/dm1_v2_pathfinding_pc34.c').read_text()
errors = []

for marker in ['F0202_GROUP_IsMovementPossible', 'F0203_GROUP_GetFirstPossibleMovementDirectionOrdinal', 'T0209084_SingleSquareMoveTowardTargetSquare']:
    if marker not in reference:
        errors.append(f'GROUP.C {marker}')
for marker in ['g_open_list', 'find_lowest_f_node', 'int dirs[4][2]', 'out->steps']:
    if marker in implementation:
        errors.append(marker)

receipt = root / 'parity-evidence/verification/dm1_v2_pathfinding_source_lock.json'
receipt.write_text(json.dumps({
    'status': 'failed' if errors else 'passed',
    'redmcsbSource': str(source),
    'anchors': ['F0202_GROUP_IsMovementPossible', 'F0203_GROUP_GetFirstPossibleMovementDirectionOrdinal', 'T0209084_SingleSquareMoveTowardTargetSquare'],
    'errors': errors,
}, indent=2) + '\n')
if errors:
    raise SystemExit('\n'.join(errors))
print('dm1_v2_pathfinding_source_lock=OK')
