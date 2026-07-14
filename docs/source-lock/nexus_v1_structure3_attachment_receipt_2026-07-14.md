# Nexus V1 Structure3 Attachment Receipt

Date: 2026-07-14

## Evidence

The tracked DMWeb Nexus DGN reference identifies Structure1A byte 1 as an
index of a 3D model in Structure3. It identifies Structure1F alcove, wall
decoration, and wall sensor byte 1 as the face number in that model. The same
reference specifies one Structure3c normal row for every Structure3b face row.

The focused corpus tests first require the canonical MD5 for every retail
`LEV00.DGN` through `LEV15.DGN`. The parser then requires a complete
Structure1A owner relation, a bounded Structure3 directory, bounded face rows,
and the existing entry-local face/normal ordinal receipt. Each selector must
resolve to an in-range Structure3 entry and to an in-range face ordinal in
that entry; the normal is only the same ordinal in that entry's paired normal
region. Any failure invalidates the whole attachment receipt.

## Host Boundary

`Nexus_V1_DgnStructure3AttachmentReceipt` reaches both
`Nexus_V1_DgnRendererHandoffReceipt` and `Nexus_V1_DgnRenderPlanReceipt`. It
records no selected mesh command and never makes the plan drawable.

## Non-Claims

The receipt does not establish an object's placement, transform, face
orientation, normal-plane use, culling, texture or palette decoding, VDP1
state, or drawing order. Original Saturn executable or capture evidence is
still required before any of those behaviors may be added.

## Verification

Strict C99 builds of `test_nexus_v1_dgn_geometry_readiness` and
`test_nexus_v1_dgn_face_mesh_corpus`, run with
`FIRESTAFF_NEXUS_DATA_DIR=/Users/bosse/.firestaff/data/nexus`.
