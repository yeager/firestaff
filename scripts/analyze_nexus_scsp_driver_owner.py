#!/usr/bin/env python3
"""Bind an observed SCSP trace PC to the authenticated 68K driver body.

This receipt proves only a source-owned command-handler corridor.  It does not
infer the SLEV selector, MAP row, SAL sample, or authorize host playback.
"""

from __future__ import annotations

import argparse
import hashlib
import re
from pathlib import Path


DRIVER_SHA256 = "68890ee4a49fd0c341bc3f0a48643e4db4b175df0b0d7dacfeb88306340052b6"
TRACE_HEADER = "FIRESTAFF_NEXUS_SCSP_WRITE_TRACE_V1"
TRACE_LINE = re.compile(
    r"addr=0x(?P<addr>[0-9a-fA-F]+) size=(?P<size>[0-9]+) "
    r"value=0x(?P<value>[0-9a-fA-F]+) pc=0x(?P<pc>[0-9a-fA-F]+)$"
)

# SDDRVS.TSK, load base 0x1000, handler runtime PC 0x3220..0x323c:
# clear/read command byte, compare against 0x12, update state, shift channel
# index, and write the SCSP per-channel register at offset 0x17.
HANDLER_OFFSET = 0x2220
HANDLER_BYTES = bytes.fromhex(
    "424010180c40001264000012121043ee187e13810000"
    "eb481b8100174e75"
)
HANDLER_PC = 0x3224


def trace_pcs(path: Path) -> list[int]:
    lines = path.read_text(encoding="ascii").splitlines()
    if not lines or lines[0] != TRACE_HEADER:
        raise ValueError(f"{path}: bad trace header")
    pcs: list[int] = []
    for number, line in enumerate(lines[1:], 2):
        match = TRACE_LINE.fullmatch(line)
        if not match:
            raise ValueError(f"{path}: malformed line {number}")
        if int(match.group("value"), 16):
            pcs.append(int(match.group("pc"), 16))
    return pcs


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("driver", type=Path)
    parser.add_argument("--load-base", type=lambda value: int(value, 0), default=0x1000)
    args = parser.parse_args()
    try:
        driver = args.driver.read_bytes()
        pcs = trace_pcs(args.trace)
    except (OSError, UnicodeError, ValueError) as error:
        print(f"NEXUS_SCSP_DRIVER_OWNER_INVALID: {error}")
        return 1

    digest = hashlib.sha256(driver).hexdigest()
    source_window = driver[HANDLER_OFFSET : HANDLER_OFFSET + len(HANDLER_BYTES)]
    source_hash_verified = digest == DRIVER_SHA256
    handler_verified = source_window == HANDLER_BYTES
    observed_handler_pc = HANDLER_PC in pcs
    print(f"driver_sha256={digest}")
    print(f"driver_sha256_verified={int(source_hash_verified)}")
    print(f"handler_file_offset=0x{HANDLER_OFFSET:04x}")
    print(f"handler_runtime_pc=0x{HANDLER_PC:04x}")
    print(f"handler_source_bytes_verified={int(handler_verified)}")
    print(f"handler_runtime_pc_observed={int(observed_handler_pc)}")
    print("handler_role=command-byte-to-driver-state-and-scsp-register-family")
    print("slev_selector_bound=0")
    print("sal_sample_bound=0")
    print("host_playback_authorized=0")
    accepted = source_hash_verified and handler_verified and observed_handler_pc
    print(f"source_owned_runtime_corridor={int(accepted)}")
    print("semantic_admission=blocked")
    return 0 if accepted else 1


if __name__ == "__main__":
    raise SystemExit(main())
