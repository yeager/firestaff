#ifndef FIRESTAFF_MUSIC_JUKEBOX_M12_H
#define FIRESTAFF_MUSIC_JUKEBOX_M12_H

/*
 * Music Jukebox — M12 launcher feature.
 *
 * A browsable music player for the DM1 soundtrack extracted from
 * SONG.DAT (DM PC v3.4).  Displays the 9 music parts plus the
 * full sequenced arrangement, with transport controls (play/pause,
 * previous/next, stop), volume adjustment, and elapsed time display.
 *
 * Audio data references:
 *   - SONG.DAT items 1..9: SND8-encoded music parts at 11025 Hz
 *   - SONG.DAT item 0: SEQ2 sequence (arrangement order for looping)
 *   - See DM1_SONG_DAT_FORMAT.md for the complete format spec
 *   - Decoded via song_dat_loader_v1.h at runtime
 *
 * The jukebox is purely a UI/state module.  Actual PCM decode and
 * SDL playback are delegated to audio_sdl_m11 and song_dat_loader_v1.
 *
 * Lifecycle:
 *   1. M12_Jukebox_Init()          — reset state
 *   2. M12_Jukebox_SetAvailable()  — mark SONG.DAT presence
 *   3. M12_Jukebox_HandleInput()   — process transport/nav commands
 *   4. M12_Jukebox_Tick()          — advance elapsed time if playing
 *   5. M12_Jukebox_GetSelected()   — query current track for display
 */

#include "song_dat_loader_v1.h"
#include "audio_sdl_m11.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Track types ─────────────────────────────────────────────────── */
typedef enum {
    M12_JUKEBOX_TRACK_ARRANGEMENT = 0, /* Full sequenced playback     */
    M12_JUKEBOX_TRACK_PART,            /* Individual music part 1..9  */
} M12_JukeboxTrackType;

/* ── Playback state ──────────────────────────────────────────────── */
typedef enum {
    M12_JUKEBOX_STOPPED = 0,
    M12_JUKEBOX_PLAYING,
    M12_JUKEBOX_PAUSED
} M12_JukeboxPlayState;

/* ── Repeat mode ─────────────────────────────────────────────────── */
typedef enum {
    M12_JUKEBOX_REPEAT_OFF = 0,   /* Stop after last track            */
    M12_JUKEBOX_REPEAT_ALL,       /* Loop entire playlist             */
    M12_JUKEBOX_REPEAT_ONE,       /* Loop current track               */
    M12_JUKEBOX_REPEAT_COUNT
} M12_JukeboxRepeatMode;

/* ── Single track entry ──────────────────────────────────────────── */
typedef struct {
    const char*            name;         /* Display name (uppercase)   */
    const char*            subtitle;     /* Extra info line            */
    M12_JukeboxTrackType   type;
    int                    partIndex;    /* SONG.DAT item 1..9, or 0
                                            for arrangement           */
    int                    durationMs;   /* Duration in milliseconds   */
} M12_JukeboxTrack;

/* Total tracks: 1 arrangement + 9 individual parts */
#define M12_JUKEBOX_TRACK_COUNT    10
#define M12_JUKEBOX_VISIBLE_LINES   8

/* ── Input commands ──────────────────────────────────────────────── */
#define M12_JUKEBOX_INPUT_UP         1
#define M12_JUKEBOX_INPUT_DOWN       2
#define M12_JUKEBOX_INPUT_PLAY_PAUSE 3
#define M12_JUKEBOX_INPUT_STOP       4
#define M12_JUKEBOX_INPUT_PREV       5
#define M12_JUKEBOX_INPUT_NEXT       6
#define M12_JUKEBOX_INPUT_VOL_UP     7
#define M12_JUKEBOX_INPUT_VOL_DOWN   8
#define M12_JUKEBOX_INPUT_REPEAT     9  /* Cycle repeat mode          */
#define M12_JUKEBOX_INPUT_BACK      10  /* Return to menu             */

/* ── Volume limits ───────────────────────────────────────────────── */
#define M12_JUKEBOX_VOL_MIN    0
#define M12_JUKEBOX_VOL_MAX  128
#define M12_JUKEBOX_VOL_STEP   8

