#include "music_jukebox_m12.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/* ── DM1 Soundtrack Track Database ────────────────────────────────
 *
 * Data sourced from SONG.DAT DM PC v3.4 (EN).
 * See DM1_SONG_DAT_FORMAT.md for the complete format specification.
 *
 * Duration values are derived from the verified SND8 sample counts
 * in DM1_SONG_DAT_FORMAT.md section 4, using the 11025 Hz sample rate.
 *
 * The "Full Arrangement" track represents the SEQ2 sequence from
 * item 0: parts 1-2-3-2-3-2-3-2-4-5-6-2-3-2-4-5-7-8-9 then loop.
 * Total duration before first loop is approximately 19.9 seconds
 * of raw PCM.
 *
 * Sequence arrangement total:
 *   Part 1 x1 = 577ms, Part 2 x5 = 13265ms, Part 3 x3 = 2661ms,
 *   Part 4 x2 = 6872ms, Part 5 x2 = 7902ms, Part 6 x1 = 634ms,
 *   Part 7 x1 = 48ms,   Part 8 x1 = 3506ms, Part 9 x1 = 4205ms
 *   Total approximately 39670ms (39.7 seconds with sequence repeats)
 */

static const M12_JukeboxTrack g_jukeboxTracks[M12_JUKEBOX_TRACK_COUNT] = {
    /* Index 0: Full sequenced arrangement */
    {
        "DUNGEON MASTER -- FULL ARRANGEMENT",
        "SEQ2 SEQUENCE: 1-2-3-2-3-2-3-2-4-5-6-2-3-2-4-5-7-8-9 (LOOP)",
        M12_JUKEBOX_TRACK_ARRANGEMENT,
        0,
        39670  /* Total with sequence repeats */
    },
    /* Indices 1..9: Individual music parts from SONG.DAT items 1..9 */
    {
        "MUSIC PART 1 -- INTRO",
        "SONG.DAT ITEM 1  |  6372 SAMPLES  |  11025 HZ",
        M12_JUKEBOX_TRACK_PART,
        1,
        577
    },
    {
        "MUSIC PART 2 -- MAIN THEME A",
        "SONG.DAT ITEM 2  |  29254 SAMPLES  |  11025 HZ",
        M12_JUKEBOX_TRACK_PART,
        2,
        2653
    },
    {
        "MUSIC PART 3 -- MAIN THEME B",
        "SONG.DAT ITEM 3  |  9785 SAMPLES  |  11025 HZ",
        M12_JUKEBOX_TRACK_PART,
        3,
        887
    },
    {
        "MUSIC PART 4 -- BRIDGE A",
        "SONG.DAT ITEM 4  |  37888 SAMPLES  |  11025 HZ",
        M12_JUKEBOX_TRACK_PART,
        4,
        3436
    },
    {
        "MUSIC PART 5 -- BRIDGE B",
        "SONG.DAT ITEM 5  |  43563 SAMPLES  |  11025 HZ",
        M12_JUKEBOX_TRACK_PART,
        5,
        3951
    },
    {
        "MUSIC PART 6 -- INTERLUDE",
        "SONG.DAT ITEM 6  |  7000 SAMPLES  |  11025 HZ",
        M12_JUKEBOX_TRACK_PART,
        6,
        634
    },
    {
        "MUSIC PART 7 -- TRANSITION",
        "SONG.DAT ITEM 7  |  535 SAMPLES  |  11025 HZ",
        M12_JUKEBOX_TRACK_PART,
        7,
        48
    },
    {
        "MUSIC PART 8 -- CODA A",
        "SONG.DAT ITEM 8  |  38656 SAMPLES  |  11025 HZ",
        M12_JUKEBOX_TRACK_PART,
        8,
        3506
    },
    {
        "MUSIC PART 9 -- CODA B",
        "SONG.DAT ITEM 9  |  46365 SAMPLES  |  11025 HZ",
        M12_JUKEBOX_TRACK_PART,
        9,
        4205
    }
};

