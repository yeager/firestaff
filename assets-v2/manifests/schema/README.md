# assets-v2 manifest schemas

These schemas define the checksum-addressed manifest layer described in `VERSION2_GRAPHICS_PLAN.md`.

- `asset-pack.schema.json` validates a per-game/per-track manifest.
- `asset-entry.schema.json` validates one logical asset binding to a SHA-256-addressed blob.

The schemas keep V2 tracks explicit: original, upscaled 4K, upscaled 1080p, and enhanced assets are separate manifest selections. Binary assets are referenced by lowercase SHA-256 digest and repository-relative `assets-v2/store/sha256/<prefix>/<sha256>.<format>` paths. The manifest is the source of truth; platform packaging may materialize copies or hardlinks later.

## Local validation gate

Use the repository-local validator when changing V2 manifest scaffolding:

```sh
python3 tools/validate_v2_manifests.py
```

That gate performs dependency-free structural checks over `assets-v2/manifests/*.manifest.json`: required top-level fields, allowed `productionClass` / `status` values, duplicate asset IDs, duplicate `packId` values across selected manifests, repository-relative path safety, positive dimensions, exact 50% 4K-to-1080p downscale (`master4k == derived1080p * 2`), and referenced spec files.

For CI or dashboards, add `--json`. The JSON summary includes per-manifest `downscaleChecks` / `downscaleFailures` counters plus top-level `checks.duplicatePackIds` and `checks.downscalePolicy` sections so operators can see that duplicate-pack and 4K-to-1080p policy gates actually ran.

For asset handoff checks, add PNG dimension probing:

```sh
python3 tools/validate_v2_manifests.py --check-files
```

`--check-files` uses Pillow when available and reports missing expected-size PNGs as warnings for assets whose status implies files should exist (`in-progress`, `rebuilt`, `approved`, `shipped`). Use `--strict-files` only when a worker intentionally wants missing expected-size PNGs to fail CI.

Run the built-in synthetic regression fixtures after validator changes:

```sh
python3 tools/validate_v2_manifests.py --self-test
```

The self-test creates temporary lightweight manifests only; it accepts an exact `3840x2160 -> 1920x1080` downscale and rejects both an off-by-one derived size and duplicate `packId` selections.
