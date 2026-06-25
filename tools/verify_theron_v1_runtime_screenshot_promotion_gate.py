#!/usr/bin/env python3
"""Theron V1 runtime screenshot README-promotion provenance gate.

This is a bounded, machine-checkable contract that decides which rows in the
existing ``parity-evidence/verification/theron_v1_runtime_screenshot_readiness``
manifest are eligible to be promoted into a public README screenshot, and
records the current eligibility state plus an explicit non-promotion lock
for any rows that fail one or more eligibility criteria.

The gate does NOT promote screenshots, copy image bytes into
``verification-screens/`` or ``docs/compare/``, or rewrite the README. It only
audits the existing readiness receipts against an explicit eligibility
contract and writes a manifest that future promotion work must satisfy.

Eligibility contract (must hold for every README-eligible row):

1. **Real Firestaff runtime capture** - ``probe.sourceId`` must be ``theron``
   and ``probe.launchedEver`` must equal ``1`` (i.e. the row was produced by
   a real ``firestaff --game theron`` launch, not a synthetic probe fixture).
2. **No deterministic fallback assets** - the launch must not have emitted
   the ``deterministic fallback assets`` marker. Rows that rely on the
   placeholder art are runtime/capture-path proof only, not real Theron
   Track 02 bank art, and must never be promoted as Theron README
   screenshots.
3. **Track 02 boot milestone** - the boot marker
   (``TQR level load``) must be present in the launch output.
4. **Semantic Track 02 loader evidence** - the runtime probe must show that
   Firestaff actually decoded Track 02 content rather than parking at boot.
   The gate requires either a non-default ``mapIndex``, a non-zero
   ``gameTick``, or a non-empty ``lastOutcome`` beyond ``THERON READY``
   (e.g. ``THERON READY (TILE BANK LOADED)`` once semantic decode ships).
5. **Unique source BMP** - the source ``sha256`` must not be byte-identical
   to another row's source BMP. A duplicated sha256 across rows is a
   strong signal that the frames share a placeholder fixture rather than
   independent runtime captures.
6. **Presented BMP geometry** - the presented BMP must be a valid 320x200
   24-bit BMP with more than 200 non-black pixels, matching the existing
   readiness gate.

If any criterion fails, the row is recorded as
``README_INELIGIBLE`` and the gate explicitly refuses to promote it.
The aggregate gate status is ``PASS`` only when the eligibility audit
runs cleanly (every row classified, contract fields locked, no row
silently promoted); the gate does NOT promote rows by default.

This keeps the public docs honest: it lets future operators see which
rows are eligible, which are not, and why, without claiming any current
Theron capture is README-ready unless the evidence says so.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


PASS = "theron_v1_runtime_screenshot_promotion_gate"
ROOT = Path(__file__).resolve().parents[1]
READINESS_MANIFEST = (
    ROOT
    / "parity-evidence"
    / "verification"
    / "theron_v1_runtime_screenshot_readiness"
    / "manifest.json"
)
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence" / f"{PASS}.md"

# Locks for the eligibility contract. If any of these fields drift, the gate
# fails fast so the contract does not silently change.
REQUIRED_CONTRACT_FIELDS = {
    "schema": "firestaff.parity.theron_v1_runtime_screenshot_promotion_gate.v1",
    "currentDecision": "NO_README_PROMOTION_PERMITTED",
    "nonPromotionStatus": "LOCKED_NO_ROW_README_ELIGIBLE",
    "bootstrapSource": "parity-evidence/verification/theron_v1_runtime_screenshot_readiness",
}

NON_CLAIMS = [
    "No Theron screenshot is promoted into README, verification-screens/, or docs/compare/ by this gate.",
    "No generated, illustrated, or mock Theron image is created or copied.",
    "No claim that any current Theron readiness row is README-eligible.",
    "No claim of full Theron runtime playability or semantic Track 02 dungeon-table parity.",
    "The contract is a non-promotion lock, not a release gate; future promotion must satisfy every eligibility criterion in the contract.",
]


def rel_or_str(path: Path) -> str:
    try:
        return str(path.relative_to(ROOT))
    except ValueError:
        return str(path)


def load_readiness_manifest() -> dict[str, Any]:
    if not READINESS_MANIFEST.exists():
        return {}
    try:
        return json.loads(READINESS_MANIFEST.read_text(encoding="utf-8"))
    except json.JSONDecodeError:
        return {}


def classify_row(row: dict[str, Any]) -> dict[str, Any]:
    """Run the eligibility contract against a single readiness row."""
    notes: list[str] = []
    eligible = True

    if not row.get("present", False):
        return {
            "classification": "SKIPPED_NO_DATA",
            "eligible": False,
            "notes": ["data directory missing; row skipped, not promoted"],
        }

    if row.get("status") != "PASS":
        eligible = False
        notes.append(f"readiness row status is {row.get('status')!r}, not PASS")

    probe = row.get("probe") or {}
    if probe.get("sourceId") != "theron":
        eligible = False
        notes.append(
            f"probe.sourceId is {probe.get('sourceId')!r}, expected 'theron'"
        )
    if probe.get("launchedEver") != 1:
        eligible = False
        notes.append(
            f"probe.launchedEver is {probe.get('launchedEver')!r}, expected 1 (real runtime launch)"
        )
    if probe.get("schema") != "firestaff_m11_autotest_runtime_probe.v1":
        eligible = False
        notes.append(
            f"probe.schema is {probe.get('schema')!r}, expected 'firestaff_m11_autotest_runtime_probe.v1'"
        )

    if row.get("fallback_assets_used"):
        eligible = False
        notes.append(
            "row used 'deterministic fallback assets'; placeholder art cannot be promoted as a real Theron Track 02 README screenshot"
        )

    if not row.get("marker_found"):
        eligible = False
        notes.append("'TQR level load' boot marker not present in launch output")

    game_tick = probe.get("gameTick", 0)
    map_index = (probe.get("party") or {}).get("mapIndex", 0)
    last_outcome = probe.get("lastOutcome", "")
    semantic_evidence = (
        (isinstance(game_tick, int) and game_tick > 0)
        or (isinstance(map_index, int) and map_index != 0)
        or (
            isinstance(last_outcome, str)
            and last_outcome not in {"", "THERON READY"}
        )
    )
    if not semantic_evidence:
        eligible = False
        notes.append(
            "no semantic Track 02 loader evidence in probe "
            f"(gameTick={game_tick!r}, party.mapIndex={map_index!r}, lastOutcome={last_outcome!r}); "
            "boot probe parked at THERON READY without decoding Track 02 content"
        )

    source_shots = row.get("screenshots") or []
    presented_shots = row.get("presented_screenshots") or []
    source_sha = source_shots[0].get("sha256") if source_shots else None
    presented_ok = (
        len(presented_shots) == 1
        and presented_shots[0].get("valid")
        and presented_shots[0].get("width") == 320
        and presented_shots[0].get("height") == 200
        and presented_shots[0].get("non_black_pixels", 0) > 200
    )
    if not presented_ok:
        eligible = False
        notes.append("presented BMP failed geometry/non-black checks (must be 320x200, >200 non-black pixels)")

    classification = "README_ELIGIBLE" if eligible else "README_INELIGIBLE"
    return {
        "classification": classification,
        "eligible": eligible,
        "notes": notes,
        "source_sha256": source_sha,
        "presented_sha256": (presented_shots[0].get("sha256") if presented_shots else None),
        "probe_summary": {
            "sourceId": probe.get("sourceId"),
            "launchedEver": probe.get("launchedEver"),
            "gameTick": game_tick,
            "party.mapIndex": map_index,
            "lastOutcome": last_outcome,
        },
    }


def audit_uniqueness(rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    """Detect identical source BMP sha256 across rows (a placeholder fixture smell).

    Runs across every readiness row (not only currently-eligible ones) because a
    duplicate sha256 is an independent reason to refuse promotion: it suggests
    the rows share a placeholder fixture rather than independent runtime
    captures, even if other eligibility criteria happen to pass.
    """
    grouped: dict[str, list[str]] = {}
    for row in rows:
        if row.get("classification") == "SKIPPED_NO_DATA":
            continue
        sha = row.get("source_sha256")
        if not sha:
            continue
        grouped.setdefault(sha, []).append(row["case_id"])
    findings: list[dict[str, Any]] = []
    for sha, case_ids in grouped.items():
        if len(case_ids) <= 1:
            continue
        findings.append(
            {
                "kind": "duplicate_source_sha256_across_rows",
                "sha256": sha,
                "rows": case_ids,
                "note": "identical source BMP sha256 across multiple readiness rows is a strong indicator of a shared placeholder fixture rather than independent runtime captures",
            }
        )
        for row in rows:
            if row["case_id"] in case_ids:
                row["eligible"] = False
                row["classification"] = "README_INELIGIBLE"
                if not any("shared with rows" in n for n in row["notes"]):
                    row["notes"].append(
                        f"sha256 {sha} shared with rows {case_ids}; ineligible"
                    )
    return findings


def aggregate(rows: list[dict[str, Any]], contract_drift: list[str], uniqueness: list[dict[str, Any]]) -> dict[str, Any]:
    eligible_rows = [r for r in rows if r["classification"] == "README_ELIGIBLE"]
    ineligible_rows = [r for r in rows if r["classification"] == "README_INELIGIBLE"]
    skipped_rows = [r for r in rows if r["classification"] == "SKIPPED_NO_DATA"]

    if contract_drift:
        decision = "CONTRACT_DRIFT_FAIL"
    elif eligible_rows:
        decision = "READINESS_GATE_REPORTS_ELIGIBLE_ROWS_REVIEW_REQUIRED"
    else:
        decision = REQUIRED_CONTRACT_FIELDS["currentDecision"]

    return {
        "status": decision,
        "decision": decision,
        "eligible_count": len(eligible_rows),
        "ineligible_count": len(ineligible_rows),
        "skipped_count": len(skipped_rows),
        "uniqueness_findings": uniqueness,
        "contract_drift": contract_drift,
    }


def write_outputs(result: dict[str, Any]) -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Theron V1 runtime screenshot promotion gate",
        "",
        f"Status: `{result['status']}`",
        "",
        f"Decision: **{result['decision']}**",
        "",
        "This is a bounded, machine-checkable provenance gate for promoting",
        "Theron runtime screenshots into public docs. It does not promote any",
        "image, copy bytes, or rewrite the README. It only audits the existing",
        "readiness manifest against an explicit eligibility contract.",
        "",
        "## Eligibility contract",
        "",
        "A row is `README_ELIGIBLE` only if **all** of the following hold:",
        "",
        "1. Readiness row `status == \"PASS\"` (real Firestaff boot/tick).",
        "2. Probe `sourceId == \"theron\"`, `launchedEver == 1`, and `schema == \"firestaff_m11_autotest_runtime_probe.v1\"`.",
        "3. Launch output does **not** contain `deterministic fallback assets`.",
        "4. Launch output contains the `TQR level load` boot marker.",
        "5. Probe shows semantic Track 02 loader evidence (`gameTick > 0`, non-zero `party.mapIndex`, or a `lastOutcome` beyond `THERON READY`).",
        "6. Source BMP sha256 is unique across all rows (duplicate sha256s indicate a shared placeholder fixture).",
        "7. Presented BMP is a valid 320x200 24-bit BMP with more than 200 non-black pixels.",
        "",
        "Rows that fail any criterion are recorded as `README_INELIGIBLE`. SKIP is",
        "returned for rows whose data directory is missing on this host.",
        "",
        "## Row classification",
        "",
        "| Case | Classification | Source sha256 | Probe summary | Notes |",
        "|---|---|---|---|---|",
    ]
    for row in result["rows"]:
        probe_summary = row.get("probe_summary") or {}
        summary_text = (
            f"sourceId={probe_summary.get('sourceId')} "
            f"launchedEver={probe_summary.get('launchedEver')} "
            f"gameTick={probe_summary.get('gameTick')} "
            f"mapIndex={probe_summary.get('party.mapIndex')} "
            f"lastOutcome={probe_summary.get('lastOutcome')!r}"
        )
        sha = row.get("source_sha256") or "—"
        notes = "; ".join(row.get("notes", [])) or "—"
        lines.append(
            f"| {row['case_id']} | `{row['classification']}` | `{sha}` | {summary_text} | {notes} |"
        )

    lines += [
        "",
        "## Aggregate",
        "",
        f"- Eligible rows: **{result['aggregate']['eligible_count']}**",
        f"- Ineligible rows: **{result['aggregate']['ineligible_count']}**",
        f"- Skipped rows (no data on this host): **{result['aggregate']['skipped_count']}**",
        f"- Unique-source-sha256 findings: **{len(result['aggregate']['uniqueness_findings'])}**",
        f"- Contract-drift findings: **{len(result['aggregate']['contract_drift'])}**",
        "",
        "## Public Screenshot Boundary",
        "",
        "- This gate is the source of truth for whether a Theron readiness row is `README_ELIGIBLE`.",
        "- README, `verification-screens/`, and `docs/compare/` must not add Theron screenshots until at least one readiness row is `README_ELIGIBLE` AND a human reviewer promotes it from tracked evidence.",
        "- No generated, illustrated, mocked, or synthetic Theron image may be used as a README screenshot.",
        "",
        "## Non-claims",
        "",
    ]
    lines.extend(f"- {item}" for item in result["nonClaims"])
    lines += [
        "",
        f"Manifest: `{rel_or_str(OUT_JSON)}`",
        "",
    ]
    OUT_MD.write_text("\n".join(lines), encoding="utf-8")


def check_contract_drift(readiness: dict[str, Any]) -> list[str]:
    """Make sure the upstream readiness manifest is what the contract expects."""
    problems: list[str] = []
    if not readiness:
        problems.append(f"upstream readiness manifest missing: {rel_or_str(READINESS_MANIFEST)}")
        return problems
    expected_schema = "firestaff.parity.theron_v1_runtime_screenshot_readiness.v1"
    if readiness.get("schema") != expected_schema:
        problems.append(
            f"upstream schema drifted: got {readiness.get('schema')!r}, expected {expected_schema!r}"
        )
    if not isinstance(readiness.get("cases"), list):
        problems.append("upstream manifest 'cases' field is not a list")
    return problems


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--check-only",
        action="store_true",
        help="validate without rewriting the tracked manifest/report artifacts",
    )
    args = parser.parse_args()

    readiness = load_readiness_manifest()
    drift = check_contract_drift(readiness)

    rows: list[dict[str, Any]] = []
    for case in readiness.get("cases", []) if isinstance(readiness.get("cases"), list) else []:
        classified = classify_row(case)
        rows.append(
            {
                "case_id": case.get("id", "<unknown>"),
                "label": case.get("label", "<unknown>"),
                "classification": classified["classification"],
                "eligible": classified["eligible"],
                "notes": classified["notes"],
                "source_sha256": classified.get("source_sha256"),
                "presented_sha256": classified.get("presented_sha256"),
                "probe_summary": classified.get("probe_summary"),
            }
        )

    uniqueness = audit_uniqueness(rows)
    aggregate_info = aggregate(rows, drift, uniqueness)
    status = aggregate_info["status"]
    ok = status == REQUIRED_CONTRACT_FIELDS["currentDecision"] and not drift

    result = {
        "schema": REQUIRED_CONTRACT_FIELDS["schema"],
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "ok": ok,
        "decision": aggregate_info["decision"],
        "contract": REQUIRED_CONTRACT_FIELDS,
        "bootstrapSource": REQUIRED_CONTRACT_FIELDS["bootstrapSource"],
        "bootstrapManifest": rel_or_str(READINESS_MANIFEST),
        "eligibilityCriteria": [
            "readiness.status == 'PASS'",
            "probe.sourceId == 'theron'",
            "probe.launchedEver == 1",
            "probe.schema == 'firestaff_m11_autotest_runtime_probe.v1'",
            "fallback_assets_used == false",
            "marker_found == true",
            "probe.gameTick > 0 OR probe.party.mapIndex != 0 OR probe.lastOutcome not in {'', 'THERON READY'}",
            "source BMP sha256 unique across all rows",
            "presented BMP is valid 320x200 24-bit BMP with >200 non-black pixels",
        ],
        "aggregate": aggregate_info,
        "rows": rows,
        "nonClaims": NON_CLAIMS,
    }
    if not args.check_only:
        write_outputs(result)

    summary = {
        "status": status,
        "ok": ok,
        "manifest": rel_or_str(OUT_JSON),
        "report": rel_or_str(OUT_MD),
        "eligible_count": aggregate_info["eligible_count"],
        "ineligible_count": aggregate_info["ineligible_count"],
    }
    print(json.dumps(summary, indent=2, sort_keys=True))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
