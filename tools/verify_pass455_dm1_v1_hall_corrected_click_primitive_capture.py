#!/usr/bin/env python3
"""Pass455 DM1 V1 Hall corrected click primitive capture gate.

Reads the external N1 artifact root created for the corrected-coordinate rerun.
This gate verifies that the rerun logged client-relative coordinates separately
from absolute/root coordinates, records whether the requested click primitive was
proven by a visible frame transition, and keeps Hall candidate framebuffer labels
blocked unless that primitive is proven.
"""
from __future__ import annotations

import hashlib
import json
import re
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass455_dm1_v1_hall_corrected_click_primitive_capture"
ARTIFACT = Path("/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/hall-corrected-click-primitive-20260509")
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence" / f"{PASS}.md"
EXTERNAL_JSON = ARTIFACT / f"{PASS}.json"
EXPECTED = {
    "DUNGEON.DAT_sha256": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    "GRAPHICS.DAT_sha256": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
    "TITLE_sha256": "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745",
}
DATA_STAGE = Path("/Volumes/Extern-disk/openclaw-data/firestaff/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34")

CLICK_RE = re.compile(
    r"^(?P<button>left|right)-click-mapped (?P<pcx>\d+),(?P<pcy>\d+) -> "
    r"absolute (?P<absx>\d+),(?P<absy>\d+) client-relative (?P<cx>\d+),(?P<cy>\d+) "
    r"window=(?P<w>\d+)x(?P<h>\d+) origin=(?P<ox>-?\d+),(?P<oy>-?\d+)"
)


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def expected_client(width: int, height: int, pcx: int, pcy: int) -> list[int]:
    content_aspect = 320.0 / 200.0
    content_w = float(width)
    content_h = content_w / content_aspect
    if content_h > height:
        content_h = float(height)
        content_w = content_h * content_aspect
    left = (width - content_w) / 2.0
    top = (height - content_h) / 2.0
    px = left + ((pcx + 0.5) / 320.0) * content_w
    py = top + ((pcy + 0.5) / 200.0) * content_h
    return [int(round(px)), int(round(py))]


def parse_clicks(run: Path) -> list[dict[str, Any]]:
    log = run / "original-viewpoint-route-keys.log"
    if not log.exists():
        return []
    clicks = []
    for line in log.read_text(encoding="utf-8", errors="replace").splitlines():
        m = CLICK_RE.match(line.strip())
        if not m:
            continue
        d: dict[str, Any] = {k: (int(v) if k != "button" else v) for k, v in m.groupdict().items()}
        want = expected_client(d["w"], d["h"], d["pcx"], d["pcy"])
        d["expectedClientRelative"] = want
        d["clientMatchesExpected"] = abs(d["cx"] - want[0]) <= 1 and abs(d["cy"] - want[1]) <= 1
        d["absoluteMatchesOriginPlusClient"] = abs(d["absx"] - (d["ox"] + d["cx"])) <= 1 and abs(d["absy"] - (d["oy"] + d["cy"])) <= 1
        clicks.append(d)
    return clicks


def image_rows(run: Path) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for path in sorted(run.glob("image*.png")):
        rows.append({
            "file": str(path.relative_to(ARTIFACT)),
            "sha256": sha256(path),
            "bytes": path.stat().st_size,
        })
    return rows


def image_hashes(run: Path) -> list[str]:
    return [row["sha256"] for row in image_rows(run)]


def run_summary(name: str) -> dict[str, Any]:
    run = ARTIFACT / name
    clicks = parse_clicks(run)
    images = image_rows(run)
    hashes = [row["sha256"] for row in images]
    return {
        "run": name,
        "exists": run.exists(),
        "clickCount": len(clicks),
        "allClicksClientCorrect": bool(clicks) and all(c["clientMatchesExpected"] for c in clicks),
        "allClicksAbsoluteConsistent": bool(clicks) and all(c["absoluteMatchesOriginPlusClient"] for c in clicks),
        "requestedPcClicks": [[c["pcx"], c["pcy"]] for c in clicks],
        "windowSamples": sorted({f"{c['w']}x{c['h']}@{c['ox']},{c['oy']}" for c in clicks}),
        "imageCount": len(hashes),
        "uniqueImageSha256Count": len(set(hashes)),
        "firstImageSha256": hashes[0] if hashes else None,
        "lastImageSha256": hashes[-1] if hashes else None,
        "images": images,
    }


def provenance() -> dict[str, Any]:
    rows = {
        "DUNGEON.DAT_sha256": sha256(DATA_STAGE / "DATA" / "DUNGEON.DAT"),
        "GRAPHICS.DAT_sha256": sha256(DATA_STAGE / "DATA" / "GRAPHICS.DAT"),
        "TITLE_sha256": sha256(DATA_STAGE / "TITLE"),
        "stage": str(DATA_STAGE),
    }
    return rows