/* ── Initialization ──────────────────────────────────────────────── */

void M12_Jukebox_Init(M12_JukeboxState* jb) {
    if (!jb) return;
    jb->selectedIndex    = 0;
    jb->scrollOffset     = 0;
    jb->playState        = M12_JUKEBOX_STOPPED;
    jb->playingIndex     = -1;
    jb->elapsedMs        = 0;
    jb->seqWordIndex     = 0;
    jb->volume           = 96;  /* 75% default */
    jb->repeatMode       = M12_JUKEBOX_REPEAT_OFF;
    jb->songDatAvailable = 0;
}

void M12_Jukebox_SetAvailable(M12_JukeboxState* jb, int available) {
    if (!jb) return;
    jb->songDatAvailable = available ? 1 : 0;
}

/* ── Track access ────────────────────────────────────────────────── */

int M12_Jukebox_TrackCount(void) {
    return M12_JUKEBOX_TRACK_COUNT;
}

const M12_JukeboxTrack* M12_Jukebox_GetTrack(int index) {
    if (index < 0 || index >= M12_JUKEBOX_TRACK_COUNT) return NULL;
    return &g_jukeboxTracks[index];
}

const M12_JukeboxTrack* M12_Jukebox_GetSelected(
    const M12_JukeboxState* jb)
{
    if (!jb) return NULL;
    return M12_Jukebox_GetTrack(jb->selectedIndex);
}

const M12_JukeboxTrack* M12_Jukebox_GetPlaying(
    const M12_JukeboxState* jb)
{
    if (!jb || jb->playingIndex < 0) return NULL;
    return M12_Jukebox_GetTrack(jb->playingIndex);
}

/* ── Scrolling ───────────────────────────────────────────────────── */

void M12_Jukebox_Scroll(M12_JukeboxState* jb, int delta) {
    int newSel;
    if (!jb) return;

    newSel = jb->selectedIndex + delta;
    if (newSel < 0) newSel = 0;
    if (newSel >= M12_JUKEBOX_TRACK_COUNT)
        newSel = M12_JUKEBOX_TRACK_COUNT - 1;
    jb->selectedIndex = newSel;

    /* Keep selected index visible within the scroll window. */
    if (jb->selectedIndex < jb->scrollOffset)
        jb->scrollOffset = jb->selectedIndex;
    if (jb->selectedIndex >= jb->scrollOffset + M12_JUKEBOX_VISIBLE_LINES)
        jb->scrollOffset = jb->selectedIndex - M12_JUKEBOX_VISIBLE_LINES + 1;
}

/* ── Internal: start playback of the selected track ──────────────── */

static void jukebox_play_selected(M12_JukeboxState* jb) {
    jb->playingIndex = jb->selectedIndex;
    jb->playState    = M12_JUKEBOX_PLAYING;
    jb->elapsedMs    = 0;
    jb->seqWordIndex = 0;
}

/* ── Internal: advance to next/prev track for transport ──────────── */

static int jukebox_advance_track(M12_JukeboxState* jb, int delta) {
    int next = jb->playingIndex + delta;

    if (next < 0) {
        if (jb->repeatMode == M12_JUKEBOX_REPEAT_ALL) {
            next = M12_JUKEBOX_TRACK_COUNT - 1;
        } else {
            next = 0;
        }
    } else if (next >= M12_JUKEBOX_TRACK_COUNT) {
        if (jb->repeatMode == M12_JUKEBOX_REPEAT_ALL) {
            next = 0;
        } else {
            /* End of playlist, stop. */
            jb->playState    = M12_JUKEBOX_STOPPED;
            jb->playingIndex = -1;
            jb->elapsedMs    = 0;
            return 1;
        }
    }

    jb->playingIndex = next;
    jb->selectedIndex = next;
    jb->elapsedMs    = 0;
    jb->seqWordIndex = 0;

    /* Update scroll to keep playing track visible. */
    if (jb->selectedIndex < jb->scrollOffset)
        jb->scrollOffset = jb->selectedIndex;
    if (jb->selectedIndex >= jb->scrollOffset + M12_JUKEBOX_VISIBLE_LINES)
        jb->scrollOffset = jb->selectedIndex - M12_JUKEBOX_VISIBLE_LINES + 1;

    return 1;
}

