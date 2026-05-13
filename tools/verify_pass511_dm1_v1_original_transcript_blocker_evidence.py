#!/usr/bin/env python3
"""Pass511: pin the remaining DM1 V1 original route/capture blocker."""
from __future__ import annotations

import hashlib
import json
import os
import subprocess
from pathlib import Path
from typing import Iterable

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
GREATSTONE = Path.home() / ".openclaw/data/firestaff-greatstone-atlas"
ORIGINALS = Path.home() / ".openclaw/data/firestaff-original-games/DM"
DM1_CANON = ORIGINALS / "_canonical/dm1"

VERIFY_DIR = ROOT / "parity-evidence/verification/pass511_dm1_v1_original_transcript_blocker_evidence"
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence/pass511_dm1_v1_original_transcript_blocker_evidence.md"
STATUS = "BLOCKED_PASS511_ORIGINAL_TRANSCRIPT_STATE_DELTA_REQUIRED"

INPUT_MANIFESTS = {
    "pass435": ROOT / "parity-evidence/verification/pass435_dm1_v1_semantic_original_route_readiness_gate/manifest.json",
    "pass497": ROOT / "parity-evidence/verification/pass497_dm1_v1_original_capture_next_blocker/manifest.json",
    "pass504": ROOT / "parity-evidence/verification/pass504_dm1_v1_original_route_state_delta_diversity_blocker/manifest.json",
    "pass508": ROOT / "parity-evidence/verification/pass508_dm1_v1_original_route_capture_blocker_evidence/manifest.json",
    "pass510": ROOT / "parity-evidence/verification/pass510_dm1_v1_original_capture_route_label_filename_fixture/manifest.json",
}

ROUTE_ARTIFACTS = [
    ROOT / "verification-screens/pass376-original-route/raw_manifest.tsv",
    ROOT / "verification-screens/pass376-original-route/original_viewport_shot_labels.tsv",
    ROOT / "verification-screens/pass376-original-route/pass80_original_frame_classifier.json",
    ROOT / "verification-screens/pass487-n2-original-pc34-click-primitives-route/raw_manifest.tsv",
    ROOT / "verification-screens/pass487-n2-original-pc34-click-primitives-route/original_viewport_shot_labels.tsv",
    ROOT / "verification-screens/pass505-original-overlay-mouse-route-recapture/raw_manifest.tsv",
    ROOT / "verification-screens/pass505-original-overlay-mouse-route-recapture/original_viewport_shot_labels.tsv",
]

TRANSCRIPT_NAME_HINTS = (
    "original_transcript",
    "original-runtime-transcript",
    "original_runtime_transcript",
    "source-visible-transcript",
    "source_visible_transcript",
    "f0380-transcript",
    "f0380_transcript",
    "f0365-transcript",
    "f0365_transcript",
    "f0366-transcript",
    "f0366_transcript",
)

SOURCE_LOCKS = [
    {
        "id": "entrance_click_is_source_command",
        "file": "COMMAND.C",
        "lines": "55-75,2428-2456",
        "needles": ["C200_COMMAND_ENTRANCE_ENTER_DUNGEON", "244, 298,  45,  58", "G0298_B_NewGame = C001_MODE_LOAD_DUNGEON"],
        "claim": "the route enters the dungeon through the source entrance command",
    },
    {
        "id": "pc34_movement_and_viewport_click_boxes",
        "file": "COMMAND.C",
        "lines": "106-121",
        "needles": ["C001_COMMAND_TURN_LEFT", "C003_COMMAND_MOVE_FORWARD", "C002_COMMAND_TURN_RIGHT", "C080_COMMAND_CLICK_IN_DUNGEON_VIEW"],
        "claim": "route input must resolve to PC34 movement or dungeon-view command boxes",
    },
    {
        "id": "command_queue_dispatch_boundary",
        "file": "COMMAND.C",
        "lines": "2029-2058,2138-2162",
        "needles": ["void F0380_COMMAND_ProcessQueue_CPSC", "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command)", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command)"],
        "claim": "each shot needs dequeued command and dispatched turn/move handler proof",
    },
    {
        "id": "turn_handler_sets_stop_wait_and_direction",
        "file": "CLIKMENU.C",
        "lines": "142-174",
        "needles": ["void F0365_COMMAND_ProcessTypes1To2_TurnParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE", "F0284_CHAMPION_SetPartyDirection"],
        "claim": "turn captures must follow source direction mutation",
    },
    {
        "id": "move_handler_sets_stop_wait_and_move_result",
        "file": "CLIKMENU.C",
        "lines": "180-347",
        "needles": ["void F0366_COMMAND_ProcessTypes3To6_MoveParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0267_MOVE_GetMoveResult_CPSCE"],
        "claim": "movement captures must follow target square and move-result handling",
    },
    {
        "id": "game_loop_redraw_after_stop_wait",
        "file": "GAMELOOP.C",
        "lines": "80-98,156-220",
        "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)", "F0380_COMMAND_ProcessQueue_CPSC();", "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking)"],
        "claim": "the redraw must consume the post-command party tuple after wait exit",
    },
    {
        "id": "viewport_composition_for_tuple",
        "file": "DUNVIEW.C",
        "lines": "8318-8620",
        "needles": ["void F0128_DUNGEONVIEW_Draw_CPSF", "P0183_i_Direction", "P0184_i_MapX", "P0185_i_MapY", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)"],
        "claim": "viewport bitmap must be composed for a known direction/X/Y tuple",
    },
    {
        "id": "pc34_present_boundary",
        "file": "DRAWVIEW.C",
        "lines": "709-858",
        "needles": ["void F0097_DUNGEONVIEW_DrawViewport", "G0296_puc_Bitmap_Viewport", "F0638_GetZone(C007_ZONE_VIEWPORT", "VIDRV_09_BlitViewPort"],
        "claim": "the crop seam must be the PC34 viewport present/blit boundary",
    },
    {
        "id": "dm1_initial_hall_tuple",
        "file": "LOADSAVE.C",
        "lines": "1936-1946",
        "needles": ["G0278_ps_DungeonHeader->InitialPartyLocation", "G0306_i_PartyMapX", "G0307_i_PartyMapY", "G0308_i_PartyDirection", "G0309_i_PartyMapIndex = 0"],
        "claim": "DM1 V1 starts from the DUNGEON.DAT initial Hall tuple",
    },
]


