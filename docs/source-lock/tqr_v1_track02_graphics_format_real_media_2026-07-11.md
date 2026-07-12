# Theron's Quest Track 02 Graphics-Format Real-Media Receipt

## Media and Method

`firestaff_theron_v1_track02_graphics_format_probe` was run against the
operator-staged raw MODE1/2352 pair under the active Firestaff user-data root:

| Variant | MD5 | Path |
| --- | --- | --- |
| JP Track 02 BIN | `b7afb338ad31be1025b53f9aff12d73a` | `/Volumes/Extern-disk/FirestaffUserData/data/theron-extras/japan/Dungeon Master - Theron's Quest (Japan) (Track 02).bin` |
| US Track 02 BIN | `f23601102138f87c33025877767ebf76` | `/Volumes/Extern-disk/FirestaffUserData/data/theron-extras/usa/Dungeon Master - Theron's Quest (USA) (Track 02).bin` |

The catalog compares only exact, nonzero 2048-byte MODE1 user-data sectors at
the established physical alignment: JP sector `n` versus US sector `n + 1`.
It slides by 16-bit words within matching sectors and recognizes only a
32-byte HuC6260-compatible 9-bit little-endian palette shape or eight
increasing little-endian `uint16_t` values with a positive, `0x20`-aligned
constant stride.

## Receipt

```text
FORMAT-SCAN sectors=3445 matching-nonzero=2022 palette-shapes=1522 stride-shapes=78 retained=64 overflow=1536 compression=0 decode=0
summary: fail=0
```

The 64 retained detail records are a bounded sample only. Separate counters
include every strict match before the detail-list capacity is applied. The
window scan intentionally permits overlapping 16-bit starts, so the counts do
not identify 1,522 independent palettes or 78 independent tables.

## Decision

Neither shape is structurally sufficient to add a Track 02 decoder route.
No original HuC6280 loader/disassembly evidence binds a candidate offset to a
VCE palette register, VDC VRAM destination, payload length, compression
scheme, or rendered object. Compression and source-loader binding remain zero,
`decoder_blocked` remains set, and
`theron_v1_track02_graphics_format_catalog_can_decode()` returns false.

The existing caller-supplied HuC6260 palette-word helper remains offset-unbound
and cannot promote catalogued media. No catalog candidate is consumed by the
Theron runtime, and verified media continues to fail closed for unbound
graphics.
