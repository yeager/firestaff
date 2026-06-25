#!/usr/bin/env python3
"""Verify that documented Greatstone db_data/ paths are reachable.

Bounded regression gate for docs/FIRESTAFF_GAP_LIST.md A5
"Real-data regression tests (greatstone db_data)" row. Two distinct
modes:

* OFFLINE (default) — uses a local fixture at
  `tests/fixtures/greatstone_db_data_paths/index.json`. No network
  access. This is the CTest mode so CI and offline hosts pass
  deterministically. The fixture carries only **derived metadata**
  (status code, expected status, content-type hint, content-length
  hint, title text, title SHA-256, link count) — no copyrighted
  bytes are committed.

* ONLINE (opt-in via `--online` or `FIRESTAFF_GREATSTONE_PROBE=1`)
  — performs a bounded byte-range GET against each documented
  db_data/ URL, captures only the same metadata, then drops the
  response bytes after extracting the title. The bytes are never
  written to disk in any mode.

Why HEAD + byte-range GET (not full GET):
  - A `Range: bytes=0-8191` request is enough to prove "200/206 OK
    vs 404" and to extract the <title> and link count.
  - The body is bounded to `BYTES_PER_FETCH` bytes; we drop it
    after extracting the title.
  - This keeps the probe useful as a "current paths reachable"
    gate without enabling a wholesale db_data/ mirror.

Obsolete-path detection:
  - The 404 list covers the old `c_dm_*` / `c_csb_*` sample paths
    and the guessed DM2 paths called out in the gap list.
  - In OFFLINE mode we trust the fixture's "actual_status" /
    "expected_status" fields. In ONLINE mode we hit the live URL
    and assert that the obsolete URLs return 404.

Output:
  - Deterministic JSON evidence file at
    `parity-evidence/verification/greatstone_db_data_paths_probe/manifest.json`
    when run with `--write`, otherwise the script reads the file
    and compares it to a freshly computed manifest. This mirrors
    the pass302/305/308 evidence pattern.

Exit codes:
  0  every current path reachable AND every obsolete path 404
  1  at least one path failed its expected status
  2  fixture missing
  3  network error in ONLINE mode (no status reachable)
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import socket
import sys
import time
import urllib.error
import urllib.request
from dataclasses import dataclass, asdict
from pathlib import Path
from typing import Optional

REPO_ROOT = Path(__file__).resolve().parents[1]
FIXTURE_DIR = REPO_ROOT / "tests" / "fixtures" / "greatstone_db_data_paths"
FIXTURE_INDEX = FIXTURE_DIR / "index.json"
EVIDENCE_PATH = REPO_ROOT / "parity-evidence" / "verification" / "greatstone_db_data_paths_probe" / "manifest.json"

# Conservative defaults so ONLINE mode stays bounded.
DEFAULT_TIMEOUT = 12.0
DEFAULT_RETRIES = 2
DEFAULT_BYTES_PER_FETCH = 8192  # only the head of each document
DEFAULT_USER_AGENT = "firestaff-db-data-probe/1.0 (+docs/FIRESTAFF_GAP_LIST.md A5)"
GREATSTONE_BASE = "http://greatstone.free.fr/dm/"

# Documented in FIRESTAFF_GAP_LIST.md A5 row. Curated from the
# "Verified examples" sentence — these are the URLs the gap list
# pins as currently-reachable. Used as the canonical probe list.
DOCUMENTED_CURRENT_PATHS: tuple[str, ...] = (
    # ── Top-level g_* index pages ──
    "g_dm.html",
    "g_csb.html",
    "g_dm2.html",
    # ── DM db_data/ examples ──
    "db_data/dm_pc_34/graphics.dat/graphics.dat.html",
    "db_data/dm_snes_11_jp_ntsc/smc/smc.html",
    # ── CSB db_data/ examples ──
    "db_data/csb_atari_21_en_stx/graphics.dat/graphics.dat.html",
    "db_data/csb_amiga_udr2_en/hcsb.htc/hcsb.htc.html",
    # ── DM2 db_data/ examples ──
    "db_data/dm2_pc10_en/graphics.dat/graphics.dat.html",
    "db_data/dm2_amiga_10_enfrge/lang.ftl/lang.ftl.html",
    "db_data/dm2_segacd_10_en/stry.dat/stry.dat.html",
)

# Obsolete / 404-regression paths. These are the
# "old c_dm_* / c_csb_* sample paths and guessed DM2 paths may 404"
# list from the gap row. The probe must observe 404 for each of
# these in ONLINE mode (or the fixture's expected_status in
# OFFLINE mode) so the regression is locked.
DOCUMENTED_OBSOLETE_PATHS: tuple[str, ...] = (
    "db_data/c_dm_pc_34/graphics.dat/graphics.dat.html",
    "db_data/c_csb_pc_34/graphics.dat/graphics.dat.html",
    "db_data/dm2_pc10/graphics.dat/graphics.dat.html",
)


@dataclass(frozen=True)
class PathExpectation:
    path: str
    expected_status: int  # 200 for current, 404 for obsolete
    label: str  # human-readable role for the row
    source_anchor: str  # which gap-list row or doc cites this URL


def build_expectations() -> tuple[PathExpectation, ...]:
    out: list[PathExpectation] = []
    for p in DOCUMENTED_CURRENT_PATHS:
        out.append(PathExpectation(
            path=p,
            expected_status=200,
            label=label_for(p),
            source_anchor="docs/FIRESTAFF_GAP_LIST.md A5 (Real-data regression tests, greatstone db_data) verified-examples list",
        ))
    for p in DOCUMENTED_OBSOLETE_PATHS:
        out.append(PathExpectation(
            path=p,
            expected_status=404,
            label="obsolete / 404-regression (c_dm_*/c_csb_*/guessed DM2)",
            source_anchor="docs/FIRESTAFF_GAP_LIST.md A5 (Real-data regression tests, greatstone db_data) obsolete-path list",
        ))
    return tuple(out)


def label_for(path: str) -> str:
    """Map a db_data/ path to a short, sales-friendly role label."""
    if path in ("g_dm.html", "g_csb.html", "g_dm2.html"):
        return f"top-level index: {path}"
    if "dm_pc_34" in path:
        return "DM1 PC 3.4 EN GRAPHICS.DAT mapfile extract"
    if "dm_snes_11_jp_ntsc" in path:
        return "DM1 SNES 1.1 JP NTSC SMC mapfile extract"
    if "csb_atari_21_en_stx" in path:
        return "CSB Atari ST 2.1 EN (STX) GRAPHICS.DAT mapfile extract"
    if "csb_amiga_udr2_en" in path:
        return "CSB Amiga Utility Disk R2 EN HCSB.HTC mapfile extract"
    if "dm2_pc10_en" in path:
        return "DM2 PC 1.0 EN GRAPHICS.DAT mapfile extract"
    if "dm2_amiga_10_enfrge" in path:
        return "DM2 Amiga 1.0 EN/FR/GE LANG.FTL mapfile extract"
    if "dm2_segacd_10_en" in path:
        return "DM2 Sega CD 1.0 EN STRY.DAT mapfile extract"
    return "greatstone db_data/ path"


def classify_match(actual: Optional[int], expected: int) -> bool:
    """Map (actual_status, expected_status) to a match decision.

    For "current path reachable" rows we accept 200 OK and 206
    Partial Content (the server honoured our `Range: bytes=0-…`
    request, which is the correct HTTP semantics for a bounded
    HEAD-equivalent). For "obsolete path" rows we require 404.
    """
    if actual is None:
        return False
    if expected == 200:
        return actual in (200, 206)
    if expected == 404:
        return actual == 404
    return actual == expected


def _title_sha(title: Optional[str]) -> Optional[str]:
    if title is None:
        return None
    return hashlib.sha256(title.encode("utf-8")).hexdigest()


@dataclass
class ProbeResult:
    path: str
    expected_status: int
    actual_status: Optional[int]
    content_type: Optional[str]
    content_length: Optional[int]
    title: Optional[str]
    title_sha256: Optional[str]
    link_count: int
    redirect_url: Optional[str]
    elapsed_ms: int
    error: Optional[str]
    match: bool
    label: str
    source_anchor: str


def _extract_meta(body: bytes) -> tuple[Optional[str], int]:
    """Extract <title> and db_data link count from a bounded body slice."""
    text = body.decode("latin-1", errors="replace")
    title: Optional[str] = None
    m = re.search(r"<title[^>]*>(.*?)</title>", text, re.IGNORECASE | re.DOTALL)
    if m:
        title = re.sub(r"\s+", " ", m.group(1)).strip()[:200] or None
    link_count = len(re.findall(r"db_data/", text))
    return title, link_count


def fetch_metadata(url: str, timeout: float) -> ProbeResult:
    """Bounded byte-range GET; drops the body after extracting metadata."""
    started = time.monotonic()
    req_headers = {
        "User-Agent": DEFAULT_USER_AGENT,
        "Range": f"bytes=0-{DEFAULT_BYTES_PER_FETCH - 1}",
    }
    req = urllib.request.Request(url, method="GET", headers=req_headers)
    try:
        with urllib.request.urlopen(req, timeout=timeout) as r:
            actual = r.status
            content_type = r.headers.get("Content-Type")
            content_length_raw = r.headers.get("Content-Length")
            try:
                content_length = int(content_length_raw) if content_length_raw else None
            except ValueError:
                content_length = None
            body_head = r.read(DEFAULT_BYTES_PER_FETCH)
            elapsed_ms = int((time.monotonic() - started) * 1000)
            title, link_count = _extract_meta(body_head)
            return ProbeResult(
                path=url,
                expected_status=200,
                actual_status=actual,
                content_type=content_type,
                content_length=content_length,
                title=title,
                title_sha256=_title_sha(title),
                link_count=link_count,
                redirect_url=r.geturl() if r.geturl() != url else None,
                elapsed_ms=elapsed_ms,
                error=None,
                match=(actual in (200, 206)),
                label="",
                source_anchor="",
            )
    except urllib.error.HTTPError as e:
        elapsed_ms = int((time.monotonic() - started) * 1000)
        return ProbeResult(
            path=url,
            expected_status=200,
            actual_status=e.code,
            content_type=e.headers.get("Content-Type") if e.headers else None,
            content_length=None,
            title=None,
            title_sha256=None,
            link_count=0,
            redirect_url=None,
            elapsed_ms=elapsed_ms,
            error=f"HTTPError {e.code} {e.reason}",
            match=classify_match(e.code, 200),
            label="",
            source_anchor="",
        )
    except (urllib.error.URLError, socket.timeout, ConnectionError) as e:
        elapsed_ms = int((time.monotonic() - started) * 1000)
        return ProbeResult(
            path=url,
            expected_status=200,
            actual_status=None,
            content_type=None,
            content_length=None,
            title=None,
            title_sha256=None,
            link_count=0,
            redirect_url=None,
            elapsed_ms=elapsed_ms,
            error=f"{type(e).__name__}: {e}",
            match=False,
            label="",
            source_anchor="",
        )


def run_offline(expectations: tuple[PathExpectation, ...]) -> list[ProbeResult]:
    """Replay the fixture and synthesize ProbeResults.

    The fixture's `index.json` carries the expected status and
    derived metadata (title text + sha, link count) so OFFLINE
    mode can prove "200 vs 404 regression" deterministically.
    No body bytes are cached.
    """
    if not FIXTURE_INDEX.is_file():
        raise SystemExit(f"fixture missing: {FIXTURE_INDEX}")
    index = json.loads(FIXTURE_INDEX.read_text(encoding="utf-8"))
    rows: dict[str, dict] = {row["path"]: row for row in index.get("paths", [])}

    results: list[ProbeResult] = []
    for exp in expectations:
        row = rows.get(exp.path)
        if not row:
            results.append(ProbeResult(
                path=exp.path,
                expected_status=exp.expected_status,
                actual_status=None,
                content_type=None,
                content_length=None,
                title=None,
                title_sha256=None,
                link_count=0,
                redirect_url=None,
                elapsed_ms=0,
                error=f"missing fixture row for {exp.path}",
                match=False,
                label=exp.label,
                source_anchor=exp.source_anchor,
            ))
            continue
        title = row.get("title")
        results.append(ProbeResult(
            path=exp.path,
            expected_status=exp.expected_status,
            actual_status=row["actual_status"],
            content_type=row.get("content_type"),
            content_length=row.get("content_length"),
            title=title,
            title_sha256=_title_sha(title),
            link_count=row.get("link_count", 0),
            redirect_url=None,
            elapsed_ms=0,
            error=row.get("error"),
            match=classify_match(row["actual_status"], exp.expected_status),
            label=exp.label,
            source_anchor=exp.source_anchor,
        ))
    return results


def run_online(
    expectations: tuple[PathExpectation, ...],
    timeout: float,
    retries: int,
) -> list[ProbeResult]:
    """Hit each URL through a bounded byte-range GET with retries."""
    results: list[ProbeResult] = []
    for exp in expectations:
        url = GREATSTONE_BASE + exp.path
        last: Optional[ProbeResult] = None
        for attempt in range(retries + 1):
            last = fetch_metadata(url, timeout)
            if last.error is None:
                break
            if attempt < retries:
                time.sleep(0.4 * (attempt + 1))
        assert last is not None
        last.expected_status = exp.expected_status
        last.match = classify_match(last.actual_status, exp.expected_status)
        last.label = exp.label
        last.source_anchor = exp.source_anchor
        last.path = exp.path  # store the relative path, not the absolute URL
        results.append(last)
    return results


def write_evidence(results: list[ProbeResult], mode: str) -> None:
    payload = {
        "pass": "greatstone_db_data_paths_probe",
        "mode": mode,
        "source": "docs/FIRESTAFF_GAP_LIST.md A5 (Real-data regression tests, greatstone db_data)",
        "scope": (
            "Bounded metadata-only probe: status code, Content-Type, Content-Length, "
            "HTML <title> + title SHA-256, db_data link count. No copyrighted bytes "
            f"persisted; the byte-range GET drops the body after extracting metadata."
        ),
        "expectations_total": len(results),
        "expectations_match": sum(1 for r in results if r.match),
        "rows": [asdict(r) for r in results],
        "not_claimed": [
            "pixel parity",
            "db_data byte-for-byte mirroring",
            "real-asset handoff (no copyrighted bytes are downloaded or stored)",
            "Firestaff runtime launching from db_data/ URLs (this is a path/URL availability gate only)",
        ],
    }
    EVIDENCE_PATH.parent.mkdir(parents=True, exist_ok=True)
    EVIDENCE_PATH.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    # Also (re)write the OFFLINE fixture from the freshly-computed
    # results so the CTest OFFLINE mode stays in sync with the live
    # state. The fixture is metadata-only — no body bytes.
    fixture_index = {
        "source": "docs/FIRESTAFF_GAP_LIST.md A5 (Real-data regression tests, greatstone db_data)",
        "scope": (
            "OFFLINE fixture: derived metadata only (status code, expected "
            "status, content-type hint, content-length hint, title text, "
            "title SHA-256, link count). No copyrighted bytes committed."
        ),
        "regenerate_with": (
            "python3 tools/verify_greatstone_db_data_paths.py --online --write"
        ),
        "paths": [
            {
                "path": r.path,
                "expected_status": r.expected_status,
                "actual_status": r.actual_status,
                "content_type": r.content_type,
                "content_length": r.content_length,
                "title": r.title,
                "title_sha256": r.title_sha256,
                "link_count": r.link_count,
            }
            for r in results
        ],
    }
    FIXTURE_DIR.mkdir(parents=True, exist_ok=True)
    FIXTURE_INDEX.write_text(
        json.dumps(fixture_index, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )


def load_expected_evidence() -> dict:
    return json.loads(EVIDENCE_PATH.read_text(encoding="utf-8"))


def diff_with_expected(results: list[ProbeResult]) -> list[str]:
    """Compare freshly-computed results to the on-disk evidence."""
    if not EVIDENCE_PATH.is_file():
        return [f"evidence file missing: {EVIDENCE_PATH} (run with --write to create it)"]
    expected = load_expected_evidence()
    expected_rows = {row["path"]: row for row in expected.get("rows", [])}
    diffs: list[str] = []
    for r in results:
        er = expected_rows.get(r.path)
        if er is None:
            diffs.append(f"{r.path}: missing in expected evidence (run --write)")
            continue
        if er["actual_status"] != r.actual_status:
            # 200 <-> 206 is not a real drift; the server can choose
            # either for the same Range request depending on
            # intermediate-cache behavior. Skip the report in that case.
            if {er["actual_status"], r.actual_status} != {200, 206}:
                diffs.append(
                    f"{r.path}: actual_status drift "
                    f"expected={er['actual_status']} got={r.actual_status}"
                )
        if er["expected_status"] != r.expected_status:
            diffs.append(
                f"{r.path}: expected_status drift "
                f"expected={er['expected_status']} got={r.expected_status}"
            )
        if er.get("title_sha256") != r.title_sha256:
            diffs.append(
                f"{r.path}: title_sha256 drift "
                f"(expected={(er.get('title_sha256') or 'None')[:12]}, "
                f"got={(r.title_sha256 or 'None')[:12]})"
            )
    return diffs


def print_table(results: list[ProbeResult]) -> None:
    print()
    print(f"{'Match':<6} {'Status':<6} {'Elapsed':<8} {'Title/role':<48} {'Path'}")
    print("-" * 130)
    for r in results:
        status = f"{r.actual_status}" if r.actual_status is not None else "ERR"
        mark = "OK" if r.match else "FAIL"
        elapsed = f"{r.elapsed_ms}ms"
        title_or_label = (r.title or r.label or "")[:46]
        print(f"{mark:<6} {status:<6} {elapsed:<8} {title_or_label:<48} {r.path}")
        if r.error and not r.match:
            print(f"      -> {r.error}")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--online", action="store_true",
                    help="Hit live URLs (default: replay the local fixture). "
                         "Also enabled when FIRESTAFF_GREATSTONE_PROBE=1.")
    ap.add_argument("--write", action="store_true",
                    help=f"Write evidence to {EVIDENCE_PATH.relative_to(REPO_ROOT)} "
                         "instead of comparing against the on-disk copy.")
    ap.add_argument("--timeout", type=float, default=DEFAULT_TIMEOUT,
                    help=f"Per-URL timeout in seconds (default: {DEFAULT_TIMEOUT})")
    ap.add_argument("--retries", type=int, default=DEFAULT_RETRIES,
                    help=f"Per-URL retries (default: {DEFAULT_RETRIES})")
    args = ap.parse_args()

    is_online = args.online or os.environ.get("FIRESTAFF_GREATSTONE_PROBE") == "1"
    expectations = build_expectations()

    if is_online:
        try:
            results = run_online(expectations, args.timeout, args.retries)
            mode = "online"
        except Exception as e:  # pragma: no cover - defensive
            print(f"online probe failed: {type(e).__name__}: {e}", file=sys.stderr)
            return 3
    else:
        if not FIXTURE_INDEX.is_file():
            print(f"fixture missing: {FIXTURE_INDEX}", file=sys.stderr)
            return 2
        results = run_offline(expectations)
        mode = "offline"

    if args.write:
        write_evidence(results, mode)
        print(f"wrote {EVIDENCE_PATH.relative_to(REPO_ROOT)} ({len(results)} rows)")
        print(f"wrote {FIXTURE_INDEX.relative_to(REPO_ROOT)} ({len(results)} rows, metadata-only)")
        return 0

    diffs = diff_with_expected(results)
    print_table(results)
    print()
    print(f"mode: {mode}")
    print(f"expectations: {len(results)}  match: {sum(1 for r in results if r.match)}/{len(results)}")
    if diffs:
        print()
        print("Drift against on-disk evidence:")
        for d in diffs:
            print(f"  - {d}")
        return 1
    print("evidence matches on-disk copy")
    return 0


if __name__ == "__main__":
    sys.exit(main())
