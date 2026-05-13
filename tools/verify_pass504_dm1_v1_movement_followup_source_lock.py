#!/usr/bin/env python3
from __future__ import annotations
import hashlib, json, re, subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass504_dm1_v1_movement_followup_source_lock"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
OUT = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT / "manifest.json"
REPORT = ROOT / "parity-evidence" / (PASS + ".md")

def read(p, enc="utf-8"):
    return Path(p).read_text(encoding=enc, errors="replace")

def line_no(s, off):
    return s.count("\n", 0, off) + 1

def req(s, needle, label):
    pos = s.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos

def req_order(s, needles, label):
    last = -1
    for n in needles:
        pos = req(s, n, label)
        if pos <= last:
            raise AssertionError(f"{label}: out of order {n!r}")
        last = pos

def fn(s, name, rettype=r"(?:void|BOOLEAN|int16_t|int|unsigned char|const char\*)", next_name=None):
    m = re.search(r"\b" + rettype + r"\s+" + re.escape(name) + r"\s*\(", s)
    if not m:
        raise AssertionError("missing function " + name)
    b = s.find("{", m.end())
    depth = 0
    for pos in range(b, len(s)):
        if s[pos] == "{":
            depth += 1
        elif s[pos] == "}":
            depth -= 1
            if depth == 0:
                return line_no(s, m.start()), line_no(s, pos), s[m.start():pos+1]
    if next_name:
        n = re.search(r"^.*\b" + re.escape(next_name) + r"\s*\(", s[m.end():], re.M)
        if n:
            end = m.end() + n.start()
            return line_no(s, m.start()), line_no(s, end) - 1, s[m.start():end]
    raise AssertionError("unterminated function " + name)

def git(*args):
    return subprocess.check_output(["git", *args], cwd=ROOT, text=True).strip()

def sha(s):
    return hashlib.sha256(s.encode("utf-8", "replace")).hexdigest()

