#!/usr/bin/env python3
"""Pass473 DM1 V1 movement/viewport/wall original capture contract.

Source-locks the next promotable original capture attempt for movement,
viewport, and wall comparison.  This gate deliberately does not reuse stale
frames as parity evidence and does not launch DOSBox; it narrows the click
primitive and source-stop boundaries that a live N2 capture must prove.
"""
from __future__ import annotations

import hashlib
import json
import os
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass473_dm1_v1_movement_viewport_wall_capture_contract"
STATUS = "PASS473_SOURCE_STOP_CLICK_PRIMITIVE_CAPTURE_CONTRACT_READY"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
REDMCSB = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))
CANON_DM1 = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1"

SOURCE_LOCKS: list[dict[str, Any]] = [
    {
        "id": "pc34_absolute_movement_click_boxes",
        "file": "COMMAND.C",
        "lines": "106-114",
        "needles": [
            "G0448_as_Graphic561_SecondaryMouseInput_Movement",
            "C001_COMMAND_TURN_LEFT,             234, 261, 125, 145",
            "C003_COMMAND_MOVE_FORWARD,          263, 289, 125, 145",
            "C002_COMMAND_TURN_RIGHT,            291, 318, 125, 145",
            "C006_COMMAND_MOVE_LEFT,             234, 261, 147, 167",
            "C005_COMMAND_MOVE_BACKWARD,         263, 289, 147, 167",
            "C004_COMMAND_MOVE_RIGHT,            291, 318, 147, 167",
            "C080_COMMAND_CLICK_IN_DUNGEON_VIEW,   0, 223,  33, 168",
        ],
        "claim": "PC34 live capture can use absolute left-click centers for movement and viewport commands; arbitrary xdotool points are not evidence.",
    },
    {
        "id": "i34e_zone_movement_dispatch",
        "file": "COMMAND.C",
        "lines": "397-403",
        "needles": [
            "C001_COMMAND_TURN_LEFT", "C068_ZONE_TURN_LEFT",
            "C003_COMMAND_MOVE_FORWARD", "C070_ZONE_MOVE_FORWARD",
            "C002_COMMAND_TURN_RIGHT", "C069_ZONE_TURN_RIGHT",
            "C080_COMMAND_CLICK_IN_DUNGEON_VIEW", "C007_ZONE_VIEWPORT",
        ],
        "claim": "I34E source dispatch maps mouse input through the same movement/viewport command space.",
    },
    {
        "id": "command_queue_reaches_turn_step_handlers",
        "file": "COMMAND.C",
        "lines": "2045-2156",
        "needles": [
            "F0380_COMMAND_ProcessQueue_CPSC",
            "F0360_COMMAND_ProcessPendingClick",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty",
        ],
        "claim": "Capture labels are source-valid only after the queued command reaches turn/step handlers, not at click/highlight time.",
    },
    {
        "id": "turn_mutates_direction_before_capture",
        "file": "CLIKMENU.C",
        "lines": "142-174",
        "needles": [
            "F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE",
            "F0284_CHAMPION_SetPartyDirection",
            "F0276_SENSOR_ProcessThingAdditionOrRemoval",
        ],
        "claim": "Turn captures must stop after source direction mutation and sensor re-entry, before the next label is accepted.",
    },
    {
        "id": "step_mutates_position_and_cooldown_before_capture",
        "file": "CLIKMENU.C",
        "lines": "180-347",
        "needles": [
            "F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "F0267_MOVE_GetMoveResult_CPSCE",
            "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks",
        ],
        "claim": "Step captures must stop after legality, move-result, and cooldown mutation; blocked moves are a separate non-movement label.",
    },
    {
        "id": "relative_step_vector_source",
        "file": "DUNGEON.C",
        "lines": "1371-1392",
        "needles": [
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "G0233_ai_Graphic559_DirectionToStepEastCount",
            "G0234_ai_Graphic559_DirectionToStepNorthCount",
            "Simulate turning right",
        ],
        "claim": "Movement/viewport labels must bind to source relative-step vectors, not emulator visual guesses.",
    },
    {
        "id": "viewport_base_and_wall_composite",
        "file": "DUNVIEW.C",
        "lines": "2962-3110",
        "needles": [
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling",
            "G0296_puc_Bitmap_Viewport",
            "F0100_DUNGEONVIEW_DrawWallSetBitmap",
            "F0102_DUNGEONVIEW_DrawDoorBitmap",
            "F0103_DUNGEONVIEW_DrawDoorFrameBitmapFlippedHorizontally",
        ],
        "claim": "Wall evidence must be captured after the viewport base is cleared and wall/door bitmaps are composited into G0296.",
    },
    {
        "id": "f0128_draws_tuple_to_viewport",
        "file": "DUNVIEW.C",
        "lines": "8318-8611",
        "needles": [
            "F0128_DUNGEONVIEW_Draw_CPSF",
            "P0183_i_Direction", "P0184_i_MapX", "P0185_i_MapY",
            "F0127_DUNGEONVIEW_DrawSquareD0C",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)",
        ],
        "claim": "The comparable viewport is the F0128 draw for the mutated direction/X/Y tuple followed by presentation.",
    },
    {
        "id": "pc34_viewport_present_seam",
        "file": "DRAWVIEW.C",
        "lines": "709-858",
        "needles": [
            "F0097_DUNGEONVIEW_DrawViewport",
            "F0638_GetZone(C007_ZONE_VIEWPORT",
            "G0296_puc_Bitmap_Viewport",
            "VIDRV_09_BlitViewPort",
        ],
        "claim": "The 224x136 crop must be taken after the PC34 viewport present seam, not from setup echo or pre-blit bitmap artifacts.",
    },
]