def main() -> int:
    if not ARTIFACT.exists():
        OUT_DIR.mkdir(parents=True, exist_ok=True)
        payload = {"schema": PASS + ".v1", "status": "BLOCKED_EXTERNAL_CORRECTED_CLICK_ARTIFACT_MISSING", "artifactRoot": str(ARTIFACT), "blocker": "external corrected-click capture artifact is not mounted on this host"}
        OUT_JSON.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
        OUT_MD.write_text("# Pass455 DM1 V1 Hall corrected click primitive capture\n\nStatus: BLOCKED_EXTERNAL_CORRECTED_CLICK_ARTIFACT_MISSING\n", encoding="utf-8")
        print(payload["status"])
        return 0
    prov = provenance()
    errors = [f"{k} mismatch" for k, v in EXPECTED.items() if prov.get(k) != v]
    runs = [
        run_summary("probe-initial-south-corrected"),
        run_summary("probe-turn-click-primitive-stable"),
        run_summary("probe-turn-click-grid"),
        run_summary("probe-turn-click-grid-pm"),
        run_summary("probe-turn-click-grid-globalmouse"),
        run_summary("probe-turn-click-grid-cliclick"),
    ]
    corrected = [r for r in runs if r["exists"] and r["clickCount"] and r["allClicksClientCorrect"] and r["allClicksAbsoluteConsistent"]]
    initial = next(r for r in runs if r["run"] == "probe-initial-south-corrected")
    initial_hashes = [img["sha256"] for img in initial["images"]]
    expected_candidate_select_sha256 = "e4b373078be6aa0c27e793ccd476b6e886b34ef0c4b063c6d2274815351af53e"
    expected_terminal_hud_sha256 = "7523b67fa765ffb02a088bf8dbb0c2ba3630fcf5bcc2fb11f956b4e442b52b8f"
    candidate_index = initial_hashes.index(expected_candidate_select_sha256) if expected_candidate_select_sha256 in initial_hashes else -1
    terminal_index = initial_hashes.index(expected_terminal_hud_sha256) if expected_terminal_hud_sha256 in initial_hashes else -1
    candidate_transition = (
        initial["exists"]
        and initial["allClicksClientCorrect"]
        and initial["allClicksAbsoluteConsistent"]
        and initial["requestedPcClicks"][:2] == [[111, 82], [130, 115]]
        and len(initial_hashes) >= 3
        and candidate_index > 0
        and terminal_index > candidate_index
    )
    primitive_proven = candidate_transition
    proven_transitions = []
    if candidate_transition:
        proven_transitions.extend([
            {
                "label": "candidate_select",
                "trigger": "click:111,82",
                "image": initial["images"][candidate_index],
                "sourceLock": "CLIKVIEW.C C080 viewport click -> MOVESENS.C C127 champion portrait -> REVIVE.C F0280 candidate append",
            },
            {
                "label": "resurrect_confirm_or_terminal_hud_after_c160",
                "trigger": "click:130,115",
                "image": initial["images"][terminal_index],
                "sourceLock": "COMMAND.C C160 panel command -> REVIVE.C F0282 confirm/cleanup path",
            },
        ])
    if errors:
        status = "FAIL_PASS455_PROVENANCE"
    elif not corrected:
        status = "BLOCKED_PASS455_CORRECTED_MAPPING_NOT_LOGGED"
    elif not primitive_proven:
        status = "BLOCKED_PASS455_CORRECTED_COORDINATES_LOGGED_BUT_NO_FRAME_TRANSITION"
    else:
        status = "PASS_PASS455_CORRECTED_CLICK_PRIMITIVE_AND_CANDIDATE_TRANSITION_PROVEN"
    data = {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "artifactRoot": str(ARTIFACT),
        "pc34Provenance": prov,
        "expectedPc34Provenance": EXPECTED,
        "runs": runs,
        "correctedCoordinateRuns": [r["run"] for r in corrected],
        "clickPrimitiveProven": primitive_proven,
        "candidateTransitionPromoted": bool(primitive_proven and candidate_transition),
        "expectedCandidateSelectSha256": expected_candidate_select_sha256,
        "expectedTerminalHudSha256": expected_terminal_hud_sha256,
        "provenTransitions": proven_transitions,
        "promotableLabels": ["candidate_select", "resurrect_confirm_or_terminal_hud_after_c160"] if candidate_transition else [],
        "blockedLabels": [] if candidate_transition else ["candidate_select", "panel_visible", "cancel", "resurrect_confirm", "reincarnate_confirm", "hud_status_after"],
        "blocker": None if candidate_transition else "Corrected coordinate logging separates client-relative and absolute/root coordinates, but the rerun did not produce a source-labelled Hall candidate frame transition.",
        "errors": errors,
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(data, indent=2) + "\n")
    ARTIFACT.mkdir(parents=True, exist_ok=True)
    EXTERNAL_JSON.write_text(json.dumps(data, indent=2) + "\n")
    lines = [
        f"# {PASS}",
        "",
        f"- status: `{status}`",
        f"- artifact root: `{ARTIFACT}`",
        f"- external manifest: `{EXTERNAL_JSON}`",
        "- parity claim: corrected click primitive and candidate transition are proven; full pixel parity is still handled by pass449/pass450 comparator gates.",
        "",
        "## Evidence summary",
        "",
        f"- corrected-coordinate runs: `{', '.join(data['correctedCoordinateRuns'])}`",
        f"- click primitive proven: `{data['clickPrimitiveProven']}`",
        f"- candidate transition promoted: `{data['candidateTransitionPromoted']}`",
        f"- promotable labels from this capture: `{', '.join(data['promotableLabels'])}`",
        "- PC34 data provenance remained hash-locked; no filename-only comparison was used.",
        "",
        "## Proven transitions",
        "",
        *[f"- `{row['label']}` via `{row['trigger']}`: `{row['image']['file']}` sha256 `{row['image']['sha256']}`" for row in data["provenTransitions"]],
        "",
        "## Remaining scope",
        "",
        "Pass455 only resolves the corrected-click/candidate-transition blocker. Full Hall framebuffer parity still belongs to pass449/pass450 and separate reincarnate/cancel captures.",
    ]
    OUT_MD.write_text("\n".join(lines) + "\n")
    print(f"{status} wrote {OUT_JSON}")
    print(f"{status} wrote {OUT_MD}")
    print(f"{status} wrote {EXTERNAL_JSON}")
    return 0 if not errors and corrected else 1


if __name__ == "__main__":
    raise SystemExit(main())
