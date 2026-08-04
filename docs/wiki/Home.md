# Firestaff Wiki

Firestaff reimplements five classic Dungeon Master game engines with source-level parity to the originals. It is a pure C application targeting macOS, Linux, Windows, iOS (AltStore Classic sideload), and Android, rendering via SDL3.

## Supported Games

| Game | Platform | Reference Source | Wiki Pages |
|------|----------|-----------------|------------|
| [Dungeon Master (DM1)](DM1-Technical-Reference) | DOS PC 3.4 | ReDMCSB | [Technical Reference](DM1-Technical-Reference), [PC34 Internals](DM1-PC34-Internals) |
| [Chaos Strikes Back (CSB)](CSB-Technical-Reference) | DOS PC 3.4 | ReDMCSB | [Technical Reference](CSB-Technical-Reference), [DSA and Save Internals](CSB-DSA-and-Save-Internals) |
| [Dungeon Master II (DM2)](DM2-Technical-Reference) | DOS | skproject | [Technical Reference](DM2-Technical-Reference), [GDAT Internals](DM2-GDAT-Internals) |
| [Theron's Quest](Therons-Quest-Technical-Reference) | PC Engine | None | [Technical Reference](Therons-Quest-Technical-Reference), [Track 02 Internals](Therons-Quest-Track02-Internals) |
| [DM Nexus](Nexus-Technical-Reference) | Sega Saturn | None | [Technical Reference](Nexus-Technical-Reference), [DGN/PRS3 Internals](Nexus-DGN-and-PRS3-Internals), [SAL/MAP Internals](Nexus-SAL-MAP-Internals) |

## Project Pages

- [Building and Installing](Building-and-Installing) — build from source, install release packages, sideload on iOS/Android
- [Architecture Overview](Architecture-Overview) — layer model, module layout, data ownership
- [Release Process](Release-Process) — versioning, CI/CD, release artifacts
- [Parity Evidence](Parity-Evidence) — how source-lock documents work
- [Game Data](Game-Data) — what original data files are needed and where to place them

## Quick Links

- [GitHub Repository](https://github.com/yeager/firestaff)
- [Releases](https://github.com/yeager/firestaff/releases)
- [Issue Tracker](https://github.com/yeager/firestaff/issues)
