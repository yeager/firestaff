#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
REDMCSB_SOURCE = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
COMMAND_C = REDMCSB_SOURCE / "COMMAND.C"
CLIKMENU_C = REDMCSB_SOURCE / "CLIKMENU.C"
CHAMPION_C = REDMCSB_SOURCE / "CHAMPION.C"
MOVESENS_C = REDMCSB_SOURCE / "MOVESENS.C"
COMPAT_C = ROOT / "dm1_v1_movement_timing_pc34_compat.c"
TEST_C = ROOT / "test_dm1_v1_movement_timing_pc34_compat.c"

def block(path: Path, start: int, end: int) -> str:
    lines = path.read_text(errors="replace").splitlines()
    return "\n".join(lines[start - 1:end])

def require(label: str, text: str, needles: list[str]) -> None:
    missing = [needle for needle in needles if needle not in text]
    if missing:
        raise AssertionError(f"{label} missing {missing}")

def require_regex(label: str, text: str, pattern: str) -> None:
    if not re.search(pattern, text, re.S):
        raise AssertionError(f"{label} missing regex {pattern}")

def verify_redmcsb() -> list[str]:
    notes = []
    require("COMMAND.C:2095-2100 movement-disabled gate", block(COMMAND_C, 2095, 2100), [
        "G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command",
        "G0310_i_DisabledMovementTicks",
        "G0311_i_ProjectileDisabledMovementTicks",
        "G0312_i_LastProjectileDisabledMovementDirection",
        "F0360_COMMAND_ProcessPendingClick",
    ]); notes.append("COMMAND.C:2095-2100 movement commands remain queued while cadence/projectile gates are active")
    require("COMMAND.C:2118-2127 single dequeue before dispatch", block(COMMAND_C, 2118, 2127), [
        "L1161_i_CommandX", "L1162_i_CommandY", "++G0433_i_CommandQueueFirstIndex",
        "G0435_B_CommandQueueLocked = C0_FALSE", "F0360_COMMAND_ProcessPendingClick",
    ]); notes.append("COMMAND.C:2118-2127 one command is dequeued before turn/move dispatch")
    require("CLIKMENU.C:256-269 movement vector mapping", block(CLIKMENU_C, 256, 269), [
        "AL1118_ui_MovementArrowIndex = P0735_ui_Command - C003_COMMAND_MOVE_FORWARD",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "G0465_ai_Graphic561_MovementArrowToStepForwardCount",
        "G0466_ai_Graphic561_MovementArrowToStepRightCount",
    ]); notes.append("CLIKMENU.C:256-269 source-locks forward/right/back/left relative step mapping")
    require("CLIKMENU.C:330-346 post-step cadence", block(CLIKMENU_C, 330, 346), [
        "AL1115_ui_Ticks = 1", "G0305_ui_PartyChampionCount",
        "L1119_ps_Champion->CurrentHealth", "F0310_CHAMPION_GetMovementTicks",
        "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks",
        "G0311_i_ProjectileDisabledMovementTicks = 0",
    ]); notes.append("CLIKMENU.C:330-346 successful party steps set disabled movement ticks from max living champion and clear projectile gate")
    require("CHAMPION.C:1180-1215 movement tick formula", block(CHAMPION_C, 1180, 1215), [
        "F0310_CHAMPION_GetMovementTicks", "F0309_CHAMPION_GetMaximumLoad",
        "MaximumLoad", "P0648_ps_Champion->Load", "MASK0x0020_WOUND_FEET",
        "C194_ICON_ARMOUR_BOOT_OF_SPEED", "return L0933_ui_Ticks",
    ]); notes.append("CHAMPION.C:1180-1215 source-locks load/wound/Boots-of-Speed movement cadence formula")
    require("MOVESENS.C:752-775 last movement time", block(MOVESENS_C, 752, 775), [
        "L0725_B_PartySquare", "P0557_T_Thing == C0xFFFF_THING_PARTY",
        "G0305_ui_PartyChampionCount", "G0313_ul_GameTime - G0362_l_LastPartyMovementTime",
        "G0362_l_LastPartyMovementTime = G0313_ul_GameTime", "G0407_s_Party.ScentCount++",
    ]); notes.append("MOVESENS.C:752-775 last party movement time changes only for real party square moves with champions")
    return notes

def verify_firestaff() -> list[str]:
    c = COMPAT_C.read_text()
    t = TEST_C.read_text()
    for marker in [
        "COMMAND.C:2095-2100", "COMMAND.C:2118-2127", "CLIKMENU.C:256-269",
        "CLIKMENU.C:330-346", "CHAMPION.C:1180-1215", "MOVESENS.C:752-775",
    ]:
        if marker not in c:
            raise AssertionError(f"compat source evidence missing {marker}")
    require_regex("party max-living cadence", c, r"ticks\s*=\s*1;.*champion->hp\.current\s*==\s*0.*continue;.*championTicks.*if \(championTicks > ticks\)")
    require_regex("successful step clears projectile gate", c, r"disabledMovementTicks.*ComputePartyStepTicks.*projectileDisabledMovementTicks\s*=\s*0")
    require_regex("last movement time square-change/champion guard", c, r"party->championCount > 0.*party->mapIndex != sourceMapIndex.*lastPartyMovementTime = currentGameTick")
    for needle in [
        "BUG0_72 load equals max uses overloaded cadence",
        "party cadence uses max living champion and ignores dead overweight champion",
        "successful side-step sets disabled movement ticks",
        "successful side-step clears projectile movement ticks",
        "turn bypasses step cadence disabled gate",
        "empty-party successful move does not record scent",
    ]:
        if needle not in t:
            raise AssertionError(f"test missing invariant label {needle!r}")
    return ["Firestaff movement timing helper and probe cover step cadence, turn bypass, side-step, projectile clear, and last-move timing"]

def main() -> None:
    print("probe=dm1_v1_movement_timing_source_lock")
    print(f"sourceRoot={REDMCSB_SOURCE}")
    for item in verify_redmcsb() + verify_firestaff():
        print(f"- {item}")
    print("dm1V1MovementTimingSourceLockOk=1")

if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        sys.exit(1)
