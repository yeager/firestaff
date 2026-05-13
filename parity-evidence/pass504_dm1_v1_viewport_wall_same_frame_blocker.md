# Pass504 - DM1 V1 viewport/wall same-frame blocker

Status: PASS504_DM1_V1_VIEWPORT_WALL_SAME_FRAME_BLOCKER_LOCKED

## Decision

Viewport/walls source and Firestaff spec/probe evidence are locked, but parity still lacks one same-frame original DM1 V1 and Firestaff wall/door occlusion capture tied to the F0128 composition and F0097 present boundary.

## ReDMCSB source locks
- DUNVIEW.C:8318-8543 / F0128_DUNGEONVIEW_Draw_CPSF ok=True - The comparison frame must be tied to the authoritative DUNVIEW far-to-near composition pass.
- DUNVIEW.C:3048-3180 / F0100/F0101/F0102/F0765 viewport blits ok=True - Wall/door occlusion proof must bind to the routes that write into G0296, not only local metadata.
- DUNVIEW.C:4547-5885 / F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF ok=True - Same-frame proof must keep object/creature/projectile layering attached to the source cell-order pass.
- DUNVIEW.C:6361-7959 / F0116-F0124 wall branches ok=True - Center/side walls occlude normal content, with explicit alcove replay exceptions that the frame must identify.
- DRAWVIEW.C:709-858 / F0097_DUNGEONVIEW_DrawViewport ok=True - A promotable screenshot/crop must be after this present boundary for the same F0128-built buffer.

## Firestaff evidence locks
- pass502_precise_blocker ok=True file=parity-evidence/pass502_dm1_v1_viewport_wall_occlusion_audit.md line=40
- pass500_source_gate ok=True file=tools/verify_pass500_dm1_v1_viewport_walls_blocker_cleanup_source_lock.py line=230
- pass499_runtime_predicate ok=True file=tools/verify_pass499_dm1_v1_wall_occlusion_runtime_evidence_gate.py line=156
- wall_spec_matrix ok=True file=tools/verify_pass496_dm1_v1_wall_occlusion_spec_matrix.py line=15
- wall_contract_probe ok=True file=probes/dm1/firestaff_dm1_v1_wall_composition_contract_probe.c line=66

## Required next proof
- canonical original DM1 V1 viewport crop for a wall/door occlusion case
- matching Firestaff crop from the same map/x/y/direction and wall/door state
- runtime trace or manifest proving Firestaff reached F0128 then F0097/VIDRV for that exact frame
- hash/region evidence showing the crop is not a repeated static gameplay frame
- explicit identification of wall return versus alcove/door two-pass exception when applicable

## Non-claims
- no new original runtime capture
- no original-vs-Firestaff pixel parity promotion
- no movement-core change

Manifest: parity-evidence/verification/pass504_dm1_v1_viewport_wall_same_frame_blocker/manifest.json
