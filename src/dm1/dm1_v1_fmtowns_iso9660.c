#include "dm1_v1_fmtowns_iso9660.h"
#include <stdio.h>
#include <string.h>

static uint16_t rd16le(const uint8_t *p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t rd32le(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

int dm1_v1_fmtowns_iso_probe(const uint8_t *data_track, size_t size) {
    if (!data_track) return 0;
    size_t pvd_offset = (size_t)DM1_FMTOWNS_ISO_PVD_SECTOR * DM1_FMTOWNS_ISO_SECTOR_SIZE;
    if (size < pvd_offset + 256) return 0;

    const uint8_t *pvd = data_track + pvd_offset;
    if (pvd[0] != 0x01) return 0;
    if (memcmp(pvd + 1, "CD001", 5) != 0) return 0;

    /* System ID at PVD offset 8, 32 bytes, space-padded. Check for "HMA-240". */
    if (memcmp(pvd + 8, "HMA-240", 7) != 0) return 0;

    return 1;
}

static int parse_directory(const uint8_t *data_track, size_t track_size,
                           uint32_t dir_lba, uint32_t dir_size,
                           const char *parent_prefix,
                           DM1_V1_FmtownsIsoLayout *out) {
    size_t dir_offset = (size_t)dir_lba * DM1_FMTOWNS_ISO_SECTOR_SIZE;
    if (dir_offset + dir_size > track_size) return -1;

    const uint8_t *dir = data_track + dir_offset;
    size_t pos = 0;

    while (pos < dir_size && out->file_count < DM1_FMTOWNS_ISO_MAX_FILES) {
        uint8_t rec_len = dir[pos];
        if (rec_len == 0) {
            size_t next_sector = ((pos / DM1_FMTOWNS_ISO_SECTOR_SIZE) + 1) *
                                 DM1_FMTOWNS_ISO_SECTOR_SIZE;
            if (next_sector >= dir_size) break;
            pos = next_sector;
            continue;
        }
        if (pos + rec_len > dir_size) break;

        uint8_t name_len = dir[pos + 32];
        uint32_t extent = rd32le(dir + pos + 2);
        uint32_t fsize = rd32le(dir + pos + 10);
        uint8_t flags = dir[pos + 25];
        int is_dir = (flags & 0x02) != 0;

        if (name_len == 1 && (dir[pos + 33] == 0x00 || dir[pos + 33] == 0x01)) {
            pos += rec_len;
            continue;
        }

        char raw_name[DM1_FMTOWNS_ISO_MAX_NAME_LEN];
        size_t copy_len = name_len;
        if (copy_len >= sizeof(raw_name)) copy_len = sizeof(raw_name) - 1;
        memcpy(raw_name, dir + pos + 33, copy_len);
        raw_name[copy_len] = '\0';

        /* Strip ";1" version suffix */
        char *semi = strchr(raw_name, ';');
        if (semi) *semi = '\0';

        if (is_dir) {
            char prefix[DM1_FMTOWNS_ISO_MAX_NAME_LEN];
            int n = snprintf(prefix, sizeof(prefix), "%s%s/", parent_prefix, raw_name);
            if (n > 0 && (size_t)n < sizeof(prefix)) {
                parse_directory(data_track, track_size, extent, fsize,
                                prefix, out);
            }
        } else {
            /* The original FM Towns presentation is owned by the root
             * AUTOEXEC/TMENU/EDM/JDM program family.  Retain its exact CD
             * records with the language data pair; ReDMCSB's PC TITLE.C
             * route is not a compatible substitute.  The names and byte
             * sizes come from the locally verified HMA-240 retail disc. */
            if (strstr(raw_name, "GRAPHICS.DAT") != NULL ||
                strstr(raw_name, "DUNGEON.DAT") != NULL ||
                strcmp(raw_name, "AUTOEXEC.BAT") == 0 ||
                strcmp(raw_name, "EDM.EXP") == 0 ||
                strcmp(raw_name, "JDM.EXP") == 0 ||
                strcmp(raw_name, "TMENU.EXP") == 0 ||
                strcmp(raw_name, "TMENU.ICN") == 0 ||
                strcmp(raw_name, "TMENU.INF") == 0) {
                DM1_V1_FmtownsIsoEntry *e = &out->files[out->file_count];
                snprintf(e->name, sizeof(e->name), "%s%s", parent_prefix, raw_name);
                e->lba = extent;
                e->size = fsize;
                e->is_directory = 0;
                out->file_count++;
            }
        }

        pos += rec_len;
    }

    return 0;
}

int dm1_v1_fmtowns_iso_parse(const uint8_t *data_track, size_t size,
                              DM1_V1_FmtownsIsoLayout *out) {
    if (!data_track || !out) return -1;
    memset(out, 0, sizeof(*out));

    if (!dm1_v1_fmtowns_iso_probe(data_track, size)) return -1;

    size_t pvd_offset = (size_t)DM1_FMTOWNS_ISO_PVD_SECTOR * DM1_FMTOWNS_ISO_SECTOR_SIZE;
    const uint8_t *pvd = data_track + pvd_offset;

    /* System ID (PVD offset 8, 32 bytes) */
    memcpy(out->system_id, pvd + 8, 32);
    out->system_id[32] = '\0';
    /* Trim trailing spaces */
    for (int i = 31; i >= 0 && out->system_id[i] == ' '; i--)
        out->system_id[i] = '\0';

    /* Volume ID (PVD offset 40, 32 bytes) */
    memcpy(out->volume_id, pvd + 40, 32);
    out->volume_id[32] = '\0';
    for (int i = 31; i >= 0 && out->volume_id[i] == ' '; i--)
        out->volume_id[i] = '\0';

    /* Volume space size (PVD offset 80, LSB-first of both-endian pair) */
    out->volume_space_size = rd32le(pvd + 80);

    /* Root directory record (PVD offset 156, 34 bytes) */
    const uint8_t *root_rec = pvd + 156;
    out->root_dir_lba = rd32le(root_rec + 2);
    out->root_dir_size = rd32le(root_rec + 10);

    return parse_directory(data_track, size, out->root_dir_lba,
                           out->root_dir_size, "", out);
}

int dm1_v1_fmtowns_iso_extract(const uint8_t *data_track, size_t track_size,
                                const DM1_V1_FmtownsIsoEntry *entry,
                                uint8_t *out_buf, size_t out_buf_size) {
    if (!data_track || !entry || !out_buf) return -1;
    if (out_buf_size < entry->size) return -1;

    size_t offset = (size_t)entry->lba * DM1_FMTOWNS_ISO_SECTOR_SIZE;
    if (offset + entry->size > track_size) return -1;

    memcpy(out_buf, data_track + offset, entry->size);
    return 0;
}
