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

typedef struct {
    const uint8_t *bytes;
    size_t size;
    size_t byte_position;
    unsigned int bit_position;
} LzxBits;

typedef struct {
    int child[2];
    int symbol;
} LzxHuffmanNode;

typedef struct {
    LzxHuffmanNode nodes[1536];
    unsigned int count;
} LzxHuffman;

static int lzx_next_bit(LzxBits *bits, unsigned int *out) {
    size_t physical;
    uint8_t byte;
    if (!bits || !out || bits->byte_position >= bits->size) return 0;
    physical = bits->byte_position & ~(size_t)1u;
    if (physical + 1u >= bits->size) return 0;
    byte = bits->bytes[physical + ((bits->byte_position & 1u) ? 0u : 1u)];
    *out = (unsigned int)((byte >> bits->bit_position) & 1u);
    if (++bits->bit_position == 8u) {
        bits->bit_position = 0u;
        ++bits->byte_position;
    }
    return 1;
}

static int lzx_next_bits(LzxBits *bits, unsigned int count, unsigned int *out) {
    unsigned int value = 0u, bit, i;
    if (!out || count > 24u) return 0;
    for (i = 0u; i < count; ++i) {
        if (!lzx_next_bit(bits, &bit)) return 0;
        value |= bit << i;
    }
    *out = value;
    return 1;
}

static void lzx_huffman_reset(LzxHuffman *tree) {
    memset(tree, 0, sizeof(*tree));
    tree->nodes[0].child[0] = -1;
    tree->nodes[0].child[1] = -1;
    tree->nodes[0].symbol = -1;
    tree->count = 1u;
}

static int lzx_huffman_new_node(LzxHuffman *tree) {
    int node;
    if (!tree || tree->count >= sizeof(tree->nodes) / sizeof(tree->nodes[0])) return -1;
    node = (int)tree->count++;
    tree->nodes[node].child[0] = -1;
    tree->nodes[node].child[1] = -1;
    tree->nodes[node].symbol = -1;
    return node;
}

static int lzx_huffman_build(LzxHuffman *tree, const uint8_t *lengths,
                             unsigned int symbol_count, unsigned int max_length) {
    unsigned int counts[17] = {0u};
    unsigned int next[17] = {0u};
    unsigned int code = 0u, i, length;
    if (!tree || !lengths || max_length > 16u) return 0;
    for (i = 0u; i < symbol_count; ++i) {
        if (lengths[i] > max_length) return 0;
        if (lengths[i] != 0u) ++counts[lengths[i]];
    }
    for (length = 1u; length <= max_length; ++length) {
        code = (code + counts[length - 1u]) << 1u;
        next[length] = code;
        if (code + counts[length] > (1u << length)) return 0;
    }
    lzx_huffman_reset(tree);
    for (i = 0u; i < symbol_count; ++i) {
        int node = 0;
        if (lengths[i] == 0u) continue;
        code = next[lengths[i]]++;
        for (length = lengths[i]; length > 0u; --length) {
            unsigned int bit = (code >> (length - 1u)) & 1u;
            int child = tree->nodes[node].child[bit];
            if (child < 0) {
                child = lzx_huffman_new_node(tree);
                if (child < 0) return 0;
                tree->nodes[node].child[bit] = child;
            }
            node = child;
        }
        if (tree->nodes[node].symbol >= 0) return 0;
        tree->nodes[node].symbol = (int)i;
    }
    return tree->count > 1u;
}

static int lzx_huffman_symbol(LzxBits *bits, const LzxHuffman *tree, int *out) {
    int node = 0;
    unsigned int bit, guard = 0u;
    if (!bits || !tree || !out) return 0;
    while (tree->nodes[node].symbol < 0) {
        if (++guard > 16u || !lzx_next_bit(bits, &bit) ||
            tree->nodes[node].child[bit] < 0) return 0;
        node = tree->nodes[node].child[bit];
    }
    *out = tree->nodes[node].symbol;
    return 1;
}

