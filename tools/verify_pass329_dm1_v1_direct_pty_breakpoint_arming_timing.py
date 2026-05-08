#!/usr/bin/env python3
from __future__ import annotations
import json, sys
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
PASS = 'pass329_dm1_v1_direct_pty_breakpoint_arming_timing'
MANIFEST = ROOT / 'parity-evidence/verification' / PASS / 'manifest.json'
REPORT = ROOT / 'parity-evidence' / f'{PASS}.md'
ALLOWED = {
    'PASS_DIRECT_PTY_F0128_CODE_STOP_PROVEN',
    'PASS_DIRECT_PTY_F0128_F0097_SEQUENCE_PROVEN',
    'BLOCKED_PASS329_LOAD_READY_MARKER_MISSING',
    'BLOCKED_PASS329_BREAKPOINT_NOT_RETAINED_ACROSS_LOAD',
    'BLOCKED_PASS329_ROUTE_INPUT_AFTER_ARMING_FAILED',
    'BLOCKED_PASS329_CODE_STOP_TRANSITION_NOT_EMITTED',
}

def fail(msg: str) -> int:
    print(json.dumps({'status': 'FAIL', 'error': msg, 'manifest': str(MANIFEST), 'report': str(REPORT)}, indent=2, sort_keys=True))
    return 1

def main() -> int:
    if not MANIFEST.exists(): return fail('missing manifest')
    if not REPORT.exists(): return fail('missing report')
    data = json.loads(MANIFEST.read_text(encoding='utf-8'))
    if data.get('schema') != PASS + '.v1': return fail('bad schema')
    status = data.get('status')
    if status not in ALLOWED: return fail(f'unexpected status {status!r}')
    if not all(r.get('ok') for r in data.get('sourceAudit', [])): return fail('source audit failed')
    addrs = data.get('addresses', {})
    if addrs.get('F0128_DUNGEONVIEW_Draw_CPSF') != '23AD:40FE': return fail('F0128 address not locked')
    if addrs.get('F0097_VIDRV_09_BlitViewPort_indirect_call') != '2809:1EFF': return fail('F0097 address not locked')
    rt = data.get('runtimeProbe', {})
    if rt.get('boundedSecondsPerStrategy', 999) > 75: return fail('strategy bound too high')
    strategies = rt.get('strategies', [])
    if rt.get('ran') and not strategies: return fail('runtime ran without strategies')
    if len(strategies) > 2: return fail('more than two strategies')
    if any('tmux' in (s.get('method', '').lower()) for s in strategies): return fail('strategy method mentions tmux')
    if set(data.get('notPromotedBy', [])) & {'BPLIST', 'BP command echo'} != {'BPLIST', 'BP command echo'}: return fail('missing negative controls')
    if status.startswith('PASS'):
        if not any(s.get('directHits', {}).get('f0128_23AD_40FE') for s in strategies): return fail('pass without F0128 direct hit')
        for s in strategies:
            for stop in s.get('stops', []):
                if stop.get('addr') == '23AD:40FE' and not (stop.get('running_marker_seen') and stop.get('prompt_reappeared_after_running')):
                    return fail('F0128 hit lacks strict transition')
    text = REPORT.read_text(encoding='utf-8', errors='replace')
    for item in ['DUNVIEW.C', 'DRAWVIEW.C', 'COMMAND.C', status]:
        if item not in text: return fail(f'report missing {item}')
    for strategy in strategies:
        p = ROOT / 'parity-evidence/verification' / PASS / (strategy['strategy'] + '.clean.txt')
        if not p.exists() or p.stat().st_size <= 0: return fail(f'missing transcript {p}')
    print(json.dumps({'status': 'PASS', 'manifest_status': status, 'strategies': [s.get('strategy') for s in strategies], 'manifest': str(MANIFEST), 'report': str(REPORT)}, indent=2, sort_keys=True))
    return 0
if __name__ == '__main__':
    raise SystemExit(main())