/* ── Input handling ──────────────────────────────────────────────── */

int M12_Jukebox_HandleInput(M12_JukeboxState* jb, int input) {
    if (!jb) return 0;

    switch (input) {
    case M12_JUKEBOX_INPUT_UP:
        M12_Jukebox_Scroll(jb, -1);
        break;

    case M12_JUKEBOX_INPUT_DOWN:
        M12_Jukebox_Scroll(jb, +1);
        break;

    case M12_JUKEBOX_INPUT_PLAY_PAUSE:
        if (!jb->songDatAvailable) break;  /* No data, no play */
        if (jb->playState == M12_JUKEBOX_PLAYING) {
            jb->playState = M12_JUKEBOX_PAUSED;
        } else if (jb->playState == M12_JUKEBOX_PAUSED &&
                   jb->playingIndex == jb->selectedIndex) {
            /* Resume the same track. */
            jb->playState = M12_JUKEBOX_PLAYING;
        } else {
            /* Start playing selected track (new or from stopped). */
            jukebox_play_selected(jb);
        }
        break;

    case M12_JUKEBOX_INPUT_STOP:
        jb->playState    = M12_JUKEBOX_STOPPED;
        jb->playingIndex = -1;
        jb->elapsedMs    = 0;
        jb->seqWordIndex = 0;
        break;

    case M12_JUKEBOX_INPUT_PREV:
        if (!jb->songDatAvailable) break;
        if (jb->playState == M12_JUKEBOX_STOPPED) {
            /* Not playing -- just move cursor. */
            M12_Jukebox_Scroll(jb, -1);
        } else {
            /* If more than 2 seconds in, restart current track.
             * Otherwise go to previous. */
            if (jb->elapsedMs > 2000) {
                jb->elapsedMs    = 0;
                jb->seqWordIndex = 0;
            } else {
                jukebox_advance_track(jb, -1);
            }
        }
        break;

    case M12_JUKEBOX_INPUT_NEXT:
        if (!jb->songDatAvailable) break;
        if (jb->playState == M12_JUKEBOX_STOPPED) {
            M12_Jukebox_Scroll(jb, +1);
        } else {
            jukebox_advance_track(jb, +1);
        }
        break;

    case M12_JUKEBOX_INPUT_VOL_UP:
        jb->volume += M12_JUKEBOX_VOL_STEP;
        if (jb->volume > M12_JUKEBOX_VOL_MAX)
            jb->volume = M12_JUKEBOX_VOL_MAX;
        break;

    case M12_JUKEBOX_INPUT_VOL_DOWN:
        jb->volume -= M12_JUKEBOX_VOL_STEP;
        if (jb->volume < M12_JUKEBOX_VOL_MIN)
            jb->volume = M12_JUKEBOX_VOL_MIN;
        break;

    case M12_JUKEBOX_INPUT_REPEAT:
        jb->repeatMode = (M12_JukeboxRepeatMode)(
            ((int)jb->repeatMode + 1) % M12_JUKEBOX_REPEAT_COUNT);
        break;

    case M12_JUKEBOX_INPUT_BACK:
        /* Stop playback on exit. */
        jb->playState    = M12_JUKEBOX_STOPPED;
        jb->playingIndex = -1;
        jb->elapsedMs    = 0;
        return 1;

    default:
        break;
    }

    return 0;
}

/* ── Tick / playback timer ───────────────────────────────────────── */

