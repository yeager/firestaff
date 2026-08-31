# DM1/CSB ReDMCSB Behaviour Audit

Reviewed 2026-08-31 against ReDMCSB WIP 2021-02-06,
`Toolchains/Common/Source/`, and the local Firestaff source tree. This is an
audit of **original behaviour**, not a list of defects Firestaff should silently
"improve" in preservation mode.

## Reading the status

- **Source modelled** means Firestaff has a source-locked implementation and
  focused test for the control-flow or data rule.
- **Capture pending** means the source rule is known, but no supplied original
  frame/trace proves its result on a selected retail edition.
- **Preserve, do not correct** means ReDMCSB records a historical game bug;
  Original mode must retain the relevant edition's behaviour. A Modern-mode
  correction needs an explicit setting and its own tests.

The previous version of this page called all listed ReDMCSB bugs “NOT FIXED”.
That was inaccurate: it conflated missing frame captures with missing code and
also treated historical behaviour as a Firestaff defect.

## High-value ReDMCSB findings

| Source item | DM1 | CSB | Current Firestaff position | Remaining proof |
|---|---|---|---|---|
| `BUG0_02` 24-bit timeline wrap | affected | affected | Capture pending; no claim that long-session wrap is reproduced or corrected | Original long-session trace/save near the 24-bit boundary |
| `BUG0_03` VBlank palette race | affected Atari ST | fixed by `CHANGE7_01_FIX` | CSB source modelled by the Atari-ST VBlank delivery path and nested-arrival regression; DM1 needs original ST capture before a timing-parity claim | DM1 and CSB low-light frame traces under load |
| `BUG0_04` / `BUG7_01` creature palette quirks | affected | affected | Source modelled palette-selection rules; preserve, do not correct | Original map-specific RGB/frame captures, especially CSB Worm maps 0, 4 and 9 |
| `BUG0_05` portrait-sensor wall ordering | affected | affected | Capture pending | Original/custom-dungeon frame that exposes the ordering |
| `BUG0_06` cropped flipped projectile/explosion | affected | affected | Preserve, do not correct; render-plan coverage exists, but no source-owned edge capture validates final pixels | Left-edge projectile and explosion captures for each selected edition |
| `BUG0_07` cropped unflipped explosion | DM 1.0 only; fixed in DM 1.1+ | fixed | Version-specific rule; do not apply a DM 1.0 defect to PC 3.4, Atari 1.1+, or CSB | Version-labelled original edge capture |
| `BUG0_08` square-first-Thing overflow | affected | affected | Bounded Firestaff data handling fails safely rather than writing outside memory; this is intentional host safety, not preservation evidence | A documented Modern safety policy if compatibility mode is ever exposed |
| `BUG0_64` floor ornament/open-pit order | affected | affected | Source draw order is recorded; capture pending | Original frame/capture pair |
| `BUG0_83` Thieves Eye/door relation | affected | n/a | Capture pending | Original animation capture |
| `BUG0_86` custom-dungeon portrait resource condition | affected | affected | Capture pending | Original custom-dungeon corpus |
| `PANEL.C` F0352/F0353 eye hold/release lifecycle | affected | affected | Source modelled — C071 records the held-eye state; mouse release restores F0347's normal inventory panel, closes the transient eye-opened chest, and reopens an action-hand container through ordinary F0333 | Original press-and-release capture for each selected edition |

## Specific corrections made by this audit

1. CSB's `CHANGE7_01_FIX` must not be reported as unimplemented. The
   source-modelled VBlank route is tested separately; frame-perfect parity is
   still open because the required original gameplay captures are absent.
2. `BUG0_07` is edition-specific. ReDMCSB marks it fixed in DM 1.1 and later,
   including the PC 3.4/CSB families Firestaff currently targets. It is wrong
   to label it an open PC 3.4 defect.
3. The CSB dialog engine-version surface is source modelled and covered by
   `test_csb_v1_graphics_extras_pc34_compat` and the boot/title handoff test;
   it is not an open graphics gap.
4. No historical ReDMCSB bug is silently “fixed” in Original mode. Firestaff
   may safely reject malformed media or prevent host-memory corruption, but
   that safety boundary is documented separately from preservation parity.
5. The DM1 D0/D1 wall and door source locks no longer mistake unavailable
   private media extractions or third-party source trees for renderer failures.
   They now require the named ReDMCSB anchors, Firestaff's corresponding
   metadata, and the focused built viewport regression. This exposed and fixed
   a real test-fixture defect: a D2-only door-frame provider left newly added
   D1 graphic pointers uninitialised, allowing a D1 draw pass to dereference
   garbage. The provider is now fully zero-initialised before its D2 fixtures
   are installed. This is a deterministic test repair, not synthetic game
   data and not a claim of capture-level pixel parity.
6. `PANEL.C:F0352` is a held control, not a one-shot object-description
   command. ReDMCSB dispatches `F0353_INVENTORY_DrawStopPressingEye` on
   release; Firestaff now models that lifecycle before generic drag routing.
   This clears transient chest/action-hand state without changing preserved
   game-data behaviour.

## Evidence locations

- ReDMCSB `DUNVIEW.C:498-525`, `5854`, and `6168-6170` — creature and
  projectile/explosion behaviours.
- ReDMCSB `DUNGEON.C:1825` — square-first-Thing overflow.
- ReDMCSB `BASE.C` and `CHANGE7_01_FIX` — CSB VBlank change.
- `docs/csb_gap_graphics.md` — CSB graphics source mappings.
- `TODO-dm1.md` and `TODO-csb.md` — only open real-data/capture work.
