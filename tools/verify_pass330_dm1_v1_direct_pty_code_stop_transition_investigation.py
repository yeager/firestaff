#!/usr/bin/env python3
import json, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PASS='pass330_dm1_v1_direct_pty_code_stop_transition_investigation'
MAN=ROOT/'parity-evidence/verification'/PASS/'manifest.json'
ALLOWED={'PASS_DIRECT_PTY_F0128_CODE_STOP_PROVEN','PASS_DIRECT_PTY_F0128_F0097_SEQUENCE_PROVEN','BLOCKED_PASS330_BREAKPOINT_ADDRESS_REBASE_UNRESOLVED','BLOCKED_PASS330_CPU_NEVER_REACHES_F0128_UNDER_ROUTE','BLOCKED_PASS330_STOP_MARKER_FORMAT_UNRECOGNIZED','BLOCKED_PASS330_ROUTE_NO_VIEWPORT_REDRAW'}
err=[]
if not MAN.exists(): err.append('missing manifest')
else:
 data=json.loads(MAN.read_text())
 if data.get('status') not in ALLOWED: err.append('bad status '+str(data.get('status')))
 if data.get('addresses',{}).get('F0128_DUNGEONVIEW_Draw_CPSF')!='23AD:40FE': err.append('F0128 address not locked')
 if data.get('addresses',{}).get('F0097_VIDRV_09_BlitViewPort_indirect_call')!='2809:1EFF': err.append('F0097 address not locked')
 if data.get('notPromotedBy')!=['BPLIST','BP command echo','tmux/capture-pane']: err.append('promotion guard missing')
 if not all(r.get('ok') for r in data.get('sourceAudit',[])): err.append('source audit failed')
 rt=data.get('runtimeProbe',{})
 if rt.get('boundedSecondsPerStrategy',999)>75: err.append('runtime bound exceeded')
 ss=rt.get('strategies',[])
 if len(ss)>2: err.append('too many runtime probes')
 if data.get('status')=='BLOCKED_PASS330_CPU_NEVER_REACHES_F0128_UNDER_ROUTE':
  if not ss: err.append('no strategy')
  else:
   s=ss[0]
   if s.get('directHits',{}).get('f0128_23AD_40FE'): err.append('blocked despite f0128 hit')
   if s.get('breakpointRetainedPostRoute') is not True: err.append('breakpoint not retained')
   if len(s.get('routeLog',[])) < 20: err.append('route not delivered')
   if not s.get('postRoutePauseCodeAddr'): err.append('missing post-route pause addr')
 if str(data.get('status','')).startswith('PASS'):
  if not any(s.get('directHits',{}).get('f0128_23AD_40FE') for s in ss): err.append('PASS without F0128 hit')
if err:
 print('FAIL', '; '.join(err)); sys.exit(1)
print('pass330 verifier OK')
