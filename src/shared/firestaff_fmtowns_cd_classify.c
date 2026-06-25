/*
 * firestaff_fmtowns_cd_classify.c
 *
 * Implementation of the bounded FM Towns CD-image import
 * classifier declared in firestaff_fmtowns_cd_classify.h.
 *
 * Scope:
 *   - Parse redump-style CUE sheets (FILE / TRACK / INDEX /
 *     PREGAP) and produce a fixed-capacity track list.
 *   - Optionally inspect the data track for the ISO 9660 PVD
 *     signature so we can confirm a real disc image was opened.
 *   - Score the parsed layout against the DMWeb FM Towns
 *     CD-audio track tables for DM1 / CSB / DM2.
 *
 * Out of scope (tracked under docs/FIRESTAFF_GAP_LIST.md):
 *   - Extracting ISO 9660 file payloads.
 *   - Decoding IMG2 / GRAPHICS.DAT / DUNGEON.DAT.
 *   - FM Towns keyboard / input bridge.
 *   - Any emulator/runtime launch.
 *
 * Style:
 *   - C99, deterministic. The in-memory CUE parser uses no malloc;
 *     the convenience file loader allocates one bounded 8 MiB buffer.
 *   - All errors return -1 or zero out the output; callers can
 *     safely bail without leaking partial state.
 */

#include "firestaff_fmtowns_cd_classify.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ISO 9660 Primary Volume Descriptor lives at sector 16 (0-based).
 * In raw MODE1/2352 it starts after the sector header at
 * 16*2352+16 bytes; in cooked MODE1/2048 it starts at 16*2048.
 * The descriptor type byte is followed by the five-byte ASCII
 * signature "CD001". */
#define ISO9660_PVD_BYTE_OFFSET_2048  (16u * 2048u + 1u)
#define ISO9660_PVD_BYTE_OFFSET_2352  (16u * 2352u + 16u + 1u)

/* ── Small string helpers ─────────────────────────────────────── */

static int ieq(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return *a == 0 && *b == 0;
}

static int starts_with(const char *s, const char *prefix) {
    while (*prefix) {
        if (*s != *prefix) return 0;
        s++; prefix++;
    }
    return 1;
}

/* Trim leading whitespace in place; returns pointer to first
 * non-whitespace character. */
static const char *ltrim(const char *s) {
    while (*s == ' ' || *s == '\t') s++;
    return s;
}

/* Trim trailing CR/LF and trailing whitespace. Mutates *line_end
 * to the new logical end of the trimmed line. */
static void rtrim(char *line, size_t *line_len) {
    size_t n = *line_len;
    while (n > 0 && (line[n-1] == '\n' || line[n-1] == '\r'
                     || line[n-1] == ' '  || line[n-1] == '\t')) {
        line[--n] = 0;
    }
    *line_len = n;
}

/* Copy a quoted string (between two unescaped double quotes)
 * from src into dst (NUL-terminated, truncated). Returns 0 on
 * success, -1 if no complete quoted string is present. */
static int copy_quoted(const char *src, char *dst, size_t dst_size) {
    const char *q1 = strchr(src, '"');
    if (!q1) return -1;
    const char *q2 = strchr(q1 + 1, '"');
    if (!q2) return -1;
    size_t n = (size_t)(q2 - q1 - 1);
    if (n >= dst_size) n = dst_size - 1;
    memcpy(dst, q1 + 1, n);
    dst[n] = 0;
    return 0;
}

/* Parse a positive integer at *s; advances *s past the digits.
 * Returns -1 if no digits found. */
static int parse_uint(const char **s) {
    if (!**s || !isdigit((unsigned char)**s)) return -1;
    int v = 0;
    while (**s && isdigit((unsigned char)**s)) {
        v = v * 10 + (**s - '0');
        (*s)++;
    }
    return v;
}

/* ── Layout parser ──────────────────────────────────────────── */

void firestaff_fmtowns_cd_classify_zero_layout(
    FirestaffFmtownsCd_Layout *layout) {
    if (!layout) return;
    memset(layout, 0, sizeof(*layout));
}

