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
- Amiga A31E and A31M original ZIP → ADF media are read entirely in RAM.  The
  A31E direct C03 handoff verifies `APPB.FTL` and `BJELoad_R` through the same
  selected ADF as `GRAPHICS.DAT`, reaches `csb-entrance-0` with the original
  A31E hash, and does not create an asset-cache copy.  A31M's original
  `APPB.FTL` language page and `KAOS.FTL` continuation now use the same
  source locator and pass the real CLI and start-menu route into runtime. The
  focused real-media regression uses virtual source locators rather than the
  legacy materialization API.
