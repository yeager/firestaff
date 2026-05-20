# Pass 126: Save Header Audio Metadata

## TODO item

`docs/parity/TODO.md`: `GLOBAL_DATA.GameID` / `MusicOn` carried in `header.reserved[36]` as placeholders, fill them in during the audio phase.

## ReDMCSB source evidence

- `Toolchains/Common/Source/DEFS.H:468-480`: `DM_SAVE_HEADER` includes `GameID`, described as the random value identifying a specific game.
- `Toolchains/Common/Source/DEFS.H:538-572`: `GLOBAL_DATA` includes `MusicOn` under `MEDIA505_F20E_F20J_P20JB_I34E_I34M_F31E_F31J_P31J`.
- `Toolchains/Common/Source/LOADSAVE.C:1546-1548`: save writes `G2024_B_PendingMusicOn` into `L1348_s_GlobalData.MusicOn`.
- `Toolchains/Common/Source/LOADSAVE.C:1590-1594`: save writes `SaveAndPlayChoice` and `G0525_l_GameID` into the save header.
- `Toolchains/Common/Source/LOADSAVE.C:2418`: new games generate `G0525_l_GameID` from two `M006_RANDOM(65536)` calls.
- `Toolchains/Common/Source/LOADSAVE.C:2665-2677`: load reads the save header, rejects restart loads with a mismatched `GameID`, then restores `G0525_l_GameID`.
- `Toolchains/Common/Source/LOADSAVE.C:2721-2748`: load reads `GLOBAL_DATA` and restores `G2024_B_PendingMusicOn` from `MusicOn`.

## Firestaff implementation

Firestaff keeps the 64-byte Phase 15 save header and uses the existing `reserved[36]` block:

- `reserved[0..3]`: `GameID`, little-endian `uint32_t`.
- `reserved[4]`: `MusicOn`, normalised to `0` or `1`.
- `reserved[5..35]`: still reserved / zero unless a future source-locked field claims it.

The new `F0790/F0791/F0792` helpers pack and read the metadata, and `F0773` now carries caller-provided `header.reserved` bytes into the written header so save -> load -> save preserves the pair bit-for-bit.
