#!/usr/bin/env python3
"""Pass590: source-lock DM1 V1 blocked wall/door/fakewall self-damage.

This is a narrow movement-collision follow-up to pass505/pass580. It does not
claim new viewport behavior; it locks the F0366 blocked-collision damage request
that happens before input discard/vblank and before successful movement timing.
"""
from __future__ import annotations

from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass590_dm1_v1_blocked_wall_door_self_damage_source_lock"
STATUS = "PASS590_DM1_V1_BLOCKED_WALL_DOOR_SELF_DAMAGE_SOURCE_LOCKED"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
GREATSTONE = Path.home() / ".openclaw/data/firestaff-greatstone-atlas"
DM = Path.home() / ".openclaw/data/firestaff-original-games/DM"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"


def read(path: Path, encoding: str = "utf-8") -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding=encoding, errors="replace")


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def compact(text: str) -> str:
    return " ".join(text.split())


def require(text: str, needle: str, label: str) -> None:
    if compact(needle) not in compact(text):
        raise AssertionError(f"{label}: missing {needle!r}")


def require_order(text: str, needles: list[str], label: str) -> None:
    flat = compact(text)
    pos = -1
    for needle in needles:
        hit = flat.find(compact(needle), pos + 1)
        if hit < 0:
            raise AssertionError(f"{label}: missing ordered needle {needle!r}")
        pos = hit


def function_body(text: str, name: str) -> tuple[int, int, str]:
    pattern = re.compile(r"\b(?:STATICFUNCTION\s+)?(?:static\s+)?(?:void|int|int16_t|BOOLEAN|unsigned\s+int16_t|struct\s+\w+|const\s+char\*)\s+" + re.escape(name) + r"\s*\(")
    match = pattern.search(text)
    if not match:
        marker = text.find(name + "(")
        if marker < 0:
            raise AssertionError(f"missing function {name}")
        match_start = text.rfind("\n", 0, marker) + 1
    else:
        match_start = match.start()
    brace = text.find("{", match_start)
    if brace < 0:
        raise AssertionError(f"missing body for {name}")
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return line_no(text, match_start), line_no(text, i), text[match_start:i + 1]
    raise AssertionError(f"unterminated function {name}")


def run(cmd: list[str], timeout: int = 180) -> str:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    if proc.returncode != 0:
        raise AssertionError(f"command failed {' '.join(cmd)}:\n{proc.stdout[-4000:]}")
    return proc.stdout.strip()


def git(*args: str) -> str:
    return run(["git", *args], timeout=60)


def find_exe(name: str) -> Path:
    candidates = []
    build_dir = os.environ.get("BUILD_DIR")
    if build_dir:
        candidates.append(Path(build_dir) / name)
    candidates.extend([ROOT / "build-pass590" / name, ROOT / "build" / name])
    candidates.extend(sorted(ROOT.glob(f"build*/{name}")))
    for candidate in candidates:
        if candidate.exists():
            return candidate
    raise AssertionError(f"missing built executable {name}")


