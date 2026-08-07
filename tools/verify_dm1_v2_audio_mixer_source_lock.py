#!/usr/bin/env python3
from pathlib import Path
import json
r=Path(__file__).resolve().parents[1]; s=Path.home()/'.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/SOUND.C'; t=s.read_text(encoding='utf-8',errors='replace'); e=[]
if 'F0064_SOUND_RequestPlay_CPSD' not in t:e.append('SOUND.C F0064')
for x in ['g_mixer','left_acc','right_acc','memset(buf,']:
 if x in (r/'src/dm1v2/dm1_v2_audio_mixer_pc34.c').read_text():e.append(x)
o=r/'parity-evidence/verification/dm1_v2_audio_mixer_source_lock.json';o.write_text(json.dumps({'status':'failed' if e else 'passed','redmcsbSource':str(s),'errors':e},indent=2)+'\n')
if e:raise SystemExit('\n'.join(e))
print('dm1_v2_audio_mixer_source_lock=OK')
