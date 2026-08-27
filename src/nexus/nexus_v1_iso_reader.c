
#include "nexus_v1_iso_reader.h"
#include "firestaff_zip_extract.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#ifdef _WIN32
#include <windows.h>
#define strcasecmp _stricmp
#else
#include <dirent.h>
#endif

static int seek_file(FILE *fp, int64_t offset)
{
#ifdef _WIN32
    return _fseeki64(fp, offset, SEEK_SET);
#else
    return fseek(fp, (long)offset, SEEK_SET);
#endif
}

static int read_sector_payload(const Nexus_ISOReader *reader, uint32_t sector,
                               int sector_size, int data_offset, uint8_t *buf) {
    int64_t offset = (int64_t)sector * sector_size + data_offset;
    size_t read_size;
    memset(buf, 0, NEXUS_ISO_DATA_SIZE);
    if (!reader || offset < 0) return -1;
    if (reader->memory) {
        if ((uint64_t)offset + NEXUS_ISO_DATA_SIZE > reader->memory_size) return -1;
        memcpy(buf, reader->memory + offset, NEXUS_ISO_DATA_SIZE);
        return 0;
    }
    if (!reader->fp || seek_file(reader->fp, offset) != 0) return -1;
    read_size = fread(buf, 1, NEXUS_ISO_DATA_SIZE, reader->fp);
    return read_size == NEXUS_ISO_DATA_SIZE ? 0 : -1;
}

static uint32_t r32le(const uint8_t *p) {
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}

/* Parse one ISO 9660 directory record */
static int parse_dir_record(const uint8_t *data, int offset, int buf_size,
                            Nexus_ISOFile *out) {
    int rec_len, name_len, i;
    if (!data || !out) return 0;
    if (offset < 0 || offset > buf_size || buf_size - offset < 1) return 0;
    rec_len = data[offset];
    if (rec_len == 0) return 0;
    /* A non-padding ISO 9660 record has a 33-byte fixed header and at
     * least one byte of identifier. Treat shorter records as corruption,
     * rather than allowing the identifier to consume the next record. */
    if (rec_len < 34 || rec_len > buf_size - offset) return -1;

    out->lba = r32le(data + offset + 2);
    out->size = r32le(data + offset + 10);
    out->is_dir = (data[offset + 25] & 0x02) != 0;

    name_len = data[offset + 32];
    if (name_len > rec_len - 33) return -1;
    memset(out->name, 0, sizeof(out->name));
    for (i = 0; i < name_len && i < 63; i++)
        out->name[i] = data[offset + 33 + i];

    /* Strip version (;1) */
    char *semi = strchr(out->name, ';');
    if (semi) *semi = 0;

    return rec_len;
}

/* The directory extents below come from the disc image, so a crafted or
 * corrupt one can point a subdirectory at its own LBA (or build a longer
 * cycle). Directory records never increment *count, so the max_files guard
 * cannot break such a loop, and every frame holds a 2048-byte sector buffer
 * -- roughly 4000 frames exhaust an 8 MB stack. ISO 9660 itself limits
 * hierarchies to 8 levels, so cap the depth. */
#define NEXUS_ISO_MAX_DIR_DEPTH 8

