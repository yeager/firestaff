#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
MAN=ROOT/'parity-evidence/verification/pass320_dm1_v1_f0097_vidrv_window_after_f0128_sequence_probe/manifest.json'
OUT=ROOT/'parity-evidence/verification/pass320_dm1_v1_f0097_vidrv_window_after_f0128_sequence_probe.json'
REPORT=ROOT/'parity-evidence/pass320_dm1_v1_f0097_vidrv_window_after_f0128_sequence_probe.md'
PASS318=ROOT/'parity-evidence/verification/pass318_dm1_v1_f0097_after_f0128_offset_window_probe.json'
SRC=Path.home()/'.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'
ADDRS={'F0128_DUNGEONVIEW_Draw_CPSF':'23AD:40FE','F0097_DUNGEONVIEW_DrawViewport_entry':'2809:1E31','F0097_after_palette_zone_setup':'2809:1EBD','F0097_before_viewport_args':'2809:1EEE','F0097_VIDRV_09_BlitViewPort_indirect_call':'2809:1EFF'}
def compact(s): return ' '.join(s.split())
def line_hits(fn, needles):
    lines=(SRC/fn).read_text(encoding='latin-1',errors='replace').splitlines(); out={}
    for n in needles:
        cn=compact(n)
        for i,l in enumerate(lines,1):
            if cn in compact(l): out[n]=i; break
    return out
def main():
    m=json.loads(MAN.read_text()) if MAN.exists() else {}
    hits=m.get('debugger_hits_captured',[]); events=m.get('debugger_events',[])
    f0128=any(h.get('kind')=='f0128_bp' for h in hits)
    f0097=any(str(h.get('kind','')).startswith('f0097_window_bp') for h in hits)
    armed_after=False
    if f0128:
        first_f0128=min(h['t'] for h in hits if h.get('kind')=='f0128_bp')
        armed_after=any(e.get('event')=='debugger_cmd' and e.get('cmd')=='BP 2809:1EFF' and e.get('t',0)>first_f0128 for e in events)
    status=m.get('status') or 'MISSING_RUNTIME_MANIFEST'
    acceptable=status in {'F0097_VIDRV_WINDOW_HIT_VERIFIED','BLOCKED_F0097_WINDOW_ARMED_AFTER_F0128_NO_HIT','BLOCKED_F0128_GATE_NOT_RECAPTURED_STRICT_STOP_FILTER'}
    anchors={
      'DRAWVIEW.C':line_hits('DRAWVIEW.C',['void F0097_DUNGEONVIEW_DrawViewport','F0638_GetZone(C007_ZONE_VIEWPORT','(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);']),
      'DUNVIEW.C':line_hits('DUNVIEW.C',['F0097_DUNGEONVIEW_DrawViewport(G0309_i_PartyMapIndex != C255_MAP_INDEX_ENTRANCE);','F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);']),
      'DEFS.H':line_hits('DEFS.H',['void (*VIDRV_09_BlitViewPort)(unsigned char* Pxxxx_puc_Bitmap, int16_t* Pxxxx_pi_Box);']),
      'IBMIO.C':line_hits('IBMIO.C',['G2156_VideoDriver = (VIDEO_DRIVER*)getvect(C255_DM_VIDEO_INTERRUPT);']),
      'VIDEODRV.C':line_hits('VIDEODRV.C',['(char*)F8161_VIDRV_09_BlitViewPort,                             /*  9 */','FUNC_DEF void F8161_VIDRV_09_BlitViewPort(']),
    }
    pass318=json.loads(PASS318.read_text()) if PASS318.exists() else {}
    out={'schema':'pass320_dm1_v1_f0097_vidrv_window_after_f0128_sequence_probe.verify.v1','status':status,'ok':acceptable,'addresses':ADDRS,'source_anchors':anchors,'runtime':{'manifest':str(MAN.relative_to(ROOT)) if MAN.exists() else None,'f0128_strict_hit':f0128,'f0097_window_hit':f0097,'vidrv_2809_1EFF_armed_after_f0128':armed_after,'hit_count':len(hits),'debugger_cmd_count':len(events)},'pass318_reuse':{'path':str(PASS318.relative_to(ROOT)) if PASS318.exists() else None,'status':pass318.get('status'),'static_ok':pass318.get('static_offset_binding') is not None,'conclusion':pass318.get('conclusion')},'blocker':m.get('blocker')}
    OUT.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n')
    REPORT.write_text(f"""# Pass320 — DM1 V1 F0097 VIDRV window after-F0128 sequencing probe

Status: `{status}`

## Source anchors

- `DRAWVIEW.C`: F0097 entry line {anchors['DRAWVIEW.C'].get('void F0097_DUNGEONVIEW_DrawViewport')}, viewport zone line {anchors['DRAWVIEW.C'].get('F0638_GetZone(C007_ZONE_VIEWPORT')}, VIDRV call line {anchors['DRAWVIEW.C'].get('(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);')}.
- `DUNVIEW.C`: F0128 calls F0097 at lines {anchors['DUNVIEW.C'].get('F0097_DUNGEONVIEW_DrawViewport(G0309_i_PartyMapIndex != C255_MAP_INDEX_ENTRANCE);')} and {anchors['DUNVIEW.C'].get('F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);')}.
- `DEFS.H`: video driver table slot 9 line {anchors['DEFS.H'].get('void (*VIDRV_09_BlitViewPort)(unsigned char* Pxxxx_puc_Bitmap, int16_t* Pxxxx_pi_Box);')}.
- `IBMIO.C`: PC runtime binds `G2156_VideoDriver` from interrupt vector line {anchors['IBMIO.C'].get('G2156_VideoDriver = (VIDEO_DRIVER*)getvect(C255_DM_VIDEO_INTERRUPT);')}.
- `VIDEODRV.C`: slot-9 table entry line {anchors['VIDEODRV.C'].get('(char*)F8161_VIDRV_09_BlitViewPort,                             /*  9 */')}, implementation line {anchors['VIDEODRV.C'].get('FUNC_DEF void F8161_VIDRV_09_BlitViewPort(')}.

## Runtime decision

- Strict F0128 stop recaptured: `{f0128}`.
- F0097/VIDRV window hit: `{f0097}`.
- Exact VIDRV candidate: `2809:1EFF`.
- Blocker: {m.get('blocker')}

Manifest: `parity-evidence/verification/pass320_dm1_v1_f0097_vidrv_window_after_f0128_sequence_probe/manifest.json`
""",encoding='utf-8')
    print(json.dumps({'status':status,'ok':acceptable,'f0128_strict_hit':f0128,'f0097_window_hit':f0097,'out':str(OUT)},indent=2,sort_keys=True))
    return 0 if acceptable else 1
if __name__=='__main__': raise SystemExit(main())