/* ── Jukebox state ───────────────────────────────────────────────── */
typedef struct {
    /* Track list */
    int                    selectedIndex;   /* Cursor in track list    */
    int                    scrollOffset;    /* First visible track     */

    /* Playback */
    M12_JukeboxPlayState   playState;
    int                    playingIndex;    /* Currently playing track
                                              (-1 if none)            */
    int                    elapsedMs;       /* Elapsed playback time   */

    /* Sequencer state for arrangement mode */
    int                    seqWordIndex;    /* Current SEQ2 word pos   */

    /* Volume (mirrors audio_sdl_m11 musicVolume, but locally tracked
       so the jukebox UI can display it without audio system queries) */
    int                    volume;          /* 0..128                  */

    /* Repeat mode */
    M12_JukeboxRepeatMode  repeatMode;

    /* Data availability */
    int                    songDatAvailable;/* Non-zero if SONG.DAT
                                              was found and parsed    */
} M12_JukeboxState;

/**
 * Initialize jukebox state to defaults (stopped, first track selected,
 * default volume, repeat off).
 */
void M12_Jukebox_Init(M12_JukeboxState* jb);

/**
 * Mark whether SONG.DAT is available.  When unavailable, the jukebox
 * shows track info but disables playback controls.
 */
void M12_Jukebox_SetAvailable(M12_JukeboxState* jb, int available);

/**
 * Total number of tracks in the jukebox.
 */
int M12_Jukebox_TrackCount(void);

/**
 * Get track entry by index (0-based).  Returns NULL if out of range.
 * Index 0 = full arrangement, indices 1..9 = individual music parts.
 */
const M12_JukeboxTrack* M12_Jukebox_GetTrack(int index);

/**
 * Get the currently selected (cursor) track.
 */
const M12_JukeboxTrack* M12_Jukebox_GetSelected(
    const M12_JukeboxState* jb);

/**
 * Get the currently playing track, or NULL if stopped.
 */
const M12_JukeboxTrack* M12_Jukebox_GetPlaying(
    const M12_JukeboxState* jb);

/**
 * Scroll the track list by delta entries (positive = down).
 * Clamps to bounds.
 */
void M12_Jukebox_Scroll(M12_JukeboxState* jb, int delta);

/**
 * Process an input command.  Returns 1 if the jukebox should close
 * (M12_JUKEBOX_INPUT_BACK), 0 otherwise.
 *
 * Play/pause toggles playback of the selected track.
 * Prev/next move to adjacent tracks and begin playing them.
 * Volume up/down adjust in M12_JUKEBOX_VOL_STEP increments.
 */
int M12_Jukebox_HandleInput(M12_JukeboxState* jb, int input);

/**
 * Advance playback timer by deltaMs milliseconds.  Call once per
 * frame/tick while the jukebox is visible.
 *
 * When a track finishes:
 *   - REPEAT_ONE: resets elapsed to 0, continues playing
 *   - REPEAT_ALL: advances to next track (wraps around)
 *   - REPEAT_OFF: advances to next track, stops at end of list
 *
 * Returns non-zero if the playing track changed (caller should
 * trigger a new audio playback request).
 */
int M12_Jukebox_Tick(M12_JukeboxState* jb, int deltaMs);

/**
 * Return the display name for the current repeat mode.
 */
const char* M12_Jukebox_RepeatModeName(M12_JukeboxRepeatMode mode);

/**
 * Return a formatted elapsed/duration string for the currently
 * playing track (e.g. "0:12 / 2:39").  Writes into the provided
 * buffer.  Returns buf on success, or "" if nothing is playing.
 */
const char* M12_Jukebox_FormatTime(const M12_JukeboxState* jb,
                                   char* buf, int bufSize);

/**
 * Return a volume bar string representation (e.g. "####------------")
 * for the current volume level.  Writes into buf.
 */
const char* M12_Jukebox_FormatVolumeBar(const M12_JukeboxState* jb,
                                        char* buf, int bufSize);

/**
 * Return the playback state as a display string:
 *   STOPPED  -> "[]"
 *   PLAYING  -> ">"
 *   PAUSED   -> "||"
 */
const char* M12_Jukebox_PlayStateIcon(M12_JukeboxPlayState state);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_MUSIC_JUKEBOX_M12_H */