int FirestaffFmtownsCd_ParseCue(const char *cue_text,
                                size_t cue_len,
                                FirestaffFmtownsCd_Layout *layout) {
    char line[FIRESTAFF_FMTOWNS_CD_MAX_FILE_LINE * 2];
    size_t pos = 0;
    int current_file = -1;
    FirestaffFmtownsCd_Track *t = NULL;

    if (!cue_text || !layout) return -1;
    firestaff_fmtowns_cd_classify_zero_layout(layout);

    while (pos < cue_len) {
        /* Extract one line into the local buffer. */
        size_t i = 0;
        size_t start = pos;
        while (pos < cue_len && cue_text[pos] != '\n' && i + 1 < sizeof(line)) {
            line[i++] = cue_text[pos++];
        }
        if (pos < cue_len && cue_text[pos] == '\n') pos++;
        line[i] = 0;
        size_t line_len = i;
        rtrim(line, &line_len);
        (void)start;

        const char *p = ltrim(line);
        if (*p == 0 || *p == ';') continue;     /* blank or comment */
        if (*p == '#') continue;

        if (starts_with(p, "FILE ")) {
            if (layout->file_count >= FIRESTAFF_FMTOWNS_CD_MAX_FILES) return -1;
            char name[FIRESTAFF_FMTOWNS_CD_MAX_FILE_LINE];
            if (copy_quoted(p, name, sizeof(name)) != 0) return -1;
            current_file = layout->file_count;
            strncpy(layout->files[current_file], name,
                    FIRESTAFF_FMTOWNS_CD_MAX_FILE_LINE - 1);
            layout->files[current_file][FIRESTAFF_FMTOWNS_CD_MAX_FILE_LINE - 1] = 0;
            layout->file_count++;
            t = NULL;
            continue;
        }

        if (starts_with(p, "TRACK ")) {
            const char *q = p + 6;
            int n = parse_uint(&q);
            if (n < 1 || n > 99) return -1;
            if (current_file < 0) return -1; /* TRACK before FILE */
            if (layout->track_count >= FIRESTAFF_FMTOWNS_CD_MAX_TRACKS) return -1;
            t = &layout->tracks[layout->track_count++];
            memset(t, 0, sizeof(*t));
            t->number = n;
            t->file_index = current_file;
            t->kind = FIRESTAFF_FMTOWNS_CD_KIND_UNKNOWN;
            t->mode = FIRESTAFF_FMTOWNS_CD_MODE_UNKNOWN;
            if (n > layout->max_track_number) layout->max_track_number = n;

            /* The remainder of the TRACK line is the mode token. */
            q = ltrim(q);
            if (ieq(q, "MODE1/2352") || ieq(q, "MODE2/2352")) {
                t->kind = FIRESTAFF_FMTOWNS_CD_KIND_DATA;
                t->mode = FIRESTAFF_FMTOWNS_CD_MODE_2352;
            } else if (ieq(q, "MODE1/2048")) {
                t->kind = FIRESTAFF_FMTOWNS_CD_KIND_DATA;
                t->mode = FIRESTAFF_FMTOWNS_CD_MODE_2048;
            } else if (ieq(q, "AUDIO")) {
                t->kind = FIRESTAFF_FMTOWNS_CD_KIND_AUDIO;
                t->mode = FIRESTAFF_FMTOWNS_CD_MODE_AUDIO;
            }
            continue;
        }

        if (starts_with(p, "INDEX ")) {
            if (!t) return -1;
            const char *q = p + 6;
            int idx = parse_uint(&q);
            if (idx < 0 || idx > 99) return -1;
            if (idx == 0) t->has_pregap = 1;
            if (idx == 1) t->has_index01 = 1;
            continue;
        }

        if (starts_with(p, "PREGAP ")) {
            if (!t) return -1;
            /* PREGAP is informational; record that we saw it. We do
             * not currently adjust track numbers based on PREGAP
             * because redump FM Towns discs typically omit it. */
            t->has_pregap = 1;
            continue;
        }

        if (starts_with(p, "TITLE ") || starts_with(p, "PERFORMER ")
            || starts_with(p, "ISRC ") || starts_with(p, "FLAGS ")
            || starts_with(p, "CATALOG ") || starts_with(p, "CDTEXTFILE ")
            || starts_with(p, "POSTGAP ") || starts_with(p, "REM ")) {
            /* Known but unused metadata; ignore. */
            continue;
        }

        /* Unknown directive: be strict in dev to catch typos,
         * but tolerate in production callers. */
        /* return -1; */
    }

    /* Post-validate. */
    if (layout->track_count == 0) return -1;
    if (layout->file_count == 0) return -1;
    if (layout->tracks[0].number != 1) return -1; /* Track 01 must exist */
    if (layout->tracks[0].kind != FIRESTAFF_FMTOWNS_CD_KIND_DATA) return -1;
    if (!layout->tracks[0].has_index01) return -1;

    /* Tally counts. */
    int i;
    for (i = 0; i < layout->track_count; i++) {
        const FirestaffFmtownsCd_Track *tt = &layout->tracks[i];
        if (tt->kind == FIRESTAFF_FMTOWNS_CD_KIND_DATA)  layout->data_track_count++;
        if (tt->kind == FIRESTAFF_FMTOWNS_CD_KIND_AUDIO) layout->audio_track_count++;
    }

    return 0;
}

