#include "theron_v1_cd_audio_availability.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

/* Source-locked CD-DA track routing receipt for Theron's Quest V1.
 *
 * Evidence:
 *   - Local original CUE sheets: $HOME/.firestaff/data/theron/TQUS.cue and
 *     TQJP.cue declare the 19-track CD layout (Track 01 AUDIO, Track 02
 *     MODE1/2048, Tracks 03-18 AUDIO, Track 19 MODE1/2048).
 *   - Locally staged original CD audio: TQUS01.ogg, TQUS03.ogg, TQ04.ogg
 *     through TQ18.ogg and JP equivalents.  These are the original CD-DA
 *     tracks transcoded to OGG; the CUEs still name the original .wav files.
 *   - No synthetic audio playback fallback existed in src/theron before this
 *     receipt; the receipt is the required gate before any Theron audio
 *     output is authorized.
 */

static const char *tqr_skip_space(const char *text) {
    if (!text) return NULL;
    while (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n') {
        ++text;
    }
    return text;
}

static int tqr_ascii_starts_ci(const char *text, const char *prefix) {
    size_t i;
    if (!text || !prefix) return 0;
    for (i = 0u; prefix[i] != '\0'; ++i) {
        if (tolower((unsigned char)text[i]) != tolower((unsigned char)prefix[i])) {
            return 0;
        }
    }
    return 1;
}

static int tqr_ascii_equal_ci(const char *a, const char *b) {
    size_t i;
    if (!a || !b) return 0;
    for (i = 0u; ; ++i) {
        unsigned char ca = (unsigned char)tolower((unsigned char)a[i]);
        unsigned char cb = (unsigned char)tolower((unsigned char)b[i]);
        if (ca != cb) return 0;
        if (ca == '\0') return 1;
    }
}

static int tqr_path_is_cue(const char *path) {
    const char *dot = path ? strrchr(path, '.') : NULL;
    return dot && tqr_ascii_equal_ci(dot, ".cue");
}

static size_t tqr_copy_string(char *out, size_t out_cap, const char *in) {
    size_t len = in ? strlen(in) : 0u;
    if (len >= out_cap) {
        if (out_cap > 0u) out[0] = '\0';
        return 0u;
    }
    memcpy(out, in, len + 1u);
    return len;
}

/* root is a directory.  A trailing path separator is optional. */
static int tqr_path_for_file(const char *root, const char *file_name,
                             char out_path[THERON_TRACK02_MOUNT_PATH_CAPACITY]) {
    size_t root_len;
    size_t file_len;
    int needs_sep;
    if (!root || !file_name || !out_path) return 0;
    root_len = strlen(root);
    file_len = strlen(file_name);
    needs_sep = root_len > 0u &&
                root[root_len - 1u] != '/' &&
                root[root_len - 1u] != '\\';
    if (root_len + (needs_sep ? 1u : 0u) + file_len >=
        THERON_TRACK02_MOUNT_PATH_CAPACITY) {
        return 0;
    }
    memcpy(out_path, root, root_len);
    if (needs_sep) out_path[root_len] = '/';
    memcpy(out_path + root_len + (needs_sep ? 1u : 0u), file_name, file_len + 1u);
    return 1;
}

static int tqr_parent_directory(
    const char *path,
    char out_dir[THERON_TRACK02_MOUNT_PATH_CAPACITY]) {
    const char *slash;
    size_t len;
    if (!path || !out_dir) return 0;
    slash = strrchr(path, '/');
    if (!slash) slash = strrchr(path, '\\');
    if (!slash) {
        out_dir[0] = '\0';
        return 1;
    }
    len = (size_t)(slash - path);
    if (len >= THERON_TRACK02_MOUNT_PATH_CAPACITY) return 0;
    memcpy(out_dir, path, len);
    out_dir[len] = '\0';
    return 1;
}

static int tqr_path_is_readable(const char *path) {
    FILE *file = path ? fopen(path, "rb") : NULL;
    if (!file) return 0;
    fclose(file);
    return 1;
}

