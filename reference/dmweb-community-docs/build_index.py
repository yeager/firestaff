#!/usr/bin/env python3
"""Genererar INDEX.md + index.json för den crawlade community/documentation/-sektionen.

Körs automatiskt av crawl.sh, eller fristående: python3 build_index.py
"""
import json
import re
import sys
from datetime import datetime, timezone
from html.parser import HTMLParser
from pathlib import Path

ROOT = Path(__file__).parent
HTML_DIR = ROOT / "html" / "community" / "documentation"  # underkatalog
ROOT_HTML = ROOT / "html" / "community" / "documentation.html"  # rot-sidan
URLS_FILE = ROOT / "_urls.txt"

PRETTY = {
    "documentation": "Documentation",
    "file-formats": "File Formats",
    "copy-protection": "Copy Protection",
    "dungeon-master-and-chaos-strikes-back": "Dungeon Master & Chaos Strikes Back",
    "dungeon-master-nexus": "Dungeon Master: Nexus",
    "miscellaneous": "Miscellaneous",
}


class MetaExtractor(HTMLParser):
    """Drar ut <title>, <meta description/keywords>, <h1>, lang, canonical."""

    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.in_title = False
        self.title = ""
        self.in_h1 = False
        self.h1 = ""
        self.description = ""
        self.keywords = ""
        self.lang = ""
        self.canonical = ""

    def handle_starttag(self, tag, attrs):
        a = dict(attrs)
        if tag == "title":
            self.in_title = True
        elif tag == "h1":
            self.in_h1 = True
        elif tag == "meta":
            n = a.get("name", "").lower()
            p = a.get("property", "").lower()
            if n == "description":
                self.description = a.get("content", "")
            elif n == "keywords":
                self.keywords = a.get("content", "")
            elif p == "og:description" and not self.description:
                self.description = a.get("content", "")
        elif tag == "html":
            self.lang = a.get("lang", "")
        elif tag == "link" and a.get("rel", "").lower() == "canonical":
            self.canonical = a.get("href", "")

    def handle_endtag(self, tag):
        if tag == "title":
            self.in_title = False
        elif tag == "h1":
            self.in_h1 = False

    def handle_data(self, data):
        if self.in_title:
            self.title += data
        elif self.in_h1:
            self.h1 += data

    @property
    def best_title(self):
        for v in (self.h1.strip(), self.title.strip()):
            if v:
                return " ".join(v.split())
        return ""


def slug_to_breadcrumb(slug_path: str) -> list[str]:
    out = []
    for p in slug_path.strip("/").split("/"):
        out.append(PRETTY.get(p, p.replace("-", " ").title()))
    return out


def human_size(n: int) -> str:
    if n < 1024:
        return f"{n} B"
    for u in ("KB", "MB", "GB"):
        n /= 1024
        if n < 1024:
            return f"{n:.1f} {u}"
    return f"{n:.1f} TB"


def slug_from_path(html_file: Path) -> str:
    """Mappar lokal filsökväg till dmweb-URL-slug.

    html/community/documentation.html                          -> /community/documentation
    html/community/documentation/foo.html                      -> /community/documentation/foo
    html/community/documentation/foo/bar.html                  -> /community/documentation/foo/bar
    """
    rel = html_file.relative_to(ROOT).as_posix()
    if rel == "html/community/documentation.html":
        return "/community/documentation"
    # Under html/community/documentation/...
    prefix = "html/community/documentation/"
    if rel.startswith(prefix):
        sub = rel[len(prefix):]
    else:
        sub = rel
    sub_noext = sub[:-5] if sub.endswith(".html") else sub
    return "/community/documentation/" + sub_noext


