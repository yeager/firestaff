# Pre-push audit — ahead stack 2026-04-27

Scope: `origin/main..HEAD` on Firestaff release worktree before push. No push performed.

## Current state

- Branch state: `## main...origin/main [ahead 32]` before this audit-note update.
- Worktree: clean at audit time before this audit-note update.
- Diff size: 172 files changed, 11,663 insertions, 772 deletions.
- Largest changed files by current size:
  - `m11_game_view.c` — 864,097 bytes (existing source file)
  - `probes/m11/firestaff_m11_game_view_probe.c` — 537,577 bytes (existing probe source)
  - 12 viewport PPM evidence files — 91,407 bytes each
  - `parity-evidence/overlays/pass83/pass83_firestaff_viewport_content_points_stats.json` — 71,474 bytes
  - `parity-evidence/overlays/pass83/pass83_champion_hud_zone_overlay_stats.json` — 68,232 bytes
- No changed file is over 1 MiB.

## Gates run

```sh
git status --short --branch
git diff --stat origin/main..HEAD
git diff --check origin/main..HEAD
git diff --name-only origin/main..HEAD
python3 file-size scan over changed files
git diff --numstat origin/main..HEAD | awk '$1=="-" || $2=="-" {print $0}'
Run the local pre-push sensitive-data scan over `origin/main..HEAD`, excluding binary screenshots.
git diff --name-only origin/main..HEAD | grep -Ei '\.(zip|7z|tar|gz|xz|bz2|dmg|pkg|deb|rpm|exe|dll|dylib|so|a|o|wasm|mp4|mov|wav|mp3)$|(^|/)(build|dist|release|target|node_modules|\.venv|venv)/'
Run the local path/exposure scan over changed text files.
git diff --name-only origin/main..HEAD | grep -E 'verification-screens/|(^|/)(tmp|scratch|release|dist|build)/|\.orig$|\.bak$|~$'
```

## Gate results

- `git status`: clean, ahead 1 after squashing the unpushed stack for public hygiene.
- `git diff --check origin/main..HEAD`: clean.
- Sensitive-data scan over final ahead diff: **0 findings** for credential patterns, private LAN IPs, absolute home paths, and email addresses.
- History hygiene: the public branch now has a single ahead commit, so earlier unpushed intermediate blobs are not part of the pushable history.
- Binary changes: no executable/archive/package blobs; remaining binary evidence is limited to small committed image/reference artifacts.
- File-size scan: no oversized blobs.
- The bad/internal `verification-screens/pass87-subagent-click-route/` and `verification-screens/pass89-subagent-enter-key-route/` capture bundles were removed from the public commit stack and relocated to private local evidence storage.

## Public hygiene actions taken

1. Replaced local source/data paths with placeholders such as `<repo-root>`, `<original-games-dm-root>`, `<greatstone-atlas-root>`, and `<redmcsb-source-root>`.
2. Replaced deprecated LAN host/IP references with `<deprecated-remote-host>` / `<deprecated-lan-ip>` placeholders.
3. Removed generated pass87/pass89 original-capture bundles from the pushable tree.
4. Squashed the ahead stack to one clean public commit so removed internal blobs do not remain in the pushed history.

## Recommendation

Push-readiness is clean for the sensitive-data/public-hygiene gates covered by this audit: credential patterns, private LAN IPs, absolute home paths, email addresses, oversized blobs, and the removed pass87/pass89 internal capture bundles.

Remaining caveat: this is a hygiene/security gate, not a positive original-overlay parity claim. Runtime parity claims still require the separate original-capture evidence gate.