/* Replace the extension of basename with .ogg.  out_ogg must have capacity
 * THERON_TRACK02_MOUNT_PATH_CAPACITY.  Returns 0 if the source has no
 * extension or the result would not fit. */
static int tqr_ogg_fallback_path(const char *basename,
                                 char out_ogg[THERON_TRACK02_MOUNT_PATH_CAPACITY]) {
    const char *dot;
    size_t stem_len;
    size_t suffix_len = strlen(".ogg");
    if (!basename || !out_ogg) return 0;
    dot = strrchr(basename, '.');
    if (!dot || dot == basename) return 0;
    stem_len = (size_t)(dot - basename);
    if (stem_len + suffix_len >= THERON_TRACK02_MOUNT_PATH_CAPACITY) return 0;
    memcpy(out_ogg, basename, stem_len);
    memcpy(out_ogg + stem_len, ".ogg", suffix_len + 1u);
    return 1;
}

/* Some documented MyAbandonware dumps split the final data extent into
 * TQJP02End.iso/TQUS02End.iso, while their supplied CUE still names the
 * pre-split TQJP02.iso/TQUS02.iso member.  This is an explicit media-layout
 * alias, not a fallback search: only these two exact CUE member names may
 * resolve to their matching sibling.  See theron_v1_track02.c
 * tqr_cue_known_split_track02_path. */
static int tqr_cue_known_split_track02_path(
    const char *root,
    const char *selected_file,
    char out_path[THERON_TRACK02_MOUNT_PATH_CAPACITY]) {
    static const struct {
        const char *declared_name;
        const char *materialized_name;
    } aliases[] = {
        { "TQJP02.iso", "TQJP02End.iso" },
        { "TQUS02.iso", "TQUS02End.iso" }
    };
    size_t i;
    if (!root || !selected_file || !out_path ||
        strchr(selected_file, '/') || strchr(selected_file, '\\')) return 0;
    for (i = 0u; i < sizeof(aliases) / sizeof(aliases[0]); ++i) {
        if (tqr_ascii_equal_ci(selected_file, aliases[i].declared_name)) {
            return tqr_path_for_file(root, aliases[i].materialized_name, out_path);
        }
    }
    return 0;
}

static int tqr_resolve_audio_file(const char *cue_path, const char *data_root,
                                  const char *declared_name,
                                  char out_path[THERON_TRACK02_MOUNT_PATH_CAPACITY]) {
    char cue_parent[THERON_TRACK02_MOUNT_PATH_CAPACITY];
    const char *root;
    char ogg_name[THERON_TRACK02_MOUNT_PATH_CAPACITY];
    if (data_root && data_root[0]) {
        root = data_root;
    } else {
        if (!tqr_parent_directory(cue_path, cue_parent)) return 0;
        root = cue_parent;
    }
    if (!tqr_path_for_file(root, declared_name, out_path)) return 0;
    if (tqr_path_is_readable(out_path)) return 1;
    /* CUEs name the original .wav CD-DA tracks; local corpus supplies .ogg. */
    if (tqr_ogg_fallback_path(declared_name, ogg_name) &&
        tqr_path_for_file(root, ogg_name, out_path) &&
        tqr_path_is_readable(out_path)) {
        return 1;
    }
    /* Known split Track 02 data alias for original MyAbandonware-style dumps. */
    if (tqr_cue_known_split_track02_path(root, declared_name, out_path) &&
        tqr_path_is_readable(out_path)) {
        return 1;
    }
    return 0;
}

/* Parse a CUE FILE line and return the filename.  Accepts both quoted
 * (`FILE "name.wav" WAVE`) and unquoted (`FILE name.wav WAVE`) forms, and
 * any declared file type (WAVE, BINARY, AIFF, etc.) because audio tracks are
 * commonly WAVE while data tracks are BINARY. */
