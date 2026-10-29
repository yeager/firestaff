#!/usr/bin/env python3
"""Exercise the bounded decoder-output/read-candidate join with test traces."""

from __future__ import annotations

import hashlib
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
VERIFIER = ROOT / "scripts/verify_theron_decoder_output_read_candidates.py"
FIRST = 0x0D0000
LENGTH = 5064


class DecoderOutputReadCandidatesTest(unittest.TestCase):
    def test_temporal_byte_match_is_reported_without_ownership_claim(self) -> None:
        with tempfile.TemporaryDirectory(prefix="theron-output-read-", dir=ROOT / ".codex-scratch") as tmp:
            root = Path(tmp)
            payload = bytes(LENGTH)
            digest = hashlib.sha256(payload).hexdigest()
            decoder = root / "decoder.trace"
            consumer = root / "consumer.trace"
            with decoder.open("w", encoding="utf-8") as out:
                for offset, value in enumerate(payload):
                    out.write(
                        f"event_sequence={1000 + offset} phase=decoder kind=mem_write "
                        f"physical={FIRST + offset:06x} value={value:02x}\n"
                    )
                out.write(
                    f"event_sequence={1000 + LENGTH} phase=decoder kind=return_rts "
                    "pc=00003c29\n"
                )
            consumer.write_text(
                "source=mednafen-pce-fast-main-ram-consumer-read "
                "event_sequence_source=TheronTraceCDSharedEventSequence\n"
                "main_ram_consumer_read sequence=0 event_sequence=7000 "
                "physical_address=0d0000 value=00 reader_physical_pc=001425 "
                "reader_code_bytes=4444444444444444\n"
                "main_ram_consumer_read sequence=1 event_sequence=999 "
                "physical_address=0d0001 value=00 reader_physical_pc=001425 "
                "reader_code_bytes=4444444444444444\n"
                "main_ram_consumer_read sequence=2 event_sequence=7001 "
                "physical_address=0d0002 value=ff reader_physical_pc=001425 "
                "reader_code_bytes=4444444444444444\n",
                encoding="utf-8",
            )
            result = subprocess.run(
                [
                    sys.executable,
                    str(VERIFIER),
                    str(decoder),
                    str(consumer),
                    "--require-candidate",
                    "--expected-output-sha256",
                    digest,
                ],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("post_decode_reads_in_output_range=2", result.stdout)
            self.assertIn("unique_output_addresses_read=2", result.stdout)
            self.assertIn("unique_byte_equal_output_addresses=1", result.stdout)
            self.assertIn("physical_pc=001425", result.stdout)
            self.assertIn("ownership and gameplay use unproven", result.stdout)

    def test_missing_candidate_fails_when_required(self) -> None:
        with tempfile.TemporaryDirectory(prefix="theron-output-read-empty-", dir=ROOT / ".codex-scratch") as tmp:
            root = Path(tmp)
            payload = bytes(LENGTH)
            digest = hashlib.sha256(payload).hexdigest()
            decoder = root / "decoder.trace"
            consumer = root / "consumer.trace"
            with decoder.open("w", encoding="utf-8") as out:
                for offset, value in enumerate(payload):
                    out.write(
                        f"event_sequence={1000 + offset} phase=decoder kind=mem_write "
                        f"physical={FIRST + offset:06x} value={value:02x}\n"
                    )
                out.write(
                    f"event_sequence={1000 + LENGTH} phase=decoder kind=return_rts "
                    "pc=00003c29\n"
                )
            consumer.write_text(
                "source=mednafen-pce-fast-main-ram-consumer-read "
                "event_sequence_source=TheronTraceCDSharedEventSequence\n",
                encoding="utf-8",
            )
            result = subprocess.run(
                [
                    sys.executable,
                    str(VERIFIER),
                    str(decoder),
                    str(consumer),
                    "--require-candidate",
                    "--expected-output-sha256",
                    digest,
                ],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(result.returncode, 1)
            self.assertIn("no post-decode byte-equal output read candidate", result.stderr)

    def test_legacy_consumer_trace_is_rejected_as_unobservable(self) -> None:
        with tempfile.TemporaryDirectory(prefix="theron-output-read-legacy-", dir=ROOT / ".codex-scratch") as tmp:
            root = Path(tmp)
            payload = bytes(LENGTH)
            digest = hashlib.sha256(payload).hexdigest()
            decoder = root / "decoder.trace"
            consumer = root / "consumer.trace"
            with decoder.open("w", encoding="utf-8") as out:
                for offset, value in enumerate(payload):
                    out.write(
                        f"event_sequence={1000 + offset} phase=decoder kind=mem_write "
                        f"physical={FIRST + offset:06x} value={value:02x}\n"
                    )
                out.write(
                    f"event_sequence={1000 + LENGTH} phase=decoder kind=return_rts "
                    "pc=00003c29\n"
                )
            consumer.write_text(
                "source=mednafen-pce-fast-main-ram-consumer-read\n",
                encoding="utf-8",
            )
            result = subprocess.run(
                [
                    sys.executable,
                    str(VERIFIER),
                    str(decoder),
                    str(consumer),
                    "--expected-output-sha256",
                    digest,
                ],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(result.returncode, 1)
            self.assertIn("lacks the shared event-sequence header", result.stderr)


if __name__ == "__main__":
    unittest.main()
