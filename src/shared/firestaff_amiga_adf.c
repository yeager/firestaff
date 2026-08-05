#include "firestaff_amiga_adf.h"

#include <stdlib.h>
#include <string.h>

enum { ADF_BLOCK = 512, ADF_WORDS = 128, ADF_FILE_TYPE = 2, ADF_DATA_TYPE = 8,
       ADF_LIST_TYPE = 16, ADF_FILE_SUBTYPE = 0xfffffffdU, ADF_DATA_BYTES = 488,
       ADF_POINTERS = 72 };

static uint32_t adf_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

static const uint8_t *adf_block(const uint8_t *image, size_t blocks,
                                uint32_t number) {
    if (!image || number == 0U || (size_t)number >= blocks) return NULL;
    return image + (size_t)number * ADF_BLOCK;
}

static int adf_append_pointers(const uint8_t *image, size_t blocks,
                               const uint8_t *header,
                               uint32_t **out, size_t *count, size_t *capacity) {
    const uint8_t *node = header;
    uint32_t extension = adf_be32(header + 126U * 4U);
    unsigned int guard = 0U;
    for (;;) {
        uint32_t block_count = adf_be32(node + 2U * 4U);
        size_t i;
        if (block_count > ADF_POINTERS || *count + block_count > 65536U) return 0;
        if (*count + block_count > *capacity) {
            size_t next = *capacity ? *capacity * 2U : 128U;
            uint32_t *grown;
            while (next < *count + block_count) next *= 2U;
            grown = (uint32_t *)realloc(*out, next * sizeof(**out));
            if (!grown) return 0;
            *out = grown; *capacity = next;
        }
        for (i = 0U; i < block_count; ++i) {
            uint32_t ptr = adf_be32(node + (ADF_WORDS - 51U - i) * 4U);
            if (!adf_block(image, blocks, ptr)) return 0;
            (*out)[(*count)++] = ptr;
        }
        if (extension == 0U) return 1;
        if (++guard > 1024U) return 0;
        node = adf_block(image, blocks, extension);
        if (!node || adf_be32(node) != ADF_LIST_TYPE ||
            adf_be32(node + 127U * 4U) != ADF_FILE_SUBTYPE ||
            adf_be32(node + 4U) != extension) return 0;
        extension = adf_be32(node + 126U * 4U);
    }
}

static int adf_visit_file(const uint8_t *image, size_t blocks, uint32_t number,
                          FirestaffAmigaAdfFileVisitor visitor, void *user) {
    const uint8_t *header = adf_block(image, blocks, number);
    uint32_t bytes; uint32_t *pointers = NULL; size_t count = 0U, capacity = 0U;
    uint8_t *contents; size_t used = 0U, i; char name[32]; unsigned int name_len;
    int rc;
    if (!header || adf_be32(header) != ADF_FILE_TYPE ||
        adf_be32(header + 127U * 4U) != ADF_FILE_SUBTYPE ||
        adf_be32(header + 4U) != number) return 0;
    name_len = header[432U];
    if (name_len == 0U || name_len > 30U) return 0;
    memcpy(name, header + 433U, name_len); name[name_len] = '\0';
    bytes = adf_be32(header + 81U * 4U);
    if (bytes == 0U || bytes > 32U * 1024U * 1024U ||
        !adf_append_pointers(image, blocks, header, &pointers, &count, &capacity)) {
        free(pointers); return 0;
    }
    contents = (uint8_t *)malloc(bytes);
    if (!contents) { free(pointers); return 0; }
    for (i = 0U; i < count && used < bytes; ++i) {
        const uint8_t *data = adf_block(image, blocks, pointers[i]);
        uint32_t part;
        if (!data || adf_be32(data) != ADF_DATA_TYPE || adf_be32(data + 4U) != number ||
            adf_be32(data + 8U) != i + 1U || (part = adf_be32(data + 12U)) > ADF_DATA_BYTES ||
            part > bytes - used) { free(contents); free(pointers); return 0; }
        memcpy(contents + used, data + 24U, part); used += part;
    }
    free(pointers);
    if (used != bytes) { free(contents); return 0; }
    rc = visitor ? visitor(name, contents, bytes, user) : 0;
    free(contents);
    return rc < 0 ? -1 : 1;
}

int firestaff_amiga_adf_visit_ofs_files(const uint8_t *image, size_t image_size,
                                        FirestaffAmigaAdfFileVisitor visitor,
                                        void *user_data) {
    size_t blocks, i; int found = 0;
    if (!image || image_size < ADF_BLOCK * 4U || image_size % ADF_BLOCK != 0U ||
        memcmp(image, "DOS\0", 4U) != 0) return -1;
    blocks = image_size / ADF_BLOCK;
    for (i = 2U; i < blocks; ++i) {
        const uint8_t *block = image + i * ADF_BLOCK;
        int result;
        if (adf_be32(block) != ADF_FILE_TYPE ||
            adf_be32(block + 127U * 4U) != ADF_FILE_SUBTYPE) continue;
        result = adf_visit_file(image, blocks, (uint32_t)i, visitor, user_data);
        if (result < 0) return -1;
        if (result > 0) ++found;
    }
    return found;
}
