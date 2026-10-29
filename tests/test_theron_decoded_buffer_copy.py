#!/usr/bin/env python3
"""Exercise shared-sequence decoder-buffer copy verification."""

from __future__ import annotations

import hashlib
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
VERIFIER = ROOT / "scripts/verify_theron_decoded_buffer_copy.py"
FIRST = 0x0D0000
LENGTH = 5064


class DecodedBufferCopyTest(unittest.TestCase):
    def setUp(self) -> None:
        (ROOT / ".codex-scratch").mkdir(exist_ok=True)

    def make_decoder(self, path: Path, payload: bytes) -> tuple[str, int]:
        with path.open("w", encoding="utf-8") as out:
            for offset, value in enumerate(payload):
                out.write(
                    f"event_sequence={1000 + offset} phase=decoder kind=mem_write "
                    f"physical={FIRST + offset:06x} value={value:02x}\n"
                )
            final_write = 1000 + LENGTH - 1
            out.write(
                f"event_sequence={final_write + 1} phase=decoder kind=return_rts "
                "pc=00003c29\n"
            )
        return hashlib.sha256(payload).hexdigest(), final_write

    def test_only_immediate_equal_following_write_is_counted_as_copy(self) -> None:
        payload = bytes(LENGTH)
        with tempfile.TemporaryDirectory(
            prefix="theron-decoded-copy-", dir=ROOT / ".codex-scratch"
        ) as tmp:
            root = Path(tmp)
            decoder = root / "decoder.trace"
            consumer = root / "consumer.trace"
            target_write = root / "target-write.trace"
            digest, final_write = self.make_decoder(decoder, payload)
            consumer.write_text(
                "source=mednafen-pce-fast-main-ram-consumer-read "
                "event_sequence_source=TheronTraceCDSharedEventSequence\n"
                "main_ram_consumer_read sequence=0 event_sequence=6070 "
                "physical_address=0d0000 value=00 reader_physical_pc=1042ab "
                "reader_code_bytes=b1009de047c8e8b1\n"
                "main_ram_consumer_read sequence=1 event_sequence=6072 "
                "physical_address=0d0001 value=00 reader_physical_pc=1042ab "
                "reader_code_bytes=b1009de047c8e8b1\n"
                "main_ram_consumer_read sequence=2 event_sequence=6074 "
                "physical_address=0d0002 value=00 reader_physical_pc=1042ab "
                "reader_code_bytes=b1009de047c8e8b1\n",
                encoding="utf-8",
            )
            target_write.write_text(
                "source=mednafen-pce-fast-main-ram-target-write "
                "event_sequence_source=TheronTraceCDSharedEventSequence\n"
                "main_ram_target_write sequence=0 event_sequence=6071 "
                "logical_address=47e0 physical_address=1047e0 value=00 "
                "writer_physical_pc=1042ad writer_code_bytes=9de047c8e8b1009d mpr2=82\n"
                "main_ram_target_write sequence=1 event_sequence=6073 "
                "logical_address=47e1 physical_address=1047e1 value=01 "
                "writer_physical_pc=1042ad writer_code_bytes=9de047c8e8b1009d mpr2=82\n"
                "main_ram_target_write sequence=2 event_sequence=6075 "
                "logical_address=47e2 physical_address=1047e2 value=00 "
                "writer_physical_pc=1042ae writer_code_bytes=9de047c8e8b1009d mpr2=82\n",
                encoding="utf-8",
            )
            result = subprocess.run(
                [
                    sys.executable,
                    str(VERIFIER),
                    str(decoder),
                    str(consumer),
                    str(target_write),
                    "--expected-output-sha256",
                    digest,
                    "--require-copy",
                ],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn(f"decoder_last_output_write_event_sequence={final_write}", result.stdout)
            self.assertIn("post_decode_source_reads_at_1042ab=3", result.stdout)
            self.assertIn("immediate_shared_sequence_byte_equal_copy_pairs=1", result.stdout)
            self.assertIn("destination_mpr2_values=82", result.stdout)
            self.assertIn("gameplay meaning and rendered effect unproven", result.stdout)

    def test_truncated_target_trace_is_rejected(self) -> None:
        payload = bytes(LENGTH)
        with tempfile.TemporaryDirectory(
            prefix="theron-decoded-copy-truncated-", dir=ROOT / ".codex-scratch"
        ) as tmp:
            root = Path(tmp)
            decoder = root / "decoder.trace"
            consumer = root / "consumer.trace"
            target_write = root / "target-write.trace"
            digest, _ = self.make_decoder(decoder, payload)
            consumer.write_text(
                "source=mednafen-pce-fast-main-ram-consumer-read "
                "event_sequence_source=TheronTraceCDSharedEventSequence\n",
                encoding="utf-8",
            )
            target_write.write_text(
                "source=mednafen-pce-fast-main-ram-target-write "
                "event_sequence_source=TheronTraceCDSharedEventSequence\n"
                "main_ram_target_write_truncated=sample_limit limit=4096\n",
                encoding="utf-8",
            )
            result = subprocess.run(
                [
                    sys.executable,
                    str(VERIFIER),
                    str(decoder),
                    str(consumer),
                    str(target_write),
                    "--expected-output-sha256",
                    digest,
                ],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(result.returncode, 1)
            self.assertIn("target-write trace is truncated", result.stderr)


if __name__ == "__main__":
    unittest.main()
