#include "csb_v1_fmtowns_cd.h"
#include <string.h>
#include <ctype.h>

#define RAW   CSB_FMTOWNS_CD_RAW_SECTOR_SIZE
#define COOKED CSB_FMTOWNS_CD_COOKED_SECTOR_SIZE
#define DOFF  CSB_FMTOWNS_CD_SECTOR_DATA_OFFSET

static uint32_t rd32le(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static const uint8_t *sector_data(const uint8_t *bin, size_t bin_size,
                                   uint32_t lba, int is_raw) {
    size_t offset;
    if (is_raw) {
        offset = (size_t)lba * RAW + DOFF;
        if (offset + COOKED > bin_size) return NULL;
    } else {
        offset = (size_t)lba * COOKED;
        if (offset + COOKED > bin_size) return NULL;
    }
    return bin + offset;
}

static int stricmp_partial(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 1;
        a++; b++;
    }
    return *a != *b;
}

int csb_v1_fmtowns_cd_probe(const uint8_t *bin_data, size_t bin_size) {
    const uint8_t *pvd;
    if (!bin_data || bin_size < (size_t)(CSB_FMTOWNS_CD_PVD_SECTOR + 1) * RAW)
        return 0;

    pvd = sector_data(bin_data, bin_size, CSB_FMTOWNS_CD_PVD_SECTOR, 1);
    if (!pvd) return 0;

    /* Check PVD signature: type 1, "CD001" */
    if (pvd[0] != 1 || memcmp(pvd + 1, "CD001", 5) != 0) return 0;

    /* Check volume ID contains "CHAOS" */
    {
        char vol[33];
        memcpy(vol, pvd + 40, 32);
        vol[32] = '\0';
        if (strstr(vol, "CHAOS") == NULL) return 0;
    }
    return 1;
}

static int parse_directory(const uint8_t *bin, size_t bin_size,
                           uint32_t dir_lba, uint32_t dir_size,
                           const char *parent_name,
                           CSB_V1_FmtownsCdLayout *layout) {
    uint32_t sectors_needed = (dir_size + COOKED - 1) / COOKED;
    uint32_t s;

    for (s = 0; s < sectors_needed && layout->file_count < CSB_FMTOWNS_CD_MAX_FILES; s++) {
        const uint8_t *sec = sector_data(bin, bin_size, dir_lba + s, layout->is_raw_2352);
        uint32_t pos = 0;
        uint32_t remaining = dir_size > (s + 1) * COOKED ?
                             COOKED : dir_size - s * COOKED;

        if (!sec) return -1;
        while (pos < remaining && pos < COOKED) {
            uint8_t rec_len = sec[pos];
            uint8_t name_len;
            uint32_t extent, size;
            uint8_t flags;
            char name[CSB_FMTOWNS_CD_MAX_NAME_LEN];
            int i;

            if (rec_len == 0) {
                pos = COOKED;
                break;
            }
            if (pos + rec_len > COOKED) break;

            name_len = sec[pos + 32];
            extent = rd32le(sec + pos + 2);
            size = rd32le(sec + pos + 10);
            flags = sec[pos + 25];

            if (name_len <= 1) {
                pos += rec_len;
                continue;
            }

            /* Copy name, strip version suffix ";1" */
            if (name_len >= CSB_FMTOWNS_CD_MAX_NAME_LEN)
                name_len = CSB_FMTOWNS_CD_MAX_NAME_LEN - 1;
            memcpy(name, sec + pos + 33, name_len);
            name[name_len] = '\0';
            for (i = 0; i < (int)name_len; i++) {
                if (name[i] == ';') { name[i] = '\0'; break; }
            }

            if (layout->file_count < CSB_FMTOWNS_CD_MAX_FILES) {
                CSB_V1_FmtownsCdFile *f = &layout->files[layout->file_count];
                memset(f, 0, sizeof(*f));
                strncpy(f->name, name, CSB_FMTOWNS_CD_MAX_NAME_LEN - 1);
                strncpy(f->parent, parent_name, CSB_FMTOWNS_CD_MAX_NAME_LEN - 1);
                f->lba = extent;
                f->size = size;
                f->is_directory = (flags & 2) ? 1 : 0;
                layout->file_count++;
            }

            /* Recurse into subdirectories */
            if ((flags & 2) && name[0] != '.') {
                if (parse_directory(bin, bin_size, extent, size,
                                   name, layout) < 0)
                    return -1;
            }

            pos += rec_len;
        }
    }
    return 0;
}

int csb_v1_fmtowns_cd_parse(const uint8_t *bin_data, size_t bin_size,
                             CSB_V1_FmtownsCdLayout *out) {
    const uint8_t *pvd;
    uint32_t root_lba, root_size;

    if (!out) return -1;
    memset(out, 0, sizeof(*out));

    if (!bin_data || bin_size < RAW * 17) return -1;

    /* Detect raw vs cooked */
    out->is_raw_2352 = 1;
    pvd = sector_data(bin_data, bin_size, CSB_FMTOWNS_CD_PVD_SECTOR, 1);
    if (!pvd || pvd[0] != 1 || memcmp(pvd + 1, "CD001", 5) != 0) {
        out->is_raw_2352 = 0;
        pvd = sector_data(bin_data, bin_size, CSB_FMTOWNS_CD_PVD_SECTOR, 0);
        if (!pvd || pvd[0] != 1 || memcmp(pvd + 1, "CD001", 5) != 0)
            return -1;
    }

    memcpy(out->volume_id, pvd + 40, 32);
    out->volume_id[32] = '\0';
    {
        int i;
        for (i = 31; i >= 0 && out->volume_id[i] == ' '; i--)
            out->volume_id[i] = '\0';
    }

    out->data_track_sectors = (uint32_t)(bin_size /
        (out->is_raw_2352 ? RAW : COOKED));

    /* Root directory record at PVD offset 156 */
    root_lba = rd32le(pvd + 156 + 2);
    root_size = rd32le(pvd + 156 + 10);

    return parse_directory(bin_data, bin_size, root_lba, root_size,
                          "", out);
}

