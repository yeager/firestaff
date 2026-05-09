#!/usr/bin/env python3
"""Verify DM1 V1 viewport stairs/pits/teleporter ordering against ReDMCSB."""
from __future__ import annotations
import argparse, json, subprocess
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SOURCE = Path('~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source').expanduser()
LOCAL_C = ROOT / 'dm1_v1_viewport_3d_pc34_compat.c'
LOCAL_H = ROOT / 'dm1_v1_viewport_3d_pc34_compat.h'
LOCAL_TEST = ROOT / 'test_dm1_v1_viewport_3d_pc34_compat.c'

CHECKS: list[dict[str, Any]] = [
    {
        'id': 'f0104-f0113-primitive-source',
        'file': 'DUNVIEW.C', 'range': '3113-3225',
        'ordered': [
            'STATICFUNCTION void F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(',
            'F0630_InitBitmapStruct2(P0112_i_NativeBitmapIndex, &L2434_s_Struct2);',
            'F0132_VIDEO_Blit(L2433_puc_Bitmap, G0296_puc_Bitmap_Viewport',
            'STATICFUNCTION void F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(',
        ],
        'why': 'Stairs/pits/walls use F0104/F0105 blits into G0296 viewport before later handoffs.'
    },
    {
        'id': 'f0115-layer-order-source',
        'file': 'DUNVIEW.C', 'range': '4547-5933',
        'ordered': [
            'STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(',
            'if ((AL0127_i_ThingType = M012_TYPE(P0141_T_Thing)) == C04_THING_TYPE_GROUP)',
            'if (AL0127_i_ThingType == C14_THING_TYPE_PROJECTILE)',
            'P0141_T_Thing = L0146_T_FirstThingToDraw; /* Restart processing list of things from the beginning.',
            'if (M012_TYPE(P0141_T_Thing) == C15_THING_TYPE_EXPLOSION)',
        ],
        'why': 'F0115 orders objects/creatures/projectiles before the final explosion pass.'
    },
    {
        'id': 'd3l2-stairs-pit-floor-things-field-wall-return',
        'file': 'DUNVIEW.C', 'range': '6230-6291',
        'ordered': [
            'case C19_ELEMENT_STAIRS_FRONT:',
            'F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(M714_NEGGRAPHIC_STAIRS_UP_D3L2, C800_ZONE_STAIRS_UP_FRONT_D3L2);',
            'case C00_ELEMENT_WALL:',
            'F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C11_WALL_D3L2], C702_ZONE_WALL_D3L2);',
            'return;',
            'case C02_ELEMENT_PIT:',
            'F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(C049_GRAPHIC_FLOOR_PIT_D3L2, C850_ZONE_FLOORPIT_D3L2);',
            'F0108_DUNGEONVIEW_DrawFloorOrnament',
            'F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF',
            'F0113_DUNGEONVIEW_DrawField',
        ],
        'why': 'D3L2 draws stairs/pit/floor before F0115; teleporter field overlays after F0115; wall returns before F0115.'
    },
    {
        'id': 'd3l-stairs-pit-floor-things-field-wall-return',
        'file': 'DUNVIEW.C', 'range': '6370-6496',
        'ordered': [
            'case C19_ELEMENT_STAIRS_FRONT:',
            'F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G0079_ai_StairsNativeBitmapIndices[C00_STAIRS_BITMAP_UP_FRONT_D3L], C802_ZONE_STAIRS_UP_FRONT_D3L);',
            'case C00_ELEMENT_WALL:',
            'F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C13_WALL_D3L], C705_ZONE_WALL_D3L);',
            'return;',
            'case C02_ELEMENT_PIT:',
            'F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(M754_GRAPHIC_FLOOR_PIT_D3L, C852_ZONE_FLOORPIT_D3L);',
            'F0108_DUNGEONVIEW_DrawFloorOrnament',
            'F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF',
            'F0113_DUNGEONVIEW_DrawField',
        ],
        'why': 'D3L has the same floor/thing/field layering, with wall early-return unless an alcove branches to F0115.'
    },
    {
        'id': 'd3c-stairs-pit-floor-things-field-wall-return',
        'file': 'DUNVIEW.C', 'range': '6660-6832',
        'ordered': [
            'case C19_ELEMENT_STAIRS_FRONT:',
            'F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G0079_ai_StairsNativeBitmapIndices[C01_STAIRS_BITMAP_UP_FRONT_D3C], C803_ZONE_STAIRS_UP_FRONT_D3C);',
            'case C00_ELEMENT_WALL:',
            'F0792_DUNGEONVIEW_DrawBitmapYYY(G2107_WallSet[C14_WALL_D3C], C704_ZONE_WALL_D3C',
            'return;',
            'case C02_ELEMENT_PIT:',
            'F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(M755_GRAPHIC_FLOOR_PIT_D3C, C853_ZONE_FLOORPIT_D3C);',
            'F0108_DUNGEONVIEW_DrawFloorOrnament',
            'F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF',
            'F0113_DUNGEONVIEW_DrawField',
        ],
        'why': 'D3C center wall occludes/returns; pit/floor/F0115/field follow source order.'
    },

    {
        'id': 'd2c-stairs-pit-floor-ceiling-things-field-wall-return',
        'file': 'DUNVIEW.C', 'range': '7260-7388',
        'ordered': [
            'F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G0079_ai_StairsNativeBitmapIndices[C03_STAIRS_BITMAP_UP_FRONT_D2C], C806_ZONE_STAIRS_UP_FRONT_D2C);',
            'case C00_ELEMENT_WALL:',
            'F0792_DUNGEONVIEW_DrawBitmapYYY(G2107_WallSet[C09_WALL_D2C], C709_ZONE_WALL_D2C',
            'return;',
            'case C02_ELEMENT_PIT:',
            'F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(L0212_ai_SquareAspect[M554_PIT_OR_TELEPORTER_VISIBLE] ? M763_GRAPHIC_FLOOR_PIT_INVISIBLE_D2C : M757_GRAPHIC_FLOOR_PIT_D2C, C856_ZONE_FLOORPIT_D2C);',
            'F0108_DUNGEONVIEW_DrawFloorOrnament',
            'F0112_DUNGEONVIEW_DrawCeilingPit(C065_GRAPHIC_CEILING_PIT_D2C, C865_ZONE_CEILING_PIT_D2C',
            'F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF',
            'F0113_DUNGEONVIEW_DrawField',
        ],
        'why': 'D2C extends the source lock to a mid-depth center square: stairs/pit/floor/ceiling precede F0115; teleporter field overlays after F0115; wall returns before common things unless alcove branches.'
    },
    {
        'id': 'd0c-stairs-pit-ceiling-things-field',
        'file': 'DUNVIEW.C', 'range': '8176-8310',
        'ordered': [
            'case C19_ELEMENT_STAIRS_FRONT:',
            'F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G0079_ai_StairsNativeBitmapIndices[C06_STAIRS_BITMAP_UP_FRONT_D0C_LEFT], C811_ZONE_STAIRS_UP_FRONT_D0L);',
            'break;',
            'case C02_ELEMENT_PIT:',
            'F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(L0222_ai_SquareAspect[M554_PIT_OR_TELEPORTER_VISIBLE] ? M767_GRAPHIC_FLOOR_PIT_INVISIBLE_D0C : M761_GRAPHIC_FLOOR_PIT_D0C, C862_ZONE_FLOORPIT_D0C);',
            'F0112_DUNGEONVIEW_DrawCeilingPit(C069_GRAPHIC_CEILING_PIT_D0C, C871_ZONE_CEILING_PIT_D0C',
            'F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF',
            'F0113_DUNGEONVIEW_DrawField',
        ],
        'why': 'D0C draws near stairs/pit/ceiling before F0115, then teleporter field over things.'
    },
]

