# pass454_dm1_v1_hall_original_input_mapping_blocker

- status: `PASS_PASS454_STALE_MAPPING_BLOCKER_SUPERSEDED_BY_CORRECTED_RERUN`
- artifact manifest: `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/hall-true-stop-20260509/manifest.json`
- artifact manifest sha256: `33b54c69685392e80b3450ffdecf618b6ad0d176edde68a0a68ec1332c33fc12`
- parity claim: **not made here**; this gate only retires the stale coordinate-space blocker when pass455 evidence exists.

## Diagnosis

The original hall-true-stop fresh-entry logs used stale coordinate mapping. The corrected-coordinate rerun now supersedes that blocker when it logs client-relative and absolute/root coordinates separately and produces the expected candidate/terminal frame transition.

## Evidence summary

- stale fresh attempts with mismapped clicks: `13`
- corrected rerun ok: `True`
- corrected requested PC clicks: `[[111, 82], [130, 115]]`
- corrected images: `6` images / `3` unique hashes
- PC34 data provenance remained hash-locked; no filename-only comparison was used.

## Resolution

The stale hall-true-stop mapping blocker is superseded by the corrected rerun. Candidate framebuffer promotion remains owned by pass455/pass449, not by this diagnostic gate.

## Pass500 cleanup verdict

Pass500 cleanup: stale coordinate-mapping blocker remains superseded by corrected rerun; current unresolved work is tracked by pass487/pass497/pass498, not this manifest.
