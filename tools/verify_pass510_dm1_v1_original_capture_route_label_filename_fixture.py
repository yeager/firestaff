#!/usr/bin/env python3
"""Pass510: prove original capture labels become crop filenames.

This is a capture-route fixture, not pixel parity evidence. It exercises the
normalization path with six generated 320x200 PNGs and route labels matching the
DM1 V1 original movement/HUD/viewport blocker labels.
"""
from __future__ import annotations

import hashlib
import json
import os
import re
import struct
import subprocess
import tempfile
import zlib
from pathlib import Path
from typing import Iterable

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts/dosbox_dm1_original_viewport_reference_capture.sh"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
ORIG = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
VERIFY_DIR = ROOT / "parity-evidence/verification/pass510_dm1_v1_original_capture_route_label_filename_fixture"
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence/pass510_dm1_v1_original_capture_route_label_filename_fixture.md"
STATUS = "PASS510_ORIGINAL_CAPTURE_ROUTE_LABEL_FILENAME_FIXTURE"

ROUTE_LABELS = [
    "start_south",
    "turn_right_west",
    "forward_west_blocked",
    "turn_left_east",
    "forward_south_corridor",
    "post_redraw_after_vblank",
]
ROUTE_EVENTS = (
    "wait:7000 enter wait:1500 click:260,50 wait:1500 click:276,140 wait:3000 "
    "shot:start_south kp6 wait:1200 shot:turn_right_west kp8 wait:1200 "
    "shot:forward_west_blocked kp4 wait:1200 shot:turn_left_east kp8 wait:1200 "
    "shot:forward_south_corridor wait:600 shot:post_redraw_after_vblank"
)

SOURCE_REFS = [
    {
        "file": "COMMAND.C",
        "lines": "2045-2156",
        "needles": [
            "F0380_COMMAND_ProcessQueue_CPSC",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty",
        ],
        "why": "queued route input must dispatch through the original command processor before movement/turn labels are semantic",
    },
    {
        "file": "CLIKMENU.C",
        "lines": "142-174,180-347",
        "needles": [
            "F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        ],
        "why": "turn/move route labels name the state mutations implemented here",
    },
    {
        "file": "DUNVIEW.C",
        "lines": "8318-8618",
        "needles": [
            "F0128_DUNGEONVIEW_Draw_CPSF",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)",
        ],
        "why": "captured viewport crops are only meaningful after this tuple-specific draw path",
    },
    {
        "file": "DRAWVIEW.C",
        "lines": "709-858",
        "needles": ["F0097_DUNGEONVIEW_DrawViewport", "G0296_puc_Bitmap_Viewport", "VIDRV_09_BlitViewPort"],
        "why": "the capture seam must be the PC34 viewport present path, not menu/setup echoes",
    },
]
ASSET_REFS = [
    ORIG / "DM.EXE",
    ORIG / "DATA/DUNGEON.DAT",
    ORIG / "DATA/GRAPHICS.DAT",
    ORIG / "TITLE",
]
GREATSTONE_MANIFEST = Path.home() / ".openclaw/data/firestaff-original-games/DM/_manifests/dm_pc34_greatstone_item_by_item_diff_20260510.json"


def norm(text: str) -> str:
    return " ".join(text.split())