static int tqr_cue_file_line(const char *line, char out_name[THERON_TRACK02_MOUNT_PATH_CAPACITY]) {
    const char *p = tqr_skip_space(line);
    const char *end;
    size_t len;
    if (!p || !tqr_ascii_starts_ci(p, "FILE") ||
        (p[4] != ' ' && p[4] != '\t')) {
        return 0;
    }
    p = tqr_skip_space(p + 4u);
    if (*p == '"') {
        ++p;
        end = strchr(p, '"');
        if (!end || end == p) return 0;
        len = (size_t)(end - p);
    } else {
        end = p;
        while (*end != '\0' && *end != ' ' && *end != '\t' &&
               *end != '\r' && *end != '\n') {
            ++end;
        }
        if (end == p) return 0;
        len = (size_t)(end - p);
    }
    if (len >= THERON_TRACK02_MOUNT_PATH_CAPACITY) return 0;
    memcpy(out_name, p, len);
    out_name[len] = '\0';
    return 1;
}

static int tqr_cue_track_mode(const char *line, unsigned int *out_track,
                              int *out_audio, int *out_data) {
    const char *p = tqr_skip_space(line);
    unsigned int track = 0u;
    if (!p || !tqr_ascii_starts_ci(p, "TRACK") ||
        (p[5] != ' ' && p[5] != '\t')) {
        return 0;
    }
    p = tqr_skip_space(p + 6u);
    if (p[0] < '0' || p[0] > '9' || p[1] < '0' || p[1] > '9') return 0;
    track = (unsigned int)((p[0] - '0') * 10 + (p[1] - '0'));
    if (p[2] != ' ' && p[2] != '\t') return 0;
    p = tqr_skip_space(p + 3u);
    if (out_track) *out_track = track;
    if (out_audio) {
        *out_audio = tqr_ascii_starts_ci(p, "AUDIO") &&
            (p[5] == '\0' || p[5] == ' ' || p[5] == '\t' ||
             p[5] == '\r' || p[5] == '\n');
    }
    if (out_data) {
        *out_data = tqr_ascii_equal_ci(p, "MODE1/2352") ||
                    tqr_ascii_equal_ci(p, "MODE1/2048") ||
                    (tqr_ascii_starts_ci(p, "MODE1/2352") &&
                     (p[10] == ' ' || p[10] == '\t' || p[10] == '\r' || p[10] == '\n')) ||
                    (tqr_ascii_starts_ci(p, "MODE1/2048") &&
                     (p[10] == ' ' || p[10] == '\t' || p[10] == '\r' || p[10] == '\n'));
    }
    return 1;
}

static void tqr_init_receipt(Theron_V1CdAudioReceipt *receipt) {
    if (!receipt) return;
    memset(receipt, 0, sizeof(*receipt));
    receipt->availability = THERON_V1_CD_AUDIO_MISSING;
}

static int tqr_is_canonical_layout(const Theron_V1CdAudioReceipt *receipt) {
    size_t i;
    if (!receipt || receipt->track_count != THERON_V1_CD_AUDIO_TRACK_COUNT) {
        return 0;
    }
    for (i = 1u; i <= THERON_V1_CD_AUDIO_TRACK_COUNT; ++i) {
        int expected_audio = (i == 1u) || (i >= 3u && i <= 18u);
        int expected_data = (i == 2u) || (i == 19u);
        if (expected_audio && !receipt->track_is_audio[i]) return 0;
        if (expected_data && receipt->track_is_audio[i]) return 0;
    }
    return 1;
}

