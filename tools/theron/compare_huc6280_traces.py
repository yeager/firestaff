#!/usr/bin/env python3
"""Compare reference and C-lift HuC6280 traces for one identical invocation.

Input is JSON Lines: header, initial state, ordered events, then final state.
Matching traces are an execution comparison only; this tool never claims game
or platform parity. Tool-format fixtures are allowed, but are labelled as such.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any


SCHEMA = "firestaff.theron.huc6280_trace.v1"
EVENT_KINDS = {"fetch", "read", "write", "mpr"}
CPU_FIELDS = ("pc", "a", "x", "y", "s", "p", "mpr")
HASH_RE = re.compile(r"^[0-9a-f]{64}$")


class TraceError(ValueError):
    """Raised when a trace is incomplete or violates the interchange schema."""


@dataclass(frozen=True)
class Trace:
    path: Path
    header: dict[str, Any]
    initial: dict[str, Any]
    events: tuple[dict[str, Any], ...]
    final: dict[str, Any]


def _require(condition: bool, message: str) -> None:
    if not condition:
        raise TraceError(message)


def _is_byte(value: Any) -> bool:
    return isinstance(value, int) and not isinstance(value, bool) and 0 <= value <= 0xFF


def _is_word(value: Any) -> bool:
    return isinstance(value, int) and not isinstance(value, bool) and 0 <= value <= 0xFFFF


def _validate_state(state: Any, where: str) -> None:
    _require(isinstance(state, dict), f"{where} must be an object")
    for field in CPU_FIELDS:
        _require(field in state, f"{where}.{field} is required")
    _require(_is_word(state["pc"]), f"{where}.pc must be a 16-bit integer")
    for field in ("a", "x", "y", "s", "p"):
        _require(_is_byte(state[field]), f"{where}.{field} must be a byte")
    mpr = state["mpr"]
    _require(isinstance(mpr, list) and len(mpr) == 9,
             f"{where}.mpr must contain MPR0 through MPR8")
    _require(all(_is_byte(value) for value in mpr),
             f"{where}.mpr entries must be bytes")
    _require(isinstance(state.get("memory_sha256"), str) and
             HASH_RE.fullmatch(state["memory_sha256"]) is not None,
             f"{where}.memory_sha256 must be a lowercase SHA-256")


def _validate_event(event: Any, expected_seq: int, where: str) -> dict[str, Any]:
    _require(isinstance(event, dict), f"{where} must be an object")
    _require(isinstance(event.get("seq"), int) and
             not isinstance(event.get("seq"), bool) and
             event["seq"] == expected_seq,
             f"{where}.seq must be contiguous from zero (expected {expected_seq})")
    kind = event.get("kind")
    _require(isinstance(kind, str) and kind in EVENT_KINDS,
             f"{where}.kind is unknown")
    _require(_is_word(event.get("pc")), f"{where}.pc must be a 16-bit integer")
    if kind == "mpr":
        _require(_is_byte(event.get("slot")) and event["slot"] <= 8,
                 f"{where}.slot must identify MPR0 through MPR8")
        _require(_is_byte(event.get("old")) and _is_byte(event.get("new")),
                 f"{where} MPR values must be bytes")
    else:
        _require(_is_word(event.get("logical")),
                 f"{where}.logical must be a 16-bit address")
        _require(_is_byte(event.get("mpr_slot")) and event["mpr_slot"] <= 8,
                 f"{where}.mpr_slot must identify MPR0 through MPR8")
        _require(_is_byte(event.get("bank")), f"{where}.bank must be a byte")
        _require(_is_word(event.get("bank_offset")) and event["bank_offset"] < 0x2000,
                 f"{where}.bank_offset must be within an 8 KiB bank")
        _require(_is_byte(event.get("value")), f"{where}.value must be a byte")
        expected_slot = event["logical"] >> 13
        _require(event["mpr_slot"] == expected_slot,
                 f"{where}.mpr_slot does not map its logical address")
        _require(event["bank_offset"] == (event["logical"] & 0x1FFF),
                 f"{where}.bank_offset does not map its logical address")
    return event


def _validate_mpr_timeline(events: tuple[dict[str, Any], ...],
                           initial: dict[str, Any], final: dict[str, Any],
                           where: str) -> None:
    mpr = list(initial["mpr"])
    for event in events:
        if event["kind"] == "mpr":
            slot = event["slot"]
            _require(mpr[slot] == event["old"],
                     f"{where}: event seq={event['seq']} has stale old MPR{slot}")
            mpr[slot] = event["new"]
        else:
            slot = event["mpr_slot"]
            _require(mpr[slot] == event["bank"],
                     f"{where}: event seq={event['seq']} bank does not match live MPR{slot}")
    _require(mpr == final["state"]["mpr"],
             f"{where}: final MPR map does not match the ordered MPR events")


def load_trace(path: Path) -> Trace:
    try:
        lines = path.read_text(encoding="utf-8").splitlines()
    except (OSError, UnicodeError) as error:
        raise TraceError(f"cannot read {path}: {error}") from error
    _require(len(lines) >= 3, f"{path}: trace is missing required records")
    records: list[dict[str, Any]] = []
    for index, line in enumerate(lines, 1):
        try:
            record = json.loads(line)
        except json.JSONDecodeError as error:
            raise TraceError(f"{path}:{index}: invalid JSON: {error.msg}") from error
        _require(isinstance(record, dict), f"{path}:{index}: record must be an object")
        records.append(record)
    header, initial, *tail = records
    _require(header.get("record") == "header", f"{path}: first record must be header")
    _require(header.get("schema") == SCHEMA, f"{path}: unsupported schema")
    _require(isinstance(header.get("role"), str) and
             header["role"] in {"reference", "lift"},
             f"{path}: invalid trace role")
    _require(isinstance(header.get("evidence"), str) and
             header["evidence"] in {"authentic-media", "tool-fixture"},
             f"{path}: evidence must be authentic-media or tool-fixture")
    media = header.get("media")
    _require(isinstance(media, dict), f"{path}: media identity is required")
    _require(isinstance(media.get("edition"), str) and media["edition"],
             f"{path}: media.edition is required")
    _require(isinstance(media.get("sha256"), str) and
             HASH_RE.fullmatch(media["sha256"]) is not None,
             f"{path}: media.sha256 must be a lowercase SHA-256")
    if header["evidence"] == "authentic-media":
        _require(media.get("authenticated") is True,
                 f"{path}: authentic-media evidence must assert media.authenticated=true")
        _require(isinstance(media.get("receipt_sha256"), str) and
                 HASH_RE.fullmatch(media["receipt_sha256"]) is not None,
                 f"{path}: authenticated media requires receipt_sha256")
    else:
        _require(media.get("authenticated") is False,
                 f"{path}: tool fixtures must assert media.authenticated=false")
    _require(isinstance(header.get("input_sha256"), str) and
             HASH_RE.fullmatch(header["input_sha256"]) is not None,
             f"{path}: input_sha256 must be a lowercase SHA-256")
    _require(isinstance(header.get("source_sha256"), str) and
             HASH_RE.fullmatch(header["source_sha256"]) is not None,
             f"{path}: source_sha256 must be a lowercase SHA-256")
    _require(initial.get("record") == "initial", f"{path}: second record must be initial")
    _validate_state(initial.get("state"), f"{path}: initial.state")

    events: list[dict[str, Any]] = []
    final: dict[str, Any] | None = None
    for record in tail:
        record_type = record.get("record")
        if record_type == "event":
            _require(final is None, f"{path}: event appears after final state")
            events.append(_validate_event(record.get("event"), len(events),
                                          f"{path}: event {len(events)}"))
        elif record_type == "final":
            _require(final is None, f"{path}: duplicate final state")
            final = record
            break
        else:
            raise TraceError(f"{path}: unknown record type {record_type!r}")
    _require(final is not None, f"{path}: final state is missing")
    _require(tail[-1] is final, f"{path}: records appear after final state")
    _require(final.get("truncated") is False, f"{path}: truncated trace is not comparable")
    _require(final.get("termination") == "return_2392",
             f"{path}: trace did not terminate at the $2392 return boundary")
    _validate_state(final.get("state"), f"{path}: final.state")
    _validate_mpr_timeline(tuple(events), initial["state"], final,
                           str(path))
    changed = final.get("changed_memory")
    _require(isinstance(changed, list), f"{path}: final.changed_memory must be an array")
    previous_address = -1
    for index, item in enumerate(changed):
        _require(isinstance(item, list) and len(item) == 2,
                 f"{path}: changed_memory[{index}] must be [address, byte]")
        address, value = item
        _require(_is_word(address) and _is_byte(value),
                 f"{path}: changed_memory[{index}] must contain a 16-bit address and byte")
        _require(address > previous_address,
                 f"{path}: changed_memory must be sorted with unique addresses")
        previous_address = address
    return Trace(path, header, initial["state"], tuple(events), final)


def _cpu_differences(left: dict[str, Any], right: dict[str, Any], label: str) -> list[str]:
    differences = []
    for field in CPU_FIELDS:
        if left[field] != right[field]:
            differences.append(f"{label}.{field}: {left[field]!r} != {right[field]!r}")
    if left["memory_sha256"] != right["memory_sha256"]:
        differences.append(f"{label}.memory_sha256 differs")
    return differences


def compare(reference: Trace, lift: Trace) -> list[str]:
    differences: list[str] = []
    if reference.header["role"] != "reference":
        differences.append("first trace role must be reference")
    if lift.header["role"] != "lift":
        differences.append("second trace role must be lift")
    if differences:
        return differences
    if reference.header["evidence"] != lift.header["evidence"]:
        differences.append("evidence class differs")
    if reference.header["media"] != lift.header["media"]:
        differences.append("media identity/receipt differs")
    if reference.header["input_sha256"] != lift.header["input_sha256"]:
        differences.append("guest input SHA-256 differs")
    differences.extend(_cpu_differences(reference.initial, lift.initial, "initial"))
    if differences:
        return differences

    event_count = max(len(reference.events), len(lift.events))
    for index in range(event_count):
        if index >= len(reference.events):
            differences.append(f"first event divergence at seq={index}: reference ended")
            break
        if index >= len(lift.events):
            differences.append(f"first event divergence at seq={index}: lift ended")
            break
        ref_event, lift_event = reference.events[index], lift.events[index]
        if ref_event != lift_event:
            differences.append(
                f"first event divergence at seq={index}: reference={ref_event!r}; lift={lift_event!r}"
            )
            break

    differences.extend(_cpu_differences(reference.final["state"], lift.final["state"], "final"))
    ref_memory = {address: value for address, value in reference.final["changed_memory"]}
    lift_memory = {address: value for address, value in lift.final["changed_memory"]}
    for address in sorted(ref_memory.keys() | lift_memory.keys()):
        if ref_memory.get(address) != lift_memory.get(address):
            differences.append(
                f"changed_memory[0x{address:04x}]: "
                f"reference={ref_memory.get(address)!r}; lift={lift_memory.get(address)!r}"
            )
    return differences


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reference", type=Path, help="reference JSONL invocation trace")
    parser.add_argument("lift", type=Path, help="C-lift JSONL invocation trace")
    args = parser.parse_args()
    try:
        reference = load_trace(args.reference)
        lift = load_trace(args.lift)
        differences = compare(reference, lift)
    except TraceError as error:
        print(f"THERON_HUC6280_TRACE_INVALID: {error}", file=sys.stderr)
        return 2
    provenance = reference.header["evidence"]
    print(f"evidence_class={provenance}")
    print(f"media_sha256={reference.header['media']['sha256']}")
    print(f"input_sha256={reference.header['input_sha256']}")
    print(f"source_sha256_reference={reference.header['source_sha256']}")
    print(f"source_sha256_lift={lift.header['source_sha256']}")
    print(f"events_reference={len(reference.events)} events_lift={len(lift.events)}")
    print("parity_claim=not-established")
    if differences:
        print("execution_comparison=divergent")
        for difference in differences:
            print(f"difference={difference}")
        return 1
    print("execution_comparison=matching_observations")
    print("claim=matching invocation traces only; full game/platform parity is not established")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
