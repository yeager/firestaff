#!/usr/bin/env python3
"""Validate bounded instruction-follow receipts around the authentic BIOS handoff."""

from __future__ import annotations

import re
import sys
from pathlib import Path


HEADER_V2 = (
    "source=mednafen-pce-fast-system-card-bootstrap-follow-v2 "
    "event_sequence_source=TheronTraceCDSharedEventSequence "
    "arm_pc=e11b bootstrap_limit=512 target_pc=4000 target_limit=128"
)
HEADER_V3 = (
    "source=mednafen-pce-fast-system-card-bootstrap-follow-v3 "
    "event_sequence_source=TheronTraceCDSharedEventSequence "
    "arm_pc=e11b bootstrap_limit=8192 target_pc=4000 target_limit=128"
)
HEADER_V4 = (
    "source=mednafen-pce-fast-system-card-bootstrap-follow-v4 "
    "event_sequence_source=TheronTraceCDSharedEventSequence "
    "arm_pc=e11b bootstrap_limit=8192 branch_follow_limit=4096 target_pc=4000 target_limit=128"
)
HEADER = HEADER_V2
ROW = re.compile(r"^(bootstrap_follow|bootstrap_branch_follow|target_follow) (.+)$")
def fields(text: str, line_number: int) -> tuple[str, dict[str, str]]:
    match = ROW.fullmatch(text)
    if not match:
        raise ValueError(f"malformed row at line {line_number}")
    row_kind, payload = match.groups()
    values: dict[str, str] = {}
    for token in payload.split():
        key, separator, value = token.partition("=")
        if not separator or key in values:
            raise ValueError(f"malformed field at line {line_number}")
        values[key] = value
    return row_kind, values


