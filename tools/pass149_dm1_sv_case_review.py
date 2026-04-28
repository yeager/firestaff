#!/usr/bin/env python3
from __future__ import annotations
import ast, json, re
from pathlib import Path
PO=Path('po/dm1.sv.po')
REPORT=Path('parity-evidence/pass149_dm1_sv_case_review.md')

def decode(lines): return ''.join(ast.literal_eval(x) for x in lines)
def quote(s):
    # one-line PO strings are enough for current dm1 strings
    return '"' + s.replace('\\','\\\\').replace('"','\\"').replace('\n','\\n') + '"'
def letters(s): return ''.join(ch for ch in s if ch.isalpha())
def upperish(s):
    l=letters(s)
    return bool(l) and l.upper()==l

def parse(text):
    entries=[]; cur={'pre':[], 'msgid_lines':[], 'msgstr_lines':[], 'start':1}; mode=None
    for ln,line in enumerate(text.splitlines(),1):
        if line.startswith('#'):
            if not cur['msgid_lines'] and not cur['msgstr_lines']: cur['start']=ln
            cur['pre'].append(line); continue
        if line.startswith('msgid '):
            if cur['msgid_lines'] or cur['msgstr_lines']:
                entries.append(cur); cur={'pre':[], 'msgid_lines':[], 'msgstr_lines':[], 'start':ln}
            mode='msgid'; cur['msgid_lines'].append(line[6:].strip()); continue
        if line.startswith('msgstr '):
            mode='msgstr'; cur['msgstr_lines'].append(line[7:].strip()); continue
        if line.startswith('"'):
            if mode: cur[mode+'_lines'].append(line.strip())
            continue
        if not line.strip() and (cur['msgid_lines'] or cur['msgstr_lines']):
            entries.append(cur); cur={'pre':[], 'msgid_lines':[], 'msgstr_lines':[], 'start':ln+1}; mode=None
    if cur['msgid_lines'] or cur['msgstr_lines']: entries.append(cur)
    for e in entries:
        e['msgid']=decode(e['msgid_lines']); e['msgstr']=decode(e['msgstr_lines'])
    return entries
text=PO.read_text()
entries=parse(text)
fixes=[]; issues=[]
manual_replacements={
    'RÄCKSLANS HORN':'RÄDSLANS HORN',
}
for e in entries:
    mid=e['msgid']; ms=e['msgstr']
    if not mid: continue
    new=ms
    if upperish(mid) and letters(ms) and letters(ms).upper()!=letters(ms):
        new=ms.upper()
        fixes.append((e['start'], mid, ms, new, 'uppercase source -> uppercase Swedish'))
    if new in manual_replacements:
        fixes.append((e['start'], mid, new, manual_replacements[new], 'Swedish typo fix'))
        new=manual_replacements[new]
    e['new_msgstr']=new
# rebuild file conservatively: comments + msgid one-line/multiline from parsed value + msgstr one-line
parts=[]
for e in entries:
    parts.extend(e['pre'])
    parts.append('msgid ' + quote(e['msgid']))
    parts.append('msgstr ' + quote(e.get('new_msgstr', e['msgstr'])))
    parts.append('')
PO.write_text('\n'.join(parts).rstrip()+"\n")
# reparse and verify
entries2=parse(PO.read_text())
remaining=[]
for e in entries2:
    if e['msgid'] and upperish(e['msgid']) and letters(e['msgstr']) and letters(e['msgstr']).upper()!=letters(e['msgstr']):
        remaining.append((e['start'],e['msgid'],e['msgstr']))
REPORT.parent.mkdir(exist_ok=True)
REPORT.write_text('\n'.join([
    '# Pass 149 — DM1 Swedish case review', '',
    f'- file: `po/dm1.sv.po`',
    f'- entries reviewed: {len(entries)}',
    f'- fixes applied: {len(fixes)}',
    f'- remaining uppercase-source case mismatches: {len(remaining)}',
    '', '## Fixes', '',
] + [f"- line {ln}: `{mid}` — `{old}` → `{new}` ({why})" for ln,mid,old,new,why in fixes] + [
    '', '## Remaining mismatches', '',
] + ([ '- none' ] if not remaining else [f"- line {ln}: `{mid}` -> `{ms}`" for ln,mid,ms in remaining]) + ['']), encoding='utf-8')
print(json.dumps({'entries':len(entries),'fixes':len(fixes),'remaining':len(remaining)}, ensure_ascii=False))
