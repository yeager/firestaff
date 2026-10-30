#include "firestaff_x68k_hdm.h"

#include <string.h>

/*
 * ReDMCSB X30J source confirms the game-side file contract:
 * Toolchains/Common/Source/PRIM1.C:132 opens A:\\FTLCODE;
 * MEMORY.C:1259 opens A:\\GRAPHICS.DAT; LOADSAVE.C:2339 opens
 * \\DUNGEON.DAT. The fixed 2DHD geometry and root/FAT offsets below were
 * derived from the operator's authentic cracked HDM and checked by matching
 * the extracted SWOOSH bytes to Reference/Original/X30J/SWOOSH.
 */
enum {
    X68K_HDM_FAT_OFFSET = 0x0400,
    X68K_HDM_FAT_BYTES = 0x1000,
    X68K_HDM_ROOT_OFFSET = 0x1400,
    X68K_HDM_ROOT_BYTES = 0x1800,
    X68K_HDM_DATA_OFFSET = 0x2c00,
    X68K_HDM_ROOT_SLOTS = X68K_HDM_ROOT_BYTES /
                          FIRESTAFF_X68K_HDM_ROOT_ENTRY_BYTES,
    X68K_HDM_CLUSTER_COUNT =
        (FIRESTAFF_X68K_HDM_IMAGE_BYTES - X68K_HDM_DATA_OFFSET) /
        FIRESTAFF_X68K_HDM_CLUSTER_BYTES,
    X68K_HDM_ATTRIBUTE_DIRECTORY = 0x10,
    X68K_HDM_ATTRIBUTE_VOLUME = 0x08,
    X68K_HDM_ATTRIBUTE_LONG_NAME = 0x0f,
    X68K_HDM_FAT12_EOC_MIN = 0x0ff8,
    X68K_HDM_FAT12_BAD = 0x0ff7,
    X68K_HDM_FAT12_RESERVED_MIN = 0x0ff0
};