/* Recursively parse directory tree */
static int parse_directory_depth(const Nexus_ISOReader *reader, uint32_t dir_lba, uint32_t dir_size,
    int sector_size, int data_offset,
    Nexus_ISOFile *files, int *count, int max_files, int depth)
{
    uint8_t sector_buf[NEXUS_ISO_DATA_SIZE];
    int sectors = (int)(dir_size / NEXUS_ISO_DATA_SIZE) +
                  (dir_size % NEXUS_ISO_DATA_SIZE != 0U ? 1 : 0);
    int s, offset;

    if (depth > NEXUS_ISO_MAX_DIR_DEPTH) return -1;

    for (s = 0; s < sectors; s++) {
        if ((uint32_t)s > UINT32_MAX - dir_lba) return -1;
        uint32_t remaining = dir_size - (uint32_t)s * NEXUS_ISO_DATA_SIZE;
        int sector_bytes = remaining > NEXUS_ISO_DATA_SIZE
            ? NEXUS_ISO_DATA_SIZE : (int)remaining;
        if (read_sector_payload(reader, dir_lba + s, sector_size, data_offset,
                                sector_buf) < 0) return -1;

        offset = 0;
        while (offset < sector_bytes && *count < max_files) {
            Nexus_ISOFile entry;
            int rec_len = parse_dir_record(sector_buf, offset,
                                             sector_bytes, &entry);
            if (rec_len < 0) return -1;
            if (rec_len == 0) break;

            if (entry.name[0] && entry.name[0] != 0 && entry.name[0] != 1) {
                if (!entry.is_dir) {
                    files[*count] = entry;
                    (*count)++;
                } else if (strcmp(entry.name, ".") != 0 &&
                           strcmp(entry.name, "..") != 0 &&
                           entry.lba != dir_lba) {
                    /* Recurse into subdirectory */
                    if (parse_directory_depth(reader, entry.lba, entry.size,
                                               sector_size, data_offset,
                                               files, count, max_files,
                                               depth + 1) < 0) {
                        return -1;
                    }
                }
            }
            offset += rec_len;
        }
    }
    return 0;
}

static int parse_directory(const Nexus_ISOReader *reader, uint32_t dir_lba, uint32_t dir_size,
    int sector_size, int data_offset,
    Nexus_ISOFile *files, int *count, int max_files)
{
    return parse_directory_depth(reader, dir_lba, dir_size, sector_size,
                                 data_offset, files, count, max_files, 0);
}

int nexus_iso_open(Nexus_ISOReader *reader, const char *bin_path) {
    uint8_t pvd[NEXUS_ISO_DATA_SIZE];
    uint32_t root_lba, root_size;
    int sector_size = NEXUS_ISO_SECTOR_SIZE;
    int data_offset = NEXUS_ISO_DATA_OFFSET;

    if (!reader || !bin_path) return -1;
    memset(reader, 0, sizeof(*reader));
    strncpy(reader->path, bin_path, sizeof(reader->path) - 1);
    reader->sector_size = sector_size;
    reader->data_offset = data_offset;

    reader->fp = fopen(bin_path, "rb");
    if (!reader->fp) return -1;

    /* Saturn dumps are commonly raw MODE1/2352 BINs; some users stage
     * plain 2048-byte ISO data images.  Accept both because the launcher
     * advertises ISO/BIN as first-class Nexus input. */
    if (read_sector_payload(reader, 16, sector_size, data_offset, pvd) < 0 ||
        pvd[0] != 1 || memcmp(pvd + 1, "CD001", 5) != 0) {
        sector_size = NEXUS_ISO_DATA_SIZE;
        data_offset = 0;
        if (read_sector_payload(reader, 16, sector_size, data_offset, pvd) < 0 ||
            pvd[0] != 1 || memcmp(pvd + 1, "CD001", 5) != 0) {
            goto fail;
        }
    }
    reader->sector_size = sector_size;
    reader->data_offset = data_offset;

    root_lba = r32le(pvd + 158);
    root_size = r32le(pvd + 166);

    /* Parse file tree */
    reader->file_count = 0;
    if (parse_directory(reader, root_lba, root_size,
        reader->sector_size, reader->data_offset,
        reader->files, &reader->file_count, NEXUS_ISO_MAX_FILES) < 0) {
        goto fail;
    }

    reader->valid = 1;
    return reader->file_count;

fail:
    fclose(reader->fp);
    reader->fp = NULL;
    return -1;
}

