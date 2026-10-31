#!/usr/bin/env python3
"""Verify a hash-bound PCE Fast UP/neutral trace comparison manifest."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path
from typing import Any


EXPECTED_SOURCE = {
    "cue_md5": "85706e7c2f658bc2792511d618dfc7a5",
    "track02_md5": "b7afb338ad31be1025b53f9aff12d73a",
    "system_card_md5": "ff1a674273fe3540ccef576376407d1d",
    "state_sha256": "2cc9938b96640a74db1a5b706113564b5d578d5011daf5f85c588ef1c98d70ee",
    "mednafen_md5": "09e5cae2a37d7c0734d970d98dc07f40",
}
EXPECTED_CUE_PATH = "/home/trv2/.firestaff/data/theron/Dungeon Master - Theron's Quest (Japan) (Rev 1).cue"
EXPECTED_STATE_PATH = "/home/trv2/firestaff-theron-evidence/emulator-created-20261003/capture-space-run-20261003/theron-jp-akutuba-f5-state.mca"
EXPECTED_CAPTURE_CONSTRAINTS = {
    "module": "pce_fast",
    "timeout_seconds": 10,
    "timeout_exit_status": 124,
    "controller_port": 0,
    "target_logical_address": "2912",
    "target_physical_address": "1f0912",
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def read_records(path: Path, prefix: str) -> list[dict[str, str]]:
    records: list[dict[str, str]] = []
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        if line.startswith(prefix + " "):
            records.append(dict(re.findall(r"([A-Za-z0-9_]+)=([^ ]+)", line)))
    return records


def load_artifact(run: dict[str, Any], name: str, root: Path) -> Path:
    try:
        item = run["artifacts"][name]
        relative = Path(item["path"])
        expected_hash = item["sha256"]
    except (KeyError, TypeError) as exc:
        raise ValueError(f"{run.get('capture_id')}: missing {name} artifact metadata") from exc
    require(not relative.is_absolute() and ".." not in relative.parts,
            f"{run.get('capture_id')}: artifact path must stay under manifest directory")
    path = (root / relative).resolve()
    require(path.parent == root.resolve(), f"{run.get('capture_id')}: artifact path escapes manifest directory")
    actual_hash = hashlib.sha256(path.read_bytes()).hexdigest()
    require(actual_hash == expected_hash,
            f"{run.get('capture_id')}: {name} SHA-256 mismatch ({actual_hash})")
    return path


def verify_up(run: dict[str, Any], root: Path) -> str:
    capture_id = run["capture_id"]
    require(run.get("stimulus") == "up" and run.get("input_script") == "up@61:120",
            f"{capture_id}: manifest does not identify the UP replay")
    input_path = load_artifact(run, "input", root)
    consumer_path = load_artifact(run, "consumer", root)
    write_path = load_artifact(run, "write", root)
    log_path = load_artifact(run, "log", root)
    log = log_path.read_text(encoding="utf-8", errors="replace")
    require(f'Loading "{EXPECTED_CUE_PATH}"' in log and f"Theron autoload state loaded: {EXPECTED_STATE_PATH}" in log,
            f"{capture_id}: log does not bind the expected authentic CUE and saved state")

    events = read_records(input_path, "scripted_pce_input_event")
    applies = read_records(input_path, "scripted_pce_input_apply")
    polls = read_records(input_path, "pce_input_read")
    require("source=mednafen-pce-fast-instrumented-input" in input_path.read_text(errors="replace"),
            f"{capture_id}: input trace is not from the expected PCE Fast instrumenter")
    matching_events = [r for r in events if r.get("frame") == "61" and r.get("key") == "up" and r.get("mask") == "0010" and r.get("port") == "0"]
    matching_applies = [r for r in applies if r.get("frame") == "61" and r.get("scripted") == "0010" and r.get("port") == "0"]
    matching_polls = [r for r in polls if r.get("frame") == "61" and r.get("cpu_pc") == "44c1" and r.get("physical_pc") == "000d04c1" and r.get("raw") == "0010" and r.get("sel") == "1" and r.get("value") == "3e" and r.get("module") == "pce_fast" and r.get("device_type") == "1" and r.get("scripted_port") == "0" and r.get("scripted_mask") == "0010"]
    require(len(matching_events) == len(matching_applies) == len(matching_polls) == 1,
            f"{capture_id}: expected one frame-61 UP event/apply/$44C1 read")

    consumers = read_records(consumer_path, "main_ram_consumer_read")
    writes = read_records(write_path, "main_ram_target_write")
    selected_reads = [r for r in consumers if r.get("logical_address") == "2912" and r.get("physical_address") == "1f0912"]
    selected_writes = [r for r in writes if r.get("logical_address") == "2912" and r.get("physical_address") == "1f0912"]
    require("TheronTraceCDSharedEventSequence" in consumer_path.read_text(errors="replace") and "TheronTraceCDSharedEventSequence" in write_path.read_text(errors="replace"),
            f"{capture_id}: RAM sidecars lack the shared event-counter header")
    before = [r for r in selected_reads if r.get("reader_pc") == "d334" and r.get("value") == "00"]
    up_writes = [r for r in selected_writes if r.get("writer_pc") == "d349" and r.get("value") == "10" and r.get("writer_code_bytes") == "8d1229730d290f29"]
    require(before and up_writes, f"{capture_id}: missing the $D334 pre-state or $D349 write of $10")

    def seq(record: dict[str, str]) -> int:
        return int(record["event_sequence"])

    ordered = None
    for writer in up_writes:
        prior = [r for r in before if seq(r) < seq(writer)]
        later = [r for r in selected_reads if r.get("reader_pc") == "d334" and r.get("reader_code_bytes") == "cd1229d00dee2029" and r.get("value") == "10" and seq(r) > seq(writer)]
        if prior and later:
            ordered = (writer, prior[-1], later[0])
            break
    require(ordered is not None, f"{capture_id}: no ordered $00 read → $10 write → $10 read")
    writer, prior_read, later_read = ordered
    return (f"{capture_id}: frame-61 UP read at $44C1; RAM sidecars show $2912 $00 read "
            f"event {seq(prior_read)} → $D349 write $10 event {seq(writer)} → "
            f"$D334 read $10 event {seq(later_read)}")


def verify_neutral(run: dict[str, Any], root: Path) -> str:
    capture_id = run["capture_id"]
    require(run.get("stimulus") == "neutral" and run.get("input_script") is None,
            f"{capture_id}: manifest does not identify a neutral run")
    input_path = load_artifact(run, "input", root)
    consumer_path = load_artifact(run, "consumer", root)
    write_path = load_artifact(run, "write", root)
    log_path = load_artifact(run, "log", root)
    log = log_path.read_text(encoding="utf-8", errors="replace")
    require(f'Loading "{EXPECTED_CUE_PATH}"' in log and f"Theron autoload state loaded: {EXPECTED_STATE_PATH}" in log,
            f"{capture_id}: log does not bind the expected authentic CUE and saved state")

    require(not read_records(input_path, "scripted_pce_input_event") and not read_records(input_path, "scripted_pce_input_apply"),
            f"{capture_id}: neutral trace contains a scripted event or apply")
    polls = read_records(input_path, "pce_input_read")
    require("source=mednafen-pce-fast-instrumented-input" in input_path.read_text(errors="replace"),
            f"{capture_id}: input trace is not from the expected PCE Fast instrumenter")
    require(polls and all(r.get("raw") == "0000" and r.get("scripted_mask") == "0000" for r in polls),
            f"{capture_id}: neutral trace contains a nonzero physical or scripted input poll")
    neutral_polls = [r for r in polls if r.get("frame") == "61" and r.get("cpu_pc") == "44c1" and r.get("module") == "pce_fast"]
    require(neutral_polls and all(r.get("raw") == "0000" and r.get("value") == "3f" and r.get("scripted_mask") == "0000" for r in neutral_polls),
            f"{capture_id}: no frame-61 neutral $44C1 poll with zero input")

    consumers = read_records(consumer_path, "main_ram_consumer_read")
    writes = read_records(write_path, "main_ram_target_write")
    selected_reads = [r for r in consumers if r.get("logical_address") == "2912" and r.get("physical_address") == "1f0912"]
    selected_writes = [r for r in writes if r.get("logical_address") == "2912" and r.get("physical_address") == "1f0912"]
    require(not [r for r in selected_reads if r.get("reader_pc") == "d334"],
            f"{capture_id}: neutral trace unexpectedly entered the $D334 consumer")
    require(not [r for r in selected_writes if r.get("writer_pc") == "d349"],
            f"{capture_id}: neutral trace unexpectedly entered the $D349 writer")
    require(selected_writes and all(r.get("writer_pc") == "d2f2" and r.get("writer_code_bytes") == "9c12299c202960ad" and r.get("value") == "00" for r in selected_writes),
            f"{capture_id}: neutral $D2F2 zero writes were missing or mismatched")
    return f"{capture_id}: frame-61 neutral $44C1 poll; no $D334/$D349 path; {len(selected_writes)} zero writes at $D2F2"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest", type=Path, help="JSON bundle manifest with per-run artifact hashes")
    args = parser.parse_args()
    try:
        root = args.manifest.resolve().parent
        manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
        require(manifest.get("format") == "firestaff-theron-pcefast-input-state-comparison-v1",
                "unsupported capture manifest format")
        require(manifest.get("source_identity") == EXPECTED_SOURCE,
                "manifest source-identity declarations do not match the expected values")
        require(manifest.get("capture_constraints") == EXPECTED_CAPTURE_CONSTRAINTS,
                "manifest capture constraints do not match the expected PCE Fast setup")
        runs = manifest["runs"]
        ids = [run["capture_id"] for run in runs]
        require(len(ids) == len(set(ids)), "capture IDs must be unique")
        artifact_paths = [item["path"] for run in runs for item in run["artifacts"].values()]
        require(len(artifact_paths) == len(set(artifact_paths)), "each run must use distinct artifact paths")
        ups = [run for run in runs if run.get("stimulus") == "up"]
        neutral = [run for run in runs if run.get("stimulus") == "neutral"]
        require(len(ups) == len(neutral) == 3, "manifest must contain three UP and three neutral runs")
        for run in ups:
            print(verify_up(run, root))
        for run in neutral:
            print(verify_neutral(run, root))
    except (OSError, KeyError, TypeError, ValueError, json.JSONDecodeError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1
    print("PASS: 3 hash-bound UP and 3 hash-bound neutral trace bundles")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
