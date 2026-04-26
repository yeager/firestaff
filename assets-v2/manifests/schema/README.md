# assets-v2 manifest schemas

These schemas define the checksum-addressed manifest layer described in `VERSION2_GRAPHICS_PLAN.md`.

- `asset-pack.schema.json` validates a per-game/per-track manifest.
- `asset-entry.schema.json` validates one logical asset binding to a SHA-256-addressed blob.

The schemas keep V2 tracks explicit: original, upscaled 4K, upscaled 1080p, and enhanced assets are separate manifest selections. Binary assets are referenced by lowercase SHA-256 digest and repository-relative `assets-v2/store/sha256/<prefix>/<sha256>.<format>` paths. The manifest is the source of truth; platform packaging may materialize copies or hardlinks later.
