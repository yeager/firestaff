#!/usr/bin/env python3
"""Theron V1 runtime screenshot README-promotion REVIEW CHECKLIST.

This is a bounded, machine-checkable companion to
``tools/verify_theron_v1_runtime_screenshot_promotion_gate.py``. The
``promotion_gate`` decides whether a readiness row satisfies the
machine-checkable eligibility contract; this tool surfaces the
explicit per-row review checklist a human reviewer must work through
before any single row may be promoted into ``verification-screens/`` or
``docs/compare/``, and folds an optional reviewer sign-off file into
the audit so the path forward is auditable rather than implicit.

The checklist does NOT promote screenshots, copy image bytes, or
rewrite the README. It only:

1. Re-derives a per-row machine-checkable ``checklist`` of the explicit
   items a human reviewer must check (Track 02 boot marker present,
   no deterministic fallback assets, semantic Track 02 loader evidence,
   unique source BMP sha256, valid 320x200 presented BMP, etc).
2. Optionally reads an operator-local reviewer sign-off file
   (``FIRESTAFF_THERON_PROMOTION_REVIEW_STATE`` or
   ``~/.firestaff/data/theron/promotion_review_state.json``) and folds
   the per-row reviewer sign-off (``reviewed``, ``reviewer``,
   ``review_date``, ``review_notes``) into the manifest.
3. Reuses the upstream ``promotion_gate`` manifest as a contract
   source-of-truth; if that manifest's
   ``currentDecision != NO_README_PROMOTION_PERMITTED`` or its
   ``nonPromotionStatus != LOCKED_NO_ROW_README_ELIGIBLE``, this tool
   records the contract drift as a ``CONTRACT_DRIFT_FAIL`` and does
   not emit a misleading checklist verdict.
4. Locks the non-promotion state until BOTH the machine contract AND
   an explicit reviewer sign-off for at least one row are present. The
   default verdict is ``REVIEW_CHECKLIST_PENDING``: the contract is
   audited, the per-row checklist is published, the reviewer sign-off
   is honored when present, and the aggregate decision remains
   ``REVIEW_CHECKLIST_NO_ROW_PROMOTED``.

Eligibility contract (must hold for every README-eligible row):

1. **Real Firestaff runtime capture** - ``probe.sourceId`` must be
   ``theron`` and ``probe.launchedEver`` must equal ``1``.
2. **No deterministic fallback assets** - the launch must not have
   emitted the ``deterministic fallback assets`` marker. Rows that
   rely on the placeholder art are runtime/capture-path proof only.
3. **Track 02 boot milestone** - the boot marker (``TQR level load``)
   must be present in the launch output.
4. **Semantic Track 02 loader evidence** - the runtime probe must show
   that Firestaff actually decoded Track 02 content rather than
   parking at boot. The gate requires either a non-default
   ``mapIndex``, a non-zero ``gameTick``, or a non-empty
   ``lastOutcome`` beyond ``THERON READY``.
5. **Unique source BMP** - the source ``sha256`` must not be
   byte-identical to another row's source BMP.
6. **Presented BMP geometry** - the presented BMP must be a valid
   320x200 24-bit BMP with more than 200 non-black pixels, matching
   the existing readiness gate.

The reviewer sign-off file (when present) must additionally include
a per-row ``reviewed: true|false`` flag. A row is only eligible for
promotion when:

- Every machine-checkable checklist item is ``PASS`` or ``SKIP``.
- The reviewer sign-off for that row reports ``reviewed: true`` with
  a non-empty ``reviewer`` and ``review_date``.

This keeps the public docs honest: it lets future operators see
exactly which items a reviewer must check, whether the reviewer has
signed off, and why the aggregate decision still locks the
non-promotion state on the current host.
"""
from __future__ import annotations

import argparse
import json
import os
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


PASS = "theron_v1_runtime_screenshot_promotion_checklist"
ROOT = Path(__file__).resolve().parents[1]

