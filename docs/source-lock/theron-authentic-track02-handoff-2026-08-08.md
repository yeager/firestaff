# Theron authentic Track 02 startup handoff

**Verified: 2026-08-08.** This receipt records a real PC Engine CD startup
through Mednafen using the supplied US Track 02 media. It is deliberately
separate from Firestaff's semantic loader and screenshot-promotion gates.

## Bound inputs

| Input | Identity |
|---|---|
| US CUE | MD5 `bae1628ca5bc9a0808d711a6de051071` |
| US Track 02 MODE1/2048 ISO consumed by the capture | MD5 `ceb02343868f80cec899e9b239aff2da` |
| System Card 3.0 | MD5 `ff1a674273fe3540ccef576376407d1d` |
| Instrumented Mednafen executable | MD5 `06ac100b11449da433d9b7abf670b240` |
| Tracked menu image | SHA-256 `c9e0752019bcdf8d32bc9ba4b77f80667ec2c02fbd035c38eefa6c0d60498204` |

The game data and System Card remain user-supplied and are not committed to
the repository. The image is a crop of the real Mednafen window, not generated
art and not a Firestaff renderer frame.

## Positive evidence

The authenticated VDC/VCE replay path also has a narrow palette receipt:
after the real BAT window is decoded into 4bpp tiles, each BAT-selected
palette group is compared byte-for-byte with the corresponding native
little-endian BGR333 words in the same authenticated VCE snapshot. This binds
the screen-space tile pixels to their source palette. It still does not bind
those cells to dungeon squares, perspective, HUD/object records, or gameplay
semantics.

The newly admitted active-dungeon screen pair is recorded by complete-file
FNV-1a identities `VRAM=105dcffb` and `VCE=ea83f117`. The raw pair remains on
the external disk; only its identity is tracked here. Its admission is still
screen-space-only and does not authorize square-to-tile or gameplay rules.

The later authenticated manual dungeon capture from 2026-08-10 is also
admitted by the production screen loader as `VRAM=5d20ebc7` and
`VCE=ea83f117`. Its external receipt records the US Track 02 hash, System
Card hash, Mednafen binary identity, CD-sector transition and 64 KiB/1 KiB
snapshot sizes. This extends the source-bound screen-state allowlist only;
it does not promote a dungeon-square, object, AI or T900 consumer.

The host event path received a real macOS Return key pair (`SDL scancode 40`),
Mednafen exposed the configured PCE Run bit as `raw=0008`, and the CD trace
recorded 56 SCSI reads and 175 raw-sector records. The first authenticated
Track 02 reads include LBA 3234 followed by LBA 4165 and later startup reads.
The resulting image shows the authentic Theron's Quest title/menu screen with
`NEW GAME`, `LOAD GAME`, the title logo, and the original copyright line.

This proves the following narrow boundary:

```text
System Card + US CUE/Track 02
        -> real Mednafen PCE input
        -> BIOS Run handoff
        -> CD Track 02 sector reads
        -> authentic Theron's Quest title/menu frame
```

## Firestaff CUE launch boundary

**Verified: 2026-08-13.** A hash-verified retail `MODE1/2048` CUE no longer
falls into Firestaff's obsolete `TRACK02 CAPTURE REQUIRED` page. The normal
M12 → M11 handoff resolves the CUE to its declared Track 02 ISO and opens the
source-backed Theron title gate. The same boundary is covered by the opt-in
real-media launcher regression and by `firestaff --game theron --data-dir
<cue-root> --boot-probe` against ISO MD5
`ceb02343868f80cec899e9b239aff2da`.

This is deliberately narrower than a gameplay admission: ISO launches at the
title/stage-select boundary and does not auto-load a dungeon. The raw
MODE1/2352 convenience path remains the only route with its separately
verified initial-level load. No level/object, AI, T700 or T900 gate changed.

## Explicitly not proven

This receipt does not prove Firestaff's `$2600` main-RAM consumer, semantic
level/object/tile binding, square-to-tile perspective, VCE palette ownership,
HUD binding, RNG consumption, creature AI/attacks/damage/loot, generator
timing, T700 stat cadence, or T900 object rules. Those remain fail-closed until
their own authenticated runtime consumer and disassembly receipts exist.

The corresponding tracked image is
`verification-screens/theron-quest-us-main-menu.png`. Its README caption
identifies it as an original-media Mednafen capture so it cannot be confused
with a completed Firestaff runtime screenshot.

## Additional original-media dungeon capture

**Verified: 2026-08-09.** A second real Mednafen session used the extracted US
CUE/BIN set and the same verified System Card. After the authentic file-select
and title/menu route, the game reached the original dungeon view. The tracked
864×696 RGBA crop is:

| Artifact | Identity |
|---|---|
| US CUE used for the session | MD5 `63dbd2fab613b2e8030ff4e44b978a39` |
| System Card 3.0 | MD5 `ff1a674273fe3540ccef576376407d1d` |
| Mednafen | 1.32.1 |
| Tracked dungeon image | SHA-256 `0ae87857bdd33dadc2881f2ff5ca00007df6b9b406f124f10115f2fa589ae540` |

This is original-game evidence only. It does not promote the Firestaff
runtime screenshot gate or prove square-to-tile, HUD, creature, T700, T900,
or other unresolved semantic consumers.

## Operator-supplied gameplay and inventory captures

**Verified: 2026-08-09.** The operator supplied two additional screenshots
from the same original US Theron's Quest/Mednafen workflow. They are tracked
as original-media reference captures, not generated Firestaff frames:

| Artifact | Dimensions | SHA-256 | Narrow evidence |
|---|---:|---|---|
| `verification-screens/theron-quest-us-akutuba-original-capture.png` | 819×657 | `13b0795054dea2a37c32392fbc2a6f212d1695f236fce394bbff824be929b718` | AKUTUBA scene/roster presentation |
| `verification-screens/theron-quest-us-inventory-original-capture.png` | 841×611 | `3d9bbd84eeb05d9e35bc9e9dfef9d38c3a5c8effa2642b5190528ece7c344c4f` | champion portrait, equipment layout, health/stamina/mana and food/water HUD |
| `verification-screens/theron-quest-us-dungeon-hud-original-capture.png` | 847×602 | `dbc654b10d3a8b163b5c0d6cea76131f1334707c8cbc58f540a030168cbd3b63` | dungeon perspective, two-champion HUD and source control panel |
| `verification-screens/theron-quest-us-dungeon-item-ground-original-capture.png` | 853×573 | `96c82861c0183563f07574bddb36abb6059bd82bd4622114e04bd9bd097c2447` | dungeon perspective with a ground item |
| `verification-screens/theron-quest-us-dungeon-niche-item-original-capture.png` | 847×625 | `ddd063e7ea584d6f06ee2a02f5ad6d5a3539aaea3bfcd65bd6c333cd5b0b9318` | dungeon perspective with an item in a wall niche |

These images are useful visual evidence for the source presentation and for
the README reference gallery. The screenshots alone do not prove the
underlying RNG, AI, attack, damage, loot, generator, T700 or T900 consumers;
those still require a same-run runtime receipt or a complete source-bound
static consumer proof. The two latest supplied files had the same
`ddd063e7ea584d6f06ee2a02f5ad6d5a3539aaea3bfcd65bd6c333cd5b0b9318` hash, so
only one copy is tracked.
