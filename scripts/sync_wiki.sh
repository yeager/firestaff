#!/usr/bin/env bash
set -euo pipefail

: "${GITHUB_REPOSITORY:?GITHUB_REPOSITORY required}"
WIKI_SRC="${WIKI_SRC:-docs/wiki}"

WIKI_DIR="$(mktemp -d)"
trap 'rm -rf "$WIKI_DIR"' EXIT

git clone "https://x-access-token:${GH_TOKEN}@github.com/${GITHUB_REPOSITORY}.wiki.git" "$WIKI_DIR" 2>/dev/null || {
    cd "$WIKI_DIR"
    git init
    git remote add origin "https://x-access-token:${GH_TOKEN}@github.com/${GITHUB_REPOSITORY}.wiki.git"
}

cp "${WIKI_SRC}"/*.md "$WIKI_DIR/"

cd "$WIKI_DIR"
git add -A
if git diff --cached --quiet; then
    echo "Wiki is up to date."
    exit 0
fi

git -c user.name="github-actions[bot]" \
    -c user.email="github-actions[bot]@users.noreply.github.com" \
    commit -m "Sync wiki from docs/wiki/"

git push origin HEAD:master || git push origin HEAD:main

echo "Wiki synced."