LOCAL_NEEDLES = [
    (LOCAL_H, 'DM1_ViewportFloorFieldOrderSpec'),
    (LOCAL_C, 's_floor_field_order_specs'),
    (LOCAL_C, 'stairs/pit/floor-ornament/F0115/teleporter-field order'),
    (LOCAL_TEST, 'test_floor_field_stairs_pit_teleporter_order'),
]

def read_range(source: Path, file: str, r: str) -> str:
    a,b=map(int,r.split('-'))
    return '\n'.join((source/file).read_text(errors='replace').splitlines()[a-1:b])

def ordered_missing(text: str, needles: list[str]) -> list[str]:
    pos=-1; miss=[]
    for n in needles:
        i=text.find(n,pos+1)
        if i<0: miss.append(n)
        else: pos=i
    return miss

def run(cmd: list[str]) -> dict[str, Any]:
    p=subprocess.run(cmd,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=30)
    return {'cmd':cmd,'returncode':p.returncode,'passed':p.returncode==0,'output_tail':p.stdout.splitlines()[-20:]}

def main() -> int:
    ap=argparse.ArgumentParser(); ap.add_argument('--source',type=Path,default=DEFAULT_SOURCE); ap.add_argument('--run-runtime',action='store_true'); ap.add_argument('--json',action='store_true')
    args=ap.parse_args(); ok=True; results=[]
    for c in CHECKS:
        text=read_range(args.source,c['file'],c['range'])
        miss=ordered_missing(text,c['ordered'])
        passed=not miss; ok &= passed
        results.append({'id':c['id'],'passed':passed,'source':[f"{c['file']}:{c['range']}"],'missing':miss,'why':c['why']})
    for path,needle in LOCAL_NEEDLES:
        passed=needle in path.read_text(errors='replace'); ok &= passed
        results.append({'id':f'local:{path.name}:{needle}','passed':passed,'source':[str(path.relative_to(ROOT))],'missing':[] if passed else [needle],'why':'Firestaff exposes and tests the source-locked stairs/pits/field order contract.'})
    runtime=None
    if args.run_runtime:
        runtime=run([str(ROOT/'build'/'test_dm1_v1_viewport_3d_pc34_compat')]); ok &= runtime['passed']
    payload={'gate':'dm1_v1_viewport_stairs_pits_teleporter_order_source_runtime_lock','source_root':str(args.source),'passed':ok,'checks':results,'runtime':runtime}
    if args.json: print(json.dumps(payload,indent=2,sort_keys=True))
    else:
        for r in results:
            print(('PASS' if r['passed'] else 'FAIL'), r['id'], '; '.join(r['source']))
            print(' ', r['why'])
            for m in r['missing']: print('  missing/order:',m)
        if runtime:
            print(('PASS' if runtime['passed'] else 'FAIL'), 'runtime', ' '.join(runtime['cmd']))
            for line in runtime['output_tail']: print(' ',line)
    return 0 if ok else 1
if __name__=='__main__': raise SystemExit(main())
