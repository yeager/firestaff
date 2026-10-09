#!/usr/bin/env python3
"""Verify the recorded authentic JP control/UP RAM-update trace pair.

The captures prove a branch-path difference and a later observed byte value.
They do not assign gameplay meaning to $2912 or prove movement or a transition.
"""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import sys


CAPTURES = {
    "control": {
        "directory": "l4c46-jp-runtime-code-follow-20261009-1252",
        "files": {
            "live.trace.3879-indirect-target": (
                "fba73593e2d714aefacd1da0868b78f1409d7adbbb77e943b9c6c7618583179f"
            ),
            "live.trace.input": "c385191cc47aa3e165fa57fdffe2647d512909f9b36f8ab87967a9c2e4899240",
            "live.trace.main-ram-consumer": (
                "39c7c46b91d7aabfde2f755d250b8deb034fed327a201250a2af0377b9fdea65"
            ),
        },
        "path": ("d334", "d337", "d339"),
        "a": "00",
        "initial_ram_value": "00",
        "allowed_ram_values": {"00"},
    },
    "up": {
        "directory": "l4c46-jp-runtime-code-follow-input-20261009-1255",
        "files": {
            "live.trace.3879-indirect-target": (
                "6bd8222e1b1f378397772d71a5490c0e92893bad43ace7e285bf7f765d5f88ad"
            ),
            "live.trace.input": "759e32789568ce128bb980aafbdeeddf49ae74a6d5c4c74babe7916a7cd400a1",
            "live.trace.main-ram-consumer": (
                "f8b4ad931f6be2ca4e28da3dceb6c2489ab22ee742e99b33ef8cd93dc2349dd2"
            ),
        },
        "path": ("d334", "d337", "d346", "d349"),
        "a": "10",
        "initial_ram_value": "00",
        "allowed_ram_values": {"00", "10"},
        "later_ram_value": "10",
    },
}

TRACE_HEADER = "source=mednafen-pce-fast-instrumented-theron-3879-indirect-target"
RAM_HEADER = "source=mednafen-pce-fast-main-ram-consumer-read"


def read_fields(line: str, prefix: str) -> dict[str, str]:
    if not line.startswith(prefix + " "):
        raise ValueError(f"malformed {prefix} record")
    result: dict[str, str] = {}
    for item in line[len(prefix) + 1 :].split():
        if "=" not in item:
            raise ValueError(f"malformed {prefix} field")
        key, value = item.split("=", 1)
        if key in result:
            raise ValueError(f"duplicate {prefix} field {key}")
        result[key] = value
    return result


def verify_hashes(directory: Path, expected: dict[str, str]) -> None:
    for filename, expected_hash in expected.items():
        actual = hashlib.sha256((directory / filename).read_bytes()).hexdigest()
        if actual != expected_hash:
            raise ValueError(f"unexpected SHA-256 for {directory.name}/{filename}")


def read_code_rows(path: Path) -> list[dict[str, str]]:
    lines = path.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] != TRACE_HEADER:
        raise ValueError(f"unexpected code trace header in {path.name}")
    rows = [
        read_fields(line, "theron_3879_step")
        for line in lines[1:]
        if line.startswith("theron_3879_step ")
    ]
    if not rows:
        raise ValueError(f"missing instruction rows in {path.name}")
    return rows


def read_ram_values(path: Path) -> tuple[str, ...]:
    lines = path.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] != RAM_HEADER:
        raise ValueError(f"unexpected main-RAM trace header in {path.name}")
    values: list[str] = []
    expected_sequence = 0
    for line in lines[1:]:
        row = read_fields(line, "main_ram_consumer_read")
        if int(row["sequence"]) != expected_sequence:
            raise ValueError(f"non-contiguous RAM-read sequence in {path.name}")
        expected_sequence += 1
        if row.get("logical_address") == "2912" and row.get("reader_pc") == "d334":
            if row.get("reader_code_bytes") != "cd1229d00dee2029":
                raise ValueError(f"unexpected $D334 caller bytes in {path.name}")
            values.append(row["value"])
    if not values:
        raise ValueError(f"missing $2912 reads from $D334 in {path.name}")
    return tuple(values)


def verify_capture(root: Path, label: str, expected: dict[str, object]) -> None:
    directory = root / str(expected["directory"])
    verify_hashes(directory, expected["files"])  # type: ignore[arg-type]
    rows = read_code_rows(directory / "live.trace.3879-indirect-target")
    positions = [
        index for index, row in enumerate(rows)
        if row.get("logical_pc") == "d334"
    ]
    if len(positions) != 1:
        raise ValueError(f"expected one bounded $D334 hit in {label}, found {len(positions)}")
    index = positions[0]
    wanted_path = expected["path"]
    following = rows[index : index + len(wanted_path)]
    if tuple(row.get("logical_pc") for row in following) != wanted_path:
        raise ValueError(f"unexpected $D334 branch path in {label}")
    compare = following[0]
    if (
        compare.get("opcode") != "cd"
        or compare.get("operand1_mapped") != "12"
        or compare.get("operand2_mapped") != "29"
        or compare.get("a") != expected["a"]
    ):
        raise ValueError(f"unexpected $D334 compare state in {label}")
    branch = following[1]
    if branch.get("opcode") != "d0" or branch.get("operand1_mapped") != "0d":
        raise ValueError(f"unexpected $D337 branch instruction in {label}")
    if label == "up":
        clear, store = following[2:]
        if (
            clear.get("opcode") != "9c"
            or clear.get("operand1_mapped") != "20"
            or clear.get("operand2_mapped") != "29"
            or store.get("opcode") != "8d"
            or store.get("operand1_mapped") != "12"
            or store.get("operand2_mapped") != "29"
            or store.get("a") != "10"
        ):
            raise ValueError("UP capture does not show the expected $D346/$D349 store path")
    values = read_ram_values(directory / "live.trace.main-ram-consumer")
    if not values or values[0] != expected["initial_ram_value"]:
        raise ValueError(f"unexpected initial $2912 read in {label}: {values[:1]}")
    if not set(values).issubset(expected["allowed_ram_values"]):
        raise ValueError(f"unexpected $2912 byte in {label}: {values}")
    later_value = expected.get("later_ram_value")
    if later_value is not None and later_value not in values[1:]:
        raise ValueError(f"later $2912 value {later_value} was not read in {label}")
    print(f"PASS: authenticated {label} capture follows the recorded $2912 path")


def main() -> int:
    evidence_root = Path(
        os.environ.get(
            "FIRESTAFF_THERON_RUNTIME_INPUT_POLL_CAPTURE_DIR",
            Path.home() / "firestaff-theron-evidence" / "capture",
        )
    )
    if not evidence_root.is_dir():
        print("SKIP: authentic Theron runtime-input evidence is unavailable")
        return 77
    for label, expected in CAPTURES.items():
        verify_capture(evidence_root, label, expected)
    print(
        "LIMIT: trace proves the recorded branch and later RAM reads, "
        "not gameplay meaning or movement"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (KeyError, OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
