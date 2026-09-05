#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent))
from firestaff_build_dir import resolve_build_dir, find_build_dir

ROOT = Path(__file__).resolve().parents[1]
RED = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    ROOT / "reference/redmcsb-20210206/Toolchains/Common/Source"))
DM1_ARCHIVE = Path(os.environ.get(
    "FIRESTAFF_DM1_PC34_ARCHIVE",
    Path.home() / ".firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"))
OUT = ROOT / "parity-evidence/verification/pass511_dm1_v1_viewport_wall_reference_lineage/manifest.json"
REPORT = ROOT / "parity-evidence/pass511_dm1_v1_viewport_wall_reference_lineage.md"
ALLOWED = [ROOT.resolve(), RED.resolve()]
DM1_HASHES = {
    "DATA/GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
    "DATA/DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
}


def local(path: Path) -> Path:
    resolved = path.resolve()
    if not any(str(resolved).startswith(str(root)) for root in ALLOWED):
        raise AssertionError(f"refusing non-local reference path: {path}")
    if not resolved.exists():
        raise AssertionError(f"missing required file: {resolved}")
    return resolved


def read(path: Path) -> str:
    return local(path).read_text(encoding="latin-1", errors="replace")


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with local(path).open("rb") as f:
        for chunk in iter(lambda: f.read(1048576), b""):
            h.update(chunk)
    return h.hexdigest()


def line_no(text: str, pos: int) -> int:
    return text.count("\n", 0, pos) + 1


def require_ordered(text: str, needles: list[str], label: str, base: int = 1) -> list[dict[str, object]]:
    hits: list[dict[str, object]] = []
    cursor = 0
    for needle in needles:
        pos = text.find(needle, cursor)
        if pos < 0:
            raise AssertionError(f"{label}: missing {needle!r}")
        hits.append({"line": base + line_no(text, pos) - 1, "needle": needle})
        cursor = pos + len(needle)
    return hits


def slice_lines(text: str, span: str) -> tuple[int, str]:
    start, end = [int(x) for x in span.split("-", 1)]
    return start, "\n".join(text.splitlines()[start - 1:end])


def source_check(ident: str, path: Path, span: str, needles: list[str]) -> dict[str, object]:
    full = read(path)
    base, excerpt = slice_lines(full, span)
    return {
        "id": ident,
        "file": path.name,
        "lines": span,
        "sha256": sha256(path),
        "hits": require_ordered(excerpt, needles, ident, base),
    }


def zip_member(path: Path, name: str) -> bytes:
    result = subprocess.run(
        ["unzip", "-p", str(path), name],
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if result.returncode not in (0, 1) or not result.stdout:
        raise AssertionError(f"cannot read retail ZIP member {name}")
    return result.stdout


def run_gate(script: str) -> dict[str, object]:
    proc = subprocess.run([sys.executable, script], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {
        "command": [sys.executable, script],
        "returncode": proc.returncode,
        "passed": proc.returncode == 0,
        "outputTail": "\n".join(proc.stdout.strip().splitlines()[-8:]),
    }


def main() -> int:
    red = [
        source_check("f0128_far_to_near_then_present", RED / "DUNVIEW.C", "8318-8610", [
            "void F0128_DUNGEONVIEW_Draw_CPSF(",
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
            "F0116_DUNGEONVIEW_DrawSquareD3L",
            "F0121_DUNGEONVIEW_DrawSquareD2C",
            "F0124_DUNGEONVIEW_DrawSquareD1C",
            "F0127_DUNGEONVIEW_DrawSquareD0C",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ]),
        source_check("f0124_d1c_alcove_and_door_occlusion", RED / "DUNVIEW.C", "7727-7938", [
            "STATICFUNCTION void F0124_DUNGEONVIEW_DrawSquareD1C",
            "case C00_ELEMENT_WALL:",
            "F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency",
            "C0x0000_CELL_ORDER_ALCOVE",
            "case C17_ELEMENT_DOOR_FRONT:",
            "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
            "F0111_DUNGEONVIEW_DrawDoor",
            "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT",
        ]),
        source_check("f0115_layer_order", RED / "DUNVIEW.C", "4547-5933", [
            "STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
            "If the first nibble is 0, then the function call is to draw objects in an alcove on a wall square.",
            "/* Draw objects */",
            "/* Draw creatures */",
            "T0115129_DrawProjectiles:",
            "/* Draw explosions */",
        ]),
        source_check("drawview_viewport_present_boundary", RED / "DRAWVIEW.C", "709-858", [
            "void F0097_DUNGEONVIEW_DrawViewport",
            "G0324_B_DrawViewportRequested = C1_TRUE",
            "M526_WaitVerticalBlank();",
            "VIDRV_09_BlitViewPort",
        ]),
    ]
    anchors = []
    if not DM1_ARCHIVE.is_file():
        raise AssertionError(f"missing retail PC34 archive: {DM1_ARCHIVE}")
    for name, expected in DM1_HASHES.items():
        data = zip_member(DM1_ARCHIVE, name)
        actual = hashlib.sha256(data).hexdigest()
        if actual != expected:
            raise AssertionError(f"{name} hash mismatch: {actual} != {expected}")
        anchors.append({"name": name, "archive": str(DM1_ARCHIVE), "sha256": actual, "bytes": len(data)})
    prereqs = [
        run_gate("tools/verify_pass509_dm1_v1_wallset_startup_binding.py"),
    ]
    failures = [Path(gate["command"][-1]).name for gate in prereqs if not gate["passed"]]
    manifest = {
        "schema": "pass511_dm1_v1_viewport_wall_reference_lineage.v1",
        "status": "passed" if not failures else "failed",
        "redmcsbPrimarySourceRoot": str(RED),
        "redmcsbChecks": red,
        "secondaryLineageChecks": [],
        "dm1Anchors": anchors,
        "prerequisiteGates": prereqs,
        "claims": [
            "ReDMCSB is the authoritative DM1 V1 viewport wall/occlusion source.",
            "Canonical DM1 PC34 GRAPHICS.DAT and DUNGEON.DAT are hash-locked in the supplied retail archive.",
        ],
        "blockers": [
            "No new original same-viewport runtime capture was produced.",
            "No Firestaff-vs-original pixel parity promotion is claimed.",
            "Runtime and pixel-parity closure are tracked by their own gates rather than this source-lineage audit.",
        ],
        "failures": failures,
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = ["# Pass511 DM1 V1 viewport/wall reference lineage", "", f"Status: {manifest['status']}", "", "## ReDMCSB primary anchors"]
    lines += [f"- {item['file']}:{item['lines']} {item['id']}" for item in red]
    lines += ["", "## DM1 canonical anchors"]
    lines += [f"- {item['name']} sha256 {item['sha256']} bytes {item['bytes']}" for item in anchors]
    lines += ["", "## Blockers"] + [f"- {item}" for item in manifest["blockers"]]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"{manifest['status'].upper()} pass511_dm1_v1_viewport_wall_reference_lineage")
    print(f"- wrote {OUT.relative_to(ROOT)}")
    print(f"- wrote {REPORT.relative_to(ROOT)}")
    for item in red:
        print(f"- ReDMCSB {item['file']}:{item['lines']} {item['id']}")
    for failure in failures:
        print(f"FAIL {failure}")
    return 0 if not failures else 1


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)
