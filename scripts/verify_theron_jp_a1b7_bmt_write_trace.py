#!/usr/bin/env python3
"""Verify complete, mapped writes from the bounded Theron $A1B7 TII trace."""

from __future__ import annotations

import argparse
import collections
import sys
from pathlib import Path


SOURCE_MARKER = "mednafen-pce-fast-a1b7-bmt-main-ram-write-v1"
OPCODE_BYTES = "737f2883281400"
SOURCE = 0x287F
DESTINATION = 0x2883
LENGTH = 0x14
BASE_RAM_PAGE = 0x1F0000


def parse_fields(line: str) -> dict[str, str]:
    fields: dict[str, str] = {}
    for token in line.split():
        key, separator, value = token.partition("=")
        if separator:
            fields[key] = value
    return fields


def verify(lines: list[str]) -> tuple[int, int]:
    if not lines or parse_fields(lines[0]).get("source") != SOURCE_MARKER:
        raise ValueError("unexpected or missing trace source marker")

    rows = [
        parse_fields(line)
        for line in lines[1:]
        if line.startswith("a1b7_bmt_write ")
    ]
    if not rows:
        raise ValueError("trace contains no A1B7 block-transfer writes")
    for sequence, row in enumerate(rows):
        if int(row["sequence"]) != sequence:
            raise ValueError(f"non-contiguous write sequence at row {sequence}")

    groups: dict[int, list[dict[str, str]]] = collections.defaultdict(list)
    for row in rows:
        groups[int(row["transfer_id"])].append(row)

    if sorted(groups) != list(range(len(groups))):
        raise ValueError("transfer IDs are not contiguous")
    observed_ids = []
    for row in rows:
        transfer_id = int(row["transfer_id"])
        if not observed_ids or observed_ids[-1] != transfer_id:
            observed_ids.append(transfer_id)
    if observed_ids != list(range(len(groups))):
        raise ValueError("writes are not ordered in contiguous transfer windows")

    unchanged_writes = 0
    expected_iterations = list(range(LENGTH))
    for transfer_id, writes in groups.items():
        iterations = [int(row["iteration"]) for row in writes]
        if iterations != expected_iterations:
            raise ValueError(
                f"transfer {transfer_id}: expected iterations 0..{LENGTH - 1}, "
                f"found {iterations}"
            )

        for iteration, row in enumerate(writes):
            expected = {
                "transfer_id": transfer_id,
                "first_observed": int(iteration == 0),
                "original_source": SOURCE,
                "original_destination": DESTINATION,
                "original_length": LENGTH,
                "source_logical": SOURCE + iteration,
                "source_physical": BASE_RAM_PAGE + ((SOURCE + iteration) & 0x1FFF),
                "destination_logical": DESTINATION + iteration,
                "destination_physical": BASE_RAM_PAGE + ((DESTINATION + iteration) & 0x1FFF),
                "backing_offset": (DESTINATION + iteration) & 0x1FFF,
                "remaining": LENGTH - iteration,
            }
            for field, value in expected.items():
                if int(row[field], 16 if field not in {"transfer_id", "first_observed"} else 10) != value:
                    raise ValueError(
                        f"transfer {transfer_id} iteration {iteration}: "
                        f"unexpected {field}={row[field]}"
                    )
            if row["code_bytes"] != OPCODE_BYTES:
                raise ValueError(f"transfer {transfer_id}: opcode bytes changed")
            if int(row["written_value"], 16) == int(row["old_value"], 16):
                unchanged_writes += 1

    return len(rows), unchanged_writes


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", help="A1B7 BMT write sidecar path, or - for stdin")
    args = parser.parse_args()

    try:
        lines = (
            sys.stdin.read().splitlines()
            if args.trace == "-"
            else Path(args.trace).read_text(encoding="utf-8").splitlines()
        )
        rows, unchanged = verify(lines)
    except (OSError, ValueError, KeyError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1

    print(
        "PASS: writes={} complete_transfers={} unchanged_destination_writes={}".format(
            rows, rows // LENGTH, unchanged
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