def source_audit() -> dict:
    clik = read(RED / "CLIKMENU.C", "latin-1")
    champion = read(RED / "CHAMPION.C", "latin-1")
    f0366_marker = "void F0366_COMMAND_ProcessTypes3To6_MoveParty"
    f0366_start_pos = clik.find(f0366_marker)
    if f0366_start_pos < 0:
        raise AssertionError("missing function F0366_COMMAND_ProcessTypes3To6_MoveParty")
    f0366_end_pos = clik.find('#include "CLIKCHAM.C"', f0366_start_pos)
    if f0366_end_pos < 0:
        raise AssertionError("missing F0366 end anchor")
    f0366_s = line_no(clik, f0366_start_pos)
    f0366_e = line_no(clik, f0366_end_pos) - 1
    f0366 = clik[f0366_start_pos:f0366_end_pos]
    f0321_s, f0321_e, f0321 = function_body(champion, "F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage")
    require_order(f0366, [
        "L1117_B_MovementBlocked = C0_FALSE;",
        "if (L1116_i_SquareType == C00_ELEMENT_WALL)",
        "if (L1116_i_SquareType == C04_ELEMENT_DOOR)",
        "if (L1116_i_SquareType == C06_ELEMENT_FAKEWALL)",
        "if (G0305_ui_PartyChampionCount == 0)",
        "if (L1117_B_MovementBlocked)",
        "F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage(L1124_i_FirstDamagedChampionIndex = F0286_CHAMPION_GetTargetChampionIndex",
        "1, MASK0x0008_WOUND_TORSO | MASK0x0010_WOUND_LEGS, C2_ATTACK_SELF",
        "F0286_CHAMPION_GetTargetChampionIndex(L1121_i_MapX, L1122_i_MapY, M017_NEXT(AL1118_ui_MovementArrowIndex))",
        "F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage(L1125_i_SecondDamagedChampionIndex, 1, MASK0x0008_WOUND_TORSO | MASK0x0010_WOUND_LEGS, C2_ATTACK_SELF)",
        "F0357_COMMAND_DiscardAllInput();",
        "F0693_WaitVerticalBlank();",
        "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
        "return;",
        "F0267_MOVE_GetMoveResult_CPSCE",
        "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
    ], "F0366 blocked wall/door/fakewall damage and blocked-return order")
    require_order(f0321, [
        "if (P0665_ui_AttackType != C0_ATTACK_NORMAL)",
        "if (P0664_ui_AllowedWounds & (1 << AL0976_i_WoundIndex))",
        "case C2_ATTACK_SELF:",
        "L0977_ui_Defense >>= 1;",
        "P0663_i_Attack = F0030_MAIN_GetScaledProduct",
        "M008_SET(G0410_ai_ChampionPendingWounds[P0662_i_ChampionIndex], (1 << M003_RANDOM(8)) & P0664_ui_AllowedWounds);",
    ], "F0321 self-attack uses allowed wound mask")
    return {
        "CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty": {
            "lines": [f0366_s, f0366_e],
            "focusedLines": [278, 323],
            "claim": "wall/door/closed real fakewall blocks request attack=1 self-damage to first and next target cells with torso|legs wound mask, then discard input/vblank/return before F0267/cooldown",
        },
        "CHAMPION.C:F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage": {
            "lines": [f0321_s, f0321_e],
            "focusedLines": [1842, 1911],
            "claim": "non-normal self attacks compute defense from the allowed wound mask and materialize pending wounds through that same mask",
        },
    }


def reference_audit() -> dict:
    required_dirs = {"redmcsb": RED, "greatstone": GREATSTONE, "originalDm": DM}
    missing = [name for name, path in required_dirs.items() if not path.exists()]
    if missing:
        raise AssertionError(f"missing required N2 reference roots: {', '.join(missing)}")
    dungeon = DM / "_canonical/dm1/DUNGEON.DAT"
    graphics = DM / "_canonical/dm1/GRAPHICS.DAT"
    overview = GREATSTONE / "raw/greatstone.free.fr__dm__g_dm.html.html"
    return {
        "canonicalDm1DungeonDat": {"path": str(dungeon), "sha256": sha256(dungeon)},
        "canonicalDm1GraphicsDat": {"path": str(graphics), "sha256": sha256(graphics)},
        "greatstoneOverview": {"path": str(overview), "sha256": sha256(overview)},
    }


