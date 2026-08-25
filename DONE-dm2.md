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
