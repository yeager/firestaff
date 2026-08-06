# DM2 FM Towns startup — P3/GDAT parity boundary

Source medium: the user-supplied HME-242 FM Towns disc, read through
`Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip` directly into process memory.
No member named below is unpacked or written to disk.

This records the evidence boundary used by the DM2 FM Towns startup path. It
uses the same *method* as the DM1 FM Towns evidence files—authenticated media,
bounded executable facts, original source coordinates, and a real-media
regression—but deliberately imports no DM1 geometry, text, palette, or input
behaviour.

## Native program identity

`SKULL.EXP` is the selected native HME-242 menu program:

| Fact | Verified value |
| --- | --- |
| MD5 | `0f4b44d286cbee35924a95e7d75ad7e5` |
| File size | 374,416 bytes |
| P3 level / header size | 1 / `0x180` |
| Runtime range | `0x180` + `0x80` bytes |
| Relocation range | `0x200` + 0 bytes |
| Load image | `0x200` + `0x5b490` bytes |
| Symbol table | absent (`0x0` + 0 bytes) |
| Initial EIP / memory requirement | `0x5741c` / `0x5b490` |

The bounded reader is `dm2_v1_boot_parse_fmtowns_p3()` in
`src/dm2/dm2_v1_boot.c`. It validates only the P3 envelope and its declared
ranges; it does not claim to execute the program.

The source cue immediately before the menu is independently constrained by
SKProject `SKULLWIN/startend.cpp`: `DM2_PLAY_MUSIC(0, true)` precedes
`DM2_SHOW_MENU_SCREEN()`. The selected executable's native 29-byte
HMP-to-CDDA table at `SKULL.EXP+0x3dac` maps cue 0 to zero (silence). Firestaff
therefore records silence instead of sending a PC HMP track or an invented
CDDA track to the FM Towns route.

## Observable startup chain

The authenticated `AUTOEXEC.BAT` plan is:

```
SWOOSH -> TITLE -> SKULL -> END
```

The first two stages use HME-242's original in-memory TWANIM records. M11
replays their source EN/DL records and source palette receipts; pointer input
is inert until TITLE completes. The `TITLE` stream supplies its own 12,862-byte
signed SND2 span and five SO events. It is transported at the source-proven
SKWIN 5500 Hz argument, not the invalid 1000 Hz value retained in the stream.

After TITLE, M11 presents only the selected FM Towns
`DATA/GRAPHICS.DAT::TITLE/0/dtImage+dtPalIRGB/4` IMG2 surface, with its local
16-colour palette. The 320x200 indexed menu framebuffer hashes to FNV-1a
`63310e49`. `dt04/0` supplies the NEW GAME (`0xD7`) and RESUME (`0xD9`)
rectangles; no host rectangle or PC palette is substituted.

This is a bounded semantic handoff, not native P3 execution. Keyboard routing,
the original SKULL continuation loop, and a real save-backed RESUME action
remain unavailable. A NEW GAME click cannot construct a party or dungeon;
RESUME remains inert without a boot-admitted original save.

When the source-owned Quit (`0xE0`) rectangle returns from `SKULL.EXP`, M11
now plays the selected `END` member before returning to Firestaff's launcher.
`END` has 420 displayed frames: its 3 EN and 382 DL rows are extended by its
native FO/NE loop records, so the raw EN+DL count is deliberately not used as
an end condition. Its five PL records are decoded for each displayed source
frame. This reuses only the in-memory HME-242 member and is not a generated
fade, title replay or PC transition.

## Keyboard boundary

The PC SKWIN reference maps its `0xD7` New Game event through the PC-only
four-byte row `D7 80 1C 00` (Enter). A bytewise search of the authenticated
`SKULL.EXP` load image (`0x200..0x5b68f`) finds zero instances of that row.
That is not evidence that FM Towns lacks keyboard input; it is evidence that
the PC table is not the FM Towns table. M11 consequently keeps generic
launcher ACCEPT/UP/DOWN tokens inert on this screen until a native P3 input
route, including its event representation and consumer, is decoded. Pointer
input continues to use only the selected title's `dt04/0` rectangles.

The executable scan uses the existing RAM-only HME-242 disc extractor. It
does not persist `SKULL.EXP`, disassemble a copied member, or borrow PC input
data.

## Regression evidence

`tests/test_dm2_fmtowns_m12_real_media.c` verifies the selected source media,
P3 receipt, stream sequence, P3 cue table, GDAT text companion coverage, IMG2
menu palette and the source rectangles while retaining all media in RAM.
`tests/test_dm2_fmtowns_m11_title_real_media.c` verifies the visible M11 chain:
SWOOSH before TITLE, source-timed TITLE/SND2 events, the exact IMG2 menu frame,
and the no-synthetic-party/no-unowned-resume boundary.

The same complete-key check is also part of the English FM Towns boot gate.
The selected PC-English `GRAPHICS.DAT` must supply a non-empty value for every
non-empty text key in the selected Japanese GDAT before M11 reaches the native
startup media. This retains the CD as the image, palette, coordinate and input
owner; it merely refuses an English session with an incomplete original text
corpus. It neither generates translations nor makes an unbound GUI call render.

Both tests require the original disc and explicitly selected PC-English
companion corpus; they skip rather than fabricate data when either is absent.
