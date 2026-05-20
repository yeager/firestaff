#!/usr/bin/env python3
from __future__ import annotations
import hashlib, json, os
from collections import Counter
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = 'pass608_dm1_v1_same_viewport_capture_blocker'
STATUS = 'BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE'
OUT_DIR = ROOT / 'parity-evidence' / 'verification' / PASS
MANIFEST = OUT_DIR / 'manifest.json'
REPORT = ROOT / 'parity-evidence' / f'{PASS}.md'
RED = Path(os.environ.get('FIRESTAFF_REDMCSB_SOURCE', str(Path.home() / '.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source')))
DM1 = Path.home() / '.openclaw/data/firestaff-original-games/DM/_canonical/dm1'
FIRESTAFF_CAPTURE_MANIFEST = ROOT / 'verification-screens/capture_manifest_sha256.tsv'
FIRESTAFF_STATE_PROBE = ROOT / 'verification-m11/capture-route-state-pass195/pass76_capture_route_state_probe.json'
EXPECTED_ORIGINAL = {
    'GRAPHICS.DAT': '2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e',
    'DUNGEON.DAT': 'd90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85',
    'TITLE': 'adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745',
}
SOURCE_LOCKS = [
    {'file':'COMMAND.C','lines':'106-114','function':'G0448_as_Graphic561_SecondaryMouseInput_Movement','needles':['C001_COMMAND_TURN_LEFT,             234, 261, 125, 145','C003_COMMAND_MOVE_FORWARD,          263, 289, 125, 145','C002_COMMAND_TURN_RIGHT,            291, 318, 125, 145','C080_COMMAND_CLICK_IN_DUNGEON_VIEW,   0, 223,  33, 168'],'claim':'PC34 mouse route tokens must hit source command zones before labels mean anything.'},
    {'file':'GAMELOOP.C','lines':'164-219','function':'F0002_MAIN_GameLoop_CPSDF','needles':['G0321_B_StopWaitingForPlayerInput = C0_FALSE;','while (M527_IsCharacterInKeyboardBuffer())','F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());','F0380_COMMAND_ProcessQueue_CPSC();','while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);'],'claim':'A capture must be after command processing lets the wait loop exit.'},
    {'file':'COMMAND.C','lines':'2045-2156','function':'F0380_COMMAND_ProcessQueue_CPSC','needles':['L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command','G2153_i_QueuedCommandsCount--;','F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);','F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);'],'claim':'Each original shot needs F0380 pop/count delta and F0365/F0366 dispatch.'},
    {'file':'CLIKMENU.C','lines':'142-174','function':'F0365_COMMAND_ProcessTypes1To2_TurnParty','needles':['void F0365_COMMAND_ProcessTypes1To2_TurnParty','G0321_B_StopWaitingForPlayerInput = C1_TRUE;','F0284_CHAMPION_SetPartyDirection'],'claim':'Turn shots must prove source direction mutation.'},
    {'file':'CLIKMENU.C','lines':'180-347','function':'F0366_COMMAND_ProcessTypes3To6_MoveParty','needles':['void F0366_COMMAND_ProcessTypes3To6_MoveParty','F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement','F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY','G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;'],'claim':'Move shots must prove accepted movement or source-visible blocked/no-op handling.'},
    {'file':'DUNVIEW.C','lines':'8318-8611','function':'F0128_DUNGEONVIEW_Draw_CPSF','needles':['void F0128_DUNGEONVIEW_Draw_CPSF','P0183_i_Direction','P0184_i_MapX','P0185_i_MapY','F0127_DUNGEONVIEW_DrawSquareD0C','F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);'],'claim':'The viewport bitmap must be composed from the same direction/X/Y tuple.'},
    {'file':'DRAWVIEW.C','lines':'709-858','function':'F0097_DUNGEONVIEW_DrawViewport','needles':['void F0097_DUNGEONVIEW_DrawViewport','F0638_GetZone(C007_ZONE_VIEWPORT','VIDRV_09_BlitViewPort'],'claim':'A promotable crop belongs after the PC34 viewport-present blit.'},
]
FRESH = {
    'capturedUtc':'2026-05-20T23:49:20Z',
    'localDate':'2026-05-21 Europe/Stockholm',
    'attemptDir':'verification-screens/pass601-same-viewport-original-diagnostic',
    'command':"OUT_DIR=$PWD/verification-screens/pass601-same-viewport-original-diagnostic DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 WAIT_BEFORE_INPUT_MS=5000 NEW_FILE_TIMEOUT_MS=6000 DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 shot:title enter wait:1200 shot:pre_enter_menu click:260,50 wait:1200 shot:after_enter_click click:276,140 wait:600 shot:forward_1 click:276,140 wait:600 shot:forward_2 click:246,140 wait:600 shot:left_turn_probe' DOSBOX=/usr/bin/dosbox xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run",
    'classifications':[
        {'index':1,'label':'title','expected':'title_or_menu','actual':'graphics_320x200_unclassified','rawSha256':'6176af21cb32ededfc6090ce39523f66a686a59ac4cd51d2c895da1c6286b987','cropSha256':'358136006c6d53d112d1cfea3d4bd0fa0902df0ad8b7130cf68778e298a24aa9'},
        {'index':2,'label':'pre_enter_menu','expected':'entrance_menu','actual':'entrance_menu','rawSha256':'9f95e1d8fae6b5dd5d2dcbeba09a164f96774347634ef9328937441bb87fbe19','cropSha256':'ea845264f9229fed624079892da4c51653b7a101b2c0fa3f95aea4a4621f7edb'},
        {'index':3,'label':'after_enter_click','expected':'dungeon_gameplay','actual':'entrance_menu','rawSha256':'17bd7e87815750b45e742964ffe93e0312d9bbdc45dd8e7358be0a069a6db1b8','cropSha256':'701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c'},
        {'index':4,'label':'forward_1','expected':'dungeon_gameplay','actual':'entrance_menu','rawSha256':'17bd7e87815750b45e742964ffe93e0312d9bbdc45dd8e7358be0a069a6db1b8','cropSha256':'701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c'},
        {'index':5,'label':'forward_2','expected':'dungeon_gameplay','actual':'dungeon_gameplay','rawSha256':'355a191cd07b5f630fe323b8c54c13321ad811330dc075155461a5c8ba346cbe','cropSha256':'701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c'},
        {'index':6,'label':'left_turn_probe','expected':'dungeon_gameplay','actual':'dungeon_gameplay','rawSha256':'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397','cropSha256':'701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c'},
    ],
    'transcriptScaffold':{'status':'SCAFFOLD_ONLY_MISSING_ORIGINAL_RUNTIME_DEBUG_FIELDS','rows':0},
}
def norm(s:str)->str: return ' '.join(s.split())
def source_window(path:Path,spec:str)->str:
    lines=path.read_text(encoding='latin-1',errors='replace').splitlines(); chunks=[]
    for part in spec.split(','):
        if '-' in part: start,end=[int(v) for v in part.split('-',1)]
        else: start=end=int(part)
        chunks.append('\n'.join(lines[start-1:end]))
    return '\n'.join(chunks)
