#!/usr/bin/env python3
"""Smoke-check the symbol backlog queue used by Firestaff subagents."""
from __future__ import annotations

import contextlib
import csv
import importlib.util
import io
import json
from collections import Counter
import subprocess
import sys
from pathlib import Path
from unittest import mock

ROOT = Path(__file__).resolve().parents[1]
BACKLOG = ROOT / "tools" / "symbol_backlog.py"


def run(args: list[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        args,
        cwd=ROOT,
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )


def require_jsonl(args: list[str], expected_reference: str) -> None:
    result = run([sys.executable, str(BACKLOG), *args, "--format", "jsonl", "--limit", "5"])
    rows = [json.loads(line) for line in result.stdout.splitlines() if line.strip()]
    if not rows:
        raise AssertionError(f"empty JSONL queue for {' '.join(args)}")
    for row in rows:
        if row.get("reference") != expected_reference:
            raise AssertionError(f"unexpected reference in queue: {row}")
        if not row.get("symbol") or not row.get("status"):
            raise AssertionError(f"incomplete backlog row: {row}")


def verify_named_source_reviews() -> None:
    result = run([
        sys.executable, str(BACKLOG), "--reference", "skproject", "--game", "DM2",
        "--format", "json", "--limit", "10000",
    ])
    items = json.loads(result.stdout)["items"]
    matching = [item for item in items
                if item["source"] == "SKULLWIN/c_move.cpp:197"]
    if not matching or any(not item.get("review_notes") for item in matching):
        raise AssertionError("named SKULLWIN rows did not receive their exact active-source reviews")


def verify_missing_audit_fallback() -> None:
    spec = importlib.util.spec_from_file_location("symbol_backlog", BACKLOG)
    if spec is None or spec.loader is None:
        raise AssertionError("could not load symbol_backlog for missing-audit regression")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    absent = ROOT / ".codex-scratch" / "missing-audit-regression.tsv"
    module.SKPROJECT_AUDIT = absent
    module.SKPROJECT_ACTIVE_SOURCE_INVENTORY = absent
    stdout = io.StringIO()
    with mock.patch.object(sys, "argv", [
        str(BACKLOG), "--allow-missing-audits", "--reference", "ReDMCSB",
        "--limit", "1", "--json",
    ]), contextlib.redirect_stdout(stdout):
        result = module.main()
    payload = json.loads(stdout.getvalue())
    if result != 0 or not payload["items"] or not payload["warnings"]:
        raise AssertionError("--allow-missing-audits did not emit the available backlog and warnings")


def verify_documented_dm2_counts() -> None:
    audit_dir = ROOT / "docs/reference/audits"
    inventory_path = audit_dir / "SKPROJECT_DM2_ACTIVE_SOURCE_INVENTORY.tsv"
    review_path = audit_dir / "SKPROJECT_DM2_ACTIVE_SOURCE_REVIEW.tsv"
    with inventory_path.open(newline="", encoding="utf-8") as handle:
        inventory = list(csv.DictReader(handle, delimiter="\t"))
    with review_path.open(newline="", encoding="utf-8") as handle:
        reviews = list(csv.DictReader(handle, delimiter="\t"))
    review_by_key = {
        (row["source_tree"], row["source_file"], row["line"], row["symbol"]): row
        for row in reviews
    }
    active_counts: Counter[str] = Counter()
    for row in inventory:
        key = (row["source_tree"], row["source_file"], row["line"], row["symbol"])
        status = review_by_key.get(key, {}).get(
            "review_status", row.get("review_status", "UNREVIEWED"))
        active_counts[status] += 1
    parity_doc = (ROOT / "docs/parity/REDMCSB_FIRESTAFF_COMPLETE_AUDIT.md").read_text(
        encoding="utf-8")
    named_doc = (audit_dir / "SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.md").read_text(
        encoding="utf-8")
    active_summary = (
        f"Applying the active-source review overrides gives "
        f"{active_counts['UNREVIEWED']:,} unreviewed, "
        f"{active_counts['PARTIAL']:,} partial, and "
        f"{active_counts['IMPLEMENTED_PARITY']:,} implemented-parity definitions."
    )
    if active_summary not in parity_doc:
        raise AssertionError("DM2 parity audit counts do not match the active-source ledgers")

    backlog_result = run([
        sys.executable, str(BACKLOG), "--reference", "skproject", "--game", "DM2",
        "--format", "json", "--limit", "0",
    ])
    backlog = json.loads(backlog_result.stdout)["summary"]
    backlog_summary = (
        f"The generated combined skproject backlog is {backlog['totalOpen']:,} open rows "
        f"({backlog['byStatus']['UNREVIEWED']:,} unreviewed, "
        f"{backlog['byStatus']['PARTIAL']:,} partial source or older-ledger rows, "
        f"{backlog['byStatus']['PARTIAL_SOURCE_OWNER']:,} partial source owners)."
    )
    if backlog_summary not in parity_doc:
        raise AssertionError("DM2 combined backlog counts do not match the generated queue")

    named_summary = (
        f"With active-source review overrides applied, the inventory currently maps "
        f"{active_counts['UNREVIEWED']:,} as `UNREVIEWED`, "
        f"{active_counts['PARTIAL']:,} as `PARTIAL`, and "
        f"{active_counts['IMPLEMENTED_PARITY']:,} as `IMPLEMENTED_PARITY`."
    )
    if named_summary not in named_doc:
        raise AssertionError("DM2 named-symbol audit counts do not match the active-source ledgers")


def main() -> int:
    run([sys.executable, "-m", "py_compile", str(BACKLOG)])
    summary = run([sys.executable, str(BACKLOG), "--limit", "0", "--json"])
    payload = json.loads(summary.stdout)
    if payload["summary"].get("totalOpen", 0) <= 0:
        raise AssertionError("symbol backlog unexpectedly empty")
    require_jsonl(["--reference", "ReDMCSB"], "ReDMCSB")
    require_jsonl(["--reference", "skproject", "--game", "DM2"], "skproject")
    verify_named_source_reviews()
    verify_missing_audit_fallback()
    verify_documented_dm2_counts()
    print("symbol backlog verifier: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
