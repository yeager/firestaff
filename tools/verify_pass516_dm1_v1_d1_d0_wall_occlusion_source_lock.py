#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
DM1 = Path("~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1").expanduser()
MANIFEST = ROOT / "parity-evidence/verification/pass516_dm1_v1_d1_d0_wall_occlusion_source_lock/manifest.json"
REPORT = ROOT / "parity-evidence/pass516_dm1_v1_d1_d0_wall_occlusion_source_lock.md"
STATUS = "PASS516_DM1_V1_D1_D0_WALL_OCCLUSION_SOURCE_LOCKED"

ALLOWED = [
    ROOT.resolve(),
    Path("~/.openclaw/data/firestaff-redmcsb-source").expanduser().resolve(),
    Path("~/.openclaw/data/firestaff-original-games/DM").expanduser().resolve(),
]

SOURCE_CHECKS = [
    {"id": "f0128-d1-then-d0-near-order", "path": RED / "DUNVIEW.C", "lines": "8524-8542", "claim": "F0128 draws D1L, D1R, D1C, then D0L, D0R, and finally D0C.", "ordered": [
        "F0122_DUNGEONVIEW_DrawSquareD1L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0123_DUNGEONVIEW_DrawSquareD1R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0125_DUNGEONVIEW_DrawSquareD0L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0126_DUNGEONVIEW_DrawSquareD0R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
    ]},
    {"id": "f0122-d1l-side-wall-return", "path": RED / "DUNVIEW.C", "lines": "7436-7460", "claim": "D1L wall draws the D1L side zone, tests only the facing ornament, then returns before open-cell content.", "ordered": [
        "case C00_ELEMENT_WALL:",
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C02_WALL_D1R], C713_ZONE_WALL_D1L);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C03_WALL_D1L], C713_ZONE_WALL_D1L);",
        "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0214_ai_SquareAspect[M551_RIGHT_WALL_ORNAMENT_ORDINAL], M585_VIEW_WALL_D1L_RIGHT);",
        "return;",
    ]},
    {"id": "f0123-d1r-side-wall-return", "path": RED / "DUNVIEW.C", "lines": "7604-7628", "claim": "D1R mirrors D1L: opposite bitmap when flipped, D1R zone, ornament probe, then return.", "ordered": [
        "case C00_ELEMENT_WALL:",
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C03_WALL_D1L], C714_ZONE_WALL_D1R);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C02_WALL_D1R], C714_ZONE_WALL_D1R);",
        "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0216_ai_SquareAspect[M553_LEFT_WALL_ORNAMENT_ORDINAL], M586_VIEW_WALL_D1R_LEFT);",
        "return;",
    ]},
    {"id": "f0124-d1c-front-wall-alcove-exception", "path": RED / "DUNVIEW.C", "lines": "7828-7844", "claim": "D1C front wall is opaque/centered and reveals contained content only for the explicit front-alcove exception.", "ordered": [
        "F0792_DUNGEONVIEW_DrawBitmapYYY(G2107_WallSet[C04_WALL_D1C], C712_ZONE_WALL_D1C, G0076_B_UseFlippedWallAndFootprintsBitmaps);",
        "F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2107_WallSet[C04_WALL_D1C], C712_ZONE_WALL_D1C);",
        "if (F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0218_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL], M587_VIEW_WALL_D1C_FRONT))",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING], P0171_i_Direction, P0172_i_MapX, P0173_i_MapY, M606_VIEW_SQUARE_D1C, C0x0000_CELL_ORDER_ALCOVE);",
    ]},
    {"id": "f0125-f0126-d0-side-walls-return-before-field", "path": RED / "DUNVIEW.C", "lines": "8007-8159", "claim": "D0 side-wall cases return before the side-lane teleporter-field tail; open D0 lanes use one back cell only.", "ordered": [
        "case C00_ELEMENT_WALL:",
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C00_WALL_D0R], C716_ZONE_WALL_D0L);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C01_WALL_D0L], C716_ZONE_WALL_D0L);",
        "return;",
        "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M610_VIEW_SQUARE_D0L]], C716_ZONE_WALL_D0L);",
        "case C00_ELEMENT_WALL:",
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C01_WALL_D0L], C717_ZONE_WALL_D0R);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C00_WALL_D0R], C717_ZONE_WALL_D0R);",
        "return;",
        "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M611_VIEW_SQUARE_D0R]], C717_ZONE_WALL_D0R);",
    ]},
]

