#!/usr/bin/env python3
"""Pass1053: DM1 V1 original champion candidate-panel evidence gate.

This verifier keeps the pass1053 evidence reproducible in CTest. It proves that
tracked original PC 3.4 candidate/resurrect-panel artifacts and their region
crops still match the pass1053 manifest, and that the Firestaff-side V1 champion
HUD/statusbox reference captures are present.

Honesty boundary:
- This is an artifact/provenance gate, not a pixel-parity gate.
- It does not claim full four-champion original HUD parity.
- It does not compare Firestaff and original screenshots pixel-by-pixel.
"""
from __future__ import annotations

import hashlib
import json
import os
import struct
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass1053_dm1_v1_original_champion_candidate_panel_gate"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST_OUT = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
PASS1053_DIR = ROOT / "verification-screens/pass1053-dm1-original-champion-candidate-panel"
PASS1053_MANIFEST = PASS1053_DIR / "manifest.json"
PASS1053_REPORT = ROOT / "parity-evidence/pass1053_dm1_v1_original_champion_candidate_panel_capture.md"
FIRESTAFF_SIDE = [
    ROOT / "verification-m11/lane3-inventory-followup-20260428-0914/party_hud_four_champions_vga.ppm",
    ROOT / "verification-m11/lane3-inventory-followup-20260428-0914/party_hud_statusbox_gfx_vga.ppm",
]
REDMCSB = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))

EXPECTED_STATUS = "PASS1053_ORIGINAL_CHAMPION_CANDIDATE_PANEL_EVIDENCE_TRACKED"
EXPECTED_FRAME_LABELS = [
    "start_before_portrait_click",
    "candidate_select_after_click_111_82",
    "resurrect_terminal_hud_after_click_130_115",
]
EXPECTED_FRAME_SHA256 = {
    "start_before_portrait_click": "50bead319e59bd42c9b5af6e4a39275e6cfc7a02fee96e6f6b766e575858fabc",
    "candidate_select_after_click_111_82": "e4b373078be6aa0c27e793ccd476b6e886b34ef0c4b063c6d2274815351af53e",
    "resurrect_terminal_hud_after_click_130_115": "7523b67fa765ffb02a088bf8dbb0c2ba3630fcf5bcc2fb11f956b4e442b52b8f",
}
EXPECTED_ROUTE_TOKENS = [
    "click:111,82",
    "click:130,115",
]
EXPECTED_CROPS = {
    "viewport": (224, 136),
    "candidate_buttons": (155, 68),
    "right_panel": (96, 136),
    "lower_panel": (320, 64),
}
SOURCE_LOCKS = [
    {
        "file": "COMMAND.C",
        "lines": "108-116,226-237,1986-1994",
        "needles": [
            "C080_COMMAND_CLICK_IN_DUNGEON_VIEW",
            "C160_COMMAND_CLICK_IN_PANEL_RESURRECT",
            "F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel",
        ],
    },
    {
        "file": "MOVESENS.C",
        "lines": "1496-1504",
        "needles": [
            "C127_SENSOR_WALL_CHAMPION_PORTRAIT",
            "F0280_CHAMPION_AddCandidateChampionToParty",
        ],
    },
    {
        "file": "REVIVE.C",
        "lines": "63-201,704-853",
        "needles": [
            "void F0280_CHAMPION_AddCandidateChampionToParty",
            "void F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel",
            "C160_COMMAND_CLICK_IN_PANEL_RESURRECT",
        ],
    },
]


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def png_dims(path: Path) -> tuple[int, int] | None:
    try:
        data = path.read_bytes()[:24]
    except OSError:
        return None
    if data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR":
        return None
    return struct.unpack(">II", data[16:24])


def ppm_dims(path: Path) -> tuple[int, int] | None:
    try:
        data = path.read_bytes()[:128]
    except OSError:
        return None
    if not data.startswith(b"P6"):
        return None
    tokens: list[bytes] = []
    i = 0
    while i < len(data) and len(tokens) < 4:
        while i < len(data) and data[i:i + 1].isspace():
            i += 1
        if i < len(data) and data[i:i + 1] == b"#":
            while i < len(data) and data[i:i + 1] not in (b"\n", b"\r"):
                i += 1
            continue
        start = i
        while i < len(data) and not data[i:i + 1].isspace():
            i += 1
        if start < i:
            tokens.append(data[start:i])
    if len(tokens) < 4:
        return None
    try:
        return int(tokens[1]), int(tokens[2])
    except ValueError:
        return None


def source_window(path: Path, ranges: str) -> str:
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    chunks: list[str] = []
    for part in ranges.split(","):
        start_s, end_s = part.split("-", 1)
        start = int(start_s)
        end = int(end_s)
        chunks.append("\n".join(lines[start - 1:end]))
    return "\n".join(chunks)


def audit_sources() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for lock in SOURCE_LOCKS:
        path = REDMCSB / lock["file"]
        found: list[str] = []
        missing: list[str] = []
        if path.exists():
            text = source_window(path, lock["lines"])
            for needle in lock["needles"]:
                (found if needle in text else missing).append(needle)
        else:
            missing = list(lock["needles"])
        rows.append({
            "file": lock["file"],
            "lines": lock["lines"],
            "exists": path.exists(),
            "needles_found": found,
            "needles_missing": missing,
            "ok": path.exists() and not missing,
        })
    return rows