int FirestaffFmtownsCd_ParseCueFile(const char *cue_path,
                                    FirestaffFmtownsCd_Layout *layout) {
    FILE *fp;
    long fsize;
    char *buf;
    size_t got;
    int rc;

    if (!cue_path || !layout) return -1;
    fp = fopen(cue_path, "rb");
    if (!fp) return -1;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return -1; }
    fsize = ftell(fp);
    if (fsize < 0 || fsize > 8 * 1024 * 1024) { fclose(fp); return -1; }
    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return -1; }
    buf = (char *)malloc((size_t)fsize + 1);
    if (!buf) { fclose(fp); return -1; }
    got = fread(buf, 1, (size_t)fsize, fp);
    fclose(fp);
    if (got != (size_t)fsize) { free(buf); return -1; }
    buf[fsize] = 0;
    rc = FirestaffFmtownsCd_ParseCue(buf, (size_t)fsize, layout);
    free(buf);
    return rc;
}

/* ── ISO 9660 PVD detection ─────────────────────────────────── */

int FirestaffFmtownsCd_DetectIso9660Pvd(FirestaffFmtownsCd_Layout *layout,
                                        const char *bin_path) {
    if (!layout) return -1;
    if (!bin_path) {
        /* CUE-only path: do not fail, just leave PVD flag cleared. */
        return 0;
    }
    if (layout->data_track_count != 1) return 0; /* not a single-data-track disc */

    /* Find the data track's mode to know the PVD byte offset. */
    int data_mode = FIRESTAFF_FMTOWNS_CD_MODE_UNKNOWN;
    int i;
    for (i = 0; i < layout->track_count; i++) {
        if (layout->tracks[i].kind == FIRESTAFF_FMTOWNS_CD_KIND_DATA) {
            data_mode = layout->tracks[i].mode;
            break;
        }
    }
    if (data_mode != FIRESTAFF_FMTOWNS_CD_MODE_2352
        && data_mode != FIRESTAFF_FMTOWNS_CD_MODE_2048) {
        return 0;
    }

    unsigned long offset = (data_mode == FIRESTAFF_FMTOWNS_CD_MODE_2352)
                            ? ISO9660_PVD_BYTE_OFFSET_2352
                            : ISO9660_PVD_BYTE_OFFSET_2048;

    FILE *fp = fopen(bin_path, "rb");
    if (!fp) return -1;
    if (fseek(fp, (long)offset, SEEK_SET) != 0) { fclose(fp); return -1; }
    unsigned char sig[6] = {0};
    if (fread(sig, 1, 6, fp) != 6) { fclose(fp); return -1; }
    fclose(fp);
    if (sig[0] == 0x01 && memcmp(sig + 1, "CD001", 5) == 0) {
        layout->has_iso9660_pvd = 1;
    }
    return 0;
}

/* ── Game candidate scoring ─────────────────────────────────── */

static int score_dm1(const FirestaffFmtownsCd_Layout *l) {
    if (l->max_track_number < 18 || l->max_track_number > 21) return 0;
    if (l->audio_track_count < FIRESTAFF_FMTOWNS_DM1_AUDIO_TRACKS_MIN) return 0;
    if (l->audio_track_count > FIRESTAFF_FMTOWNS_DM1_AUDIO_TRACKS_MAX) return 0;
    int score = 50;
    /* Track 20 silence / unused tracks 04,07,20 shape: data track
     * is 01, audio is 02..20, and at least 17 audio tracks present. */
    if (l->max_track_number == 20 && l->audio_track_count == 19) score = 95;
    else if (l->max_track_number == 20 && l->audio_track_count == 18) score = 80;
    else if (l->max_track_number == 19 && l->audio_track_count == 18) score = 75;
    if (l->has_iso9660_pvd) score += 5;
    if (score > 100) score = 100;
    return score;
}

static int score_csb(const FirestaffFmtownsCd_Layout *l) {
    if (l->max_track_number < 28 || l->max_track_number > 32) return 0;
    if (l->audio_track_count < FIRESTAFF_FMTOWNS_CSB_AUDIO_TRACKS_MIN) return 0;
    if (l->audio_track_count > FIRESTAFF_FMTOWNS_CSB_AUDIO_TRACKS_MAX) return 0;
    int score = 50;
    if (l->max_track_number == 31 && l->audio_track_count == 30) score = 95;
    else if (l->max_track_number == 30 && l->audio_track_count == 29) score = 80;
    else if (l->max_track_number == 31 && l->audio_track_count == 29) score = 75;
    if (l->has_iso9660_pvd) score += 5;
    if (score > 100) score = 100;
    return score;
}

