python3 - <<'PY'
import json, re
from pathlib import Path
root=Path('.')
data=json.loads(Path('asset_validator_checksums_m12.json').read_text())
entries=data['entries']
by={g:[e for e in entries if e.get('gameId')==g] for g in ('csb','dm2')}
print('source', data.get('source'))
for g, rows in by.items():
    filenames=sorted(set(e['filename'] for e in rows))
    graphics_hashes=[e['md5'] for e in rows if e['filename'].upper() in ('GRAPHICS.DAT','CSBGRAPH.DAT','DM2GRAPHICS.DAT','SKULLKEEP.GFX')]
    print(f'{g}: entries={len(rows)} filenames={filenames}')
    print(f'{g}: graphics_hashes={graphics_hashes}')
asset=Path('asset_status_m12.c').read_text()
for g in ('csb','dm2'):
    hashes=set(e['md5'] for e in by[g])
    block=re.search(rf'static const M12_VersionSpec g_{g}Versions\[\].*?\n}};', asset, re.S)
    found=re.findall(r'"([0-9a-f]{32})"', block.group(0) if block else '')
    print(f'{g}: hardcoded_hashes={found}')
    missing=[h for h in found if h not in hashes]
    print(f'{g}: hardcoded_hashes_in_json={not missing} missing={missing}')
    if missing: raise SystemExit(1)
print('required_file_count_model=one matched version spec per game; not paired graphics+dungeon')
PY
