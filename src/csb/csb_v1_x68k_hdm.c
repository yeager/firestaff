#include "csb_v1_x68k_hdm.h"

#include <ctype.h>
#include <string.h>

enum {
    X68K_RESERVED_SECTORS = 1,
    X68K_FAT_COUNT = 2,
    X68K_SECTORS_PER_FAT = 2,
    X68K_ROOT_ENTRIES = 192,
    X68K_ROOT_SECTORS = 6,
    X68K_SECTORS_PER_CLUSTER = 1,
    X68K_FAT12_EOC = 0xff8
};

static uint16_t le16(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t le32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static int name83(const char *name, uint8_t out[11]) {
    const char *dot;
    size_t i, base = 0u, ext = 0u;

    if (!name || !name[0]) return 0;
    memset(out, ' ', 11u);
    dot = strrchr(name, '.');
    if (dot) {
        if (dot == name || !dot[1]) return 0;
        for (i = 0u; name + i < dot; ++i) {
            if (i >= 8u || !(isalnum((unsigned char)name[i]) ||
                            name[i] == '_' || name[i] == '-')) return 0;
            out[base++] = (uint8_t)toupper((unsigned char)name[i]);
        }
        for (i = 0u; dot[1u + i]; ++i) {
            if (i >= 3u || !(isalnum((unsigned char)dot[1u + i]) ||
                            dot[1u + i] == '_' || dot[1u + i] == '-')) return 0;
            out[8u + ext++] = (uint8_t)toupper((unsigned char)dot[1u + i]);
        }
    } else {
        for (i = 0u; name[i]; ++i) {
            if (i >= 8u || !(isalnum((unsigned char)name[i]) ||
                            name[i] == '_' || name[i] == '-')) return 0;
            out[base++] = (uint8_t)toupper((unsigned char)name[i]);
        }
    }
    return base && (!dot || ext);
}

static int layout(const uint8_t *hdm, size_t hdm_size, size_t *fat_offset,
                  size_t *root_offset, size_t *data_offset) {
    const size_t fat = X68K_RESERVED_SECTORS * CSB_V1_X68K_HDM_BYTES_PER_SECTOR;
    const size_t root = fat + X68K_FAT_COUNT * X68K_SECTORS_PER_FAT *
        CSB_V1_X68K_HDM_BYTES_PER_SECTOR;
    const size_t data = root + X68K_ROOT_SECTORS * CSB_V1_X68K_HDM_BYTES_PER_SECTOR;

    if (!hdm || hdm_size != CSB_V1_X68K_HDM_BYTES_PER_DISK ||
        data > hdm_size || hdm[0] != 0x60u || hdm[1] != 0x1cu ||
        memcmp(hdm + 2u, "Hudson soft 2.00", 16u) != 0 ||
        hdm[fat] != 0xfeu || hdm[fat + 1u] != 0xffu || hdm[fat + 2u] != 0xffu)
        return 0;
    *fat_offset = fat; *root_offset = root; *data_offset = data;
    return 1;
}

static uint16_t fat12_next(const uint8_t *fat, size_t fat_bytes,
                           uint16_t cluster) {
    const size_t at = (size_t)cluster + (cluster >> 1);
    uint16_t packed;
    if (at + 1u >= fat_bytes) return 0u;
    packed = (uint16_t)fat[at] | ((uint16_t)fat[at + 1u] << 8);
    return (cluster & 1u) ? (uint16_t)(packed >> 4) :
        (uint16_t)(packed & 0x0fffu);
}

int csb_v1_x68k_hdm_probe(const uint8_t *hdm, size_t hdm_size,
                          CSB_V1_X68kHdmReceipt *out_receipt) {
    size_t fat_offset, root_offset, data_offset, i;
    CSB_V1_X68kHdmReceipt receipt;

    if (!layout(hdm, hdm_size, &fat_offset, &root_offset, &data_offset)) return 0;
    (void)fat_offset;
    memset(&receipt, 0, sizeof(receipt));
    receipt.sectors_per_cluster = X68K_SECTORS_PER_CLUSTER;
    receipt.fat_count = X68K_FAT_COUNT;
    receipt.sectors_per_fat = X68K_SECTORS_PER_FAT;
    receipt.root_entry_count = X68K_ROOT_ENTRIES;
    receipt.data_offset = (uint32_t)data_offset;
    for (i = 0u; i < X68K_ROOT_ENTRIES; ++i) {
        const uint8_t *entry = hdm + root_offset + i * 32u;
        if (entry[0] == 0x00u) break;
        if (entry[0] != 0xe5u && (entry[11] & 0x18u) == 0u) ++receipt.root_file_count;
    }
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int csb_v1_x68k_hdm_extract_root_file(const uint8_t *hdm, size_t hdm_size,
                                      const char *name, uint8_t *out_bytes,
                                      size_t out_capacity, size_t *out_size,
                                      CSB_V1_X68kHdmReceipt *out_receipt) {
    uint8_t wanted[11];
    const uint8_t *entry = NULL, *fat;
    size_t fat_offset, root_offset, data_offset, i, copied = 0u;
    uint16_t cluster;
    uint32_t file_size;
    uint8_t seen[CSB_V1_X68K_HDM_BYTES_PER_DISK /
                 CSB_V1_X68K_HDM_BYTES_PER_SECTOR];

    if (out_size) *out_size = 0u;
    if (!out_size || !name83(name, wanted) ||
        !layout(hdm, hdm_size, &fat_offset, &root_offset, &data_offset)) return 0;
    for (i = 0u; i < X68K_ROOT_ENTRIES; ++i) {
        const uint8_t *candidate = hdm + root_offset + i * 32u;
        if (candidate[0] == 0x00u) break;
        if (candidate[0] != 0xe5u && (candidate[11] & 0x18u) == 0u &&
            memcmp(candidate, wanted, 11u) == 0) {
            entry = candidate;
            break;
        }
    }
    if (!entry) return 0;
    cluster = le16(entry + 26u); file_size = le32(entry + 28u);
    if (!file_size || cluster < 2u || file_size > hdm_size ||
        (out_bytes && out_capacity < file_size)) return 0;
    fat = hdm + fat_offset;
    memset(seen, 0, sizeof(seen));
    while (copied < file_size) {
        size_t offset, take = (size_t)file_size - copied;
        if (cluster < 2u || cluster >= X68K_FAT12_EOC ||
            cluster >= sizeof(seen) || seen[cluster]) return 0;
        seen[cluster] = 1u;
        offset = data_offset + (size_t)(cluster - 2u) * CSB_V1_X68K_HDM_BYTES_PER_SECTOR;
        if (offset > hdm_size || CSB_V1_X68K_HDM_BYTES_PER_SECTOR > hdm_size - offset)
            return 0;
        if (take > CSB_V1_X68K_HDM_BYTES_PER_SECTOR)
            take = CSB_V1_X68K_HDM_BYTES_PER_SECTOR;
        if (out_bytes) memcpy(out_bytes + copied, hdm + offset, take);
        copied += take;
        if (copied == file_size) break;
        cluster = fat12_next(fat, X68K_SECTORS_PER_FAT *
                             CSB_V1_X68K_HDM_BYTES_PER_SECTOR, cluster);
    }
    *out_size = file_size;
    return csb_v1_x68k_hdm_probe(hdm, hdm_size, out_receipt);
}

int csb_v1_x68k_hdm_root_entry(const uint8_t *hdm, size_t hdm_size,
                               uint16_t entry_index,
                               CSB_V1_X68kHdmRootEntry *out_entry) {
    size_t fat_offset, root_offset, data_offset, i;
    uint16_t found = 0u;
    if (!out_entry || !layout(hdm, hdm_size, &fat_offset, &root_offset,
                              &data_offset)) return 0;
    (void)fat_offset; (void)data_offset;
    memset(out_entry, 0, sizeof(*out_entry));
    for (i = 0u; i < X68K_ROOT_ENTRIES; ++i) {
        const uint8_t *entry = hdm + root_offset + i * 32u;
        size_t base = 8u, ext = 3u;
        if (entry[0] == 0x00u) break;
        if (entry[0] == 0xe5u || (entry[11] & 0x18u) != 0u) continue;
        if (found++ != entry_index) continue;
        while (base && entry[base - 1u] == ' ') --base;
        while (ext && entry[8u + ext - 1u] == ' ') --ext;
        if (!base) return 0;
        memcpy(out_entry->name, entry, base);
        if (ext) {
            out_entry->name[base++] = '.';
            memcpy(out_entry->name + base, entry + 8u, ext);
            base += ext;
        }
        out_entry->name[base] = '\0';
        out_entry->attributes = entry[11];
        out_entry->first_cluster = le16(entry + 26u);
        out_entry->byte_count = le32(entry + 28u);
        return 1;
    }
    return 0;
}