Theron_V1CdAudioReceipt theron_v1_cd_audio_availability(
    const char *cue_path,
    const char *data_root) {
    Theron_V1CdAudioReceipt receipt;
    FILE *cue = NULL;
    char line[2048];
    char current_file[THERON_TRACK02_MOUNT_PATH_CAPACITY] = {0};
    unsigned int max_track = 0u;
    int parse_error = 0;
    size_t i;

    tqr_init_receipt(&receipt);
    if (!cue_path || !cue_path[0]) {
        tqr_copy_string(receipt.unavailable_reason,
                        sizeof(receipt.unavailable_reason),
                        "No CUE path supplied");
        receipt.availability = THERON_V1_CD_AUDIO_CUE_NOT_FOUND;
        return receipt;
    }
    if (!tqr_path_is_cue(cue_path)) {
        tqr_copy_string(receipt.unavailable_reason,
                        sizeof(receipt.unavailable_reason),
                        "Supplied path is not a CUE sheet");
        receipt.availability = THERON_V1_CD_AUDIO_CUE_NOT_FOUND;
        return receipt;
    }

    cue = fopen(cue_path, "rb");
    if (!cue) {
        tqr_copy_string(receipt.unavailable_reason,
                        sizeof(receipt.unavailable_reason),
                        "CUE sheet is unreadable");
        receipt.availability = THERON_V1_CD_AUDIO_CUE_NOT_FOUND;
        return receipt;
    }

    while (fgets(line, sizeof(line), cue) != NULL) {
        char parsed[THERON_TRACK02_MOUNT_PATH_CAPACITY];
        unsigned int track = 0u;
        int audio = 0, data = 0;
        if (tqr_cue_file_line(line, parsed)) {
            tqr_copy_string(current_file, sizeof(current_file), parsed);
            continue;
        }
        if (tqr_cue_track_mode(line, &track, &audio, &data)) {
            if (track < 1u || track > THERON_V1_CD_AUDIO_TRACK_COUNT) {
                parse_error = 1;
                break;
            }
            if (track > max_track) max_track = track;
            receipt.track_is_audio[track] = audio;
            if (audio) ++receipt.audio_track_count;
            if (data) ++receipt.data_track_count;
            if (current_file[0]) {
                tqr_copy_string(receipt.track_paths[track],
                                sizeof(receipt.track_paths[track]),
                                current_file);
                if (audio || data) {
                    receipt.track_present[track] =
                        tqr_resolve_audio_file(cue_path, data_root,
                                               current_file,
                                               receipt.track_paths[track]);
                }
            }
            continue;
        }
    }
    fclose(cue);

    receipt.track_count = max_track;
    if (data_root && data_root[0]) {
        tqr_copy_string(receipt.audio_directory,
                        sizeof(receipt.audio_directory), data_root);
    } else {
        char cue_parent[THERON_TRACK02_MOUNT_PATH_CAPACITY];
        if (tqr_parent_directory(cue_path, cue_parent)) {
            tqr_copy_string(receipt.audio_directory,
                            sizeof(receipt.audio_directory), cue_parent);
        }
    }

    if (parse_error) {
        tqr_copy_string(receipt.unavailable_reason,
                        sizeof(receipt.unavailable_reason),
                        "CUE track number out of canonical 1..19 range");
        receipt.availability = THERON_V1_CD_AUDIO_CUE_PARSE_ERROR;
        return receipt;
    }

    if (!tqr_is_canonical_layout(&receipt)) {
        tqr_copy_string(receipt.unavailable_reason,
                        sizeof(receipt.unavailable_reason),
                        "CUE track layout does not match original Theron CD");
        receipt.availability = THERON_V1_CD_AUDIO_LAYOUT_MISMATCH;
        return receipt;
    }

    for (i = 1u; i <= THERON_V1_CD_AUDIO_TRACK_COUNT; ++i) {
        if (receipt.track_is_audio[i] && !receipt.track_present[i]) {
            tqr_copy_string(receipt.unavailable_reason,
                            sizeof(receipt.unavailable_reason),
                            "Original CD-DA track file missing on disk");
            receipt.availability = THERON_V1_CD_AUDIO_TRACK_FILE_MISSING;
            return receipt;
        }
        /* Data tracks must also be present for a source-locked receipt. */
        if (!receipt.track_is_audio[i] && !receipt.track_present[i]) {
            tqr_copy_string(receipt.unavailable_reason,
                            sizeof(receipt.unavailable_reason),
                            "Original data track file missing on disk");
            receipt.availability = THERON_V1_CD_AUDIO_TRACK_FILE_MISSING;
            return receipt;
        }
    }

    receipt.availability = THERON_V1_CD_AUDIO_READY;
    receipt.playback_allowed = 1;
    return receipt;
}
