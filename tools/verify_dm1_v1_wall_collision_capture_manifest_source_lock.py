#!/usr/bin/env python3
"""Source-lock the DM1 V1 wall/collision runtime capture manifest path.

This is a focused integration-facing guard for the TODO rule that wall and
collision reports need exact map/x/y/direction plus capture evidence before
acceptance. It runs the deterministic M11 capture probe, validates the emitted
manifest/screenshots, and binds that evidence to ReDMCSB movement/collision
source anchors and hash-locked DM1 PC data.
"""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
DM = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1"
PASS = "dm1_v1_wall_collision_capture_manifest_source_lock"
STATUS = "DM1_V1_WALL_COLLISION_CAPTURE_MANIFEST_SOURCE_LOCKED"

EXPECTED_HASHES = {
    "DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    "GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
}

EXPECTED_CAPTURES = [
    {
        "label": "01_start_south_1_3",
        "action": "start",
        "party": {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 2},
        "pipeline": {"turnApplied": 0, "stepApplied": 0, "movementBlocked": 0, "viewportDirty": 0},
    },
    {
        "label": "02_turn_right_west_1_3",
        "action": "turn_right",
        "party": {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 3},
        "pipeline": {"command": 2, "turnApplied": 1, "stepApplied": 0, "movementBlocked": 0, "viewportDirty": 1},
    },
    {
        "label": "03_blocked_west_wall_1_3",
        "action": "forward_into_west_wall",
        "party": {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 3},
        "pipeline": {"command": 3, "turnApplied": 0, "stepApplied": 0, "movementBlocked": 1, "viewportDirty": 0},
    },
    {
        "label": "04_forward_south_1_4",
        "action": "turn_left_then_forward_south",
        "party": {"mapIndex": 0, "mapX": 1, "mapY": 4, "direction": 2},
        "pipeline": {"command": 3, "turnApplied": 0, "stepApplied": 1, "movementBlocked": 0, "viewportDirty": 1},
    },
]


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


def require_order(text: str, needles: list[str], label: str) -> None:
    flat = compact(text)
    pos = -1
    for needle in needles:
        hit = flat.find(compact(needle), pos + 1)
        if hit < 0:
            raise AssertionError(f"{label}: missing ordered needle {needle!r}")
        pos = hit


def function_body(text: str, name: str) -> tuple[int, int, str]:
    pattern = re.compile(
        r"\b(?:STATICFUNCTION\s+)?(?:static\s+)?(?:void|int|int16_t|BOOLEAN|unsigned\s+char|unsigned\s+int16_t|struct\s+\w+|const\s+char\*)\s+"
        + re.escape(name)
        + r"\s*\("
    )
    match = pattern.search(text)
    if not match:
        marker = text.find(name + "(")
        if marker < 0:
            raise AssertionError(f"missing function {name}")
        start = text.rfind("\n", 0, marker) + 1
    else:
        start = match.start()
    brace = text.find("{", start)
    if brace < 0:
        raise AssertionError(f"missing body for {name}")
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return line_no(text, start), line_no(text, pos), text[start:pos + 1]
    raise AssertionError(f"unterminated function {name}")


def source_slice(text: str, start_marker: str, end_marker: str | None = None) -> tuple[int, int, str]:
    start = text.find(start_marker)
    if start < 0:
        raise AssertionError(f"missing source marker {start_marker!r}")
    end = text.find(end_marker, start + len(start_marker)) if end_marker else len(text)
    if end < 0:
        raise AssertionError(f"missing source end marker {end_marker!r}")
    return line_no(text, start), line_no(text, end), text[start:end]


def run(cmd: list[str], timeout: int = 180) -> str:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    if proc.returncode != 0:
        raise AssertionError(f"command failed {' '.join(cmd)}:\n{proc.stdout[-4000:]}")
    return proc.stdout.strip()


