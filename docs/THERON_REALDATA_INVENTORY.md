# Theron’s Quest: real-data inventory

This page records the local media identities used by Firestaff’s Theron
bring-up. It separates the PC Engine CD data track from the related Track 19
ISO files so a valid-looking image cannot be attached to the wrong loader.

## Authenticated local media

| File | Size | MD5 | Ownership in Firestaff |
|---|---:|---|---|
| `TQUS02.bin` | 8,104,992 | `f23601102138f87c33025877767ebf76` | US Track 02, raw MODE1/2352 |
| `TQJP02.bin` | 8,102,640 | `b7afb338ad31be1025b53f9aff12d73a` | JP Track 02, raw MODE1/2352 |
| `TQJP02End.iso` | 305,152 | `397039af02d50d15c70b74088eb8a1cb` | Complete JP Rev. 1 Track 02 ISO payload (149 sectors) |
| `TQUS19.iso` | 5,984,256 | `51b40a17b92a30339957ba564aa0015c` | US Track 19 ISO |
| `TQJP19.iso` | 6,291,456 | `f9f069a5e489b91207f3156059b756f1` | JP Track 19 ISO |
| `TQUS02-ceb02343868f80cec899e9b239aff2da.iso` | 6,596,608 | `ceb02343868f80cec899e9b239aff2da` | Materialized US split Track 02 ISO |

`TQJP19.iso` is not the JP Track 02 image. Its `f9f069…` identity belongs to
the Track 19 metadata readers. The JP `TQJP02End.iso` file is instead the
complete 149-sector Track 02 ISO payload; it must not be treated as a full
six-megabyte Track 19 projection. The US distribution is the one that needs
the `TQUS19.iso` plus `TQUS02End.iso` materialization step.

## Real bindings currently admitted

- The raw US and JP Track 02 files have hash-verified startup bitmap spans,
  level-bank offsets, object/ground-reference receipts and HuC6280 bank-$1f
  disassembly bytes.
- The US and JP Track 19 ISOs have hash-verified item-name, item-property,
  level-label and startup-envelope metadata receipts.
- The raw BIN palette windows are copied only after the exact regional hash and
  MODE1/2352 user-data mapping pass. The US assembled ISO has a separate
  direct-ISO palette path.
- The production viewport and dungeon handoff remain fail-closed. A byte
  span, palette-shaped window, or indexed atlas is not promoted to a dungeon,
  object, VDC/VCE consumer, HUD, or semantic level binding by itself.

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