def norm(text: str) -> str:
    return " ".join(text.split())


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def read_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8")) if path.exists() else {"missing": str(path)}


def source_window(path: Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    chunks: list[str] = []
    for part in spec.split(","):
        start, end = [int(x) for x in part.split("-", 1)]
        chunks.append("\n".join(lines[start - 1 : end]))
    return "\n".join(chunks)


def audit_sources() -> list[dict]:
    rows = []
    for lock in SOURCE_LOCKS:
        path = RED / lock["file"]
        text = source_window(path, lock["lines"]) if path.exists() else ""
        missing = [needle for needle in lock["needles"] if norm(needle) not in norm(text)]
        rows.append({**lock, "path": str(path), "exists": path.exists(), "ok": path.exists() and not missing, "missing": missing})
    return rows


def rows_for_paths(paths: Iterable[Path]) -> list[dict]:
    rows = []
    for path in paths:
        rows.append({
            "path": str(path),
            "relative": str(path.relative_to(ROOT)) if path.is_relative_to(ROOT) else str(path),
            "exists": path.exists(),
            "size": path.stat().st_size if path.exists() else None,
            "sha256": sha256(path) if path.exists() and path.is_file() else None,
        })
    return rows


def local_reference_rows() -> list[dict]:
    return rows_for_paths([
        GREATSTONE / "index/SUMMARY.md",
        GREATSTONE / "raw/greatstone.free.fr__dm__d_articles_dungeon_html.html.html",
        ORIGINALS / "_manifests/dm_pc34_greatstone_item_by_item_diff_20260510.json",
        DM1_CANON / "DUNGEON.DAT",
        DM1_CANON / "GRAPHICS.DAT",
        DM1_CANON / "TITLE",
        DM1_CANON / "Dungeon-Master_DOS_EN.zip",
    ])


def find_transcript_candidates() -> list[dict]:
    candidates = []
    pattern = "|".join(TRANSCRIPT_NAME_HINTS)
    roots = [ROOT, ORIGINALS, GREATSTONE]
    for root in roots:
        if not root.exists():
            continue
        proc = subprocess.run(
            ["bash", "-lc", "rg --files | rg -i " + json.dumps(pattern)],
            cwd=root,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            check=False,
            timeout=10,
        )
        for line in proc.stdout.splitlines():
            path = root / line
            rel = str(path.relative_to(ROOT)) if path.is_relative_to(ROOT) else str(path)
            if "pass511_dm1_v1_original_transcript_blocker_evidence" in rel or "__pycache__" in rel:
                continue
            if path.is_file():
                candidates.append({"path": str(path), "relative": rel, "size": path.stat().st_size, "sha256": sha256(path)})
    return sorted(candidates, key=lambda row: row["path"])


def manifest_summary() -> dict:
    out = {}
    for name, path in INPUT_MANIFESTS.items():
        data = read_json(path)
        out[name] = {"path": str(path.relative_to(ROOT)), "status": data.get("status"), "ok": data.get("ok"), "blocker": data.get("blocker"), "nextBlocker": data.get("nextBlocker"), "narrowedBlocker": data.get("narrowedBlocker")}
    return out


def build() -> dict:
    sources = audit_sources()
    refs = local_reference_rows()
    route_artifacts = rows_for_paths(ROUTE_ARTIFACTS)
    transcripts = find_transcript_candidates()
    manifests = manifest_summary()
    required = {
        "redmcsb_source_audit_ok": all(row["ok"] for row in sources),
        "n2_local_references_present": all(row["exists"] for row in refs),
        "route_capture_artifacts_present": all(row["exists"] for row in route_artifacts),
        "pass435_still_blocks_semantic_promotion": manifests["pass435"]["status"] == "BLOCKED_PASS435_SEMANTIC_ORIGINAL_ROUTE_NOT_READY",
        "pass497_next_blocker_locked": manifests["pass497"]["status"] == "PASS497_ORIGINAL_CAPTURE_NEXT_BLOCKER_LOCKED",
        "pass504_state_delta_diversity_still_blocked": manifests["pass504"]["status"] == "BLOCKED_PASS504_ORIGINAL_ROUTE_STATE_DELTA_DIVERSITY_NOT_PROVEN",
        "pass508_capture_blocker_tightened": manifests["pass508"]["status"] == "PASS508_ORIGINAL_ROUTE_CAPTURE_BLOCKER_EVIDENCE_TIGHTENED",
        "pass510_label_fixture_green": manifests["pass510"]["status"] == "PASS510_ORIGINAL_CAPTURE_ROUTE_LABEL_FILENAME_FIXTURE",
        "external_original_transcript_absent": len(transcripts) == 0,
    }
    problems = [name for name, ok in required.items() if not ok]
    problems.extend(f"source lock failed {row['file']}:{row['lines']} {row['missing']}" for row in sources if not row["ok"])
    return {
        "schema": "firestaff.parity.pass511_dm1_v1_original_transcript_blocker_evidence.v1",
        "status": STATUS if not problems else "FAIL_PASS511_ORIGINAL_TRANSCRIPT_BLOCKER_EVIDENCE",
        "ok": not problems,
        "sourceRoot": str(RED),
        "sourceAudit": sources,
        "localReferences": refs,
        "routeArtifacts": route_artifacts,
        "transcriptCandidates": transcripts,
        "inputManifests": manifests,
        "required": required,
        "blocker": "DM1 V1 original route/capture promotion is now blocked specifically on an external original transcript or equivalent debugger trace. Existing route/crop artifacts and label normalization are present, but no artifact binds each captured shot to the source-visible command, stop-wait, redraw, and PC34 present boundary.",
        "nextEvidenceRequired": ["record each shot command id plus X/Y from F0380", "record matching F0365 or F0366 handler hit and G0321 write", "record later F0128 direction/X/Y/map tuple", "record F0097/VIDRV present before screenshot acceptance", "rerun pass435/pass504/pass508 after transcript capture"],

        "nonClaims": ["no DOSBox/emulator behavior guessed from screen images", "no original-vs-Firestaff pixel parity claim", "no movement or viewport implementation change", "no promotion of duplicate/static captures"],
        "problems": problems,
    }


def write_report(data: dict) -> None:
    lines = ["# Pass511 - DM1 V1 original transcript blocker evidence", "", f"Status: {data['status']}", "", data["blocker"], "", "## ReDMCSB Source Audit", ""]
    for row in data["sourceAudit"]:
        lines.append(f"- {row['file']}:{row['lines']} {row['id']} ok={row['ok']}: {row['claim']}")
    lines.extend(["", "## N2 Local References", ""])
    lines.extend(f"- {row['path']} exists={row['exists']} sha256={row['sha256']}" for row in data["localReferences"])
    lines.extend(["", "## Current Route/Capture Artifacts", ""])
    lines.extend(f"- {row['relative']} exists={row['exists']} sha256={row['sha256']}" for row in data["routeArtifacts"])
    lines.extend(["", "## Existing Gate Boundary", ""])
    for name, row in data["inputManifests"].items():
        detail = row["blocker"] or row["nextBlocker"] or row["narrowedBlocker"] or ""
        lines.append(f"- {name} status={row['status']} ok={row['ok']}; {detail}".rstrip())
    lines.extend(["", "## Transcript Search", ""])
    if data["transcriptCandidates"]:
        lines.extend(f"- {row['relative']} size={row['size']} sha256={row['sha256']}" for row in data["transcriptCandidates"])
    else:
        lines.append("- No external original transcript/state-delta transcript artifact found under this worktree, the N2 DM originals, or the N2 Greatstone atlas mirror.")
    lines.extend(["", "## Next Evidence Required", ""])
    lines.extend(f"- {item}" for item in data["nextEvidenceRequired"])
    lines.extend(["", "## Non-Claims", ""])
    lines.extend(f"- {item}" for item in data["nonClaims"])
    lines.append("")
    REPORT.write_text("\n".join(lines), encoding="utf-8")


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