UPSTREAM_GATE_MANIFEST = (
    ROOT
    / "parity-evidence"
    / "verification"
    / "theron_v1_runtime_screenshot_promotion_gate"
    / "manifest.json"
)
READINESS_MANIFEST = (
    ROOT
    / "parity-evidence"
    / "verification"
    / "theron_v1_runtime_screenshot_readiness"
    / "manifest.json"
)
DEFAULT_REVIEW_STATE = (
    Path.home() / ".firestaff" / "data" / "theron" / "promotion_review_state.json"
)
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence" / f"{PASS}.md"

# Locks for the upstream contract. If the upstream promotion gate drifts
# (e.g. someone flips currentDecision away from NO_README_PROMOTION_PERMITTED
# or changes the schema id), this tool records a contract-drift fail and
# refuses to emit a misleading checklist verdict.
REQUIRED_UPSTREAM_FIELDS = {
    "schema": "firestaff.parity.theron_v1_runtime_screenshot_promotion_gate.v1",
    "currentDecision": "NO_README_PROMOTION_PERMITTED",
    "nonPromotionStatus": "LOCKED_NO_ROW_README_ELIGIBLE",
}

NON_CLAIMS = [
    "No Theron screenshot is promoted into README, verification-screens/, or docs/compare/ by this checklist.",
    "No generated, illustrated, or mock Theron image is created or copied.",
    "No claim that any current Theron readiness row is reviewer-signed-off or README-eligible.",
    "No claim of full Theron runtime playability or semantic Track 02 dungeon-table parity.",
    "The checklist is a non-promotion lock, not a release gate; future promotion must satisfy every checklist item and produce a reviewer sign-off for at least one row.",
]

# Per-row reviewer checklist. Each item is a stable key + human-readable
# description + a verdict string the gate emits. Verdicts:
#   PASS   - the item is satisfied by the existing receipts.
#   FAIL   - the item is not satisfied; reason recorded.
#   SKIP   - the item does not apply (e.g. data is absent on this host).
#   PENDING_REVIEWER - the machine side is fine but a human reviewer
#                      sign-off is the next step.
CHECKLIST_ITEMS: list[dict[str, str]] = [
    {
        "key": "real_runtime_capture",
        "label": "real Firestaff Theron launch under dummy video (probe.sourceId=='theron', launchedEver==1)",
    },
    {
        "key": "no_fallback_assets",
        "label": "launch output does not contain 'deterministic fallback assets'",
    },
    {
        "key": "tqr_level_load_marker",
        "label": "launch output contains the 'TQR level load' boot marker",
    },
    {
        "key": "semantic_track02_evidence",
        "label": "runtime probe shows semantic Track 02 loader evidence (gameTick>0, party.mapIndex!=0, or lastOutcome beyond 'THERON READY')",
    },
    {
        "key": "unique_source_bmp",
        "label": "source BMP sha256 is unique across all rows (duplicate sha256s indicate a shared placeholder fixture)",
    },
    {
        "key": "presented_bmp_geometry",
        "label": "presented BMP is a valid 320x200 24-bit BMP with more than 200 non-black pixels",
    },
    {
        "key": "reviewer_signoff",
        "label": "operator-local reviewer sign-off file reports reviewed=true for this row with a non-empty reviewer and review_date",
    },
]


def rel_or_str(path: Path) -> str:
    try:
        return str(path.relative_to(ROOT))
    except ValueError:
        return str(path)


def load_json(path: Path) -> dict[str, Any]:
    if not path.exists():
        return {}
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError:
        return {}


def load_review_state(path: Path) -> dict[str, Any]:
    """Load the operator-local reviewer sign-off file.

    The reviewer state file is operator-edited and never committed; its
    absence is a successful ``REVIEWER_STATE_ABSENT`` status, not a
    failure. When present, it must parse as a JSON object with a
    ``rows`` array; unknown fields are ignored to keep the format
    forward-compatible.
    """
    if not path.exists():
        return {"present": False, "path": str(path), "rows": []}
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        return {
            "present": True,
            "path": str(path),
            "parse_error": str(exc),
            "rows": [],
        }
    if not isinstance(payload, dict):
        return {
            "present": True,
            "path": str(path),
            "parse_error": "review state must be a JSON object",
            "rows": [],
        }
    rows = payload.get("rows", [])
    if not isinstance(rows, list):
        rows = []
    return {
        "present": True,
        "path": str(path),
        "schema": payload.get("schema"),
        "rows": rows,
    }