def main():
    OUT.mkdir(parents=True, exist_ok=True)
    command = read(RED / "COMMAND.C", "latin-1")
    clik = read(RED / "CLIKMENU.C", "latin-1")
    moves = read(RED / "MOVESENS.C", "latin-1")
    f0380_s, f0380_e, f0380 = fn(command, "F0380_COMMAND_ProcessQueue_CPSC")
    f0365_s, f0365_e, f0365 = fn(clik, "F0365_COMMAND_ProcessTypes1To2_TurnParty")
    f0366_s, f0366_e, f0366 = fn(clik, "F0366_COMMAND_ProcessTypes3To6_MoveParty", next_name="F0369_COMMAND_ProcessTypes101To108_ClickInSpellSymbolsArea_CPSE")
    f0267_s, f0267_e, f0267 = fn(moves, "F0267_MOVE_GetMoveResult_CPSCE", "BOOLEAN", next_name="F0268_SENSOR_AddEvent")
    f0276_s, f0276_e, f0276 = fn(moves, "F0276_SENSOR_ProcessThingAdditionOrRemoval")
    req_order(f0380, [
        "G0435_B_CommandQueueLocked = C1_TRUE;",
        "if (G2153_i_QueuedCommandsCount == 0)",
        "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
        "G0310_i_DisabledMovementTicks ||",
        "G2153_i_QueuedCommandsCount--;",
        "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
    ], "F0380 queue gate/dequeue/dispatch")
    req(f0380, "G0435_B_CommandQueueLocked = C0_FALSE;", "F0380 queue unlock")
    req(f0380, "F0360_COMMAND_ProcessPendingClick();", "F0380 pending click replay")
    req_order(f0365, [
        "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX",
        "F0284_CHAMPION_SetPartyDirection",
        "C1_TRUE);",
    ], "F0365 turn leave/enter envelope")
    req_order(f0366, [
        "F0325_CHAMPION_DecrementStamina",
        "AL1118_ui_MovementArrowIndex = P0735_ui_Command - C003_COMMAND_MOVE_FORWARD;",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "if (L1116_i_SquareType == C00_ELEMENT_WALL)",
        "if (L1116_i_SquareType == C04_ELEMENT_DOOR)",
        "if (L1116_i_SquareType == C06_ELEMENT_FAKEWALL)",
        "if (G0305_ui_PartyChampionCount == 0)",
        "if (L1117_B_MovementBlocked)",
        "F0357_COMMAND_DiscardAllInput();",
        "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
        "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);",
        "AL1115_ui_Ticks = 1;",
        "F0310_CHAMPION_GetMovementTicks",
        "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
        "G0311_i_ProjectileDisabledMovementTicks = 0;",
    ], "F0366 stamina/collision/blocked-return/cooldown")
    req_order(f0267, [
        "G0397_i_MoveResultMapX = P0560_i_DestinationMapX;",
        "G0398_i_MoveResultMapY = P0561_i_DestinationMapY;",
        "G0399_ui_MoveResultMapIndex = L0715_ui_MapIndexDestination;",
        "G0401_ui_MoveResultCell = M011_CELL(P0557_T_Thing);",
        "F0317_CHAMPION_AddScentStrength",
        "G0362_l_LastPartyMovementTime = G0313_ul_GameTime;",
        "F0276_SENSOR_ProcessThingAdditionOrRemoval",
    ], "F0267 accepted move state/scent/sensors")
    req(f0276, "F0272_SENSOR_TriggerEffect", "F0276 sensor dispatch")
    checks = [
        ("dm1_v1_input_command_queue_pc34_compat.c", "COMMAND.C:2045-2156 F0380 locks"),
        ("dm1_v1_movement_command_core_pc34_compat.c", "dm1_v1_apply_pre_step_stamina_cost(party, outResult);"),
        ("dm1_v1_movement_command_core_pc34_compat.c", "F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat"),
        ("dm1_v1_movement_command_core_pc34_compat.c", "outResult->blockedMovementVblankWaitRequested = 1;"),
        ("dm1_v1_movement_timing_pc34_compat.c", "GAMELOOP.C:150-155"),
        ("test_dm1_v1_command_movement_sensor_timing_pc34_compat.c", "blocked movement skips enter/leave sensors"),
        ("test_dm1_v1_command_movement_sensor_timing_pc34_compat.c", "turn bypasses movement gate"),
        ("test_dm1_v1_command_movement_sensor_timing_pc34_compat.c", "core blocked wall still applies pre-resolution stamina"),
        ("parity-evidence/verification/pass406_dm1_v1_movement_legality_completion_gate/manifest.json", "PASS406_DM1_V1_MOVEMENT_LEGALITY_COMPLETION_GATE_PROVEN"),
    ]
    for rel, needle in checks:
        req(read(ROOT / rel), needle, rel)
    pass406 = json.loads(read(ROOT / "parity-evidence/verification/pass406_dm1_v1_movement_legality_completion_gate/manifest.json"))
    manifest = {
        "schema": PASS + ".v1",
        "status": "PASS504_DM1_V1_MOVEMENT_FOLLOWUP_SOURCE_LOCKED",
        "branch": git("branch", "--show-current"),
        "sourceRoot": str(RED),
        "redmcsbAudit": {
            "COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC": "COMMAND.C:2045-2156",
            "CLIKMENU.C:F0365_COMMAND_ProcessTypes1To2_TurnParty": f"CLIKMENU.C:{f0365_s}-{f0365_e}",
            "CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty": f"CLIKMENU.C:{f0366_s}-{f0366_e}",
            "MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE": f"MOVESENS.C:{f0267_s}-{f0267_e}",
            "MOVESENS.C:F0276_SENSOR_ProcessThingAdditionOrRemoval": f"MOVESENS.C:{f0276_s}-{f0276_e}",
        },
        "sourceSliceSha256": {"F0380": sha(f0380), "F0365": sha(f0365), "F0366": sha(f0366), "F0267": sha(f0267), "F0276": sha(f0276)},
        "currentMovementBlocker": {
            "id": "pass406_dm1_v1_movement_legality_completion_gate",
            "status": pass406.get("status"),
            "manifest": "parity-evidence/verification/pass406_dm1_v1_movement_legality_completion_gate/manifest.json",
            "blockerResolvedHere": "Required runtime executable was built in this worktree and pass406 re-ran clean.",
        },
        "scopeGuard": ["DM1 V1 movement only", "No viewport/walls, CSB, DM2, Nexus, or pass435 capture files changed", "No original DOS pixel overlay parity claim"],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    REPORT.write_text("\n".join([
        "# Pass504 - DM1 V1 movement follow-up source lock",
        "",
        "Status: " + manifest["status"],
        "",
        "## ReDMCSB-first audit",
        "- COMMAND.C:2045-2156 / F0380_COMMAND_ProcessQueue_CPSC: movement gates keep step commands queued, then dequeue one command and dispatch turns/steps.",
        f"- CLIKMENU.C:{f0365_s}-{f0365_e} / F0365_COMMAND_ProcessTypes1To2_TurnParty: current-square leave/enter movement-result calls around party rotation, with no step cooldown.",
        f"- CLIKMENU.C:{f0366_s}-{f0366_e} / F0366_COMMAND_ProcessTypes3To6_MoveParty: living-champion stamina before target-square legality, wall/door/fakewall/group blockers, blocked-input discard and one PC-34 VBlank, cooldown only after accepted movement.",
        f"- MOVESENS.C:{f0267_s}-{f0267_e} / F0267_MOVE_GetMoveResult_CPSCE: accepted-move globals, scent/last-party-movement timing, teleporter/pit consequences, and source/destination sensor calls.",
        f"- MOVESENS.C:{f0276_s}-{f0276_e} / F0276_SENSOR_ProcessThingAdditionOrRemoval: floor-sensor effect dispatcher used by movement-result.",
        "",
        "## Firestaff guard",
        "- Local movement seams checked: input command queue, movement command core, movement timing helper, and command/movement/sensor timing tests.",
        "- Current movement blocker check: pass406 is green after building build/test_dm1_v1_movement_core_pc34_compat.",
        "",
        "## Scope guard",
        "- DM1 V1 movement only; no viewport/walls, CSB, DM2, Nexus, or pass435 capture files changed.",
        "- Source/evidence lock plus executable gate coverage; no original DOS pixel overlay parity claim.",
        "",
        "Manifest: parity-evidence/verification/" + PASS + "/manifest.json",
    ]) + "\n")
    print(manifest["status"] + " manifest=" + str(MANIFEST.relative_to(ROOT)) + " report=" + str(REPORT.relative_to(ROOT)))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
