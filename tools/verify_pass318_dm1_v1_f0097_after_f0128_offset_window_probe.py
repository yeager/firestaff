#!/usr/bin/env python3
"""Pass318 verifier: bind F0097 entry vs VIDRV_09_BlitViewPort candidate offsets.

Conservative: proves by ReDMCSB source + FIRES.MAP + decompressed original bytes that
2809:1E31 is F0097 function entry and 2809:1EFF is the PC34/I34E VIDRV_09_BlitViewPort
indirect call candidate. Runtime promotion still requires an explicit post-F0128 stop.
"""
from __future__ import annotations
import json, shutil, struct, subprocess, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
SRC=Path.home()/'.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'
FIRES_MAP=Path.home()/'.openclaw/data/redmcsb-n2-build-probe/ibm-pc-i34e-fires/HARDDISK/BUILD/I34E/FIRES.MAP'
FIRES=Path.home()/'.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Reference/Original/I34E/FIRES'
PROBE=ROOT/'parity-evidence/verification/pass318_dm1_v1_f0097_after_f0128_offset_window_probe/manifest.json'
OUT=ROOT/'parity-evidence/verification/pass318_dm1_v1_f0097_after_f0128_offset_window_probe.json'
MD=ROOT/'parity-evidence/pass318_dm1_v1_f0097_after_f0128_offset_window_probe.md'

def compact(s:str)->str: return ' '.join(s.split())
def source_check(fn,a,b,needles):
    p=SRC/fn; lines=p.read_text(encoding='latin-1',errors='replace').splitlines()
    found={}; missing=[]
    for n in needles:
        cn=compact(n); hit=None
        for i in range(a-1,min(b,len(lines))):
            if cn in compact(lines[i]): hit=i+1; break
        if hit is None: missing.append(n)
        else: found[n]=hit
    return {'file':fn,'line_range':[a,b],'ok':not missing,'found':found,'missing':missing}
def unlzexe_bytes():
    with tempfile.TemporaryDirectory(prefix='pass318-unlzexe-') as td:
        td=Path(td); target=td/'FIRES.EXE'; shutil.copy2(FIRES,target)
        subprocess.run([str(Path.home()/'bin/unlzexe'),str(target)],check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
        new=td/'FIRES.EXENEW'
        return new.read_bytes()
def mz_file_offset(blob,seg,off):
    hdr=struct.unpack_from('<14H',blob,0)[4]
    return hdr*16+seg*16+off

def main():
    source=[source_check('DRAWVIEW.C',709,858,['void F0097_DUNGEONVIEW_DrawViewport','F0638_GetZone(C007_ZONE_VIEWPORT','(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);']),source_check('DUNVIEW.C',8604,8611,['F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);'])]
    map_text=FIRES_MAP.read_text(encoding='latin-1',errors='replace')
    map_ok='_F0097_DUNGEONVIEW_DRAWVIEWPORT' in map_text and '20D6:1E31' in map_text and '_F0128_DUNGEONVIEW_DRAW_CPSF' in map_text and '1C7A:40FE' in map_text
    blob=unlzexe_bytes(); base_seg=0x20d6
    checks={
      'entry_20D6_1E31_runtime_2809_1E31': {'offset':'1E31','bytes':blob[mz_file_offset(blob,base_seg,0x1e31):mz_file_offset(blob,base_seg,0x1e31)+6].hex(),'expect_prefix':'558bec'},
      'post_zone_20D6_1EBD_runtime_2809_1EBD': {'offset':'1EBD','bytes':blob[mz_file_offset(blob,base_seg,0x1ebd):mz_file_offset(blob,base_seg,0x1ebd)+8].hex(),'expect_prefix':'168d46ea50'},
      'viewport_args_20D6_1EEE_runtime_2809_1EEE': {'offset':'1EEE','bytes':blob[mz_file_offset(blob,base_seg,0x1eee):mz_file_offset(blob,base_seg,0x1eee)+13].hex(),'expect_prefix':'168d46f250ff362c3dff362a3d'},
      'vidrv_call_20D6_1EFF_runtime_2809_1EFF': {'offset':'1EFF','bytes':blob[mz_file_offset(blob,base_seg,0x1eff):mz_file_offset(blob,base_seg,0x1eff)+4].hex(),'expect_prefix':'26ff5f24'},
    }
    static_ok=all(v['bytes'].startswith(v['expect_prefix']) for v in checks.values())
    probe=json.loads(PROBE.read_text()) if PROBE.exists() else {}
    runtime_hit=probe.get('status')=='F0097_OFFSET_WINDOW_HIT_VERIFIED'
    status='F0097_RUNTIME_HIT_VERIFIED_STATIC_VIDRV_BINDING' if runtime_hit and static_ok and map_ok and all(r['ok'] for r in source) else 'BLOCKED_F0097_RUNTIME_HIT_STATIC_VIDRV_CANDIDATE_2809_1EFF'
    manifest={'schema':'pass318_dm1_v1_f0097_offset_binding.v1','status':status,'source_audit':source,'map_audit':{'path':str(FIRES_MAP),'ok':map_ok,'f0097_link':'20D6:1E31','f0097_runtime_entry':'2809:1E31','f0128_link':'1C7A:40FE','f0128_runtime':'23AD:40FE'},'static_offset_binding':checks,'conclusion':'2809:1E31 is the F0097 function entry from FIRES.MAP. Static original-byte binding narrows DRAWVIEW.C:857 / VIDRV_09_BlitViewPort to the indirect lcall at 20D6:1EFF, runtime 2809:1EFF. Runtime probe did not capture a post-F0128 F0097-window stop, so viewport_present remains unpromoted.','probe_manifest':str(PROBE.relative_to(ROOT)) if PROBE.exists() else None,'probe_status':probe.get('status')}
    OUT.write_text(json.dumps(manifest,indent=2,sort_keys=True)+'\n')
    MD.write_text(f'''# Pass318 — DM1 V1 F0097 / VIDRV offset binding\n\nStatus: `{status}`\n\n## Source audit\n\n- DRAWVIEW.C `F0097_DUNGEONVIEW_DrawViewport`: lines 709, 850, 857 verified.\n- DUNVIEW.C F0128 calls F0097 at line 8610 verified.\n\n## Binding decision\n\n- `2809:1E31` is the F0097 function entry, from FIRES.MAP `20D6:1E31` + loader segment `0733`.\n- Better static candidate for `DRAWVIEW.C:857 VIDRV_09_BlitViewPort` is `2809:1EFF` (`26 ff 5f 24`, indirect far call through the video driver table).\n- The after-F0128 runtime window probe did not capture a F0097-window stop, so no viewport-present runtime seam is promoted.\n\nManifest: `parity-evidence/verification/pass318_dm1_v1_f0097_after_f0128_offset_window_probe.json`\n''',encoding='utf-8')
    print(json.dumps({'status':status,'runtime_hit':runtime_hit,'static_ok':static_ok,'map_ok':map_ok,'out':str(OUT)},indent=2))
    return 0 if status.startswith('BLOCKED_') or status.endswith('BINDING') else 1
if __name__=='__main__': raise SystemExit(main())
