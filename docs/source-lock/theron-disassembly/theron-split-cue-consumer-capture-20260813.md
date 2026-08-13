# Theron's Quest split-CUE consumer capture — 2026-08-13

This is an external-disk capture only. The raw trace sidecars are not
repository assets.

## Media and normalization

- US raw Track 02: `TQUS02.bin`, MD5
  `f23601102138f87c33025877767ebf76`.
- The supplied retail CUE layout was normalized privately from the verified
  split ISO pair `TQUS19.iso + TQUS02End.iso`.
- The assembled private ISO was MD5
  `ceb02343868f80cec899e9b239aff2da`.
- System Card 3.0 MD5:
  `ff1a674273fe3540ccef576376407d1d`.

## Runtime receipt

Capture output directory:
`/Volumes/Extern-disk/theron-capture-split-cue-20260813c`.

- 161 raw CD sectors were observed.
- 32 game-owned `$E009` dispatches were observed.
- The main-RAM sidecar contains 65,536 sequential reads and passes the
  parser-only receipt.
- The `$2600–$27FF` target window contains 512 reads, all from reader
  `$CB22`, all zero-valued. There are no runtime or `$C3A0–$C429` reads.
- Spawn-register sidecar: 256 samples. It still has no valid `$B0E5` spawn
  entry or source-owned target publication.
- RNG sidecar: 5,120 window rows. This remains execution provenance only; no
  source-owned return join is present.

Sidecar SHA-256 hashes:

| Sidecar | SHA-256 |
| --- | --- |
| `theron.trace` | `21183994b510a76c6816d236f25a70891480211bc09d56b4cfde2ebec60ae145` |
| `theron.trace.cd` | `7384848831110a71ea346d75a766c29bfd53b8130a8df17f38c580f05ea6e928` |
| `theron.trace.main-ram-consumer` | `9fd4b48d23136df7767ec88f940b59f2b27d3c798f0b2c992500be13a887bc1c` |
| `theron.trace.main-ram-loader` | `83d2f0e0dd7d673261cb3d59b2f27ac5b07ce8f041fdbc8cb07a543e7b55a72d` |
| `theron.trace.spawn-registers` | `80ad8ec4a7070a18d8f2b653e068087bcd734615faca9cb373fcc2c3ed46ea2b` |
| `theron.trace.rng-consumer` | `f7823efbd534bcfde9fbe58024c4e5d57238c294fda1383f61d11115048f3cd2` |

## Admission boundary

This capture proves corrected media normalization and authenticated transport
provenance. It does not prove a level, square, object, HUD, creature, combat,
RNG-return, T700 or T900 consumer. Production semantic publication remains
blocked.
