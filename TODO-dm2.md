# Firestaff TODO — DM2

Reviewed 2026-08-29. Only open work is listed here.

- Complete the source `DM2_DISPLAY_VIEWPORT` pass ordering inside the original
  224x136 backbuffer and `RECT_7` presentation route. The native indoor
  runtime now owns a separate backbuffer, copies the retail RAW4 `RECT_7`
  portion unscaled, then draws the source HUD on the 320x200 screen. The
  source-plane, creature, item, projectile and wall-ornament routes now clip
  to that 224x136 pass; canonical retail GDAT regression tests compare every
  plane byte there and guard both buffer boundaries. The private allocation
  remains full logical size as a defensive guard while remaining special
  passes are audited. Native real-media New Game now proves map 0's T600
  outdoor route is admitted by M11 with real assets and zero fallback draws
  on the PC-DOS archive; the Amiga, FM Towns, and Mac real-media New Game
  routes are also covered. This establishes runtime admission, not
  original-vs-Firestaff pixel parity. Remaining work is outdoor's distinct
  composition, transition stretching and same-tuple original-capture
  comparison; retain only GDAT-owned pixels.
- Replace the non-promotable H2313 DOSBox attempt with labelled original
  captures. Its four existing `224x136` crops are byte-identical and have no
  route labels, so they prove neither a dungeon state nor a same-state pair
  and must not be relabelled or promoted. Capture an authenticated PC 1.0 EN
  `dungeon_gameplay` route with the matching `320x200` frames, `RECT_7`
  crops, input transcript and source-data hashes; then capture the same
  labelled state from Firestaff for comparison.
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
