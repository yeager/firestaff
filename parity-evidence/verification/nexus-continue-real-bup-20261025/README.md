# Nexus retail Continue and save-load evidence (2026-10-25)

## Purpose

Record the authentic Japanese retail startup path from the title menu through
Continue and into gameplay with a real Saturn backup image. This is Ymir
reference evidence; it does not claim Firestaff parity.

## Inputs and environment

- Emulator: Ymir 0.4.1, launched with `--debug` on TRV2 (`192.168.3.120`).
- Disc: `Dungeon Master Nexus (Japan).cue`, SHA-256
  `878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699`.
- BIOS: authenticated Japanese Saturn BIOS, SHA-256
  `dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`.
- Save: authentic retail-written four-champion BUP, 32,768 bytes, SHA-256
  `87d041baecca76a629b18b34e13b47adca1d5995a69bcac769aaca54c8f34035`.
- Ymir profile: `/dev/shm/ymir-nexus-real-bup-menu-20261025`, isolated from
  the user's normal profile. The BUP was copied into this profile; its hash
  was checked before and after the run and remained unchanged.
- The BIOS, disc image, and BUP payload are not included in this evidence
  directory.

## Observed sequence

1. The retail title menu displayed `NEW GAME`, `CONTINUE`, and
   `CONFIGURATION`.
2. `F` held for about one second opened the menu. `S` moved selection to
   `CONTINUE` (the profile maps these keys to Start and D-pad Down).
3. `J` held for about half a second confirmed Continue and opened the native
   Japanese `LOAD` panel. It listed the internal-memory entry `B01 0:06
   NORMAL`.
4. `J` held for about one second confirmed the entry. The screen displayed
   `Now Loading`; approximately nine seconds later the game showed a dungeon
   view with the four-champion HUD.

The key durations are host-time observations, not frame-exact input
thresholds. This demonstrates the retail Continue/save-load path in Ymir. It
does not demonstrate Firestaff title-menu rendering, BUP parsing, or parity.

## Captures

| File | Observation | SHA-256 |
|---|---|---|
| `01-title-menu.png` | Retail title menu, New Game selected | `1564ea2a4c307a31988231e268ecf0005c1aa9ac96e7ec22a0c0fbe97e4ad6b3` |
| `02-continue-selected.png` | Continue selected | `2b4e16d9a1f1e5fd9ac2ee8e90c6e3b68caa8759937aa9bb9cfa1438655dc2f3` |
| `03-load-screen.png` | Native Load panel and internal-memory slot | `5e20c31980926e599343b25dbdc1bc75b945ee382e9c313439996fc08b633444` |
| `04-loaded-gameplay.png` | Loaded dungeon and party HUD | `d6e963e73ccd7b0c7c66a9482175106f97d3c5b1369951d0999417a7366afb0b` |
