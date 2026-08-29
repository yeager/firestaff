# Firestaff TODO — CSB

Reviewed 2026-08-29. Only open work is listed here.

- Obtain checksum-verified DSA-bearing saves and per-edition save corpora;
  use them to extend native gameplay, timer and transaction coverage.
- Add original-data HUD, viewport, title, door and audio capture comparisons
  for Atari ST, Amiga and FM Towns. Do not use CSBWin as a PC game route.
- Keep V2.2 presentation closed until a real source-owned material/pixel
  binding exists.
- Replace the blocked legacy Atari 7z MINI.DAT wrappers with a native,
  in-memory 7z reader before restoring that optional corpus. They previously
  invoked an external extractor and wrote supplied game media to temporary
  disk, which violates Firestaff's media-ingestion contract.

## Deferred original-data corpus

DSA-bearing saves and visual/audio captures are deferred while native Atari,
Amiga and FM Towns media paths continue to be improved from the supplied
original packages.  Later work must record media hashes and provenance before
using newly supplied captures; it must never generate a replacement corpus.
