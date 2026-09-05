#!/usr/bin/env python3
"""Verify explicitly admitted DM1 M11 presentation literals and Swedish PO coverage."""

import ast
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src/engine/m11_game_view.c"
POT = ROOT / "po/dm1.pot"
SV = ROOT / "po/dm1.sv.po"


def parse_po(path: Path) -> dict[str, str]:
    entries: dict[str, str] = {}
    current_id = None
    current_str = None
    mode = None
    for raw in path.read_text(encoding="utf-8").splitlines() + [""]:
        if raw.startswith("msgid "):
            if current_id:
                entries[current_id] = current_str or ""
            current_id = ast.literal_eval(raw[6:])
            current_str = ""
            mode = "id"
        elif raw.startswith("msgstr "):
            current_str = ast.literal_eval(raw[7:])
            mode = "str"
        elif raw.startswith('"'):
            value = ast.literal_eval(raw)
            if mode == "id" and current_id is not None:
                current_id += value
            elif mode == "str" and current_str is not None:
                current_str += value
        elif not raw and current_id:
            entries[current_id] = current_str or ""
            current_id = None
            current_str = None
            mode = None
    return entries


def main() -> int:
    source = SOURCE.read_text(encoding="utf-8")
    marked = sorted(set(re.findall(
        r'M11_DM1_PRESENTED\(\s*"((?:[^"\\]|\\.)*)"\s*\)', source)))
    marked = [ast.literal_eval('"' + value + '"') for value in marked]
    pot = parse_po(POT)
    sv = parse_po(SV)
    errors = []
    if not marked:
        errors.append("no M11_DM1_PRESENTED literals found")
    for msgid in marked:
        if msgid not in pot:
            errors.append(f"dm1.pot missing marked runtime msgid: {msgid!r}")
        if not sv.get(msgid):
            errors.append(f"dm1.sv.po missing nonblank translation: {msgid!r}")
    required_source = (
        'return fs_po_gettext_in_domain(m11_po_domain_for_state(state), sourceText);',
        'case M11_GAME_SOURCE_CSB_BOOT:',
        'return "csb";',
        'case M11_GAME_SOURCE_DM2_BOOT:',
        'return "dm2";',
        'case M11_GAME_SOURCE_NEXUS_DGN:',
        'return "nexus";',
        'case M11_GAME_SOURCE_THERON_TRACK02:',
        'return "theron";',
        'm11_translate_for_state(state, levelName)',
        'm11_translate_for_state(state, skillNames[i])',
        'm11_translate_for_state(state, statNames[i])',
        'fs_po_gettext_in_domain("dm1", dm1PresentedNames[creatureType])',
        'const char* sourceMessage = &decoded[2];',
        'snprintf(presented, sizeof(presented), "\\n%s",',
    )
    for token in required_source:
        if token not in source:
            errors.append(f"runtime domain boundary missing: {token}")
    if errors:
        for error in errors:
            print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print(f"PASS: {len(marked)} explicitly admitted DM1 M11 literals; "
          f"Swedish {len(marked)}/{len(marked)}; domain boundary locked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