static int score_dm2(const FirestaffFmtownsCd_Layout *l) {
    if (l->max_track_number < 6 || l->max_track_number > 9) return 0;
    if (l->audio_track_count < FIRESTAFF_FMTOWNS_DM2_AUDIO_TRACKS_MIN) return 0;
    if (l->audio_track_count > FIRESTAFF_FMTOWNS_DM2_AUDIO_TRACKS_MAX) return 0;
    int score = 50;
    if (l->max_track_number == 8 && l->audio_track_count == 7) score = 90;
    else if (l->max_track_number == 8 && l->audio_track_count == 6) score = 80;
    else if (l->max_track_number == 6 && l->audio_track_count == 5) score = 85;
    if (l->has_iso9660_pvd) score += 5;
    if (score > 100) score = 100;
    return score;
}

FirestaffFmtownsCd_Classification FirestaffFmtownsCd_Classify(
    const FirestaffFmtownsCd_Layout *layout) {
    FirestaffFmtownsCd_Classification out;
    memset(&out, 0, sizeof(out));

    if (!layout || layout->track_count == 0) {
        out.game = FIRESTAFF_FMTOWNS_CD_GAME_NONE;
        return out;
    }

    int s_dm1 = score_dm1(layout);
    int s_csb = score_csb(layout);
    int s_dm2 = score_dm2(layout);

    out.audio_track_count = layout->audio_track_count;
    out.max_track_number = layout->max_track_number;

    if (s_dm1 == 0 && s_csb == 0 && s_dm2 == 0) {
        out.game = FIRESTAFF_FMTOWNS_CD_GAME_NONE;
        out.confidence = 0;
        return out;
    }

    if (s_dm1 >= s_csb && s_dm1 >= s_dm2) {
        out.game = FIRESTAFF_FMTOWNS_CD_GAME_DM1;
        out.confidence = s_dm1;
    } else if (s_csb >= s_dm2) {
        out.game = FIRESTAFF_FMTOWNS_CD_GAME_CSB;
        out.confidence = s_csb;
    } else {
        out.game = FIRESTAFF_FMTOWNS_CD_GAME_DM2;
        out.confidence = s_dm2;
    }

    /* Mark a positive match only when confidence clears a 60-point
     * bar so we never claim a game on a single data track + 1 audio. */
    if (out.confidence < 60) {
        out.game = FIRESTAFF_FMTOWNS_CD_GAME_NONE;
        out.confidence = 0;
    }

    /* Unused/silent-track receipt: cross-reference the documented
     * "unused" tracks per DMWeb. We only set this when the disc
     * actually scored positively. */
    if (out.game == FIRESTAFF_FMTOWNS_CD_GAME_DM1) {
        /* DM1: track 04, 07, 20 are unused; track 20 is silence. */
        out.unused_track_match = (layout->max_track_number == 20) ? 1 : 0;
    } else if (out.game == FIRESTAFF_FMTOWNS_CD_GAME_CSB) {
        /* CSB: three unused water-drop-like tracks somewhere in
         * 02..31; we cannot verify which without audio fingerprinting,
         * so we only mark "yes" when the track count matches exactly. */
        out.unused_track_match = (layout->audio_track_count == 30) ? 1 : 0;
    } else if (out.game == FIRESTAFF_FMTOWNS_CD_GAME_DM2) {
        /* DM2: silent track 8 + quieter tracks 2..6. */
        out.unused_track_match = (layout->max_track_number == 8) ? 1 : 0;
    }

    return out;
}

/* ── Self test ───────────────────────────────────────────────── */

#define ST_FAIL(msg) do {                                                  \
    fprintf(stderr, "firestaff_fmtowns_cd_classify FAIL: %s\n", msg);      \
    return 0;                                                              \
} while (0)

#define ST_ASSERT(cond, msg) do {                                          \
    if (!(cond)) { fprintf(stderr, "%s:%d: %s (%s)\n",                     \
                            __FILE__, __LINE__, msg, #cond);               \
                   return 0; }                                             \
} while (0)

/* ── Redump DM1 v2.0 fixture (synthetic CUE body).
 *
 * Tracks 02..20 = audio, with 04, 07, 20 unused per DMWeb. We model
 * the "full" redump layout, which carries all 19 audio tracks even
 * if some are silent. */
