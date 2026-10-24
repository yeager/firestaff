#!/usr/bin/env python3
"""Ensure broad translation updates preserve reviewed Indonesian Theron text."""

from __future__ import annotations

import importlib.util
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "po" / "translations_other.py"
SPEC = importlib.util.spec_from_file_location("translations_other", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
translations_other = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(translations_other)


PO_FIXTURE = '''msgid ""
msgstr ""
"Language: id\\n"

msgid "NORTH"
msgstr "UTARA"

msgid "DROP"
msgstr "JATUH"

msgid "USE"
msgstr "PAKAI"

msgid "FORWARD"
msgstr ""

msgid "WAIT"
msgstr ""
'''


def translation(catalog: str, key: str) -> str:
    import re

    match = re.search(
        rf'^msgid "{re.escape(key)}"\nmsgstr "(.*)"$',
        catalog,
        flags=re.MULTILINE,
    )
    if match is None:
        raise AssertionError(f"missing catalog entry: {key}")
    return match.group(1)


class TheronTranslationUpdateTests(unittest.TestCase):
    def test_unfiltered_update_preserves_indonesian_catalog_entries(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            po_dir = Path(temporary)
            catalog = po_dir / "theron.id.po"
            catalog.write_text(PO_FIXTURE, encoding="utf-8")
            environment = os.environ.copy()
            environment["FIRESTAFF_PO_DIR"] = str(po_dir)
            subprocess.run(
                [sys.executable, str(SCRIPT)],
                check=True,
                cwd=ROOT,
                env=environment,
                capture_output=True,
                text=True,
            )
            updated = catalog.read_text(encoding="utf-8")

        self.assertEqual(translation(updated, "NORTH"), "UTARA")
        self.assertEqual(translation(updated, "DROP"), "JATUH")
        self.assertEqual(translation(updated, "USE"), "PAKAI")
        self.assertEqual(translation(updated, "FORWARD"), "FORWARD")
        self.assertEqual(translation(updated, "WAIT"), "WAIT")

    def test_chinese_turn_and_side_step_captions_are_distinct_and_fit(self) -> None:
        captions = translations_other.THERON_MOUSE["zh"]
        self.assertNotEqual(captions["TURN L"], captions["STEP L"])
        self.assertNotEqual(captions["TURN R"], captions["STEP R"])
        self.assertLessEqual(len(captions["TURN L"]), 7)
        self.assertLessEqual(len(captions["TURN R"]), 7)
        self.assertLessEqual(len(captions["STEP L"]), 7)
        self.assertLessEqual(len(captions["STEP R"]), 7)


if __name__ == "__main__":
    unittest.main()
