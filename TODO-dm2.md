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
  passes are audited. Native real-media New Game routes are covered for
  PC-DOS, Amiga, FM Towns, and Mac. The PC-DOS start is map 0's indoor Skullkeep
  cave: SKProject `IS_MAP_INSIDE` reads the active map's GRAPHICSSET scene flag
  (map 0 uses graphics set 2 with flags `0x000b`), rather than treating map 0
  as outdoor. A previous real-media test incorrectly required the T600 outdoor
  route at this start and has been corrected. The PC-DOS launch and indoor
  runtime frame are admitted with real assets and zero fallback draws; this
  establishes runtime admission, not original-vs-Firestaff pixel parity.
  A separate PC-9821 retail CUE/BIN-in-ZIP reader is wired through the DM2
  source owner and explicit `pc98` launcher choice. The authentic archive is
  now hash-verified, selectable in the launcher, and reaches the original
  title with its CDDA tracks available. New Game now prepares the authentic
  source-owned world and runtime-session candidate; the PC-9821 GDAT's missing
  static-animation entry for creature 41 is retained as the no-frame state
  returned by the original getter. The in-viewport M11 click now confirms the
  File_header-rooted first champion already selected by STARTEND, commits
  GAME_LOAD, and an M11 movement command is checked against the changed
  runtime pose. Additional party-selection cycles, sustained gameplay and
  audible CDDA output are not yet verified. Keep the PC-9801 demo and IBM
  PS/V floppy inputs preservation-only.
  Remaining work is outdoor's distinct composition, transition stretching
  and same-tuple original-capture comparison; retain only GDAT-owned pixels.
- Pair the newly captured, labelled PC 1.0 EN original New Game route with
  Firestaff at the same game state. The retired H2313 crops remain
  non-promotable because they are byte-identical and lack route labels; they
  must not be relabelled. The new original route establishes menu → New Game
  → real runtime with matching `320x200` frames, `RECT_7` crops, input
  transcript and source-data hashes. Its final two runtime captures are
  byte-identical, so it proves one stable runtime pose only. Capture the same
  labelled pose from Firestaff, then add a same-state pair manifest and diff;
  do not promote either side as pixel parity before that comparison exists.
- Extend real-media gameplay evidence across DOS, Amiga, FM Towns and Mac for
  dialog/input ordering, creature AI/drop routes, audio and save/resume.
- For the Japanese FM Towns edition, pair one original-emulator session with
  Firestaff at the same startup checkpoints. The retained original trace
  proves pre-title → FTL → castle title → emulator-directed input → first
  dungeon, but its nominal menu checkpoint was still title animation and it
  has no valid audio receipt. Obtain a source-owned menu/loading/HUD trace,
  VRTC/CRTC palette/register receipts, and a same-state Firestaff comparison
  before correcting or claiming parity for the reported palette, missing-menu
  and post-New-Game dungeon defects. Do not use Firestaff's own decoder output
  as the expected original framebuffer.
- Capture an original PC-DOS `SKSAVE1` WIELD input-to-CD/RAM trace with a
  valid encounter, weapon choice, command arguments and RNG timing, then
  bind the remaining WIELD fallback/luck and creature-drop route to that
  trace. Do not replace it with a generated weapon, creature, save or combat
  result; keep the creature-drop gate closed until the original interaction
  identifies a valid route.
- Bind renderer/HUD V2.2 material, clipping and outdoor routes to original
  GDAT/capture evidence; synthetic V2.2 art is allowed only as a fixture.
