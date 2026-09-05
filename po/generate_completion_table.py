#!/usr/bin/env python3
"""Generate the checked-in per-language/per-domain PO completion matrix."""
from __future__ import annotations
import argparse
import ast
from pathlib import Path

DOMAINS = ("startup-menu", "firestaff", "dm1", "csb", "dm2", "nexus", "theron")
BEGIN = "<!-- completion-table:begin -->"
END = "<!-- completion-table:end -->"
NOTE = "¹ `N/A` means that the domain currently has no extracted source entries. It is a coverage gap, not 100% completion."

def quoted(line: str) -> str:
    return ast.literal_eval(line[line.index('"'):].strip())

def entries(path: Path) -> list[tuple[str, str, bool]]:
    result: list[tuple[str, str, bool]] = []
    msgid: list[str] | None = None
    msgstr: list[str] | None = None
    active: list[str] | None = None
    fuzzy = False
    def finish() -> None:
        nonlocal msgid, msgstr, active, fuzzy
        if msgid is not None and "".join(msgid): result.append(("".join(msgid), "".join(msgstr or []), fuzzy))
        msgid = msgstr = active = None
        fuzzy = False
    for raw in path.read_text(encoding="utf-8").splitlines():
        if raw.startswith("#, ") and "fuzzy" in raw[3:].split(", "): fuzzy = True
        elif raw.startswith("msgid "):
            entry_fuzzy = fuzzy
            finish(); fuzzy = entry_fuzzy
            msgid = [quoted(raw)]; msgstr = []; active = msgid
        elif raw.startswith("msgstr "):
            msgstr = [quoted(raw)]; active = msgstr
        elif raw.startswith('"') and active is not None: active.append(quoted(raw))
    finish()
    return result

def render(po_dir: Path) -> str:
    totals = {d: len(entries(po_dir / f"{d}.pot")) for d in DOMAINS}
    locales = sorted({p.name.rsplit(".", 2)[1] for d in DOMAINS for p in po_dir.glob(f"{d}.*.po")})
    lines = [BEGIN, "| Language | Launcher | Shared UI | DM1 | CSB | DM2 | Nexus | Theron |", "|---|---:|---:|---:|---:|---:|---:|---:|"]
    for locale in locales:
        cells = []
        for domain in DOMAINS:
            total = totals[domain]
            path = po_dir / f"{domain}.{locale}.po"
            if total == 0: cells.append("N/A¹")
            elif not path.exists(): cells.append("—")
            elif locale == "en": cells.append(f"source ({total}/{total})")
            else:
                translated = sum(bool(text) and not fuzzy for _, text, fuzzy in entries(path))
                cells.append(f"{translated}/{total} ({translated * 100 // total}%)")
        lines.append(f"| `{locale}` | " + " | ".join(cells) + " |")
    return "\n".join(lines + [END, "", NOTE])

def updated_readme(text: str, generated: str) -> str:
    """Return README text with exactly one current generated block."""
    if BEGIN in text and END in text:
        before, rest = text.split(BEGIN, 1); _, after = rest.split(END, 1)
        old_note = "\n\n" + NOTE
        if after.startswith(old_note): after = after[len(old_note):]
        text = before.rstrip() + "\n\n" + generated + after
    else:
        anchor = "## Validation"
        if anchor not in text: raise SystemExit(f"missing README anchor: {anchor}")
        text = text.replace(anchor, "## Translation completion\n\n" + generated + "\n\n" + anchor)
    return text.rstrip() + "\n"

def main() -> int:
    parser = argparse.ArgumentParser()
    default_po = Path(__file__).resolve().parent
    parser.add_argument("--po-dir", type=Path, default=default_po)
    parser.add_argument("--readme", type=Path)
    parser.add_argument("--check", action="store_true", help="fail instead of writing when the table is stale")
    args = parser.parse_args()
    readme = args.readme or args.po_dir / "README.md"
    text = readme.read_text(encoding="utf-8")
    generated = render(args.po_dir)
    updated = updated_readme(text, generated)
    if args.check:
        if updated != text:
            print(f"stale completion table: {readme}")
            return 1
        print(f"PASS: completion table is current: {readme}")
    else:
        readme.write_text(updated, encoding="utf-8")
    return 0

if __name__ == "__main__": raise SystemExit(main())
