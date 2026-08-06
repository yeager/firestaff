#include "dm2_v1_fmtowns_disc.h"
#include <stdlib.h>
#include <string.h>

static const uint8_t *sector_data(const uint8_t *image, uint32_t lba) {
    return image + (size_t)lba * DM2_FMTOWNS_SECTOR_SIZE
                 + DM2_FMTOWNS_SECTOR_HEADER_SIZE;
}

static uint32_t rd32le(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void copy_strA(char *dst, const uint8_t *src, int len) {
    int i;
    memcpy(dst, src, (size_t)len);
    dst[len] = '\0';
    for (i = len - 1; i >= 0 && dst[i] == ' '; i--)
        dst[i] = '\0';
}

static int scan_directory(const uint8_t *image, size_t image_size,
                          uint32_t dir_lba, uint32_t dir_size,
                          DM2_V1_FmtownsDiscReceipt *out) {
    uint8_t dirbuf[2048];
    uint32_t sectors_read = 0;
    uint32_t pos = 0;

    if ((uint64_t)dir_lba * DM2_FMTOWNS_SECTOR_SIZE +
        DM2_FMTOWNS_SECTOR_SIZE > image_size)
        return -1;

    while (sectors_read * DM2_FMTOWNS_SECTOR_DATA_SIZE < dir_size &&
           sectors_read < 8) {
        uint32_t lba = dir_lba + sectors_read;
        if ((uint64_t)lba * DM2_FMTOWNS_SECTOR_SIZE +
            DM2_FMTOWNS_SECTOR_SIZE > image_size)
            break;
        memcpy(dirbuf, sector_data(image, lba),
               DM2_FMTOWNS_SECTOR_DATA_SIZE);
        sectors_read++;

        pos = 0;
        while (pos < DM2_FMTOWNS_SECTOR_DATA_SIZE) {
            uint8_t rec_len = dirbuf[pos];
            uint8_t name_len;
            char name[64];
            uint32_t file_lba, file_size;
            uint8_t flags;
            size_t bare_len;

            if (rec_len == 0) break;
            if (pos + rec_len > DM2_FMTOWNS_SECTOR_DATA_SIZE) break;

            name_len = dirbuf[pos + 32];
            if (name_len < 1 || name_len > 63) { pos += rec_len; continue; }

            memcpy(name, dirbuf + pos + 33, name_len);
            name[name_len] = '\0';

            file_lba  = rd32le(dirbuf + pos + 2);
            file_size = rd32le(dirbuf + pos + 10);
            flags     = dirbuf[pos + 25];

            /* Strip ";1" version suffix */
            bare_len = strlen(name);
            if (bare_len > 2 && name[bare_len - 2] == ';')
                name[bare_len - 2] = '\0';

            if (flags & 2) {
                /* Directory — recurse into DATA/ */
                if ((strcmp(name, "DATA") == 0 || strcmp(name, "data") == 0) &&
                    file_lba != dir_lba) {
                    scan_directory(image, image_size, file_lba, file_size, out);
                }
            } else {
                DM2_V1_FmtownsIsoEntry ent;
                memset(&ent, 0, sizeof(ent));
                strncpy(ent.name, name, sizeof(ent.name) - 1);
                ent.lba  = file_lba;
                ent.size = file_size;
                ent.is_directory = 0;

                if (strcmp(name, "CD.DAT") == 0 ||
                    strcmp(name, "cd.dat") == 0) {
                    out->cd_dat = ent;
                    out->has_cd_dat = 1;
                } else if (strcmp(name, "DUNGEON.DAT") == 0 ||
                           strcmp(name, "dungeon.dat") == 0) {
                    out->dungeon_dat = ent;
                    out->has_dungeon_dat = 1;
                } else if (strcmp(name, "GRAPHICS.DAT") == 0 ||
                           strcmp(name, "graphics.dat") == 0) {
                    out->graphics_dat = ent;
                    out->has_graphics_dat = 1;
                } else if (strcmp(name, "AUTOEXEC.BAT") == 0) {
                    out->autoexec_bat = ent;
                    out->has_autoexec_bat = 1;
                } else if (strcmp(name, "SWOOSH") == 0) {
                    out->swoosh = ent;
                    out->has_swoosh = 1;
                } else if (strcmp(name, "TITLE") == 0) {
                    out->title = ent;
                    out->has_title = 1;
                } else if (strcmp(name, "TWANIM.EXP") == 0) {
                    out->twanim_exp = ent;
                    out->has_twanim_exp = 1;
                } else if (strcmp(name, "SKULL.EXP") == 0) {
                    out->skull_exp = ent;
                    out->has_skull_exp = 1;
                } else if (strcmp(name, "END") == 0) {
                    out->end = ent;
                    out->has_end = 1;
                }
            }
            pos += rec_len;
        }
    }
    return 0;
}

int dm2_v1_fmtowns_disc_probe(const uint8_t *image, size_t image_size,
                              DM2_V1_FmtownsDiscReceipt *out) {
    const uint8_t *pvd;
    const uint8_t *root_rec;
    uint32_t root_lba, root_size;

    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    if (!image) return -1;

    /* Need at least sector 16 (PVD) + 1 data sector */
    if (image_size < 17u * DM2_FMTOWNS_SECTOR_SIZE) return -1;

    /* Primary Volume Descriptor at sector 16 */
    pvd = sector_data(image, 16);
    if (pvd[0] != 0x01 || memcmp(pvd + 1, "CD001", 5) != 0)
        return -1;

    copy_strA(out->system_id, pvd + 8, 32);
    copy_strA(out->volume_id, pvd + 40, 32);

    out->data_track_sectors = (uint32_t)(image_size / DM2_FMTOWNS_SECTOR_SIZE);

    /* Root directory record at PVD offset 156 */
    root_rec = pvd + 156;
    root_lba  = rd32le(root_rec + 2);
    root_size = rd32le(root_rec + 10);

    if (scan_directory(image, image_size, root_lba, root_size, out) != 0)
        return -1;

    out->valid = (out->has_graphics_dat && out->has_dungeon_dat);
    out->startup_media_complete =
        out->has_autoexec_bat && out->has_swoosh && out->has_title &&
        out->has_twanim_exp && out->has_skull_exp && out->has_end;
    return out->valid ? 0 : -1;
}

int dm2_v1_fmtowns_disc_startup_plan(
    const uint8_t *image, size_t image_size,
    const DM2_V1_FmtownsDiscReceipt *receipt,
    DM2_V1_FmtownsStartupPlan *out) {
    uint8_t *script = NULL;
    size_t script_size = 0u;
    const char *swoosh;
    const char *title;
    const char *skull;
    const char *end;

    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    if (!image || !receipt || !receipt->startup_media_complete ||
        dm2_v1_fmtowns_disc_extract_alloc(image, image_size,
                                           &receipt->autoexec_bat,
                                           &script, &script_size) != 0 ||
        !script || script_size == 0u) {
        free(script);
        return -1;
    }
    /* The retail script is ASCII and ends well inside this bounded ISO file.
     * A private terminator lets strstr operate without reading the disc arena
     * beyond AUTOEXEC.BAT. */
    {
        uint8_t *terminated_script = realloc(script, script_size + 1u);
        if (!terminated_script) {
            free(script);
            return -1;
        }
        script = terminated_script;
    }
    script[script_size] = '\0';
    swoosh = strstr((const char *)script, "\\RUN386 TWANIM SWOOSH +AF +AH +AR");
    title = strstr((const char *)script, "\\RUN386 TWANIM TITLE +AB +AE +AR");
    skull = strstr((const char *)script, "\\RUN386 SKULL");
    end = strstr((const char *)script, "\\RUN386 TWANIM END +AC7 +AR");
    if (!swoosh || !title || !skull || !end ||
        !(swoosh < title && title < skull && skull < end)) {
        free(script);
        return -1;
    }
    out->valid = 1;
    out->stage_count = 4;
    out->stages[0] = DM2_FMTOWNS_STARTUP_STAGE_SWOOSH;
    out->stages[1] = DM2_FMTOWNS_STARTUP_STAGE_TITLE;
    out->stages[2] = DM2_FMTOWNS_STARTUP_STAGE_SKULL;
    out->stages[3] = DM2_FMTOWNS_STARTUP_STAGE_END;
    free(script);
    return 0;
}

int dm2_v1_fmtowns_disc_extract(const uint8_t *image, size_t image_size,
                                const DM2_V1_FmtownsIsoEntry *entry,
                                uint8_t *buf, size_t buf_size) {
    uint32_t remaining, lba;

    if (!image || !entry || !buf) return -1;
    if (buf_size < entry->size) return -1;

    remaining = entry->size;
    lba = entry->lba;

    while (remaining > 0) {
        uint32_t chunk;
        if ((uint64_t)lba * DM2_FMTOWNS_SECTOR_SIZE +
            DM2_FMTOWNS_SECTOR_SIZE > image_size)
            return -1;
        chunk = remaining > DM2_FMTOWNS_SECTOR_DATA_SIZE
              ? DM2_FMTOWNS_SECTOR_DATA_SIZE : remaining;
        memcpy(buf, sector_data(image, lba), chunk);
        buf += chunk;
        remaining -= chunk;
        lba++;
    }
    return 0;
}

int dm2_v1_fmtowns_disc_extract_alloc(const uint8_t *image, size_t image_size,
                                      const DM2_V1_FmtownsIsoEntry *entry,
                                      uint8_t **out_data, size_t *out_size) {
    uint8_t *buf;

    if (!out_data || !out_size) return -1;
    *out_data = NULL;
    *out_size = 0;
    if (!image || !entry || entry->size == 0) return -1;

    buf = malloc(entry->size);
    if (!buf) return -1;

    if (dm2_v1_fmtowns_disc_extract(image, image_size, entry,
                                    buf, entry->size) != 0) {
        free(buf);
        return -1;
    }

    *out_data = buf;
    *out_size = entry->size;
    return 0;
}

int dm2_v1_fmtowns_cdda_track_info(int track_num,
                                   const uint32_t *track_start_sectors,
                                   int track_count,
                                   uint32_t total_disc_sectors,
                                   DM2_V1_FmtownsCddaTrackInfo *out) {
    int idx;
    uint32_t start, end;

    if (!out || !track_start_sectors) return -1;
    if (track_num < (int)DM2_FMTOWNS_CDDA_FIRST_TRACK ||
        track_num > (int)DM2_FMTOWNS_CDDA_LAST_TRACK)
        return -1;

    idx = track_num - 1;
    if (idx >= track_count) return -1;

    start = track_start_sectors[idx];
    end = (idx + 1 < track_count) ? track_start_sectors[idx + 1]
                                  : total_disc_sectors;

    out->disc_track   = (uint8_t)track_num;
    out->start_sector = start;
    out->sector_count = end - start;
    out->pcm_bytes    = out->sector_count * DM2_FMTOWNS_SECTOR_SIZE;
    out->duration_sec = (float)out->pcm_bytes / (float)DM2_FMTOWNS_CDDA_BYTES_PER_SEC;
    return 0;
}

int dm2_v1_fmtowns_cdda_extract(const uint8_t *image, size_t image_size,
                                uint32_t start_sector, uint32_t sector_count,
                                uint8_t **out_pcm, size_t *out_pcm_size) {
    size_t byte_offset, byte_count;
    uint8_t *buf;

    if (!out_pcm || !out_pcm_size) return -1;
    *out_pcm = NULL;
    *out_pcm_size = 0;
    if (!image || sector_count == 0) return -1;

    byte_offset = (size_t)start_sector * DM2_FMTOWNS_SECTOR_SIZE;
    byte_count  = (size_t)sector_count * DM2_FMTOWNS_SECTOR_SIZE;
    if (byte_offset + byte_count > image_size) return -1;

    buf = malloc(byte_count);
    if (!buf) return -1;

    memcpy(buf, image + byte_offset, byte_count);

    *out_pcm = buf;
    *out_pcm_size = byte_count;
    return 0;
}
