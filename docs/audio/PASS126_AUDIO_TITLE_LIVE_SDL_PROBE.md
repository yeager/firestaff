# Pass 126 - Title music live SDL opt-in probe

Scope: DM1 V1 title music only. This pass does not cover generic SFX, sound
packs, creature sounds, ambient dungeon audio, or the in-game music toggle.

## ReDMCSB source evidence

- `MUSIC.C:475-492` - guarded track playback: `F0719_PlayMusicTrack` pauses CD
  audio, checks `G2024_B_PendingMusicOn`, validates the track index, then starts
  CD playback and marks `G4085_MUSIC_PLAYING`.
- `MUSIC.C:540-557` - stop/pause path clears `G4085_MUSIC_PLAYING` rather than
  assuming a device is always active.
- `MUSIC.C:568-581` - `F0741_MUSIC_PlayGameMusic` maps a music index through
  `G2038_auc_MusicIndexToMusicTrack` before calling the playback function.
- `MUSIC.C:597-655` - music updates are gated by `G2024_B_PendingMusicOn`,
  delayed by countdown state, and paused when the pending music state is off.
- `DM.C:20-26`, `DM.C:629-636`, `DM.C:762-780`, `DM.C:823-858`, and
  `DM.C:940-1015` - the DOS launcher exposes explicit hardware choices,
  detects available sound hardware, accepts `-s*` overrides, and emits the
  selected runtime sound parameter.
- `ENTRANCE.C:152-168` and `ENTRANCE.C:328-340` - entrance door sounds are SFX
  calls, not title music; they remain outside this pass.
- `TITLE.C:94-115` and `TITLE.C:249-280` - title frontend work loads/draws the
  title graphics and cadence/fade path; it does not directly start music.

## Firestaff lock

- `src/shared/audio_sdl_m11.c:393-402` keeps SDL playback opt-in through
  `FIRESTAFF_AUDIO_ENABLE_SDL`.
- `src/shared/audio_sdl_m11.c:644-727` loads decoded title music while leaving
  the live SDL stream closed unless the opt-in is set.
- `src/shared/audio_sdl_m11.c:923-946` queues title music only when the original
  SONG.DAT phrase is available and the SDL backend is active.
- `probes/m11/firestaff_m11_pass54_song_runtime_probe.c` now covers both the
  disabled gate and the dummy-driver live SDL queue path without relying on
  audible/manual playback.

Verification command on N2: `ctest --test-dir build -R "m11_title_song_(runtime|live_sdl)" --output-on-failure`.
