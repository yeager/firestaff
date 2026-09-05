# pass454_dm1_v1_hall_original_input_mapping_blocker

- status: `BLOCKED_EXTERNAL_HALL_TRUE_STOP_ARTIFACT_MISSING`
- artifact manifest: `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/hall-true-stop-20260509/manifest.json`
- artifact manifest sha256: `missing`
- parity claim: **not made here**; this gate only retires the stale coordinate-space blocker when pass455 evidence exists.

## Diagnosis

External hall true-stop artifact is not mounted on this host.

## Evidence summary

- stale fresh attempts with mismapped clicks: `0`
- corrected rerun ok: `False`
- corrected requested PC clicks: `[]`
- corrected images: `0` images / `0` unique hashes
- PC34 data provenance remained hash-locked; no filename-only comparison was used.

## Resolution

The stale hall-true-stop mapping blocker is superseded by the corrected rerun. Candidate framebuffer promotion remains owned by pass455/pass449, not by this diagnostic gate.