int nexus_iso_open_memory(Nexus_ISOReader *reader, uint8_t *data,
                          size_t data_size, const char *source_name) {
    uint8_t pvd[NEXUS_ISO_DATA_SIZE];
    uint32_t root_lba, root_size;
    if (!reader || !data || data_size == 0U) { free(data); return -1; }
    memset(reader, 0, sizeof(*reader));
    reader->memory = data;
    reader->memory_size = data_size;
    reader->sector_size = NEXUS_ISO_SECTOR_SIZE;
    reader->data_offset = NEXUS_ISO_DATA_OFFSET;
    if (source_name) strncpy(reader->path, source_name, sizeof(reader->path) - 1U);
    if (read_sector_payload(reader, 16, reader->sector_size, reader->data_offset, pvd) < 0 ||
        pvd[0] != 1 || memcmp(pvd + 1, "CD001", 5) != 0) {
        reader->sector_size = NEXUS_ISO_DATA_SIZE;
        reader->data_offset = 0;
        if (read_sector_payload(reader, 16, reader->sector_size, reader->data_offset, pvd) < 0 ||
            pvd[0] != 1 || memcmp(pvd + 1, "CD001", 5) != 0) goto fail;
    }
    root_lba = r32le(pvd + 158);
    root_size = r32le(pvd + 166);
    if (parse_directory(reader, root_lba, root_size, reader->sector_size,
                        reader->data_offset, reader->files, &reader->file_count,
                        NEXUS_ISO_MAX_FILES) < 0) goto fail;
    reader->valid = 1;
    return reader->file_count;
fail:
    free(reader->memory);
    reader->memory = NULL;
    reader->memory_size = 0U;
    return -1;
}

static int cue_keyword(const char *p, const char *keyword)
{
    size_t i;
    if (!p || !keyword) return 0;
    while (*p && isspace((unsigned char)*p)) ++p;
    for (i = 0; keyword[i]; ++i) {
        if (!p[i] || tolower((unsigned char)p[i]) !=
                     tolower((unsigned char)keyword[i])) {
            return 0;
        }
    }
    return p[i] == '\0' || isspace((unsigned char)p[i]);
}

static int cue_file_name(const char *line, char out_name[256])
{
    const char *p;
    const char *end;
    size_t count;

    if (!line || !out_name || !cue_keyword(line, "FILE")) return 0;
    p = line;
    while (*p && !isspace((unsigned char)*p)) ++p;
    while (*p && isspace((unsigned char)*p)) ++p;
    if (*p == '"') {
        ++p;
        end = strchr(p, '"');
    } else {
        end = p;
        while (*end && !isspace((unsigned char)*end)) ++end;
    }
    if (!end || end <= p || (count = (size_t)(end - p)) >= 256U) return 0;
    memcpy(out_name, p, count);
    out_name[count] = '\0';
    /* CUE sheets are frequently authored on Windows while being launched on
     * POSIX. A path separator is not part of the disc image identity. */
    for (size_t i = 0; i < count; ++i) {
        if (out_name[i] == '\\') out_name[i] = '/';
    }
    return 1;
}

