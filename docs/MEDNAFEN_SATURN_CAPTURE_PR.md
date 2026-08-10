# Mednafen Saturn frame-capture PR boundary

## Status

Firestaff has a working external Mednafen 1.32.1 witness that captures
authentic Saturn VDP1/VDP2 state. The upstream candidate is now separated from
the Firestaff probe: it uses two non-persistent Saturn settings, serializes
16-bit values explicitly in big-endian order, and contains no Nexus, BIOS,
disc, input-automation, CD, or SH-2 tracing code.

The upstreamable part is a small, opt-in Saturn frame-dump facility. It must
not depend on a game, BIOS, disc image, Firestaff, or a particular asset
format.

## Proposed upstream change

Add an optional Saturn debug capture setting to the Mednafen Saturn core. When
enabled, one or more completed frames are written to a versioned stream at the
top-blanking boundary, before the next frame's renderer work begins.

Each frame record should contain:

- VDP1 VRAM and both framebuffer pages;
- the raw VDP2 register image;
- VDP2 VRAM and CRAM;
- the visible-frame number and draw-buffer selector.

The current candidate uses explicit big-endian 16-bit serialization to retain
the Saturn bus representation. Capture is disabled by default and must not
alter emulation output, timing, input, save states, or normal logging. The
format remains a review candidate until Mednafen maintainers choose whether
the final public interface should be a setting or a debugger command.

## Deliberately excluded

The following remain local investigation tools and do not belong in the
generic Mednafen PR:

- CD sector/LBA traces;
- SH-2 source-read, DMA, RAM-write, or PC traces;
- scripted controller input;
- Nexus file names, PRS3, DGN, DM.BIN, FONT256, SLEV, SAL, or SDDRVS rules;
- BIOS, disc images, raw captures, or extracted retail data.

Those traces establish provenance for Firestaff's source-faithful Nexus
implementation. They are not required to make a general Saturn frame dump
useful to emulator developers.

## Acceptance criteria for the PR

1. A stock Saturn title runs unchanged with capture disabled.
2. Capture enabled produces the same VDP1/VDP2 memory state as the existing
   renderer at the chosen frame boundary.
3. Two runs with identical inputs produce byte-identical frame records.
4. The capture path is bounded by an explicit frame limit and handles
   file-open failures without terminating emulation.
5. The format and option are documented in Mednafen's Saturn documentation.
6. A small host-side reader or test fixture validates header, record sizes,
   and endianness without requiring copyrighted game data.

## Firestaff evidence

The external witness has already produced authenticated Saturn captures with
VDP1/VDP2 registers, VRAM, CRAM, and post-render timing. Nexus PRS3 decoding
and DGN texture decoding pass against the user's verified retail corpus. The
remaining Firestaff work is to bind the active VDP2 character/tile consumer
and VDP1 scene materials to those captures. That work must not be represented
as complete merely because a generic capture file exists.

The implementation can therefore be submitted upstream as two separately
reviewable changes:

1. Mednafen: generic, opt-in Saturn frame capture.
2. Firestaff: Nexus-specific readers, provenance joins, and source-faithful
   rendering gates.
