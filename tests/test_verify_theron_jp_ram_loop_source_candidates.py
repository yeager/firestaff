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

    report = verifier.classify([fetch], reads, writes, {})
    if report["matching_latest_address_scoped_writes"] != 3:
        raise AssertionError("matching writer bytes were not counted")
    if report["matching_writes_with_source_valid_ea99_read_candidate_within_16_events"] != 3:
        raise AssertionError("ordered same-value source reads within the window were not joined")
    if report["fetch_rows_fully_candidate_backed"] != 1:
        raise AssertionError("fully candidate-backed fetch row was not counted")

    mismatching = [*writes[:-1], row(35, addresses[-1], 0xEA)]
    report = verifier.classify([fetch], reads, mismatching, {})
    if report["mismatching_latest_address_scoped_writes"] != 1:
        raise AssertionError("latest-write byte mismatch was not reported")
    if report["fetch_rows_fully_candidate_backed"] != 0:
        raise AssertionError("mismatching fetch row was incorrectly fully backed")

    wrong_reader = [*reads[:-1], {**reads[-1], "reader_pc": "e4c5"}]
    report = verifier.classify([fetch], wrong_reader, writes, {})
    if report["matching_writes_without_source_read_candidate_within_16_events"] != 1:
        raise AssertionError("non-EA99 read was accepted as a source candidate")
    if report["fetch_rows_fully_candidate_backed"] != 1:
        raise AssertionError("matching writes were conflated with missing source candidates")
    if report["fetch_rows_fully_source_candidate_backed"] != 0:
        raise AssertionError("row without all source candidates was fully source-backed")

    delayed_and_repeated = [*reads[:-1],
                            {**reads[-1], "event_sequence": "27"},
                            {**reads[-1], "event_sequence": "28"}]
    report = verifier.classify([fetch], delayed_and_repeated, writes, {})
    if report["nearest_source_read_candidate_gap_1"] != 2:
        raise AssertionError("two-event source-read candidate gap was not recorded")
    if report["ambiguous_source_read_candidate_byte_slots"] != 1:
        raise AssertionError("repeated-value source candidates were not marked ambiguous")

    target_fetch = {**fetch, "bytes": "ad28ea"}
    target_writes = {
        addresses[-1]: [(30, values[-1], 0xEA9C), (35, 0xEA, 0x4055)],
    }
    report = verifier.classify([target_fetch], reads, writes, target_writes)
    if report["matching_latest_address_scoped_writes"] != 3:
        raise AssertionError("all-PC target write did not supersede the older $EA9C write")
    if report["latest_non_ea9c_writer_byte_slots"] != 1:
        raise AssertionError("non-$EA9C target writer was not distinguished")
    if report["fetch_rows_fully_source_candidate_backed"] != 0:
        raise AssertionError("non-$EA9C writer was promoted to a CD source join")

    instruction_rows = []
    helper_reads = []
    helper_writes = []
    helper_event_by_pc = {}
    physical_start = 0x102109
    unique_addresses = sorted({
        physical_start + pc - 0x6109 + offset
        for pc, instruction in verifier.HELPER_INSTRUCTIONS.items()
        for offset in range(len(instruction))
    })
    value_by_address = {}
    for address_index, address in enumerate(unique_addresses):
        value = 0x60 if address == 0x102114 else next(
            instruction[offset]
            for pc, instruction in verifier.HELPER_INSTRUCTIONS.items()
            for offset in range(len(instruction))
            if physical_start + pc - 0x6109 + offset == address
        )
        value_by_address[address] = value
        write_event = 10 + address_index * 20
        helper_writes.append({
            "event_sequence": str(write_event),
            "physical_address": f"{address:06x}",
            "value": f"{value:02x}",
        })
        helper_reads.append({
            "event_sequence": str(write_event - 1),
            "value": f"{value:02x}",
            "source_valid": "1",
            "reader_pc": "ea99",
        })

    for index, (pc, instruction) in enumerate(verifier.HELPER_INSTRUCTIONS.items()):
        event = 1000 + index * 2
        helper_event_by_pc[pc] = event
        addresses = [physical_start + pc - 0x6109 + offset for offset in range(3)]
        values = bytes(value_by_address.get(address, 0xEA) for address in addresses)
        instruction_rows.append({
            "dispatch_event": "1",
            "index": str(index),
            "event_sequence": str(event),
            "logical_pc": f"{pc:04x}",
            "bytes": values.hex(),
            "physical_bytes": ",".join(f"{address:06x}" for address in addresses),
        })
    instruction_rows.append({
        "dispatch_event": "1",
        "index": str(len(instruction_rows)),
        "event_sequence": str(1010),
        "logical_pc": f"{verifier.HELPER_RETURN_PC:04x}",
        "bytes": "000000",
        "physical_bytes": "10032b,10032c,10032d",
    })
    helper_target_writes = {
        0x1000D9 + index: [(helper_event_by_pc[pc] + 1, 0xEA, pc)]
        for index, pc in enumerate((0x610B, 0x610E, 0x6111))
    }
    helper_report = verifier.classify_patch_helper(
        instruction_rows, helper_reads, helper_writes, helper_target_writes
    )
    if helper_report["complete_source_patch_helper_calls"] != 1:
        raise AssertionError("complete source patch-helper sequence was not recognized")
    if helper_report["patch_helper_bytes_matching_latest_ea9c_write"] != 12:
        raise AssertionError("patch-helper instruction bytes did not match source writes")
    if helper_report["patch_helper_target_writes_verified"] != 3:
        raise AssertionError("patch-helper stores were not joined to target writes")

    print("PASS: RAM-loop verifier distinguishes matching, mismatching, and non-EA99 candidates")


if __name__ == "__main__":
    main()