static const char *kDm1Cue =
    "FILE \"Track01.bin\" BINARY\r\n"
    "  TRACK 01 MODE1/2352\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track02.bin\" BINARY\r\n"
    "  TRACK 02 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track03.bin\" BINARY\r\n"
    "  TRACK 03 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track04.bin\" BINARY\r\n"
    "  TRACK 04 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track05.bin\" BINARY\r\n"
    "  TRACK 05 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track06.bin\" BINARY\r\n"
    "  TRACK 06 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track07.bin\" BINARY\r\n"
    "  TRACK 07 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track08.bin\" BINARY\r\n"
    "  TRACK 08 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track09.bin\" BINARY\r\n"
    "  TRACK 09 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track10.bin\" BINARY\r\n"
    "  TRACK 10 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track11.bin\" BINARY\r\n"
    "  TRACK 11 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track12.bin\" BINARY\r\n"
    "  TRACK 12 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track13.bin\" BINARY\r\n"
    "  TRACK 13 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track14.bin\" BINARY\r\n"
    "  TRACK 14 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track15.bin\" BINARY\r\n"
    "  TRACK 15 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track16.bin\" BINARY\r\n"
    "  TRACK 16 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track17.bin\" BINARY\r\n"
    "  TRACK 17 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track18.bin\" BINARY\r\n"
    "  TRACK 18 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track19.bin\" BINARY\r\n"
    "  TRACK 19 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track20.bin\" BINARY\r\n"
    "  TRACK 20 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n";

/* CSB v3.1 fixture: 30 audio tracks (02..31). */
static const char *kCsbCue =
    "FILE \"Track01.bin\" BINARY\r\n"
    "  TRACK 01 MODE1/2352\r\n"
    "    INDEX 01 00:00:00\r\n";
/* We'll append Track 02..31 below to keep the fixture readable. */
static const char *kCsbTail =
    "FILE \"Track31.bin\" BINARY\r\n"
    "  TRACK 31 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n";

/* DM2 v1.0 fixture: data + tracks 02..06 + silent track 8. */
static const char *kDm2Cue =
    "FILE \"Track01.bin\" BINARY\r\n"
    "  TRACK 01 MODE1/2352\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track02.bin\" BINARY\r\n"
    "  TRACK 02 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track03.bin\" BINARY\r\n"
    "  TRACK 03 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track04.bin\" BINARY\r\n"
    "  TRACK 04 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track05.bin\" BINARY\r\n"
    "  TRACK 05 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track06.bin\" BINARY\r\n"
    "  TRACK 06 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track07.bin\" BINARY\r\n"
    "  TRACK 07 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n"
    "FILE \"Track08.bin\" BINARY\r\n"
    "  TRACK 08 AUDIO\r\n"
    "    INDEX 01 00:00:00\r\n";

/* Build the CSB cue sheet in-memory (02..31). */
static int build_csb_cue(char *out, size_t cap) {
    size_t n = 0;
    int written = snprintf(out, cap - n, "%s", kCsbCue);
    if (written < 0 || (size_t)written >= cap - n) return -1;
    n += (size_t)written;
    int i;
    for (i = 2; i <= 30; i++) {
        written = snprintf(out + n, cap - n,
            "FILE \"Track%02d.bin\" BINARY\r\n"
            "  TRACK %02d AUDIO\r\n"
            "    INDEX 01 00:00:00\r\n",
            i, i);
        if (written < 0 || (size_t)written >= cap - n) return -1;
        n += (size_t)written;
    }
    written = snprintf(out + n, cap - n, "%s", kCsbTail);
    if (written < 0 || (size_t)written >= cap - n) return -1;
    n += (size_t)written;
    return (int)n;
}

static int test_parse_dm1_layout(void) {
    FirestaffFmtownsCd_Layout l;
    int rc = FirestaffFmtownsCd_ParseCue(kDm1Cue, strlen(kDm1Cue), &l);
    ST_ASSERT(rc == 0, "DM1 CUE should parse");
    ST_ASSERT(l.track_count == 20, "DM1 should have 20 tracks");
    ST_ASSERT(l.data_track_count == 1, "DM1 should have 1 data track");
    ST_ASSERT(l.audio_track_count == 19, "DM1 should have 19 audio tracks");
    ST_ASSERT(l.max_track_number == 20, "DM1 max track = 20");
    ST_ASSERT(l.tracks[0].kind == FIRESTAFF_FMTOWNS_CD_KIND_DATA, "track 1 data");
    ST_ASSERT(l.tracks[0].mode == FIRESTAFF_FMTOWNS_CD_MODE_2352, "track 1 mode 2352");
    ST_ASSERT(l.tracks[4].kind == FIRESTAFF_FMTOWNS_CD_KIND_AUDIO, "track 5 audio");
    ST_ASSERT(l.tracks[19].kind == FIRESTAFF_FMTOWNS_CD_KIND_AUDIO, "track 20 audio");
    ST_ASSERT(l.file_count == 20, "DM1 should have 20 FILE entries");
    ST_ASSERT(strcmp(l.files[0], "Track01.bin") == 0, "file 0 name");
    ST_ASSERT(strcmp(l.files[19], "Track20.bin") == 0, "file 19 name");
    return 1;
}

