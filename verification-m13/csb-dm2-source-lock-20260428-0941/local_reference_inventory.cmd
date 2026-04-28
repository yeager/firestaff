python3 - <<'PY'
from pathlib import Path
import hashlib
base=Path.home()/'.openclaw/data/firestaff-original-games/DM'
refs=[
 base/'Game,Chaos_Strikes_Back,Atari_ST,Software.7z',
 base/'Game,Chaos_Strikes_Back,Amiga,Software.7z',
 base/'Game,Dungeon_Master_II,DOS,Source,Disassembly,Software.7z',
 base/'Game,Dungeon_Master_II,US,DOS,HMI_Update,Software.7z',
 base/'Game,Dungeon_Master_II,US,DOS,Patch,Software.7z',
 base/'Dungeon-Master-II-Skullkeep_DOS_EN.zip',
 base/'Dungeon_Master_II_-_The_Legend_of_Skullkeep_1994.zip',
 base/'Dungeon Master 2.zip',
 base/'_extracted/dm2-dos-asm/SKULL.ASM',
]
for p in refs:
    if p.exists():
        h=hashlib.sha256(p.read_bytes()).hexdigest()
        print(f'FOUND {p} size={p.stat().st_size} sha256={h}')
    else:
        print(f'MISSING {p}')
red=Path.home()/'.openclaw/data/firestaff-redmcsb-source'
for p in [red/'README_FIRESTAFF.md', red/'dmweb.free.fr_Stuff_ReDMCSB_Documentation_BugsAndChanges.htm.html', red/'local-candidates.txt']:
    print(f"REDMCSB_REF {'FOUND' if p.exists() else 'MISSING'} {p}")
PY
