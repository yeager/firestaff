#!/usr/bin/env python3
from __future__ import annotations

import json
import os
import subprocess
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
DUNVIEW = RED / "DUNVIEW.C"
DRAWVIEW = RED / "DRAWVIEW.C"
LOCAL_C = ROOT / "dm1_v1_viewport_3d_pc34_compat.c"
LOCAL_H = ROOT / "dm1_v1_viewport_3d_pc34_compat.h"
PROBE = ROOT / "probes/dm1/firestaff_dm1_v1_wall_composition_contract_probe.c"
PASS496 = ROOT / "parity-evidence/verification/pass496_dm1_v1_wall_occlusion_spec_matrix/manifest.json"
VERIFY_DIR = ROOT / "parity-evidence/verification/pass499_dm1_v1_wall_occlusion_runtime_evidence_gate"
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence/pass499_dm1_v1_wall_occlusion_runtime_evidence_gate.md"
STATUS = "PASS499_DM1_V1_WALL_OCCLUSION_RUNTIME_EVIDENCE_GATE_LOCKED"

SOURCE_WINDOWS = [
    {
        "id": "dunview_f0128_far_to_near_then_present",
        "file": DUNVIEW,
        "refs": ["DUNVIEW.C:8318-8610"],
        "needles_in_order": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling",
            "F0116_DUNGEONVIEW_DrawSquareD3L",
            "F0117_DUNGEONVIEW_DrawSquareD3R",
            "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF",
            "F0119_DUNGEONVIEW_DrawSquareD2L",
            "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF",
            "F0121_DUNGEONVIEW_DrawSquareD2C",
            "F0122_DUNGEONVIEW_DrawSquareD1L",
            "F0123_DUNGEONVIEW_DrawSquareD1R",
            "F0124_DUNGEONVIEW_DrawSquareD1C",
            "F0125_DUNGEONVIEW_DrawSquareD0L",
            "F0126_DUNGEONVIEW_DrawSquareD0R",
            "F0127_DUNGEONVIEW_DrawSquareD0C",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "claim": "F0128 composes floor/ceiling and wall squares in far-to-near order, then hands the composed viewport to F0097.",
    },
    {
        "id": "drawview_f0097_present_boundary",
        "file": DRAWVIEW,
        "refs": ["DRAWVIEW.C:709-858"],
        "needles_in_order": [
            "void F0097_DUNGEONVIEW_DrawViewport",
            "G0324_B_DrawViewportRequested = C1_TRUE;",
            "F0638_GetZone(C007_ZONE_VIEWPORT",
            "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);",
        ],
        "claim": "F0097 marks the redraw request and reaches the PC34 viewport blit boundary for G0296.",
    },
]

FIRESTAFF_LOCKS = [
    ("wall_spec_table", LOCAL_C, "static const DM1_ViewportWallDrawSpec s_wall_draw_specs[]"),
    ("wall_spec_accessor_count", LOCAL_C, "dm1_viewport_3d_wall_draw_spec_count"),
    ("wall_spec_accessor_by_square", LOCAL_C, "dm1_viewport_3d_get_wall_draw_spec_for_square"),
    ("door_front_occlusion_specs", LOCAL_C, "s_door_front_occlusion_specs"),
    ("wall_contract_probe_expected_matrix", PROBE, "struct ExpectedWallContract"),
    ("wall_contract_probe_source_output", PROBE, "wallCompositionMatrix source="),
    ("door_contract_probe_output", PROBE, "doorFrontComposition source="),
    ("public_wall_spec_type", LOCAL_H, "DM1_ViewportWallDrawSpec"),
    ("public_door_front_spec_type", LOCAL_H, "DM1_ViewportDoorFrontOcclusionSpec"),
]

PROBE_TEST = "firestaff_dm1_v1_wall_composition_contract_probe"
PRIOR_GATES = [
    "pass496_dm1_v1_wall_occlusion_spec_matrix",
    "pass490_dm1_v1_wall_occlusion_merge_readiness",
    "dm1_v1_viewport_3d_occlusion_metadata_gate",
]


def read(path: Path) -> str:
    return path.read_text(encoding="latin-1", errors="replace")


def line_no(text: str, needle: str) -> int | None:
    pos = text.find(needle)
    if pos < 0:
        return None
    return text[:pos].count("\n") + 1


def ordered_check(spec: dict) -> dict:
    text = read(spec["file"])
    cursor = 0
    lines: list[int | None] = []
    missing: list[str] = []
    for needle in spec["needles_in_order"]:
        pos = text.find(needle, cursor)
        if pos < 0:
            missing.append(needle)
            lines.append(line_no(text, needle))
        else:
            lines.append(text[:pos].count("\n") + 1)
            cursor = pos + len(needle)
    return {**spec, "file": str(spec["file"]), "ok": not missing and all(line is not None for line in lines), "lines": lines, "missing": missing}


