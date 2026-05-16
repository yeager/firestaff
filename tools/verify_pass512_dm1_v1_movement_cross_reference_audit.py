# NOTE: Updated line ranges after v0.9.0 merge
#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import json
import os
import subprocess
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass512_dm1_v1_movement_cross_reference_audit"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
STATUS = "BLOCKED_PASS512_DM1_V1_MOVEMENT_CROSS_REFERENCE_AUDIT_GREATSTONE_DETAIL_PAGES_MISSING"

RED = Path(os.environ.get("FIRESTAFF_REDMCSB_SOURCE", str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")))
GREATSTONE = Path(os.environ.get("FIRESTAFF_GREATSTONE_ATLAS", str(Path.home() / ".openclaw/data/firestaff-greatstone-atlas")))
ORIGINALS = Path(os.environ.get("FIRESTAFF_DM_ORIGINALS", str(Path.home() / ".openclaw/data/firestaff-original-games/DM")))
CSBWIN = Path(os.environ.get("FIRESTAFF_CSBWIN_SOURCE", str(Path.home() / ".openclaw/data/firestaff-csbwin-source/CSBWin")))
CSB = Path(os.environ.get("FIRESTAFF_CSB_SOURCE", str(Path.home() / ".openclaw/data/firestaff-csb-source/CSB/src")))

SOURCE_LOCKS: list[dict[str, Any]] = [
    {
        "id": "redmcsb-game-loop-input-to-command",
        "root": RED,
        "file": "GAMELOOP.C",
        "lines": "164-219",
        "encoding": "latin-1",
        "claim": "keyboard input is drained through F0361 before F0380 processes the queue in the input wait loop",
        "needles": [
            "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
            "while (M527_IsCharacterInKeyboardBuffer())",
            "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());",
            "F0380_COMMAND_ProcessQueue_CPSC();",
            "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);",
        ],
    },
    {
        "id": "redmcsb-command-table-and-dispatch",
        "root": RED,
        "file": "COMMAND.C",
        "lines": "396-405,636-685,2045-2156",
        "encoding": "latin-1",
        "claim": "mouse/key movement tables map to C001..C006 and F0380 dispatches turn/step commands after cooldown filtering",
        "needles": [
            "C001_COMMAND_TURN_LEFT",
            "C003_COMMAND_MOVE_FORWARD",
            "C002_COMMAND_TURN_RIGHT",
            "C006_COMMAND_MOVE_LEFT",
            "C005_COMMAND_MOVE_BACKWARD",
            "C004_COMMAND_MOVE_RIGHT",
            "G0310_i_DisabledMovementTicks",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
    },
    {
        "id": "redmcsb-turn-step-collision-timing",
        "root": RED,
        "file": "CLIKMENU.C",
        "lines": "142-347",
        "encoding": "latin-1",
        "claim": "F0365/F0366 own turning, relative stepping, collision, stairs, F0267 handoff, and movement cooldown",
        "needles": [
            "F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "F0357_COMMAND_DiscardAllInput();",
            "F0267_MOVE_GetMoveResult_CPSCE",
            "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
        ],
    },
    {
        "id": "redmcsb-coordinate-sensor-commit",
        "root": RED,
        "file": "MOVESENS.C",
        "lines": "438-497,760-818,1553-1794",
        "encoding": "latin-1",
        "claim": "F0267 commits party X/Y and source-before-destination movement sensors; direction-gated sensors read G0308",
        "needles": [
            "G0306_i_PartyMapX = P0560_i_DestinationMapX;",
            "G0307_i_PartyMapY = P0561_i_DestinationMapY;",
            "G0362_l_LastPartyMovementTime = G0313_ul_GameTime;",
            "F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX",
            "F0175_GROUP_GetThing(G0306_i_PartyMapX, G0307_i_PartyMapY)",
            "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX",
            "L0768_B_TriggerSensor = (L0779_i_SensorData == M000_INDEX_TO_ORDINAL(G0308_i_PartyDirection));",
        ],
    },
    {
        "id": "redmcsb-movement-to-view-tuple",
        "root": RED,
        "file": "DUNVIEW.C",
        "lines": "8318-8338,8468-8542",
        "encoding": "latin-1",
        "claim": "F0128 consumes direction/mapX/mapY and derives visible squares with F0150",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "REGISTER int16_t P0183_i_Direction",
            "REGISTER int16_t P0184_i_MapX",
            "REGISTER int16_t P0185_i_MapY",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, -1",
            "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
        ],
    },
    {
        "id": "csbwin-secondary-turn-step",
        "root": CSBWIN,
        "file": "CSBCode.cpp",
        "lines": "7845-8337",
        "encoding": "utf-8",
        "claim": "CSBWin keeps TurnParty/MoveParty around partyFacing, MoveObject, d.partyX/Y, and partyMoveDisableTimer",
        "needles": [
            "RESTARTABLE _TurnParty",
            "SetPartyFacing((d.partyFacing + deltaFacing) & 3);",
            "MOVEBUTN *MoveParty",
            "d.partyMoveDisableTimer = (i16)partyMoveData.delay;",
            "MoveObject(RN(RNnul),d.partyX,d.partyY,newX,newY,NULL, NULL);",
            "d.partyMoveDisableTimer = time2move;",
        ],
    },
    {
        "id": "csbwin-secondary-moveobject",
        "root": CSBWIN,
        "file": "Code11f52.cpp",
        "lines": "2792-2866,2956-2960",
        "encoding": "utf-8",
        "claim": "CSBWin MoveObject treats RNnul as party and commits d.partyX/Y",
        "needles": ["i16 MoveObject", "MPO_party", "d.partyX = sw(newX);", "d.partyY = sw(newY);"],
    },
    {
        "id": "csbwin-secondary-drawviewport",
        "root": CSBWIN,
        "file": "Viewport.cpp",
        "lines": "6694-7170",
        "encoding": "utf-8",
        "claim": "CSBWin DrawViewport consumes facing/x/y and marks the viewport updated",
        "needles": ["void DrawViewport(i32 facing,i32 x,i32 y)", "DrawViewportCount++;", "MarkViewportUpdated(D0W);"],
    },
    {
        "id": "csb-secondary-turn-step",
        "root": CSB,
        "file": "CSBCode.cpp",
        "lines": "7868-8360",
        "encoding": "utf-8",
        "claim": "CSB lineage preserves the same TurnParty/MoveParty party tuple and movement-disable semantics",
        "needles": [
            "RESTARTABLE _TurnParty",
            "SetPartyFacing((d.partyFacing + deltaFacing) & 3);",
            "MOVEBUTN *MoveParty",
            "d.partyMoveDisableTimer = (i16)partyMoveData.delay;",
            "MoveObject(RN(RNnul),d.partyX,d.partyY,newX,newY,NULL, NULL);",
            "d.partyMoveDisableTimer = time2move;",
        ],
    },
    {
        "id": "csb-secondary-moveobject",
        "root": CSB,
        "file": "Code11f52.cpp",
        "lines": "2780-2920,3011-3012",
        "encoding": "utf-8",
        "claim": "CSB lineage MoveObject commits d.partyX/Y for RNnul movement/teleport paths",
        "needles": ["i16 MoveObject", "MPO_party", "d.partyX = sw(newX);", "d.partyY = sw(newY);"],
    },
    {
        "id": "csb-secondary-drawviewport",
        "root": CSB,
        "file": "Viewport.cpp",
        "lines": "6701-7176",
        "encoding": "utf-8",
        "claim": "CSB lineage DrawViewport consumes facing/x/y and marks viewport update",
        "needles": ["void DrawViewport(i32 facing,i32 x,i32 y)", "DrawViewportCount++;", "MarkViewportUpdated(D0W);"],
    },
]

EXPECTED_HASHES = {
    "dm1/DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    "dm1/GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
    "dm1/TITLE": "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745",
}

PRIOR_GATES = {
    "pass509_n2_reference_anchor": (ROOT / "parity-evidence/verification/pass509_dm1_v1_movement_n2_reference_anchor/manifest.json", "PASS509_DM1_V1_MOVEMENT_N2_REFERENCE_ANCHORED"),
    "pass511_original_route_contract": (ROOT / "parity-evidence/verification/pass511_dm1_v1_movement_original_route_contract/manifest.json", "PASS511_DM1_V1_MOVEMENT_ORIGINAL_ROUTE_CONTRACT_LOCKED"),
    "pass508_remaining_gap": (ROOT / "parity-evidence/verification/pass508_dm1_v1_movement_remaining_gap_after_pass374/manifest.json", "BLOCKED_PASS508_DM1_V1_MOVEMENT_REMAINING_ORIGINAL_OVERLAY_GAP_PROVED"),
}

def norm(text: str) -> str:
    return " ".join(text.split())

def read_text(path: Path, encoding: str = "utf-8") -> str:
    if not path.is_file():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding=encoding, errors="replace")

def source_block(root: Path, file_name: str, ranges: str, encoding: str) -> str:
    lines = read_text(root / file_name, encoding).splitlines()
    out: list[str] = []
    for span in ranges.split(","):
        start, end = [int(x) for x in span.split("-", 1)]
        if start < 1 or end > len(lines):
            raise AssertionError(f"{root / file_name}:{span} outside file length {len(lines)}")
        out.extend(lines[start - 1:end])
    return "\n".join(out)

def verify_sources() -> list[dict[str, Any]]:
    rows = []
    for item in SOURCE_LOCKS:
        block = norm(source_block(item["root"], item["file"], item["lines"], item["encoding"]))
        missing = [needle for needle in item["needles"] if norm(needle) not in block]
        if missing:
            raise AssertionError("{}:{} missing {}".format(item["root"] / item["file"], item["lines"], missing))
        rows.append({"id": item["id"], "path": str(item["root"] / item["file"]), "lines": item["lines"], "claim": item["claim"]})
    return rows

def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()

def verify_originals() -> list[dict[str, Any]]:
    rows = []
    readme = read_text(ORIGINALS / "_canonical/dm1/README.md")
    for rel, expected in EXPECTED_HASHES.items():
        path = ORIGINALS / "_canonical" / rel
        got = sha256(path)
        if got != expected:
            raise AssertionError(f"{path} sha256 {got} != {expected}")
        if path.name not in readme:
            raise AssertionError(f"canonical README does not name {path.name}")
        rows.append({"path": str(path), "sha256": got, "size": path.stat().st_size})
    return rows

def verify_greatstone() -> dict[str, Any]:
    diff = ORIGINALS / "_manifests/dm_pc34_greatstone_item_by_item_diff_20260510.json"
    data = json.loads(read_text(diff))
    summary = data["summary"]
    if summary["result"] != "PASS" or summary["total_mismatches"] != 0:
        raise AssertionError(f"Greatstone diff not clean: {summary}")
    comparisons = data["comparisons"]
    required = {
        comparisons["pc34_graphics"]["url"],
        comparisons["pc34_dungeon"]["url"],
    }
    pages = json.loads(read_text(GREATSTONE / "index/pages.json"))
    urls = {row["url"] for row in pages}
    if "http://greatstone.free.fr/dm/g_dm.html" not in urls:
        raise AssertionError("Greatstone overview page missing from page index")
    return {"diff": str(diff), "summary": summary, "requiredUrls": sorted(required), "overviewUrl": "http://greatstone.free.fr/dm/g_dm.html"}

def load_json(path: Path) -> dict[str, Any]:
    return json.loads(read_text(path))

def verify_prior_gates() -> list[dict[str, Any]]:
    rows = []
    for name, (path, expected) in PRIOR_GATES.items():
        status = load_json(path).get("status") if path.is_file() else None
        if status != expected:
            raise AssertionError(f"{name} status {status!r} != {expected!r}")
        rows.append({"id": name, "path": str(path.relative_to(ROOT)), "status": status})
    return rows

def run(cmd: list[str]) -> str:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=60)
    if proc.returncode != 0:
        raise AssertionError("command failed " + " ".join(cmd) + ":\n" + proc.stdout[-3000:])
    return proc.stdout.strip()

