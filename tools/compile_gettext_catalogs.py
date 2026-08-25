#!/usr/bin/env python3
"""Compile Firestaff gettext PO catalogs into build-tree MO files.

The repository intentionally tracks human-editable .po/.pot sources only.
This stdlib-only compiler keeps a native build independent of a host msgfmt
installation and writes every generated .mo below the requested build tree.
"""

from __future__ import annotations

import argparse
import ast
import gettext
import struct
from pathlib import Path


def _quoted(value: str) -> str:
    return ast.literal_eval(value.strip())


def parse_po(path: Path) -> dict[str, str]:
    entries: dict[str, str] = {}
    flags: set[str] = set()
    context: str | None = None
    msgid: str | None = None
    plural: str | None = None
    translations: dict[int, str] = {}
    active: tuple[str, int | None] | None = None

    def flush() -> None:
        nonlocal flags, context, msgid, plural, translations, active
        if msgid is not None and "fuzzy" not in flags:
            key = msgid if context is None else f"{context}\x04{msgid}"
            if plural is not None:
                key = f"{key}\0{plural}"
                value = "\0".join(translations[index]
                                  for index in sorted(translations))
            else:
                value = translations.get(0, "")
            entries[key] = value
        flags = set()
        context = None
        msgid = None
        plural = None
        translations = {}
        active = None

    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line:
            flush()
        elif line.startswith("#,"):
            flags.update(part.strip() for part in line[2:].split(","))
        elif line.startswith("#"):
            continue
        elif line.startswith("msgctxt "):
            context = _quoted(line[8:])
            active = ("context", None)
        elif line.startswith("msgid_plural "):
            plural = _quoted(line[13:])
            active = ("plural", None)
        elif line.startswith("msgid "):
            msgid = _quoted(line[6:])
            active = ("id", None)
        elif line.startswith("msgstr["):
            end = line.index("]")
            index = int(line[7:end])
            translations[index] = _quoted(line[end + 1:].strip())
            active = ("str", index)
        elif line.startswith("msgstr "):
            translations[0] = _quoted(line[7:])
            active = ("str", 0)
        elif line.startswith('"') and active is not None:
            suffix = _quoted(line)
            kind, index = active
            if kind == "context":
                context = (context or "") + suffix
            elif kind == "plural":
                plural = (plural or "") + suffix
            elif kind == "id":
                msgid = (msgid or "") + suffix
            else:
                translations[index or 0] = translations.get(index or 0, "") + suffix
        else:
            raise ValueError(f"{path}: unsupported PO line: {raw}")
    flush()
    return entries


def write_mo(entries: dict[str, str], output: Path) -> None:
    ordered = sorted(entries.items())
    ids = b"\0".join(key.encode("utf-8") for key, _ in ordered) + b"\0"
    strings = b"\0".join(value.encode("utf-8") for _, value in ordered) + b"\0"
    count = len(ordered)
    original_table = 28
    translation_table = original_table + count * 8
    ids_offset = translation_table + count * 8
    strings_offset = ids_offset + len(ids)
    header = struct.pack("<7I", 0x950412DE, 0, count, original_table,
                         translation_table, 0, strings_offset + len(strings))
    original_entries: list[bytes] = []
    translated_entries: list[bytes] = []
    offset = ids_offset
    for key, _ in ordered:
        encoded = key.encode("utf-8")
        original_entries.append(struct.pack("<2I", len(encoded), offset))
        offset += len(encoded) + 1
    offset = strings_offset
    for _, value in ordered:
        encoded = value.encode("utf-8")
        translated_entries.append(struct.pack("<2I", len(encoded), offset))
        offset += len(encoded) + 1
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(header + b"".join(original_entries) +
                       b"".join(translated_entries) + ids + strings)


def studio_source_ids(template: Path) -> set[str]:
    """Return literal gettext ids used by the shipped Studio applications."""
    root = template.parent.parent
    scripts = (
        root / "scripts" / "firestaff_artpack_studio.py",
        root / "scripts" / "firestaff_dungeon_studio.py",
        root / "scripts" / "firestaff_savegame_editor.py",
    )
    ids: set[str] = set()
    for script in scripts:
        tree = ast.parse(script.read_text(encoding="utf-8"), filename=str(script))
        for node in ast.walk(tree):
            if (isinstance(node, ast.Call) and isinstance(node.func, ast.Name) and
                    node.func.id == "_" and node.args and
                    isinstance(node.args[0], ast.Constant) and
                    isinstance(node.args[0].value, str)):
                ids.add(node.args[0].value)
    return ids


def verify_template(template: Path, english: Path) -> None:
    source = parse_po(english)
    template_entries = parse_po(template)
    source_ids = set(source) - {""}
    template_ids = set(template_entries) - {""}
    if source_ids != template_ids:
        missing = sorted(source_ids - template_ids)
        extra = sorted(template_ids - source_ids)
        raise SystemExit(
            f"template mismatch: missing={missing[:3]} extra={extra[:3]}")
    nonempty = [key for key, value in template_entries.items()
                if key and value]
    if nonempty:
        raise SystemExit(f"template has translated strings: {nonempty[:3]}")
    source_ids = studio_source_ids(template)
    if source_ids != template_ids:
        missing = sorted(source_ids - template_ids)
        extra = sorted(template_ids - source_ids)
        raise SystemExit(
            f"template source coverage mismatch: missing={missing[:3]} extra={extra[:3]}")


def compile_catalogs(source_dir: Path, output_dir: Path) -> int:
    count = 0
    for po in sorted((source_dir / "studio").glob("*.po")):
        lang = po.stem
        output = output_dir / "locale" / lang / "LC_MESSAGES" / "firestaff_studio.mo"
        entries = parse_po(po)
        write_mo(entries, output)
        with output.open("rb") as handle:
            gettext.GNUTranslations(handle)
        count += 1
    # Keep legacy flat catalogs available as build outputs too.  The native
    # launcher reads its PO source directly; these are for gettext consumers.
    for po in sorted(source_dir.glob("??.po")):
        entries = parse_po(po)
        write_mo(entries, output_dir / f"{po.stem}.mo")
        count += 1
    return count


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-dir", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--verify-template", action="store_true")
    args = parser.parse_args()
    if args.verify_template:
        verify_template(args.source_dir / "firestaff_studio.pot",
                        args.source_dir / "studio" / "en.po")
    count = compile_catalogs(args.source_dir, args.output_dir)
    print(f"gettext catalogs: generated {count} .mo files in {args.output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
