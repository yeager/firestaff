# CSB Amiga 3.5 CTRaw PACK disassembly audit

Status: `PACK_FRAMING_VERIFIED_DECODER_STILL_OPEN`

Date: 2026-10-14

## Scope

This note records a clean-room format investigation for the four original
Amiga 3.5 CTRaw members. The goal is native Firestaff intake without a runtime
dependency on SPS/CAPS. SPS CAPS was used locally as a non-commercial oracle;
its decoder implementation is not copied, linked, bundled, or adapted here.

Archive:

`Chaos Strikes Back for Amiga 3.5 CTRaw.7z`

SHA-256:

`e4362e36c65fcde533346ac559698a016ec804b748113f73ac6b510ef67397a8`

## Original member measurements

Each member contains 164 DATA/TRCK pairs and 328 PACK records, two PACKs per
track. Firestaff's `firestaff_caps_raw_read` accepts all four members after
checking container/chunk bounds and header/payload CRCs.

The first English game disk's first DATA record has body size 18,582 bytes and
two extents of 5,894 and 12,680 bytes. Its PACK records declare:

| Pack | Unpacked bytes | Unpacked CRC-32 | Packed bytes | Packed CRC-32 |
| --- | ---: | --- | ---: | --- |
| 0 | 50,212 | `8c3e6334` | 5,870 | `d5b3859b` |
| 1 | 62,758 | `adaa695a` | 12,656 | `94051a5c` |

Across the four disks, pack 0 expands to roughly 48–50 KiB and pack 1 to roughly
62 KiB per track. The two streams therefore represent large intermediate
buffers, not the final approximately 12.5 KiB bit-cell buffer exposed by the
CAPS track API.

## Binary observations

The local ARM64 CAPSImage binary retains symbols for `CCTRawCodec`, including
`GetPackHeader`, `DecompressDensity`, `DecompressTrack`, and
`DecompressTrackData`. Disassembly confirms these facts:

- The pack header is 24 bytes, and the decoder checks the `PACK` signature,
  header CRC, packed length, and packed-payload CRC.
- Density and track data use separate expansion paths.
- The expansion routine reads control bytes backward from the end of the
  packed payload and fills the output buffer backward. Its control flow has
  several token classes, short and extended copy/fill paths, and an ARM64
  vectorized path for bulk expansion.
- The token grammar is not yet reconstructed. These observations are not an
  implementation specification and must not be used to claim native CTRaw
  support.

The disassembly was inspected from the local development-only binary at
`.codex-scratch/csb-ctraw-oracle/sps/CAPSImage.framework/CAPSImage`. No vendor
source was used to produce Firestaff code.

## Oracle probe and limitations

A development-only probe at
`.codex-scratch/csb-ctraw-oracle/capsdump.c` calls the documented CAPS image
and track APIs. On the English game disk, track 5/head 0 returns one normalized
track-data buffer of 12,551 bytes and no sector records. This is a later
bit-cell representation; it does not expose the two PACK expansion buffers,
so its output cannot validate either per-PACK uncompressed CRC. The first
tracks are also marked flakey/noise by the oracle. Neither the probe nor the
previous ADF conversion proves lossless CTRaw decoding or damaged-track
recovery.

## Next verification gate

Implement the PACK token grammar independently, with strict input/output
bounds. For each original disk, require all 328 decoded pack sizes and CRC-32
values to match their headers. Then correlate the two decoded pack roles with
the CAPS track/revolution output, including an ordinary readable track and a
copy-protection or multi-revolution track. Keep the oracle and generated
images outside Firestaff runtime and distribution. Only after those checks
pass should the native decoder be wired into Amiga 3.5 media discovery and the
A35E/A35M start-menu routes.
