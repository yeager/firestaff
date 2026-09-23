#!/usr/bin/env python3
"""Write complete, deterministic gettext headers to Firestaff POT files."""
from __future__ import annotations

import argparse
import ast
import string
from pathlib import Path

STAMP = "1970-01-01 00:00+0000"
CONTACT = "Firestaff Localization Team <daniel@danielnylander.se>"


def normalize_python_brace_flags(lines: list[str]) -> list[str]:
    """Keep Python format flags stable across gettext versions.

    Newer xgettext releases infer ``python-brace-format`` automatically while
    Ubuntu 24.04's xgettext does not. Infer the flag from each Python msgid so
    generated catalogs remain identical across platforms.
    """
    formatter = string.Formatter()
    index = 0
    while index < len(lines):
        if not lines[index].startswith("msgid "):
            index += 1
            continue

        fragments = [lines[index][len("msgid "):]]
        cursor = index + 1
        while cursor < len(lines) and not lines[cursor].startswith("msgstr "):
            if lines[cursor].startswith('"'):
                fragments.append(lines[cursor])
            cursor += 1
        try:
            msgid = "".join(ast.literal_eval(fragment) for fragment in fragments)
            has_brace_fields = any(
                field_name is not None
                for _, field_name, _, _ in formatter.parse(msgid)
            )
        except (SyntaxError, ValueError):
            has_brace_fields = False

        if has_brace_fields:
            block_start = index
            while block_start > 0 and lines[block_start - 1] != "":
                block_start -= 1
            flag_index = next(
                (i for i in range(block_start, index)
                 if lines[i].startswith("#, ")),
                None,
            )
            if flag_index is None:
                lines.insert(index, "#, python-brace-format")
                index += 1
                cursor += 1
            elif "python-brace-format" not in lines[flag_index].split(", "):
                lines[flag_index] += ", python-brace-format"
        index = cursor + 1
    return lines


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
    normalized = lines[:msgid_index] + header + lines[end:]
    if project == "firestaff-studio":
        normalized = normalize_python_brace_flags(normalized)
    path.write_text("\n".join(normalized) + "\n", encoding="utf-8")


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