def sha256(path:Path)->str:
    h=hashlib.sha256()
    with path.open('rb') as fh:
        for chunk in iter(lambda: fh.read(1024*1024), b''): h.update(chunk)
    return h.hexdigest()
def rel(path:Path)->str:
    try: return str(path.relative_to(ROOT))
    except ValueError: return str(path)
def audit_source():
    rows=[]
    for lock in SOURCE_LOCKS:
        path=RED/lock['file']; text=source_window(path,lock['lines']) if path.exists() else ''
        missing=[n for n in lock['needles'] if norm(n) not in norm(text)]
        rows.append({**lock,'path':str(path),'exists':path.exists(),'ok':path.exists() and not missing,'missing':missing})
    return rows
def audit_original_files():
    rows=[]
    for name, expected in EXPECTED_ORIGINAL.items():
        path=DM1/name; actual=sha256(path) if path.exists() else None
        rows.append({'file':name,'path':str(path),'exists':path.exists(),'sha256':actual,'expectedSha256':expected,'ok':actual==expected})
    return rows
def read_firestaff_capture_manifest():
    rows=[]
    for line in FIRESTAFF_CAPTURE_MANIFEST.read_text(encoding='utf-8').splitlines():
        if not line or line.startswith('#'): continue
        kind, filename, width, height, size, digest = line.split('\t')
        if kind == 'viewport_224x136': rows.append({'filename':filename,'width':int(width),'height':int(height),'bytes':int(size),'sha256':digest})
    return rows
def duplicate_groups(rows,key):
    counts=Counter(row[key] for row in rows)
    return {v:[row['index'] for row in rows if row[key]==v] for v,c in counts.items() if c>1}