static int lzx_read_delta_lengths(LzxBits *bits, uint8_t *lengths,
                                  unsigned int count, int alternate) {
    uint8_t prelengths[20];
    LzxHuffman precode;
    unsigned int value, i = 0u, n, length;
    int symbol;
    for (value = 0u; value < 20u; ++value) {
        if (!lzx_next_bits(bits, 4u, &n)) return 0;
        prelengths[value] = (uint8_t)n;
    }
    if (!lzx_huffman_build(&precode, prelengths, 20u, 15u)) return 0;
    while (i < count) {
        if (!lzx_huffman_symbol(bits, &precode, &symbol)) return 0;
        if (symbol <= 16) {
            n = 1u;
            length = (unsigned int)((lengths[i] + 17u - (unsigned int)symbol) % 17u);
        } else if (symbol == 17) {
            if (!lzx_next_bits(bits, 4u, &value)) return 0;
            n = value + 4u - (unsigned int)alternate;
            length = 0u;
        } else if (symbol == 18) {
            if (!lzx_next_bits(bits, 5u + (unsigned int)alternate, &value)) return 0;
            n = value + 20u - (unsigned int)alternate;
            length = 0u;
        } else if (symbol == 19) {
            if (!lzx_next_bits(bits, 1u, &value) ||
                !lzx_huffman_symbol(bits, &precode, &symbol) || symbol > 16) return 0;
            n = value + 4u - (unsigned int)alternate;
            length = (unsigned int)((lengths[i] + 17u - (unsigned int)symbol) % 17u);
        } else {
            return 0;
        }
        if (n == 0u || n > count - i) return 0;
        memset(lengths + i, (int)length, n);
        i += n;
    }
    return 1;
}

static uint32_t lzx_crc32(const uint8_t *bytes, size_t size) {
    uint32_t crc = UINT32_C(0xffffffff);
    size_t i;
    unsigned int j;
    for (i = 0u; i < size; ++i) {
        crc ^= bytes[i];
        for (j = 0u; j < 8u; ++j)
            crc = (crc >> 1u) ^ (UINT32_C(0xedb88320) & (uint32_t)-(int)(crc & 1u));
    }
    return ~crc;
}