int csb_v1_fmtowns_cd_extract(const uint8_t *bin_data, size_t bin_size,
                               const CSB_V1_FmtownsCdFile *entry,
                               uint8_t *out_buf, size_t out_buf_size) {
    size_t remaining, chunk;
    uint32_t lba;
    int is_raw;

    if (!bin_data || !entry || !out_buf || out_buf_size < entry->size)
        return -1;

    /* Detect raw vs cooked based on bin size vs sector count */
    is_raw = (bin_size > entry->lba * (size_t)RAW);

    remaining = entry->size;
    lba = entry->lba;
    while (remaining > 0) {
        const uint8_t *sec = sector_data(bin_data, bin_size, lba, is_raw);
        if (!sec) return -1;
        chunk = remaining > COOKED ? COOKED : remaining;
        memcpy(out_buf, sec, chunk);
        out_buf += chunk;
        remaining -= chunk;
        lba++;
    }
    return 0;
}

const CSB_V1_FmtownsCdFile *csb_v1_fmtowns_cd_find(
    const CSB_V1_FmtownsCdLayout *layout,
    const char *parent_dir,
    const char *filename) {
    int i;
    if (!layout || !filename) return NULL;

    for (i = 0; i < layout->file_count; i++) {
        const CSB_V1_FmtownsCdFile *f = &layout->files[i];
        if (stricmp_partial(f->name, filename) != 0) continue;
        if (parent_dir) {
            if (stricmp_partial(f->parent, parent_dir) != 0) continue;
        }
        return f;
    }
    return NULL;
}

int csb_v1_fmtowns_cdda_parse_cue(const char *cue_text, size_t cue_len,
                                   CSB_V1_FmtownsCddaLayout *out) {
    const char *p, *end;
    int track_idx = 0;

    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    if (!cue_text || cue_len == 0) return -1;

    p = cue_text;
    end = cue_text + cue_len;

    while (p < end) {
        const char *line = p;
        while (p < end && *p != '\n') p++;
        if (p < end) p++;

        /* Skip whitespace */
        while (line < p && (*line == ' ' || *line == '\t')) line++;

        if (strncmp(line, "TRACK ", 6) == 0) {
            int track_num = 0;
            const char *t = line + 6;
            while (*t >= '0' && *t <= '9') {
                track_num = track_num * 10 + (*t - '0');
                t++;
            }
            /* Only audio tracks */
            while (*t == ' ') t++;
            if (strncmp(t, "AUDIO", 5) == 0 &&
                track_idx < (int)CSB_FMTOWNS_CD_CDDA_TRACK_COUNT) {
                out->tracks[track_idx].track_number = (uint32_t)track_num;
                track_idx++;
            }
        } else if (strncmp(line, "INDEX 01 ", 9) == 0 && track_idx > 0) {
            const char *t = line + 9;
            int mm = 0, ss = 0, ff = 0;
            while (*t >= '0' && *t <= '9') { mm = mm*10 + (*t-'0'); t++; }
            if (*t == ':') t++;
            while (*t >= '0' && *t <= '9') { ss = ss*10 + (*t-'0'); t++; }
            if (*t == ':') t++;
            while (*t >= '0' && *t <= '9') { ff = ff*10 + (*t-'0'); t++; }

            {
                uint32_t sector = (uint32_t)(mm * 60 * 75 + ss * 75 + ff);
                CSB_V1_FmtownsCddaTrack *tr = &out->tracks[track_idx - 1];
                tr->start_sector = sector;
                tr->byte_offset = sector * RAW;
            }
        }
    }

    out->track_count = track_idx;

    /* Calculate sector counts from consecutive track starts */
    {
        int i;
        for (i = 0; i < track_idx - 1; i++) {
            out->tracks[i].sector_count =
                out->tracks[i + 1].start_sector - out->tracks[i].start_sector;
            out->tracks[i].byte_length = out->tracks[i].sector_count * RAW;
        }
    }
    out->valid = (track_idx >= 20) ? 1 : 0;
    return 0;
}

int csb_v1_fmtowns_cdda_extract(const uint8_t *bin_data, size_t bin_size,
                                 const CSB_V1_FmtownsCddaTrack *track,
                                 uint8_t *out_buf, size_t out_buf_size) {
    size_t start, length;

    if (!bin_data || !track || !out_buf) return -1;

    start = (size_t)track->start_sector * RAW;
    length = (size_t)track->sector_count * RAW;

    if (length == 0 || start + length > bin_size ||
        length > out_buf_size) return -1;

    memcpy(out_buf, bin_data + start, length);
    return (int)length;
}
