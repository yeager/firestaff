#include "firestaff_fmtowns_disc.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

static uint32_t rd_u32le(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static size_t sector_offset(FmtownsSectorFormat fmt, uint32_t lba) {
    if (fmt == FMTOWNS_SECTOR_2352)
        return (size_t)lba * 2352u;
    return (size_t)lba * 2048u;
}

static const uint8_t *sector_data(const uint8_t *image, size_t image_size,
                                  FmtownsSectorFormat fmt, uint32_t lba) {
    size_t off = sector_offset(fmt, lba);
    size_t data_off = (fmt == FMTOWNS_SECTOR_2352) ? off + 16u : off;
    if (data_off + 2048u > image_size) return NULL;
    return image + data_off;
}

static void clean_iso_name(char *dst, const char *src, size_t len) {
    size_t i;
    for (i = 0; i < len && i < 63; i++) {
        if (src[i] == ';') break;
        dst[i] = src[i];
    }
    while (i > 0 && dst[i-1] == ' ') i--;
    dst[i] = '\0';
}

static int read_directory(const uint8_t *image, size_t image_size,
                          FmtownsSectorFormat fmt,
                          uint32_t dir_lba, uint32_t dir_size,
                          const char *prefix,
                          FmtownsDiscProbeResult *out) {
    uint32_t remaining = dir_size;
    uint32_t lba = dir_lba;

    while (remaining > 0) {
        const uint8_t *sec = sector_data(image, image_size, fmt, lba);
        if (!sec) return -1;

        uint32_t chunk = remaining > 2048u ? 2048u : remaining;
        uint32_t pos = 0;
        while (pos + 33 < chunk) {
            uint8_t rec_len = sec[pos];
            if (rec_len == 0) break;
            if (pos + rec_len > chunk) break;

            uint8_t name_len = sec[pos + 32];
            if (name_len == 0 || name_len == 1) {
                pos += rec_len;
                continue;
            }

            uint32_t entry_lba = rd_u32le(sec + pos + 2);
            uint32_t entry_size = rd_u32le(sec + pos + 10);
            uint8_t flags = sec[pos + 25];
            int is_dir = (flags & 2) != 0;

            if (out->entry_count < FMTOWNS_MAX_ISO_ENTRIES) {
                FmtownsIsoEntry *e = &out->entries[out->entry_count];
                char raw_name[64];
                clean_iso_name(raw_name, (const char *)sec + pos + 33, name_len);

                if (prefix[0]) {
                    snprintf(e->name, sizeof(e->name), "%s/%s", prefix, raw_name);
                } else {
                    strncpy(e->name, raw_name, sizeof(e->name) - 1);
                    e->name[sizeof(e->name) - 1] = '\0';
                }
                e->lba = entry_lba;
                e->size = entry_size;
                e->is_directory = is_dir;
                out->entry_count++;
            }

            if (is_dir) {
                char sub_prefix[128];
                char raw_name[64];
                clean_iso_name(raw_name, (const char *)sec + pos + 33, name_len);
                if (prefix[0])
                    snprintf(sub_prefix, sizeof(sub_prefix), "%s/%s", prefix, raw_name);
                else
                    strncpy(sub_prefix, raw_name, sizeof(sub_prefix) - 1);
                read_directory(image, image_size, fmt,
                              entry_lba, entry_size, sub_prefix, out);
            }

            pos += rec_len;
        }
        remaining -= chunk;
        lba++;
    }
    return 0;
}

int fmtowns_disc_probe(const uint8_t *image, size_t image_size,
                       FmtownsSectorFormat format,
                       FmtownsDiscProbeResult *out) {
    const uint8_t *pvd;

    if (!image || !out) return -1;
    memset(out, 0, sizeof(*out));
    out->sector_format = format;

    pvd = sector_data(image, image_size, format, 16);
    if (!pvd) return -1;
    if (pvd[0] != 1 || memcmp(pvd + 1, "CD001", 5) != 0) return -1;

    memcpy(out->system_id, pvd + 8, 32);
    out->system_id[32] = '\0';
    memcpy(out->volume_id, pvd + 40, 32);
    out->volume_id[32] = '\0';

    for (int i = 31; i >= 0 && out->system_id[i] == ' '; i--)
        out->system_id[i] = '\0';
    for (int i = 31; i >= 0 && out->volume_id[i] == ' '; i--)
        out->volume_id[i] = '\0';

    out->volume_sectors = rd_u32le(pvd + 80);

    uint32_t root_lba = rd_u32le(pvd + 158);
    uint32_t root_size = rd_u32le(pvd + 166);

    read_directory(image, image_size, format, root_lba, root_size, "", out);
    out->valid = 1;
    return 0;
}

const FmtownsIsoEntry *fmtowns_disc_find(const FmtownsDiscProbeResult *result,
                                         const char *name) {
    if (!result || !name) return NULL;
    for (int i = 0; i < result->entry_count; i++) {
        if (strcasecmp(result->entries[i].name, name) == 0)
            return &result->entries[i];
    }
    return NULL;
}

int fmtowns_disc_extract_alloc(const uint8_t *image, size_t image_size,
                               FmtownsSectorFormat format,
                               const FmtownsIsoEntry *entry,
                               uint8_t **out_data, size_t *out_size) {
    if (!image || !entry || !out_data || !out_size) return -1;
    *out_data = NULL;
    *out_size = 0;

    uint8_t *buf = (uint8_t *)malloc(entry->size);
    if (!buf) return -1;

    uint32_t remaining = entry->size;
    uint32_t lba = entry->lba;
    uint32_t written = 0;

    while (remaining > 0) {
        const uint8_t *sec = sector_data(image, image_size, format, lba);
        if (!sec) { free(buf); return -1; }
        uint32_t chunk = remaining > 2048u ? 2048u : remaining;
        memcpy(buf + written, sec, chunk);
        written += chunk;
        remaining -= chunk;
        lba++;
    }

    *out_data = buf;
    *out_size = entry->size;
    return 0;
}

int fmtowns_cdda_extract(const uint8_t *image, size_t image_size,
                         uint32_t start_sector, uint32_t sector_count,
                         uint8_t **out_pcm, size_t *out_pcm_size) {
    if (!image || !out_pcm || !out_pcm_size) return -1;
    *out_pcm = NULL;
    *out_pcm_size = 0;

    size_t pcm_bytes = (size_t)sector_count * FMTOWNS_CDDA_SECTOR_SIZE;
    size_t start_off = (size_t)start_sector * FMTOWNS_CDDA_SECTOR_SIZE;

    if (start_off + pcm_bytes > image_size) return -1;

    uint8_t *buf = (uint8_t *)malloc(pcm_bytes);
    if (!buf) return -1;

    memcpy(buf, image + start_off, pcm_bytes);
    *out_pcm = buf;
    *out_pcm_size = pcm_bytes;
    return 0;
}

int fmtowns_cue_parse_track_starts(const char *cue, size_t cue_size,
                                   uint32_t *track_starts, int max_tracks) {
    const char *p = cue;
    const char *end = cue + cue_size;
    int count = 0;
    int current_track = -1;

    if (!cue || !track_starts) return -1;
    memset(track_starts, 0, (size_t)max_tracks * sizeof(uint32_t));

    while (p < end) {
        while (p < end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) p++;
        if (p >= end) break;

        const char *line = p;
        while (p < end && *p != '\n' && *p != '\r') p++;
        size_t line_len = (size_t)(p - line);

        char buf[256];
        if (line_len >= sizeof(buf)) continue;
        memcpy(buf, line, line_len);
        buf[line_len] = '\0';

        if (strncmp(buf, "TRACK", 5) == 0) {
            current_track = atoi(buf + 5);
        } else {
            char *idx = strstr(buf, "INDEX 01");
            if (!idx) continue;
            char *ts = idx + 8;
            while (*ts == ' ' || *ts == '\t') ts++;
            int mm = 0, ss = 0, ff = 0;
            if (sscanf(ts, "%d:%d:%d", &mm, &ss, &ff) == 3 &&
                current_track >= 0 && current_track < max_tracks) {
                track_starts[current_track] =
                    (uint32_t)(mm * 60 * 75 + ss * 75 + ff);
                if (current_track + 1 > count)
                    count = current_track + 1;
            }
        }
    }
    return count;
}