int nexus_iso_open_cue(Nexus_ISOReader *reader, const char *cue_path) {
    FILE *cue;
    char line[512];
    char cue_dir[512];
    char candidate_name[256];
    char candidate_path[768];
    char *last_slash;
    int matches = 0;

    if (!reader || !cue_path) return -1;
    memset(reader, 0, sizeof(*reader));
    cue = fopen(cue_path, "r");
    if (!cue) return -1;

    strncpy(cue_dir, cue_path, sizeof(cue_dir) - 1U);
    cue_dir[sizeof(cue_dir) - 1U] = '\0';
    last_slash = strrchr(cue_dir, '/');
    if (!last_slash) last_slash = strrchr(cue_dir, '\\');
    if (last_slash) {
        last_slash[1] = '\0';
    } else {
        cue_dir[0] = '\0';
    }

    /* A Saturn CUE can list CDDA before data, split files, or a nonstandard
     * track order. The original failure chose the first FILE and fed audio
     * bytes to the ISO/PRS3 path. Probe each declared payload and retain only
     * the one whose ISO tree carries the Nexus disc signature. */
    while (fgets(line, sizeof(line), cue)) {
        Nexus_ISOReader candidate;
        int count;
        if (!cue_file_name(line, candidate_name)) continue;
        if (snprintf(candidate_path, sizeof(candidate_path), "%s%s",
                     cue_dir, candidate_name) <= 0 ||
            strlen(candidate_path) >= sizeof(candidate_path)) {
            continue;
        }
        memset(&candidate, 0, sizeof(candidate));
        count = nexus_iso_open(&candidate, candidate_path);
        if (count > 0 && nexus_iso_is_nexus(&candidate)) {
            if (++matches > 1) {
                nexus_iso_close(&candidate);
                nexus_iso_close(reader);
                fclose(cue);
                return -1;
            }
            *reader = candidate;
        } else nexus_iso_close(&candidate);
    }
    fclose(cue);
    return matches == 1 ? reader->file_count : -1;
}

static int nexus_iso_path_equals(const char *left, const char *right)
{
    size_t i;
    if (!left || !right) return 0;
    for (i = 0; left[i] && right[i]; ++i) {
        char a = left[i] == '\\' ? '/' : left[i];
        char b = right[i] == '\\' ? '/' : right[i];
#ifdef _WIN32
        if (tolower((unsigned char)a) != tolower((unsigned char)b)) return 0;
#else
        if (a != b) return 0;
#endif
    }
    return left[i] == '\0' && right[i] == '\0';
}

static int nexus_iso_path_has_cue_extension(const char *path)
{
    const char *dot = path ? strrchr(path, '.') : NULL;
    return dot && strcasecmp(dot, ".cue") == 0;
}

static int nexus_iso_consider_owning_cue(const char *cue_path,
                                         const char *data_track_path,
                                         char *out_cue_path,
                                         int out_cue_path_size,
                                         int *matches)
{
    Nexus_ISOReader reader;
    if (!cue_path || !data_track_path || !out_cue_path || !matches ||
        nexus_iso_open_cue(&reader, cue_path) < 0) return 0;
    if (nexus_iso_path_equals(reader.path, data_track_path)) {
        ++*matches;
        if (*matches == 1 && (int)strlen(cue_path) < out_cue_path_size) {
            memcpy(out_cue_path, cue_path, strlen(cue_path) + 1U);
        }
    }
    nexus_iso_close(&reader);
    return 1;
}