static uint16_t x68k_hdm_read_le16(const uint8_t *p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t x68k_hdm_read_le32(const uint8_t *p) {
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static int x68k_hdm_fat12_next(const FirestaffX68kHdm *hdm,
                               uint16_t cluster,
                               uint16_t *out_next) {
    const size_t pair_offset = (size_t)cluster + (size_t)(cluster / 2u);
    uint16_t packed;
    if (!hdm || !hdm->image || !out_next ||
        pair_offset + 1u >= X68K_HDM_FAT_BYTES) {
        return -1;
    }
    packed = (uint16_t)((uint16_t)hdm->image[X68K_HDM_FAT_OFFSET + pair_offset] |
                        ((uint16_t)hdm->image[X68K_HDM_FAT_OFFSET + pair_offset + 1u] << 8));
    if ((cluster & 1u) != 0u) {
        packed >>= 4;
    }
    *out_next = (uint16_t)(packed & 0x0fffu);
    return 0;
}

static int x68k_hdm_append_name_part(char *out,
                                     size_t out_capacity,
                                     size_t *used,
                                     const uint8_t *part,
                                     size_t part_capacity) {
    size_t n = part_capacity;
    size_t i;
    while (n > 0u && part[n - 1u] == (uint8_t)' ') --n;
    if (*used + n >= out_capacity) return -1;
    for (i = 0u; i < n; ++i) {
        const uint8_t ch = part[i];
        if (ch < 0x21u || ch > 0x7eu || ch == '/' || ch == '\\') return -1;
        out[(*used)++] = (char)ch;
    }
    out[*used] = '\0';
    return 0;
}

static int x68k_hdm_entry_name(const uint8_t *raw, char out_name[13]) {
    size_t used = 0u;
    out_name[0] = '\0';
    if (x68k_hdm_append_name_part(out_name, 13u, &used, raw, 8u) != 0) {
        return -1;
    }
    if (raw[8] != (uint8_t)' ') {
        if (used + 1u >= 13u) return -1;
        out_name[used++] = '.';
        out_name[used] = '\0';
        if (x68k_hdm_append_name_part(out_name, 13u, &used, raw + 8u, 3u) != 0) {
            return -1;
        }
    }
    return used == 0u ? -1 : 0;
}

int FirestaffX68kHdm_Open(const uint8_t *image,
                          size_t image_size,
                          FirestaffX68kHdm *out) {
    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    if (!image || image_size != FIRESTAFF_X68K_HDM_IMAGE_BYTES ||
        image[X68K_HDM_FAT_OFFSET] != 0xfeu ||
        image[X68K_HDM_FAT_OFFSET + 1u] != 0xffu ||
        image[X68K_HDM_FAT_OFFSET + 2u] != 0xffu) {
        return -1;
    }
    out->image = image;
    out->image_size = image_size;
    return 0;
}

int FirestaffX68kHdm_NextRootEntry(const FirestaffX68kHdm *hdm,
                                   uint16_t *cursor,
                                   FirestaffX68kHdmRootEntry *out) {
    if (!hdm || !hdm->image || !cursor || !out ||
        hdm->image_size != FIRESTAFF_X68K_HDM_IMAGE_BYTES ||
        *cursor > X68K_HDM_ROOT_SLOTS) {
        return -1;
    }
    while (*cursor < X68K_HDM_ROOT_SLOTS) {
        const uint16_t slot = *cursor;
        const uint8_t *raw = hdm->image + X68K_HDM_ROOT_OFFSET +
                             (size_t)slot * FIRESTAFF_X68K_HDM_ROOT_ENTRY_BYTES;
        const uint8_t first = raw[0];
        const uint8_t attributes = raw[11];
        ++*cursor;
        if (first == 0u) return 1;
        if (first == 0xe5u || attributes == X68K_HDM_ATTRIBUTE_LONG_NAME ||
            (attributes & X68K_HDM_ATTRIBUTE_VOLUME) != 0u) {
            continue;
        }
        memset(out, 0, sizeof(*out));
        if (x68k_hdm_entry_name(raw, out->name) != 0) return -1;
        out->attributes = attributes;
        out->first_cluster = x68k_hdm_read_le16(raw + 26u);
        out->size = x68k_hdm_read_le32(raw + 28u);
        out->directory_slot = slot;
        return 0;
    }
    return 1;
}

int FirestaffX68kHdm_ReadRootFile(const FirestaffX68kHdm *hdm,
                                  const FirestaffX68kHdmRootEntry *entry,
                                  uint8_t *out,
                                  size_t out_capacity) {
    uint16_t cluster;
    size_t required_clusters;
    size_t steps = 0u;
    if (!hdm || !hdm->image || !entry ||
        hdm->image_size != FIRESTAFF_X68K_HDM_IMAGE_BYTES ||
        (entry->attributes & X68K_HDM_ATTRIBUTE_DIRECTORY) != 0u ||
        entry->size > out_capacity || (entry->size != 0u && !out)) {
        return -1;
    }
    if (entry->size == 0u) return 0;
    required_clusters = ((size_t)entry->size +
                         FIRESTAFF_X68K_HDM_CLUSTER_BYTES - 1u) /
                        FIRESTAFF_X68K_HDM_CLUSTER_BYTES;
    if (required_clusters == 0u || required_clusters > X68K_HDM_CLUSTER_COUNT) {
        return -1;
    }

    /* Validate the whole FAT chain before exposing any file bytes. */
    cluster = entry->first_cluster;
    while (steps < required_clusters) {
        uint16_t next_cluster;
        if (++steps > required_clusters || cluster < 2u ||
            (size_t)(cluster - 2u) >= X68K_HDM_CLUSTER_COUNT) {
            return -1;
        }
        if (x68k_hdm_fat12_next(hdm, cluster, &next_cluster) != 0) return -1;
        if (steps == required_clusters) {
            if (next_cluster < X68K_HDM_FAT12_EOC_MIN) return -1;
        } else if (next_cluster < 2u ||
                   next_cluster >= X68K_HDM_FAT12_RESERVED_MIN ||
                   next_cluster == X68K_HDM_FAT12_BAD) {
            return -1;
        } else {
            cluster = next_cluster;
        }
    }

    cluster = entry->first_cluster;
    steps = 0u;
    {
        size_t copied = 0u;
        while (copied < entry->size) {
            uint16_t next_cluster;
            size_t data_offset;
            size_t chunk = (size_t)entry->size - copied;
            if (++steps > required_clusters || cluster < 2u ||
                (size_t)(cluster - 2u) >= X68K_HDM_CLUSTER_COUNT) {
                return -1;
            }
            data_offset = X68K_HDM_DATA_OFFSET +
                          (size_t)(cluster - 2u) * FIRESTAFF_X68K_HDM_CLUSTER_BYTES;
            if (data_offset > hdm->image_size ||
                FIRESTAFF_X68K_HDM_CLUSTER_BYTES > hdm->image_size - data_offset) {
                return -1;
            }
            if (chunk > FIRESTAFF_X68K_HDM_CLUSTER_BYTES) {
                chunk = FIRESTAFF_X68K_HDM_CLUSTER_BYTES;
            }
            memcpy(out + copied, hdm->image + data_offset, chunk);
            copied += chunk;
            if (copied < entry->size) {
                if (x68k_hdm_fat12_next(hdm, cluster, &next_cluster) != 0) return -1;
                cluster = next_cluster;
            }
        }
    }
    return 0;
}