def source_window(path: Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    out: list[str] = []
    for part in spec.split(","):
        start, end = [int(x) for x in part.split("-", 1)]
        out.extend(lines[start - 1 : end])
    return "\n".join(out)


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def png_chunk(kind: bytes, payload: bytes) -> bytes:
    return struct.pack(">I", len(payload)) + kind + payload + struct.pack(">I", zlib.crc32(kind + payload) & 0xFFFFFFFF)


def write_png(path: Path, width: int, height: int, rgb: tuple[int, int, int]) -> None:
    raw = b"".join(b"\x00" + bytes(rgb) * width for _ in range(height))
    payload = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    data = b"\x89PNG\r\n\x1a\n" + png_chunk(b"IHDR", payload) + png_chunk(b"IDAT", zlib.compress(raw)) + png_chunk(b"IEND", b"")
    path.write_bytes(data)


def expected_name(index: int, label: str) -> str:
    stem = re.sub(r"[^a-z0-9_-]+", "_", label.lower()).strip("_")
    return f"{index:02d}_{stem}_original_viewport_224x136.ppm"


def parse_tsv(path: Path) -> list[dict[str, str]]:
    lines = path.read_text(encoding="utf-8").splitlines()
    headers = lines[0].split("\t")
    rows = []
    for line in lines[1:]:
        if line:
            rows.append(dict(zip(headers, line.split("\t"))))
    return rows


def audit_sources() -> list[dict[str, object]]:
    rows = []
    for ref in SOURCE_REFS:
        path = RED / ref["file"]
        text = source_window(path, ref["lines"])
        missing = [needle for needle in ref["needles"] if norm(needle) not in norm(text)]
        rows.append({**ref, "ok": not missing, "missing": missing})
    return rows


def asset_rows(paths: Iterable[Path]) -> list[dict[str, object]]:
    rows = []
    for path in paths:
        rows.append({
            "path": str(path),
            "exists": path.exists(),
            "size": path.stat().st_size if path.exists() else None,
            "sha256": sha256(path) if path.exists() else None,
        })
    return rows


def greatstone_row() -> dict[str, object]:
    if not GREATSTONE_MANIFEST.exists():
        return {"path": str(GREATSTONE_MANIFEST), "exists": False}
    data = json.loads(GREATSTONE_MANIFEST.read_text(encoding="utf-8"))
    summary = data.get("summary", {})
    return {
        "path": str(GREATSTONE_MANIFEST),
        "exists": True,
        "sha256": sha256(GREATSTONE_MANIFEST),
        "result": summary.get("result"),
        "pc34GraphicsItems": summary.get("pc34_graphics_items"),
        "pc34DungeonMaps": summary.get("pc34_dungeon_maps"),
        "totalMismatches": summary.get("total_mismatches"),
        "note": summary.get("note"),
    }


def run_fixture() -> dict[str, object]:
    with tempfile.TemporaryDirectory(prefix="pass510-original-labels-") as tmp:
        out = Path(tmp) / "capture"
        out.mkdir()
        colors = [(16, 16, 16), (32, 16, 16), (16, 32, 16), (16, 16, 32), (48, 48, 16), (16, 48, 48)]
        for idx, rgb in enumerate(colors, 1):
            write_png(out / f"image{idx:04d}.png", 320, 200, rgb)

        env = os.environ.copy()
        env.update({
            "OUT_DIR": str(out),
            "DM1_ORIGINAL_ROUTE_EVENTS": ROUTE_EVENTS,
        })
        proc = subprocess.run(
            [str(SCRIPT), "--normalize-only"],
            cwd=ROOT,
            env=env,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )
        labels_path = out / "original_viewport_shot_labels.tsv"
        crop_manifest_path = out / "original_viewport_224x136_manifest.tsv"
        rows = parse_tsv(labels_path) if labels_path.exists() else []
        crops = parse_tsv(crop_manifest_path) if crop_manifest_path.exists() else []
        expected = [expected_name(i, label) for i, label in enumerate(ROUTE_LABELS, 1)]
        actual = [row.get("filename") for row in rows]
        crop_names = [row.get("filename") for row in crops]
        legacy_drift = [name for name in actual if name and name not in expected]
        return {
            "returncode": proc.returncode,
            "stdoutLineCount": len(proc.stdout.splitlines()),
            "normalizerReportedSuccess": "[pass-70] normalized original viewport crops:" in proc.stdout,
            "routeEvents": ROUTE_EVENTS,
            "expectedFilenames": expected,
            "actualLabelManifestFilenames": actual,
            "actualCropManifestFilenames": crop_names,
            "routeLabels": [row.get("route_label") for row in rows],
            "allLabelsMatchFilenames": actual == expected,
            "allCropsMatchFilenames": crop_names == expected,
            "legacyFilenameDrift": legacy_drift,
            "cropRows": crops,
        }


def main() -> int:
    source_rows = audit_sources()
    assets = asset_rows(ASSET_REFS)
    greatstone = greatstone_row()
    fixture = run_fixture()
    problems = []
    if fixture["returncode"] != 0:
        problems.append(f"normalize-only fixture failed with {fixture['returncode']}")
    if fixture["allLabelsMatchFilenames"] is not True:
        problems.append("route labels did not become shot-label manifest filenames")
    if fixture["allCropsMatchFilenames"] is not True:
        problems.append("route labels did not become crop manifest filenames")
    if fixture["legacyFilenameDrift"]:
        problems.append(f"legacy filename drift remains: {fixture['legacyFilenameDrift']}")
    problems += [f"source lock failed {row['file']}:{row['lines']}" for row in source_rows if not row["ok"]]
    problems += [f"missing original asset {row['path']}" for row in assets if not row["exists"]]
    if greatstone.get("result") != "PASS" or greatstone.get("totalMismatches") != 0:
        problems.append("GreatStone PC34 item-by-item manifest is not a zero-mismatch PASS")

    payload = {
        "status": STATUS,
        "ok": not problems,
        "scope": "normalization fixture for original capture route labels; no DOSBox launch and no pixel parity claim",
        "fixture": fixture,
        "sourceRefs": source_rows,
        "originalAssets": assets,
        "greatstonePc34Manifest": greatstone,
        "decision": "capture-route filename drift is fixed: shot labels now name normalized crop artifacts, so pass304/pass435/pass487 can reason over required labels without hard-coded legacy filename mismatch",
        "problems": problems,
        "nonClaims": ["no original-vs-Firestaff pixel parity", "no proof of a fresh semantic six-state DOSBox route", "no tracked screenshot promotion"],
    }
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    REPORT.write_text(
        "\n".join([
            "# Pass510 - DM1 V1 original capture route label filename fixture",
            "",
            f"Status: `{STATUS}`",
            "",
            "The capture normalizer now uses each `shot:<label>` token as the normalized crop filename stem. This removes the hard-coded legacy filename drift that made pass487 report `turn_left_click` beside `02_ingame_turn_right...`.",
            "",
            "## Fixture",
            f"- labels: `{', '.join(ROUTE_LABELS)}`",
            f"- filenames match labels: `{fixture['allLabelsMatchFilenames']}`",
            f"- crop manifest matches labels: `{fixture['allCropsMatchFilenames']}`",
            f"- legacy drift rows: `{len(fixture['legacyFilenameDrift'])}`",
            "",
            "## Source references audited",
            *[f"- `{row['file']}:{row['lines']}` ok={row['ok']} - {row['why']}" for row in source_rows],
            "",
            "## Original assets checked",
            *[f"- `{Path(row['path']).name}` exists={row['exists']} sha256={row['sha256']}" for row in assets],
            "",
            "## GreatStone cross-check",
            f"- manifest: `{GREATSTONE_MANIFEST}`",
            f"- result: `{greatstone.get('result')}`; pc34 graphics items: `{greatstone.get('pc34GraphicsItems')}`; dungeon maps: `{greatstone.get('pc34DungeonMaps')}`; mismatches: `{greatstone.get('totalMismatches')}`",
            "",
            "## Non-claims",
            "This fixture does not launch DOSBox, does not promote screenshots, and does not claim pixel parity.",
        ])
        + "\n",
        encoding="utf-8",
    )
    if problems:
        print("FAIL_PASS510_DM1_V1_ORIGINAL_CAPTURE_ROUTE_LABEL_FILENAME_FIXTURE")
        for problem in problems:
            print(problem)
        return 1
    print(STATUS)
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
