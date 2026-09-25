# Theron’s Quest: real-data inventory

This page records the local media identities used by Firestaff’s Theron
bring-up. It separates the PC Engine CD data track from the related Track 19
ISO files so a valid-looking image cannot be attached to the wrong loader.

## Authenticated local media

| File | Size | MD5 | Ownership in Firestaff |
|---|---:|---|---|
| `TQUS02.bin` | 8,104,992 | `f23601102138f87c33025877767ebf76` | US Track 02, raw MODE1/2352 |
| `TQJP02.bin` | 8,102,640 | `b7afb338ad31be1025b53f9aff12d73a` | JP Track 02, raw MODE1/2352 |
| `TQJP02End.iso` | 305,152 | `397039af02d50d15c70b74088eb8a1cb` | Hash-identified 149-sector JP Rev. 1 payload; supplied bytes are zero-filled and rejected as launch media |
| `TQUS19.iso` | 5,984,256 | `51b40a17b92a30339957ba564aa0015c` | US Track 19 ISO |
| `Dungeon Master - Theron's Quest (USA) (Track 19).bin` | 7,754,544 | `fd45d13690a214b17b48a6b7c05b93b4` | US Track 19 raw MODE1/2352; 225-sector pregap, 2,922 ISO-equivalent payload sectors, 150-sector tail |
| `TQJP19.iso` | 6,291,456 | `f9f069a5e489b91207f3156059b756f1` | JP Track 19 ISO |
| `Dungeon Master - Theron's Quest (Japan) (Rev 1) (Track 19).bin` | 7,752,192 | `27d54f58154662885bb67d5967e5111e` | JP Rev. 1 Track 19, raw MODE1/2352 with 224-sector CUE pregap |
| `TQUS02-ceb02343868f80cec899e9b239aff2da.iso` | 6,596,608 | `ceb02343868f80cec899e9b239aff2da` | Materialized US split Track 02 ISO |

`TQJP19.iso` is not the JP Track 02 image. Its `f9f069…` identity belongs to
the Track 19 metadata readers. The JP `TQJP02End.iso` file is a distinct
149-sector, hash-identified Track 02 payload, but the supplied bytes are
zero-filled; its identity and length do not establish dungeon-bank content,
and file-backed campaign discovery now rejects it as unstartable.
It must not be treated as a full six-megabyte Track 19 projection. The US distribution is the one that needs
the `TQUS19.iso` plus `TQUS02End.iso` materialization step.

## Real bindings currently admitted

- The raw US and JP Track 02 files have hash-verified startup bitmap spans,
  level-bank offsets, object/ground-reference receipts and HuC6280 bank-$1f
  disassembly bytes.
- The US and JP Track 19 ISOs, plus the supplied JP Rev. 1 raw Track 19 BIN,
  have hash-verified item-name, item-property, level-label and startup-envelope
  metadata receipts. The raw form retains its CUE-defined 224-sector pregap;
  only its following user-data sectors are normalized in memory for these
  ISO-addressed readers.
- The authentic US raw Track 19 BIN is now also staged locally. Its complete
  2,922-sector post-pregap payload was compared sector-by-sector with
  `TQUS19.iso`; the probe also bound its decoded name/property bank against
  the authenticated US Track 02 dungeon-4 item source. The trailing 150 raw
  sectors are retained in the source file but are not part of the ISO view.
- The raw BIN palette windows are copied only after the exact regional hash and
  MODE1/2352 user-data mapping pass. The US assembled ISO has a separate
  direct-ISO palette path.
- The production viewport and dungeon handoff remain fail-closed. A byte
  span, palette-shaped window, or indexed atlas is not promoted to a dungeon,
  object, VDC/VCE consumer, HUD, or semantic level binding by itself.
- The production forcefield handoff retains the cross-checked numeric
  champion-record fields, but compiles out the unbound US names and titles.
  The explicit roster table remains available only to fixture/probe targets;
  JP startup names/titles are admitted only through the real JP cluster reader.
- The JP raw Track 02 champion cluster is independently receipt-bound at raw
  offset `0x0B3D98`: eight newline/NUL-framed records decode their numeric
  fields from the A–P nibble representation. A wrong regional MD5, offset
  mutation or malformed field rejects the receipt.
- Later-level runtime receipts retain the retail resource frame length
  (`LE16(+2)-5`) and exact user-data end offset in addition to the block and
  payload hashes. The payload remains opaque until the HuC6280 consumer is
  bound.

## Known placeholder boundaries

The following are intentionally not presented as real game data:

- procedural stone palettes and inferred V1 tile/material mappings;
- synthetic V2.2 shapes, HUD widgets and overlay pixels;
- inferred US roster names or JP names copied into the US route;
- fixture dungeon/object consumers and random generator placement;
- a host viewport draw without an authenticated HuC6280 `$2600` RAM consumer
  and source-LBA join.

The source and capture requirements are tracked in `TODO.md`, especially
`THERON-PALETTE-ROUTE`, `THERON-BITMAP-ROUTE-PROVENANCE`,
`THERON-V1-TRACK02-VRAM-CONSUMER` and `THERON-V1-HUC6280-RAM-CONSUMER`.
The provenance boundary follows the Theron’s Quest platform and Track 02
description in `docs/DMWEB_REFERENCE.md` and the local source-lock notes.