LOCAL_CHECKS = [
    {"id": "local-d1-d0-wall-specs-present", "path": ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "lines": "281-312", "claim": "Firestaff exposes D1/D0 wall metadata with ReDMCSB return/alcove source anchors.", "ordered": [
        "DM1_VIEW_SQUARE_D1L,  DM1_WALL_D1L,  DM1_WALL_D1R",
        "DUNVIEW.C:7459-7460 side ornament then return",
        "DM1_VIEW_SQUARE_D1R,  DM1_WALL_D1R,  DM1_WALL_D1L",
        "DUNVIEW.C:7627-7628 side ornament then return",
        "DM1_VIEW_SQUARE_D1C,  DM1_WALL_D1C,  DM1_WALL_D1C",
        "DUNVIEW.C:7842-7843 front alcove draws F0115",
        "DM1_VIEW_SQUARE_D0L,  DM1_WALL_D0L,  DM1_WALL_D0R",
        "DUNVIEW.C:8036-8038 wall case returns",
        "DM1_VIEW_SQUARE_D0R,  DM1_WALL_D0R,  DM1_WALL_D0L",
        "DUNVIEW.C:8142-8144 wall case returns",
    ]},
    {"id": "local-side-occlusion-d1-d0-cell-orders-present", "path": ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "lines": "139-171", "claim": "Open side branches keep their source cell-order contracts separate from wall-return blockers.", "ordered": [
        "DM1_VIEW_SQUARE_D1L, 0x0032",
        "DM1_VIEW_SQUARE_D1R, 0x0041",
        "DM1_VIEW_SQUARE_D0L, 0x0002",
        "DM1_VIEW_SQUARE_D0R, 0x0001",
    ]},
    {"id": "local-runtime-test-covers-d1-d0-wall-occlusion", "path": ROOT / "tests/test_dm1_v1_viewport_3d_pc34_compat.c", "lines": "234-310", "claim": "The narrow runtime test checks D1/D0 zone/pairing and wall item occlusion outcomes.", "ordered": [
        "DM1_VIEW_SQUARE_D1L,  DM1_WALL_D1L,  DM1_WALL_D1R",
        "DM1_VIEW_SQUARE_D1R,  DM1_WALL_D1R,  DM1_WALL_D1L",
        "DM1_VIEW_SQUARE_D1C,  DM1_WALL_D1C,  DM1_WALL_D1C",
        "DM1_VIEW_SQUARE_D0L,  DM1_WALL_D0L,  DM1_WALL_D0R",
        "DM1_VIEW_SQUARE_D0R,  DM1_WALL_D0R,  DM1_WALL_D0L",
        "test_wall_item_occlusion_alcove_exception",
    ]},
]

ANCHORS = ["GRAPHICS.DAT", "DUNGEON.DAT", "TITLE", "README.md"]

def allow(path: Path) -> Path:
    resolved = path.expanduser().resolve()
    if not any(str(resolved).startswith(str(prefix)) for prefix in ALLOWED):
        raise SystemExit(f"refusing non-local evidence path: {path}")
    return resolved

def read(path: Path) -> str:
    return allow(path).read_text(encoding="latin-1", errors="replace")

def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with allow(path).open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def slice_lines(text: str, spec: str) -> tuple[int, str]:
    start, end = (int(part) for part in spec.split("-", 1))
    return start, "\n".join(text.splitlines()[start - 1:end])

def line_for(body: str, base: int, pos: int) -> int:
    return base + body.count("\n", 0, pos)

