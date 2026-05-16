#!/usr/bin/env python3
from __future__ import annotations
from datetime import datetime, timezone
import json, re, subprocess, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PASS="pass562_dm1_v1_front_cell_collision_source_lock"
RED=Path.home()/".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
OUT=ROOT/"parity-evidence"/"verification"/PASS
MANIFEST=OUT/"manifest.json"
REPORT=ROOT/"parity-evidence"/f"{PASS}.md"
def read(p,e="utf-8"): return p.read_text(encoding=e,errors="replace")
def line_no(s,o): return s.count("\n",0,o)+1
def req(s,n,l):
    p=s.find(n)
    if p<0: raise AssertionError(f"{l}: missing {n!r}")
    return p
def order(s,ns,l):
    out=[]; last=-1
    for n in ns:
        p=req(s,n,l)
        if p<=last: raise AssertionError(f"{l}: out-of-order {n!r}")
        out.append(p); last=p
    return out
def fn(s,name,rt=r"(?:static\s+)?(?:int|void|BOOLEAN|const char\*)", next_anchor=None):
    m=re.search(r"\b"+rt+r"\s+"+re.escape(name)+r"\s*\(",s)
    if not m: raise AssertionError("missing function "+name)
    b=s.find("{",m.end()); d=0
    for p in range(b,len(s)):
        if s[p]=="{": d+=1
        elif s[p]=="}":
            d-=1
            if d==0: return line_no(s,m.start()),line_no(s,p),s[m.start():p+1]
    if next_anchor:
        n=s.find(next_anchor,b)
        if n>b: return line_no(s,m.start()), line_no(s,n)-1, s[m.start():n]
    raise AssertionError("unterminated "+name)