PRIOR_GATES = [
    ("pass449", "parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/hall_candidate_framebuffer_compare.json"),
    ("pass450", "parity-evidence/verification/pass450_dm1_v1_hall_completion_audit_matrix/manifest.json"),
    ("pass451", "parity-evidence/verification/pass451_dm1_v1_hall_original_capture_storage_policy/manifest.json"),
    ("pass466", "parity-evidence/verification/pass466_dm1_v1_initial_hall_c080_source_stop_capture_path/manifest.json"),
    ("pass472_inputs", "parity-evidence/verification/pass451_dm1_v1_hall_original_capture_storage_policy/manifest.json"),
]

CLICK_PRIMITIVES = [
    {"label": "turn_left", "command": "C001", "xy": [247, 135], "box": [234, 261, 125, 145]},
    {"label": "move_forward", "command": "C003", "xy": [276, 135], "box": [263, 289, 125, 145]},
    {"label": "turn_right", "command": "C002", "xy": [304, 135], "box": [291, 318, 125, 145]},
    {"label": "move_left", "command": "C006", "xy": [247, 157], "box": [234, 261, 147, 167]},
    {"label": "move_backward", "command": "C005", "xy": [276, 157], "box": [263, 289, 147, 167]},
    {"label": "move_right", "command": "C004", "xy": [304, 157], "box": [291, 318, 147, 167]},
    {"label": "hall_portrait_c080", "command": "C080", "xy": [111, 82], "box": [0, 223, 33, 168], "fromPass": "pass466"},
]

CAPTURE_CONTRACT = [
    {"label": "initial_tuple", "stop": "fresh Hall/map tuple decoded from canonical DUNGEON.DAT before movement", "requires": ["pass466 initial state", "pass472 N2 canonical path"]},
    {"label": "post_turn", "primitive": "turn_left or turn_right", "stop": "after CLIKMENU.C F0365 mutates G0308_i_PartyDirection and sensors are processed"},
    {"label": "post_step", "primitive": "move_forward/back/left/right", "stop": "after CLIKMENU.C F0366 calls F0267 and sets G0310_i_DisabledMovementTicks"},
    {"label": "post_viewport_present", "stop": "after DUNVIEW.C F0128 draws G0296 for tuple and DRAWVIEW.C F0097 blits C007_ZONE_VIEWPORT"},
    {"label": "wall_comparator_input", "stop": "use only fresh 224x136 viewport crops from post_viewport_present with locked data hashes; pass376/pass304 frames remain review-only unless recaptured under this contract"},
]


def norm(text: str) -> str:
    return " ".join(text.split())


def source_window(path: Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    chunks: list[str] = []
    for part in spec.split(","):
        if "-" in part:
            start, end = [int(x) for x in part.split("-", 1)]
        else:
            start = end = int(part)
        chunks.append("\n".join(lines[start - 1:end]))
    return "\n".join(chunks)


def digest(path: Path) -> str | None:
    if not path.is_file():
        return None
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def run(cmd: list[str]) -> dict[str, Any]:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False)
    return {"cmd": cmd, "returncode": proc.returncode, "output_tail": proc.stdout[-3000:]}


