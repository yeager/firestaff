#!/usr/bin/env python3
"""Regression test for build-tree gettext catalog generation."""
from __future__ import annotations

import gettext
import importlib.util
import shutil
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
WORK = ROOT / "build" / "test-gettext-catalog-compiler"
MODULE_PATH = ROOT / "tools" / "compile_gettext_catalogs.py"


def load_compiler():
    spec = importlib.util.spec_from_file_location("compile_gettext_catalogs", MODULE_PATH)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def write_catalog(path: Path, language: str, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        'msgid ""\n'
        'msgstr ""\n'
        f'"Language: {language}\\n"\n'
        '"Content-Type: text/plain; charset=UTF-8\\n"\n'
        '\n'
        'msgid "Open"\n'
        f'msgstr "{text}"\n',
        encoding="utf-8",
    )


def main() -> int:
    shutil.rmtree(WORK, ignore_errors=True)
    try:
        source = WORK / "source"
        output = WORK / "output"
        write_catalog(source / "studio" / "sv.po", "sv", "Öppna")
        stale = output / "locale" / "fr" / "LC_MESSAGES" / "firestaff_studio.mo"
        stale.parent.mkdir(parents=True, exist_ok=True)
        stale.write_bytes(b"stale")
        unrelated = output / "locale" / "fr" / "LC_MESSAGES" / "another.mo"
        unrelated.write_bytes(b"keep")

        compiler = load_compiler()
        assert compiler.compile_catalogs(source, output) == 1

        generated = output / "locale" / "sv" / "LC_MESSAGES" / "firestaff_studio.mo"
        assert generated.is_file()
        with generated.open("rb") as handle:
            translations = gettext.GNUTranslations(handle)
        assert translations.gettext("Open") == "Öppna"
        assert not stale.exists(), "removed PO left a stale domain catalog"
        assert unrelated.read_bytes() == b"keep", "cleanup crossed gettext domains"
    finally:
        shutil.rmtree(WORK, ignore_errors=True)
    print("PASS: gettext catalogs rebuild cleanly without stale domain output")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
