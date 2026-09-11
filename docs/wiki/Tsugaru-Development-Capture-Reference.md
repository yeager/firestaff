# Tsugaru Development and Capture Reference

Tsugaru is an external FM Towns/Marty emulator used only to study licensed
original media and collect local reference evidence. It is not bundled with
Firestaff, invoked by Firestaff, required by players, or a source of runtime
assets. This page describes the tool boundary so capture work is repeatable
without weakening preservation or introducing an emulator dependency.

The reference implementation is [TOWNSEMU](https://github.com/captainys/TOWNSEMU),
which is BSD 3-Clause licensed. Its own stated primary target is the FM Towns
II MX. It implements FM Towns/Marty hardware, rather than merely rendering a
game's files, so it is useful for discovering boot, CD, input, video, and
sound behavior that must subsequently be implemented natively in Firestaff.

## Boundary and evidence policy

| Permitted development use | Never a Firestaff runtime requirement |
|---|---|
| Boot an owner-supplied ROM plus original disc image | Tsugaru executable, GUI, CUI, ROM or firmware |
| Record a local emulator framebuffer, audio trace, state or debugger log | Extracted game media, a capture, state file or debugger log |
| Compare Firestaff's independently-native result with a local reference | A dependency on Tsugaru's renderer, CPU, audio, input or CD code |
| Submit an upstream emulator fix after independent validation | Publish proprietary ROMs, game data, raw captures or disassembly |

Local evidence remains outside version control. A receipt may record hashes,
tool revision, machine configuration, media family, timestamps, and validation
outcome, but not a personal filesystem path or copyrighted payload.

## Components

Tsugaru has a GUI frontend and a `Tsugaru_CUI` command-line frontend. The GUI
uses the CUI executable internally. For automated reference work Firestaff
uses CUI because it accepts explicit machine/media arguments and commands such
as `SS` (Save Screenshot). The core is organized around these subsystems:

| Subsystem | Responsibility relevant to Firestaff evidence |
|---|---|
| CPU/fidelity layer | 386/486-family guest execution and exceptions; High Fidelity is intended for guests that need fuller protected-mode/task behavior |
| Towns devices | CRTC, VRAM, palette, sprite, CD-ROM, SCSI, FDC, keyboard, mouse, game port, timer, sound and MIDI |
| `TownsThread` | VM run/pause lifecycle, real-time throttling, scheduled devices and render cadence |
| `TownsRender` | CRTC page composition into an emulated framebuffer; `SS` calls `RenderQuiet`, not a desktop screenshot |
| command interpreter | `RESET`, `RUN`, `SS`, `QSS`, state/debugger and file-transfer commands |
| argument parser | ROM/media/machine/input/timing/profile command-line configuration |

The CUI command loop and VM run on separate threads. Treat host-side command
delivery and emulator process shutdown as tooling concerns; only a validated
emulated framebuffer/audio/state is reference evidence.

## ROMs and machine identity

Tsugaru consumes a directory of ROM files compatible with the UNZ convention.
The exact ROM revision and selected Towns model materially affect boot, BIOS
services, font behavior, I/O identity, CPU/memory mapping and timing. A
capture receipt therefore includes ROM hashes and the chosen `-TOWNSTYPE`.

Supported model selectors include `MODEL2`, `2F`, `20F`, `UX`, `CX`, `UG`,
`HG`, `HR`, `UR`, `MA`, `MX`, `ME`, `MF`, `HC`, and `MARTY`. The model changes
the machine-id response and, for UX/UG, memory mapping. A Marty ROM also
requires `-TOWNSTYPE MARTY`; supplying the ROM alone does not select Marty
hardware.

Do not use a different model merely to obtain a visually convenient frame.
For a parity claim, preserve a machine/ROM/media tuple and document why that
tuple matches the released game edition.

## Media and preservation

Tsugaru accepts CD images including ISO, CUE and MDS; its project documentation
recommends MDS/MDF or CCD-style descriptions for discs with audio. CUE files
can encode pregaps ambiguously, so two CUE generators may describe the same
disc with different audio-track positions. Firestaff's capture harness accepts
an original CUE plus its exact referenced track image only when that pair is
the available real medium, hashes both, and never converts it into a new game
image.

For stronger preservation evidence prefer an original multi-track descriptor
that preserves index/pregap information. In all cases:

1. Keep the supplied archive/media immutable.
2. Stage only a private, disposable development copy when the emulator needs
   seekable adjacent CUE/BIN or CUE/IMG files.
3. Preserve the selected descriptor-to-track relation in the receipt.
4. Reject a capture if media loading, VM execution or framebuffer validation
   fails.

CD, floppy, SCSI CD and hard-disk options are independent. `-CD` selects the
internal CD path; `-SCSICD0` through `-SCSICD6` select SCSI devices. `-FD0`
through `-FD3` attach floppy images. A Towns boot choice must match the device
actually attached.

## Boot behavior

`-BOOTKEY` emulates hardware-time startup key combinations. Common values are
`CD` (C+D, internal CD), `F0`–`F3` (floppy), `H0`–`H4` (SCSI), `ICM`, `DEBUG`,
`PADA`, `PADB`, `PADAB`, `FAST`, and `SLOW`. This is not interchangeable with
typing keys after a game has begun.

The relevant implementation facts are:

- `FMTownsCommon::Reset` resets keyboard and game-port device state.
- The keyboard emits boot scan-code pairs when the BIOS polls port `0x600` and
  reports data-ready through `0x602`.
- `NotifyDiskRead` ends the boot-key sequence after the first disk read.
- The CUI `RESET CD` command is useful for reproducing a fresh, explicit
  internal-CD boot in an interactive investigation.

Consequently a capture runner must prove that its launch/setup ordering
preserves the requested boot combination after the final VM reset. A black
BIOS framebuffer is not proof of a successful CD boot.

## Timing and CPU fidelity

Tsugaru starts with no-wait behavior, then normally changes its timing policy
after the first disk read. `-NOWAITBOOT` bypasses the host-time memory-test
delay but retains the normal post-read policy. `-NOWAIT` leaves the VM
unthrottled and is useful for diagnosis; it must be explicitly recorded because
host elapsed time no longer represents original wall-clock timing. `-YESWAIT`
and the normal timing path are appropriate when measuring presentation timing.

The default fidelity layer optimizes paths commonly used by Towns software.
`-HIGHFIDELITY` enables fuller CPU behavior and is recommended by Tsugaru for
Windows 3.1/95 and guests needing more complete exception/task handling. It
does not automatically make a capture authentic: a capture still needs a
matching ROM/model/media tuple and a stable, observable boot.

For Firestaff, timing measurements must distinguish:

| Claim | Required evidence |
|---|---|
| Frame content | Native framebuffer capture and source-media receipt |
| Frame ordering | Timestamped frame sequence plus emulator timing mode |
| CDDA/audio behavior | Original audio capture, track/index identity and no emulator audio warning |
| Original speed | A throttled/reference-time session, not merely `-NOWAIT` output |

## Video and screenshots

The Towns CRTC can expose one or two display pages. `TownsRender::Prepare`
reads CRTC mode, page visibility, layer geometry, palette and priority;
`RenderQuiet` then composes emulated VRAM into an image. `SS file` invokes
this path and writes a PNG. `SS file 0` or `SS file 1` selects a page in
two-page mode. The command is preferable to an X11/Wayland screenshot because
it excludes host cursor, compositor, title bar and control overlays.

Firestaff's capture rule is deliberately strict:

- Require every requested `SS` image.
- Reject zero-size, one-colour and all-black images.
- Reject a session that reports VM abort.
- Record image dimensions/hash and backend in the local receipt.
- Do not promote a layer dump, an emulator window image, or a host fallback to
  a composite-game parity frame.

An all-black output commonly means that the CRTC has not enabled a display
page yet; it does not establish that a title's black screen is authentic.
`DUMP CRTC`, `DUMP CDROM`, `DUMP STATUS`, `DUMP SCHEDULE`, and the debugger
are diagnostic tools for separating a BIOS/boot problem from a rendering
problem.

## Input, mouse and game ports

FM Towns uses a relative MSX mouse. Tsugaru's integrated mouse support depends
on knowing the guest cursor convention for a given TBIOS/application; a middle
button can switch to differential integration. Capture automation must not
equate an X11 coordinate with a source-game coordinate until it has validated
the active Towns video mode, scaling, page origin and mouse integration mode.

Game-port values include keyboard emulation and physical/analogue pads. They
are useful to reproduce original pad-only paths, but Firestaff must implement
the resulting game behavior natively rather than importing an emulator input
mapping.

## Audio

Tsugaru models FM/PCM sound, CDDA and MIDI-related devices. Sound has its own
timing/buffering behavior; an audio artifact is usable only when the emulator
reports no dropped/incorrect samples and the capture identifies the attached
disc tracks. Do not infer a missing Firestaff sound route from a silent or
buffer-underflowed emulator session.

## States, debugger and transfer facilities

Tsugaru supports loadable states, quick-state controls, command-line initial
commands, debugger dumps/breakpoints, memory dumps, screenshot directories,
event logs and VM↔host transfer facilities (TGDRV, XMODEM and its transfer
protocol). They are valuable for locating a deterministic original state, but
their outputs are analysis artifacts:

- State files are emulator-specific and do not prove a native Firestaff save
  format.
- Debugger addresses, logs and memory dumps stay local and out of Git.
- Shared-folder/file-transfer features are never enabled for an ordinary
  evidence capture unless the transfer itself is the object of the test.
- Network forwarding (`-RSTCP`, NAT/port forwarding) is unnecessary for game
  capture and should remain disabled; its own documentation warns that traffic
  is unencrypted.

## Firestaff capture procedure

The maintained helper is `scripts/capture_fmtowns_original_startup.sh`. It:

1. Requires caller-supplied licensed ZIP media, ROM directory and an explicit
   timestamp timeline.
2. Extracts only into a disposable ignored development area because CUE media
   requires co-located seekable track files.
3. Constructs a private case-normalized ROM view without renaming/copying the
   caller's firmware.
4. Uses CUI `SS` for the emulated framebuffer; it never accepts a desktop
   capture.
5. Uses `FORCEQUIT` after the last requested frame because the asynchronous
   CUI ordinary teardown is unsuitable as a capture-success signal.
6. Fails closed on missing frames, blank frames, VM abort, invalid media shape
   or non-zero process status.
7. Writes a local receipt only after all validation succeeds.

The helper currently supports DM1 and CSB FM Towns CUE/BIN and CUE/IMG media.
It does not establish Firestaff visual parity by itself; it provides a
reproducible original-reference input to a separate native comparison.

## Known investigation boundaries

Tsugaru is actively maintained and its own documentation notes incomplete
CPU/instruction coverage outside the common Towns software path. Treat an
emulator failure as an investigation lead, not as evidence that the original
game behaves the same way. Record failures in a local receipt, retain only
their non-proprietary diagnosis, and continue with another source or emulator
when appropriate.

For the current DM1/CSB capture effort, a frame is promoted only after all of
the following are true: the correct original media is mounted; the requested
boot path is observed; CRTC output is nonblank; the VM reports no abort; the
capture has a complete receipt; and Firestaff's native output is compared at
the same defined state.

## Related Firestaff documentation

- [DM1 FM Towns guide](DM1-FMTowns-Guide)
- [CSB FM Towns guide](CSB-FMTowns-Guide)
- [Game Data](Game-Data)
- [Parity Evidence](Parity-Evidence)
- [Preservation](Preservation)