def audit_sources() -> list[dict[str, Any]]:
    rows = []
    for lock in SOURCE_LOCKS:
        path = REDMCSB / lock["file"]
        text = source_window(path, lock["lines"]) if path.exists() else ""
        missing = [needle for needle in lock["needles"] if norm(needle) not in norm(text)]
        rows.append({**lock, "path": str(path), "exists": path.exists(), "ok": path.exists() and not missing, "missing": missing})
    return rows


def prior_gate_rows() -> list[dict[str, Any]]:
    rows = []
    for label, rel in PRIOR_GATES:
        path = ROOT / rel
        row: dict[str, Any] = {"label": label, "path": rel, "exists": path.exists(), "sha256": digest(path)}
        if path.exists() and path.suffix == ".json":
            try:
                data = json.loads(path.read_text(encoding="utf-8"))
                row["status"] = data.get("status") or data.get("result", {}).get("status")
                row["ok"] = True
            except Exception as exc:
                row.update({"ok": False, "error": str(exc)})
        else:
            row["ok"] = path.exists()
        rows.append(row)
    return rows


def data_rows() -> list[dict[str, Any]]:
    expected = {
        "GRAPHICS.DAT": {"bytes": 363417, "sha256": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e"},
        "DUNGEON.DAT": {"bytes": 33357, "sha256": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85"},
    }
    rows = []
    for name, exp in expected.items():
        path = CANON_DM1 / name
        actual = {"bytes": path.stat().st_size, "sha256": digest(path)} if path.exists() else {}
        rows.append({"name": name, "path": str(path), "exists": path.exists(), "expected": exp, "actual": actual, "ok": path.exists() and actual == exp})
    return rows


def probe_rows() -> list[dict[str, Any]]:
    return [
        run(["./build/firestaff_dm1_v1_canonical_movement_probe"]),
        run(["./build/firestaff_dm1_v1_playable_route_probe"]),
    ]


def write_report(data: dict[str, Any]) -> None:
    lines = [
        "# Pass473 — DM1 V1 movement/viewport/wall capture contract",
        "",
        f"Status: {data['status']}",
        "",
        "Scope: source-stop and click-primitive gate only. No DOSBox run and no pixel parity claim.",
        "",
        "## Next promotable click primitives",
    ]
    for item in CLICK_PRIMITIVES:
        lines.append(f"- {item['label']} {item['command']} center={item['xy']} box={item['box']}")
    lines += ["", "## Source-stop contract"]
    for row in CAPTURE_CONTRACT:
        lines.append(f"- {row['label']} — {row['stop']}")
    lines += ["", "## ReDMCSB locks"]
    for row in data["sources"]:
        lines.append(f"- {row['file']}:{row['lines']} — {row['claim']} ok={row['ok']}")
    lines += ["", "## Prior evidence used as blockers/context only"]
    for row in data["priorGates"]:
        lines.append(f"- {row['label']} {row['path']} exists={row['exists']} status={row.get('status')} sha256={row.get('sha256')}")
    lines += ["", "## Narrowed decision"]
    lines.append("The next live N2 capture must recapture movement/viewport/wall frames using the locked PC34 click centers above, then label frames only after F0365/F0366 state mutation and F0128→F0097 viewport presentation. Existing pass304/pass376/pass449/pass450/pass451/pass466/pass472 artifacts remain context or readiness evidence, not parity inputs for this gate.")
    lines += ["", "## Non-claims", "No live original recapture, no Firestaff-vs-original pixel parity, and no promotion of stale viewport/wall frames is claimed."]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    sources = audit_sources()
    probes = probe_rows()
    data = {
        "pass": PASS,
        "status": STATUS,
        "generatedUtc": datetime.now(timezone.utc).isoformat(),
        "redmcsb": str(REDMCSB),
        "lockedData": data_rows(),
        "clickPrimitives": CLICK_PRIMITIVES,
        "captureContract": CAPTURE_CONTRACT,
        "sources": sources,
        "priorGates": prior_gate_rows(),
        "probes": probes,
    }
    errors = []
    errors += ["source lock failed: %s missing=%s" % (r["id"], r["missing"]) for r in sources if not r["ok"]]
    errors += ["locked data failed: %s" % r["name"] for r in data["lockedData"] if not r["ok"]]
    errors += ["probe failed: %s" % " ".join(r["cmd"]) for r in probes if r["returncode"] != 0]
    data["ok"] = not errors
    data["errors"] = errors
    MANIFEST.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(data)
    if errors:
        print("FAIL", *errors, sep="\n")
        return 1
    print(f"PASS {STATUS}")
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