static int test_classify_dm1(void) {
    FirestaffFmtownsCd_Layout l;
    int rc = FirestaffFmtownsCd_ParseCue(kDm1Cue, strlen(kDm1Cue), &l);
    ST_ASSERT(rc == 0, "DM1 CUE should parse");
    FirestaffFmtownsCd_Classification c = FirestaffFmtownsCd_Classify(&l);
    ST_ASSERT(c.game == FIRESTAFF_FMTOWNS_CD_GAME_DM1, "DM1 candidate");
    ST_ASSERT(c.confidence >= 90, "DM1 high confidence");
    ST_ASSERT(c.audio_track_count == 19, "DM1 audio count 19");
    ST_ASSERT(c.max_track_number == 20, "DM1 max 20");
    ST_ASSERT(c.unused_track_match == 1, "DM1 unused-track match");
    return 1;
}

static int test_parse_csb_layout(void) {
    char buf[32 * 1024];
    int len = build_csb_cue(buf, sizeof(buf));
    ST_ASSERT(len > 0, "build CSB CUE");
    FirestaffFmtownsCd_Layout l;
    int rc = FirestaffFmtownsCd_ParseCue(buf, (size_t)len, &l);
    ST_ASSERT(rc == 0, "CSB CUE should parse");
    ST_ASSERT(l.track_count == 31, "CSB should have 31 tracks");
    ST_ASSERT(l.audio_track_count == 30, "CSB should have 30 audio tracks");
    ST_ASSERT(l.max_track_number == 31, "CSB max track = 31");
    ST_ASSERT(l.tracks[30].number == 31, "CSB track 31 indexed");
    return 1;
}

static int test_classify_csb(void) {
    char buf[32 * 1024];
    int len = build_csb_cue(buf, sizeof(buf));
    ST_ASSERT(len > 0, "build CSB CUE");
    FirestaffFmtownsCd_Layout l;
    int rc = FirestaffFmtownsCd_ParseCue(buf, (size_t)len, &l);
    ST_ASSERT(rc == 0, "CSB CUE should parse");
    FirestaffFmtownsCd_Classification c = FirestaffFmtownsCd_Classify(&l);
    ST_ASSERT(c.game == FIRESTAFF_FMTOWNS_CD_GAME_CSB, "CSB candidate");
    ST_ASSERT(c.confidence >= 90, "CSB high confidence");
    return 1;
}

static int test_parse_dm2_layout(void) {
    FirestaffFmtownsCd_Layout l;
    int rc = FirestaffFmtownsCd_ParseCue(kDm2Cue, strlen(kDm2Cue), &l);
    ST_ASSERT(rc == 0, "DM2 CUE should parse");
    ST_ASSERT(l.track_count == 8, "DM2 should have 8 tracks");
    ST_ASSERT(l.audio_track_count == 7, "DM2 should have 7 audio tracks");
    ST_ASSERT(l.max_track_number == 8, "DM2 max track = 8");
    return 1;
}

static int test_classify_dm2(void) {
    FirestaffFmtownsCd_Layout l;
    int rc = FirestaffFmtownsCd_ParseCue(kDm2Cue, strlen(kDm2Cue), &l);
    ST_ASSERT(rc == 0, "DM2 CUE should parse");
    FirestaffFmtownsCd_Classification c = FirestaffFmtownsCd_Classify(&l);
    ST_ASSERT(c.game == FIRESTAFF_FMTOWNS_CD_GAME_DM2, "DM2 candidate");
    ST_ASSERT(c.confidence >= 80, "DM2 high confidence");
    ST_ASSERT(c.unused_track_match == 1, "DM2 silent track 8 match");
    return 1;
}

static int test_iso_iso9660_only_layout(void) {
    /* ISO/CUE: single .iso file with multiple TRACK entries (cooked
     * MODE1/2048). Redump convention. */
    const char *cue =
        "FILE \"game.iso\" BINARY\r\n"
        "  TRACK 01 MODE1/2048\r\n"
        "    INDEX 01 00:00:00\r\n"
        "  TRACK 02 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "  TRACK 03 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "  TRACK 04 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "  TRACK 05 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "  TRACK 06 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n";
    FirestaffFmtownsCd_Layout l;
    int rc = FirestaffFmtownsCd_ParseCue(cue, strlen(cue), &l);
    ST_ASSERT(rc == 0, "ISO/CUE should parse");
    ST_ASSERT(l.tracks[0].mode == FIRESTAFF_FMTOWNS_CD_MODE_2048, "ISO mode 2048");
    ST_ASSERT(l.file_count == 1, "single ISO file");
    return 1;
}

