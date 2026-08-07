#!/usr/bin/env python3
import json
from pathlib import Path
root=Path(__file__).resolve().parents[1]
src=Path.home()/'.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'
out=root/'parity-evidence/verification/dm1_v2_host_state_extensions_source_lock.json'
checks={'COMMAND.C':['F0366_COMMAND_ProcessTypes3To6_MoveParty','C140_COMMAND_SAVE_GAME','F0355_INVENTORY_Toggle_CPSE'],'LOADSAVE.C':['F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF','F0435_STARTEND_LoadGame'],'PANEL.C':['F0355_INVENTORY_Toggle_CPSE']}
errors=[]
for name,needles in checks.items():
    text=(src/name).read_text(encoding='utf-8',errors='replace') if (src/name).exists() else ''
    for needle in needles:
        if needle not in text: errors.append(f'{name}: {needle}')
for rel,forbidden in {'src/dm1v2/dm1_v2_input_remap_pc34.c':['g_v2_bindings','fopen('],'src/dm1v2/dm1_v2_auto_save_pc34.c':['g_autosave','autosave_slot'],'src/dm1v2/dm1_v2_inventory_sort_pc34.c':['g_inv','s_actual_count','strncpy(']}.items():
    text=(root/rel).read_text()
    for needle in forbidden:
        if needle in text: errors.append(f'{rel}: {needle}')
result={'status':'failed' if errors else 'passed','scope':'DM1 V2 host-owned input, autosave and inventory state','redmcsbSourceRoot':str(src),'anchors':checks,'errors':errors}
out.parent.mkdir(parents=True,exist_ok=True); out.write_text(json.dumps(result,indent=2,sort_keys=True)+'\n')
if errors: raise SystemExit('\n'.join(errors))
print(f'dm1_v2_host_state_extensions_source_lock=OK evidence={out.relative_to(root)}')
