#include "nexus_v1_bpx_bpk.h"
#include "nexus_v1_bpk_archive.h"

#include <ctype.h>
#include <string.h>

static uint16_t rb16(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint32_t rb32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) |
           (uint32_t)p[3];
}

static int ascii_equal_ignore_case(char a, char b) {
    return tolower((unsigned char)a) == tolower((unsigned char)b);
}

static int has_archive_suffix(const char *path) {
    size_t len;
    if (!path) return 0;
    len = strlen(path);
    if (len < 4) return 0;
    return path[len - 4] == '.' &&
           ascii_equal_ignore_case(path[len - 3], 'b') &&
           ascii_equal_ignore_case(path[len - 2], 'p') &&
           (ascii_equal_ignore_case(path[len - 1], 'k') ||
            ascii_equal_ignore_case(path[len - 1], 'x'));
}

static int is_zero_reserved(const uint8_t *p, size_t count) {
    size_t i;
    for (i = 0; i < count; ++i) {
        if (p[i] != 0) return 0;
    }
    return 1;
}

static int range_fits(size_t offset, size_t count, size_t size) {
    return offset <= size && count <= size - offset;
}

int nexus_v1_bpx_bpk_identify_marker(const char *path,
                                     uint32_t size,
                                     const char *sha256,
                                     Nexus_V1_BpxBpkFormat *out_format) {
    if (!out_format) return NEXUS_V1_BPX_BPK_ERR_NULL;
    *out_format = NEXUS_V1_BPX_BPK_FORMAT_UNKNOWN;

    if (sha256 &&
        size == NEXUS_V1_MENU_BPK_SIZE &&
        strcmp(sha256, NEXUS_V1_MENU_BPK_SHA256) == 0) {
        *out_format = NEXUS_V1_BPX_BPK_FORMAT_VERIFIED_MENU_BPK_MARKER;
        return NEXUS_V1_BPX_BPK_OK;
    }

    if (has_archive_suffix(path)) {
        return NEXUS_V1_BPX_BPK_ERR_UNSUPPORTED;
    }

    return NEXUS_V1_BPX_BPK_ERR_BAD_MAGIC;
}

int nexus_v1_bpx0_parse(const uint8_t *data,
                        size_t size,
                        Nexus_V1_BpxBpkArchive *out_archive) {
    uint16_t version;
    uint16_t entry_count;
    uint32_t table_offset;
    uint32_t data_offset;
    size_t table_bytes;
    uint16_t i;

    if (!data || !out_archive) return NEXUS_V1_BPX_BPK_ERR_NULL;
    memset(out_archive, 0, sizeof(*out_archive));

    if (size < NEXUS_V1_BPX0_HEADER_SIZE) {
        return NEXUS_V1_BPX_BPK_ERR_TOO_SMALL;
    }
    if (memcmp(data, "BPX0", 4) != 0) {
        return NEXUS_V1_BPX_BPK_ERR_BAD_MAGIC;
    }

    version = rb16(data + 4);
    if (version != 1u) return NEXUS_V1_BPX_BPK_ERR_UNSUPPORTED;

    entry_count = rb16(data + 6);
    if (entry_count == 0u || entry_count > NEXUS_V1_BPX_BPK_MAX_ENTRIES) {
        return NEXUS_V1_BPX_BPK_ERR_COUNT;
    }

    table_offset = rb32(data + 8);
    data_offset = rb32(data + 12);
    table_bytes = (size_t)entry_count * NEXUS_V1_BPX0_ENTRY_SIZE;
    if (!range_fits(table_offset, table_bytes, size) ||
        data_offset < table_offset + table_bytes ||
        data_offset > size) {
        return NEXUS_V1_BPX_BPK_ERR_BOUNDS;
    }

    out_archive->format = NEXUS_V1_BPX_BPK_FORMAT_SYNTHETIC_BPX0;
    out_archive->entry_count = entry_count;
    out_archive->table_offset = table_offset;
    out_archive->data_offset = data_offset;

    for (i = 0; i < entry_count; ++i) {
        const uint8_t *record = data + table_offset + ((size_t)i * NEXUS_V1_BPX0_ENTRY_SIZE);
        Nexus_V1_BpxBpkEntry *entry = &out_archive->entries[i];
        uint32_t offset;
        uint32_t packed_size;
        uint32_t unpacked_size;
        uint8_t method;

        memcpy(entry->name, record, sizeof(entry->name));
        entry->name[sizeof(entry->name) - 1] = '\0';
        offset = rb32(record + 16);
        packed_size = rb32(record + 20);
        unpacked_size = rb32(record + 24);
        method = record[28];

        if (!is_zero_reserved(record + 29, 3)) {
            return NEXUS_V1_BPX_BPK_ERR_UNSUPPORTED;
        }
        if (method != NEXUS_V1_BPX_BPK_METHOD_STORED &&
            method != NEXUS_V1_BPX_BPK_METHOD_COMPRESSED_UNKNOWN) {
            return NEXUS_V1_BPX_BPK_ERR_METHOD;
        }
        if (offset < data_offset || !range_fits(offset, packed_size, size)) {
            return NEXUS_V1_BPX_BPK_ERR_BOUNDS;
        }
        if (method == NEXUS_V1_BPX_BPK_METHOD_STORED &&
            packed_size != unpacked_size) {
            return NEXUS_V1_BPX_BPK_ERR_BOUNDS;
        }
        if (entry->name[0] == '\0') {
            return NEXUS_V1_BPX_BPK_ERR_BOUNDS;
        }

        entry->offset = offset;
        entry->packed_size = packed_size;
        entry->unpacked_size = unpacked_size;
        entry->method = method;
    }

    return NEXUS_V1_BPX_BPK_OK;
}

