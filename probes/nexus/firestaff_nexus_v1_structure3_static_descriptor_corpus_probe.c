/* Corpus-proves only the documented Structure3 00xx -> Structure2 descriptor
 * join. Descriptor payload bytes remain opaque and no renderer is authorized. */
#include "nexus_v1_engine.h"
#include "asset_find_by_hash.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint16_t be16(const uint8_t *p) { return (uint16_t)((p[0] << 8) | p[1]); }
static uint32_t be32(const uint8_t *p) { return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3]; }
static uint8_t *read_file(const char *path, int *size) {
    FILE *f; long n; uint8_t *p;
    if (!path || !size || !(f = fopen(path, "rb"))) return NULL;
    if (fseek(f, 0L, SEEK_END) || (n = ftell(f)) <= 0 || n > INT_MAX || fseek(f, 0L, SEEK_SET)) { fclose(f); return NULL; }
    p = malloc((size_t)n);
    if (!p || fread(p, 1U, (size_t)n, f) != (size_t)n) { free(p); fclose(f); return NULL; }
    fclose(f); *size = (int)n; return p;
}
static int canonical_file_matches_md5(const char *path, const char *md5) {
    uint8_t *bytes; int size; int matches;
    bytes = read_file(path, &size);
    if (!bytes) return 0;
    matches = nexus_v1_dgn_bytes_match_canonical_md5(bytes, size, md5);
    free(bytes);
    return matches;
}
static uint64_t hbyte(uint64_t h, uint8_t x) { return (h ^ x) * UINT64_C(1099511628211); }
static uint64_t hu16(uint64_t h, uint16_t x) { h = hbyte(h, (uint8_t)x); return hbyte(h, (uint8_t)(x >> 8)); }
static uint64_t hu32(uint64_t h, uint32_t x) { h = hu16(h, (uint16_t)x); return hu16(h, (uint16_t)(x >> 16)); }
static int bounded_anchor(const Nexus_V1_DgnStructure2Payload *p, uint32_t x) {
    uint32_t end = (uint32_t)(p->opaque_payload_offset + p->opaque_payload_size);
    return x == 0U || (x >= (uint32_t)p->opaque_payload_offset && x < end && (x & 1U) == 0U);
}
static uint32_t next_anchor(const Nexus_V1_Level *level, uint32_t anchor) {
    uint32_t next = (uint32_t)(level->structure2_payload.opaque_payload_offset + level->structure2_payload.opaque_payload_size); int i;
    for (i = 0; i < level->structure2_texture_count; ++i) {
        const Nexus_V1_DgnStructure2Texture *d = &level->structure2_textures[i];
        if (d->image_relative_offset > anchor && d->image_relative_offset < next) next = d->image_relative_offset;
        if (d->palette_relative_offset > anchor && d->palette_relative_offset < next) next = d->palette_relative_offset;
    }
    return next;
}
int main(int argc, char **argv) {
    uint64_t corpus = UINT64_C(1469598103934665603); int static_faces = 0, levels = 0, level;
    uint64_t raw_window_bytes = 0U;
    if (argc != 2) { fprintf(stderr, "usage: %s NEXUS_DATA_DIRECTORY\n", argv[0]); return 2; }
    for (level = 0; level < 16; ++level) {
        char name[16], path[1024]; const char *md5; uint8_t *bytes; int size, entry; Nexus_V1_Level data;
        snprintf(name, sizeof(name), "LEV%02d.DGN", level); md5 = nexus_v1_known_file_md5(name);
        if (snprintf(path, sizeof(path), "%s/%s", argv[1], name) >= (int)sizeof(path) || !md5 || !canonical_file_matches_md5(path, md5) || !(bytes = read_file(path, &size))) goto fail;
        memset(&data, 0, sizeof(data));
        if (nexus_v1_level_load(&data, bytes, size, level) || !data.structure3_face_materials.valid || !data.structure3_face_materials.selector_bindings_complete || !data.structure2_texture_table_valid || !data.structure2_payload.descriptor_offset_envelope_valid) { free(bytes); goto fail; }
        uint32_t structure2_base = (uint32_t)be16(bytes + 0x14U) * 2048U;
        if (structure2_base >= (uint32_t)size) { free(bytes); goto fail; }
        corpus = hbyte(corpus, (uint8_t)level);
        for (entry = 0; entry < data.structure3_directory.entry_count; ++entry) {
            uint32_t off = be32(bytes + data.structure3_payload.byte_offset + 4 + entry * 4);
            const uint8_t *header = bytes + data.structure3_payload.byte_offset + off;
            uint16_t count = be16(header + 6); uint32_t face_off = be32(header + 16); int face;
            for (face = 0; face < count; ++face) {
                const uint8_t *row = bytes + data.structure3_payload.byte_offset + face_off + face * 12;
                uint16_t fill = be16(row + 10); const Nexus_V1_DgnStructure2Texture *d;
                if (!(row[8] & 0x40U) || (fill & 0xff00U)) continue;
                if ((fill & 0xffU) >= (uint16_t)data.structure2_texture_count) { free(bytes); goto fail; }
                d = &data.structure2_textures[fill & 0xffU];
                if (d->image_id != (fill & 0xffU) || !bounded_anchor(&data.structure2_payload, d->image_relative_offset) || !bounded_anchor(&data.structure2_payload, d->palette_relative_offset)) { free(bytes); goto fail; }
                corpus = hu16(corpus, (uint16_t)entry); corpus = hu16(corpus, (uint16_t)face);
                corpus = hu16(corpus, fill); corpus = hu16(corpus, d->encoding); corpus = hu16(corpus, d->palette_id);
                corpus = hu16(corpus, d->width); corpus = hu16(corpus, d->height);
                {
                    uint32_t image_next = next_anchor(&data, d->image_relative_offset);
                    uint32_t palette_next = d->palette_relative_offset ? next_anchor(&data, d->palette_relative_offset) : 0U;
                    if (image_next <= d->image_relative_offset || (d->palette_relative_offset && palette_next <= d->palette_relative_offset)) { free(bytes); goto fail; }
                    if (structure2_base + image_next > (uint32_t)size ||
                        (d->palette_relative_offset && structure2_base + palette_next > (uint32_t)size)) { free(bytes); goto fail; }
                    corpus = hu32(corpus, d->image_relative_offset); corpus = hu32(corpus, image_next);
                    corpus = hu32(corpus, d->palette_relative_offset); corpus = hu32(corpus, palette_next);
                    for (uint32_t i = d->image_relative_offset; i < image_next; ++i) corpus = hbyte(corpus, bytes[structure2_base + i]);
                    raw_window_bytes += image_next - d->image_relative_offset;
                    if (d->palette_relative_offset) {
                        for (uint32_t i = d->palette_relative_offset; i < palette_next; ++i) corpus = hbyte(corpus, bytes[structure2_base + i]);
                        raw_window_bytes += palette_next - d->palette_relative_offset;
                    }
                }
                ++static_faces;
            }
        }
        free(bytes); ++levels;
    }
    printf("verified_levels=%d\nstatic_face_descriptor_bindings=%d\nraw_capture_window_bytes=%llu\n", levels, static_faces, (unsigned long long)raw_window_bytes);
    printf("structure3_static_descriptor_corpus_fnv1a64=%016llx\n", (unsigned long long)corpus);
    printf("payload_decoder_proven=0\nrenderer_authorized=0\n"); return 0;
fail: fprintf(stderr, "canonical Structure3/Structure2 descriptor corpus unavailable\n"); return 1;
}
