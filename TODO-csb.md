# Firestaff TODO — CSB

Reviewed 2026-09-05. Only open work is listed here.

- Replace the C37 wander bridge's locally reseeded relative direction with
  the authenticated GROUP.C F0209 control flow. GROUP.C:2153-2155 performs
  a one-bit movement gate followed by an absolute two-bit direction using
  the shared stream (BASE.C:1717,1765). The current runtime instead combines
  dungeon/time/position into a local seed and adds raw low bits to direction.
  Audit persistent RNG ownership, retry/admission order and source event
  context before changing this branch; a shifted-mask-only patch is not
  sufficient. See `docs/parity/DM1_RAW_RNG_CONSUMER_AUDIT.md` for scan scope.

- Extend the source-profile C37 levitation/pit checks to original platform
  captures and exact RNG evidence. Audit fall-damage Defense=255 semantics
  and moving-group aftermath separately; C25 immunity is not proof of those
  paths. Verify levitation across chained teleporter/pit destinations.

- Extend chest verification to Atari/Amiga pointer geometry, chest switches,
  runtime-driven owner changes and native save boundaries. F31 EN/JP now
  verifies open-slot persistence, pickup and permitted replacement in both
  presentation modes, with synchronized runtime chains. Original oversized
  residents remain forbidden to reinsert; do not loosen G0038 slot masks to
  make such placements appear supported.

- Verify FM Towns distance-volume production in the shared audio runtime.
  Transport now scales the native 1..127 driver domain correctly, and direct
  local events use 127. The runtime request branch already implements the
  MEDIA551 division-before-multiplication distance formula; add end-to-end
  coverage through completed-event history and compare original captures.

- Close the authenticated FM Towns Japanese catalog gap: extraction from
  graphics MD5 `761d6fc588b31aeaaa9caf3725e111b9` and dungeon MD5
  `7ca51c17ef8bd542ca5f0273672ec1a5` produces 218 non-header msgids;
  all keys are now present in `po/csb.sv.po`, with 177 still untranslated.
  The Atari ST corpus (221 msgids) passes the same comparison. Existing
  catalog completion statistics must not be interpreted as coverage of all
  retail editions. Review remaining Japanese M564 object-name translations
  against their original indices and verify displayed live-consumer text.
  M11's object/action consumers and the extractor now share native CP932 to
  UTF-8 conversion. Add and verify the missing translations at the live
  Japanese consumers; Japanese glyph-raster parity remains separate work.

- Add authentic inscription-bearing Atari, Amiga and F31E runtime fixtures or
  captures and assert the candidate framebuffer pixel delta for the now-wired
  C02/M648/F0635 paths, including the now-wired distant/side zones. Obtain an
  allowed authentic F31J glyph-raster strategy: its exact F0168 second decode
  and F0646 substring route are wired, but F0644 pixels remain fail-closed
  because the retail CD delegates them to the FM Towns EGB system font and
  contains no font ROM. English M648 is never borrowed. Original-capture pixel
  parity remains unclaimed.

- Obtain checksum-verified DSA-bearing saves and per-edition save corpora;
  use them to extend native gameplay, timer and transaction coverage.
- Add original-data HUD, viewport, title, door and audio capture comparisons
  for Atari ST, Amiga and FM Towns. Do not use CSBWin as a PC game route.
  Amiga RGB4 register expansion is now compared and fixed from an authentic
  FS-UAE/Kickstart 1.3 title route; retain as open only a same-source-VBlank
  image pair plus the remaining HUD/viewport/door/audio comparisons. Atari
  still needs genuine compatible TOS capture firmware: the staged EmuTOS
  image reproducibly trips the protected retail program's Bus Error handler.
  Legacy loose-file capture tests that labeled CSB as PC3.4 have been removed;
  replacements must name and authenticate one of the three supported retail
  platforms.
- Extend source-text extraction beyond the now-bound object/action names and
  reviewed Utility Disk DB2 instruction. Atari ST now
  supplies M564 from item 556 and the G0490 subtable from C560 item 560 to
  both the live runtime and catalog generator;
  FM Towns also binds English/Japanese M564 and DYNA_BUTTONS to its live
  runtime directly from the packed CD. Amiga A31M binds M564/C699 from its
  selected big-endian DMCSB2 media, while A31E/A35E bind compiled G0490 only
  from the hash-locked APPB on the same admitted disk. The live selected
  dungeon now promotes the reviewed utility instruction to the CSB POT while
  rejecting structured champion records and their encoded statistics. The
  native Eye path now walks a real C07 scroll to its platform-specific C02
  reference and authentic text pool without consulting DM1 `world.things`.
  Continue walking square-chain relations at their native presentation
  consumers before promoting further messages and inscriptions.
  Add executable-owned dialogs and Hint Oracle text the same way; never restore
  invented semantic keys or translate encoded champion-stat payloads.
  The final main-draw message-area pass now preserves the live F0168-decoded
  C02/DSA receipt and cannot overwrite it with M11 telemetry; extend the same
  source ownership to the remaining inscription consumers.
  Its persistence is also source-timed: supported Atari/Amiga/FM Towns rows
  remain visible for 70 game ticks and disappear at the F0046 deadline.
- Keep V2.2 presentation closed until a real source-owned material/pixel
  binding exists.
## Deferred original-data corpus

DSA-bearing saves and visual/audio captures are deferred while native Atari,
Amiga and FM Towns media paths continue to be improved from the supplied
original packages.  Later work must record media hashes and provenance before
using newly supplied captures; it must never generate a replacement corpus.