def source_audit() -> dict[str, str]:
    if not RED.exists():
        raise AssertionError(f"missing ReDMCSB source root: {RED}")
    dungeon = read(RED / "DUNGEON.C", "latin-1")
    clikmenu = read(RED / "CLIKMENU.C", "latin-1")
    movesens = read(RED / "MOVESENS.C", "latin-1")
    dunview = read(RED / "DUNVIEW.C", "latin-1")

    f0150_s, f0150_e, f0150 = source_slice(
        dungeon,
        "void F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "unsigned char F0151_DUNGEON_GetSquare",
    )
    f0366_s, f0366_e, f0366 = source_slice(
        clikmenu,
        "void F0366_COMMAND_ProcessTypes3To6_MoveParty",
        '#include "CLIKCHAM.C"',
    )
    f0267_s, f0267_e, f0267 = source_slice(
        movesens,
        "BOOLEAN F0267_MOVE_GetMoveResult_CPSCE",
        "void F0268_SENSOR_AddEvent",
    )
    f0128_s, f0128_e, f0128 = source_slice(dunview, "void F0128_DUNGEONVIEW_Draw_CPSF", None)

    require_order(
        f0150,
        [
            "*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0254_i_StepsForwardCount",
            "P0253_i_Direction += 1, P0253_i_Direction &= 3",
            "*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0255_i_StepsRightCount",
        ],
        "F0150 forward/right relative coordinate update",
    )
    require_order(
        f0366,
        [
            "static int16_t G0465_ai_Graphic561_MovementArrowToStepForwardCount[4] =",
            "1,   /* Forward */",
            "static int16_t G0466_ai_Graphic561_MovementArrowToStepRightCount[4] =",
            "1,    /* Right */",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "if (L1116_i_SquareType == C00_ELEMENT_WALL)",
            "if (L1116_i_SquareType == C04_ELEMENT_DOOR)",
            "if (L1116_i_SquareType == C06_ELEMENT_FAKEWALL)",
            "F0357_COMMAND_DiscardAllInput();",
            "F0693_WaitVerticalBlank();",
            "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
            "return;",
            "F0267_MOVE_GetMoveResult_CPSCE",
            "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
        ],
        "F0366 movement blocked path before accepted F0267/cooldown",
    )
    require_order(
        f0267,
        [
            "if (P0560_i_DestinationMapX >= 0)",
            "if (P0557_T_Thing == C0xFFFF_THING_PARTY)",
            "G0306_i_PartyMapX = P0560_i_DestinationMapX;",
            "G0307_i_PartyMapY = P0561_i_DestinationMapY;",
            "L0718_i_RequiredTeleporterScope = MASK0x0002_SCOPE_OBJECTS_OR_PARTY;",
        ],
        "F0267 accepted party coordinate mutation",
    )
    require_order(
        f0128,
        [
            "void F0128_DUNGEONVIEW_Draw_CPSF(",
            "P0183_i_Direction",
            "P0184_i_MapX",
            "P0185_i_MapY",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 1, 0",
        ],
        "F0128 redraw is parameterized by party direction/map coordinates",
    )
    return {
        "DUNGEON.C:F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement": f"DUNGEON.C:{f0150_s}-{f0150_e}",
        "CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty": f"CLIKMENU.C:{f0366_s}-{f0366_e}",
        "MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE": f"MOVESENS.C:{f0267_s}-{f0267_e}",
        "DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF": f"DUNVIEW.C:{f0128_s}-{f0128_e}",
    }


def asset_audit() -> dict[str, dict[str, object]]:
    rows: dict[str, dict[str, object]] = {}
    runtime_dir = Path(os.environ.get("FIRESTAFF_DATA", str(Path.home() / ".firestaff/data")))
    for name, expected in EXPECTED_HASHES.items():
        canonical = DM / name
        runtime = runtime_dir / name
        for label, path in ((f"canonical_{name}", canonical), (f"runtime_{name}", runtime)):
            if not path.exists():
                raise AssertionError(f"missing {label}: {path}")
            got = sha256(path)
            if got != expected:
                raise AssertionError(f"{label} sha256 {got} != {expected}")
            rows[label] = {"path": str(path), "size": path.stat().st_size, "sha256": got}
    return rows


def find_probe() -> Path:
    name = "firestaff_m11_wall_collision_capture_probe"
    candidates = []
    build_dir = os.environ.get("BUILD_DIR")
    if build_dir:
        candidates.append(Path(build_dir) / name)
    candidates.extend([ROOT / "build" / name, ROOT / "build-wall-collision-capture" / name])
    candidates.extend(sorted(ROOT.glob(f"build*/{name}")))
    for candidate in candidates:
        if candidate.exists():
            return candidate
    raise AssertionError(f"missing built executable {name}")