def check_original_frames(pass1053: dict[str, Any]) -> list[dict[str, Any]]:
    frames_by_label = {f.get("label"): f for f in pass1053.get("frames", [])}
    rows: list[dict[str, Any]] = []
    for label in EXPECTED_FRAME_LABELS:
        frame = frames_by_label.get(label, {})
        path = PASS1053_DIR / frame.get("file", f"{label}.png")
        dims = png_dims(path) if path.exists() else None
        file_hash = sha256(path) if path.exists() else None
        crop_rows: list[dict[str, Any]] = []
        for crop, expected_dims in EXPECTED_CROPS.items():
            crop_path = PASS1053_DIR / "crops" / f"{label}_{crop}.png"
            crop_dims = png_dims(crop_path) if crop_path.exists() else None
            region = frame.get("regions", {}).get(crop, {})
            crop_rows.append({
                "crop": crop,
                "path": str(crop_path.relative_to(ROOT)),
                "exists": crop_path.exists(),
                "dims": list(crop_dims) if crop_dims else None,
                "expected_dims": list(expected_dims),
                "manifest_dims": [region.get("width"), region.get("height")],
                "ok": crop_path.exists()
                and crop_dims == expected_dims
                and [region.get("width"), region.get("height")] == list(expected_dims),
            })
        rows.append({
            "label": label,
            "path": str(path.relative_to(ROOT)),
            "exists": path.exists(),
            "dims": list(dims) if dims else None,
            "sha256": file_hash,
            "manifest_sha256": frame.get("sha256"),
            "expected_sha256": EXPECTED_FRAME_SHA256[label],
            "crops": crop_rows,
            "ok": path.exists()
            and dims == (320, 200)
            and file_hash == EXPECTED_FRAME_SHA256[label]
            and frame.get("sha256") == EXPECTED_FRAME_SHA256[label]
            and all(c["ok"] for c in crop_rows),
        })
    return rows


def check_firestaff_side() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for path in FIRESTAFF_SIDE:
        dims = ppm_dims(path) if path.exists() else None
        rows.append({
            "path": str(path.relative_to(ROOT)),
            "exists": path.exists(),
            "dims": list(dims) if dims else None,
            "sha256": sha256(path) if path.exists() else None,
            "ok": path.exists() and dims is not None and dims[0] > 0 and dims[1] > 0,
        })
    return rows


def write_outputs(result: dict[str, Any]) -> None:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST_OUT.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass1053 DM1 V1 original champion candidate-panel gate",
        "",
        f"Status: `{result['status']}`",
        "",
        "This CTest gate keeps the existing original-PC34 candidate/resurrect-panel",
        "evidence reproducible. It verifies manifest status, image hashes, crop",
        "dimensions, source anchors, and the existing Firestaff-side champion HUD",
        "reference captures.",
        "",
        "## Inputs",
        "",
        f"- Original evidence: `{PASS1053_DIR.relative_to(ROOT)}`",
        f"- Source report: `{PASS1053_REPORT.relative_to(ROOT)}`",
        "- Firestaff-side references:",
    ]
    for row in result["firestaff_side"]:
        lines.append(f"  - `{row['path']}` dims={row['dims']} ok={row['ok']}")
    lines += [
        "",
        "## Original frames",
        "",
        "| Label | SHA256 | Crops OK | Status |",
        "|---|---|---:|---|",
    ]
    for row in result["original_frames"]:
        lines.append(
            f"| `{row['label']}` | `{row['sha256'][:16] if row['sha256'] else ''}` | "
            f"{all(c['ok'] for c in row['crops'])} | {'OK' if row['ok'] else 'FAIL'} |"
        )
    lines += [
        "",
        "## Non-claims",
        "",
        "- This is not a Firestaff-vs-original pixel diff.",
        "- This does not close the full four-champion HUD/status-panel capture gap.",
        "- This does not promote any same-state champion-panel parity row.",
        "",
        f"Manifest: `{MANIFEST_OUT.relative_to(ROOT)}`",
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    pass1053 = json.loads(PASS1053_MANIFEST.read_text(encoding="utf-8")) if PASS1053_MANIFEST.exists() else {}
    source_rows = audit_sources()
    original_rows = check_original_frames(pass1053)
    firestaff_rows = check_firestaff_side()
    route = pass1053.get("route", [])
    non_claims = pass1053.get("non_claims", [])
    ok = (
        PASS1053_MANIFEST.exists()
        and PASS1053_REPORT.exists()
        and pass1053.get("status") == EXPECTED_STATUS
        and all(token in route for token in EXPECTED_ROUTE_TOKENS)
        and any("not a full four-champion party HUD capture" in claim for claim in non_claims)
        and all(row["ok"] for row in source_rows)
        and all(row["ok"] for row in original_rows)
        and all(row["ok"] for row in firestaff_rows)
    )
    result = {
        "schema": "firestaff.parity.pass1053_dm1_v1_original_champion_candidate_panel_gate.v1",
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "status": "PASS1053_ORIGINAL_CHAMPION_CANDIDATE_PANEL_GATE" if ok else "FAIL",
        "honesty": "Artifact gate only; no Firestaff-vs-original pixel parity is claimed.",
        "source_locks": source_rows,
        "manifest_status": pass1053.get("status"),
        "route_tokens_ok": all(token in route for token in EXPECTED_ROUTE_TOKENS),
        "non_claim_boundary_ok": any("not a full four-champion party HUD capture" in claim for claim in non_claims),
        "original_frames": original_rows,
        "firestaff_side": firestaff_rows,
    }
    write_outputs(result)
    if ok:
        print("PASS pass1053 DM1 V1 original champion candidate-panel gate")
        return 0
    print("FAIL pass1053 DM1 V1 original champion candidate-panel gate")
    print(f"manifest: {MANIFEST_OUT.relative_to(ROOT)}")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