def main():
    if not HTML_DIR.exists() and not ROOT_HTML.exists():
        print(f"Saknar både {HTML_DIR} och {ROOT_HTML} - kör crawl.sh först", file=sys.stderr)
        sys.exit(1)

    urls = []
    if URLS_FILE.exists():
        urls = [u.strip() for u in URLS_FILE.read_text().splitlines() if u.strip()]

    html_files = sorted(HTML_DIR.rglob("*.html")) if HTML_DIR.exists() else []
    if ROOT_HTML.exists():
        html_files = [ROOT_HTML] + html_files

    entries = []
    for html_file in html_files:
        slug = slug_from_path(html_file)
        # Hitta matchande URL
        url = None
        for u in urls:
            if u.endswith(slug + "/") or u.endswith(slug):
                url = u
                break
        if url is None:
            url = "http://dmweb.free.fr" + slug

        try:
            content = html_file.read_text(encoding="utf-8", errors="replace")
        except Exception as e:
            print(f"FAIL {html_file}: {e}", file=sys.stderr)
            continue

        ex = MetaExtractor()
        try:
            ex.feed(content)
        except Exception:
            pass

        stat = html_file.stat()
        entries.append({
            "url": url,
            "slug": slug,
            "local_path": str(html_file.relative_to(ROOT)),
            "title": ex.best_title,
            "description": ex.description.strip(),
            "keywords": [k.strip() for k in ex.keywords.split(",") if k.strip()],
            "lang": ex.lang,
            "size_bytes": stat.st_size,
            "fetched_at": datetime.fromtimestamp(stat.st_mtime, tz=timezone.utc).isoformat(),
            "breadcrumb": slug_to_breadcrumb(slug),
        })

    # JSON
    json_path = ROOT / "index.json"
    json_path.write_text(json.dumps({
        "source": "http://dmweb.free.fr/community/documentation/",
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "total_pages": len(entries),
        "total_bytes": sum(e["size_bytes"] for e in entries),
        "entries": entries,
    }, indent=2, ensure_ascii=False))

    # Markdown
    md = []
    md.append("# dmweb.free.fr — `/community/documentation/` Index")
    md.append("")
    md.append("**Källa:** http://dmweb.free.fr/community/documentation/  ")
    md.append(f"**Genererad:** {datetime.now(timezone.utc).strftime('%Y-%m-%d %H:%M:%S UTC')}  ")
    md.append(f"**Antal sidor:** {len(entries)}  ")
    md.append(f"**Total storlek:** {human_size(sum(e['size_bytes'] for e in entries))}")
    md.append("")
    md.append("Alla sidor är speglade lokalt under `html/community/documentation/`")
    md.append("(rot-sidan ligger som `html/community/documentation.html`). Detta är en")
    md.append("forsknings-/referenskopia för Firestaff-projektet")
    md.append("(källåterhållsam DM/CSB/DM2/Nexus/Theron-motor).")
    md.append("")
    md.append("---")
    md.append("")

    # Gruppera per sektion (breadcrumb[1] om finns, annars "Documentation")
    sections = {}
    for e in entries:
        sec = e["breadcrumb"][1] if len(e["breadcrumb"]) >= 2 else "Documentation"
        sections.setdefault(sec, []).append(e)

    for section in sorted(sections.keys()):
        md.append(f"## {section}")
        md.append("")
        for e in sections[section]:
            md.append(f"### [{e['title'] or e['slug']}]({e['local_path']})")
            md.append("")
            md.append(f"- **URL:** <{e['url']}>")
            md.append(f"- **Sökväg:** `{e['slug']}`")
            md.append(f"- **Lokal fil:** `{e['local_path']}`")
            md.append(f"- **Storlek:** {human_size(e['size_bytes'])}")
            md.append(f"- **Språk:** `{e['lang']}`")
            if e["description"]:
                md.append(f"- **Beskrivning:** {' '.join(e['description'].split())}")
            if e["keywords"]:
                md.append(f"- **Nyckelord:** {', '.join(e['keywords'])}")
            md.append("")

    (ROOT / "INDEX.md").write_text("\n".join(md))

    print(f"Skrev {json_path} ({len(entries)} entries)")
    print(f"Skrev {ROOT / 'INDEX.md'}")


if __name__ == "__main__":
    main()
