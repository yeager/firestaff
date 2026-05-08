#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
MANIFEST=ROOT/'parity-evidence/verification/pass325_dm1_v1_alternative_runtime_evidence_path/manifest.json'
REPORT=ROOT/'parity-evidence/pass325_dm1_v1_alternative_runtime_evidence_path.md'
ALLOWED={'PASS_ALTERNATIVE_RUNTIME_EVIDENCE_PATH_FOUND','BLOCKED_NO_ALTERNATIVE_RUNTIME_EVIDENCE_PATH_FOUND'}
def main():
    errors=[]
    if not MANIFEST.exists(): errors.append('missing manifest')
    else:
        data=json.loads(MANIFEST.read_text(encoding='utf-8'))
        if data.get('schema')!='pass325_dm1_v1_alternative_runtime_evidence_path.v1': errors.append('bad schema')
        if data.get('status') not in ALLOWED: errors.append('bad status')
        if not all(item.get('ok') for item in data.get('sourceAudit',[])): errors.append('source audit failed')
        priors=data.get('priorArtifactAudit',{})
        for key in ['pass315','pass318','pass320','pass321','pass322','pass323']:
            if key not in priors or not priors[key].get('status'): errors.append(f'missing prior audit {key}')
        tests={t.get('id'):t for t in data.get('tests',[])}
        direct=tests.get('direct_pty_dosbox_debug_stdin_stdout')
        if not direct or direct.get('result')!='PASS_DIRECT_PTY_DEBUGGER_COMMANDS_ACCEPTED': errors.append('direct PTY control primitive not proven')
        if direct and direct.get('boundedSeconds',999)>60: errors.append('direct PTY test exceeded bound')
        if 'strace_read_write_capture' not in tests: errors.append('missing strace approach record')
        for t in data.get('tests',[]):
            if t.get('boundedSeconds',999)>60: errors.append(f"test {t.get('id')} exceeded bound")
            lp=ROOT/t.get('sanitizedLog','')
            if not lp.exists() or lp.stat().st_size<=0: errors.append(f"missing sanitized log for {t.get('id')}")
        nextp=data.get('decision',{}).get('nextRequiredPrimitive','')
        if '23AD:40FE' not in nextp or ('2809:1EFF' not in nextp and '2809:1E31' not in nextp) or '22F4:0699' not in nextp:
            errors.append('next required primitive does not name F0128/F0097/F0380 addresses')
    if not REPORT.exists(): errors.append('missing report')
    if errors:
        print(json.dumps({'status':'FAIL','errors':errors},indent=2,sort_keys=True)); return 1
    print(json.dumps({'status':'PASS','manifest_status':json.loads(MANIFEST.read_text()).get('status'),'manifest':str(MANIFEST),'report':str(REPORT)},indent=2,sort_keys=True)); return 0
if __name__=='__main__': raise SystemExit(main())
