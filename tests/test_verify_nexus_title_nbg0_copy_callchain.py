#!/usr/bin/env python3
"""Source-level guard for the full-member Nexus SH-2 call-chain verifier."""

from pathlib import Path


source = (Path(__file__).resolve().parents[1] /
          "scripts/verify_nexus_title_nbg0_copy_callchain.py").read_text()
required = (
    "DM_BASE = 0x06010040",
    "COPY_PC = 0x0602312C",
    "COPY_ENTRY = 0x06023112",
    "COPY_RTS = 0x06023144",
    "OUTER_ENTRY = 0x060230C0",
    "CALL_SITE = 0x06022772",
    "RETURN_ADDRESS = CALL_SITE + 4",
    "TITLE_FRAME = 12596",
    "COPY_BYTES = 31616",
    "for offset in range(0, len(data) - 1, 2):",
    "opcode >> 12 != 0xB",
    "copy row has no PR return-address witness",
    "copy rows have inconsistent live PR values",
    "expected {COPY_BYTES} frame-{TITLE_FRAME} copy rows",
    "--static-only",
    "title_nbg0_copy_callchain=verified",
    "title_nbg0_copy_live_pr",
    "semantic_admission=blocked",
)
missing = [item for item in required if item not in source]
if missing:
    raise SystemExit("missing call-chain invariant: " + ", ".join(missing))
print("ok: Nexus title copier full-member call-chain verifier is source-locked")