def verify_lines(lines: list[str], require_launch: bool = True) -> dict[str, int]:
    if not lines or lines[0] not in (HEADER_V2, HEADER_V3, HEADER_V4):
        raise ValueError("unexpected bootstrap-follow trace header")
    bootstrap_limit = 8192 if lines[0] in (HEADER_V3, HEADER_V4) else 512
    branch_limit = 4096 if lines[0] == HEADER_V4 else 0
    counts = {"bootstrap_follow": 0, "bootstrap_branch_follow": 0, "target_follow": 0}
    bootstrap_rows: list[dict[str, str]] = []
    branch_rows: list[dict[str, str]] = []
    target_rows: list[dict[str, str]] = []
    first_pc: dict[str, int] = {}
    truncation_seen = {
        "bootstrap_follow": False,
        "bootstrap_branch_follow": False,
        "target_follow": False,
    }
    previous_event_sequence = -1
    for line_number, line in enumerate(lines[1:], start=2):
        if line == f"bootstrap_follow_truncated cap={bootstrap_limit}":
            if truncation_seen["bootstrap_follow"] or counts["bootstrap_follow"] != bootstrap_limit:
                raise ValueError("bootstrap truncation marker disagrees with row count")
            truncation_seen["bootstrap_follow"] = True
            continue
        if line == f"bootstrap_branch_follow_truncated cap={branch_limit}" and branch_limit:
            if truncation_seen["bootstrap_branch_follow"] or counts["bootstrap_branch_follow"] != branch_limit:
                raise ValueError("bootstrap branch-follow truncation marker disagrees with row count")
            truncation_seen["bootstrap_branch_follow"] = True
            continue
        if line == "target_follow_truncated cap=128":
            if truncation_seen["target_follow"] or counts["target_follow"] != 128:
                raise ValueError("target truncation marker disagrees with row count")
            truncation_seen["target_follow"] = True
            continue
        kind, row = fields(line, line_number)
        if truncation_seen[kind]:
            raise ValueError(f"{kind} row appears after its truncation marker")
        expected_index = counts[kind]
        if int(row.get("index", "-1"), 10) != expected_index:
            raise ValueError(f"non-contiguous {kind} index at line {line_number}")
        kind_limit = {
            "bootstrap_follow": bootstrap_limit,
            "bootstrap_branch_follow": branch_limit,
            "target_follow": 128,
        }[kind]
        if kind_limit == 0 or expected_index >= kind_limit:
            raise ValueError(f"{kind} exceeds its configured cap")
        event_sequence_text = row["event_sequence"]
        if not re.fullmatch(r"(?:0|[1-9][0-9]*)", event_sequence_text):
            raise ValueError(f"malformed shared event sequence at line {line_number}")
        event_sequence = int(event_sequence_text, 10)
        if event_sequence <= previous_event_sequence:
            raise ValueError(f"non-increasing shared event sequence at line {line_number}")
        previous_event_sequence = event_sequence
        pc = int(row["logical_pc"], 16)
        if not 0 <= pc <= 0xFFFF:
            raise ValueError(f"logical PC is out of range at line {line_number}")
        physical_pc = int(row["physical_pc"], 16)
        mprs = [int(row[f"mpr{i}"], 16) for i in range(8)]
        if not 0 <= physical_pc <= 0xFFFFFF or any(not 0 <= value <= 0xFF for value in mprs):
            raise ValueError(f"physical address or MPR is out of range at line {line_number}")
        mapped_pc = (mprs[(pc >> 13) & 7] << 13) | (pc & 0x1FFF)
        if mapped_pc != physical_pc:
            raise ValueError(f"logical/physical PC mapping mismatch at line {line_number}")
        bytes_value = row["bytes"]
        int(bytes_value, 16)
        physical_bytes = [int(value, 16) for value in row["physical_bytes"].split(",")]
        valid_mask = int(row["valid_mask"], 16)
        if (
            len(bytes_value) != 8
            or len(physical_bytes) != 4
            or not 0 <= valid_mask <= 0xF
            or not valid_mask & 1
            or any(not 0 <= value <= 0xFFFFFF for value in physical_bytes)
        ):
            raise ValueError(f"instruction snapshot width mismatch at line {line_number}")
        for index, physical_address in enumerate(physical_bytes):
            if valid_mask & (1 << index):
                logical = (pc + index) & 0xFFFF
                expected_physical = (mprs[(logical >> 13) & 7] << 13) | (logical & 0x1FFF)
                if physical_address != expected_physical:
                    raise ValueError(f"mapped instruction byte mismatch at line {line_number}")
        if expected_index == 0 and kind != "bootstrap_branch_follow":
            first_pc[kind] = pc
        if kind == "bootstrap_follow":
            bootstrap_rows.append(row)
        elif kind == "bootstrap_branch_follow":
            branch_rows.append(row)
        else:
            target_rows.append(row)
        counts[kind] += 1
    if counts["bootstrap_follow"] and first_pc.get("bootstrap_follow") != 0xE11B:
        raise ValueError("bootstrap follow did not begin at $E11B")
    if counts["target_follow"] and first_pc.get("target_follow") != 0x4000:
        raise ValueError("target follow did not begin at $4000")
    if not counts["bootstrap_follow"] or not counts["target_follow"]:
        raise ValueError("capture did not observe both the BIOS bootstrap and $4000 target")
    if lines[0] == HEADER_V4 and require_launch:
        verify_launch_branch(branch_rows, target_rows)
    return {"bootstrap_follow": counts["bootstrap_follow"], "target_follow": counts["target_follow"]}


