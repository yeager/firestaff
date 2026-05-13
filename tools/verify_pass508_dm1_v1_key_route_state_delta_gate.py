#!/usr/bin/env python3
from __future__ import annotations
import hashlib, json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass508_dm1_v1_key_route_state_delta_gate"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
DM1 = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1"
GREATSTONE = Path.home() / ".openclaw/data/firestaff-greatstone-atlas"
CSBWIN = Path.home() / ".openclaw/data/firestaff-csbwin-source/CSBWin"
OUT = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT / "manifest.json"
REPORT = ROOT / "parity-evidence" / (PASS + ".md")

INPUTS = {
    "pass387": ROOT / "parity-evidence/verification/pass387_keyboard_f0361_queue_write/manifest.json",
    "pass391": ROOT / "parity-evidence/verification/pass391_dm1_v1_queued_command_dispatch/manifest.json",
    "pass487": ROOT / "parity-evidence/verification/pass487_dm1_v1_original_click_capture_blocker/manifest.json",
    "pass495": ROOT / "parity-evidence/verification/pass495_dm1_v1_f0365_f0366_runtime_static_boundary/manifest.json",
    "pass497": ROOT / "parity-evidence/verification/pass497_dm1_v1_original_capture_next_blocker/manifest.json",
    "pass498": ROOT / "parity-evidence/verification/pass498_dm1_v1_original_post_command_state_delta_boundary/manifest.json",
}
SOURCE = [
    ("COMMAND.C","636-685","PC34/I34E keyboard rows map route keys to C001/C002 turns and C003..C006 movement commands.",["G0459_as_Graphic561_SecondaryKeyboardInput_Movement","MEDIA707_I34E_I34M","C001_COMMAND_TURN_LEFT,     0x004B","C003_COMMAND_MOVE_FORWARD,  0x004C","C002_COMMAND_TURN_RIGHT,    0x004D","C006_COMMAND_MOVE_LEFT,     0x004F","C005_COMMAND_MOVE_BACKWARD, 0x0050","C004_COMMAND_MOVE_RIGHT,    0x0051"]),
    ("COMMAND.C","1709-1813","F0361 resolves keyboard input and writes the command queue before F0380 consumes it.",["void F0361_COMMAND_ProcessKeyPress","G0435_B_CommandQueueLocked = C1_TRUE","G0443_ps_PrimaryKeyboardInput","G0444_ps_SecondaryKeyboardInput","G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command","G2153_i_QueuedCommandsCount++;","F0360_COMMAND_ProcessPendingClick();"]),
    ("COMMAND.C","2045-2156","F0380 rejects gated movement before dequeue, otherwise pop-loads exactly one command and dispatches turns/steps.",["void F0380_COMMAND_ProcessQueue_CPSC","G0435_B_CommandQueueLocked = C1_TRUE;","if (G2153_i_QueuedCommandsCount == 0)","L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;","G0310_i_DisabledMovementTicks","G2153_i_QueuedCommandsCount--;","F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);","F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"]),
    ("CLIKMENU.C","142-174","F0365 accepted turns set stop-wait and mutate party direction.",["void F0365_COMMAND_ProcessTypes1To2_TurnParty","G0321_B_StopWaitingForPlayerInput = C1_TRUE;","F0276_SENSOR_ProcessThingAdditionOrRemoval","F0284_CHAMPION_SetPartyDirection"]),
    ("CLIKMENU.C","180-347","F0366 computes movement, blocks before commit, or commits via F0267 and applies cooldown after success.",["void F0366_COMMAND_ProcessTypes3To6_MoveParty","F0325_CHAMPION_DecrementStamina","F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement","F0357_COMMAND_DiscardAllInput();","F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);","G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;"]),
    ("MOVESENS.C","738-780","F0267 records destination map tuple and movement timing/scent state after accepted party movement.",["G0397_i_MoveResultMapX = P0560_i_DestinationMapX;","G0398_i_MoveResultMapY = P0561_i_DestinationMapY;","G0399_ui_MoveResultMapIndex = L0715_ui_MapIndexDestination;","G0362_l_LastPartyMovementTime = G0313_ul_GameTime;"]),
    ("GAMELOOP.C","90-219","The next promotable capture must be after stop/tick lets the loop redraw from the updated party tuple.",["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);","while (M527_IsCharacterInKeyboardBuffer())","F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer())","F0380_COMMAND_ProcessQueue_CPSC();","while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"]),
]
ORDERS = [
    ("COMMAND.C","2045-2156",["G0435_B_CommandQueueLocked = C1_TRUE;","L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;","if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT)","L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;","G0435_B_CommandQueueLocked = C0_FALSE;","F0360_COMMAND_ProcessPendingClick();","F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);","F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"]),
    ("CLIKMENU.C","180-347",["if (L1117_B_MovementBlocked)","F0357_COMMAND_DiscardAllInput();","G0321_B_StopWaitingForPlayerInput = C0_FALSE;","return;","F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);","G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;"]),
]
def compact(s): return " ".join(s.split())
def window(file, spec):
    a,b=map(int,spec.split("-"))
    return "\n".join((RED/file).read_text(encoding="latin-1", errors="replace").splitlines()[a-1:b])
def require(label, text, needles):
    flat=compact(text)
    missing=[n for n in needles if compact(n) not in flat]
    if missing: raise AssertionError(label + " missing " + repr(missing))
