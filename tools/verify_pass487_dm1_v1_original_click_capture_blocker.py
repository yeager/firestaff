#!/usr/bin/env python3
"""Pass487: classify fresh N2 original PC34 click-route capture.

Evidence-only gate for the original overlay/capture blocker. It records that
source-locked click primitives can reach gameplay frames on N2, while also
keeping the capture blocked because labels do not produce unique source-visible
movement/viewport states yet.
"""
from __future__ import annotations

import hashlib
import json
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
RUN_DIR = ROOT / "verification-screens/pass487-n2-original-pc34-click-primitives-route"
VERIFY_DIR = ROOT / "parity-evidence/verification/pass487_dm1_v1_original_click_capture_blocker"
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence/pass487_dm1_v1_original_click_capture_blocker.md"
STATUS = "PASS487_ORIGINAL_CLICK_ROUTE_REACHES_GAMEPLAY_STILL_LABEL_BLOCKED"
EXPECTED_LABELS = [
    "party_ready_click_gate",
    "turn_left_click",
    "turn_right_click",
    "move_forward_click",
    "move_backward_click",
    "turn_left_2_click",
]
SOURCE_REFS = [
    {"file": "COMMAND.C", "lines": "106-114", "needles": ["C001_COMMAND_TURN_LEFT", "C003_COMMAND_MOVE_FORWARD", "C002_COMMAND_TURN_RIGHT", "C006_COMMAND_MOVE_LEFT", "C005_COMMAND_MOVE_BACKWARD", "C004_COMMAND_MOVE_RIGHT"]},
    {"file": "COMMAND.C", "lines": "2045-2156", "needles": ["F0380_COMMAND_ProcessQueue_CPSC", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"]},
    {"file": "CLIKMENU.C", "lines": "142-174", "needles": ["F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0284_CHAMPION_SetPartyDirection"]},
    {"file": "CLIKMENU.C", "lines": "180-347", "needles": ["F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0267_MOVE_GetMoveResult_CPSCE"]},
    {"file": "DUNGEON.C", "lines": "1371-1392", "needles": ["F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "G0233_ai_Graphic559_DirectionToStepEastCount", "G0234_ai_Graphic559_DirectionToStepNorthCount"]},
    {"file": "MOVESENS.C", "lines": "316-843", "needles": ["F0267_MOVE_GetMoveResult_CPSCE", "G0362_l_LastPartyMovementTime", "F0265_MOVE_CreateEvent60To61_MoveGroup"]},
    {"file": "DUNVIEW.C", "lines": "2962-3000,8318-8618", "needles": ["F0098_DUNGEONVIEW_DrawFloorAndCeiling", "F0128_DUNGEONVIEW_Draw_CPSF", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)"]},
    {"file": "DRAWVIEW.C", "lines": "709-858", "needles": ["F0097_DUNGEONVIEW_DrawViewport", "G0296_puc_Bitmap_Viewport", "VIDRV_09_BlitViewPort"]},
]


def norm(s: str) -> str:
    return " ".join(s.split())


def source_window(path: Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    out: list[str] = []
    for part in spec.split(","):
        start, end = [int(x) for x in part.split("-", 1)]
        out.extend(lines[start - 1:end])
    return "\n".join(out)


def sha(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def labels() -> list[dict[str, str]]:
    path = RUN_DIR / "original_viewport_shot_labels.tsv"
    rows = []
    for line in path.read_text(encoding="utf-8").splitlines()[1:]:
        idx, filename, label, token = line.split("\t")
        rows.append({"index": idx, "filename": filename, "label": label, "token": token})
    return rows


def audit_sources() -> list[dict[str, object]]:
    rows = []
    for ref in SOURCE_REFS:
        path = RED / ref["file"]
        text = source_window(path, ref["lines"])
        missing = [n for n in ref["needles"] if norm(n) not in norm(text)]
        rows.append({**ref, "ok": not missing, "missing": missing})
    return rows


def main() -> int:
    classifier_path = RUN_DIR / "pass80_original_frame_classifier.json"
    if not classifier_path.exists():
        raise SystemExit(f"missing classifier: {classifier_path}")
    classifier = json.loads(classifier_path.read_text(encoding="utf-8"))
    label_rows = labels()
    seen_labels = [r["label"] for r in label_rows]
    captures = classifier.get("captures", [])
    classes = [c.get("classification") for c in captures]
    hashes = [c.get("sha256") for c in captures]
    source_rows = audit_sources()
    problems = []
    if seen_labels != EXPECTED_LABELS:
        problems.append(f"labels mismatch: {seen_labels}")
    if classes[:1] != ["entrance_menu"] or classes[1:] != ["dungeon_gameplay"] * 5:
        problems.append(f"unexpected classes: {classes}")
    if len(set(hashes[1:])) != 1:
        problems.append("post-entry gameplay frames are not the expected repeated blocker hash")
    if not classifier.get("problems") or "duplicate" not in " ".join(classifier.get("problems", [])).lower():
        problems.append("classifier did not retain duplicate-frame blocker")
    problems += [f"source lock failed {r['file']}:{r['lines']}" for r in source_rows if not r["ok"]]
    payload = {
        "status": STATUS,
        "ok": not problems,
        "generatedUtc": datetime.now(timezone.utc).isoformat(),
        "runDir": str(RUN_DIR.relative_to(ROOT)),
        "classifier": str(classifier_path.relative_to(ROOT)),
        "classifierSha256": sha(classifier_path),
        "labels": label_rows,
        "classes": classes,
        "duplicateSha256Counts": classifier.get("duplicate_sha256_counts", {}),
        "sourceRefs": source_rows,
        "decision": "route mechanism partially unblocked: fresh N2 click primitives reach gameplay; parity promotion remains blocked by menu first frame and repeated post-entry gameplay hash",
        "problems": problems,
        "nonClaims": ["no original-vs-Firestaff pixel parity", "no promotion of pass487 frames as overlay references", "no source/runtime movement-state stop proven"],
    }
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass487 — original PC34 click-route capture blocker",
        "",
        f"Status: `{STATUS}`",
        "",
        "Fresh N2 DOSBox capture using source-locked PC34 click centers reached gameplay frames, but it is still not promotable overlay evidence.",
        "",
        "## Classification",
        f"- labels: `{', '.join(seen_labels)}`",
        f"- classes: `{', '.join(classes)}`",
        f"- duplicate hashes: `{payload['duplicateSha256Counts']}`",
        "- decision: route mechanism partially unblocked; source-state labeling remains blocked by repeated post-entry gameplay frames.",
        "",
        "## Source references audited",
    ]
    for row in source_rows:
        lines.append(f"- `{row['file']}:{row['lines']}` ok={row['ok']}")
    lines += [
        "",
        "## Gates",
        "- `scripts/dosbox_dm1_original_viewport_reference_capture.sh --run` on N2 with six labeled shots",
        "- `python3 tools/pass80_original_frame_classifier.py verification-screens/pass487-n2-original-pc34-click-primitives-route --fail-on-duplicates` retained the expected duplicate-frame blocker",
        "- this verifier records the blocker instead of promoting stale/duplicate frames",
        "",
        "## Non-claims",
        "No original-vs-Firestaff pixel parity and no overlay promotion are claimed.",
    ]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    if problems:
        print("FAIL")
        for problem in problems:
            print(problem)
        return 1
    print(f"PASS {STATUS}")
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
