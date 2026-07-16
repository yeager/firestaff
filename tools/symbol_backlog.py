#!/usr/bin/env python3
"""Build prioritized source-symbol backlogs from the Firestaff audit TSVs.

The audits are deliberately conservative: a symbol can be named in Firestaff
without being behaviorally verified. This tool keeps that distinction and
emits only work items that still need implementation, triage, or proof.
"""
from __future__ import annotations

import argparse
import csv
import json
from collections import Counter, defaultdict
from pathlib import Path
from typing import Iterable

ROOT = Path(__file__).resolve().parents[1]
REDMCSB_AUDIT = ROOT / "docs/reference/audits/REDMCSB_CALLABLE_SYMBOL_FULL_AUDIT.tsv"
SKPROJECT_AUDIT = ROOT / "docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv"
SYMBOL_DISPOSITIONS = ROOT / "docs/reference/audits/SYMBOL_DISPOSITIONS.tsv"

REDMCSB_OPEN = {"MISSING", "UNCERTAIN_NUMBERED_EVIDENCE"}
SKPROJECT_OPEN = {"MISSING", "UNCERTAIN"}
OPEN_DISPOSITIONS = {"", "OPEN", "NEEDS_IMPLEMENTATION", "NEEDS_VERIFICATION"}

REDMCSB_RUNTIME_FAMILIES = {
    "CHAMPION",
    "DUNGEON",
    "DUNVIEW",
    "DRAWVIEW",
    "EVENT",
    "GAMELOOP",
    "GROUP",
    "LOADSAVE",
    "MOVE",
    "MOVESENS",
    "PANEL",
    "PROJEXP",
    "SPELLS",
    "TIMELINE",
}

CSB_RUNTIME_FAMILIES = {
    "DSA",
    "CSBCode",
    "TIMER",
    "Graphics",
    "SAVEGAME",
}

DM2_RUNTIME_SOURCES = {
    "SKULLWIN/c_gdatfile.cpp",
    "SKULLWIN/c_gfx_blit.cpp",
    "SKULLWIN/c_gfx_str.cpp",
    "SKULLWIN/c_gfx_pal.cpp",
    "SKULLWIN/c_gfx_decode.cpp",
    "SKULLWIN/c_map.cpp",
    "SKULLWIN/c_move.cpp",
    "SKULLWIN/c_weather.cpp",
    "SKULLWIN/c_sound.cpp",
    "SKWIN/SkWinCore.cpp",
}


def read_tsv(path: Path) -> list[dict[str, str]]:
    if not path.exists():
        raise FileNotFoundError(f"required audit TSV is missing: {path}")
    with path.open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle, delimiter="\t"))


def read_optional_tsv(path: Path, warnings: list[str]) -> list[dict[str, str]]:
    if not path.exists():
        warnings.append(f"missing audit TSV: {path}")
        return []
    return read_tsv(path)


def read_dispositions(path: Path = SYMBOL_DISPOSITIONS) -> dict[tuple[str, str], dict[str, str]]:
    if not path.exists():
        return {}
    dispositions: dict[tuple[str, str], dict[str, str]] = {}
    for row in read_tsv(path):
        reference = row.get("reference", "").strip()
        symbol = row.get("symbol", "").strip()
        disposition = row.get("disposition", "").strip().upper()
        if not reference or not symbol:
            continue
        normalized = dict(row)
        normalized["disposition"] = disposition
        dispositions[(reference, symbol)] = normalized
    return dispositions


def disposition_for(
    dispositions: dict[tuple[str, str], dict[str, str]],
    reference: str,
    symbol: str,
) -> dict[str, str]:
    return dispositions.get((reference, symbol), {})


def is_open_disposition(disposition: str) -> bool:
    return disposition.strip().upper() in OPEN_DISPOSITIONS


def redmcsb_game(row: dict[str, str]) -> str:
    family = row.get("family", "")
    source = row.get("reference_source", "")
    if family in CSB_RUNTIME_FAMILIES or "CSB" in source or "DSA" in source:
        return "CSB"
    if family in REDMCSB_RUNTIME_FAMILIES:
        return "DM1"
    return "DM1/CSB"


def priority(status: str, family: str, source: str) -> int:
    score = 0
    if status in {"MISSING"}:
        score += 100
    if status in {"UNCERTAIN", "UNCERTAIN_NUMBERED_EVIDENCE"}:
        score += 60
    if family in REDMCSB_RUNTIME_FAMILIES:
        score += 30
    if family in CSB_RUNTIME_FAMILIES:
        score += 30
    if source in DM2_RUNTIME_SOURCES:
        score += 30
    if any(token in source.upper() for token in ("VIEW", "GRAPH", "LOAD", "SAVE", "MENU", "WEATHER")):
        score += 10
    return score


