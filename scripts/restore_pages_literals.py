#!/usr/bin/env python3
"""Restore literal Liquid endraw examples after Jekyll builds the site."""

from __future__ import annotations

import argparse
from pathlib import Path
import re


TOKEN = re.compile(r"__FIRESTAFF_LIQUID_LITERAL_([0-9a-f]+)__")


def restore(site: Path) -> int:
    restored = 0
    for page in site.rglob("*.html"):
        original = page.read_text()

        def decode(match: re.Match[str]) -> str:
            nonlocal restored
            restored += 1
            return bytes.fromhex(match.group(1)).decode("utf-8")

        updated = TOKEN.sub(decode, original)
        if updated != original:
            page.write_text(updated)
    return restored


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--site", type=Path, default=Path("_site"))
    args = parser.parse_args()
    site = args.site.resolve()
    if not site.is_dir():
        raise SystemExit(f"Built Pages site directory not found: {site}")
    restored = restore(site)
    print(f"Restored {restored} literal Liquid examples.")


if __name__ == "__main__":
    main()