static int lzx_decode_stream(const uint8_t *compressed, size_t compressed_size,
                             uint8_t *output, size_t output_size) {
    static const uint8_t additional[32] = {
        0u,0u,0u,0u,1u,1u,2u,2u,3u,3u,4u,4u,5u,5u,6u,6u,
        7u,7u,8u,8u,9u,9u,10u,10u,11u,11u,12u,12u,13u,13u,14u,14u
    };
    static const uint16_t base[32] = {
        0u,1u,2u,3u,4u,6u,8u,12u,16u,24u,32u,48u,64u,96u,128u,192u,
        256u,384u,512u,768u,1024u,1536u,2048u,3072u,4096u,6144u,
        8192u,12288u,16384u,24576u,32768u,49152u
    };
    LzxBits bits = {compressed, compressed_size, 0u, 0u};
    uint8_t lengths[768] = {0u};
    LzxHuffman maincode, offsetcode;
    size_t position = 0u, block_end = 0u;
    unsigned int last_offset = 1u, block_type = 0u;
    while (position < output_size) {
        int symbol;
        if (position >= block_end) {
            uint8_t offset_lengths[8];
            unsigned int block_size, value, i;
            if (!lzx_next_bits(&bits, 3u, &block_type) || block_type < 2u || block_type > 3u)
                return 0;
            if (block_type == 3u) {
                for (i = 0u; i < 8u; ++i) {
                    if (!lzx_next_bits(&bits, 3u, &value)) return 0;
                    offset_lengths[i] = (uint8_t)value;
                }
                if (!lzx_huffman_build(&offsetcode, offset_lengths, 8u, 7u)) return 0;
            }
            if (!lzx_next_bits(&bits, 8u, &value)) return 0;
            block_size = value << 16u;
            if (!lzx_next_bits(&bits, 8u, &value)) return 0;
            block_size |= value << 8u;
            if (!lzx_next_bits(&bits, 8u, &value)) return 0;
            block_size |= value;
            if (block_size == 0u || (size_t)block_size > output_size - position) return 0;
            block_end = position + block_size;
            if (!lzx_read_delta_lengths(&bits, lengths, 256u, 0) ||
                !lzx_read_delta_lengths(&bits, lengths + 256u, 512u, 1) ||
                !lzx_huffman_build(&maincode, lengths, 768u, 16u)) return 0;
        }
        if (!lzx_huffman_symbol(&bits, &maincode, &symbol)) return 0;
        if (symbol < 256) {
            output[position++] = (uint8_t)symbol;
        } else {
            unsigned int offset_class = (unsigned int)symbol & 31u;
            unsigned int length_class = ((unsigned int)symbol - 256u) >> 5u;
            unsigned int offset = base[offset_class];
            unsigned int length = (unsigned int)base[length_class] + 3u;
            unsigned int value, i;
            if (length_class >= 16u) return 0;
            if (offset == 0u) offset = last_offset;
            else if (block_type == 3u && additional[offset_class] >= 3u) {
                if (!lzx_next_bits(&bits, additional[offset_class] - 3u, &value) ||
                    !lzx_huffman_symbol(&bits, &offsetcode, &symbol)) return 0;
                offset += (value << 3u) + (unsigned int)symbol;
            } else if (!lzx_next_bits(&bits, additional[offset_class], &value)) {
                return 0;
            } else {
                offset += value;
            }
            if (!lzx_next_bits(&bits, additional[length_class], &value)) return 0;
            length += value;
            if (offset == 0u || offset > 65536u || offset > position ||
                length > block_end - position) return 0;
            for (i = 0u; i < length; ++i) output[position + i] = output[position + i - offset];
            position += length;
            last_offset = offset;
        }
    }
    return position == output_size;
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

int dm2_v1_amiga_lzx_extract_entry(const DM2_V1_AmigaLzxArchive *archive,
                                   const uint8_t *archive_bytes,
                                   const DM2_V1_AmigaLzxEntry *entry,
                                   uint8_t **out_bytes, size_t *out_size) {
    unsigned int target, start, end, i;
    size_t group_size = 0u, target_offset = 0u;
    uint8_t *group = NULL;
    uint8_t *result;
    if (!archive || !archive->valid || !archive_bytes || !entry || !out_bytes || !out_size) return 0;
    *out_bytes = NULL;
    *out_size = 0u;
    for (target = 0u; target < archive->entry_count; ++target)
        if (entry == &archive->entries[target]) break;
    if (target == archive->entry_count) return 0;
    end = target;
    while (end < archive->entry_count && archive->entries[end].compressed_size == 0u) ++end;
    if (end == archive->entry_count) return 0;
    start = end;
    while (start > 0u && archive->entries[start - 1u].compressed_size == 0u) --start;
    for (i = start; i <= end; ++i) {
        if ((size_t)archive->entries[i].uncompressed_size >
            DM2_V1_AMIGA_LZX_MAX_SIZE - group_size) return 0;
        if (i < target) target_offset += archive->entries[i].uncompressed_size;
        group_size += archive->entries[i].uncompressed_size;
    }
    if (archive->entries[end].data_offset > archive->size ||
        archive->entries[end].compressed_size > archive->size - archive->entries[end].data_offset)
        return 0;
    group = (uint8_t *)malloc(group_size);
    if (!group) return 0;
    if (archive->entries[end].method == DM2_V1_AMIGA_LZX_METHOD_STORE) {
        if (archive->entries[end].compressed_size != group_size) { free(group); return 0; }
        memcpy(group, archive_bytes + archive->entries[end].data_offset, group_size);
    } else if (archive->entries[end].method == DM2_V1_AMIGA_LZX_METHOD_LZX) {
        if (!lzx_decode_stream(archive_bytes + archive->entries[end].data_offset,
                               archive->entries[end].compressed_size, group, group_size)) {
            free(group);
            return 0;
        }
    } else {
        free(group);
        return 0;
    }
    result = (uint8_t *)malloc(entry->uncompressed_size);
    if (!result) { free(group); return 0; }
    memcpy(result, group + target_offset, entry->uncompressed_size);
    free(group);
    if (lzx_crc32(result, entry->uncompressed_size) != entry->data_crc32) {
        free(result);
        return 0;
    }
    *out_bytes = result;
    *out_size = entry->uncompressed_size;
    return 1;
}
