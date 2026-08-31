# Firestaff TODO — DM2

Reviewed 2026-08-29. Only open work is listed here.

- Complete the source `DM2_DISPLAY_VIEWPORT` pass ordering inside the original
  224x136 backbuffer and `RECT_7` presentation route. The native indoor
  runtime now owns a separate backbuffer, copies the retail RAW4 `RECT_7`
  portion unscaled, then draws the source HUD on the 320x200 screen. The
  renderer still has a 320x200 logical coordinate space, so its private
  allocation deliberately covers that full extent before the 224x136 RECT_7
  copy. Remaining work is the authenticated 224x136 source-coordinate pass,
  outdoor's distinct composition, transition stretching and same-tuple
  original-capture comparison; retain only GDAT-owned pixels.
- Extend real-media gameplay evidence across DOS, Amiga, FM Towns and Mac for
  dialog/input ordering, creature AI/drop routes, audio and save/resume.
- Capture an original PC-DOS `SKSAVE1` WIELD input-to-CD/RAM trace with a
  valid encounter, weapon choice, command arguments and RNG timing, then
  bind the remaining WIELD fallback/luck and creature-drop route to that
  trace. Do not replace it with a generated weapon, creature, save or combat
  result; keep the creature-drop gate closed until the original interaction
  identifies a valid route.
- Bind renderer/HUD V2.2 material, clipping and outdoor routes to original
  GDAT/capture evidence; synthetic V2.2 art is allowed only as a fixture.
