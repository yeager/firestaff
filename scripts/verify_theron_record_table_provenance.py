#!/usr/bin/env python3
"""Verify a direct Track 02 -> $611D record-table provenance join.

This consumes only local Mednafen sidecars. It deliberately proves byte
provenance, not level/object/gameplay semantics.
"""

import argparse
import sys
from collections import defaultdict


def fields(line):
    return dict(item.split("=", 1) for item in line.split()[1:] if "=" in item)


def load(path, prefix):
    rows = []
    with open(path, encoding="utf-8") as stream:
        for line in stream:
            if line.startswith(prefix + " "):
                rows.append(fields(line.rstrip("\n")))
    return rows


def number(row, name):
    return int(row[name], 16) if name.endswith(("address", "logical", "physical", "pc", "physical_pc")) else int(row[name])


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("ram_provenance")
    parser.add_argument("record_watch")
    parser.add_argument("--minimum-records", type=int, default=1)
    args = parser.parse_args()

    direct = load(args.ram_provenance, "theron_ram_provenance_direct")
    watch = load(args.record_watch, "theron_record_watch")
    watch_index = defaultdict(int)
    for row in watch:
        if row.get("op") != "write":
            continue
        key = tuple(row.get(name) for name in
                    ("logical_address", "physical_address", "value", "pc"))
        watch_index[key] += 1

    groups = defaultdict(list)
    failures = []
    for row in direct:
        logical = number(row, "destination_logical")
        if not 0x611D <= logical <= 0x6126:
            continue
        key = tuple(row.get(name) for name in
                    ("destination_logical", "destination_physical", "value", "writer_pc"))
        if watch_index[key] != 1:
            failures.append((key, watch_index[key]))
        physical = number(row, "destination_physical")
        groups[(row.get("source_lba"), physical - (logical - 0x611D))].append(row)

    complete = []
    for key, rows in groups.items():
        addresses = {number(row, "destination_logical") for row in rows}
        offsets = {row.get("source_offset") for row in rows}
        if addresses == set(range(0x611D, 0x6127)) and len(offsets) == 10:
            complete.append((key, rows))

    if failures or len(complete) < args.minimum_records:
        print("FAIL: direct record-table provenance join", file=sys.stderr)
        print(f"direct_rows={len(direct)} complete_records={len(complete)} "
              f"watch_mismatches={len(failures)}", file=sys.stderr)
        return 1

    print(f"PASS: direct_rows={len(direct)} complete_records={len(complete)} "
          "record_watch_exact_matches=" +
          str(sum(len(rows) for _, rows in complete)))
    for (lba, physical), rows in sorted(complete):
        print(f"record source_lba={lba} destination_physical={physical} "
              f"source_offsets={min(row['source_offset'] for row in rows)}.."
              f"{max(row['source_offset'] for row in rows)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