int M12_Jukebox_Tick(M12_JukeboxState* jb, int deltaMs) {
    const M12_JukeboxTrack* track;
    if (!jb) return 0;
    if (jb->playState != M12_JUKEBOX_PLAYING) return 0;
    if (jb->playingIndex < 0 ||
        jb->playingIndex >= M12_JUKEBOX_TRACK_COUNT) return 0;

    jb->elapsedMs += deltaMs;

    track = &g_jukeboxTracks[jb->playingIndex];
    if (jb->elapsedMs < track->durationMs) return 0;

    /* Track finished -- handle repeat/advance. */
    switch (jb->repeatMode) {
    case M12_JUKEBOX_REPEAT_ONE:
        jb->elapsedMs    = 0;
        jb->seqWordIndex = 0;
        return 1;

    case M12_JUKEBOX_REPEAT_ALL:
        return jukebox_advance_track(jb, +1);

    case M12_JUKEBOX_REPEAT_OFF:
    default:
        if (jb->playingIndex < M12_JUKEBOX_TRACK_COUNT - 1) {
            return jukebox_advance_track(jb, +1);
        }
        /* Last track, stop. */
        jb->playState    = M12_JUKEBOX_STOPPED;
        jb->playingIndex = -1;
        jb->elapsedMs    = 0;
        return 1;

    case M12_JUKEBOX_REPEAT_COUNT:
        /* Shouldn't happen; treat as off. */
        break;
    }

    return 0;
}

/* ── Display helpers ─────────────────────────────────────────────── */

const char* M12_Jukebox_RepeatModeName(M12_JukeboxRepeatMode mode) {
    switch (mode) {
    case M12_JUKEBOX_REPEAT_OFF: return "REPEAT OFF";
    case M12_JUKEBOX_REPEAT_ALL: return "REPEAT ALL";
    case M12_JUKEBOX_REPEAT_ONE: return "REPEAT ONE";
    default:                     return "REPEAT ?";
    }
}

const char* M12_Jukebox_FormatTime(const M12_JukeboxState* jb,
                                   char* buf, int bufSize)
{
    const M12_JukeboxTrack* track;
    int elSec, elMin, durSec, durMin;

    if (!jb || !buf || bufSize < 1) return "";
    if (jb->playingIndex < 0 ||
        jb->playingIndex >= M12_JUKEBOX_TRACK_COUNT) {
        buf[0] = '\0';
        return buf;
    }

    track = &g_jukeboxTracks[jb->playingIndex];

    elSec = jb->elapsedMs / 1000;
    elMin = elSec / 60;
    elSec %= 60;

    durSec = track->durationMs / 1000;
    durMin = durSec / 60;
    durSec %= 60;

    snprintf(buf, (size_t)bufSize, "%d:%02d / %d:%02d",
             elMin, elSec, durMin, durSec);
    return buf;
}

const char* M12_Jukebox_FormatVolumeBar(const M12_JukeboxState* jb,
                                        char* buf, int bufSize)
{
    int segments, filled, i;

    if (!jb || !buf || bufSize < 1) return "";

    /* 16-segment volume bar using ASCII characters.
     * Filled segments use '#', empty use '-'. */
    segments = 16;
    filled = (jb->volume * segments + M12_JUKEBOX_VOL_MAX / 2)
                 / M12_JUKEBOX_VOL_MAX;
    if (filled > segments) filled = segments;
    if (filled < 0) filled = 0;

    if (bufSize < segments + 1) {
        buf[0] = '\0';
        return buf;
    }

    for (i = 0; i < filled; i++)   buf[i] = '#';
    for (     ; i < segments; i++)  buf[i] = '-';
    buf[segments] = '\0';

    return buf;
}

const char* M12_Jukebox_PlayStateIcon(M12_JukeboxPlayState state) {
    switch (state) {
    case M12_JUKEBOX_PLAYING: return ">";
    case M12_JUKEBOX_PAUSED:  return "||";
    case M12_JUKEBOX_STOPPED: return "[]";
    default:                  return "?";
    }
}
