# Nexus V1 Structure3 Renderer-Plan Receipt

Date: 2026-07-14

## Evidence

`FIRESTAFF_NEXUS_DATA_DIR=/Users/bosse/.firestaff/data/nexus` supplies the
original `LEV00.DGN` through `LEV15.DGN` retail corpus. The focused corpus
target parses all 16 files and records 1,144 Structure3 entries and 18,478
entry-local face-to-normal ordinal pairs. All 18,478 normal vectors meet the
existing signed-16.16 unit-length receipt.

The existing Structure3 face-material receipt also proves that each
texture-flagged `00xx` selector has a bounded local Structure2 descriptor and
each `08xx` selector has a bounded Structure1G declaration. This is an
identifier join only.

## Host Boundary

`nexus_v1_level_build_dgn_view_render_plan()` now copies both receipts from
`Nexus_V1_DgnRendererHandoffReceipt` to `Nexus_V1_DgnRenderPlanReceipt` before
the no-draw status branch. A renderer-facing consumer can therefore audit the
same bounded selector and face/normal correspondence that the parser and
handoff observed, including when the level stays blocked.

## Non-Claims

This receipt does not decode Structure2 payload bytes, assign palettes or
texture pixels, choose transforms, define normal-plane use, clip geometry, or
issue a VDP1 draw. Those require independent original Saturn executable or
capture evidence for payload/palette decoding, transform application, and VDP1
ordering.

## Verification

Ninja targets `test_nexus_v1_dgn_geometry_readiness` and
`test_nexus_v1_dgn_face_mesh_corpus`; CTest names
`nexus_v1_dgn_geometry_readiness` and `nexus_v1_dgn_face_mesh_corpus`.
