#!/usr/bin/env python3
"""Pass610: lock Firestaff viewport-crop capture readiness for DM1 V1 wall rows.

This gate stays on the Firestaff side. It proves the deterministic wall/collision
capture probe emits both full-frame screenshots and source-geometry 224x136
viewport crops for the same map/X/Y/direction rows. It does not claim original
PC34 pixel parity.
"""
from __future__ import annotations

import hashlib
import json
import os
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))
PASS = "pass610_dm1_v1_firestaff_viewport_crop_capture_gate"
STATUS = "PASS610_DM1_V1_FIRESTAFF_VIEWPORT_CROP_CAPTURE_LOCKED"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

EXPECTED_ROWS = [
    ("01_start_south_1_3", 0, 1, 3, 2),
    ("02_turn_right_west_1_3", 0, 1, 3, 3),
    ("03_blocked_west_wall_1_3", 0, 1, 3, 3),
    ("04_forward_south_1_4", 0, 1, 4, 2),
]

SOURCE_LOCKS = [
    {
        "file": "COORD.C",
        "lines": "1693-1722",
        "needles": [
            "int16_t G2067_i_ViewportScreenX = 0;",
            "int16_t G2068_i_ViewportScreenY = 33;",
            "int16_t G2073_C224_ViewportPixelWidth = 224;",
            "int16_t G2074_C136_ViewportHeight = 136;",
        ],
        "claim": "PC34 dungeon viewport crops use x=0, y=33, width=224, height=136.",
    },
    {
        "file": "DUNVIEW.C",
        "lines": "2962-3003,3048-3078,8318-8610",
        "needles": [
            "void F0098_DUNGEONVIEW_DrawFloorAndCeiling",
            "G0296_puc_Bitmap_Viewport",
            "M100_PIXEL_WIDTH(G0296_puc_Bitmap_Viewport) = G2073_C224_ViewportPixelWidth;",
            "M101_PIXEL_HEIGHT(G0296_puc_Bitmap_Viewport) = G2074_C136_ViewportHeight;",
            "void F0100_DUNGEONVIEW_DrawWallSetBitmap",
            "void F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency",
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "claim": "Walls compose into G0296 before F0128 presents the dungeon view.",
    },
    {
        "file": "DRAWVIEW.C",
        "lines": "842-857",
        "needles": [
            "F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT",
            "F0638_GetZone(C007_ZONE_VIEWPORT, L2413_ai_Box);",
            "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);",
        ],
        "claim": "F0097 presents G0296 through C007_ZONE_VIEWPORT, so the crop is the compare boundary.",
    },
]


def read_source(path: Path) -> str:
    return path.read_text(encoding="latin-1", errors="replace")


def source_window(text: str, spec: str) -> str:
    lines = text.splitlines()
    out: list[str] = []
    for part in spec.split(","):
        if "-" in part:
            first, last = (int(p) for p in part.split("-", 1))
        else:
            first = last = int(part)
        out.extend(lines[first - 1:last])
    return "\n".join(out)


def compact(value: str) -> str:
    return " ".join(value.split())


def audit_source() -> list[dict[str, object]]:
    if not RED.exists():
        raise AssertionError(f"missing ReDMCSB source root: {RED}")
    rows: list[dict[str, object]] = []
    for lock in SOURCE_LOCKS:
        body = compact(source_window(read_source(RED / lock["file"]), lock["lines"]))
        missing = [needle for needle in lock["needles"] if compact(needle) not in body]
        rows.append({
            "file": lock["file"],
            "lines": lock["lines"],
            "ok": not missing,
            "claim": lock["claim"],
            "missing": missing,
        })
    return rows


def find_probe() -> Path:
    name = "firestaff_m11_wall_collision_capture_probe"
    candidates: list[Path] = []
    build_dir = os.environ.get("BUILD_DIR")
    if build_dir:
        candidates.append(Path(build_dir) / name)
    candidates.extend([
        Path.cwd() / name,
        ROOT / "build" / name,
        ROOT / "build-pass610" / name,
        ROOT / "build-wall-collision-capture" / name,
    ])
    candidates.extend(sorted(ROOT.glob(f"build*/{name}")))
    for candidate in candidates:
        if candidate.exists():
            return candidate
    raise AssertionError(f"missing built executable {name}; candidates={candidates}")


def run_probe() -> tuple[Path, str]:
    probe = find_probe()
    out_dir = Path.cwd() / PASS
    out_dir.mkdir(parents=True, exist_ok=True)
    data_dir = Path(os.environ.get("FIRESTAFF_DATA", str(Path.home() / ".firestaff/data")))
    proc = subprocess.run(
        [str(probe), str(data_dir), str(out_dir)],
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        timeout=180,
    )
    if proc.returncode != 0:
        raise AssertionError(proc.stdout[-4000:])
    if "PASS dm1_v1_wall_collision_runtime_capture rows=4" not in proc.stdout:
        raise AssertionError(f"probe did not report four passing rows:\n{proc.stdout}")
    return out_dir, proc.stdout.strip()


def validate_ppm(path: Path, width: int, height: int) -> str:
    header = f"P6\n{width} {height}\n255\n".encode("ascii")
    if not path.exists():
        raise AssertionError(f"missing PPM: {path}")
    data = path.read_bytes()
    if not data.startswith(header):
        raise AssertionError(f"bad PPM header for {path}: {data[:32]!r}")
    expected_size = len(header) + width * height * 3
    if len(data) != expected_size:
        raise AssertionError(f"bad PPM size for {path}: {len(data)} != {expected_size}")
    return hashlib.sha256(data).hexdigest()


def audit_runtime(out_dir: Path) -> dict[str, object]:
    manifest_path = out_dir / "dm1_v1_wall_collision_runtime_capture.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    captures = manifest.get("captures", [])
    if len(captures) != len(EXPECTED_ROWS):
        raise AssertionError(f"expected {len(EXPECTED_ROWS)} captures, got {len(captures)}")
    source_evidence = "\n".join(manifest.get("sourceEvidence", []))
    for anchor in ["COORD.C:1693-1722", "DRAWVIEW.C:842-857", "DUNVIEW.C:8318-8618"]:
        if anchor not in source_evidence:
            raise AssertionError(f"capture manifest missing source evidence anchor {anchor}")
    if "not an original DOS pixel-parity claim" not in manifest.get("honesty", ""):
        raise AssertionError("manifest must preserve the no-original-pixel-parity non-claim")

    rows: list[dict[str, object]] = []
    viewport_hashes: list[str] = []
    for capture, expected in zip(captures, EXPECTED_ROWS):
        label, map_index, x, y, direction = expected
        if capture.get("label") != label:
            raise AssertionError(f"label mismatch: {capture}")
        party = capture.get("party", {})
        if (party.get("mapIndex"), party.get("mapX"), party.get("mapY"), party.get("direction")) != (map_index, x, y, direction):
            raise AssertionError(f"party tuple mismatch for {label}: {party}")
        full_hash = validate_ppm(out_dir / capture["screenshot"], 320, 200)
        crop_name = capture.get("viewportCrop")
        if crop_name != f"{label}_viewport_224x136.ppm":
            raise AssertionError(f"viewport crop name mismatch for {label}: {crop_name}")
        crop_hash = validate_ppm(out_dir / crop_name, 224, 136)
        viewport_hashes.append(crop_hash)
        rows.append({
            "label": label,
            "party": party,
            "screenshot": capture["screenshot"],
            "screenshotSha256": full_hash,
            "viewportCrop": crop_name,
            "viewportCropSha256": crop_hash,
        })
    if len(set(viewport_hashes)) < 3:
        raise AssertionError("viewport crops collapsed to fewer than three distinct hashes")
    return {"outDir": str(out_dir), "rows": rows, "distinctViewportCropHashes": len(set(viewport_hashes))}


def audit_repo() -> None:
    probe = (ROOT / "probes/m11/firestaff_m11_wall_collision_capture_probe.c").read_text(encoding="utf-8")
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    for needle in [
        "VIEWPORT_X = 0",
        "VIEWPORT_Y = 33",
        "VIEWPORT_W = 224",
        "VIEWPORT_H = 136",
        "dump_vga_viewport_ppm",
        "viewportCrop",
        "COORD.C:1693-1722 PC34 viewport origin/224x136 dimensions",
        "DRAWVIEW.C:842-857 F0097 presents G0296 through C007_ZONE_VIEWPORT",
    ]:
        if needle not in probe:
            raise AssertionError(f"probe source missing {needle}")
    if PASS not in cmake:
        raise AssertionError(f"CMake missing {PASS} test")


def write_outputs(source: list[dict[str, object]], runtime: dict[str, object], stdout: str, problems: list[str]) -> None:
    payload = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": STATUS if not problems else "FAIL_PASS610_DM1_V1_FIRESTAFF_VIEWPORT_CROP_CAPTURE",
        "ok": not problems,
        "sourceRoot": str(RED),
        "sourceEvidence": source,
        "runtime": runtime,
        "probeStdoutTail": "\n".join(stdout.splitlines()[-4:]),
        "nonClaims": [
            "no original PC34 frame was captured",
            "no original-vs-Firestaff pixel parity is promoted",
            "the crop hashes are Firestaff capture-readiness evidence only",
            "no TODO.md update",
        ],
        "problems": problems,
    }
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass610 - DM1 V1 Firestaff viewport crop capture gate",
        "",
        f"Status: {payload['status']}",
        "",
        "This gate locks the Firestaff-side 224x136 viewport crop artifacts needed before any later same-viewport original/Firestaff comparison can be promoted.",
        "",
        "Source evidence:",
    ]
    for row in source:
        lines.append(f"- {row['file']}:{row['lines']} ok={row['ok']} - {row['claim']}")
    lines += ["", "Runtime crops:"]
    for row in runtime.get("rows", []):
        party = row["party"]
        lines.append(
            f"- {row['label']} map={party['mapIndex']} x={party['mapX']} y={party['mapY']} dir={party['direction']} "
            f"crop={row['viewportCrop']} sha256={row['viewportCropSha256']}"
        )
    lines += ["", "Non-claims:"]
    lines.extend(f"- {item}" for item in payload["nonClaims"])
    if problems:
        lines += ["", "Problems:"]
        lines.extend(f"- {problem}" for problem in problems)
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    problems: list[str] = []
    source: list[dict[str, object]] = []
    runtime: dict[str, object] = {}
    stdout = ""
    try:
        source = audit_source()
        problems.extend(f"source lock failed: {row['file']}:{row['lines']}" for row in source if not row["ok"])
        audit_repo()
        out_dir, stdout = run_probe()
        runtime = audit_runtime(out_dir)
    except Exception as exc:  # gate should emit a manifest on failure.
        problems.append(str(exc))
    write_outputs(source, runtime, stdout, problems)
    print(STATUS if not problems else "FAIL_PASS610_DM1_V1_FIRESTAFF_VIEWPORT_CROP_CAPTURE")
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    return 0 if not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())
