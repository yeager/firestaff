#ifndef FIRESTAFF_SOUNDTRACK_SELECTOR_M11_H
#define FIRESTAFF_SOUNDTRACK_SELECTOR_M11_H

/*
 * soundtrack_selector_m11 — soundtrack profile selector.
 *
 * Picks where the music for a given logical track name should come
 * from. Three profiles:
 *
 *   0 = Original     : use the original V1 song stream loaded from
 *                      SONG.DAT / GRAPHICS.DAT (no path resolved).
 *   1 = Remastered   : look for a remastered track under the data
 *                      directory (data/music/remastered/<trackName>.ogg)
 *                      and fall back to Original when missing.
 *   2 = Custom Folder: look for <customMusicPath>/<trackName>.ogg|.mp3|.wav
 *                      and fall back to Original when missing.
 *
 * The Original profile never resolves a path (callers keep using the
 * V1 SONG.DAT loader). The other two profiles return a fully-resolved
 * absolute path when a matching file is found.
 *
 * Source: no ReDMCSB equivalent — Firestaff audio extra. Default mode
 * is 0 (Original), so V1 launches stay bit-identical.
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M11_SOUNDTRACK_MODE_ORIGINAL   = 0,
    M11_SOUNDTRACK_MODE_REMASTERED = 1,
    M11_SOUNDTRACK_MODE_CUSTOM     = 2,
    M11_SOUNDTRACK_MODE_COUNT
};

/* Return values from M11_Soundtrack_GetTrackPath. */
enum {
    M11_SOUNDTRACK_RESULT_ORIGINAL  = 0,  /* use built-in V1 path; outPath untouched */
    M11_SOUNDTRACK_RESULT_RESOLVED  = 1,  /* outPath filled with absolute path */
    M11_SOUNDTRACK_RESULT_FALLBACK  = 2   /* requested file missing — fall back to Original */
};

/* Stable English label for the mode (0..2). NULL if out of range. */
const char* M11_Soundtrack_GetLabel(int mode);

/* 1 if the mode is valid (0..M11_SOUNDTRACK_MODE_COUNT-1). */
int M11_Soundtrack_IsValid(int mode);

/*
 * Resolve a track to a playable file path.
 *
 *   mode           : M11_SOUNDTRACK_MODE_*
 *   trackName      : logical name, e.g. "title", "dungeon1" (no
 *                    extension; resolver probes .ogg/.mp3/.wav).
 *   customMusicPath: directory to probe when mode = CUSTOM; may be
 *                    NULL/empty (then CUSTOM degrades to FALLBACK).
 *   outPath        : caller-owned buffer for the resolved path. May be
 *                    NULL if the caller only wants the result code.
 *   outSize        : size of outPath in bytes.
 *
 * Returns one of the M11_SOUNDTRACK_RESULT_* codes.
 *
 *   ORIGINAL → mode was 0 (or trackName was NULL/empty).
 *   RESOLVED → outPath now contains a valid path on disk.
 *   FALLBACK → no matching file found; caller should fall back to
 *              Original. outPath cleared.
 */
int M11_Soundtrack_GetTrackPath(int mode,
                                const char* trackName,
                                const char* customMusicPath,
                                char* outPath,
                                int outSize);

/* Resolve the default custom-music directory. Writes ~/.firestaff/music
 * (with $HOME expanded) into outPath. Returns 1 on success, 0 when no
 * HOME is available. outPath/outSize behave like snprintf. */
int M11_Soundtrack_GetDefaultCustomDir(char* outPath, int outSize);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_SOUNDTRACK_SELECTOR_M11_H */