int nexus_v1_bpx_prs3_parse(const uint8_t *data,
                            size_t size,
                            Nexus_V1_BpxBpkArchive *out_archive) {
    uint16_t entry_count;
    uint32_t table_offset;
    uint32_t data_offset;
    size_t table_bytes;
    uint16_t i;

    if (!data || !out_archive) return NEXUS_V1_BPX_BPK_ERR_NULL;
    memset(out_archive, 0, sizeof(*out_archive));

    if (size < NEXUS_V1_BPX0_HEADER_SIZE) {
        return NEXUS_V1_BPX_BPK_ERR_TOO_SMALL;
    }
    if (memcmp(data, "BPX3", 4) != 0) {
        return NEXUS_V1_BPX_BPK_ERR_BAD_MAGIC;
    }

    /* Layout: magic BPX3, version u16, count u16, table_offset u32,
     * data_offset u32. Identical to BPX0 except for the magic byte. */
    if (rb16(data + 4) != 1u) return NEXUS_V1_BPX_BPK_ERR_UNSUPPORTED;

    entry_count = rb16(data + 6);
    if (entry_count == 0u || entry_count > NEXUS_V1_BPX_BPK_MAX_ENTRIES) {
        return NEXUS_V1_BPX_BPK_ERR_COUNT;
    }

    table_offset = rb32(data + 8);
    data_offset = rb32(data + 12);
    table_bytes = (size_t)entry_count * NEXUS_V1_BPX0_ENTRY_SIZE;
    if (!range_fits(table_offset, table_bytes, size) ||
        data_offset < table_offset + table_bytes ||
        data_offset > size) {
        return NEXUS_V1_BPX_BPK_ERR_BOUNDS;
    }

    out_archive->format = NEXUS_V1_BPX_BPK_FORMAT_SYNTHETIC_PRS3;
    out_archive->entry_count = entry_count;
    out_archive->table_offset = table_offset;
    out_archive->data_offset = data_offset;

    for (i = 0; i < entry_count; ++i) {
        const uint8_t *record = data + table_offset + ((size_t)i * NEXUS_V1_BPX0_ENTRY_SIZE);
        Nexus_V1_BpxBpkEntry *entry = &out_archive->entries[i];
        uint16_t width;
        uint8_t mode;
        uint8_t height;
        uint32_t offset;
        uint32_t pixel_count;

        /* 32-byte synthetic PRS3 record layout (BPX3):
         *   bytes  0..16 : entry name (NUL-padded)
         *   bytes 16..18 : width (BE uint16)
         *   byte  18    : mode tag (6/14/22/30)
         *   byte  19    : height (BE uint8) -- aligned to MENU.BPK byte 19
         *   bytes 20..24 : pixel count (BE uint32, must equal width*height)
         *   bytes 24..28 : payload offset (BE uint32)
         *   bytes 28..32 : reserved (must be zero)
         *
         * The PRS3 magic ("PRS3") and version word (0x00000001) are
         * implicit and re-emitted by extract/printer helpers when needed.
         * This keeps every entry at the fixed 32-byte BPX record size
         * while preserving the MENU.BPK byte 19 anchor for height. */
        memcpy(entry->name, record, sizeof(entry->name));
        entry->name[sizeof(entry->name) - 1] = '\0';
        if (entry->name[0] == '\0') {
            return NEXUS_V1_BPX_BPK_ERR_BOUNDS;
        }

        width = rb16(record + 16);
        mode = record[18];
        height = record[19];

        /* pass1083: a directory-trailer entry uses mode == MODE_TRAILER
         * (10). It carries no PRS3 magic, no width/height/pixel_count,
         * no payload offset. Real MENU.BPK entry[0] is the only observed
         * trailer; bytes 0..4 / 4..8 hold BE uint32 file offsets into
         * the last two picture entries. Synthetic BPX3 trailers store
         * the same shape inside the 32-byte record so launcher/probe
         * code can verify the contract without needing real assets. */
        if (mode == NEXUS_V1_BPK_MODE_TRAILER) {
            if (width != 0u || height != 0u) {
                return NEXUS_V1_BPX_BPK_ERR_BOUNDS;
            }
            if (!is_zero_reserved(record + 20, 12)) {
                return NEXUS_V1_BPX_BPK_ERR_UNSUPPORTED;
            }
            entry->offset = 0U;
            entry->packed_size = 0U;
            entry->unpacked_size = 0U;
            entry->method = NEXUS_V1_BPX_BPK_METHOD_DIRECTORY_TRAILER;
            entry->width = 0U;
            entry->height = 0U;
            entry->mode = mode;
            entry->pixel_count = 0U;
            entry->has_prs3_magic = 0;
            continue;
        }

        if (mode != NEXUS_V1_BPK_MODE_8BPP &&
            mode != NEXUS_V1_BPK_MODE_16BPP &&
            mode != NEXUS_V1_BPK_MODE_24BPP &&
            mode != NEXUS_V1_BPK_MODE_32BPP) {
            return NEXUS_V1_BPX_BPK_ERR_METHOD;
        }
        if (width == 0u || height == 0u) {
            return NEXUS_V1_BPX_BPK_ERR_BOUNDS;
        }

        pixel_count = rb32(record + 20);
        if (pixel_count != (uint32_t)width * (uint32_t)height) {
            return NEXUS_V1_BPX_BPK_ERR_BOUNDS;
        }

        offset = rb32(record + 24);
        if (offset < data_offset || offset > size) {
            return NEXUS_V1_BPX_BPK_ERR_BOUNDS;
        }
        if (!is_zero_reserved(record + 28, 4)) {
            return NEXUS_V1_BPX_BPK_ERR_UNSUPPORTED;
        }

        entry->offset = offset;
        entry->packed_size = (uint32_t)(size - offset);
        entry->unpacked_size = pixel_count * (uint32_t)mode;
        entry->method = NEXUS_V1_BPX_BPK_METHOD_PRS3_UNKNOWN;
        entry->width = width;
        entry->height = height;
        entry->mode = mode;
        entry->pixel_count = pixel_count;
        entry->has_prs3_magic = 1;
    }

    return NEXUS_V1_BPX_BPK_OK;
}

