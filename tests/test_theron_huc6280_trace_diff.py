#!/usr/bin/env python3
"""Test the trace comparator with artificial tool-format fixtures only.

These records exercise validation and diff behavior. They are not Theron game
data, emulator captures, CPU evidence, or a parity fixture.
"""

from __future__ import annotations

import json
import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "tools/theron/compare_huc6280_traces.py"
HASH_A = "a" * 64
HASH_B = "b" * 64


class HuC6280TraceDiffTests(unittest.TestCase):
    def setUp(self) -> None:
        scratch = ROOT / ".codex-scratch"
        scratch.mkdir(exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(
            prefix="theron-huc6280-trace-format-", dir=scratch
        )
        self.directory = Path(self.temp.name)
        self.reference = self.directory / "reference.jsonl"
        self.lift = self.directory / "lift.jsonl"
        self.header_common = {
            "schema": "firestaff.theron.huc6280_trace.v1",
            "evidence": "tool-fixture",
            "media": {
                "edition": "tool-format-only",
                "sha256": HASH_A,
                "authenticated": False,
            },
            "input_sha256": HASH_B,
        }
        self.initial = {
            "pc": 0x2390,
            "a": 0,
            "x": 0,
            "y": 0,
            "s": 0xFD,
            "p": 0x24,
            "mpr": [0xFF, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x3F],
            "memory_sha256": HASH_A,
        }
        self.events = [
            {"seq": 0, "kind": "fetch", "pc": 0x2390,
             "logical": 0x2390, "mpr_slot": 1, "bank": 0x1F,
             "bank_offset": 0x390, "value": 0x44},
            {"seq": 1, "kind": "mpr", "pc": 0x2459,
             "slot": 1, "old": 0x1F, "new": 0x7A},
            {"seq": 2, "kind": "read", "pc": 0x2496,
             "logical": 0x2000, "mpr_slot": 1, "bank": 0x7A,
             "bank_offset": 0, "value": 0x33},
        ]
        self.final = {
            "termination": "return_2392",
            "truncated": False,
            "state": {
                **self.initial,
                "pc": 0x2392,
                "mpr": [0xFF, 0x7A, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x3F],
            },
            "changed_memory": [[0x3100, 0x5A]],
        }
        self._write_pair()

    def tearDown(self) -> None:
        self.temp.cleanup()

    def _records(self, role: str, events=None, final=None, **header_overrides):
        header = {
            "record": "header",
            **self.header_common,
            "role": role,
            "source_sha256": HASH_A if role == "reference" else HASH_B,
            **header_overrides,
        }
        records = [
            header,
            {"record": "initial", "state": self.initial},
            *({"record": "event", "event": event}
              for event in self.events if events is None),
            *({"record": "event", "event": event} for event in (events or [])),
            {"record": "final", **(final or self.final)},
        ]
        return records

    def _write_pair(self, *, ref_events=None, lift_events=None,
                    ref_final=None, lift_final=None, lift_initial=None,
                    lift_header_overrides=None):
        left = self._records("reference", events=ref_events, final=ref_final)
        right = self._records("lift", events=lift_events, final=lift_final,
                              **(lift_header_overrides or {}))
        if lift_initial is not None:
            right[1]["state"] = lift_initial
        for path, records in ((self.reference, left), (self.lift, right)):
            path.write_text("".join(json.dumps(record) + "\n" for record in records),
                            encoding="utf-8")

    def _run(self):
        return subprocess.run(
            ["python3", str(SCRIPT), str(self.reference), str(self.lift)],
            capture_output=True,
            text=True,
            check=False,
        )

    def test_matching_tool_fixtures_are_never_called_parity(self):
        result = self._run()
        self.assertEqual(result.returncode, 0, result.stderr + result.stdout)
        self.assertIn("evidence_class=tool-fixture", result.stdout)
        self.assertIn("execution_comparison=matching_observations", result.stdout)
        self.assertIn("parity_claim=not-established", result.stdout)

    def test_reports_first_ordered_event_divergence(self):
        changed = [dict(event) for event in self.events]
        changed[2]["value"] = 0x34
        self._write_pair(lift_events=changed)
        result = self._run()
        self.assertEqual(result.returncode, 1)
        self.assertIn("first event divergence at seq=2", result.stdout)

    def test_requires_identical_initial_state(self):
        changed = dict(self.initial)
        changed["a"] = 1
        self._write_pair(lift_initial=changed)
        result = self._run()
        self.assertEqual(result.returncode, 1)
        self.assertIn("initial.a", result.stdout)

    def test_reports_named_final_register_and_memory_differences(self):
        changed = json.loads(json.dumps(self.final))
        changed["state"]["x"] = 3
        changed["changed_memory"] = [[0x3100, 0x5B], [0x3101, 0x99]]
        self._write_pair(lift_final=changed)
        result = self._run()
        self.assertEqual(result.returncode, 1)
        self.assertIn("final.x", result.stdout)
        self.assertIn("changed_memory[0x3100]", result.stdout)
        self.assertIn("changed_memory[0x3101]", result.stdout)

    def test_rejects_truncated_or_non_returning_reference(self):
        changed = json.loads(json.dumps(self.final))
        changed["truncated"] = True
        self._write_pair(ref_final=changed)
        result = self._run()
        self.assertEqual(result.returncode, 2)
        self.assertIn("truncated trace is not comparable", result.stderr)

    def test_rejects_authentic_label_without_receipt(self):
        self._write_pair(lift_header_overrides={"evidence": "authentic-media"})
        result = self._run()
        self.assertEqual(result.returncode, 2)
        self.assertIn("media.authenticated=true", result.stderr)

    def test_rejects_wrong_resolved_mpr_slot(self):
        changed = [dict(self.events[0]), dict(self.events[1])]
        changed[0]["mpr_slot"] = 2
        self._write_pair(ref_events=changed)
        result = self._run()
        self.assertEqual(result.returncode, 2)
        self.assertIn("does not map its logical address", result.stderr)

    def test_rejects_memory_event_that_uses_stale_mpr_bank(self):
        changed = [dict(self.events[0]), dict(self.events[1]),
                   {"seq": 2, "kind": "read", "pc": 0x245A,
                    "logical": 0x2000, "mpr_slot": 1, "bank": 0x1F,
                    "bank_offset": 0, "value": 0x33}]
        self._write_pair(ref_events=changed)
        result = self._run()
        self.assertEqual(result.returncode, 2)
        self.assertIn("bank does not match live MPR1", result.stderr)


if __name__ == "__main__":
    unittest.main()