def redmcsb_backlog(
    rows: Iterable[dict[str, str]],
    dispositions: dict[tuple[str, str], dict[str, str]],
    include_disposed: bool,
) -> list[dict[str, object]]:
    items: list[dict[str, object]] = []
    for row in rows:
        status = row.get("status", "")
        if status not in REDMCSB_OPEN:
            continue
        symbol = row.get("symbol", "")
        disposition = disposition_for(dispositions, "ReDMCSB", symbol)
        disposition_name = disposition.get("disposition", "")
        if not include_disposed and not is_open_disposition(disposition_name):
            continue
        family = row.get("family", "")
        source = row.get("reference_source", "")
        game = redmcsb_game(row)
        items.append({
            "game": game,
            "reference": "ReDMCSB",
            "symbol": symbol,
            "family": family,
            "source": source,
            "status": status,
            "firestaff_mapping": row.get("firestaff_mapping", ""),
            "disposition": disposition_name or "OPEN",
            "disposition_owner": disposition.get("owner", ""),
            "disposition_evidence": disposition.get("evidence", ""),
            "priority": priority(status, family, source),
        })
    return sorted(items, key=lambda item: (-int(item["priority"]), str(item["game"]), str(item["symbol"])))


def skproject_backlog(
    rows: Iterable[dict[str, str]],
    dispositions: dict[tuple[str, str], dict[str, str]],
    include_disposed: bool,
) -> list[dict[str, object]]:
    items: list[dict[str, object]] = []
    for row in rows:
        status = row.get("status", "")
        if status not in SKPROJECT_OPEN:
            continue
        symbol = row.get("symbol", "")
        disposition = disposition_for(dispositions, "skproject", symbol)
        disposition_name = disposition.get("disposition", "")
        if not include_disposed and not is_open_disposition(disposition_name):
            continue
        source = row.get("source_file", "")
        family = row.get("family", "")
        items.append({
            "game": "DM2",
            "reference": "skproject",
            "symbol": symbol,
            "family": family,
            "source": f"{source}:{row.get('line', '')}",
            "status": status,
            "firestaff_mapping": row.get("firestaff_mapping", ""),
            "disposition": disposition_name or "OPEN",
            "disposition_owner": disposition.get("owner", ""),
            "disposition_evidence": disposition.get("evidence", ""),
            "priority": priority(status, family, source),
        })
    return sorted(items, key=lambda item: (-int(item["priority"]), str(item["source"]), str(item["symbol"])))


def summarize(items: Iterable[dict[str, object]]) -> dict[str, object]:
    by_game: Counter[str] = Counter()
    by_reference: Counter[str] = Counter()
    by_status: Counter[str] = Counter()
    by_family: dict[str, Counter[str]] = defaultdict(Counter)
    materialized = list(items)
    for item in materialized:
        game = str(item["game"])
        by_game[game] += 1
        by_reference[str(item["reference"])] += 1
        by_status[str(item["status"])] += 1
        by_family[game][str(item["family"])] += 1
    return {
        "totalOpen": len(materialized),
        "byGame": dict(sorted(by_game.items())),
        "byReference": dict(sorted(by_reference.items())),
        "byStatus": dict(sorted(by_status.items())),
        "topFamiliesByGame": {
            game: dict(counter.most_common(12))
            for game, counter in sorted(by_family.items())
        },
    }


def print_text(items: list[dict[str, object]], limit: int) -> None:
    summary = summarize(items)
    print(json.dumps(summary, indent=2, sort_keys=True))
    print()
    for item in items[:limit]:
        mapping = str(item.get("firestaff_mapping") or "-")
        disposition = str(item.get("disposition") or "OPEN")
        evidence = str(item.get("disposition_evidence") or "")
        suffix = f" [{disposition}: {evidence}]" if disposition != "OPEN" and evidence else ""
        print(
            f"{item['game']} {item['reference']} {item['status']} "
            f"p{item['priority']} {item['symbol']} {item['source']} -> {mapping}"
            f"{suffix}"
        )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game", choices=["DM1", "CSB", "DM1/CSB", "DM2"], help="only emit one queue")
    parser.add_argument("--limit", type=int, default=50)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--include-disposed", action="store_true", help="include rows with explicit closed dispositions")
    parser.add_argument(
        "--allow-missing-audits",
        action="store_true",
        help="emit the available backlog instead of failing when an audit TSV is absent",
    )
    parser.add_argument(
        "--dispositions",
        type=Path,
        default=SYMBOL_DISPOSITIONS,
        help="TSV with reference/symbol disposition evidence",
    )
    args = parser.parse_args()

    dispositions = read_dispositions(args.dispositions)
    warnings: list[str] = []
    if args.allow_missing_audits:
        redmcsb_rows = read_optional_tsv(REDMCSB_AUDIT, warnings)
        skproject_rows = read_optional_tsv(SKPROJECT_AUDIT, warnings)
    else:
        redmcsb_rows = read_tsv(REDMCSB_AUDIT)
        skproject_rows = read_tsv(SKPROJECT_AUDIT)
    items = redmcsb_backlog(redmcsb_rows, dispositions, args.include_disposed)
    items.extend(skproject_backlog(skproject_rows, dispositions, args.include_disposed))
    if args.game:
        items = [item for item in items if item["game"] == args.game]
    items.sort(key=lambda item: (-int(item["priority"]), str(item["game"]), str(item["symbol"])))
    if args.json:
        print(json.dumps({"summary": summarize(items), "warnings": warnings, "items": items[:args.limit]}, indent=2, sort_keys=True))
    else:
        for warning in warnings:
            print(f"WARNING: {warning}")
        print_text(items, args.limit)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
