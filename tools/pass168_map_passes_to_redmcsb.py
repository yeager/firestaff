#!/usr/bin/env python3
"""Pass168: map Firestaff pass artifacts to ReDMCSB source anchors.

Scans pass tools/evidence, extracts explicit source-file/function/constant anchors,
resolves them against the local ReDMCSB source tree, and writes a coverage matrix.
"""
from __future__ import annotations
import json, re, subprocess
from pathlib import Path
from collections import defaultdict

REPO = Path(__file__).resolve().parent.parent
SRC = Path.home()/'.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'
OUT = REPO/'parity-evidence/verification/pass168_redmcsb_pass_source_map'
ARTIFACT_GLOBS = [
    'tools/pass*.py', 'tools/verify_v1*.py',
    'parity-evidence/pass*.md', 'parity-evidence/dm1_all_graphics_phase*.md',
    'parity-evidence/verification/pass*/README.md', 'parity-evidence/verification/pass*/manifest.json',
]
PASS_RE = re.compile(r'(?:pass|phase)(\d+[a-z]?)', re.I)
FILE_RE = re.compile(r'\b([A-Z][A-Z0-9_]{1,15}\.(?:C|H))\b')
FUNC_RE = re.compile(r'\b([FGM]\d{3,4}_[A-Za-z0-9_]+|C\d{3,4}_[A-Z0-9_]+|M\d{3,4}_[A-Za-z0-9_]+|G\d{3,4}_[A-Za-z0-9_]+)\b')
ZONE_RE = re.compile(r'\b(C\d{3,4}(?:_[A-Z0-9_]+)?|M\d{3,4}(?:_[A-Za-z0-9_]+)?|G\d{3,4}(?:_[A-Za-z0-9_]+)?)\b')
SOURCE_WORDS = ('ReDMCSB','COMMAND.C','DUNVIEW.C','CLIKVIEW.C','MOVESENS.C','REVIVE.C','MENU.C','ENTRANCE.C','TITLE.C','DRAWVIEW.C','COORD.C','DEFS.H','CHAMPION','source')


def pass_id(path: Path, text: str) -> str:
    m = PASS_RE.search(str(path)) or PASS_RE.search(text[:500])
    return m.group(1).lower() if m else 'unclassified'


def rg_token(token: str, files_hint: set[str]) -> list[dict]:
    if not SRC.exists(): return []
    cmd = ['rg','-n','--sort','path','-F',token,str(SRC)]
    try:
        out = subprocess.run(cmd, text=True, capture_output=True, timeout=4).stdout
    except Exception:
        return []
    hits=[]
    for line in sorted(out.splitlines())[:40]:
        parts=line.split(':',2)
        if len(parts)<3: continue
        p=Path(parts[0])
        if files_hint and p.name.upper() not in files_hint and len(hits)>=3:
            continue
        hits.append({'file':p.name,'line':int(parts[1]),'text':parts[2].strip()[:180]})
    return hits[:8]


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    artifacts=[]
    for g in ARTIFACT_GLOBS:
        artifacts.extend(sorted(REPO.glob(g)))
    grouped=defaultdict(lambda: {'artifacts':[], 'files':set(), 'tokens':set(), 'mentions':0, 'bytes':0})
    for p in artifacts:
        if not p.is_file(): continue
        try:
            p.relative_to(OUT)
            continue
        except ValueError:
            pass
        try:
            text=p.read_text(errors='ignore')
        except Exception:
            continue
        pid=pass_id(p.relative_to(REPO), text)
        g=grouped[pid]
        g['artifacts'].append(str(p.relative_to(REPO)))
        g['bytes'] += len(text)
        if any(w in text for w in SOURCE_WORDS): g['mentions'] += 1
        for f in FILE_RE.findall(text): g['files'].add(f.upper())
        for t in FUNC_RE.findall(text): g['tokens'].add(t)
        # low volume C/M/G refs from source-heavy artifacts only
        if any(w in text for w in SOURCE_WORDS):
            for t in ZONE_RE.findall(text):
                if len(g['tokens']) < 120: g['tokens'].add(t)
    records=[]
    for pid in sorted(grouped, key=lambda x: (not x.isdigit(), int(re.match(r'\d+',x).group()) if re.match(r'\d+',x) else 9999, x)):
        g=grouped[pid]
        files=sorted(g['files'])
        tokens=sorted(g['tokens'])
        anchors=[]
        # File existence anchors
        for f in files[:20]:
            matches=list(SRC.glob(f)) if SRC.exists() else []
            if matches:
                anchors.append({'kind':'file','token':f,'hits':[{'file':matches[0].name,'line':1,'text':'file exists'}]})
        # Token anchors
        for t in tokens[:35]:
            hits=rg_token(t, set(files))
            if hits:
                anchors.append({'kind':'token','token':t,'hits':hits})
            if len(anchors)>=24: break
        status = 'source-locked' if anchors and (files or len(anchors)>=2) else ('source-mentioned-unresolved' if g['mentions'] else 'needs-redmcsb-anchor')
        records.append({
            'pass':pid, 'status':status, 'artifact_count':len(g['artifacts']),
            'artifacts':g['artifacts'][:12], 'source_files':files[:30],
            'tokens':tokens[:60], 'anchors':anchors[:24],
        })
    summary={
        'schema':'pass168_redmcsb_pass_source_map.v1',
        'source_root':str(SRC), 'artifact_count':len(artifacts),
        'passes':len(records),
        'status_counts':dict(sorted({s:sum(1 for r in records if r['status']==s) for s in {r['status'] for r in records}}.items())),
        'records':records,
    }
    (OUT/'manifest.json').write_text(json.dumps(summary, indent=2)+'\n')
    lines=['# Pass168 — Firestaff pass → ReDMCSB source map','',f'- source root: `{SRC}`',f'- pass groups: {len(records)}',f"- status counts: {summary['status_counts']}",'','## Priority unmapped / weakly mapped','']
    weak=[r for r in records if r['status']!='source-locked']
    for r in weak[:80]:
        lines.append(f"- pass `{r['pass']}`: **{r['status']}** — artifacts={r['artifact_count']} — first=`{r['artifacts'][0] if r['artifacts'] else ''}`")
    lines += ['','## Source-locked matrix','']
    for r in records:
        top=[]
        for a in r['anchors'][:5]:
            if a['hits']:
                h=a['hits'][0]
                top.append(f"{a['token']}→{h['file']}:{h['line']}")
        lines.append(f"- pass `{r['pass']}`: **{r['status']}**; files={', '.join(r['source_files'][:6]) or '-'}; anchors={'; '.join(top) or '-'}")
    lines += ['','## Next action','', '- Prioritize replacing `needs-redmcsb-anchor` route/capture passes with explicit source-file/function evidence before doing coordinate or emulator tuning.', '- For already source-locked route blockers, run runtime probes only after the source sequence is represented in `M11_GameView_HandlePointer`/mouse resolver tests.']
    (OUT/'README.md').write_text('\n'.join(lines)+'\n')
    print(json.dumps({'out':str(OUT),'passes':len(records),'status_counts':summary['status_counts']}, indent=2))
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
