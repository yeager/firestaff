# Firestaff V2 checksum-addressed asset store

This directory is the prototype landing zone for content-addressed V2 asset files.

Store objects use SHA-256 paths:

```text
assets-v2/store/sha256/xx/<sha256>.<ext>
```

- `xx` is the first two lowercase hex characters of the SHA-256 digest.
- `<ext>` preserves the ingested file extension in lowercase; extensionless files use `.bin`.
- Manifest snippets must use repository-relative paths only.
- This store is a V2 tooling prototype and does not change V1/M10 runtime semantics.

Use `tools/v2_asset_store.py ingest` to copy a file into the store and emit a manifest snippet. Use `tools/v2_asset_store.py validate` to re-check snippets against stored bytes.
