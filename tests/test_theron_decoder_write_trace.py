#!/usr/bin/env python3
"""Exercise exact decoder/code-window write trace joining."""

from __future__ import annotations

import hashlib
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
VERIFIER = ROOT / "scripts/verify_theron_decoder_write_trace.py"
FIRST = 0x0D0000
LENGTH = 5064
CODE_HEADER = (
    "source=mednafen-pce-fast-code-ram-write "
    "event_sequence_source=TheronTraceCDSharedEventSequence\n"
)


class DecoderCodeWriteTraceTest(unittest.TestCase):
    def setUp(self) -> None:
        (ROOT / ".codex-scratch").mkdir(exist_ok=True)

    def make_decoder(self, path: Path, payload: bytes) -> tuple[str, list[tuple[int, int, int]]]:
        decoder_writes = []
        with path.open("w", encoding="utf-8") as out:
            for offset, value in enumerate(payload):
                sequence = 1000 + offset
                address = FIRST + offset
                decoder_writes.append((sequence, address, value))
                out.write(
                    f"event_sequence={sequence} phase=decoder kind=mem_write "
                    f"physical={address:06x} value={value:02x}\n"
                )
            out.write(
                f"event_sequence={1000 + LENGTH} phase=decoder kind=return_rts "
                "pc=00003c29\n"
            )
        return hashlib.sha256(payload).hexdigest(), decoder_writes

    def make_code_trace(
        self, path: Path, decoder_writes: list[tuple[int, int, int]], omit_offset: int | None = None
    ) -> None:
        with path.open("w", encoding="utf-8") as out:
            out.write(CODE_HEADER)
            for offset, (sequence, address, value) in enumerate(decoder_writes):
                if offset == omit_offset:
                    continue
                out.write(
                    f"pce_fast_code_ram_write sequence={offset} event_sequence={sequence + 1} "
                    f"physical_address={address:06x} value={value:02x} "
                    "writer_physical_pc=1f1d1d\n"
                )
            out.write(
                f"pce_fast_code_ram_write sequence={LENGTH} event_sequence={1001 + LENGTH} "
                f"physical_address={FIRST:06x} value=ff writer_physical_pc=1f1d1d\n"
            )

    def run_verifier(self, decoder: Path, code: Path, digest: str) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            [
                sys.executable,
                str(VERIFIER),
                str(decoder),
                str(code),
                "--expected-output-sha256",
                digest,
                "--require-complete",
            ],
            text=True,
            capture_output=True,
            check=False,
        )

    def test_complete_output_write_coverage_separates_post_return_writes(self) -> None:
        payload = bytes(LENGTH)
        with tempfile.TemporaryDirectory(
            prefix="theron-decoder-write-", dir=ROOT / ".codex-scratch"
        ) as tmp:
            root = Path(tmp)
            decoder = root / "decoder.trace"
            code = root / "code-write.trace"
            digest, decoder_writes = self.make_decoder(decoder, payload)
            self.make_code_trace(code, decoder_writes)
            result = self.run_verifier(decoder, code, digest)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("exact_decoder_write_matches=5064", result.stdout)
            self.assertIn("decoder_output_writes_without_exact_code_trace_match=0", result.stdout)
            self.assertIn("unmatched_code_window_writes_after_decoder_return=1", result.stdout)
            self.assertIn("decoder_writer_physical_pc=1f1d1d exact_matches=5064", result.stdout)

    def test_missing_decoder_write_match_fails_when_complete_coverage_is_required(self) -> None:
        payload = bytes(LENGTH)
        with tempfile.TemporaryDirectory(
            prefix="theron-decoder-write-missing-", dir=ROOT / ".codex-scratch"
        ) as tmp:
            root = Path(tmp)
            decoder = root / "decoder.trace"
            code = root / "code-write.trace"
            digest, decoder_writes = self.make_decoder(decoder, payload)
            self.make_code_trace(code, decoder_writes, omit_offset=17)
            result = self.run_verifier(decoder, code, digest)
            self.assertEqual(result.returncode, 1)
            self.assertIn("exact_decoder_write_matches=5063", result.stdout)
            self.assertIn("decoder_output_writes_without_exact_code_trace_match=1", result.stdout)


if __name__ == "__main__":
    unittest.main()