def write_report(manifest: dict[str, Any]) -> None:
    lines = [
        "# Pass512 - DM1 V1 movement cross-reference audit",
        "",
        "Status: {}".format(manifest["status"]),
        "",
        "Scope: DM1 V1 movement/förflyttning only. This is evidence, not a runtime behavior change.",
        "",
        "## Primary ReDMCSB locks",
        "",
    ]
    for row in manifest["sourceAudit"]:
        if "firestaff-redmcsb-source" in row["path"]:
            lines.append("- PASS {}:{} - {}".format(row["path"], row["lines"], row["claim"]))
    lines += ["", "## Secondary source cross-checks", ""]
    for row in manifest["sourceAudit"]:
        if "firestaff-redmcsb-source" not in row["path"]:
            lines.append("- PASS {}:{} - {}".format(row["path"], row["lines"], row["claim"]))
    lines += ["", "## Data anchors", ""]
    for row in manifest["originalAnchors"]:
        lines.append("- PASS {} sha256 {}".format(row["path"], row["sha256"]))
    g = manifest["greatstoneAnchor"]
    lines.append("- PASS {} result {} with {} mismatches.".format(g["diff"], g["summary"]["result"], g["summary"]["total_mismatches"]))
    if g.get("missingRequiredUrls"):
        lines.append("- BLOCKED Greatstone detail pages missing from local index: {}".format(", ".join(g["missingRequiredUrls"])))
    lines += ["", "## Current blocker evidence", ""]
    for row in manifest["priorGates"]:
        lines.append("- PASS {} status {}".format(row["path"], row["status"]))
    lines += [
        "",
        "Current blocker remains original-runtime evidence: a non-static PC/I34E keyboard-buffer/F0380 route transcript and representative original movement/HUD/viewport overlay captures tied to before/after party tuple.",
        "",
        "Not claimed: pixel parity, viewport rendering changes, binary-level F0380 body breakpoint, or route promotion from Firestaff-only evidence.",
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")

def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": STATUS,
        "scope": "DM1 V1 movement input->command->movement->view tuple evidence",
        "redmcsbRoot": str(RED),
        "greatstoneRoot": str(GREATSTONE),
        "originalsRoot": str(ORIGINALS),
        "csbwinRoot": str(CSBWIN),
        "csbRoot": str(CSB),
        "sourceAudit": verify_sources(),
        "originalAnchors": verify_originals(),
        "greatstoneAnchor": verify_greatstone(),
        "priorGates": verify_prior_gates(),
        "remainingBlocker": "original PC/I34E keyboard-buffer/F0380 route transcript, representative original movement/HUD/viewport overlay captures, and local Greatstone PC34 detail-page index completion",
        "notClaimed": ["pixel parity", "viewport rendering edits", "binary-level direct F0380 body breakpoint", "promotion from Firestaff-only movement evidence"],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(json.dumps({"status": STATUS, "manifest": str(MANIFEST.relative_to(ROOT)), "report": str(REPORT.relative_to(ROOT)), "sourceChecks": len(manifest["sourceAudit"])}, indent=2, sort_keys=True))

if __name__ == "__main__":
    raise SystemExit(main())