def validate_ppm(path: Path) -> None:
    if not path.exists():
        raise AssertionError(f"missing screenshot {path}")
    expected_header = b"P6\n320 200\n255\n"
    with path.open("rb") as fh:
        header = fh.read(len(expected_header))
    if header != expected_header:
        raise AssertionError(f"unexpected PPM header for {path}: {header!r}")
    expected_size = len(expected_header) + 320 * 200 * 3
    if path.stat().st_size != expected_size:
        raise AssertionError(f"unexpected PPM size for {path}: {path.stat().st_size} != {expected_size}")


def runtime_capture_audit() -> dict[str, object]:
    probe = find_probe()
    build_dir = Path(os.environ.get("BUILD_DIR", str(ROOT / "build")))
    out_dir = build_dir / PASS
    out_dir.mkdir(parents=True, exist_ok=True)
    data_dir = Path(os.environ.get("FIRESTAFF_DATA", str(Path.home() / ".firestaff/data")))
    output = run([str(probe), str(data_dir), str(out_dir)])
    if "PASS dm1_v1_wall_collision_runtime_capture rows=4" not in output:
        raise AssertionError(f"capture probe did not report pass:\n{output}")
    manifest_path = out_dir / "dm1_v1_wall_collision_runtime_capture.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    if manifest.get("schema") != "firestaff.dm1_v1_wall_collision_runtime_capture.v1":
        raise AssertionError("unexpected capture manifest schema")
    honesty = manifest.get("honesty", "")
    if "not an original DOS pixel-parity claim" not in honesty:
        raise AssertionError("capture manifest must keep the non-DOS-pixel-parity caveat")
    source_evidence = "\n".join(manifest.get("sourceEvidence", []))
    for anchor in ["DUNGEON.C:1371-1391", "CLIKMENU.C:180-347", "MOVESENS.C:438-444", "DUNVIEW.C:8318-8618"]:
        if anchor not in source_evidence:
            raise AssertionError(f"manifest missing source anchor {anchor}")
    captures = manifest.get("captures", [])
    if len(captures) != len(EXPECTED_CAPTURES):
        raise AssertionError(f"expected 4 captures, got {len(captures)}")
    for actual, expected in zip(captures, EXPECTED_CAPTURES):
        if actual.get("label") != expected["label"] or actual.get("action") != expected["action"]:
            raise AssertionError(f"capture label/action mismatch: {actual}")
        for key, value in expected["party"].items():
            if actual.get("party", {}).get(key) != value:
                raise AssertionError(f"{expected['label']} party {key} mismatch: {actual}")
        for key, value in expected["pipeline"].items():
            if actual.get("pipeline", {}).get(key) != value:
                raise AssertionError(f"{expected['label']} pipeline {key} mismatch: {actual}")
        validate_ppm(out_dir / actual["screenshot"])
    return {
        "probe": str(probe),
        "outDir": str(out_dir),
        "stdoutLastLine": output.splitlines()[-1],
        "captureCount": len(captures),
    }


def repository_audit() -> None:
    cmake = read(ROOT / "CMakeLists.txt")
    probe_c = read(ROOT / "probes/m11/firestaff_m11_wall_collision_capture_probe.c")
    evidence = read(ROOT / "parity-evidence/dm1_v1_wall_collision_capture_manifest_source_lock.md")
    for needle in [
        "dm1_v1_wall_collision_runtime_capture",
        "dm1_v1_wall_collision_capture_manifest_source_lock",
        "firestaff_m11_wall_collision_capture_probe",
    ]:
        if needle not in cmake:
            raise AssertionError(f"CMake missing {needle}")
    for needle in [
        "01_start_south_1_3",
        "03_blocked_west_wall_1_3",
        "movementBlocked",
        "DUNGEON.C:1371-1391",
        "CLIKMENU.C:180-347",
    ]:
        if needle not in probe_c:
            raise AssertionError(f"capture probe missing {needle}")
    for needle in [STATUS, "DUNGEON.C:1371-1391", "CLIKMENU.C:180-347", "MOVESENS.C:316-565", "not an original DOS pixel-parity claim"]:
        if needle not in evidence:
            raise AssertionError(f"evidence doc missing {needle}")


def main() -> int:
    anchors = source_audit()
    assets = asset_audit()
    capture = runtime_capture_audit()
    repository_audit()
    print(STATUS)
    print(json.dumps({"anchors": anchors, "assets": assets, "capture": capture}, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
