#!/usr/bin/env python3
"""Write complete, deterministic gettext headers to Firestaff POT files."""
from __future__ import annotations

import argparse
from pathlib import Path

STAMP = "1970-01-01 00:00+0000"
CONTACT = "Firestaff Localization Team <daniel@danielnylander.se>"


def normalize(path: Path, project: str) -> None:
    lines = path.read_text(encoding="utf-8").splitlines()
    try:
        msgid_index = next(
            i for i in range(len(lines) - 1)
            if lines[i] == 'msgid ""' and lines[i + 1] == 'msgstr ""'
        )
    except StopIteration as exc:
        raise SystemExit(f"{path}: missing gettext header entry") from exc

    # xgettext marks its placeholder header fuzzy. A POT header is metadata,
    # not an uncertain translation, so remove only that adjacent flag.
    if msgid_index and lines[msgid_index - 1] == "#, fuzzy":
        del lines[msgid_index - 1]
        msgid_index -= 1

    end = msgid_index + 2
    while end < len(lines) and lines[end].startswith('"'):
        end += 1
    header = [
        'msgid ""',
        'msgstr ""',
        f'"Project-Id-Version: {project}\\n"',
        '"Report-Msgid-Bugs-To: daniel@danielnylander.se\\n"',
        f'"POT-Creation-Date: {STAMP}\\n"',
        f'"PO-Revision-Date: {STAMP}\\n"',
        f'"Last-Translator: {CONTACT}\\n"',
        f'"Language-Team: {CONTACT}\\n"',
        '"Language: en\\n"',
        '"MIME-Version: 1.0\\n"',
        '"Content-Type: text/plain; charset=UTF-8\\n"',
        '"Content-Transfer-Encoding: 8bit\\n"',
    ]
    path.write_text("\n".join(lines[:msgid_index] + header + lines[end:]) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="+", type=Path)
    args = parser.parse_args()
    for path in args.files:
        name = path.name.removesuffix(".pot").replace("_", "-")
        project = name if name == "firestaff" or name.startswith("firestaff-") else f"firestaff-{name}"
        normalize(path, project)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
