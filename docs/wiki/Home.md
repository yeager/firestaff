# Firestaff Wiki

> **Status reviewed 2026-08-06.** Firestaff has five active game targets.
> DM1 V1 is the strongest playable route; CSB, DM2, Nexus and Theron are
> documented as source-locked or real-data bring-up work where the end-to-end
> boundary is still open. See [Project status](../PROJECT_STATUS.md) and the
> [documentation index](../DOCUMENTATION_INDEX.md) for the canonical matrix.

Firestaff reimplements five classic Dungeon Master game engines with source-level parity to the originals. It is a pure C application targeting macOS, Linux, Windows, iOS (AltStore Classic sideload), and Android, rendering via SDL3.

## Supported Games

| Game | Platform | Reference Source | Wiki Pages |
|------|----------|-----------------|------------|
| [Dungeon Master (DM1)](DM1-Technical-Reference) | DOS PC 3.4 | ReDMCSB | [Technical Reference](DM1-Technical-Reference), [PC34 Internals](DM1-PC34-Internals), [RE Documentation](DM1-Reverse-Engineering) |
| [Chaos Strikes Back (CSB)](CSB-Technical-Reference) | DOS PC 3.4 | ReDMCSB | [Technical Reference](CSB-Technical-Reference), [FM Towns guide](CSB-FMTowns-Guide), [DSA and Save Internals](CSB-DSA-and-Save-Internals), [RE Documentation](CSB-Reverse-Engineering) |
| [Dungeon Master II (DM2)](DM2-Technical-Reference) | DOS | skproject | [Technical Reference](DM2-Technical-Reference), [GDAT Internals](DM2-GDAT-Internals), [RE Documentation](DM2-Reverse-Engineering) |
| [Theron's Quest](Therons-Quest-Technical-Reference) | PC Engine CD | PC Engine disassembly and CD analysis | [Technical Reference](Therons-Quest-Technical-Reference), [Track 02 Internals](Therons-Quest-Track02-Internals), [RE Documentation](Therons-Quest-Reverse-Engineering) |
| [DM Nexus](Nexus-Technical-Reference) | Sega Saturn | Saturn SH-2 disassembly and retail media analysis | [Technical Reference](Nexus-Technical-Reference), [DGN/PRS3 Internals](Nexus-DGN-and-PRS3-Internals), [SAL/MAP Internals](Nexus-SAL-MAP-Internals), [RE Documentation](Nexus-Reverse-Engineering) |

## Platform Guides

- [macOS](Platform-macOS) — DMG/ZIP install, Homebrew build, Gatekeeper notes
- [Windows](Platform-Windows) — installer/portable ZIP, MSYS2 build
- [Linux](Platform-Linux) — deb/rpm, Steam Deck (pacman/AppImage), build from source
- [iOS](Platform-iOS) — AltStore Classic sideloading, SideStore, game data via Files app
- [Android](Platform-Android) — APK sideloading, game data transfer

## Reverse Engineering

- [Reverse Engineering Index](Reverse-Engineering-Index) — overview of all RE documentation, cross-game formats, parity evidence
- [DM1 RE](DM1-Reverse-Engineering) — 1388 F-numbered functions, DUNGEON.DAT format, creature types, ReDMCSB file map
- [CSB RE](CSB-Reverse-Engineering) — IMG1 graphics format, DSA scripting system, 357 CSB modules
- [DM2 RE](DM2-Reverse-Engineering) — 56 skproject files, GDAT format, symbol audit, hex-offset modules
- [Theron RE](Therons-Quest-Reverse-Engineering) — PC Engine CD layout, Track 02 format, champion/item system
- [Nexus RE](Nexus-Reverse-Engineering) — Saturn disc structure, DGN geometry, BPK/PRS3 archive, MNS materials

## Project Pages

- [Building and Installing](Building-and-Installing) — build from source, install release packages, sideload on iOS/Android
- [Architecture Overview](Architecture-Overview) — layer model, module layout, data ownership
- [Release Process](Release-Process) — versioning, CI/CD, release artifacts
- [Parity Evidence](Parity-Evidence) — how source-lock documents work
- [Game Data](Game-Data) — what original data files are needed and where to place them
- [Documentation Index](../DOCUMENTATION_INDEX) — complete cross-game status and page map

## Quick Links

- [GitHub Repository](https://github.com/yeager/firestaff)
- [Releases](https://github.com/yeager/firestaff/releases)
- [Issue Tracker](https://github.com/yeager/firestaff/issues)