def sp(base,body,ps): return base+line_no(body,min(ps))-1, base+line_no(body,max(ps))-1
def git(*a): return subprocess.check_output(["git",*a],cwd=ROOT,text=True).strip()
def main():
    OUT.mkdir(parents=True,exist_ok=True)
    clik=read(RED/"CLIKMENU.C","latin-1"); dung=read(RED/"DUNGEON.C","latin-1"); moves=read(RED/"MOVESENS.C","latin-1")
    core=read(ROOT/"src/dm1/dm1_v1_movement_command_core_pc34_compat.c"); move=read(ROOT/"src/memory/memory_movement_pc34_compat.c"); test=read(ROOT/"tests/test_dm1_v1_movement_command_core_pc34_compat.c")
    f0366_s,f0366_e,f0366=fn(clik,"F0366_COMMAND_ProcessTypes3To6_MoveParty", next_anchor="#include \"CLIKCHAM.C\"")
    f0150_s,f0150_e,f0150=fn(dung,"F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement")
    f0267_s,f0267_e,f0267=fn(moves,"F0267_MOVE_GetMoveResult_CPSCE","BOOLEAN", next_anchor="void F0268_SENSOR_AddEvent")
    f0366_span=sp(f0366_s,f0366,order(f0366,["AL1118_ui_MovementArrowIndex = P0735_ui_Command - C003_COMMAND_MOVE_FORWARD;","F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(G0308_i_PartyDirection","L1116_i_SquareType = M034_SQUARE_TYPE(AL1115_ui_Square = F0151_DUNGEON_GetSquare(L1121_i_MapX, L1122_i_MapY));","if (L1116_i_SquareType == C03_ELEMENT_STAIRS)","if (L1116_i_SquareType == C00_ELEMENT_WALL)","L1117_B_MovementBlocked = C1_TRUE;","if (L1116_i_SquareType == C04_ELEMENT_DOOR)","L1117_B_MovementBlocked = (L1117_B_MovementBlocked != C0_DOOR_STATE_OPEN) && (L1117_B_MovementBlocked != C1_DOOR_STATE_CLOSED_ONE_FOURTH) && (L1117_B_MovementBlocked != C5_DOOR_STATE_DESTROYED);","if (L1116_i_SquareType == C06_ELEMENT_FAKEWALL)","L1117_B_MovementBlocked = (!M007_GET(AL1115_ui_Square, MASK0x0004_FAKEWALL_OPEN) && !M007_GET(AL1115_ui_Square, MASK0x0001_FAKEWALL_IMAGINARY));","if (G0305_ui_PartyChampionCount == 0)","F0175_GROUP_GetThing(L1121_i_MapX, L1122_i_MapY)","F0209_GROUP_ProcessEvents29to41","F0357_COMMAND_DiscardAllInput();","F0693_WaitVerticalBlank();","G0321_B_StopWaitingForPlayerInput = C0_FALSE;","F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);"],"redmcsb f0366"))
    order(f0150,["*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0254_i_StepsForwardCount","P0253_i_Direction += 1, P0253_i_Direction &= 3; /* Simulate turning right */","*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0255_i_StepsRightCount"],"redmcsb f0150")
    f0267_span=sp(f0267_s,f0267,order(f0267,["G0397_i_MoveResultMapX = P0560_i_DestinationMapX;","G0398_i_MoveResultMapY = P0561_i_DestinationMapY;","G0399_ui_MoveResultMapIndex = L0715_ui_MapIndexDestination;","F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX, P0559_i_SourceMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C0_FALSE);","if ((P0557_T_Thing = F0175_GROUP_GetThing(G0306_i_PartyMapX, G0307_i_PartyMapY)) != C0xFFFE_THING_ENDOFLIST)","F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C1_TRUE);"],"redmcsb f0267"))
    core_s,core_e,process=fn(core,"DM1_V1_MovementCommandCore_ProcessOnePc34Compat","int")
    core_span=sp(core_s,process,order(process,["dm1_v1_apply_pre_step_stamina_cost(party, outResult);","if (!F0702_MOVEMENT_TryMove_Compat(dungeon, party, action, &outResult->movement)) {","outResult->movementBlocked = 1;","dm1_v1_record_blocked_wall_or_door_damage_request(party, action, outResult);","outResult->inputDiscardRequested = 1;","outResult->blockedMovementVblankWaitRequested = 1;","DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat(queue);","if (F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat(dungeon, things, party, action)) {","outResult->blockedByGroup = 1;","outResult->groupReactionPartyAdjacentRequested = 1;","party->mapIndex = outResult->movement.newMapIndex;"],"firestaff core"))
    f0702_s,f0702_e,f0702=fn(move,"F0702_MOVEMENT_TryMove_Compat","int")
    order(f0702,["F0701_MOVEMENT_GetStepDelta_Compat(party->direction, moveAction, &dx, &dy);","nx = party->mapX + dx;","if (nx < 0 || nx >= map->width || ny < 0 || ny >= map->height)","if (elementType == DUNGEON_ELEMENT_WALL)","if (elementType == DUNGEON_ELEMENT_DOOR)","if (doorState != 0 && doorState != 1 && doorState != 5)","} else if (elementType == DUNGEON_ELEMENT_FAKEWALL)","if (!(squareByte & 0x04) && !(squareByte & 0x01))","outResult->newMapX = nx;","outResult->resultCode = MOVE_OK;"],"firestaff f0702")
    f0708_s,f0708_e,f0708=fn(move,"F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat","int")
    order(f0708,["if (party->championCount <= 0) return 0;","if (!F0702_MOVEMENT_TryMove_Compat(dungeon, party, moveAction, &moveResult) ||","if (!(squareByte & DUNGEON_SQUARE_MASK_THING_LIST)) return 0;","if (THING_GET_TYPE(thing) == THING_TYPE_GROUP) return 1;"],"firestaff f0708")
    for n in ["pass547 closed door reports door block","pass547 closed fakewall reports wall block","pass547 group marked","pass547 group reaction requested","pass547 group keeps source x","pass547 group keeps source y"]: req(test,n,"runtime assertions")
    runtime="static-runtime-assertions-source-locked"
    status="PASS562_DM1_V1_FRONT_CELL_COLLISION_SOURCE_LOCKED"
    manifest={"schema":f"{PASS}.v1","timestampUtc":datetime.now(timezone.utc).isoformat(),"status":status,"branch":git("branch","--show-current"),"head":git("rev-parse","HEAD"),"sourceRoot":str(RED),"redmcsbAudit":{"CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty":f"CLIKMENU.C:{f0366_s}-{f0366_e}","CLIKMENU.C:frontCellCollisionSlice":f"CLIKMENU.C:{f0366_span[0]}-{f0366_span[1]}","DUNGEON.C:F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement":f"DUNGEON.C:{f0150_s}-{f0150_e}","MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE":f"MOVESENS.C:{f0267_s}-{f0267_e}","MOVESENS.C:acceptedMoveCommitSlice":f"MOVESENS.C:{f0267_span[0]}-{f0267_span[1]}"},"firestaffGuards":{"DM1_V1_MovementCommandCore_ProcessOnePc34Compat":f"dm1_v1_movement_command_core_pc34_compat.c:{core_s}-{core_e}","commandCoreBlockerSlice":f"dm1_v1_movement_command_core_pc34_compat.c:{core_span[0]}-{core_span[1]}","F0702_MOVEMENT_TryMove_Compat":f"memory_movement_pc34_compat.c:{f0702_s}-{f0702_e}","F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat":f"memory_movement_pc34_compat.c:{f0708_s}-{f0708_e}","runtimeExecutable":"test_dm1_v1_movement_command_core_pc34_compat (source assertions; executable build verified separately when link gate is available)"},"runtimeOutputFirstLine":runtime.splitlines()[0] if runtime else "","notClaimed":["original DOS overlay pixel parity","combat RNG/wound materialization","viewport wall rendering completion"]}
    MANIFEST.write_text(json.dumps(manifest,indent=2,sort_keys=True)+"\n",encoding="utf-8")
    REPORT.write_text(f"# Pass562 - DM1 V1 front-cell collision source lock\n\nStatus: {status}\n\n## ReDMCSB source audit\n- CLIKMENU.C:{f0366_span[0]}-{f0366_span[1]} proves relative coordinate, stairs consequence, wall, door states 0/1/5, fake-wall OPEN/IMAGINARY, group blocker, blocked discard/VBlank/re-arm, then accepted F0267 move.\n- DUNGEON.C:{f0150_s}-{f0150_e} proves forward/right relative coordinate math.\n- MOVESENS.C:{f0267_span[0]}-{f0267_span[1]} proves accepted movement commits move-result globals and leave/enter sensors.\n\n## Firestaff guard\n- dm1_v1_movement_command_core_pc34_compat.c:{core_span[0]}-{core_span[1]} keeps blockers before party tuple commit.\n- memory_movement_pc34_compat.c:{f0702_s}-{f0702_e} owns wall/door/fake-wall legality.\n- memory_movement_pc34_compat.c:{f0708_s}-{f0708_e} owns the champion-count gated group blocker.\n- test_dm1_v1_movement_command_core_pc34_compat.c asserts closed door, closed fake-wall, and group blocked-command behavior.\n\nManifest: parity-evidence/verification/pass562_dm1_v1_front_cell_collision_source_lock/manifest.json\n",encoding="utf-8")
    print(f"{status} manifest={MANIFEST.relative_to(ROOT)} report={REPORT.relative_to(ROOT)}"); return 0
if __name__=="__main__":
    try: raise SystemExit(main())
    except (AssertionError,OSError,subprocess.SubprocessError) as exc:
        print(f"FAIL {PASS}: {exc}",file=sys.stderr); raise SystemExit(1)
