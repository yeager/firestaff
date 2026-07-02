#include "firestaff_theron_media_classify.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <io.h>
#else
#include <dirent.h>
#endif

#define THERON_PVD_2048_OFFSET (16UL * 2048UL)
#define THERON_PVD_2352_OFFSET (16UL * 2352UL + 16UL)
#define THERON_CUE_MAX_BYTES (1024UL * 1024UL)
#define THERON_SCAN_MAX_DEPTH 3
#define THERON_SCAN_MAX_FILES 160

static int th_char_lower(int c) {
    return (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
}

static int th_ieq(const char* a, const char* b) {
    if (!a || !b) {
        return 0;
    }
    while (*a && *b) {
        if (th_char_lower((unsigned char)*a) !=
            th_char_lower((unsigned char)*b)) {
            return 0;
        }
        ++a;
        ++b;
    }
    return *a == '\0' && *b == '\0';
}

static int th_starts_with_i(const char* s, const char* prefix) {
    if (!s || !prefix) {
        return 0;
    }
    while (*prefix) {
        if (th_char_lower((unsigned char)*s) !=
            th_char_lower((unsigned char)*prefix)) {
            return 0;
        }
        ++s;
        ++prefix;
    }
    return 1;
}

static const char* th_ltrim(const char* s) {
    while (s && (*s == ' ' || *s == '\t')) {
        ++s;
    }
    return s ? s : "";
}

static void th_rtrim(char* line, size_t* line_len) {
    size_t n;
    if (!line || !line_len) {
        return;
    }
    n = *line_len;
    while (n > 0U &&
           (line[n - 1U] == '\n' ||
            line[n - 1U] == '\r' ||
            line[n - 1U] == ' ' ||
            line[n - 1U] == '\t')) {
        line[--n] = '\0';
    }
    *line_len = n;
}

static const char* th_basename(const char* path) {
    const char* base = path ? path : "";
    const char* p;
    for (p = base; *p; ++p) {
        if (*p == '/' || *p == '\\') {
            base = p + 1;
        }
    }
    return base;
}

static const char* th_extension(const char* path) {
    const char* base = th_basename(path);
    const char* dot = strrchr(base, '.');
    return dot ? dot : "";
}

static int th_has_ext(const char* path, const char* ext) {
    return th_ieq(th_extension(path), ext);
}

static void th_copy(char* out, size_t out_size, const char* text) {
    if (!out || out_size == 0U) {
        return;
    }
    snprintf(out, out_size, "%s", text ? text : "");
}

static int th_copy_quoted(const char* src, char* out, size_t out_size) {
    const char* q1;
    const char* q2;
    size_t n;
    if (!src || !out || out_size == 0U) {
        return 0;
    }
    q1 = strchr(src, '"');
    if (!q1) {
        return 0;
    }
    q2 = strchr(q1 + 1, '"');
    if (!q2) {
        return 0;
    }
    n = (size_t)(q2 - q1 - 1);
    if (n >= out_size) {
        n = out_size - 1U;
    }
    memcpy(out, q1 + 1, n);
    out[n] = '\0';
    return 1;
}

static int th_parse_uint(const char** p) {
    int value = 0;
    const char* s;
    if (!p || !*p || **p < '0' || **p > '9') {
        return -1;
    }
    s = *p;
    while (*s >= '0' && *s <= '9') {
        value = value * 10 + (*s - '0');
        ++s;
    }
    *p = s;
    return value;
}

static int th_file_has_iso9660_pvd_at(const char* path, unsigned long offset) {
    unsigned char sig[6];
    FILE* fp;
    if (!path || path[0] == '\0') {
        return 0;
    }
    fp = fopen(path, "rb");
    if (!fp) {
        return 0;
    }
    if (fseek(fp, (long)offset, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }
    if (fread(sig, 1U, sizeof(sig), fp) != sizeof(sig)) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return sig[0] == 0x01 && memcmp(sig + 1U, "CD001", 5U) == 0;
}

static int th_file_has_iso9660_pvd(const char* path) {
    return th_file_has_iso9660_pvd_at(path, THERON_PVD_2048_OFFSET) ||
           th_file_has_iso9660_pvd_at(path, THERON_PVD_2352_OFFSET);
}

void FirestaffTheronMedia_Init(FirestaffTheronMediaStatus* status) {
    if (!status) {
        return;
    }
    memset(status, 0, sizeof(*status));
    status->layout = FIRESTAFF_THERON_MEDIA_LAYOUT_UNKNOWN;
    status->data_track_number = 0;
}

static void th_finalize(FirestaffTheronMediaStatus* status) {
    if (!status) {
        return;
    }
    status->launch_candidate = status->has_track02_data ? 1 : 0;
    if (status->has_cue) {
        if (status->iso_file_count > 0 && status->ogg_file_count > 0) {
            status->layout = FIRESTAFF_THERON_MEDIA_LAYOUT_ISO_OGG_CUE;
        } else if (status->bin_file_count > 0) {
            status->layout = FIRESTAFF_THERON_MEDIA_LAYOUT_BIN_CUE;
        } else if (status->iso_file_count > 0) {
            status->layout = FIRESTAFF_THERON_MEDIA_LAYOUT_ISO;
        } else if (status->ogg_file_count > 0) {
            status->layout = FIRESTAFF_THERON_MEDIA_LAYOUT_OGG_ONLY;
            status->launch_candidate = 0;
        }
    } else if (status->layout == FIRESTAFF_THERON_MEDIA_LAYOUT_UNKNOWN) {
        if (status->iso_file_count > 0) {
            status->layout = FIRESTAFF_THERON_MEDIA_LAYOUT_ISO;
        } else if (status->bin_file_count > 0) {
            status->layout = FIRESTAFF_THERON_MEDIA_LAYOUT_RAW_BIN;
        } else if (status->ogg_file_count > 0) {
            status->layout = FIRESTAFF_THERON_MEDIA_LAYOUT_OGG_ONLY;
        }
    }
}

int FirestaffTheronMedia_ParseCue(const char* cue_text,
                                  size_t cue_len,
                                  FirestaffTheronMediaStatus* status) {
    char line[1024];
    char current_file[FIRESTAFF_THERON_MEDIA_PATH_CAPACITY];
    int current_track = 0;
    int saw_track = 0;
    size_t pos = 0U;

    if (!cue_text || !status) {
        return -1;
    }
    FirestaffTheronMedia_Init(status);
    status->has_cue = 1;
    current_file[0] = '\0';

    while (pos < cue_len) {
        size_t i = 0U;
        size_t line_len;
        const char* p;
        while (pos < cue_len && cue_text[pos] != '\n' && i + 1U < sizeof(line)) {
            line[i++] = cue_text[pos++];
        }
        if (pos < cue_len && cue_text[pos] == '\n') {
            ++pos;
        }
        line[i] = '\0';
        line_len = i;
        th_rtrim(line, &line_len);
        p = th_ltrim(line);
        if (*p == '\0' || *p == ';' || *p == '#') {
            continue;
        }
        if (th_starts_with_i(p, "REM ") ||
            th_starts_with_i(p, "TITLE ") ||
            th_starts_with_i(p, "PERFORMER ") ||
            th_starts_with_i(p, "CATALOG ") ||
            th_starts_with_i(p, "FLAGS ") ||
            th_starts_with_i(p, "ISRC ")) {
            continue;
        }
        if (th_starts_with_i(p, "FILE ")) {
            if (!th_copy_quoted(p, current_file, sizeof(current_file))) {
                return -1;
            }
            if (status->candidate_path[0] == '\0') {
                th_copy(status->candidate_path,
                        sizeof(status->candidate_path),
                        current_file);
            }
            if (th_has_ext(current_file, ".bin")) {
                ++status->bin_file_count;
            } else if (th_has_ext(current_file, ".iso")) {
                ++status->iso_file_count;
            } else if (th_has_ext(current_file, ".ogg")) {
                ++status->ogg_file_count;
            }
            continue;
        }
        if (th_starts_with_i(p, "TRACK ")) {
            const char* q = th_ltrim(p + 6);
            int track = th_parse_uint(&q);
            if (track < 1 || track > 99 || current_file[0] == '\0') {
                return -1;
            }
            current_track = track;
            saw_track = 1;
            q = th_ltrim(q);
            if (th_starts_with_i(q, "AUDIO")) {
                ++status->audio_track_count;
            } else if (th_starts_with_i(q, "MODE1/") ||
                       th_starts_with_i(q, "MODE2/")) {
                if (track == 2) {
                    status->has_track02_data = 1;
                    status->data_track_number = track;
                    th_copy(status->candidate_path,
                            sizeof(status->candidate_path),
                            current_file);
                }
            }
            continue;
        }
        if (th_starts_with_i(p, "INDEX ") ||
            th_starts_with_i(p, "PREGAP ") ||
            th_starts_with_i(p, "POSTGAP ")) {
            if (!saw_track || current_track <= 0) {
                return -1;
            }
            continue;
        }
    }
    if (!saw_track) {
        FirestaffTheronMedia_Init(status);
        return -1;
    }
    th_finalize(status);
    return 0;
}

static int th_read_file_to_buffer(const char* path, char** out, size_t* out_len) {
    FILE* fp;
    long size;
    char* data;
    size_t got;
    if (!path || !out || !out_len) {
        return 0;
    }
    *out = NULL;
    *out_len = 0U;
    fp = fopen(path, "rb");
    if (!fp) {
        return 0;
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    size = ftell(fp);
    if (size < 0 || (unsigned long)size > THERON_CUE_MAX_BYTES) {
        fclose(fp);
        return 0;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }
    data = (char*)malloc((size_t)size + 1U);
    if (!data) {
        fclose(fp);
        return 0;
    }
    got = fread(data, 1U, (size_t)size, fp);
    fclose(fp);
    if (got != (size_t)size) {
        free(data);
        return 0;
    }
    data[size] = '\0';
    *out = data;
    *out_len = (size_t)size;
    return 1;
}

static int th_resolve_cue_candidate_path(const char* cue_path,
                                         FirestaffTheronMediaStatus* status) {
    char parent[FIRESTAFF_THERON_MEDIA_PATH_CAPACITY];
    char resolved[FIRESTAFF_THERON_MEDIA_PATH_CAPACITY];
    if (!cue_path || !status || status->candidate_path[0] == '\0') {
        return 0;
    }
    if (!FSP_ParentDir(parent, sizeof(parent), cue_path)) {
        return 0;
    }
    if (!FSP_JoinPath(resolved, sizeof(resolved), parent, status->candidate_path)) {
        return 0;
    }
    th_copy(status->candidate_path, sizeof(status->candidate_path), resolved);
    if (status->layout == FIRESTAFF_THERON_MEDIA_LAYOUT_ISO ||
        status->layout == FIRESTAFF_THERON_MEDIA_LAYOUT_ISO_OGG_CUE) {
        status->has_iso9660_pvd = th_file_has_iso9660_pvd(resolved);
    }
    return 1;
}

int FirestaffTheronMedia_ClassifyPath(const char* path,
                                      FirestaffTheronMediaStatus* status) {
    if (!path || !status) {
        return -1;
    }
    FirestaffTheronMedia_Init(status);
    if (th_has_ext(path, ".cue")) {
        char* cue = NULL;
        size_t cue_len = 0U;
        int rc;
        if (!th_read_file_to_buffer(path, &cue, &cue_len)) {
            return -1;
        }
        rc = FirestaffTheronMedia_ParseCue(cue, cue_len, status);
        free(cue);
        if (rc != 0) {
            return -1;
        }
        (void)th_resolve_cue_candidate_path(path, status);
        return 0;
    }
    if (th_has_ext(path, ".iso")) {
        status->layout = FIRESTAFF_THERON_MEDIA_LAYOUT_ISO;
        status->iso_file_count = 1;
        status->has_track02_data = 1;
        status->data_track_number = 2;
        status->launch_candidate = 1;
        status->has_iso9660_pvd = th_file_has_iso9660_pvd(path);
        th_copy(status->candidate_path, sizeof(status->candidate_path), path);
        return 0;
    }
    if (th_has_ext(path, ".bin")) {
        status->layout = FIRESTAFF_THERON_MEDIA_LAYOUT_RAW_BIN;
        status->bin_file_count = 1;
        status->has_track02_data = 1;
        status->data_track_number = 2;
        status->launch_candidate = 1;
        status->has_iso9660_pvd = th_file_has_iso9660_pvd(path);
        th_copy(status->candidate_path, sizeof(status->candidate_path), path);
        return 0;
    }
    if (th_has_ext(path, ".ogg")) {
        status->layout = FIRESTAFF_THERON_MEDIA_LAYOUT_OGG_ONLY;
        status->ogg_file_count = 1;
        status->audio_track_count = 1;
        th_copy(status->candidate_path, sizeof(status->candidate_path), path);
        return 0;
    }
    return -1;
}

static int th_status_rank(const FirestaffTheronMediaStatus* status) {
    if (!status) {
        return 0;
    }
    if (status->layout == FIRESTAFF_THERON_MEDIA_LAYOUT_BIN_CUE) {
        return 60;
    }
    if (status->layout == FIRESTAFF_THERON_MEDIA_LAYOUT_ISO_OGG_CUE) {
        return 55;
    }
    if (status->layout == FIRESTAFF_THERON_MEDIA_LAYOUT_ISO) {
        return status->has_iso9660_pvd ? 50 : 45;
    }
    if (status->layout == FIRESTAFF_THERON_MEDIA_LAYOUT_RAW_BIN) {
        return 40;
    }
    if (status->layout == FIRESTAFF_THERON_MEDIA_LAYOUT_OGG_ONLY) {
        return 10;
    }
    return 0;
}

static int th_consider_path(const char* path,
                            FirestaffTheronMediaStatus* best,
                            int* best_rank) {
    FirestaffTheronMediaStatus candidate;
    int rank;
    if (!path || !best || !best_rank) {
        return 0;
    }
    if (FirestaffTheronMedia_ClassifyPath(path, &candidate) != 0) {
        return 0;
    }
    rank = th_status_rank(&candidate);
    if (rank > *best_rank) {
        *best = candidate;
        *best_rank = rank;
    }
    return 1;
}

static int th_scan_dir(const char* root,
                       int depth,
                       int* visited,
                       FirestaffTheronMediaStatus* best,
                       int* best_rank) {
#if defined(_WIN32)
    char pattern[FIRESTAFF_THERON_MEDIA_PATH_CAPACITY];
    intptr_t handle;
    struct _finddata_t ent;
#else
    DIR* dir;
    struct dirent* ent;
#endif
    if (!root || !visited || !best || !best_rank ||
        depth > THERON_SCAN_MAX_DEPTH || *visited >= THERON_SCAN_MAX_FILES) {
        return 0;
    }
#if defined(_WIN32)
    if (!FSP_JoinPath(pattern, sizeof(pattern), root, "*")) {
        return 0;
    }
    handle = _findfirst(pattern, &ent);
    if (handle == -1) {
        return 0;
    }
    do {
        char child[FIRESTAFF_THERON_MEDIA_PATH_CAPACITY];
        const char* name = ent.name;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            continue;
        }
        if (!FSP_JoinPath(child, sizeof(child), root, name)) {
            continue;
        }
        if (ent.attrib & _A_SUBDIR) {
            th_scan_dir(child, depth + 1, visited, best, best_rank);
        } else {
            ++(*visited);
            (void)th_consider_path(child, best, best_rank);
        }
    } while (*visited < THERON_SCAN_MAX_FILES && _findnext(handle, &ent) == 0);
    _findclose(handle);
#else
    dir = opendir(root);
    if (!dir) {
        return 0;
    }
    while (*visited < THERON_SCAN_MAX_FILES && (ent = readdir(dir)) != NULL) {
        char child[FIRESTAFF_THERON_MEDIA_PATH_CAPACITY];
        const char* name = ent->d_name;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            continue;
        }
        if (!FSP_JoinPath(child, sizeof(child), root, name)) {
            continue;
        }
        if (FSP_DirExists(child)) {
            th_scan_dir(child, depth + 1, visited, best, best_rank);
        } else {
            ++(*visited);
            (void)th_consider_path(child, best, best_rank);
        }
    }
    closedir(dir);
#endif
    return *best_rank > 0 ? 1 : 0;
}

int FirestaffTheronMedia_ClassifyDirectory(const char* root,
                                           FirestaffTheronMediaStatus* status) {
    int visited = 0;
    int best_rank = 0;
    if (!root || !status) {
        return -1;
    }
    FirestaffTheronMedia_Init(status);
    if (!FSP_DirExists(root)) {
        return -1;
    }
    return th_scan_dir(root, 0, &visited, status, &best_rank) && best_rank > 0 ? 0 : -1;
}

const char* FirestaffTheronMedia_LayoutId(FirestaffTheronMediaLayout layout) {
    switch (layout) {
        case FIRESTAFF_THERON_MEDIA_LAYOUT_RAW_BIN: return "raw-bin";
        case FIRESTAFF_THERON_MEDIA_LAYOUT_ISO: return "iso";
        case FIRESTAFF_THERON_MEDIA_LAYOUT_BIN_CUE: return "bin-cue";
        case FIRESTAFF_THERON_MEDIA_LAYOUT_ISO_OGG_CUE: return "iso-ogg-cue";
        case FIRESTAFF_THERON_MEDIA_LAYOUT_OGG_ONLY: return "ogg-only";
        case FIRESTAFF_THERON_MEDIA_LAYOUT_UNKNOWN:
        default: return "unknown";
    }
}

const char* FirestaffTheronMedia_LayoutLabel(FirestaffTheronMediaLayout layout) {
    switch (layout) {
        case FIRESTAFF_THERON_MEDIA_LAYOUT_RAW_BIN: return "raw Track 02 BIN";
        case FIRESTAFF_THERON_MEDIA_LAYOUT_ISO: return "Track 02 ISO";
        case FIRESTAFF_THERON_MEDIA_LAYOUT_BIN_CUE: return "BIN/CUE Track 02";
        case FIRESTAFF_THERON_MEDIA_LAYOUT_ISO_OGG_CUE: return "ISO/OGG CUE";
        case FIRESTAFF_THERON_MEDIA_LAYOUT_OGG_ONLY: return "OGG audio only";
        case FIRESTAFF_THERON_MEDIA_LAYOUT_UNKNOWN:
        default: return "unknown";
    }
}

static int th_check(int condition, int* failures) {
    if (!condition && failures) {
        ++(*failures);
    }
    return condition;
}

int FirestaffTheronMedia_SelfTest(void) {
    int failures = 0;
    FirestaffTheronMediaStatus s;
    const char* bin_cue =
        "FILE \"Track01.ogg\" OGG\n"
        "  TRACK 01 AUDIO\n"
        "    INDEX 01 00:00:00\n"
        "FILE \"track02.bin\" BINARY\n"
        "  TRACK 02 MODE1/2352\n"
        "    INDEX 01 00:00:00\n";
    const char* iso_ogg_cue =
        "REM Firestaff synthetic Theron ISO/OGG layout\n"
        "FILE \"Track01.ogg\" OGG\n"
        "  TRACK 01 AUDIO\n"
        "    INDEX 01 00:00:00\n"
        "FILE \"TQUS02.iso\" BINARY\n"
        "  TRACK 02 MODE1/2048\n"
        "    INDEX 01 00:00:00\n"
        "FILE \"Track03.ogg\" OGG\n"
        "  TRACK 03 AUDIO\n"
        "    INDEX 01 00:00:00\n";
    const char* ogg_only =
        "FILE \"Track01.ogg\" OGG\n"
        "  TRACK 01 AUDIO\n"
        "    INDEX 01 00:00:00\n";

    th_check(FirestaffTheronMedia_ParseCue(bin_cue, strlen(bin_cue), &s) == 0, &failures);
    th_check(s.layout == FIRESTAFF_THERON_MEDIA_LAYOUT_BIN_CUE, &failures);
    th_check(s.has_track02_data == 1 && s.launch_candidate == 1, &failures);
    th_check(s.bin_file_count == 1 && s.ogg_file_count == 1, &failures);

    th_check(FirestaffTheronMedia_ParseCue(iso_ogg_cue, strlen(iso_ogg_cue), &s) == 0, &failures);
    th_check(s.layout == FIRESTAFF_THERON_MEDIA_LAYOUT_ISO_OGG_CUE, &failures);
    th_check(s.iso_file_count == 1 && s.ogg_file_count == 2, &failures);
    th_check(strcmp(s.candidate_path, "TQUS02.iso") == 0, &failures);

    th_check(FirestaffTheronMedia_ParseCue(ogg_only, strlen(ogg_only), &s) == 0, &failures);
    th_check(s.layout == FIRESTAFF_THERON_MEDIA_LAYOUT_OGG_ONLY, &failures);
    th_check(s.launch_candidate == 0, &failures);

    th_check(FirestaffTheronMedia_ParseCue("TRACK 02 MODE1/2048\n",
                                           strlen("TRACK 02 MODE1/2048\n"),
                                           &s) != 0,
             &failures);
    th_check(strcmp(FirestaffTheronMedia_LayoutId(FIRESTAFF_THERON_MEDIA_LAYOUT_ISO_OGG_CUE),
                    "iso-ogg-cue") == 0,
             &failures);
    return failures == 0 ? 0 : -1;
}
