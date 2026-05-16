#!/usr/bin/env python3
"""Verify the DM1 V1 projectile/movement interlock is source-locked.

Concrete invariant: in PC34/V1-style source, a thrown object sets a short
projectile movement lock in the party facing direction; the command queue then
blocks only cardinal movement whose absolute direction matches that stored
projectile direction.  Successful party movement clears that projectile lock;
the gameloop decrements it once per tick.  This is deliberately narrower than
plain square legality.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED_ROOT = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
SRC = {
    "CHAMPION.C": RED_ROOT / "CHAMPION.C",
    "COMMAND.C": RED_ROOT / "COMMAND.C",
    "GAMELOOP.C": RED_ROOT / "GAMELOOP.C",
    "CLIKMENU.C": RED_ROOT / "CLIKMENU.C",
}
ORCH_C = ROOT / "src/memory/memory_tick_orchestrator_pc34_compat.c"
PROBE_C = ROOT / "probes/firestaff_m10_tick_orchestrator_probe.c"
MOVE_LOCK = ROOT / "tools/verify_dm1_v1_movement_source_lock.py"
INPUT_LOCK = ROOT / "tools/verify_dm1_v1_input_command_queue_source_lock.py"


def block(path: Path, start: int, end: int) -> str:
    lines = path.read_text(encoding="latin-1").splitlines()
    return "\n".join(lines[start - 1:end])


def compact(text: str) -> str:
    return " ".join(text.split())


def require(cite: str, text: str, needles: list[str]) -> None:
    hay = compact(text)
    for needle in needles:
        if compact(needle) not in hay:
            raise AssertionError(f"{cite}: missing {needle!r}")


def require_re(cite: str, text: str, pattern: str) -> None:
    if not re.search(pattern, text, re.S):
        raise AssertionError(f"{cite}: missing pattern {pattern!r}")


def main() -> int:
    citations: list[str] = []

    # ReDMCSB audit, source first.
    require("CHAMPION.C:2051-2071", block(SRC["CHAMPION.C"], 2051, 2071), [
        "void F0326_CHAMPION_ShootProjectile",
        "F0212_PROJECTILE_Create",
        "BUG0_46 You can run into a projectile shot by a champion",
        "G0311_i_ProjectileDisabledMovementTicks = 4",
        "G0312_i_LastProjectileDisabledMovementDirection = L0990_ui_Direction",
    ])
    citations.append("CHAMPION.C:2051-2071 shoot projectile note/optional lock path")

    require("CHAMPION.C:2183-2193", block(SRC["CHAMPION.C"], 2183, 2193), [
        "F0304_CHAMPION_AddSkillExperience",
        "F0212_PROJECTILE_Create",
        "M021_NORMALIZE(G0308_i_PartyDirection + P0685_i_Side)",
        "G0311_i_ProjectileDisabledMovementTicks = 4",
        "G0312_i_LastProjectileDisabledMovementDirection = G0308_i_PartyDirection",
        "return C1_TRUE",
    ])
    citations.append("CHAMPION.C:2183-2193 thrown object sets 4-tick party-facing projectile movement lock")

    require("COMMAND.C:2095-2100", block(SRC["COMMAND.C"], 2095, 2100), [
        "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command",
        "L1160_i_Command >= C003_COMMAND_MOVE_FORWARD",
        "L1160_i_Command <= C006_COMMAND_MOVE_LEFT",
        "G0311_i_ProjectileDisabledMovementTicks",
        "G0312_i_LastProjectileDisabledMovementDirection == (M021_NORMALIZE(G0308_i_PartyDirection + L1160_i_Command - C003_COMMAND_MOVE_FORWARD))",
        "F0360_COMMAND_ProcessPendingClick",
    ])
    citations.append("COMMAND.C:2095-2100 projectile lock blocks only matching absolute cardinal movement")

    require("COMMAND.C:2104-2110", block(SRC["COMMAND.C"], 2104, 2110), [
        "G0310_i_DisabledMovementTicks || (G0311_i_ProjectileDisabledMovementTicks",
        "G0312_i_LastProjectileDisabledMovementDirection == (M021_NORMALIZE",
        "goto T0380042",
    ])
    citations.append("COMMAND.C:2104-2110 alternate queue path applies the same projectile movement gate")

    require("CLIKMENU.C:330-346", block(SRC["CLIKMENU.C"], 330, 346), [
        "AL1115_ui_Ticks = 1",
        "F0310_CHAMPION_GetMovementTicks",
        "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks",
        "G0311_i_ProjectileDisabledMovementTicks = 0",
    ])
    citations.append("CLIKMENU.C:330-346 successful party movement clears projectile movement lock")

    require("GAMELOOP.C:150-155", block(SRC["GAMELOOP.C"], 150, 155), [
        "if (G0310_i_DisabledMovementTicks)",
        "G0310_i_DisabledMovementTicks--",
        "if (G0311_i_ProjectileDisabledMovementTicks)",
        "G0311_i_ProjectileDisabledMovementTicks--",
    ])
    citations.append("GAMELOOP.C:150-155 movement locks decrement once per tick")

    # Firestaff focused implementation/probe/source-lock coverage.
    orch = ORCH_C.read_text(encoding="utf-8")
    probe = PROBE_C.read_text(encoding="utf-8")
    move_lock = MOVE_LOCK.read_text(encoding="utf-8")
    input_lock = INPUT_LOCK.read_text(encoding="utf-8")

    require("memory_tick_orchestrator_pc34_compat.c:projectile movement gate", orch, [
        "movement_command_disabled_redmcsb_compat",
        "G0311_i_ProjectileDisabledMovementTicks suppresses only the",
        "G0312_i_LastProjectileDisabledMovementDirection",
        "if (world->disabledMovementTicks > 0) return 1",
        "world->projectileDisabledMovementTicks > 0",
        "lastProjectileDisabledMovementDirection & 3",
        "world->projectileDisabledMovementTicks = 0",
        "if (world->projectileDisabledMovementTicks > 0) world->projectileDisabledMovementTicks--",
    ])
    require_re(
        "memory_tick_orchestrator_pc34_compat.c absolute direction check",
        orch,
        r"absoluteDirection\s*=\s*movement_action_absolute_direction\(world->party\.direction,\s*moveAction\).*lastProjectileDisabledMovementDirection\s*&\s*3\)\s*==\s*absoluteDirection",
    )

    require("firestaff_m10_tick_orchestrator_probe.c:projectile movement interlock probe", probe, [
        "projectileDisabledMovementTicks blocks only matching absolute movement direction",
        "projectileDisabledMovementTicks allows non-matching absolute movement direction",
        "periodic effects decrement projectileDisabledMovementTicks once per tick",
        "successful move sets disabledMovementTicks to max living champion movement ticks and clears projectile cooldown",
    ])

    require("tools/verify_dm1_v1_movement_source_lock.py keeps implementation gate", move_lock, [
        "movement_command_disabled_redmcsb_compat",
        "COMMAND.C:2095-2100 / 2104-2110",
        "projectileDisabledMovementTicks blocks only matching absolute movement direction",
    ])
    require("tools/verify_dm1_v1_input_command_queue_source_lock.py keeps queue gate", input_lock, [
        "COMMAND.C:2045-2156 queue dequeue/gate/dispatch",
        "G0311_i_ProjectileDisabledMovementTicks",
    ])

    print("DM1 V1 projectile/movement interlock source-lock gate passed")
    for item in citations:
        print(f"- {item}")
    print("- Firestaff: memory_tick_orchestrator_pc34_compat.c projectile gate/clear/decrement verified")
    print("- Firestaff: firestaff_m10_tick_orchestrator_probe.c focused projectile movement interlock assertions verified")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)
