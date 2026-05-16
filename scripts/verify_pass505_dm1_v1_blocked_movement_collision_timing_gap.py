#!/usr/bin/env python3
"""Pass505: source-lock DM1 V1 blocked-step collision timing."""
from __future__ import annotations

import hashlib
import json
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
DM = Path.home() / ".openclaw/data/firestaff-original-games/DM"
GREATSTONE = Path.home() / ".openclaw/data/firestaff-greatstone-atlas"
PASS = "pass505_dm1_v1_blocked_movement_collision_timing_gap"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence" / f"{PASS}.md"

def read(path: Path, encoding: str = "latin-1") -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding=encoding, errors="replace")

def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def function_body(text: str, name: str) -> tuple[int, int, str]:
    m = re.search(rf"^(?:STATICFUNCTION\s+)?(?:void|int16_t|BOOLEAN|unsigned\s+char|int)\s+{re.escape(name)}\s*\(", text, re.M)
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", m.end())
    if brace < 0:
        raise AssertionError(f"missing body for {name}")
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text.count("\n", 0, m.start()) + 1, text.count("\n", 0, i) + 1, text[m.start() : i + 1]
    next_m = re.search(r"^(?:STATICFUNCTION\s+)?(?:void|int16_t|BOOLEAN|unsigned\s+char|int)\s+F\d+_[A-Za-z0-9_]+\s*\(", text[m.start() + 1 :], re.M)
    if next_m:
        end = m.start() + 1 + next_m.start()
        return text.count("\n", 0, m.start()) + 1, text.count("\n", 0, end), text[m.start() : end]
    raise AssertionError(f"unterminated function {name}")

def compact(text: str) -> str:
    return " ".join(text.split())

def require(text: str, needle: str, label: str) -> None:
    if compact(needle) not in compact(text):
        raise AssertionError(f"missing {label}: {needle!r}")

def require_order(text: str, needles: list[str], label: str) -> None:
    flat = compact(text)
    pos = -1
    for needle in needles:
        hit = flat.find(compact(needle), pos + 1)
        if hit < 0:
            raise AssertionError(f"{label}: missing ordered needle {needle!r}")
        pos = hit

def git_out(*args: str) -> str:
    return subprocess.check_output(["git", *args], cwd=ROOT, text=True).strip()

def source_audit() -> dict:
    clik = read(RED / "CLIKMENU.C")
    command = read(RED / "COMMAND.C")
    movesens = read(RED / "MOVESENS.C")
    vblank = read(RED / "VBLANK.C")
    f0366_s, f0366_e, f0366 = function_body(clik, "F0366_COMMAND_ProcessTypes3To6_MoveParty")
    f0380_s, f0380_e, f0380 = function_body(command, "F0380_COMMAND_ProcessQueue_CPSC")
    f0267_s, f0267_e, f0267 = function_body(movesens, "F0267_MOVE_GetMoveResult_CPSCE")
    f0693_s, f0693_e, f0693 = function_body(vblank, "F0693_WaitVerticalBlank")
    require_order(f0380, [
        "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
        "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT)",
        "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
        "G2153_i_QueuedCommandsCount--;",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
    ], "F0380 dequeues a step command before F0366 collision legality")
    require_order(f0366, [
        "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
        "F0325_CHAMPION_DecrementStamina",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "if (L1116_i_SquareType == C00_ELEMENT_WALL)",
        "if (L1116_i_SquareType == C04_ELEMENT_DOOR)",
        "if (L1116_i_SquareType == C06_ELEMENT_FAKEWALL)",
        "if (L1117_B_MovementBlocked)",
        "F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage",
        "F0175_GROUP_GetThing",
        "F0209_GROUP_ProcessEvents29to41",
        "F0357_COMMAND_DiscardAllInput();",
        "F0693_WaitVerticalBlank();",
        "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
        "return;",
        "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);",
        "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
        "G0311_i_ProjectileDisabledMovementTicks = 0;",
    ], "F0366 blocked collision path returns before successful movement timing")
    require(f0267, "G0362_l_LastPartyMovementTime = G0313_ul_GameTime;", "F0267 true-step timestamp side effect")
    require(f0267, "F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX", "F0267 source leave sensor side effect")
    require(f0267, "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX", "F0267 destination enter sensor side effect")
    require_order(f0693, [
        "void F0693_WaitVerticalBlank",
        "L0901B9:",
        "BNE        L0901B9",
    ], "PC-family blocked movement waits on the common vertical blank primitive")
    return {
        "COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC": [f0380_s, f0380_e],
        "CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty": [f0366_s, f0366_e],
        "MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE": [f0267_s, f0267_e],
        "VBLANK.C:F0693_WaitVerticalBlank": [f0693_s, f0693_e],
    }

