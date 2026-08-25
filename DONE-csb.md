# Firestaff DONE — CSB

Reviewed 2026-08-25. Completed work only.

- The native Atari STX CLI route verifies original title startup, runtime and
  start-menu entry using the supplied campaign media.
- The M12/M11 Atari STX route now retains the hash-verified original
  `ANIMATE.SCR`/`ANIMATE.DAT` container when the selected runtime cache holds
  only `GRAPHICS.DAT`/`DUNGEON.DAT`; 50 Hz VBlank cadence, final FTLCODE
  handoff and first native HUD/viewport frame are exercised against that media.
- On that verified Atari route, native Enter/Accept now crosses the retained
  ANIM.C → FTLCODE handoff instead of being lost in the unrelated PC startup
  dispatcher. A requested CSB PC platform is explicitly rejected before any
  media or cache selection, because no original DOS/PC release exists.
- The Atari M12/M11 handoff regression now fails safely and precisely when a
  selected package cannot be opened, instead of cascading or crashing.
