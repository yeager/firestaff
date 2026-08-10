#!/usr/bin/env python3
"""Pass376 verifier: credit pass375 deferred explosion pass in the completion matrix."""
from __future__ import annotations

import json
import pathlib
import subprocess
import sys
from datetime import datetime, timezone
from typing import Any

ROOT = pathlib.Path(__file__).resolve().parents[1]
PASS = "pass376_dm1_v1_completion_deferred_explosion_credit"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
PASS375_EVIDENCE = ROOT / "parity-evidence" / "pass375_dm1_v1_deferred_explosion_pass.md"
PASS375_VERIFIER = ROOT / "tools" / "verify_pass375_dm1_v1_deferred_explosion_pass.py"
MATRIX = ROOT / "parity-evidence" / "verification" / "firestaff_completion_matrix.json"
DOC = ROOT / "docs" / "parity" / "COMPLETION_MATRIX.md"
EXPECTED_STATUS = "PASS376_DM1_V1_DEFERRED_EXPLOSION_COMPLETION_CREDIT_PROVED"


def run(cmd: list[str], timeout: int = 120) -> dict[str, Any]:
    p = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    return {"cmd": cmd, "returncode": p.returncode, "outputTail": p.stdout[-4000:]}


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    checks: list[dict[str, Any]] = []

    pass375 = run([sys.executable, str(PASS375_VERIFIER.relative_to(ROOT))])
    checks.append({
        "kind": "pass375_deferred_explosion_prerequisite",
        "ok": pass375["returncode"] == 0 and "status=PASS_PASS375_DM1_V1_DEFERRED_EXPLOSION_PASS" in pass375["outputTail"],
        "result": pass375,
    })

    evidence = PASS375_EVIDENCE.read_text(encoding="utf-8") if PASS375_EVIDENCE.exists() else ""
    for needle in [
        "DUNVIEW.C:5915-5933",
        "m11_draw_effect_cue() no longer draws explosions",
        "after all visible side and center object/creature/projectile contents have drawn",
        "It preserves the existing source-backed explosion bitmap code",
    ]:
        checks.append({"kind": "pass375_evidence_marker", "needle": needle, "ok": needle in evidence})

    matrix = json.loads(MATRIX.read_text(encoding="utf-8"))
    dm1 = next(r for r in matrix["rows"] if r["target"] == "DM1 V1")
    viewport_score, viewport_note = dm1["scores"]["viewport_ui_render"]
    # pass376 originally landed DM1 at 57/58% with the pass375 deferred
    # explosion credit; the matrix has since compounded further credits
    # into the broader viewport/HUD/mirror note.  Accept any current
    # score >= the pass376 baseline (12 viewport / 57 completion).
    checks.append({
        "kind": "completion_matrix_credit",
        "ok": dm1["completionPercent"] >= 57 and dm1["points"] >= 57 and viewport_score >= 12,
        "observed": {"completionPercent": dm1["completionPercent"], "points": dm1["points"], "viewport_ui_render": viewport_score, "note": viewport_note},
    })

    doc = DOC.read_text(encoding="utf-8")
    # The evidence doc's DM1 V1 row now carries the compounded completion
    # percentage instead of the exact 57/58% pass376 asserted.  Accept
    # any DM1 V1 row whose percent is at least 57.
    dm1_row_ok = False
    import re as _re
    m = _re.search(r"\|\s*DM1 V1\s*\|\s*(\d+)%\s*\|\s*(\d+)/100\s*\|", doc)
    if m and int(m.group(1)) >= 57 and int(m.group(2)) >= 57:
        dm1_row_ok = True
    checks.append({
        "kind": "completion_doc_credit",
        "ok": dm1_row_ok,
    })
    r = run([sys.executable, "tools/verify_firestaff_completion_matrix.py"])
    checks.append({"kind": "firestaff_completion_matrix_verifier", "ok": r["returncode"] == 0, "result": r})
    r = run([sys.executable, "tools/firestaff_completion_status.py"])
    m = _re.search(r"DM1 V1 \| completionPercent=(\d+)%", r["outputTail"])
    checks.append({"kind": "firestaff_completion_status_cli", "ok": r["returncode"] == 0 and bool(m) and int(m.group(1)) >= 57, "result": r})

    ok = all(c.get("ok") for c in checks)
    status = EXPECTED_STATUS if ok else "BLOCKED_PASS376_DM1_V1_DEFERRED_EXPLOSION_COMPLETION_CREDIT"
    manifest = {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"])["outputTail"].strip(),
        "head": run(["git", "rev-parse", "HEAD"])["outputTail"].strip(),
        "completionImpact": "DM1 V1 verified completion increases from 56/100 to 57/100 by crediting one additional viewport_ui_render point for pass375's ReDMCSB F0115 after-all-packed-cells deferred explosion pass.",
        "notClaimed": ["pixel-perfect viewport parity", "representative original overlay regression", "full object/creature/projectile gameplay parity", "original DOS runtime explosion overlay comparison"],
        "checks": checks,
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass376 — DM1 V1 deferred-explosion completion credit",
        "",
        f"Status: `{status}`",
        "",
        "## ReDMCSB source audit first",
        "",
        "- `DUNVIEW.C:4547-4582` defines `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF` and its per-cell object/creature/projectile plus later explosion-pass contract.",
        "- `DUNVIEW.C:5915-5933` exits the packed-cell loop, starts `/* Draw explosions */`, restarts the thing list, and filters `C15_THING_TYPE_EXPLOSION` after the prior cell passes.",
        "- pass375 verifies Firestaff now keeps projectiles in the per-cell effect path while moving explosions into `m11_draw_dm1_deferred_explosion_pass()` after visible side and center contents.",
        "",
        "## Landable update",
        "",
        "This pass credits pass375 in the conservative completion matrix: DM1 V1 moves from `56/100` to `57/100`; `viewport_ui_render` moves from `11/20` to `12/20`.",
        "",
        "The credit is narrow: it covers the source-locked layer boundary for viewport explosions in the movement/viewport/wall renderer. It does not claim pixel parity, representative original overlay regression, or complete gameplay-system parity.",
        "",
        "## Gates",
        "",
    ]
    for c in checks:
        lines.append("- `{}` ok={}".format(c["kind"], c.get("ok")))
    lines += ["", f"Manifest: `{MANIFEST.relative_to(ROOT)}`"]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(json.dumps({"status": status, "manifest": str(MANIFEST.relative_to(ROOT)), "report": str(REPORT.relative_to(ROOT))}, indent=2))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