static int test_reject_no_data_track(void) {
    /* All audio, no data track: should fail. */
    const char *cue =
        "FILE \"a.bin\" BINARY\r\n"
        "  TRACK 01 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"b.bin\" BINARY\r\n"
        "  TRACK 02 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n";
    FirestaffFmtownsCd_Layout l;
    int rc = FirestaffFmtownsCd_ParseCue(cue, strlen(cue), &l);
    ST_ASSERT(rc != 0, "all-audio CUE must be rejected");
    return 1;
}

static int test_reject_track_before_file(void) {
    const char *cue =
        "  TRACK 01 MODE1/2352\r\n"
        "    INDEX 01 00:00:00\r\n";
    FirestaffFmtownsCd_Layout l;
    int rc = FirestaffFmtownsCd_ParseCue(cue, strlen(cue), &l);
    ST_ASSERT(rc != 0, "TRACK before FILE must be rejected");
    return 1;
}

static int test_reject_truncated_file_quote(void) {
    const char *cue = "FILE \"unterminated.bin\r\n";
    FirestaffFmtownsCd_Layout l;
    int rc = FirestaffFmtownsCd_ParseCue(cue, strlen(cue), &l);
    ST_ASSERT(rc != 0, "unterminated quote must be rejected");
    return 1;
}

static int test_no_match_single_audio_track(void) {
    /* 1 data + 1 audio should be too small to claim any game. */
    const char *cue =
        "FILE \"data.bin\" BINARY\r\n"
        "  TRACK 01 MODE1/2352\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"audio.bin\" BINARY\r\n"
        "  TRACK 02 AUDIO\r\n"
        "    INDEX 01 00:00:00\r\n";
    FirestaffFmtownsCd_Layout l;
    int rc = FirestaffFmtownsCd_ParseCue(cue, strlen(cue), &l);
    ST_ASSERT(rc == 0, "small disc should parse");
    FirestaffFmtownsCd_Classification c = FirestaffFmtownsCd_Classify(&l);
    ST_ASSERT(c.game == FIRESTAFF_FMTOWNS_CD_GAME_NONE,
              "tiny disc should not match");
    ST_ASSERT(c.confidence == 0, "confidence zero");
    return 1;
}