def build_checklist_for_row(
    gate_row: dict[str, Any],
    reviewer_row: dict[str, Any] | None,
) -> list[dict[str, str]]:
    """Build the explicit per-row checklist of machine + reviewer items.

    Each item carries a ``verdict`` of ``PASS``, ``FAIL``, ``SKIP``, or
    ``PENDING_REVIEWER``, and a ``note`` that explains the verdict in
    the same audit-friendly tone the upstream gate uses.
    """
    notes = gate_row.get("notes", []) or []
    notes_blob = " | ".join(notes)
    items: list[dict[str, str]] = []

    # 1. real_runtime_capture
    probe_summary = gate_row.get("probe_summary") or {}
    real_runtime = (
        probe_summary.get("sourceId") == "theron"
        and probe_summary.get("launchedEver") == 1
    )
    items.append(
        {
            "key": "real_runtime_capture",
            "label": CHECKLIST_ITEMS[0]["label"],
            "verdict": "PASS" if real_runtime else "FAIL",
            "note": (
                f"probe.sourceId={probe_summary.get('sourceId')!r}, "
                f"launchedEver={probe_summary.get('launchedEver')!r}"
            ),
        }
    )

    # 2. no_fallback_assets
    fallback_used = "deterministic fallback assets" in notes_blob
    items.append(
        {
            "key": "no_fallback_assets",
            "label": CHECKLIST_ITEMS[1]["label"],
            "verdict": "FAIL" if fallback_used else "PASS",
            "note": (
                "row used 'deterministic fallback assets'; placeholder art cannot be promoted as a real Theron Track 02 README screenshot"
                if fallback_used
                else "no 'deterministic fallback assets' marker in row notes"
            ),
        }
    )

    # 3. tqr_level_load_marker
    marker_missing = any("boot marker not present" in n for n in notes)
    items.append(
        {
            "key": "tqr_level_load_marker",
            "label": CHECKLIST_ITEMS[2]["label"],
            "verdict": "FAIL" if marker_missing else "PASS",
            "note": (
                "'TQR level load' boot marker not present in launch output"
                if marker_missing
                else "'TQR level load' boot marker present in row notes"
            ),
        }
    )

    # 4. semantic_track02_evidence
    semantic_missing = any("no semantic Track 02 loader evidence" in n for n in notes)
    items.append(
        {
            "key": "semantic_track02_evidence",
            "label": CHECKLIST_ITEMS[3]["label"],
            "verdict": "FAIL" if semantic_missing else "PASS",
            "note": (
                f"gameTick={probe_summary.get('gameTick')!r}, "
                f"party.mapIndex={probe_summary.get('party.mapIndex')!r}, "
                f"lastOutcome={probe_summary.get('lastOutcome')!r}"
            ),
        }
    )

    # 5. unique_source_bmp
    duplicate_sha = any("shared with rows" in n for n in notes)
    source_sha = gate_row.get("source_sha256")
    items.append(
        {
            "key": "unique_source_bmp",
            "label": CHECKLIST_ITEMS[4]["label"],
            "verdict": "FAIL" if duplicate_sha else "PASS",
            "note": (
                f"source sha256 {source_sha} duplicated across rows"
                if duplicate_sha
                else f"source sha256 {source_sha or '<none>'} unique"
            ),
        }
    )

    # 6. presented_bmp_geometry
    geometry_missing = any("presented BMP failed geometry/non-black checks" in n for n in notes)
    items.append(
        {
            "key": "presented_bmp_geometry",
            "label": CHECKLIST_ITEMS[5]["label"],
            "verdict": "FAIL" if geometry_missing else "PASS",
            "note": (
                "presented BMP failed geometry/non-black checks (must be 320x200, >200 non-black pixels)"
                if geometry_missing
                else "presented BMP geometry/non-black checks pass"
            ),
        }
    )

    # 7. reviewer_signoff
    if reviewer_row is None:
        items.append(
            {
                "key": "reviewer_signoff",
                "label": CHECKLIST_ITEMS[6]["label"],
                "verdict": "PENDING_REVIEWER",
                "note": "no operator-local reviewer sign-off present for this row",
            }
        )
    else:
        reviewed = bool(reviewer_row.get("reviewed"))
        reviewer = str(reviewer_row.get("reviewer") or "").strip()
        review_date = str(reviewer_row.get("review_date") or "").strip()
        if reviewed and reviewer and review_date:
            items.append(
                {
                    "key": "reviewer_signoff",
                    "label": CHECKLIST_ITEMS[6]["label"],
                    "verdict": "PASS",
                    "note": f"reviewer sign-off recorded: reviewer={reviewer!r}, review_date={review_date!r}",
                }
            )
        else:
            missing = []
            if not reviewed:
                missing.append("reviewed")
            if not reviewer:
                missing.append("reviewer")
            if not review_date:
                missing.append("review_date")
            items.append(
                {
                    "key": "reviewer_signoff",
                    "label": CHECKLIST_ITEMS[6]["label"],
                    "verdict": "PENDING_REVIEWER",
                    "note": f"reviewer sign-off incomplete; missing fields: {', '.join(missing) or '<unknown>'}",
                }
            )

    return items


