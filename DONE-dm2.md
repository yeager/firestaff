# Firestaff DONE — DM2

Reviewed 2026-08-25. Completed work only.

- Native FM Towns ZIP/CUE/IMG intake reads original media in RAM, verifies the
  source-owned graphics/dungeon pair and preserves virtual source ownership.
- Real FM Towns M12/M11 startup, title and gameplay corpus checks pass with
  the authentic FM Towns archive and English DOS companion.
- The authentic Amiga installer archive now reaches title, original New Game,
  runtime and a visible native CHARSHEET inventory frame. Its 121×72 RAW4
  source is clipped to the original 119×70 destination using verified GDAT
  pixels and palette, rather than a substitute surface.
- The authentic PC-DOS ZIP now starts through both CLI and the start menu.
  M12 retains its verified `data/GRAPHICS.DAT` and `data/DUNGEON.DAT` virtual
  paths and the native DM2 boot owner reads them only in RAM.
- The authentic Amiga installer now binds its native big-endian, 16-colour
  `INTERFACE_GENERAL/0` PalIRGB field 0 rather than PC field `0xfe`/PAL16.
  The source-owned HUD, outdoor sky and ground produce an accepted M11 frame
  with real assets and no fallback drawing; the ZIP remains memory-owned.
