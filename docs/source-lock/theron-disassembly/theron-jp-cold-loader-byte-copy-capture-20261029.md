# Theron's Quest JP cold loader byte-copy capture

Date: 2026-10-29

## Capture setup

This cold PCE Fast run used Japanese Rev. 1 Track 02 (MD5
`b7afb338ad31be1025b53f9aff12d73a`) and the headered System Card 3 image whose
ROM body MD5 is `38179df8f4ac870017db21ebcbf53114`. No save-state was loaded.
The private capture is retained on TRV2 at
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-cold-joined-20261029-r2/`.
The wrapper exited at VCE schema validation before the validator fix; the
preserved individual traces were merged manually with the same temporal-only
interpretation. The CD trace uses the reader-MPR-context header, which the
production merger now accepts alongside its earlier header.

## Source-to-RAM evidence

The event-ordered traces contain 47 byte writes with an immediately preceding
CD data-port read (`write.event_sequence = read.event_sequence + 1`) and equal
byte values. The observed runs are:

| Track 02 LBA | User-byte offsets | Physical RAM destination |
| --- | --- | --- |
| 4521 | 0–16 | `0x100000–0x100010` |
| 4589 | 0–16 | `0x100000–0x100010` |
| 5215 | 298–310 | `0x0d089f–0x0d08ab` |

At physical System Card address `0x0ea99`, the bytes decode as `AD 08 18`
(`LDA $1808`), followed at `0x0ea9c` by `91 FA` (`STA ($FA),Y`). These
instructions agree with the traced reader and writer PCs and support a direct
BIOS byte-copy interpretation for these observed bytes. This does not identify
the meaning of the copied bytes or establish a complete loader routine.

## Limits

The VWR trace reached its 1,048,576-record limit at shared event 1,048,831.
The first source-bound CD data read occurred at event 1,048,832, so there is no
VWR coverage during or after these transfers. The CD trace also reached its
65,536-record cap and ended at LBA 5215, short of the full Track 02 range.
Palette writes continued, but do not identify tile content or world cells.

The input verifier witnessed 9 of 11 scheduled button masks; polls at frames
12,510 and 13,020 were not observed. No screenshot was emitted, so this run
does not confirm that the first-person dungeon was reached. The SBI sidecar was
reported missing. No tile-to-world mapping, dungeon-cell semantics, gameplay
mouse behavior, or native renderer parity follows from this partial cold-load
trace. Chronological adjacency alone does not establish causal provenance;
the byte-copy observation is separately supported by the matching byte
values, MPR context and BIOS instruction sequence described above.
