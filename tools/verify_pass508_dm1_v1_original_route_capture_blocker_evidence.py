#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import json
from collections import Counter
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
ORIGINAL_PC34 = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
GREATSTONE_ATLAS = Path.home() / ".openclaw/data/firestaff-greatstone-atlas"
GREATSTONE_DIFF = Path.home() / ".openclaw/data/firestaff-original-games/DM/_manifests/dm_pc34_greatstone_item_by_item_diff_20260510.json"
VERIFY_DIR = ROOT / "parity-evidence/verification/pass508_dm1_v1_original_route_capture_blocker_evidence"
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence/pass508_dm1_v1_original_route_capture_blocker_evidence.md"
STATUS = "PASS508_ORIGINAL_ROUTE_CAPTURE_BLOCKER_EVIDENCE_TIGHTENED"

PASS304 = ROOT / "parity-evidence/verification/pass304_dm1_v1_original_viewport_capture_blocker_manifest.json"
PASS308 = ROOT / "parity-evidence/verification/pass308_original_capture_execution_manifest.json"
PASS435 = ROOT / "parity-evidence/verification/pass435_dm1_v1_semantic_original_route_readiness_gate/manifest.json"
PASS487 = ROOT / "parity-evidence/verification/pass487_dm1_v1_original_click_capture_blocker/manifest.json"
PASS497 = ROOT / "parity-evidence/verification/pass497_dm1_v1_original_capture_next_blocker/manifest.json"
PASS498 = ROOT / "parity-evidence/verification/pass498_dm1_v1_original_post_command_state_delta_boundary/manifest.json"

