#!/usr/bin/env python3
"""Generate the reviewed CSB FM Towns JP object-index l10n bridge."""
from pathlib import Path
import ast

ROOT = Path(__file__).resolve().parent.parent
POT = ROOT / "po" / "csb.pot"
OUT = ROOT / "src" / "engine" / "csb_fmtowns_jp_object_l10n.inc"

def quoted(line):
    return ast.literal_eval(line[line.index('"'):])


def quote_c(value):
    """Return a C string literal with explicit control-byte escapes."""
    escaped = []
    for character in value:
        codepoint = ord(character)
        if character == "\\":
            escaped.append("\\\\")
        elif character == '"':
            escaped.append('\\"')
        elif codepoint < 0x20 or codepoint == 0x7f:
            escaped.append(f"\\{codepoint:03o}")
        else:
            escaped.append(character)
    return '"' + ''.join(escaped) + '"'


def main():
    names = []
    comments = []
    current = None
    for line in POT.read_text(encoding="utf-8").splitlines():
        if line.startswith("#."):
            comments.append(line)
        elif line.startswith("msgid "):
            current = quoted(line)
        elif line.startswith("msgstr"):
            if current and any("Source-owned M564 object name" in c for c in comments):
                names.append(current)
            comments = []
            current = None
    # The selected JP media has 177 M564 names; the final English-only name
    # has no Japanese counterpart. The first 177 are index-identical and are
    # reviewed against both original streams.
    if len(names) != 178:
        raise SystemExit(f"expected 178 English M564 names, got {len(names)}")
    out = ["/* Generated from reviewed po/csb.pot M564 order; do not edit. */",
           "/* The selected F31J M564 stream has 177 entries. */",
           "static const char *const m11_csb_fmtowns_jp_object_msgids[177] = {"]
    for name in names[:177]:
        out.append("    " + quote_c(name) + ",")
    out += ["};", ""]
    OUT.write_text("\n".join(out), encoding="utf-8")
    print(f"generated {OUT.relative_to(ROOT)}")

if __name__ == "__main__": main()