def row_machine_eligible(checklist: list[dict[str, str]]) -> bool:
    """A row is machine-eligible iff every non-reviewer item is PASS."""
    for item in checklist:
        if item["key"] == "reviewer_signoff":
            continue
        if item["verdict"] != "PASS":
            return False
    return True


def row_reviewer_signed_off(checklist: list[dict[str, str]]) -> bool:
    for item in checklist:
        if item["key"] == "reviewer_signoff":
            return item["verdict"] == "PASS"
    return False


def row_classification(checklist: list[dict[str, str]]) -> str:
    if row_machine_eligible(checklist) and row_reviewer_signed_off(checklist):
        return "REVIEWER_PROMOTED_README_ELIGIBLE"
    if row_machine_eligible(checklist):
        return "MACHINE_ELIGIBLE_AWAITING_REVIEWER"
    return "REVIEW_CHECKLIST_INELIGIBLE"


def audit_uniqueness(rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    """Detect identical source BMP sha256 across rows.

    Mirrors the upstream promotion-gate uniqueness audit so this tool
    surfaces the same shared-placeholder-fixture smell. The audit runs
    across every row (not just reviewer-signed ones) because a
    duplicate sha256 is an independent reason to refuse promotion.
    """
    grouped: dict[str, list[str]] = {}
    for row in rows:
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
    return findings


def check_upstream_drift(upstream: dict[str, Any]) -> list[str]:
    problems: list[str] = []
    if not upstream:
        problems.append(
            f"upstream promotion-gate manifest missing: {rel_or_str(UPSTREAM_GATE_MANIFEST)}"
        )
        return problems
    contract = upstream.get("contract") or {}
    # The upstream promotion gate mirrors REQUIRED_UPSTREAM_FIELDS into
    # both the contract block and the top-level decision field, so we
    # check the contract block first and only fall back to the top-level
    # field for currentDecision when the contract block omits it. This
    # keeps the drift check honest against the 2026-06-25 manifest shape
    # while still pinning the contract id and non-promotion status.
    for key, want in REQUIRED_UPSTREAM_FIELDS.items():
        got = contract.get(key)
        if got is None and key == "currentDecision":
            got = upstream.get("decision")
        if got != want:
            problems.append(
                f"upstream contract field {key!r} drifted: got {got!r}, expected {want!r}"
            )
    if not isinstance(upstream.get("rows"), list):
        problems.append("upstream manifest 'rows' field is not a list")
    return problems


def aggregate(
    rows: list[dict[str, Any]],
    drift: list[str],
    uniqueness: list[dict[str, Any]],
    reviewer_state: dict[str, Any],
) -> dict[str, Any]:
    machine_eligible = [
        r for r in rows if r["classification"] == "MACHINE_ELIGIBLE_AWAITING_REVIEWER"
    ]
    reviewer_promoted = [
        r for r in rows
        if r["classification"] == "REVIEWER_PROMOTED_README_ELIGIBLE"
    ]
    ineligible = [
        r for r in rows if r["classification"] == "REVIEW_CHECKLIST_INELIGIBLE"
    ]
    skipped = [r for r in rows if r["classification"] == "SKIPPED_NO_DATA"]

    if drift:
        decision = "CONTRACT_DRIFT_FAIL"
    elif reviewer_promoted:
        # Even with a reviewer sign-off, the upstream gate remains the
        # source-of-truth for whether the README may be touched. If the
        # gate is green AND a reviewer has signed off, the contract can
        # move to REVIEWER_PROMOTED_README_ELIGIBLE; the actual byte
        # promotion is still the operator's responsibility and must
        # touch ``verification-screens/``/``docs/compare/`` directly.
        decision = "REVIEWER_PROMOTED_README_ELIGIBLE"
    elif machine_eligible:
        decision = "REVIEW_CHECKLIST_MACHINE_ELIGIBLE_AWAITING_REVIEWER"
    else:
        decision = "REVIEW_CHECKLIST_NO_ROW_PROMOTED"

    return {
        "status": decision,
        "decision": decision,
        "machine_eligible_count": len(machine_eligible),
        "reviewer_promoted_count": len(reviewer_promoted),
        "ineligible_count": len(ineligible),
        "skipped_count": len(skipped),
        "uniqueness_findings": uniqueness,
        "contract_drift": drift,
        "reviewer_state_present": bool(reviewer_state.get("present")),
        "reviewer_state_path": reviewer_state.get("path"),
    }


def write_outputs(result: dict[str, Any]) -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Theron V1 runtime screenshot promotion checklist",
        "",
        f"Status: `{result['status']}`",
        "",
        f"Decision: **{result['decision']}**",
        "",
        "This is a bounded, machine-checkable companion to the runtime",
        "screenshot promotion gate. It surfaces the explicit per-row",
        "review checklist a human reviewer must work through before any",
        "Theron capture may be promoted into public docs, and folds an",
        "optional operator-local reviewer sign-off file into the audit",
        "so the path forward is auditable rather than implicit.",
        "",
        "## Reviewer Checklist",
        "",
        "For every readiness row, the reviewer must confirm each of the",
        "following items is `PASS`:",
        "",
    ]
    for item in CHECKLIST_ITEMS:
        lines.append(f"- **{item['key']}** - {item['label']}")

    lines += [
        "",
        "## Per-row checklist",
        "",
        "Each row below carries the machine-checkable verdict for every",
        "checklist item, plus the optional reviewer sign-off verdict.",
        "A row is `REVIEWER_PROMOTED_README_ELIGIBLE` only when every",
        "machine item is `PASS` AND the operator-local reviewer sign-off",
        "is `PASS` for that row. The aggregate decision remains",
        "`REVIEW_CHECKLIST_NO_ROW_PROMOTED` until both halves of the",
        "contract are satisfied for at least one row.",
        "",
        "| Case | Classification | Machine | Reviewer | Items |",
        "|---|---|---|---|---|",
    ]
    for row in result["rows"]:
        item_verdicts = ", ".join(
            f"{i['key']}={i['verdict']}" for i in row["checklist"]
        )
        machine = "✓" if row["machine_eligible"] else "—"
        reviewer = "✓" if row["reviewer_signed_off"] else "—"
        lines.append(
            f"| {row['case_id']} | `{row['classification']}` | {machine} | {reviewer} | {item_verdicts} |"
        )

    lines += [
        "",
        "## Aggregate",
        "",
        f"- Decision: **{result['aggregate']['decision']}**",
        f"- Machine-eligible rows (awaiting reviewer): **{result['aggregate']['machine_eligible_count']}**",
        f"- Reviewer-promoted rows: **{result['aggregate']['reviewer_promoted_count']}**",
        f"- Ineligible rows: **{result['aggregate']['ineligible_count']}**",
        f"- Skipped rows (no data on this host): **{result['aggregate']['skipped_count']}**",
        f"- Unique-source-sha256 findings: **{len(result['aggregate']['uniqueness_findings'])}**",
        f"- Contract-drift findings: **{len(result['aggregate']['contract_drift'])}**",
        f"- Reviewer sign-off file present: **{result['aggregate']['reviewer_state_present']}**",
        "",
    ]
    if result["aggregate"]["reviewer_state_path"]:
        lines.append(
            f"- Reviewer sign-off path: `{result['aggregate']['reviewer_state_path']}`"
        )
    else:
        lines.append(
            "- Reviewer sign-off path: `<not configured>` (default: `~/.firestaff/data/theron/promotion_review_state.json`, override via `FIRESTAFF_THERON_PROMOTION_REVIEW_STATE`)"
        )
    if result.get("reviewerStateParseError"):
        lines.append(
            f"- Reviewer sign-off parse error: `{result['reviewerStateParseError']}` "
            "(treated as PENDING_REVIEWER for every row)"
        )

    lines += [
        "",
        "## Public Screenshot Boundary",
        "",
        "- This checklist is the second guardrail for whether a Theron readiness row is reviewer-promotable.",
        "- The promotion gate (`tools/verify_theron_v1_runtime_screenshot_promotion_gate.py`) is still the source-of-truth for the machine contract; the checklist only adds the explicit reviewer workflow on top.",
        "- README, `verification-screens/`, and `docs/compare/` must not add Theron screenshots until at least one row is `REVIEWER_PROMOTED_README_ELIGIBLE` AND the upstream promotion gate is green AND a human reviewer has signed off.",
        "- No generated, illustrated, mocked, or synthetic Theron image may be used as a README screenshot.",
        "",
        "## Reviewer sign-off file shape",
        "",
        "The operator-local reviewer sign-off file is a JSON object:",
        "",
        "```json",
        "{",
        "  \"schema\": \"firestaff.theron_v1_promotion_review_state.v1\",",
        "  \"rows\": [",
        "    {",
        "      \"case_id\": \"canonical_pcengine_root\",",
        "      \"reviewed\": true,",
        "      \"reviewer\": \"<name>\",",
        "      \"review_date\": \"YYYY-MM-DD\",",
        "      \"review_notes\": \"<optional free-form notes>\"",
        "    }",
        "  ]",
        "}",
        "```",
        "",
        "Default path: `~/.firestaff/data/theron/promotion_review_state.json`.",
        "Override via `FIRESTAFF_THERON_PROMOTION_REVIEW_STATE`. The file",
        "is operator-local and must never be committed.",
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


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--check-only",
        action="store_true",
        help="validate without rewriting the tracked manifest/report artifacts",
    )
    parser.add_argument(
        "--review-state",
        type=Path,
        default=None,
        help="path to the operator-local reviewer sign-off file "
        "(default: $FIRESTAFF_THERON_PROMOTION_REVIEW_STATE or "
        "~/.firestaff/data/theron/promotion_review_state.json)",
    )
    args = parser.parse_args()

    env_path = os.environ.get("FIRESTAFF_THERON_PROMOTION_REVIEW_STATE")
    if args.review_state is not None:
        review_state_path = args.review_state
    elif env_path:
        review_state_path = Path(env_path)
    else:
        review_state_path = DEFAULT_REVIEW_STATE
    review_state_path = review_state_path.expanduser().resolve()

    upstream = load_json(UPSTREAM_GATE_MANIFEST)
    readiness = load_json(READINESS_MANIFEST)
    reviewer_state = load_review_state(review_state_path)
    drift = check_upstream_drift(upstream)

    reviewer_by_case: dict[str, dict[str, Any]] = {}
    for row in reviewer_state.get("rows", []):
        if not isinstance(row, dict):
            continue
        case_id = str(row.get("case_id") or "").strip()
        if case_id:
            reviewer_by_case[case_id] = row

    rows: list[dict[str, Any]] = []
    for gate_row in upstream.get("rows", []) if isinstance(upstream.get("rows"), list) else []:
        if not isinstance(gate_row, dict):
            continue
        case_id = str(gate_row.get("case_id") or "<unknown>")
        if gate_row.get("classification") == "SKIPPED_NO_DATA":
            rows.append(
                {
                    "case_id": case_id,
                    "label": gate_row.get("label", "<unknown>"),
                    "classification": "SKIPPED_NO_DATA",
                    "machine_eligible": False,
                    "reviewer_signed_off": False,
                    "checklist": [],
                    "source_sha256": gate_row.get("source_sha256"),
                    "presented_sha256": gate_row.get("presented_sha256"),
                    "probe_summary": gate_row.get("probe_summary"),
                    "notes": ["data directory missing; row skipped, not promoted"],
                }
            )
            continue
        checklist = build_checklist_for_row(
            gate_row,
            reviewer_by_case.get(case_id),
        )
        machine_ok = row_machine_eligible(checklist)
        reviewer_ok = row_reviewer_signed_off(checklist)
        rows.append(
            {
                "case_id": case_id,
                "label": gate_row.get("label", "<unknown>"),
                "classification": row_classification(checklist),
                "machine_eligible": machine_ok,
                "reviewer_signed_off": reviewer_ok,
                "checklist": checklist,
                "source_sha256": gate_row.get("source_sha256"),
                "presented_sha256": gate_row.get("presented_sha256"),
                "probe_summary": gate_row.get("probe_summary"),
                "reviewer_signoff_record": reviewer_by_case.get(case_id),
            }
        )

    uniqueness = audit_uniqueness(rows)
    aggregate_info = aggregate(rows, drift, uniqueness, reviewer_state)
    status = aggregate_info["status"]

    # Honest lock: the current non-promotion status is the default, and
    # only a contract-drift or reviewer-promoted aggregate state is a
    # reason to flip away from REVIEW_CHECKLIST_NO_ROW_PROMOTED.
    ok = not drift

    result = {
        "schema": "firestaff.parity.theron_v1_runtime_screenshot_promotion_checklist.v1",
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "ok": ok,
        "decision": aggregate_info["decision"],
        "upstreamGateManifest": rel_or_str(UPSTREAM_GATE_MANIFEST),
        "upstreamGate": {
            "schema": upstream.get("schema"),
            "currentDecision": upstream.get("decision"),
            "nonPromotionStatus": (
                (upstream.get("contract") or {}).get("nonPromotionStatus")
            ),
        },
        "reviewerStatePath": str(review_state_path),
        "reviewerStatePresent": bool(reviewer_state.get("present")),
        "reviewerStateSchema": reviewer_state.get("schema"),
        "reviewerStateParseError": reviewer_state.get("parse_error"),
        "checklistItems": CHECKLIST_ITEMS,
        "eligibilityCriteria": [
            "real Firestaff Theron launch (probe.sourceId=='theron', launchedEver==1)",
            "launch output does not contain 'deterministic fallback assets'",
            "launch output contains the 'TQR level load' boot marker",
            "runtime probe shows semantic Track 02 loader evidence (gameTick>0, party.mapIndex!=0, or lastOutcome beyond 'THERON READY')",
            "source BMP sha256 is unique across all rows",
            "presented BMP is a valid 320x200 24-bit BMP with >200 non-black pixels",
            "operator-local reviewer sign-off reports reviewed=true with non-empty reviewer and review_date",
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
        "machine_eligible_count": aggregate_info["machine_eligible_count"],
        "reviewer_promoted_count": aggregate_info["reviewer_promoted_count"],
        "ineligible_count": aggregate_info["ineligible_count"],
        "skipped_count": aggregate_info["skipped_count"],
        "reviewer_state_present": aggregate_info["reviewer_state_present"],
    }
    print(json.dumps(summary, indent=2, sort_keys=True))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