def local_lock_check(item: tuple[str, Path, str]) -> dict:
    cid, path, needle = item
    text = path.read_text(encoding="utf-8", errors="replace") if path.exists() else ""
    return {"id": cid, "file": str(path), "ok": path.exists() and needle in text, "line": line_no(text, needle), "needle": needle}


def run(cmd: list[str], timeout: int = 120) -> dict:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    return {"cmd": cmd, "returncode": proc.returncode, "ok": proc.returncode == 0, "tail": proc.stdout[-4000:]}


def main() -> int:
    problems: list[str] = []
    source = [ordered_check(spec) for spec in SOURCE_WINDOWS]
    firestaff = [local_lock_check(item) for item in FIRESTAFF_LOCKS]

    if not PASS496.exists():
        problems.append("missing pass496 manifest")
        pass496_payload = {}
    else:
        pass496_payload = json.loads(PASS496.read_text(encoding="utf-8"))
        if pass496_payload.get("ok") is not True:
            problems.append("pass496 manifest is not ok")

    results = pass496_payload.get("results", []) if isinstance(pass496_payload, dict) else []
    coverage = {
        "sourceWindowsOk": all(row["ok"] for row in source),
        "firestaffLocksOk": all(row["ok"] for row in firestaff),
        "pass496MatrixRows": len(results),
        "pass496SourceRowsOk": sum(1 for row in results if row.get("ok") and "line_range" in row),
        "pass496LocalRowsOk": sum(1 for row in results if row.get("ok") and "line" in row),
    }
    if coverage["pass496MatrixRows"] < 15:
        problems.append("pass496 coverage unexpectedly small")

    candidates = [
        Path(os.environ["FIRESTAFF_BUILD_DIR"]) if "FIRESTAFF_BUILD_DIR" in os.environ else None,
        ROOT / "build",
        Path("/tmp/firestaff-blockers-build-current"),
    ]
    build_dir = next((p for p in candidates if p and (p / "CTestTestfile.cmake").exists()), ROOT / "build")
    probe_run = None
    if build_dir.exists():
        gate_re = f"^({'|'.join([PROBE_TEST] + PRIOR_GATES)})$"
        probe_run = run(["ctest", "--test-dir", str(build_dir), "-R", gate_re, "--output-on-failure"], timeout=240)
        if not probe_run["ok"]:
            problems.append("compiled probe/prior gates failed")
    else:
        problems.append("missing build directory for compiled probe test")

    problems.extend(f"source lock failed: {row['id']}" for row in source if not row["ok"])
    problems.extend(f"firestaff lock failed: {row['id']}" for row in firestaff if not row["ok"])

    status = STATUS if not problems else "FAIL_PASS499_DM1_V1_WALL_OCCLUSION_RUNTIME_EVIDENCE_GATE"
    promotion = "Promote wall-occlusion runtime evidence only when pass496 source/spec matrix is ok, the compiled Firestaff wall-composition probe passes, and a runtime/capture lane reaches the F0128 to F0097 present boundary for the same viewport. This gate is source/probe coverage only."
    manifest = {
        "schema": "firestaff.parity.pass499_dm1_v1_wall_occlusion_runtime_evidence_gate.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "ok": not problems,
        "sourceRoot": str(RED),
        "sourceLocks": source,
        "firestaffLocks": firestaff,
        "coverage": coverage,
        "probeTest": PROBE_TEST,
        "priorGates": PRIOR_GATES,
        "probeRun": probe_run,
        "promotionPredicate": promotion,
        "nonClaims": [
            "no new DOSBox debugger stop",
            "no original-vs-Firestaff pixel parity",
            "no promotion of static repeated screenshots",
        ],
        "problems": problems,
    }
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass499 — DM1 V1 wall occlusion runtime evidence gate",
        "",
        f"Status: `{status}`",
        "",
        "## Decision",
        "",
        promotion,
        "",
        "## Source locks",
    ]
    for row in source:
        lines.append(f"- `{row['id']}` ok={row['ok']} refs={', '.join(row['refs'])}: {row['claim']}")
    lines += ["", "## Firestaff coverage", ""]
    for row in firestaff:
        lines.append(f"- `{row['id']}` ok={row['ok']} line={row['line']}")
    lines += [
        "",
        "## Gates",
        "",
        f"- compiled probe: `{PROBE_TEST}`",
        f"- prior gates: `{', '.join(PRIOR_GATES)}`",
        f"- pass496 manifest rows: `{coverage['pass496MatrixRows']}`",
        "",
        "## Non-claims",
        "",
    ]
    lines.extend(f"- {item}" for item in manifest["nonClaims"])
    if problems:
        lines += ["", "## Problems", ""]
        lines.extend(f"- {problem}" for problem in problems)
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(status)
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    return 0 if not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())
