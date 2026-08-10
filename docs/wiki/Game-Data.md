# Game data

Firestaff reads original media supplied by the player. The launcher identifies
editions by content hash, not by a filename or folder convention, and it does
not include any game data itself.

Start with [Game-data setup](https://github.com/yeager/firestaff/blob/main/docs/DATA_SETUP.md) when preparing a data folder.
It explains the required paired media, supported archive and disc containers,
and the role of optional presentation and save files.

The detailed [game-data format reference](https://github.com/yeager/firestaff/blob/main/docs/GAME_DATA_FORMATS.md) is the
technical companion to that guide. It covers the current implementations of:

- DM1 and CSB IMG3, IMG1, DMCSB1/LZW and FM Towns IMG2 graphics;
- DM1/CSB dungeon data, platform program media, campaign files and save
  families, including the distinct FM Towns and CSBWin boundaries;
- DM2 GDAT, G1 dungeon records, GDAT PCM and platform media;
- Nexus DGN/DMDF, MNS, PRS3, BGR555, SAL and MAP evidence; and
- Theron's Quest raw MODE1/2352 Track 02, record framing and SRM saves.

The format reference labels each path as read, runtime-bound, opaque or
closed. A parsed file does not automatically mean a playable route. The
[project status](https://github.com/yeager/firestaff/blob/main/docs/PROJECT_STATUS.md) records the current runtime boundary
for each game.