int nexus_iso_find_cue_for_data_track(const char *data_track_path,
                                      char *out_cue_path,
                                      int out_cue_path_size)
{
    char directory[512];
    char *separator;
    int matches = 0;

    if (!data_track_path || !data_track_path[0] || !out_cue_path ||
        out_cue_path_size <= 1) return -1;
    out_cue_path[0] = '\0';
    if (strlen(data_track_path) >= sizeof(directory)) return -1;
    memcpy(directory, data_track_path, strlen(data_track_path) + 1U);
    separator = strrchr(directory, '/');
    if (!separator) separator = strrchr(directory, '\\');
    if (!separator) return -1;
    separator[1] = '\0';

#ifdef _WIN32
    {
        WIN32_FIND_DATAA entry;
        HANDLE search;
        char pattern[sizeof(directory) + 5U];
        if (snprintf(pattern, sizeof(pattern), "%s*.cue", directory) <= 0 ||
            strlen(pattern) >= sizeof(pattern)) return -1;
        search = FindFirstFileA(pattern, &entry);
        if (search == INVALID_HANDLE_VALUE) return -1;
        do {
            char cue_path[768];
            if ((entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
                !nexus_iso_path_has_cue_extension(entry.cFileName)) continue;
            if (snprintf(cue_path, sizeof(cue_path), "%s%s", directory,
                         entry.cFileName) <= 0 || strlen(cue_path) >= sizeof(cue_path)) continue;
            nexus_iso_consider_owning_cue(cue_path, data_track_path,
                                          out_cue_path, out_cue_path_size, &matches);
        } while (FindNextFileA(search, &entry));
        FindClose(search);
    }
#else
    {
        DIR *dir = opendir(directory);
        struct dirent *entry;
        if (!dir) return -1;
        while ((entry = readdir(dir)) != NULL) {
            char cue_path[768];
            if (!nexus_iso_path_has_cue_extension(entry->d_name)) continue;
            if (snprintf(cue_path, sizeof(cue_path), "%s%s", directory,
                         entry->d_name) <= 0 || strlen(cue_path) >= sizeof(cue_path)) continue;
            nexus_iso_consider_owning_cue(cue_path, data_track_path,
                                          out_cue_path, out_cue_path_size, &matches);
        }
        closedir(dir);
    }
#endif
    if (matches != 1) out_cue_path[0] = '\0';
    return matches == 1 ? 0 : -1;
}

int nexus_iso_cue_media_receipt(const char *cue_path,
                               Nexus_ISO_CueMediaReceipt *out)
{
    FILE *cue;
    char line[512];
    char cue_dir[512];
    char candidate_name[256];
    char candidate_path[768];
    char *last_slash;

    if (!cue_path || !out) return -1;
    memset(out, 0, sizeof(*out));
    cue = fopen(cue_path, "r");
    if (!cue) return -1;

    strncpy(cue_dir, cue_path, sizeof(cue_dir) - 1U);
    cue_dir[sizeof(cue_dir) - 1U] = '\0';
    last_slash = strrchr(cue_dir, '/');
    if (!last_slash) last_slash = strrchr(cue_dir, '\\');
    if (last_slash) {
        last_slash[1] = '\0';
    } else {
        cue_dir[0] = '\0';
    }

    while (fgets(line, sizeof(line), cue)) {
        FILE *payload;
        if (!cue_file_name(line, candidate_name)) continue;
        if (snprintf(candidate_path, sizeof(candidate_path), "%s%s",
                     cue_dir, candidate_name) <= 0 ||
            strlen(candidate_path) >= sizeof(candidate_path)) {
            fclose(cue);
            return -1;
        }
        ++out->declared_file_count;
        payload = fopen(candidate_path, "rb");
        if (payload) {
            ++out->present_file_count;
            fclose(payload);
        } else {
            ++out->missing_file_count;
        }
    }
    fclose(cue);
    out->valid = out->declared_file_count > 0 &&
                 out->missing_file_count == 0;
    return out->valid ? 0 : 1;
}

int nexus_iso_cue_audio_track_path(const char *cue_path,
                                   int track_number,
                                   char *out_path,
                                   int out_path_size)
{
    FILE *cue;
    char line[512];
    char cue_dir[512];
    char current_file[256];
    char candidate[768];
    char keyword[16];
    char *last_slash;
    int matches = 0;

    if (!cue_path || !out_path || out_path_size <= 1 ||
        track_number < 1 || track_number > 99) return -1;
    out_path[0] = '\0';
    cue = fopen(cue_path, "r");
    if (!cue) return -1;
    strncpy(cue_dir, cue_path, sizeof(cue_dir) - 1U);
    cue_dir[sizeof(cue_dir) - 1U] = '\0';
    last_slash = strrchr(cue_dir, '/');
    if (!last_slash) last_slash = strrchr(cue_dir, '\\');
    if (last_slash) last_slash[1] = '\0';
    else cue_dir[0] = '\0';
    current_file[0] = '\0';

    while (fgets(line, sizeof(line), cue)) {
        int declared_track;
        char mode[32];
        if (cue_file_name(line, current_file)) continue;
        if (!current_file[0] ||
            sscanf(line, " %15s %d %31s", keyword, &declared_track, mode) != 3 ||
            strcasecmp(keyword, "TRACK") != 0) {
            continue;
        }
        if (declared_track != track_number || strcasecmp(mode, "AUDIO") != 0)
            continue;
        if (snprintf(candidate, sizeof(candidate), "%s%s", cue_dir,
                     current_file) <= 0 ||
            strlen(candidate) >= sizeof(candidate)) {
            fclose(cue);
            return -1;
        }
        {
            FILE *payload = fopen(candidate, "rb");
            if (!payload) {
                fclose(cue);
                return -1;
            }
            fclose(payload);
        }
        if (++matches != 1 || (int)strlen(candidate) >= out_path_size) {
            fclose(cue);
            out_path[0] = '\0';
            return -1;
        }
        memcpy(out_path, candidate, strlen(candidate) + 1U);
    }
    fclose(cue);
    if (matches != 1) out_path[0] = '\0';
    return matches == 1 ? 0 : -1;
}

static int nexus_iso_cue_audio_track_name(const uint8_t *cue,
                                          size_t cue_size,
                                          int track_number,
                                          char out_name[256])
{
    size_t offset = 0U;
    char current_file[256];
    int matches = 0;

    if (!cue || cue_size == 0U || !out_name || track_number < 1 ||
        track_number > 99) return -1;
    out_name[0] = '\0';
    current_file[0] = '\0';
    while (offset < cue_size) {
        char line[512];
        size_t line_size = 0U;
        char keyword[16];
        char mode[32];
        int declared_track;

        while (offset + line_size < cue_size && cue[offset + line_size] != '\n' &&
               line_size + 1U < sizeof(line)) ++line_size;
        if (offset + line_size < cue_size && cue[offset + line_size] != '\n') {
            while (offset < cue_size && cue[offset] != '\n') ++offset;
            if (offset < cue_size) ++offset;
            continue;
        }
        memcpy(line, cue + offset, line_size);
        line[line_size] = '\0';
        offset += line_size;
        if (offset < cue_size && cue[offset] == '\n') ++offset;
        if (cue_file_name(line, current_file)) continue;
        if (!current_file[0] ||
            sscanf(line, " %15s %d %31s", keyword, &declared_track, mode) != 3 ||
            strcasecmp(keyword, "TRACK") != 0 || declared_track != track_number ||
            strcasecmp(mode, "AUDIO") != 0) continue;
        if (++matches != 1) {
            out_name[0] = '\0';
            return -1;
        }
        memcpy(out_name, current_file, strlen(current_file) + 1U);
    }
    if (matches != 1) out_name[0] = '\0';
    return matches == 1 ? 0 : -1;
}

int nexus_iso_zip_cue_audio_track_path(const char *zip_path,
                                       int track_number,
                                       char *out_path,
                                       int out_path_size)
{
    uint8_t *cue = NULL;
    uint8_t *payload = NULL;
    size_t cue_size = 0U;
    size_t payload_size = 0U;
    char member_name[256];
    int result = -1;

    if (!zip_path || !zip_path[0] || !out_path || out_path_size <= 1) return -1;
    out_path[0] = '\0';
    if (firestaff_zip_extract_by_suffix(zip_path, ".cue", &cue, &cue_size) != 0 ||
        nexus_iso_cue_audio_track_name(cue, cue_size, track_number,
                                       member_name) != 0 ||
        firestaff_zip_extract_by_name(zip_path, member_name, &payload,
                                      &payload_size) != 0 || payload_size == 0U ||
        snprintf(out_path, (size_t)out_path_size, "%s::%s", zip_path,
                 member_name) >= out_path_size) {
        goto done;
    }
    result = 0;
done:
    free(cue);
    free(payload);
    if (result != 0) out_path[0] = '\0';
    return result;
}

const Nexus_ISOFile *nexus_iso_find(const Nexus_ISOReader *reader, const char *name) {
    int i;
    if (!reader || !name) return NULL;
    for (i = 0; i < reader->file_count; i++) {
        if (strcasecmp(reader->files[i].name, name) == 0)
            return &reader->files[i];
    }
    return NULL;
}

int nexus_iso_read_file(Nexus_ISOReader *reader, const Nexus_ISOFile *file,
    uint8_t *buffer, int buffer_size)
{
    uint8_t sector_buf[NEXUS_ISO_DATA_SIZE];
    int remaining, offset = 0;
    uint32_t sector;

    if (!reader || (!reader->fp && !reader->memory) || !file || !buffer || buffer_size < 0) return -1;
    if (file->size > (uint32_t)buffer_size || file->size > (uint32_t)INT_MAX)
        return -1;

    remaining = (int)file->size;
    sector = file->lba;

    while (remaining > 0) {
        int chunk = remaining > NEXUS_ISO_DATA_SIZE ? NEXUS_ISO_DATA_SIZE : remaining;
        if (read_sector_payload(reader, sector, reader->sector_size,
                                reader->data_offset, sector_buf) < 0) return -1;
        memcpy(buffer + offset, sector_buf, chunk);
        offset += chunk;
        remaining -= chunk;
        if (remaining > 0 && sector == UINT32_MAX) return -1;
        sector++;
    }
    return (int)file->size;
}

int nexus_iso_read_file_chunk(Nexus_ISOReader *reader, const Nexus_ISOFile *file,
    int file_offset, uint8_t *buffer, int chunk_size)
{
    uint8_t sector_buf[NEXUS_ISO_DATA_SIZE];
    int read_total = 0;
    uint32_t sector;
    int sector_offset;

    if (!reader || (!reader->fp && !reader->memory) || !file || !buffer ||
        file_offset < 0 || chunk_size < 0 ||
        (uint32_t)file_offset > file->size ||
        (uint32_t)chunk_size > file->size - (uint32_t)file_offset) {
        return -1;
    }
    if (chunk_size == 0) return 0;

    if ((uint32_t)(file_offset / NEXUS_ISO_DATA_SIZE) >
        UINT32_MAX - file->lba) return -1;
    sector = file->lba + (uint32_t)(file_offset / NEXUS_ISO_DATA_SIZE);
    sector_offset = file_offset % NEXUS_ISO_DATA_SIZE;

    while (read_total < chunk_size) {
        int avail, to_copy;
        if (read_sector_payload(reader, sector, reader->sector_size,
                                reader->data_offset, sector_buf) < 0) return -1;
        avail = NEXUS_ISO_DATA_SIZE - sector_offset;
        to_copy = chunk_size - read_total;
        if (to_copy > avail) to_copy = avail;
        memcpy(buffer + read_total, sector_buf + sector_offset, to_copy);
        read_total += to_copy;
        if (read_total < chunk_size && sector == UINT32_MAX) return -1;
        sector++;
        sector_offset = 0;
    }
    return read_total;
}

int nexus_iso_file_count(const Nexus_ISOReader *reader) {
    return reader ? reader->file_count : 0;
}

void nexus_iso_close(Nexus_ISOReader *reader) {
    if (reader && reader->fp) {
        fclose(reader->fp);
        reader->fp = NULL;
    }
    if (reader && reader->memory) {
        free(reader->memory);
        reader->memory = NULL;
        reader->memory_size = 0U;
    }
    if (reader) reader->valid = 0;
}

int nexus_iso_is_nexus(const Nexus_ISOReader *reader) {
    /* DM.BIN plus the first playable DGN are the admission signature.  The
     * title-only LEV00.DGN is not sufficient: accepting it alone lets a
     * disc enter the Nexus runtime without an authenticated gameplay level. */
    return reader && reader->valid &&
           nexus_iso_find(reader, "DM.BIN") != NULL &&
           nexus_iso_find(reader, "LEV01.DGN") != NULL;
}