SOURCE_LOCKS = [
    {"id": "entrance_enter_command_reaches_load_dungeon", "file": "COMMAND.C", "lines": "63-72,2428-2456", "needles": ["C200_COMMAND_ENTRANCE_ENTER_DUNGEON", "G0298_B_NewGame = C001_MODE_LOAD_DUNGEON", "M566_COMMAND_ENTRANCE_RESUME"], "claim": "the entrance click route is real source input, not a synthetic label"},
    {"id": "entrance_loop_processes_queue_and_opens_doors", "file": "ENTRANCE.C", "lines": "739-747,850-883,939-944", "needles": ["G0441_ps_PrimaryMouseInput = G0445_as_Graphic561_PrimaryMouseInput_Entrance", "G0298_B_NewGame = C099_MODE_WAITING_ON_ENTRANCE", "F0380_COMMAND_ProcessQueue_CPSC();", "F0438_STARTEND_OpenEntranceDoors();"], "claim": "entrance waits on F0380 and opens doors only after G0298 changes"},
    {"id": "pc34_movement_click_zones", "file": "COMMAND.C", "lines": "106-114", "needles": ["C001_COMMAND_TURN_LEFT", "C003_COMMAND_MOVE_FORWARD", "C002_COMMAND_TURN_RIGHT", "C005_COMMAND_MOVE_BACKWARD", "C080_COMMAND_CLICK_IN_DUNGEON_VIEW"], "claim": "movement/viewport click coordinates must map to these PC34 zones"},
    {"id": "queue_dispatch_to_turn_or_move", "file": "COMMAND.C", "lines": "2045-2156", "needles": ["L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;", "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;", "G2153_i_QueuedCommandsCount--;", "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"], "claim": "a promotable shot must prove the dequeued command, not only host labels"},
    {"id": "turn_handler_mutates_direction", "file": "CLIKMENU.C", "lines": "142-174", "needles": ["void F0365_COMMAND_ProcessTypes1To2_TurnParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;", "F0284_CHAMPION_SetPartyDirection"], "claim": "turn shots need F0365 direction mutation before the redraw"},
    {"id": "move_handler_computes_destination", "file": "CLIKMENU.C", "lines": "180-347", "needles": ["void F0366_COMMAND_ProcessTypes3To6_MoveParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0267_MOVE_GetMoveResult_CPSCE"], "claim": "move shots need F0366 destination/move-result evidence before the redraw"},
    {"id": "game_loop_next_redraw_boundary", "file": "GAMELOOP.C", "lines": "90,164,215-219", "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);", "G0321_B_StopWaitingForPlayerInput = C0_FALSE;", "F0380_COMMAND_ProcessQueue_CPSC();", "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"], "claim": "the next promotable frame is after command wait exits and the next F0128 consumes the tuple"},
    {"id": "viewport_composition_and_present", "file": "DUNVIEW.C", "lines": "8318-8611", "needles": ["void F0128_DUNGEONVIEW_Draw_CPSF", "P0183_i_Direction", "P0184_i_MapX", "P0185_i_MapY", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"], "claim": "F0128 must compose G0296 for the same direction/X/Y tuple"},
    {"id": "pc34_viewport_blit_present", "file": "DRAWVIEW.C", "lines": "709-858", "needles": ["void F0097_DUNGEONVIEW_DrawViewport", "G0296_puc_Bitmap_Viewport", "F0638_GetZone(C007_ZONE_VIEWPORT", "VIDRV_09_BlitViewPort"], "claim": "the crop/pixel seam must be the PC34 viewport present path"},
]
ASSET_LOCKS = [
    {"id": "pc34_executable", "path": ORIGINAL_PC34 / "DM.EXE", "variant": "dm-pc34/DungeonMasterPC34", "claim": "capture route must launch the N2-local PC34 executable variant"},
    {"id": "pc34_dungeon_dat", "path": ORIGINAL_PC34 / "DATA/DUNGEON.DAT", "variant": "dm-pc34/DungeonMasterPC34 DATA", "claim": "route state and map tuple evidence must bind to this exact dungeon.dat"},
    {"id": "pc34_graphics_dat", "path": ORIGINAL_PC34 / "DATA/GRAPHICS.DAT", "variant": "dm-pc34/DungeonMasterPC34 DATA", "claim": "viewport/crop evidence must bind to this exact graphics.dat"},
    {"id": "pc34_title", "path": ORIGINAL_PC34 / "TITLE", "variant": "dm-pc34/DungeonMasterPC34", "claim": "startup/entrance handoff must bind to this exact TITLE asset"},
]

def norm(text: str) -> str:
    return " ".join(text.split())

def source_window(path: Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    chunks = []
    for part in spec.split(","):
        if "-" in part:
            start, end = [int(x) for x in part.split("-", 1)]
        else:
            start = end = int(part)
        chunks.append("\n".join(lines[start - 1:end]))
    return "\n".join(chunks)

def sha256(path: Path) -> str | None:
    if not path.exists():
        return None
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def audit_sources():
    rows = []
    for lock in SOURCE_LOCKS:
        path = RED / lock["file"]
        text = source_window(path, lock["lines"]) if path.exists() else ""
        missing = [needle for needle in lock["needles"] if norm(needle) not in norm(text)]
        rows.append({**lock, "path": str(path), "ok": path.exists() and not missing, "missing": missing})
    return rows

def audit_assets():
    rows = []
    for lock in ASSET_LOCKS:
        path = lock["path"]
        rows.append({
            **lock,
            "path": str(path),
            "exists": path.exists(),
            "bytes": path.stat().st_size if path.exists() else None,
            "sha256": sha256(path),
            "ok": path.exists() and path.is_file(),
        })
    return rows

def audit_greatstone():
    pages = GREATSTONE_ATLAS / "index/pages.json"
    files = GREATSTONE_ATLAS / "index/files.json"
    diff = GREATSTONE_DIFF
    return {
        "atlasRoot": str(GREATSTONE_ATLAS),
        "pagesIndex": str(pages),
        "filesIndex": str(files),
        "pc34DiffManifest": str(diff),
        "pagesIndexExists": pages.exists(),
        "filesIndexExists": files.exists(),
        "pc34DiffManifestExists": diff.exists(),
        "pc34DiffManifestSha256": sha256(diff),
        "ok": GREATSTONE_ATLAS.exists() and pages.exists() and files.exists() and diff.exists(),
        "claim": "Greatstone remains a local secondary atlas/provenance reference; ReDMCSB source and N2-local PC34 asset hashes are the promotion boundary.",
    }

def read_json(path: Path):
    if not path.exists():
        raise SystemExit(f"missing required manifest: {path}")
    return json.loads(path.read_text(encoding="utf-8"))

def route_rows(data):
    rows = data.get("blockerFindings", {}).get("routeCaptureRows", [])
    return [{"index": r.get("index"), "routeLabel": r.get("routeLabel"), "classification": r.get("classification"), "sha256": r.get("sha256"), "filenameMatchesRouteLabel": r.get("filenameMatchesRouteLabel"), "isRepeatedPostEntryStaticHash": r.get("isRepeatedPostEntryStaticHash")} for r in rows]

def build():
    p304 = read_json(PASS304); p308 = read_json(PASS308); p435 = read_json(PASS435)
    p487 = read_json(PASS487); p497 = read_json(PASS497); p498 = read_json(PASS498)
    blockers = p487.get("blockerFindings", {})
    rows = route_rows(p487)
    post_entry_rows = rows[1:]
    hash_counts = Counter(str(row.get("sha256")) for row in rows if row.get("sha256"))
    observed = {
        "pass304Status": p304.get("status"), "pass308Status": p308.get("status"), "pass435Status": p435.get("status"),
        "pass487Status": p487.get("status"), "pass497Status": p497.get("status"), "pass498Status": p498.get("status"),
        "pass304RouteLabelCoverage": p304.get("routeLabelCoverage"), "pass308Coverage": p308.get("coverage"),
        "pass487Classes": p487.get("classes"), "pass487RouteRows": rows, "pass487DuplicateSha256Counts": p487.get("duplicateSha256Counts"),
        "postEntryStaticSha256": blockers.get("postEntryGameplaySha256"), "rawHashCounts": dict(hash_counts),
        "postEntryRowsRepeatStaticHash": all(row.get("isRepeatedPostEntryStaticHash") for row in post_entry_rows),
        "postEntryRegionStatsRepeated": blockers.get("postEntryRegionStatsRepeated") is True,
        "filenameLabelDriftRows": len(blockers.get("filenameLabelDrift", [])), "pass497NextBlocker": p497.get("nextBlocker"), "pass498NarrowedBlocker": p498.get("narrowedBlocker"),
    }
    source_audit = audit_sources()
    asset_audit = audit_assets()
    greatstone_audit = audit_greatstone()
    required = {
        "source_audit_ok": all(row["ok"] for row in source_audit),
        "asset_audit_ok": all(row["ok"] for row in asset_audit),
        "greatstone_local_reference_ok": greatstone_audit["ok"],
        "pass304_still_blocks_route_promotion": p304.get("status") == "BLOCKED_ORIGINAL_PC34_VIEWPORT_CAPTURE_NOT_ROUTE_PROVEN",
        "pass308_records_execution_without_state_oracle": p308.get("status") == "PASS_CAPTURE_EXECUTED_STATE_ORACLE_PENDING",
        "pass435_semantic_route_not_ready": p435.get("status") == "BLOCKED_PASS435_SEMANTIC_ORIGINAL_ROUTE_NOT_READY",
        "pass487_reaches_gameplay_but_static": p487.get("ok") is True and observed["postEntryRowsRepeatStaticHash"],
        "pass497_next_blocker_locked": p497.get("ok") is True and "source-visible post-command state delta" in str(p497.get("nextBlocker", "")),
        "pass498_boundary_locked": p498.get("ok") is True and "post-command state-delta boundary" in str(p498.get("narrowedBlocker", "")),
    }
    problems = [name for name, ok in required.items() if not ok]
    problems.extend("source lock failed {0}:{1} {2}".format(row["file"], row["lines"], row["missing"]) for row in source_audit if not row["ok"])
    return {"schema": "firestaff.parity.pass508_dm1_v1_original_route_capture_blocker_evidence.v1", "status": STATUS if not problems else "FAIL_PASS508_ORIGINAL_ROUTE_CAPTURE_BLOCKER_EVIDENCE", "ok": not problems, "sourceRoot": str(RED), "sourceAudit": source_audit, "originalAssetAudit": asset_audit, "greatstoneAudit": greatstone_audit, "inputs": {"pass304": str(PASS304.relative_to(ROOT)), "pass308": str(PASS308.relative_to(ROOT)), "pass435": str(PASS435.relative_to(ROOT)), "pass487": str(PASS487.relative_to(ROOT)), "pass497": str(PASS497.relative_to(ROOT)), "pass498": str(PASS498.relative_to(ROOT))}, "observed": observed, "required": required, "blocker": "Original DM1 V1 overlay/crop promotion remains blocked only at source-visible post-command state-delta proof. The route reaches gameplay from the hash-locked N2 PC34 asset set, but current post-entry frames repeat the same static hash/region fingerprint and are not bound to F0380 -> F0365/F0366 -> subsequent F0128 -> F0097/VIDRV for each route label.", "nextEvidenceRequired": ["capture each route shot at or after the F0097/VIDRV present boundary following the matching command", "record the F0380 command id/X/Y and the F0365 or F0366 handler reached for that shot", "record the later F0128 direction/X/Y tuple consumed for the same shot", "reject repeated 48ed static gameplay hashes unless source state proves the command was intentionally blocked/no-op"], "nonClaims": ["no DOSBox run launched", "no original-vs-Firestaff pixel parity", "no promotion of pass487 static frames", "no movement or viewport implementation change"], "problems": problems}

def write_report(data):
    lines = [
        "# Pass508 - DM1 V1 original route/capture blocker evidence",
        "",
        "Status: " + str(data["status"]),
        "",
        data["blocker"],
        "",
        "## ReDMCSB source anchors",
        "",
    ]
    for row in data["sourceAudit"]:
        lines.append("- {file}:{lines} - ok={ok}; {claim}".format(**row))
    lines += ["", "## N2 original asset locks", ""]
    for row in data["originalAssetAudit"]:
        lines.append("- {id}: ok={ok}; bytes={bytes}; sha256={sha256}; {claim}".format(**row))
    gs = data["greatstoneAudit"]
    lines += [
        "",
        "## Greatstone local reference",
        "",
        "- atlasRoot: {0}".format(gs["atlasRoot"]),
        "- pagesIndexExists={0}; filesIndexExists={1}; pc34DiffManifestExists={2}; pc34DiffManifestSha256={3}".format(gs["pagesIndexExists"], gs["filesIndexExists"], gs["pc34DiffManifestExists"], gs["pc34DiffManifestSha256"]),
        "- {0}".format(gs["claim"]),
    ]
    observed = data["observed"]
    lines += [
        "",
        "## Current evidence",
        "",
        "- pass304: {0}; routeLabelCoverage={1}".format(observed["pass304Status"], observed["pass304RouteLabelCoverage"]),
        "- pass308: {0}; coverage={1}".format(observed["pass308Status"], observed["pass308Coverage"]),
        "- pass435: {0}".format(observed["pass435Status"]),
        "- pass487: {0}; classes={1}".format(observed["pass487Status"], observed["pass487Classes"]),
        "- pass487 duplicate hashes: {0}".format(observed["pass487DuplicateSha256Counts"]),
        "- post-entry static hash: {0}; repeatedRows={1}; repeatedRegions={2}".format(observed["postEntryStaticSha256"], observed["postEntryRowsRepeatStaticHash"], observed["postEntryRegionStatsRepeated"]),
        "- filename/route-label drift rows: {0}".format(observed["filenameLabelDriftRows"]),
        "- pass497 next blocker: {0}".format(observed["pass497NextBlocker"]),
        "- pass498 narrowed blocker: {0}".format(observed["pass498NarrowedBlocker"]),
        "",
        "## Route rows",
        "",
    ]
    for row in observed["pass487RouteRows"]:
        lines.append("- {index} {routeLabel} {classification} {sha256} filenameMatchesRouteLabel={filenameMatchesRouteLabel} repeatedStatic={isRepeatedPostEntryStaticHash}".format(**row))
    lines += ["", "## Next evidence required", ""]
    lines.extend("- " + item for item in data["nextEvidenceRequired"])
    lines += ["", "## Non-claims", ""]
    lines.extend("- " + item for item in data["nonClaims"])
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")

def main() -> int:
    data = build()
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(data)
    print(data["status"])
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    if data["problems"]:
        for problem in data["problems"]:
            print(problem)
        return 1
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