def audit(checks: list[dict[str, object]]) -> list[dict[str, object]]:
    rows = []
    for check in checks:
        path = allow(check["path"])  # type: ignore[arg-type]
        base, body = slice_lines(read(path), check["lines"])  # type: ignore[arg-type]
        cursor = 0
        hits = []
        missing = []
        for needle in check["ordered"]:  # type: ignore[index]
            pos = body.find(needle, cursor)
            if pos < 0:
                missing.append(needle)
            else:
                hits.append({"line": line_for(body, base, pos), "needle": needle})
                cursor = pos + len(needle)
        rows.append({"id": check["id"], "status": "PASS" if not missing else "FAIL", "path": str(path), "sourceFile": path.name, "lines": check["lines"], "sha256": sha256(path), "claim": check["claim"], "hits": hits, "missing": missing})
    return rows

def run(command: list[str]) -> dict[str, object]:
    proc = subprocess.run(command, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {"command": command, "returncode": proc.returncode, "passed": proc.returncode == 0, "outputTail": "\n".join(proc.stdout.strip().splitlines()[-12:])}

def local_refs() -> list[dict[str, object]]:
    refs = []
    for name in ANCHORS:
        path = allow(DM1 / name)
        row = {"id": f"dm1-{name.lower()}", "path": str(path), "exists": path.exists()}
        if path.is_file():
            row.update({"bytes": path.stat().st_size, "sha256": sha256(path)})
        refs.append(row)
    return refs

def write_report(manifest: dict[str, object]) -> None:
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = ["# Pass516 DM1 V1 D1/D0 wall occlusion source lock", "", f"Status: {manifest['status']}", "", "## Claim", "", "ReDMCSB composes D1 before D0, then D0C last. D1L/D1R and D0L/D0R side-wall cases draw their side wall zone and return before open-cell content/field tails. D1C front wall only reveals contained things through the explicit front-alcove exception.", "", "## Primary ReDMCSB Evidence"]
    for row in manifest["redmcsbChecks"]:  # type: ignore[index]
        lines += ["", f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})", f"  - {row['claim']}"]
        lines += [f"  - line {hit['line']}: {hit['needle']}" for hit in row["hits"]]
        lines += [f"  - missing: {miss}" for miss in row["missing"]]
    lines += ["", "## Firestaff Evidence"]
    for row in manifest["firestaffChecks"]:  # type: ignore[index]
        lines += ["", f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})", f"  - {row['claim']}"]
    lines += ["", "## Verification"]
    for row in manifest["verificationRuns"]:  # type: ignore[index]
        lines += ["", f"- command: {' '.join(row['command'])}", f"  - returncode: {row['returncode']}", "  - output tail:", "~~~", row["outputTail"], "~~~"]
    lines += ["", "## Non-Claims", "", "- No input or movement dispatch code is changed.", "- No renderer runtime behavior is changed.", "- DANNESBURK was not used."]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")

def main(check_only: bool = False) -> int:
    redmcsb = audit(SOURCE_CHECKS)
    firestaff = audit(LOCAL_CHECKS)
    failed = [row["id"] for row in redmcsb + firestaff if row["status"] != "PASS"]
    if check_only:
        print("PASS pass516 check-only" if not failed else "FAIL pass516 check-only: " + ",".join(failed))
        return 0 if not failed else 1
    runtime = run([str(ROOT / "build" / "test_dm1_v1_viewport_3d_pc34_compat")])
    check = run([sys.executable, str(Path(__file__).resolve()), "--check-only"])
    refs = local_refs()
    ok = not failed and runtime["passed"] and check["passed"] and all(row["exists"] for row in refs)
    manifest = {"schema": "pass516_dm1_v1_d1_d0_wall_occlusion_source_lock.v1", "status": "passed" if ok else "failed", "statusToken": STATUS if ok else "FAILED_PASS516_DM1_V1_D1_D0_WALL_OCCLUSION_SOURCE_LOCK", "redmcsbRoot": str(RED), "redmcsbChecks": redmcsb, "firestaffChecks": firestaff, "localReferences": refs, "verificationRuns": [runtime, check]}
    write_report(manifest)
    print(manifest["statusToken"])
    print(f"- wrote {MANIFEST.relative_to(ROOT)}")
    print(f"- wrote {REPORT.relative_to(ROOT)}")
    return 0 if ok else 1

if __name__ == "__main__":
    raise SystemExit(main("--check-only" in sys.argv))
