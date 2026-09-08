#!/usr/bin/env python3
"""Translate empty Firestaff PO entries while preserving runtime tokens.

This is an editor aid, not a build step.  It only fills entries whose
translation is empty and writes a review marker, so a generated translation
cannot silently replace a curated one.  PO validation and a native-speaker
review remain required before release.
"""
from __future__ import annotations

import argparse
import html
import re
import subprocess
import time
from pathlib import Path

import polib

ROOT = Path(__file__).resolve().parent
DOMAINS = ("dm1", "csb", "dm2", "startup-menu", "firestaff", "nexus", "theron")
TOKEN = re.compile(r"(?:\{[^{}]+\}|%\d*\$?[#0 +\-]*\d*(?:\.\d+)?[a-zA-Z]|\\x01\d|\^.|<[^>]+>)")


def mask(text: str) -> tuple[str, list[str]]:
    tokens: list[str] = []

    def substitute(match: re.Match[str]) -> str:
        tokens.append(match.group(0))
        return f"ZXQPROTECT{len(tokens) - 1}QXZ"

    return TOKEN.sub(substitute, text), tokens


def unmask(text: str, tokens: list[str]) -> str:
    for index, token in enumerate(tokens):
        text = text.replace(f"ZXQPROTECT{index}QXZ", token)
    return text


def translate(text: str, language: str) -> str:
    response = subprocess.run(
        ["curl", "--fail", "--silent", "--show-error", "--max-time", "20", "--get",
         "--data-urlencode", "sl=en", "--data-urlencode", f"tl={language}",
         "--data-urlencode", f"q={text}", "https://translate.google.com/m"],
        check=True, capture_output=True, text=True,
    )
    match = re.search(r'<div class="result-container">(.*?)</div>', response.stdout, re.DOTALL)
    if not match:
        raise RuntimeError("translation response did not contain a result")
    value = html.unescape(re.sub(r"<[^>]+>", "", match.group(1))).strip()
    if not value:
        raise RuntimeError("translation service returned an empty value")
    return value


def translate_batch(sources: list[str], language: str) -> list[str]:
    """Translate independent entries together without allowing them to merge."""
    try:
        return _translate_batch(sources, language)
    except RuntimeError:
        if len(sources) == 1:
            raise
        midpoint = len(sources) // 2
        return (translate_batch(sources[:midpoint], language)
                + translate_batch(sources[midpoint:], language))


def _translate_batch(sources: list[str], language: str) -> list[str]:
    separators = [f"ZXQENTRY{index:04d}QXZ" for index in range(len(sources) - 1)]
    joined = "\n".join(
        part for pair in zip(sources, separators + [""]) for part in pair if part
    )
    translated = translate(joined, language)
    parts = [translated]
    for separator in separators:
        split = re.split(rf"\s*{separator}\s*", parts.pop(), maxsplit=1)
        if len(split) != 2:
            raise RuntimeError(f"translation response lost entry separator {separator}")
        parts.extend(split)
    if len(parts) != len(sources):
        raise RuntimeError("translation response returned the wrong number of entries")
    return parts


def paths(language: str) -> list[Path]:
    result = [ROOT / f"{domain}.{language}.po" for domain in DOMAINS]
    result.append(ROOT / "studio" / f"{language}.po")
    return result


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("language", choices=("de", "es", "fr"))
    parser.add_argument("--limit", type=int, default=25)
    parser.add_argument("--delay", type=float, default=0.2)
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--activate-fuzzy", action="store_true",
                        help="activate non-empty fuzzy entries after token-safe review")
    args = parser.parse_args()
    if args.limit < 1:
        parser.error("--limit must be positive")

    candidates: list[tuple[Path, polib.POFile, polib.POEntry, str, list[str]]] = []
    for path in paths(args.language):
        catalog = polib.pofile(str(path))
        for entry in catalog:
            if len(candidates) >= args.limit:
                break
            if entry.obsolete or entry.msgstr or entry.msgid_plural:
                continue
            source, tokens = mask(entry.msgid)
            candidates.append((path, catalog, entry, source, tokens))
        if len(candidates) >= args.limit:
            break
    changed_catalogs: set[Path] = set()
    offset = 0
    while offset < len(candidates):
        batch = []
        encoded_length = 0
        while offset < len(candidates) and len(batch) < 20:
            candidate = candidates[offset]
            # Google Translate's mobile form caps source text at 2,048 chars.
            # Reserve room for immutable separators and URL form overhead.
            next_length = encoded_length + len(candidate[3]) + 24
            if batch and next_length > 1_600:
                break
            batch.append(candidate)
            encoded_length = next_length
            offset += 1
        results = translate_batch([item[3] for item in batch], args.language)
        for (path, _catalog, entry, _source, tokens), result in zip(batch, results):
            result = unmask(result, tokens)
            if any(token not in result for token in tokens):
                raise RuntimeError(f"token loss in {path}: {entry.msgid!r}")
            entry.msgstr = result
            entry.comment = (entry.comment + "\n" if entry.comment else "") + "Machine-translation draft; requires language review."
            changed_catalogs.add(path)
            print(f"{path.relative_to(ROOT)}: {entry.msgid!r} -> {result!r}")
        time.sleep(args.delay)
    if args.activate_fuzzy:
        for path in paths(args.language):
            catalog = polib.pofile(str(path))
            changed = False
            for entry in catalog:
                if entry.msgstr and "fuzzy" in entry.flags:
                    entry.flags.remove("fuzzy")
                    changed = True
            if changed:
                # Use the catalog object just read for this activation-only path.
                if not args.dry_run:
                    catalog.save(str(path))
    if not args.dry_run:
        for path in changed_catalogs:
            next(catalog for candidate_path, catalog, *_ in candidates if candidate_path == path).save(str(path))
    print(f"updated {len(candidates)} {args.language} entries")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