def verify_launch_branch(
    branch_rows: list[dict[str, str]], target_rows: list[dict[str, str]]
) -> dict[str, int]:
    """Require a contiguous trace from the BIOS carry branch through mapped $4000."""
    def require_instruction_bytes(row: dict[str, str], length: int, name: str) -> None:
        required_mask = (1 << length) - 1
        valid_mask = int(row["valid_mask"], 16)
        if valid_mask & required_mask != required_mask:
            raise ValueError(f"{name} instruction snapshot has invalid opcode or operand bytes")

    if not branch_rows:
        raise ValueError("bootstrap trace did not observe the $E11D BIOS carry branch")
    segments: list[list[dict[str, str]]] = []
    segment: list[dict[str, str]] = []
    for row in branch_rows:
        pc = int(row["logical_pc"], 16)
        if pc == 0xE11D:
            if segment:
                segments.append(segment)
            segment = [row]
            continue
        if segment:
            segment.append(row)
            if pc in (0xE129, 0x4000):
                segments.append(segment)
                segment = []
    if segment:
        segments.append(segment)

    saw_alternate = False
    for path in segments:
        branch = path[0]
        branch_event = int(branch["event_sequence"])
        require_instruction_bytes(branch, 2, "$E11D BCC")
        if int(branch["bytes"][:4], 16) != 0x900D:
            raise ValueError("$E11D does not contain the authentic BCC $E12C instruction")
        if len(path) < 2:
            raise ValueError("$E11D branch trace has no following instruction")
        if int(branch["p"], 16) & 0x01:
            saw_alternate = True
            if int(path[1]["logical_pc"], 16) != 0xE11F:
                raise ValueError("set-carry $E11D branch was not followed by $E11F")
            continue
        if int(path[1]["logical_pc"], 16) != 0xE12C:
            raise ValueError("clear-carry $E11D branch was not immediately followed by $E12C")
        load = path[1]
        require_instruction_bytes(load, 2, "$E12C LDA")
        if int(load["bytes"][:4], 16) != 0xA904:
            raise ValueError("$E12C does not contain authentic LDA #$04 bytes")
        if len(path) < 4 or int(path[2]["logical_pc"], 16) != 0xE12E:
            raise ValueError("$E12C was not immediately followed by the $E12E MPR setup call")
        setup = path[2]
        require_instruction_bytes(setup, 2, "$E12E BSR")
        if int(setup["bytes"][:4], 16) != 0x440D:
            raise ValueError("$E12E does not contain authentic BSR $E13D bytes")
        if int(path[3]["logical_pc"], 16) != 0xE13D:
            raise ValueError("$E12E BSR did not immediately enter the $E13D setup routine")
        jump_index = next(
            (index for index, row in enumerate(path[4:], start=4)
             if int(row["logical_pc"], 16) == 0xE130),
            None,
        )
        if jump_index is None:
            raise ValueError("$E13D setup did not return to the $E130 launch jump")
        jump = path[jump_index]
        require_instruction_bytes(jump, 3, "$E130 JMP")
        if int(jump["bytes"][:6], 16) != 0x4C0040:
            raise ValueError("$E130 does not contain authentic JMP $4000 bytes")
        if jump_index + 1 >= len(path) or int(path[jump_index + 1]["logical_pc"], 16) != 0x4000:
            raise ValueError("$E130 JMP was not immediately followed by the $4000 instruction")
        target = path[jump_index + 1]
        if int(target["physical_pc"], 16) != 0x008000 or int(target["mpr2"], 16) != 0x04:
            raise ValueError("$4000 launch target did not map through MPR2=$04 to physical $008000")
        return {
            "branch_event": branch_event,
            "mpr_setup_event": int(setup["event_sequence"]),
            "launch_jump_event": int(jump["event_sequence"]),
            "target_event": int(target["event_sequence"]),
        }
    if saw_alternate:
        raise ValueError("$E11D only selected the authentic set-carry alternate route; no launch branch was captured")
    raise ValueError("bootstrap branch trace did not contain the authentic launch path")


def verify(path: Path, require_launch: bool = True) -> dict[str, int]:
    return verify_lines(path.read_text(encoding="ascii").splitlines(), require_launch)


def main() -> int:
    import argparse

    parser = argparse.ArgumentParser(
        description="Validate the bounded System Card bootstrap and optional launch branch."
    )
    parser.add_argument("--allow-missing-launch", action="store_true")
    parser.add_argument("trace", type=Path)
    args = parser.parse_args()
    try:
        counts = verify(args.trace, require_launch=not args.allow_missing_launch)
    except (OSError, UnicodeError, ValueError, KeyError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print(
        "PASS: "
        f"status=bounded bootstrap_rows={counts['bootstrap_follow']} "
        f"target_rows={counts['target_follow']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