def build_payload():
    states=json.loads(FIRESTAFF_STATE_PROBE.read_text(encoding='utf-8'))['snapshots']
    original_rows=FRESH['classifications']; raw_dupes=duplicate_groups(original_rows,'rawSha256'); crop_dupes=duplicate_groups(original_rows,'cropSha256')
    blockers=[]
    if [r for r in original_rows if r['actual'] != r['expected']]: blockers.append('fresh original diagnostic class sequence is not the expected title/menu -> entrance -> gameplay route')
    if raw_dupes: blockers.append('fresh original diagnostic has duplicate raw 320x200 frames')
    if crop_dupes: blockers.append('fresh original diagnostic has duplicate 224x136 viewport crops')
    if FRESH['transcriptScaffold']['rows'] == 0: blockers.append('fresh original diagnostic produced no command/state/redraw transcript rows')
    blockers.append('original rows do not bind map/X/Y/direction to F0380 -> F0365/F0366 -> F0128 -> F0097 for the same sampled frame')
    return {'schema':f'firestaff.parity.{PASS}.v1','status':STATUS,'ok':True,'decision':'No promotable same-viewport original/Firestaff manifest exists yet. Firestaff has exact fixture state and viewport hashes, and original PC34 assets/tools are present, but the latest N2 original diagnostic still lacks the command-state-redraw-present transcript required to bind an original frame to any Firestaff map/X/Y/direction row.','sourceRoot':str(RED),'sourceAudit':audit_source(),'originalAssetAudit':audit_original_files(),'firestaffEvidence':{'stateProbe':rel(FIRESTAFF_STATE_PROBE),'captureManifest':rel(FIRESTAFF_CAPTURE_MANIFEST),'states':states,'viewportHashes':read_firestaff_capture_manifest()},'freshOriginalDiagnostic':{**FRESH,'rawDuplicateRouteIndices':raw_dupes,'cropDuplicateRouteIndices':crop_dupes},'blockers':blockers,'promotionRequires':['exact original map/X/Y/direction/wall-door state for every sampled frame','command id and queue delta from F0380 for the sampled command','matching F0365/F0366 handler/state delta or source-visible blocked/no-op proof','later F0128 tuple and F0097/VIDRV present boundary before screenshot acceptance','a Firestaff fixture row with the same map/X/Y/direction/wall-door state and reproducible viewport hash'],'nonClaims':['no original-vs-Firestaff pixel parity is claimed','no ReDMCSB source-table gap is claimed','no Firestaff renderer behavior is changed','fresh original images are diagnostic-only and remain unpromoted']}
def write_report(payload):
    lines=['# Pass608 - DM1 V1 same-viewport capture blocker','',f"Status: {payload['status']}",'','## Decision','',payload['decision'],'','## Source audit','']
    for row in payload['sourceAudit']: lines.append(f"- {row['file']}:{row['lines']} {row['function']} ok={row['ok']} - {row['claim']}")
    lines += ['','## Firestaff fixture','']
    for state in payload['firestaffEvidence']['states']: lines.append(f"- {state['capture']}: map={state['mapIndex']} x={state['mapX']} y={state['mapY']} dir={state['direction']} tick={state['tick']} spell={state['spellPanelOpen']} inventory={state['inventoryPanelActive']}")
    lines += ['','## Fresh original diagnostic','',f"- Command: `{payload['freshOriginalDiagnostic']['command']}`",f"- Transcript scaffold: {payload['freshOriginalDiagnostic']['transcriptScaffold']['status']} rows={payload['freshOriginalDiagnostic']['transcriptScaffold']['rows']}",'','| # | label | expected | actual | raw sha | crop sha |','|---|---|---|---|---|---|']
    for row in payload['freshOriginalDiagnostic']['classifications']: lines.append(f"| {row['index']} | {row['label']} | {row['expected']} | {row['actual']} | `{row['rawSha256'][:12]}` | `{row['cropSha256'][:12]}` |")
    lines += ['','## Blockers','']; lines.extend(f'- {b}' for b in payload['blockers'])
    lines += ['','## Promotion requires','']; lines.extend(f'- {b}' for b in payload['promotionRequires'])
    lines += ['','## Non-claims','']; lines.extend(f'- {b}' for b in payload['nonClaims'])
    lines += ['',f'Manifest: {rel(MANIFEST)}']
    REPORT.write_text('\n'.join(lines)+'\n',encoding='utf-8')
def main():
    OUT_DIR.mkdir(parents=True,exist_ok=True); payload=build_payload(); failures=[]
    failures += [f"source audit failed {r['file']}:{r['lines']}" for r in payload['sourceAudit'] if not r['ok']]
    failures += [f"original asset hash failed {r['file']}" for r in payload['originalAssetAudit'] if not r['ok']]
    if len(payload['firestaffEvidence']['states']) != 6: failures.append('Firestaff state probe must contain six snapshots')
    if len(payload['firestaffEvidence']['viewportHashes']) != 6: failures.append('Firestaff capture manifest must contain six viewport crops')
    if failures: payload['status']='FAIL_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_BLOCKER_GATE'; payload['ok']=False; payload['failures']=failures
    MANIFEST.write_text(json.dumps(payload,indent=2,sort_keys=True)+'\n',encoding='utf-8'); write_report(payload)
    print(json.dumps({'status':payload['status'],'ok':payload['ok'],'manifest':rel(MANIFEST),'report':rel(REPORT),'blockers':payload['blockers']},indent=2,sort_keys=True))
    return 0 if payload['ok'] else 1
if __name__ == '__main__': raise SystemExit(main())
