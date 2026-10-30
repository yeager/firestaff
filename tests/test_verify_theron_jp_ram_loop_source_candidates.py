#!/usr/bin/env python3
"""Exercise bounded source-candidate joins for the JP RAM-loop verifier."""

from pathlib import Path
import sys

repo = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(repo / "scripts"))

import verify_theron_jp_ram_loop_source_candidates as verifier


def row(event: int, address: int, value: int) -> dict[str, str]:
    return {
        "event_sequence": str(event),
        "physical_address": f"{address:06x}",
        "value": f"{value:02x}",
    }


def main() -> None:
    fetch = {
        "event_sequence": "40",
        "logical_pc": "40d2",
        "bytes": "ad2822",
        "physical_bytes": "1000d2,1000d3,1000d4",
        "valid_mask": "7",
    }
    addresses = (0x1000D2, 0x1000D3, 0x1000D4)
    values = (0xAD, 0x28, 0x22)
    writes = [row(event, address, value)
              for event, address, value in zip((10, 20, 30), addresses, values, strict=True)]
    reads = [
        {"event_sequence": str(event), "value": f"{value:02x}",
         "source_valid": "1", "reader_pc": "ea99"}
        for event, value in zip((9, 19, 29), values, strict=True)
    ]

    report = verifier.classify([fetch], reads, writes)
    if report["matching_latest_prior_ea9c_writes"] != 3:
        raise AssertionError("matching writer bytes were not counted")
    if report["matches_with_immediately_prior_source_valid_ea99_read"] != 3:
        raise AssertionError("immediately preceding source reads were not joined")
    if report["fetch_rows_fully_candidate_backed"] != 1:
        raise AssertionError("fully candidate-backed fetch row was not counted")

    mismatching = [*writes[:-1], row(35, addresses[-1], 0xEA)]
    report = verifier.classify([fetch], reads, mismatching)
    if report["mismatching_latest_prior_ea9c_writes"] != 1:
        raise AssertionError("latest-write byte mismatch was not reported")
    if report["fetch_rows_fully_candidate_backed"] != 0:
        raise AssertionError("mismatching fetch row was incorrectly fully backed")

    wrong_reader = [*reads[:-1], {**reads[-1], "reader_pc": "e4c5"}]
    report = verifier.classify([fetch], wrong_reader, writes)
    if report["matching_writes_without_immediately_prior_source_valid_ea99_read"] != 1:
        raise AssertionError("non-EA99 read was accepted as a source candidate")

    print("PASS: RAM-loop verifier distinguishes matching, mismatching, and non-EA99 candidates")


if __name__ == "__main__":
    main()