static int test_pvd_detection_synthetic(void) {
    /* Build a minimal "BIN file" in a temp path with a PVD signature
     * at the right byte offset for the data-track mode we are testing
     * (MODE1/2352 for the DM1 fixture), then verify detection. */
    char tmpl[] = "/tmp/firestaff_fmtowns_pvd_XXXXXX";
    int fd = mkstemp(tmpl);
    ST_ASSERT(fd >= 0, "mkstemp");
    /* Pad up to ISO9660_PVD_BYTE_OFFSET_2352, then write the 6-byte
     * signature (`\x01CD001`) at that exact offset, then pad a little
     * more so the corruption step can also write 6 bytes safely. */
    unsigned long pvd_offset = ISO9660_PVD_BYTE_OFFSET_2352;
    unsigned long file_size  = pvd_offset + 32u;
    FILE *fp = fdopen(fd, "r+b");
    ST_ASSERT(fp != NULL, "fdopen");
    static const unsigned char zero[256] = {0};
    unsigned long written = 0;
    while (written < file_size) {
        unsigned long chunk = file_size - written;
        if (chunk > sizeof(zero)) chunk = sizeof(zero);
        size_t n = fwrite(zero, 1, chunk, fp);
        ST_ASSERT(n == chunk, "zero pad");
        written += chunk;
    }
    if (fseek(fp, (long)pvd_offset, SEEK_SET) != 0) {
        ST_FAIL("seek to PVD offset");
    }
    unsigned char sig[6] = {0x01, 'C', 'D', '0', '0', '1'};
    size_t n = fwrite(sig, 1, sizeof(sig), fp);
    ST_ASSERT(n == sizeof(sig), "write signature");
    fclose(fp);

    FirestaffFmtownsCd_Layout l;
    int rc = FirestaffFmtownsCd_ParseCue(kDm1Cue, strlen(kDm1Cue), &l);
    ST_ASSERT(rc == 0, "DM1 CUE should parse for PVD test");
    rc = FirestaffFmtownsCd_DetectIso9660Pvd(&l, tmpl);
    ST_ASSERT(rc == 0, "PVD detect should not fail");
    ST_ASSERT(l.has_iso9660_pvd == 1, "PVD should be flagged");

    /* Now corrupt the signature and re-check. */
    fp = fopen(tmpl, "r+b");
    ST_ASSERT(fp != NULL, "reopen temp");
    fseek(fp, (long)pvd_offset, SEEK_SET);
    unsigned char junk[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    n = fwrite(junk, 1, sizeof(junk), fp);
    ST_ASSERT(n == sizeof(junk), "write junk");
    fclose(fp);
    firestaff_fmtowns_cd_classify_zero_layout(&l);
    rc = FirestaffFmtownsCd_ParseCue(kDm1Cue, strlen(kDm1Cue), &l);
    ST_ASSERT(rc == 0, "DM1 CUE reparse");
    rc = FirestaffFmtownsCd_DetectIso9660Pvd(&l, tmpl);
    ST_ASSERT(rc == 0, "PVD detect no-fail");
    ST_ASSERT(l.has_iso9660_pvd == 0, "PVD should NOT be flagged on junk");

    remove(tmpl);
    return 1;
}

static int test_pvd_no_bin_path(void) {
    FirestaffFmtownsCd_Layout l;
    int rc = FirestaffFmtownsCd_ParseCue(kDm1Cue, strlen(kDm1Cue), &l);
    ST_ASSERT(rc == 0, "DM1 CUE should parse");
    rc = FirestaffFmtownsCd_DetectIso9660Pvd(&l, NULL);
    ST_ASSERT(rc == 0, "PVD detect NULL bin should not fail");
    ST_ASSERT(l.has_iso9660_pvd == 0, "PVD stays 0 with NULL bin");
    return 1;
}

static int test_lf_only_line_endings(void) {
    /* Build a DM1 cue with only LF endings, no CR. */
    const char *src = kDm1Cue;
    char buf[8192];
    size_t n = 0;
    while (*src && n + 1 < sizeof(buf)) {
        if (*src != '\r') buf[n++] = *src;
        src++;
    }
    buf[n] = 0;
    FirestaffFmtownsCd_Layout l;
    int rc = FirestaffFmtownsCd_ParseCue(buf, n, &l);
    ST_ASSERT(rc == 0, "LF-only CUE should parse");
    ST_ASSERT(l.track_count == 20, "LF DM1 should still have 20 tracks");
    return 1;
}

static int test_rem_and_comments(void) {
    /* redump CUE sheets often include REM GENRE "..." and similar
     * comments. The parser should tolerate them. */
    const char *cue =
        "REM GENRE \"Game\"\r\n"
        "REM DATE 1989\r\n"
        "; trailing comment\r\n"
        "FILE \"Track01.bin\" BINARY\r\n"
        "  TRACK 01 MODE1/2352\r\n"
    "    INDEX 01 00:00:00\r\n";
    FirestaffFmtownsCd_Layout l;
    int rc = FirestaffFmtownsCd_ParseCue(cue, strlen(cue), &l);
    ST_ASSERT(rc == 0, "REM/; comments should be tolerated");
    ST_ASSERT(l.track_count == 1, "1 track after REMs");
    return 1;
}

static int test_pregap_marker(void) {
    /* Track 02 with a PREGAP + INDEX 00 + INDEX 01. */
    const char *cue =
        "FILE \"Track01.bin\" BINARY\r\n"
        "  TRACK 01 MODE1/2352\r\n"
        "    INDEX 01 00:00:00\r\n"
        "FILE \"Track02.bin\" BINARY\r\n"
        "  TRACK 02 AUDIO\r\n"
        "    PREGAP 00:02:00\r\n"
        "    INDEX 01 00:00:00\r\n";
    FirestaffFmtownsCd_Layout l;
    int rc = FirestaffFmtownsCd_ParseCue(cue, strlen(cue), &l);
    ST_ASSERT(rc == 0, "PREGAP should parse");
    ST_ASSERT(l.tracks[1].has_pregap == 1, "PREGAP should set marker");
    ST_ASSERT(l.tracks[1].has_index01 == 1, "INDEX 01 set on track 02");
    return 1;
}

int FirestaffFmtownsCd_SelfTest(void) {
    int total = 0, passed = 0;
    #define RUN(name) do { total++; if (name()) passed++; } while (0)
    RUN(test_parse_dm1_layout);
    RUN(test_classify_dm1);
    RUN(test_parse_csb_layout);
    RUN(test_classify_csb);
    RUN(test_parse_dm2_layout);
    RUN(test_classify_dm2);
    RUN(test_iso_iso9660_only_layout);
    RUN(test_reject_no_data_track);
    RUN(test_reject_track_before_file);
    RUN(test_reject_truncated_file_quote);
    RUN(test_no_match_single_audio_track);
    RUN(test_pvd_detection_synthetic);
    RUN(test_pvd_no_bin_path);
    RUN(test_lf_only_line_endings);
    RUN(test_rem_and_comments);
    RUN(test_pregap_marker);
    #undef RUN
    return (passed == total) ? 0 : -1;
}