def firestaff_audit() -> dict:
    core_c = read(ROOT / "src/dm1/dm1_v1_movement_command_core_pc34_compat.c")
    core_h = read(ROOT / "include/dm1_v1_movement_command_core_pc34_compat.h")
    test = read(ROOT / "tests/test_dm1_v1_movement_command_core_pc34_compat.c")
    cmake = read(ROOT / "CMakeLists.txt")
    f_core_s, f_core_e, core_body = function_body(core_c, "DM1_V1_MovementCommandCore_ProcessOnePc34Compat")
    f_req_s, f_req_e, req_body = function_body(core_c, "dm1_v1_record_blocked_wall_or_door_damage_request")
    require_order(req_body, [
        "firstCell = dm1_v1_normalize_cell(movementArrowIndex + party->direction + 2);",
        "outResult->blockedByWallOrDoorDamageRequested = 1;",
        "outResult->blockedByWallOrDoorDamageAttack = 1;",
        "outResult->blockedByWallOrDoorDamageAttackTypeSelf = 2;",
        "outResult->blockedByWallOrDoorDamageAllowedWounds = 0x0018u;",
        "outResult->blockedByWallOrDoorDamageFirstCell = firstCell;",
        "outResult->blockedByWallOrDoorDamageSecondCell = dm1_v1_normalize_cell(firstCell + 1);",
    ], "Firestaff damage request fields")
    require_order(core_body, [
        "dm1_v1_apply_pre_step_stamina_cost(party, outResult);",
        "if (!F0702_MOVEMENT_TryMove_Compat(dungeon, party, action, &outResult->movement))",
        "dm1_v1_record_blocked_wall_or_door_damage_request(party, action, outResult);",
        "outResult->inputDiscardRequested = 1;",
        "outResult->blockedMovementVblankWaitRequested = 1;",
        "return 1;",
        "if (F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat",
        "outResult->blockedByGroup = 1;",
        "outResult->groupReactionPartyAdjacentRequested = 1;",
    ], "Firestaff blocked wall/door/fakewall before group and before success path")
    for needle in [
        "blockedByWallOrDoorDamageAttack",
        "blockedByWallOrDoorDamageAttackTypeSelf",
        "blockedByWallOrDoorDamageAllowedWounds",
        "blockedByWallOrDoorDamageFirstCell",
        "blockedByWallOrDoorDamageSecondCell",
    ]:
        require(core_h, needle, f"header field {needle}")
    for needle in [
        "blocked wall damage attack is one",
        "blocked wall damage attack type self",
        "blocked wall damage wounds torso legs",
        "pass590 closed door damage attack is one",
        "pass590 closed door damage wounds torso legs",
        "pass590 closed fakewall damage attack type self",
        "pass590 closed fakewall damage second cell",
        "pass547 group no wall damage request",
    ]:
        require(test, needle, f"focused test label {needle}")
    require(cmake, "pass590_dm1_v1_blocked_wall_door_self_damage_source_lock", "CTest gate")
    return {
        "src/dm1/dm1_v1_movement_command_core_pc34_compat.c:dm1_v1_record_blocked_wall_or_door_damage_request": {"lines": [f_req_s, f_req_e]},
        "src/dm1/dm1_v1_movement_command_core_pc34_compat.c:DM1_V1_MovementCommandCore_ProcessOnePc34Compat": {"lines": [f_core_s, f_core_e]},
        "focusedCTest": "dm1_v1_movement_command_core_pc34_compat",
    }


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    exe = find_exe("test_dm1_v1_movement_command_core_pc34_compat")
    test_output = run([str(exe)], timeout=120)
    last_line = test_output.splitlines()[-1]
    if "0 failed" not in last_line and "dm1V1MovementCommandCoreInvariantOk=1" not in last_line:
        raise AssertionError(test_output[-1000:])
    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": STATUS,
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "branch": git("branch", "--show-current"),
        "head": git("rev-parse", "HEAD"),
        "worktree": str(ROOT),
        "redmcsbRoot": str(RED),
        "sourceAudit": source_audit(),
        "referenceAudit": reference_audit(),
        "firestaffAudit": firestaff_audit(),
        "testExecutable": str(exe),
        "testOutputLastLine": last_line,
        "closedGap": "blocked wall/door/closed-real-fakewall self-damage request fields and ordering are source-locked without touching viewport/occlusion rendering",
        "notClaimed": [
            "new original DOS runtime trace",
            "actual random wound materialization parity beyond the recorded request",
            "viewport or wall occlusion behavior",
            "CSB or DM2 movement behavior",
        ],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass590 - DM1 V1 blocked wall/door self-damage source lock",
        "",
        f"Status: {STATUS}",
        f"Manifest: {MANIFEST.relative_to(ROOT)}",
        "",
        "## ReDMCSB Source Audit",
    ]
    for name, detail in manifest["sourceAudit"].items():
        lines.append(f"- {name}: lines {detail['lines'][0]}-{detail['lines'][1]}, focused {detail['focusedLines'][0]}-{detail['focusedLines'][1]} - {detail['claim']}")
    lines.extend([
        "",
        "## Firestaff Gate",
        "- Command core records attack=1, attackType=C2_ATTACK_SELF, allowedWounds=0x0018, first target cell, and next target cell for wall/door/closed-real-fakewall blocks.",
        "- Focused CTest: dm1_v1_movement_command_core_pc34_compat.",
        "",
        "## Reference Anchors",
        f"- DUNGEON.DAT sha256 {manifest['referenceAudit']['canonicalDm1DungeonDat']['sha256']}",
        f"- GRAPHICS.DAT sha256 {manifest['referenceAudit']['canonicalDm1GraphicsDat']['sha256']}",
        f"- Greatstone overview sha256 {manifest['referenceAudit']['greatstoneOverview']['sha256']}",
        "",
        "## Not Claimed",
    ])
    lines.extend(f"- {item}" for item in manifest["notClaimed"])
    lines.append("")
    REPORT.write_text("\n".join(lines), encoding="utf-8")
    print(f"{STATUS} manifest={MANIFEST.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
