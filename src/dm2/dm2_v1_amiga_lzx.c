/*
 * In-memory index reader for the original DM2 Amiga installer archive.
 *
 * The supplied Disk 1 "DM2 Install" script concatenates dm2_arcsplit1..6
 * into DM2_archive.LZX and invokes its bundled unlzx.  DMWeb documents the
 * Amiga edition as a hard-disk installation.  Firestaff must retain that
 * boundary in memory: this module never writes, extracts or publishes data.
 */

#include "dm2_v1_amiga_lzx.h"

#include <stdlib.h>
#include <string.h>

enum {
    DM2_V1_AMIGA_LZX_HEADER_SIZE = 10u,
    DM2_V1_AMIGA_LZX_ENTRY_FIXED_SIZE = 31u,
    DM2_V1_AMIGA_LZX_METHOD_STORE = 0u,
    DM2_V1_AMIGA_LZX_METHOD_LZX = 2u,
    DM2_V1_AMIGA_LZX_MAX_SIZE = 16u * 1024u * 1024u
};

static uint16_t lzx_le16(const uint8_t *p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t lzx_le32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

int dm2_v1_amiga_lzx_join_parts(
    const DM2_V1_AmigaLzxPart parts[DM2_V1_AMIGA_LZX_PART_COUNT],
    uint8_t **out_bytes, size_t *out_size) {
    size_t total = 0u;
    size_t offset = 0u;
    unsigned int i;
    uint8_t *joined;
    if (!parts || !out_bytes || !out_size) return 0;
    *out_bytes = NULL;
    *out_size = 0u;
    for (i = 0u; i < DM2_V1_AMIGA_LZX_PART_COUNT; ++i) {
        if (!parts[i].bytes || parts[i].size == 0u ||
            parts[i].size > DM2_V1_AMIGA_LZX_MAX_SIZE - total) return 0;
        total += parts[i].size;
    }
    if (total < DM2_V1_AMIGA_LZX_HEADER_SIZE) return 0;
    joined = (uint8_t *)malloc(total);
    if (!joined) return 0;
    for (i = 0u; i < DM2_V1_AMIGA_LZX_PART_COUNT; ++i) {
        memcpy(joined + offset, parts[i].bytes, parts[i].size);
        offset += parts[i].size;
    }
    if (memcmp(joined, "LZX\0", 4u) != 0) {
        free(joined);
        return 0;
    }
    *out_bytes = joined;
    *out_size = total;
    return 1;
}

void dm2_v1_amiga_lzx_free(uint8_t *bytes) {
    free(bytes);
}

int dm2_v1_amiga_lzx_parse(DM2_V1_AmigaLzxArchive *out,
                           const uint8_t *bytes, size_t size) {
    size_t offset;
    unsigned int count = 0u;
    if (!out || !bytes || size < DM2_V1_AMIGA_LZX_HEADER_SIZE ||
        size > DM2_V1_AMIGA_LZX_MAX_SIZE || memcmp(bytes, "LZX\0", 4u) != 0) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    offset = DM2_V1_AMIGA_LZX_HEADER_SIZE;
    while (offset < size) {
        const uint8_t *header;
        DM2_V1_AmigaLzxEntry *entry;
        uint8_t comment_size;
        uint8_t name_size;
        size_t metadata_size;
        if (count == DM2_V1_AMIGA_LZX_ENTRY_MAX ||
            size - offset < DM2_V1_AMIGA_LZX_ENTRY_FIXED_SIZE) return 0;
        header = bytes + offset;
        comment_size = header[14u];
        name_size = header[30u];
        if (name_size == 0u || name_size >= DM2_V1_AMIGA_LZX_NAME_MAX ||
            (size_t)comment_size > size - offset - DM2_V1_AMIGA_LZX_ENTRY_FIXED_SIZE ||
            (size_t)name_size > size - offset - DM2_V1_AMIGA_LZX_ENTRY_FIXED_SIZE - comment_size) {
            return 0;
        }
        metadata_size = DM2_V1_AMIGA_LZX_ENTRY_FIXED_SIZE +
                        (size_t)comment_size + (size_t)name_size;
        entry = &out->entries[count];
        entry->attributes = lzx_le16(header);
        entry->uncompressed_size = lzx_le32(header + 2u);
        entry->compressed_size = lzx_le32(header + 6u);
        entry->os = header[10u];
        entry->method = header[11u];
        entry->flags = lzx_le16(header + 12u);
        entry->version = header[15u];
        entry->data_crc32 = lzx_le32(header + 22u);
        if (entry->uncompressed_size == 0u ||
            (entry->method != DM2_V1_AMIGA_LZX_METHOD_STORE &&
             entry->method != DM2_V1_AMIGA_LZX_METHOD_LZX)) return 0;
        memcpy(entry->name, header + DM2_V1_AMIGA_LZX_ENTRY_FIXED_SIZE,
               name_size);
        entry->name[name_size] = '\0';
        offset += metadata_size;
        entry->data_offset = offset;
        /* A zero compressed size is a legal LZX solid-stream continuation. */
        if (entry->compressed_size != 0u) {
            if ((size_t)entry->compressed_size > size - offset) return 0;
            offset += entry->compressed_size;
        }
        ++count;
    }
    if (count == 0u || offset != size) return 0;
    out->valid = 1;
    out->size = size;
    out->entry_count = count;
    return 1;
}

const DM2_V1_AmigaLzxEntry *dm2_v1_amiga_lzx_find(
    const DM2_V1_AmigaLzxArchive *archive, const char *name) {
    unsigned int i;
    if (!archive || !archive->valid || !name || name[0] == '\0') return NULL;
    for (i = 0u; i < archive->entry_count; ++i) {
        if (strcmp(archive->entries[i].name, name) == 0) return &archive->entries[i];
    }
    return NULL;
}

int dm2_v1_amiga_lzx_has_install_payload(const DM2_V1_AmigaLzxArchive *archive) {
    static const char *const required[] = {
        "DUNGEON.DAT", "GRAPHICS.DAT", "CD.DAT",
        "music/SK00.MOD", "music/SK09.MOD"
    };
    size_t i;
    if (!archive || !archive->valid) return 0;
    for (i = 0u; i < sizeof(required) / sizeof(required[0]); ++i) {
        if (!dm2_v1_amiga_lzx_find(archive, required[i])) return 0;
    }
    return 1;
}
