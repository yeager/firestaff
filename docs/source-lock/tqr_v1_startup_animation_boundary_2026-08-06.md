# Theron V1 startup animation boundary

The current Theron startup path has two different kinds of evidence that must
not be conflated:

- `theron_v1_startup_media.c` binds authenticated Track 02 bitmap samples,
  the four startup atlas routes, real font tiles, and a variant-specific
  palette candidate from the supplied US/JP BIN data.
- The former M11 eight-frame, six-tick title gate has been removed. It never
  selected a different source frame and was therefore synthetic timing, not
  animation evidence. M11 now exposes one source-backed title frame and
  accepts menu input immediately.

This is intentionally not promoted as original animation parity. The real
Track 02 atlas currently proves indexed startup pixels and their source byte
spans, but not a game-owned frame table, VBlank cadence, animation command, or
VDC destination sequence. The static title is therefore an explicit evidence
boundary, not a source-derived animation claim.

The next valid promotion requires one of the following, tied to the same
authenticated JP/US media identity:

1. a disassembly-backed frame/command table whose source consumer selects the
   startup frame and cadence; or
2. an original emulator/app capture that binds executing code, VDC/VCE writes,
   frame changes, and the corresponding Track 02 byte spans.

Until that evidence exists, do not crop the 96×8 atlas strip into guessed
frames, invent a reveal effect, or replace the static source-backed pixels
with procedural animation. The focused launcher and palette regressions prove
the current boundary only: startup media is real, while animation semantics
remain gated.

Relevant implementation points:

- `src/theron/theron_v1_startup_media.c` — raw startup atlas and palette
  admission.
- `src/engine/m11_game_view.c` — static title presentation and startup input.
- `tests/test_theron_v1_m11_launcher_handoff_boundary.c` — real launcher
  handoff and title readiness receipt.
- `tests/test_theron_v1_startup_media_palette_bind.c` — real US/JP palette
  binding.
- `docs/source-lock/theron-disassembly/` — current HuC6280 consumer evidence;
  it does not yet identify the startup animation consumer.
