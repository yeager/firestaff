# Firestaff game-data setup

Firestaff contains no game assets. Supply media from copies you own and point
the launcher at the folder that contains them. The scanner identifies content
by hash, so filenames and the surrounding folder layout are only suggestions.

No BIOS, firmware, System Card, operating-system ROM, or external emulator is
part of the setup or is consulted by the production launcher. Game data is the
complete runtime input contract.

Keep original archives and disc images intact. Firestaff can scan supported
loose files, ZIP archives and disc-image containers directly; do not unpack
your game collection just to make it visible to the launcher. A recognised
file is a launch-data check, not a claim that every game is already playable.
See the [project status](PROJECT_STATUS.md) for that boundary.

For the exact container and record families that Firestaff currently knows,
read the [game-data format reference](GAME_DATA_FORMATS.md). It distinguishes
decoded data from runtime-bound data and records the formats that deliberately
remain opaque or closed.

## Start requirements

These are the source-data roles that must be present before the launcher can
admit a game. A matching name alone is never enough: the bytes must match a
catalogued original edition.

| Game | Required source data | Accepted editions and containers |
|---|---|---|
| Dungeon Master | `GRAPHICS.DAT` and `DUNGEON.DAT` from the same edition | The catalog currently includes PC 3.4, Atari ST, FM Towns and Amiga editions; loose files or supported original-media containers |
| Chaos Strikes Back | `GRAPHICS.DAT` or `CSBGRAPH.DAT`, plus `DUNGEON.DAT` from the same edition | The catalog currently includes PC 3.4, Atari ST, FM Towns and Amiga editions; loose files or supported original-media containers |
| Dungeon Master II: Skullkeep | `GRAPHICS.DAT` and `DUNGEON.DAT` from the same edition | PC English/French/JewelCase, PC-9801 demo, PC-9821 Japanese, FM Towns Japanese and Amiga English (68020+, OCS/ECS-compatible). FM Towns uses `DATA/` on the original CD; the Amiga installer archive is read as original media. |
| DM Nexus | The original Saturn `DM.BIN` marker plus the associated original Saturn data source | Keep a complete CUE/BIN disc image. Firestaff can also identify an already-extracted, verified Saturn data set, but extraction is not a setup requirement. |
| Theron's Quest | One hash-recognised Track 02 data image | US or Japanese Track 02 in its original BIN or ISO form. Keep the matching CUE with BIN media when it is available. |

For the three DAT-based games, both files are mandatory. Do not mix
`GRAPHICS.DAT` from one release with `DUNGEON.DAT` from another. The launcher
rejects an incomplete or mismatched pair instead of guessing a compatible
version.

## Optional original media

Optional means that the file is not needed to identify the basic launch data.
It can still improve a presentation path, enable a language or make an
original save available. Keep it with the matching edition when you have it.

### Dungeon Master

`TITLE`/`TITLE.DAT` and `SWOOSH`/`SWOOSH.DAT` contain original startup and FTL
presentation media. `SONG.DAT` supplies the original music where that edition
includes it. The PC multilingual release can additionally provide
`DUNGEONF.DAT` and `DUNGEONG.DAT` for French and German dungeon text; the
English `DUNGEON.DAT` remains the base file.

### Chaos Strikes Back

Original title, animation and utility media are optional to the data gate.
Their names differ by platform and include `SWOOSH`, `TITL.DAT`, `ENDA.DAT`,
`HCSB.HTC`, `HCSB.DAT`, `ANIMATE.DAT`, `ANIMATE.SCR`, `ANIMATE.FTL`,
`CHAOS.FTL` and `SWITCH.DAT`. Keep campaign and utility saves such as
`MINI.DAT`, `MINIF.DAT`, `MINIG.DAT` and `CSBGAME.DAT` only if you want to
resume a source-supported save. They are never a substitute for the required
graphics and dungeon pair.

### Dungeon Master II: Skullkeep

DM2 is playable from the authenticated DOS, Amiga, FM Towns and Macintosh
source families. M12 keeps the selected edition's owner when several versions
are present below one root; a DOS `data` symlink is valid when it resolves to
the verified DOS files.

PC music is stored in the original GDAT data. FM Towns presentation media,
including `AUTOEXEC.BAT`, `TWANIM.EXP`, `SWOOSH`, `TITLE`, `SKULL.EXP` and
`END`, belongs with the original disc archive. Firestaff reads the MODE1/2352
disc image and animation streams in memory. For English FM Towns text, also
provide the verified PC English `GRAPHICS.DAT` companion; it localises text
only and never replaces the Japanese FM Towns dungeon or presentation owner.

The Amiga installer archive is read as the original in-memory LZX source. The
Mac retail/demo ZIP keeps its HFS/resource-fork owner and big-endian dungeon
records. Do not mix `GRAPHICS.DAT` and `DUNGEON.DAT` across editions.

The Amiga edition's `CD.DAT` and `SK00.MOD` through `SK09.MOD` are its
original map-music data. Keep them with the six-disk installer archive for
the Amiga music route. SKSAVE files are optional resume media; they are not
required for a new game and do not relax the source-data gate.

### DM Nexus

The original Saturn disc also contains level, model, menu and sound resources,
including `LEV*.DGN`, `*.MNS`, `MENU.BPK`, `FACE.BIN`, `STABG.BIN`, `SLEV*.BIN`
and `SNDLEV*.SAL`/`SNDLEV*.MAP`. Keep the complete data track rather than
collecting individual files. Audio CD tracks are preserved with the original
disc image but are not part of the current data-admission check.

### Theron's Quest

Track 02 is the required game-data track. Other tracks, including Track 19
metadata/audio media, are optional to admission. Preserve the complete CUE/BIN
set when you have it so original track layout and future media routes remain
available.

## Suggested layout

The launcher searches recursively, so this layout is only for convenience:

```
~/.firestaff/data/
  dm1/
  csb/
  dm2/
  nexus/
  theron/
```

Use the launcher setting or `--data-dir DIR` to choose another root. Check
what Firestaff recognises without launching a game:

```bash
firestaff --scan-data
firestaff --data-dir /path/to/your/games --scan-data
```

### Scan cache

Firestaff stores verified file hashes in its local scan cache.  On a later
scan of the same directory it reuses that inventory instead of hashing every
file again.  Each cached result is rechecked against its path, size and
modification time, so removed, replaced or changed files are scanned again
automatically.  The cache grows with the library and is local-only; it never
contains game data and must not be committed.

Do not commit game data, saves or disc images to the Firestaff repository.
