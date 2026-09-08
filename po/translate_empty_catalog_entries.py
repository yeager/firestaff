#!/usr/bin/env python3
"""Translate incomplete Firestaff PO entries while preserving runtime tokens.

This is an editor aid, not a build step.  It can fill empty entries and, when
explicitly requested, replace English-source fallback values.  It writes a
review marker, so generated text cannot silently replace a curated
translation.  PO validation and a native-speaker review remain required
before release.
"""
from __future__ import annotations

import argparse
import html
import json
import os
import re
import subprocess
import time
from pathlib import Path

import polib

ROOT = Path(__file__).resolve().parent
DOMAINS = ("dm1", "csb", "dm2", "startup-menu", "firestaff", "nexus", "theron")
# These are Firestaff's supported UI locales.  Keep this list aligned with the
# launcher locale picker: a locale is not considered shipped until all game,
# shell and Studio catalogs exist.
LANGUAGES = (
    "cs", "da", "de", "en", "es", "fi", "fr", "hu", "id", "it",
    "ja", "ko", "nl", "no", "pl", "pt", "ru", "sv", "tr", "zh",
)
TOKEN = re.compile(r"(?:\{[^{}]+\}|%\d*\$?[#0 +\-]*\d*(?:\.\d+)?[a-zA-Z]|\\x01\d|\^.|<[^>]+>)")


def usable_translation(value: str) -> bool:
    """Reject HTML/error documents accidentally returned by public endpoints."""
    lowered = value.lower()
    return bool(value and not any(marker in lowered for marker in (
        "<html", "<!doctype", "error 500", "server error",
        "google.com/images/errors", "af-error-page",
    )))


def mask(text: str) -> tuple[str, list[str]]:
    tokens: list[str] = []

    def substitute(match: re.Match[str]) -> str:
        # printf tokens survive the translation services verbatim, while their
        # opaque replacement is sometimes discarded as an unknown word.
        # Keep them literal and retain masking for runtime control/tag tokens.
        if match.group(0).startswith("%"):
            return match.group(0)
        tokens.append(match.group(0))
        return f"ZXQPROTECT{len(tokens) - 1}QXZ"

    return TOKEN.sub(substitute, text), tokens


def unmask(text: str, tokens: list[str]) -> str:
    for index, token in enumerate(tokens):
        # Some translation engines add harmless whitespace between the
        # deliberately opaque marker's words.  Accept only that exact shape;
        # a missing or renumbered marker is still rejected by the caller.
        text = re.sub(rf"ZXQPROTECT\s*{index}\s*QXZ", token, text,
                      flags=re.IGNORECASE)
    return text


def translate(text: str, language: str) -> str:
    """Translate with a browser endpoint and a structured public fallback.

    The HTML form occasionally returns a consent/rate-limit page without a
    result container.  The fallback has the same source/target contract but
    lets us distinguish that response from a valid empty translation.
    """
    request = ["curl", "--fail", "--silent", "--show-error", "--max-time", "20", "--get",
               # Canonical game strings can be English, Japanese, or another
               # platform-native language.  Detect the source instead of
               # incorrectly forcing English (notably for FM Towns catalogs).
               "--data-urlencode", "sl=auto", "--data-urlencode", f"tl={language}",
               "--data-urlencode", f"q={text}"]
    if not os.environ.get("FIRESTAFF_TRANSLATE_SKIP_GOOGLE"):
        response = subprocess.run(
            request + ["https://translate.google.com/m"],
            check=True, capture_output=True, text=True,
        )
        match = re.search(r'<div class="result-container">(.*?)</div>', response.stdout, re.DOTALL)
        if match:
            value = html.unescape(re.sub(r"<[^>]+>", "", match.group(1))).strip()
            if usable_translation(value):
                return value
        try:
            fallback = subprocess.run(
                request + ["--data-urlencode", "client=gtx", "--data-urlencode", "dt=t",
                           "https://translate.googleapis.com/translate_a/single"],
                check=True, capture_output=True, text=True,
            )
            payload = json.loads(fallback.stdout)
            value = "".join(part[0] for part in payload[0] if part and part[0]).strip()
            if usable_translation(value):
                return value
        except (subprocess.CalledProcessError, json.JSONDecodeError, IndexError, TypeError):
            pass
    # MyMemory is deliberately last: it is slower and has a smaller public
    # quota, but provides a resilient non-Google path when Google returns a
    # rate-limit/consent page during a long catalog refresh.
    try:
        memory = subprocess.run(
            ["curl", "--fail", "--silent", "--show-error", "--max-time", "40", "--get",
             "--data-urlencode", f"q={text}",
             "--data-urlencode", f"langpair=auto|{language}",
             "https://api.mymemory.translated.net/get"],
            check=True, capture_output=True, text=True,
        )
        value = json.loads(memory.stdout).get("responseData", {}).get("translatedText", "").strip()
    except (subprocess.CalledProcessError, json.JSONDecodeError) as error:
        raise RuntimeError("translation services did not return a usable response") from error
    if not usable_translation(value):
        raise RuntimeError("translation service returned an empty or error value")
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


def is_foreign_source(text: str) -> bool:
    """Whether text visibly contains a non-Latin source language.

    This deliberately leaves ASCII game identifiers, spell syllables, printf
    layouts, and platform names alone.  It is intended for platform media
    such as FM Towns' Japanese strings that leaked into another locale's
    catalog as a source fallback.
    """
    return any(ord(character) > 0x024F for character in text)


