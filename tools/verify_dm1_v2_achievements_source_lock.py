#!/usr/bin/env python3
import json
from pathlib import Path
r=Path(__file__).resolve().parents[1]; s=Path.home()/'.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'; e=[]
for f,n in {'GAMELOOP.C':'F0444_STARTEND_Endgame','ENDGAME.C':'F0490_MEMORY_LoadDecompressAndExpandGraphic','CHAMPION.C':'F0047_TEXT_MESSAGEAREA_PrintMessage'}.items():
 if n not in (s/f).read_text(encoding='utf-8',errors='replace'): e.append(f'{f}: {n}')
for f,n in {'src/dm1v2/dm1_v2_achievements.c':['g_achievement_names','0xFF000000'],'src/dm1v2/dm1_v2_achievements_pc34.c':['g_achievements','g_notifications','fopen(']}.items():
 for x in n:
  if x in (r/f).read_text(): e.append(f'{f}: {x}')
o=r/'parity-evidence/verification/dm1_v2_achievements_source_lock.json'; o.write_text(json.dumps({'status':'failed' if e else 'passed','redmcsbSourceRoot':str(s),'errors':e},indent=2)+'\n')
if e: raise SystemExit('\n'.join(e))
print('dm1_v2_achievements_source_lock=OK')