def verify_sources():
    out=[]
    for f,lines,claim,needles in SOURCE:
        require(f+":"+lines, window(f,lines), needles)
        out.append({"file":f,"lines":lines,"claim":claim})
    for f,lines,needles in ORDERS:
        flat=compact(window(f,lines)); pos=-1
        for n in needles:
            hit=flat.find(compact(n), pos+1)
            if hit < 0: raise AssertionError(f+":"+lines+" order missing "+repr(n))
            pos=hit
    return out
def read_json(path):
    if not path.exists(): raise AssertionError("missing input manifest: "+str(path))
    return json.loads(path.read_text(encoding="utf-8"))
def sha(path):
    h=hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1048576), b""): h.update(chunk)
    return h.hexdigest()
def secondary():
    expected={"DUNGEON.DAT":"d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85","GRAPHICS.DAT":"2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e","TITLE":"adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745"}
    hashes={k:sha(DM1/k) for k in expected}
    for k,v in expected.items():
        if hashes[k] != v: raise AssertionError("DM1 hash mismatch "+k)
    require("Greatstone SUMMARY",(GREATSTONE/"index/SUMMARY.md").read_text(encoding="utf-8", errors="replace"),["http://greatstone.free.fr/dm/g_dm.html","dungeon.dat","graphics.dat"])
    require("CSBWin Code11f52.cpp",(CSBWIN/"Code11f52.cpp").read_text(encoding="utf-8", errors="replace"),["i16 MoveObject","d.partyX = sw(newX);","d.partyY = sw(newY);","d.LastPartyMoveTime = d.Time;"])
    return {"canonicalDm1Sha256":hashes,"greatstoneSummary":str(GREATSTONE/"index/SUMMARY.md"),"csbwinMoveObject":str(CSBWIN/"Code11f52.cpp")}
def main():
    src=verify_sources(); sec=secondary(); m={k:read_json(v) for k,v in INPUTS.items()}
    p391=m["pass391"].get("proofPredicates",{}); p487=m["pass487"].get("blockerFindings",{}); p497=m["pass497"].get("observed",{}); p498=m["pass498"].get("requiredObserved",{})
    required={
        "pass387_keyboard_queue_write_proven": m["pass387"].get("proofPredicates",{}).get("queuedCommandCountWriteObserved") is True,
        "pass391_f0380_pop_load_after_queue_write": p391.get("f0380PopLoadAfterQueueWriteObserved") is True,
        "pass391_g2153_decrement_observed": p391.get("g2153DecrementPopLoadObserved") is True,
        "pass391_f0365_or_f0366_dispatch_observed": p391.get("f0365OrF0366DispatchObserved") is True,
        "pass495_runtime_chain_closed": m["pass495"].get("runtimeStopEvidence",{}).get("ok") is True,
        "pass498_static_no_state_delta_current": p498.get("static_no_state_delta_currently_present") is True,
        "pass487_repeated_post_entry_hash_current": p487.get("postEntryGameplayHashRepeated") is True or bool(p497.get("duplicateSha256Counts",{}).get("48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397")),
    }
    problems=[k for k,v in required.items() if not v]
    status="PASS508_DM1_V1_KEY_ROUTE_STATE_DELTA_GATE_LOCKED" if not problems else "FAIL_PASS508_DM1_V1_KEY_ROUTE_STATE_DELTA_GATE"
    rule=["F0361 queue write and G2153 increment","F0380 pop/load/decrement for same command","F0365 turn or F0366 movement handling after dequeue","later redraw/present over mutated direction/X/Y tuple, or explicit source-proven blocked/no-op","reject repeated 48ed static gameplay frames and route-label-only filenames"]
    manifest={"schema":"firestaff.parity."+PASS+".v1","status":status,"ok":not problems,"redmcsbRoot":str(RED),"sourceAudit":src,"secondaryReferences":sec,"inputManifests":{k:str(v.relative_to(ROOT)) for k,v in INPUTS.items()},"requiredObserved":required,"promotionRule":rule,"narrowedBlocker":"Movement/key-route processing is source/runtime proven through F0380 and F0365/F0366; original capture is still blocked at post-command state-delta-to-redraw evidence because the current post-entry frames repeat sha256 48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397.","nonClaims":["no new DOSBox/FIRES run","no pixel parity promotion","no viewport wall draw-order claim"],"problems":problems}
    OUT.mkdir(parents=True, exist_ok=True); MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True)+"\n")
    lines=["# Pass508 - DM1 V1 key-route state-delta gate","",status,"",manifest["narrowedBlocker"],"","## ReDMCSB source audit"]
    lines += ["- "+x["file"]+":"+x["lines"]+" - "+x["claim"] for x in src]
    lines += ["","## Promotion rule"] + ["- "+x for x in rule] + ["","## Secondary references"]
    lines += ["- canonical DM1 "+k+" sha256 "+v for k,v in sec["canonicalDm1Sha256"].items()]
    lines += ["- Greatstone atlas: "+sec["greatstoneSummary"],"- CSBWin movement cross-check: "+sec["csbwinMoveObject"],"","## Gate","- python3 tools/verify_pass508_dm1_v1_key_route_state_delta_gate.py"]
    REPORT.write_text("\n".join(lines)+"\n")
    print(json.dumps({"status":status,"manifest":str(MANIFEST.relative_to(ROOT)),"report":str(REPORT.relative_to(ROOT)),"problems":problems}, indent=2, sort_keys=True))
    return 0 if not problems else 1
if __name__ == "__main__": raise SystemExit(main())
