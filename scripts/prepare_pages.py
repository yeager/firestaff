#!/usr/bin/env python3
"""Prepare the Markdown documentation tree for a Jekyll Pages build."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import re
import shutil
from urllib.parse import quote


LINK = re.compile(
    r"(?P<open>!?\[[^\]]*\]\()(?P<ws>\s*)"
    r"(?P<target><[^>]+>|[^\s)]+)(?P<tail>[^)]*\))"
)
FRONT_MATTER = re.compile(r"\A---\r?\n.*?\r?\n---\r?\n", re.DOTALL)
END_RAW = re.compile(r"\{%[-+]?\s*endraw\s*[-+]?%\}", re.IGNORECASE)
LIQUID_LITERAL_PREFIX = "__FIRESTAFF_LIQUID_LITERAL_"


def github_url(repo_root: Path, target: Path) -> str | None:
    if not target.is_relative_to(repo_root):
        return None
    kind = "tree" if target.is_dir() else "blob"
    relative = quote(target.relative_to(repo_root).as_posix(), safe="/")
    return f"https://github.com/yeager/firestaff/{kind}/main/{relative}"


def rewrite_page_links(page: Path, site_root: Path, repo_root: Path) -> None:
    docs_source = repo_root / "docs"
    root_readme = site_root / "README.md"

    def rewrite(match: re.Match[str]) -> str:
        raw = match.group("target")
        angle = raw.startswith("<") and raw.endswith(">")
        target = raw[1:-1] if angle else raw
        path, marker, fragment = target.partition("#")
        if not path or path.startswith("/"):
            return match.group(0)

        if page == root_readme:
            # The copied repository README is shown from the Pages root. Make
            # its repository-relative docs/ links resolve beside it on site.
            if path.startswith("docs/"):
                path = path[5:]
            candidate = (site_root / path).resolve()
            if not candidate.exists():
                candidate = (repo_root / path).resolve()
        else:
            source_page = docs_source / page.relative_to(site_root)
            candidate = (source_page.parent / path).resolve()
            if not candidate.exists() and Path(path).suffix.lower() == ".html":
                markdown_page = candidate.with_suffix(".md")
                if markdown_page.is_file():
                    candidate = markdown_page
            if not candidate.exists() and not Path(path).suffix:
                markdown_candidate = candidate.with_suffix(".md")
                if markdown_candidate.is_file():
                    candidate = markdown_candidate

            if candidate.is_dir():
                for index_name in ("index.md", "Home.md"):
                    index = candidate / index_name
                    if index.is_file():
                        candidate = index
                        break

            if candidate == repo_root / "README.md":
                candidate = site_root / "README.md"
            elif candidate == repo_root / "LICENSE":
                candidate = site_root / "LICENSE"
            elif candidate.is_relative_to(docs_source):
                candidate = site_root / candidate.relative_to(docs_source)

        if not candidate.exists():
            return match.group(0)

        if candidate.is_relative_to(site_root) and not candidate.is_dir():
            site_target = candidate.with_suffix(".html") if candidate.suffix.lower() == ".md" else candidate
            new_path = Path(os.path.relpath(site_target, page.parent)).as_posix()
            new_target = new_path + (marker + fragment if marker else "")
        else:
            url = github_url(repo_root, candidate)
            if url is None:
                return match.group(0)
            new_target = url + (marker + fragment if marker else "")

        if angle:
            new_target = f"<{new_target}>"
        return match.group("open") + match.group("ws") + new_target + match.group("tail")

    page.write_text(LINK.sub(rewrite, page.read_text()))


def prepare(repo_root: Path, output: Path) -> None:
    docs_root = repo_root / "docs"
    if not docs_root.is_dir():
        raise SystemExit(f"Documentation source directory not found: {docs_root}")
    if output == docs_root or docs_root in output.parents:
        raise SystemExit("The Pages staging directory must be outside docs/.")
    if output.exists():
        raise SystemExit(f"Pages staging directory already exists: {output}")

    output.parent.mkdir(parents=True, exist_ok=True)
    shutil.copytree(docs_root, output, ignore=shutil.ignore_patterns("._*"))
    shutil.copy2(repo_root / "README.md", output / "README.md")
    shutil.copy2(repo_root / "LICENSE", output / "LICENSE")

    readme = output / "README.md"
    readme.write_text(readme.read_text().replace("(docs/", "("))

    for page in output.rglob("*.md"):
        rewrite_page_links(page, output, repo_root)
        content = page.read_text()
        front_matter = FRONT_MATTER.match(content)
        if front_matter:
            header, body = content[: front_matter.end()], content[front_matter.end() :]
        else:
            header, body = "---\nlayout: default\n---\n\n", content
        if LIQUID_LITERAL_PREFIX in body:
            raise SystemExit(f"Reserved Pages placeholder found in {page}")

        def protect_endraw(match: re.Match[str]) -> str:
            encoded = match.group(0).encode("utf-8").hex()
            return f"{LIQUID_LITERAL_PREFIX}{encoded}__"

        body = END_RAW.sub(protect_endraw, body)
        page.write_text(f"{header}{{% raw %}}\n{body}\n{{% endraw %}}\n")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo-root", type=Path, default=Path.cwd())
    parser.add_argument("--output", type=Path, default=Path("_pages_source"))
    args = parser.parse_args()

    repo_root = args.repo_root.resolve()
    output = args.output if args.output.is_absolute() else repo_root / args.output
    prepare(repo_root, output.resolve())


if __name__ == "__main__":
    main()