def bootstrap_catalogs(language: str) -> None:
    """Create any missing locale/domain catalogs from their canonical POT.

    This is deliberately explicit rather than allowing a missing catalog to
    mean an accidental English fallback.  `msginit` supplies correct gettext
    headers and leaves every player-facing entry available for translation.
    """
    for path in paths(language):
        if path.exists():
            continue
        template = ROOT / ("firestaff_studio.pot" if path.parent.name == "studio"
                           else f"{path.name.rsplit('.', 2)[0]}.pot")
        path.parent.mkdir(parents=True, exist_ok=True)
        subprocess.run(
            ["msginit", "--no-translator", "--locale", language,
             "--input", str(template), "--output-file", str(path)],
            check=True,
        )


def propagate_existing_translations(language: str) -> int:
    """Reuse a non-fuzzy native translation for an identical source string.

    Domains intentionally remain independent at runtime.  This maintenance
    step merely avoids translating the same label or item name again and only
    accepts a translation that is not the English source fallback.
    """
    catalogs = [(path, polib.pofile(str(path))) for path in paths(language)
                if path.exists()]
    known: dict[str, str] = {}
    for _path, catalog in catalogs:
        for entry in catalog:
            if (not entry.obsolete and entry.msgstr and "fuzzy" not in entry.flags
                    and entry.msgstr != entry.msgid and not entry.msgid_plural):
                known.setdefault(entry.msgid, entry.msgstr)
    changed = 0
    for path, catalog in catalogs:
        dirty = False
        for entry in catalog:
            replacement = known.get(entry.msgid)
            if not entry.obsolete and not entry.msgstr and replacement:
                entry.msgstr = replacement
                entry.comment = ((entry.comment + "\n") if entry.comment else "") + \
                    "Reused reviewed translation from another Firestaff domain."
                changed += 1
                dirty = True
        if dirty:
            catalog.save(str(path))
    return changed


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("language", nargs="*", choices=LANGUAGES,
                        help="one or more locales to process")
    parser.add_argument("--all", action="store_true",
                        help="process every supported locale except English")
    parser.add_argument("--bootstrap", action="store_true",
                        help="create missing game and Studio catalogs first")
    parser.add_argument("--propagate", action="store_true",
                        help="reuse identical non-fuzzy translations from sibling domains")
    parser.add_argument("--propagate-only", action="store_true",
                        help="perform only sibling-domain reuse; do not call a translation service")
    parser.add_argument("--limit", type=int, default=25)
    parser.add_argument("--delay", type=float, default=0.2)
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--activate-fuzzy", action="store_true",
                        help="activate non-empty fuzzy entries after token-safe review")
    parser.add_argument("--replace-source-fallback", action="store_true",
                        help="also translate msgstr values that still equal their msgid")
    parser.add_argument("--foreign-source-only", action="store_true",
                        help="with --replace-source-fallback, process only non-Latin source text")
    args = parser.parse_args()
    if args.limit < 1:
        parser.error("--limit must be positive")
    if args.foreign_source_only and not args.replace_source_fallback:
        parser.error("--foreign-source-only requires --replace-source-fallback")
    languages = list(args.language)
    if args.all:
        languages = [language for language in LANGUAGES if language != "en"]
    if not languages:
        parser.error("select one or more locales, or use --all")
    if "en" in languages:
        parser.error("English is the source locale and needs no translation")

    total = 0
    for language in languages:
        if args.bootstrap and not args.dry_run:
            bootstrap_catalogs(language)
        if (args.propagate or args.propagate_only) and not args.dry_run:
            reused = propagate_existing_translations(language)
            if reused:
                print(f"reused {reused} {language} entries from sibling domains")
        if args.propagate_only:
            if args.activate_fuzzy and not args.dry_run:
                for path in paths(language):
                    catalog = polib.pofile(str(path))
                    changed = False
                    for entry in catalog:
                        if entry.msgstr and "fuzzy" in entry.flags:
                            entry.flags.remove("fuzzy")
                            changed = True
                    if changed:
                        catalog.save(str(path))
            continue
        candidates: list[tuple[Path, polib.POFile, polib.POEntry, str, list[str]]] = []
        for path in paths(language):
            if not path.exists():
                raise RuntimeError(f"missing catalog: {path}; rerun with --bootstrap")
            catalog = polib.pofile(str(path))
            for entry in catalog:
                if len(candidates) >= args.limit:
                    break
                if (entry.obsolete or entry.msgid_plural or
                        (entry.msgstr and not (
                            args.replace_source_fallback and entry.msgstr == entry.msgid))):
                    continue
                if args.foreign_source_only and not is_foreign_source(entry.msgid):
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
            results = translate_batch([item[3] for item in batch], language)
            for (path, _catalog, entry, _source, tokens), result in zip(batch, results):
                result = unmask(result, tokens)
                if any(token not in result for token in tokens):
                    raise RuntimeError(f"token loss in {path}: {entry.msgid!r}")
                entry.msgstr = result
                entry.comment = (entry.comment + "\n" if entry.comment else "") + "Machine-translation draft; requires language review."
                changed_catalogs.add(path)
            # A public service can rate-limit a later batch.  Checkpoint each
            # completed, token-validated batch so an interrupted large locale
            # refresh never loses earlier valid translations.
            if not args.dry_run:
                for path in changed_catalogs:
                    next(catalog for candidate_path, catalog, *_ in candidates
                         if candidate_path == path).save(str(path))
            time.sleep(args.delay)
        # Activate last: candidate catalogs above are held in memory, so doing
        # this before saving them would silently restore their old fuzzy flags.
        if args.activate_fuzzy and not args.dry_run:
            for path in paths(language):
                catalog = polib.pofile(str(path))
                changed = False
                for entry in catalog:
                    if entry.msgstr and "fuzzy" in entry.flags:
                        entry.flags.remove("fuzzy")
                        changed = True
                if changed:
                    catalog.save(str(path))
        total += len(candidates)
        print(f"updated {len(candidates)} {language} entries")
    print(f"updated {total} entries across {len(languages)} locale(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