const Nexus_V1_BpxBpkEntry *nexus_v1_bpx_bpk_find_entry(
    const Nexus_V1_BpxBpkArchive *archive,
    const char *name) {
    uint16_t i;
    if (!archive || !name) return NULL;
    for (i = 0; i < archive->entry_count; ++i) {
        if (strncmp(archive->entries[i].name, name, sizeof(archive->entries[i].name)) == 0) {
            return &archive->entries[i];
        }
    }
    return NULL;
}

int nexus_v1_bpx_bpk_extract_stored(const uint8_t *archive_data,
                                    size_t archive_size,
                                    const Nexus_V1_BpxBpkEntry *entry,
                                    uint8_t *out,
                                    size_t out_size) {
    if (!archive_data || !entry || !out) return NEXUS_V1_BPX_BPK_ERR_NULL;
    if (entry->method != NEXUS_V1_BPX_BPK_METHOD_STORED) {
        return NEXUS_V1_BPX_BPK_ERR_METHOD;
    }
    if (entry->packed_size != entry->unpacked_size) {
        return NEXUS_V1_BPX_BPK_ERR_BOUNDS;
    }
    if ((size_t)entry->unpacked_size > out_size) {
        return NEXUS_V1_BPX_BPK_ERR_OUTPUT_TOO_SMALL;
    }
    if (!range_fits(entry->offset, entry->packed_size, archive_size)) {
        return NEXUS_V1_BPX_BPK_ERR_BOUNDS;
    }

    memcpy(out, archive_data + entry->offset, entry->packed_size);
    return (int)entry->unpacked_size;
}

const char *nexus_v1_bpx_bpk_status_string(int status) {
    switch (status) {
    case NEXUS_V1_BPX_BPK_OK: return "ok";
    case NEXUS_V1_BPX_BPK_ERR_NULL: return "null";
    case NEXUS_V1_BPX_BPK_ERR_TOO_SMALL: return "too-small";
    case NEXUS_V1_BPX_BPK_ERR_BAD_MAGIC: return "bad-magic";
    case NEXUS_V1_BPX_BPK_ERR_UNSUPPORTED: return "unsupported";
    case NEXUS_V1_BPX_BPK_ERR_COUNT: return "bad-count";
    case NEXUS_V1_BPX_BPK_ERR_BOUNDS: return "bad-bounds";
    case NEXUS_V1_BPX_BPK_ERR_NOT_FOUND: return "not-found";
    case NEXUS_V1_BPX_BPK_ERR_OUTPUT_TOO_SMALL: return "output-too-small";
    case NEXUS_V1_BPX_BPK_ERR_METHOD: return "bad-method";
    default: return "unknown-status";
    }
}