def data_audit() -> dict:
    dungeon = DM / "_canonical/dm1/DUNGEON.DAT"
    graphics = DM / "_canonical/dm1/GRAPHICS.DAT"
    overview = GREATSTONE / "raw/greatstone.free.fr__dm__g_dm.html.html"
    map_page = GREATSTONE / "raw/greatstone.free.fr__dm__db_data__dm_pc_34__dungeon.dat__dungeon.html.html"
    diff = DM / "_manifests/dm_pc34_greatstone_item_by_item_diff_20260510.json"
    if not map_page.exists():
        map_page = overview
    return {
        "canonicalDm1DungeonDat": {"path": str(dungeon), "sha256": sha256(dungeon)},
        "canonicalDm1GraphicsDat": {"path": str(graphics), "sha256": sha256(graphics)},
        "greatstoneOverview": {"path": str(overview), "sha256": sha256(overview)},
        "greatstoneMapOrOverviewAnchor": {"path": str(map_page), "sha256": sha256(map_page)},
        "greatstonePc34DiffManifest": {"path": str(diff), "sha256": sha256(diff)},
    }

def firestaff_audit() -> dict:
    core_c = read(ROOT / "src/dm1/dm1_v1_movement_command_core_pc34_compat.c", "utf-8")
    core_h = read(ROOT / "include/dm1_v1_movement_command_core_pc34_compat.h", "utf-8")
    test = read(ROOT / "tests/test_dm1_v1_command_movement_sensor_timing_pc34_compat.c", "utf-8")
    cmake = read(ROOT / "CMakeLists.txt", "utf-8")
    for needle in [
        "dm1_v1_apply_pre_step_stamina_cost(party, outResult);",
        "F0702_MOVEMENT_TryMove_Compat(dungeon, party, action, &outResult->movement)",
        "outResult->inputDiscardRequested = 1;",
        "outResult->blockedMovementVblankWaitRequested = 1;",
        "DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat(queue);",
        "DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat",
        "outResult->viewportRedrawRequested = 1;",
    ]:
        require(core_c, needle, f"command-core seam {needle}")
    for needle in ["blockedMovementVblankWaitRequested", "inputDiscardRequested", "viewportRedrawRequested"]:
        require(core_h, needle, f"result field {needle}")
    for needle in [
        "core blocked wall skips sensors",
        "core blocked wall skips viewport",
        "core blocked wall still applies pre-resolution stamina",
        "core blocked wall discards input",
        "core blocked wall clears queued followup",
        "core group block skips viewport",
        "blocked movement skips timing update",
        "blocked movement preserves last movement time",
    ]:
        require(test, needle, f"integration probe {needle}")
    require(cmake, "test_dm1_v1_command_movement_sensor_timing_pc34_compat", "focused CTest target")
    return {
        "commandCore": "blocked wall/door/fakewall/group returns before successful timing/sensor/viewport path",
        "focusedCTest": "dm1_v1_command_movement_sensor_timing_pc34_compat",
    }

def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": "PASS505_DM1_V1_BLOCKED_MOVEMENT_COLLISION_TIMING_SOURCE_LOCKED",
        "branch": git_out("branch", "--show-current"),
        "head": git_out("rev-parse", "HEAD"),
        "redmcsbRoot": str(RED),
        "sourceAudit": source_audit(),
        "originalDataAudit": data_audit(),
        "firestaffAudit": firestaff_audit(),
        "closedGap": "blocked step/collision timing: a dequeued step can spend stamina and then be blocked, but blocked collision discards queued follow-up input, requests one vblank wait, keeps input wait armed, and returns before F0267 sensor/timestamp/cooldown/viewport side effects",
        "notClaimed": [
            "new original DOS runtime breakpoint hit",
            "pixel parity",
            "creature reaction timeline materialization beyond source-locked request flag",
        ],
    }
    OUT_JSON.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass505 - DM1 V1 blocked movement collision timing gap",
        "",
        f"Status: {manifest['status']}",
        "",
        "## ReDMCSB-first audit",
    ]
    for name, span in manifest["sourceAudit"].items():
        lines.append(f"- {name} - {span}")
    lines += [
        "",
        "## Original data / Greatstone anchors",
        f"- DUNGEON.DAT sha256 {manifest['originalDataAudit']['canonicalDm1DungeonDat']['sha256']}",
        f"- GRAPHICS.DAT sha256 {manifest['originalDataAudit']['canonicalDm1GraphicsDat']['sha256']}",
        f"- Greatstone overview sha256 {manifest['originalDataAudit']['greatstoneOverview']['sha256']}",
        f"- Greatstone PC34 diff manifest sha256 {manifest['originalDataAudit']['greatstonePc34DiffManifest']['sha256']}",
        "",
        "## Closed gap",
        manifest["closedGap"],
        "",
        "## Not claimed",
    ]
    lines.extend(f"- {item}" for item in manifest["notClaimed"])
    lines.append("")
    OUT_MD.write_text("\n".join(lines), encoding="utf-8")
    print(manifest["status"])
    print(f"manifest={OUT_JSON.relative_to(ROOT)}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
