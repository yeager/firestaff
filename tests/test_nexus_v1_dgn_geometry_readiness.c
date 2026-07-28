#include "nexus_v1_dungeon.h"
#include "nexus_v1_engine.h"
#include "nexus_v1_bpk_archive.h"
#include "nexus_v1_launcher.h"
#include "nexus_v1_palette.h"
#include "nexus_v1_prs3_capture_trace_schema.h"
#include "nexus_v1_viewport.h"
#include "asset_find_by_hash.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_fail;

static const char *expected_dgn_md5(int level) {
    static const char *const hashes[16] = {
        "603ec9c531a92539babdda84ab09e78e",
        "751e1442bf7dccbd41bf146b5be144ab",
        "e2cb85d9fedc27f894a84e0f465fcde1",
        "19637d6b59849565f64565aed786d7ea",
        "85abc1b822e5c66ec4e99f1f676c140e",
        "ed5d54ab0ac1c927c1346dd966c8a5cc",
        "58c336ff6146e7216f0081e726823ea1",
        "c19e6038a017a320515ecbb66f6da197",
        "9bfc31bea631345a3660c2645be0e95b",
        "32a6450f29eb7babd73fcbe7a0310f22",
        "2928440e9c21457929f1323a28a42f70",
        "d7be5cd0d6e5c10afe99ec9950614fad",
        "db1cf70d6730615f73f191fad5e11e32",
        "f8876d0181d79727013236a6b597b99b",
        "a634dd5e95567ecbbbc332350c8cf12b",
        "5e6e237074f1e6b0decc629868a51f3c"
    };
    return level >= 0 && level < 16 ? hashes[level] : NULL;
}

static int real_dgn_is_hash_verified(const char *path, int level) {
    const char *expected = expected_dgn_md5(level);
    return expected && asset_file_matches_md5(path, expected);
}

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        printf("FAIL: %s\n", msg); \
        g_fail++; \
    } \
} while (0)

typedef struct {
    int packet_count;
    int invalid_packet_count;
} PackageGeometryVisitCount;

typedef struct {
    int packet_count;
    int invalid_packet_count;
} AnimatedMaterialVisitCount;

typedef struct {
    int packet_count;
    int invalid_packet_count;
} AnimatedMaterialImageVisitCount;

typedef struct {
    int packet_count;
    int invalid_packet_count;
} UntexturedFaceVisitCount;

static int count_package_geometry_packet(
    void *context, const Nexus_V1_DgnStructure3PackageGeometryPacket *packet)
{
    PackageGeometryVisitCount *count = (PackageGeometryVisitCount *)context;
    if (!count || !packet) return -1;
    ++count->packet_count;
    if (!packet->valid || !packet->source_geometry_bound ||
        !packet->material_descriptor_bound || !packet->no_draw_only ||
        packet->fallback_visuals_permitted ||
        !packet->blocks_real_dgn_mesh_render ||
        !packet->material_target.image_payload_interval_bound ||
        packet->material_target.image_payload_candidate_byte_count == 0U ||
        (packet->material_target.descriptor_target.descriptor
                 .palette_relative_offset != 0U &&
         (!packet->material_target.palette_payload_interval_bound ||
          packet->material_target.palette_payload_candidate_byte_count == 0U))) {
        ++count->invalid_packet_count;
    }
    return 0;
}

static int count_animated_material_packet(
    void *context, const Nexus_V1_DgnStructure3AnimatedMaterialPacket *packet)
{
    AnimatedMaterialVisitCount *count = (AnimatedMaterialVisitCount *)context;
    if (!count || !packet) return -1;
    ++count->packet_count;
    if (!packet->valid || !packet->source_geometry_bound ||
        !packet->animation_declaration_bound || !packet->first_descriptor_bound ||
        packet->animation_execution_permitted ||
        packet->pixel_palette_vdp1_semantics_proven || packet->decoder_permitted ||
        !packet->no_draw_only || packet->fallback_visuals_permitted ||
        !packet->blocks_real_dgn_mesh_render) {
        ++count->invalid_packet_count;
    }
    return 0;
}

static int count_animated_material_image_packet(
    void *context, const Nexus_V1_DgnStructure3AnimatedMaterialImagePacket *packet)
{
    AnimatedMaterialImageVisitCount *count =
        (AnimatedMaterialImageVisitCount *)context;
    if (!count || !packet) return -1;
    ++count->packet_count;
    if (!packet->valid || !packet->source_geometry_bound ||
        !packet->animation_declaration_bound || !packet->descriptor_bound ||
        !packet->descriptor_target.valid ||
        packet->descriptor_target.descriptor.image_id !=
            packet->structure2_image_id ||
        packet->animation_execution_permitted ||
        packet->pixel_palette_vdp1_semantics_proven || packet->decoder_permitted ||
        !packet->no_draw_only || packet->fallback_visuals_permitted ||
        !packet->blocks_real_dgn_mesh_render) {
        ++count->invalid_packet_count;
    }
    return 0;
}

static int count_untextured_face_packet(
    void *context, const Nexus_V1_DgnStructure3UntexturedFacePacket *packet)
{
    UntexturedFaceVisitCount *count = (UntexturedFaceVisitCount *)context;
    if (!count || !packet) return -1;
    ++count->packet_count;
    if (!packet->valid || !packet->source_geometry_bound ||
        !packet->raw_fill_bound || packet->flat_fill_semantics_proven ||
        packet->pixel_palette_vdp1_semantics_proven || packet->decoder_permitted ||
        !packet->no_draw_only || packet->fallback_visuals_permitted ||
        !packet->blocks_real_dgn_mesh_render || (packet->face.flags & 0x40U)) {
        ++count->invalid_packet_count;
    }
    return 0;
}

static void wb16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xffU);
}

static void wl16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xffU);
    p[1] = (uint8_t)(v >> 8);
}

static void wb32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)((v >> 16) & 0xffU);
    p[2] = (uint8_t)((v >> 8) & 0xffU);
    p[3] = (uint8_t)(v & 0xffU);
}

static uint64_t fnv1a64_update(uint64_t hash, const uint8_t *data, size_t size) {
    size_t i;
    for (i = 0; i < size; ++i) {
        hash ^= data[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static uint64_t fnv1a64(const uint8_t *data, size_t size) {
    return fnv1a64_update(UINT64_C(1469598103934665603), data, size);
}

static uint64_t structure3_capture_bundle_fnv1a64(
    const Nexus_V1_DgnStructure3CaptureImport *capture)
{
    const uint8_t *spans[6] = {
        capture->texture_span, capture->palette_state, capture->vdp1_state,
        capture->transform_state, capture->normal_culling_state,
        capture->vdp1_command
    };
    const size_t sizes[6] = {
        capture->texture_span_size, capture->palette_state_size,
        capture->vdp1_state_size, capture->transform_state_size,
        capture->normal_culling_state_size, capture->vdp1_command_size
    };
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t span;

    for (span = 0U; span < 6U; ++span) {
        uint8_t length[8];
        size_t byte;
        for (byte = 0U; byte < sizeof(length); ++byte)
            length[byte] = (uint8_t)(sizes[span] >> (byte * 8U));
        hash = fnv1a64_update(hash, length, sizeof(length));
        hash = fnv1a64_update(hash, spans[span], sizes[span]);
    }
    return hash;
}

static uint64_t structure3_capture_trace_order_fnv1a64(
    const uint64_t sequence[6])
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t lane;

    for (lane = 0U; lane < 6U; ++lane) {
        uint8_t lane_id = (uint8_t)lane;
        uint8_t value[8];
        size_t byte;
        hash = fnv1a64_update(hash, &lane_id, sizeof(lane_id));
        for (byte = 0U; byte < sizeof(value); ++byte)
            value[byte] = (uint8_t)(sequence[lane] >> (byte * 8U));
        hash = fnv1a64_update(hash, value, sizeof(value));
    }
    return hash;
}

static int write_capture_span(const char *path, const uint8_t *data, size_t size)
{
    FILE *file = fopen(path, "wb");
    if (!file) return 0;
    if (fwrite(data, 1U, size, file) != size) {
        fclose(file);
        return 0;
    }
    return fclose(file) == 0;
}

static uint32_t fnv1a32(const uint8_t *data, size_t size) {
    uint32_t hash = 2166136261U;
    size_t i;
    for (i = 0U; i < size; ++i) {
        hash ^= data[i];
        hash *= 16777619U;
    }
    return hash;
}

static uint32_t fnv1a32_repeated(const uint8_t *data, size_t size, int count) {
    uint32_t hash = 2166136261U;
    int repeat;
    for (repeat = 0; repeat < count; ++repeat) {
        size_t i;
        for (i = 0U; i < size; ++i) {
            hash ^= data[i];
            hash *= 16777619U;
        }
    }
    return hash;
}

static uint8_t *cell_at(uint8_t *structure1, int structure1b_rel, int x, int y) {
    return structure1 + structure1b_rel +
           ((y * NEXUS_MAX_MAP_SIZE + x) *
            NEXUS_DGN_STRUCTURE1B_CELL_BYTES);
}

static int build_dmweb_dgn(uint8_t *buf,
                           int buf_size,
                           int structure1_blocks,
                           int structure1b_rel,
                           int geometry_bytes) {
    uint8_t *structure1;
    int useful;

    if (!buf || buf_size <= 0) {
        return -1;
    }
    memset(buf, 0, (size_t)buf_size);
    useful = structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + geometry_bytes;
    if (useful > structure1_blocks * NEXUS_DGN_BLOCK_SIZE) {
        return -1;
    }

    wb16(buf + 0x0C, 1U);
    wb16(buf + 0x0E, (uint16_t)structure1_blocks);
    wb32(buf + 0x10, (uint32_t)useful);

    structure1 = buf + NEXUS_DGN_BLOCK_SIZE;
    structure1[0] = 0x00;
    structure1[1] = 0x50;
    structure1[2] = 0x40;
    structure1[3] = 0x40;
    wb32(structure1 + 0x10, 0x38U);
    wb32(structure1 + 0x14, (uint32_t)structure1b_rel);
    wb32(structure1 + 0x18,
         (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES));
    if (geometry_bytes >= 168) {
        int records_end = 152 + ((geometry_bytes - 152) / 16) * 16;
        int record_count = (records_end - 152) / 16;
        int record;
        if (record_count >
            NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_MASK + 1) {
            record_count = NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_MASK + 1;
            records_end = 152 + record_count * 16;
        }
        /* Bounded Structure1C, reserved 0x24 span, then opaque 0x30 rows. */
        wb32(structure1 + 0x24,
             (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 24));
        wb32(structure1 + 0x2c,
             (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 152));
        wb32(structure1 + 0x30,
             (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 152));
        wb32(structure1 + 0x34,
             (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES +
                        records_end));
        structure1[structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES] = 6;
        for (record = 0; record < record_count - 1; ++record) {
            structure1[structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES +
                       152 + record * 16 +
                       NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_BYTE] =
                (uint8_t)record;
        }
    }
    return 0;
}

static void set_collision_ref(uint8_t *structure1,
                              int structure1b_rel,
                              int x,
                              int y,
                              int ref) {
    uint8_t *cell = cell_at(structure1, structure1b_rel, x, y);
    cell[6] = (uint8_t)((ref >> 8) & 0x0f);
    cell[7] = (uint8_t)(ref & 0xff);
}

static void set_post_grid_0x30_ref(uint8_t *structure1,
                                   int structure1b_rel,
                                   int x,
                                   int y,
                                   int ref) {
    uint8_t *cell = cell_at(structure1, structure1b_rel, x, y);
    cell[5] = (uint8_t)((ref >> 4) & 0xff);
    cell[6] = (uint8_t)((cell[6] & 0x0fU) | ((ref & 0x0f) << 4));
}

static void set_structure1a_owner_ref(uint8_t *structure1,
                                       int structure1b_rel,
                                       int x, int y, int ref) {
    uint8_t *cell = cell_at(structure1, structure1b_rel, x, y);
    cell[4] = (uint8_t)(cell[4] | 0x80U);
    cell[5] = (uint8_t)((ref >> 4) & 0xff);
    cell[6] = (uint8_t)((cell[6] & 0x0fU) | ((ref & 0x0f) << 4));
}

static void set_floor_flags(uint8_t *structure1, int structure1b_rel,
                            int x, int y, uint16_t flags) {
    uint8_t *cell = cell_at(structure1, structure1b_rel, x, y);
    cell[0] = (uint8_t)(flags >> 8);
    cell[1] = (uint8_t)(flags & 0xffU);
}

static void build_structure1f_fixture(uint8_t *structure1,
                                      int structure1b_rel) {
    uint8_t *structure1f = structure1 + structure1b_rel +
        NEXUS_DGN_STRUCTURE1B_BYTES + 312;
    static const uint8_t tags[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT] =
        {0x10U, 0x11U, 0x12U, 0x20U, 0x21U, 0x22U};
    static const int sizes[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT] =
        {8, 12, 16, 12, 12, 16};
    static const int counts[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT] =
        {2, 2, 2, 2, 2, 4};
    int family;
    int cursor = NEXUS_DGN_STRUCTURE1F_HEADER_BYTES;

    wb32(structure1 + 0x34, (uint32_t)(structure1b_rel +
                                        NEXUS_DGN_STRUCTURE1B_BYTES + 312));
    wb16(structure1f, 0x0034U);
    wb16(structure1f + 2, 0x0012U);
    for (family = 0; family < NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT; ++family) {
        int record;
        wb16(structure1f + 4 + family * 2, (uint16_t)counts[family]);
        for (record = 0; record < counts[family]; ++record) {
            uint8_t *entry = structure1f + cursor + record * sizes[family];
            entry[0] = tags[family];
            if (family <= NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS) {
                entry[1] = (uint8_t)(10 + record);
                entry[2] = (uint8_t)(20 + record);
            }
        }
        cursor += counts[family] * sizes[family];
    }
    structure1f[NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 3] = 4;
    structure1f[NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 4] = 0x8eU;
    structure1f[NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 5] = 7;
    structure1f[NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 7] = 15;
    /* Floor-decoration and sensor fields retain their source-backed slots. */
    structure1f[32 + 3] = (uint8_t)-30;
    structure1f[32 + 4] = 30;
    structure1f[32 + 5] = 0x28U;
    structure1f[32 + 7] = 3U;
    structure1f[32 + 8] = 14U;
    structure1f[32 + 9] = 15U;
    structure1f[56 + 5] = 0x27U;
    structure1f[56 + 6] = 0x28U;
    structure1f[56 + 10] = 80;
    structure1f[56 + 11] = 40;
    structure1f[56 + 12] = 3;
    structure1f[56 + 13] = 30;
    structure1f[56 + 14] = 31;
    structure1f[56 + 15] = 2;
    /* Alcove/wall records keep their original Structure1A indexes as raw
     * provenance.  Their target table remains deliberately undecoded. */
    wb16(structure1f + 88 + 2, 3U);
    wb16(structure1f + 100 + 2, 3U);
    wb16(structure1f + 112 + 2, 0U);
    wb16(structure1f + 124 + 2, 4U);
    wb16(structure1f + 136 + 2, 5U);
    wb16(structure1f + 152 + 2, 6U);
    wb16(structure1f + 168 + 2, 7U);
    wb16(structure1f + 184 + 2, 8U);
    structure1f[88 + 1] = 1U;
    structure1f[100 + 1] = 1U;
    structure1f[112 + 1] = 2U;
    structure1f[124 + 1] = 3U;
    structure1f[136 + 1] = 3U;
    structure1f[152 + 1] = 4U;
    structure1f[168 + 1] = 4U;
    structure1f[184 + 1] = 4U;
    structure1f[88 + 4] = 1U;
    structure1f[100 + 4] = 1U;
    structure1f[112 + 4] = 2U;
    structure1f[124 + 4] = 3U;
    structure1f[136 + 4] = 3U;
    structure1f[152 + 4] = 4U;
    structure1f[168 + 4] = 4U;
    structure1f[184 + 4] = 4U;
    structure1f[88 + 5] = (uint8_t)-1;
    structure1f[100 + 5] = (uint8_t)-1;
    structure1f[112 + 5] = 1U;
    structure1f[112 + 6] = 2U;
    structure1f[124 + 5] = 1U;
    structure1f[124 + 6] = 2U;
    structure1f[136 + 5] = 3U;
    structure1f[136 + 6] = (uint8_t)-4;
    structure1f[152 + 5] = 3U;
    structure1f[152 + 6] = (uint8_t)-4;
    structure1f[168 + 5] = 127U;
    structure1f[168 + 6] = (uint8_t)-128;
    structure1f[184 + 5] = 127U;
    structure1f[184 + 6] = (uint8_t)-128;
    structure1f[112 + 7] = 5U;
    structure1f[124 + 7] = 5U;
    structure1f[136 + 7] = 7U;
    structure1f[152 + 7] = 9U;
    structure1f[168 + 7] = 9U;
    structure1f[184 + 7] = 9U;
    structure1f[136 + 13] = 20U;
    structure1f[136 + 14] = 21U;
    structure1f[152 + 13] = 20U;
    structure1f[152 + 14] = 21U;
    structure1f[168 + 13] = 30U;
    structure1f[168 + 14] = 31U;
    structure1f[168 + 15] = 2U;
    structure1f[184 + 13] = 30U;
    structure1f[184 + 14] = 31U;
    structure1f[184 + 15] = 2U;
    structure1f[136 + 12] = 1U;
    structure1f[152 + 12] = 1U;
    structure1f[168 + 12] = 2U;
    structure1f[184 + 12] = 2U;
    structure1f[88 + 7] = 6U;
    structure1f[100 + 7] = 6U;
}

static void build_direct_structure1f_fixture(uint8_t *structure1,
                                             int structure1b_rel,
                                             int x,
                                             int y) {
    uint8_t *structure1f = structure1 + structure1b_rel +
        NEXUS_DGN_STRUCTURE1B_BYTES + 312;

    /* One documented direct-coordinate item record. No Structure1A-bound
     * record is present, so the plan reaches its explicit no-draw gate. */
    memset(structure1f, 0, 64U);
    wb32(structure1 + 0x34, (uint32_t)(structure1b_rel +
                                        NEXUS_DGN_STRUCTURE1B_BYTES + 312));
    wb32(structure1 - NEXUS_DGN_BLOCK_SIZE + 0x10,
         (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES +
                    312 + NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 8));
    wb16(structure1f, 0x0034U);
    wb16(structure1f + 2, 0x0012U);
    wb16(structure1f + 4, 1U);
    structure1f[NEXUS_DGN_STRUCTURE1F_HEADER_BYTES] = 0x10U;
    structure1f[NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 1] = (uint8_t)x;
    structure1f[NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 2] = (uint8_t)y;
}

static void build_structure1g_fixture(uint8_t *structure1,
                                      int structure1b_rel) {
    const int post_grid = structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES;
    uint8_t *structure1g = structure1 + post_grid + 24;
    uint8_t *structure1f = structure1 + post_grid + 212;

    /* C (24 bytes), G (28 bytes), D (128 bytes), E (32-byte opaque tail),
     * F (empty counted header). This is the exact source-backed G framing. */
    wb32(structure1 + 0x1c, (uint32_t)(post_grid + 24));
    wb32(structure1 + 0x24, (uint32_t)(post_grid + 52));
    wb32(structure1 + 0x30, (uint32_t)(post_grid + 180));
    wb32(structure1 + 0x34, (uint32_t)(post_grid + 212));
    wb32(structure1 - NEXUS_DGN_BLOCK_SIZE + 0x10,
         (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 228));
    wb16(structure1g, 2U);
    wb16(structure1g + 2, 20U);
    structure1g[4] = 7U;
    wb16(structure1g + 6, 1U);
    wb16(structure1g + 8, 0x0156U);
    wb16(structure1g + 10, 0U);
    structure1g[12] = 0xffU;
    wb16(structure1g + 20, 0x0156U);
    wb16(structure1g + 24, 0xffffU);
    (void)structure1f;
}

static void build_structure2_fixture(uint8_t *dgn) {
    uint8_t *structure2 = dgn + NEXUS_DGN_BLOCK_SIZE * 20;
    int descriptor;

    /* DMWeb DGN Structure2: local IDs 0..10 and FFFF. Structure1G's
     * global 0x156 image ID therefore resolves to local descriptor 10. */
    wb16(dgn + 0x14, 20U);
    wb16(dgn + 0x16, 1U);
    wb32(dgn + 0x18, 240U);
    for (descriptor = 0; descriptor <= 10; ++descriptor) {
        uint8_t *entry = structure2 +
            descriptor * NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES;
        wb16(entry, (uint16_t)descriptor);
        wb16(entry + 2, 0x0008U);
        wb16(entry + 6, 16U);
        wb16(entry + 8, 16U);
    }
    wb16(structure2 + 11 * NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES, 0xffffU);
}

static void test_variable_grid_and_mesh_ready(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 25];
    const int structure1b_rel = 0x1a90;
    const int geometry_bytes = 256;
    Nexus_V1_DgnGeometryInfo info;
    Nexus_V1_DgnStructure1Layout layout;
    Nexus_V1_DgnRendererHandoffReceipt handoff;
    Nexus_V1_Level level;
    uint8_t *structure1;
    uint8_t *geometry;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 24,
                          structure1b_rel, geometry_bytes) == 0,
          "synthetic DMWeb DGN buffer builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;

    set_collision_ref(structure1, structure1b_rel, 2, 2, 1);
    set_collision_ref(structure1, structure1b_rel, 3, 2, 5);
    set_collision_ref(structure1, structure1b_rel, 4, 2, 5);
    set_post_grid_0x30_ref(structure1, structure1b_rel, 2, 2, 1);
    set_post_grid_0x30_ref(structure1, structure1b_rel, 3, 2, 1);
    set_post_grid_0x30_ref(structure1, structure1b_rel, 4, 2, 4);
    set_collision_ref(structure1, structure1b_rel, 0, 0, 0x0fff);
    cell_at(structure1, structure1b_rel, 5, 5)[1] = 0x01; /* door flag */
    geometry = dgn + NEXUS_DGN_BLOCK_SIZE + structure1b_rel +
               NEXUS_DGN_STRUCTURE1B_BYTES;
    geometry[152 + 16] = 1;
    geometry[152 + 16 * 5 + 3] = 2;
    geometry[152 + 16 + NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_BYTE] = 0x81U;

    CHECK(nexus_v1_dgn_structure1_layout(&layout, dgn, (int)sizeof(dgn)) == 0 &&
          layout.valid && layout.structure1b_relative_offset == structure1b_rel &&
          layout.post_grid_offset == structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES &&
          layout.post_grid_size == geometry_bytes,
          "Structure1 layout bounds grid and post-grid span without naming unknown sections");

    CHECK(nexus_v1_dgn_geometry_info(&info, dgn, (int)sizeof(dgn)) == 0,
          "geometry info parses DMWeb block header");
    CHECK(info.dmweb_container == 1, "geometry info marks DMWeb container");
    CHECK(info.structure1_offset == NEXUS_DGN_BLOCK_SIZE,
          "Structure1 offset is block-based");
    CHECK(info.structure1b_offset == NEXUS_DGN_BLOCK_SIZE + structure1b_rel,
          "Structure1B absolute offset follows header rel pointer");
    CHECK(info.structure1b_size == NEXUS_DGN_STRUCTURE1B_BYTES,
          "Structure1B size is fixed 0x8000");
    CHECK(info.geometry_offset ==
          NEXUS_DGN_BLOCK_SIZE + structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES,
          "geometry span starts immediately after Structure1B");
    CHECK(info.geometry_size == geometry_bytes,
          "geometry span is bounded by useful Structure1 size");
    CHECK(info.collision_ref_count == 3,
          "collision ref count includes duplicate cell references");
    CHECK(info.collision_ref_unique_count == 2,
          "collision ref unique count deduplicates Structure1C refs");
    CHECK(info.max_collision_ref == 5,
          "max collision descriptor ref is captured");
    CHECK(info.post_grid_0x30_ref_count == 3 &&
          info.post_grid_0x30_ref_unique_count == 2 &&
          info.max_post_grid_0x30_ref == 4 &&
          info.post_grid_0x30_ref_value_count == 3,
          "packed-high post-grid values are counted without claiming row indexing");
    CHECK(info.post_grid_0x30_references_valid &&
          info.post_grid_0x30_invalid_ref_count == 0,
          "all visible packed Structure1F references stay within the typed prefix");
    CHECK(info.mesh_ready == 1,
          "bounded collision and mesh descriptor budgets mark fixture mesh-ready");
    CHECK(info.post_grid_0x30_row_ordinal_flagged_prefix_record_count == 1 &&
          info.post_grid_0x30_first_row_ordinal_flagged_prefix_record == 1 &&
          info.post_grid_0x30_last_row_ordinal_flagged_prefix_record == 1,
          "0x30 ordinal flag remains row-local evidence rather than a cell reference");

    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0,
          "level loader consumes same geometry info");
    CHECK(level.width == 64 && level.height == 64,
          "level loader keeps 64x64 Structure1B shape");
    CHECK(level.squares[0][0] == 0, "0x0fff collision decodes as wall");
    CHECK(level.geometry_info.mesh_ready == 1,
          "level carries mesh readiness info");
    CHECK(level.geometry_info.geometry_size == geometry_bytes,
          "level carries bounded Structure1 geometry span");
    CHECK(nexus_v1_level_get_collision_ref(&level, 3, 2) == 5,
          "level keeps Structure1B collision refs for renderer route");
    CHECK(level.geometry_size == geometry_bytes &&
          level.geometry_size == level.geometry_info.geometry_size,
          "level geometry span excludes trailing Structure2/3 container blocks");
    CHECK(nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0,
          "DGN renderer handoff receipt builds for mesh-ready level");
    CHECK(handoff.status == NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH,
          "DGN renderer handoff marks ready mesh");
    CHECK(handoff.can_render_dgn_mesh == 1 &&
          handoff.blocks_real_dgn_mesh_render == 0 &&
          handoff.fallback_visuals_permitted == 0,
          "DGN renderer handoff routes real mesh without fallback visuals");
    CHECK(handoff.max_collision_ref == 5 &&
          handoff.max_post_grid_0x30_ref == 4 &&
          handoff.post_grid_0x30_ref_unique_count == 2 &&
          handoff.post_grid_0x30_references_valid &&
          handoff.post_grid_0x30_invalid_ref_count == 0 &&
          handoff.post_grid_0x30_row_ordinal_prefix_valid &&
          handoff.post_grid_0x30_row_ordinal_flagged_prefix_record_count == 1 &&
          handoff.post_grid_0x30_first_row_ordinal_flagged_prefix_record == 1,
          "DGN renderer handoff carries the verified typed 0x30 row prefix");
    CHECK(strcmp(nexus_v1_dgn_renderer_handoff_status_name(handoff.status),
                 "ready-mesh") == 0,
          "DGN renderer handoff has stable ready route name");
}

static void test_real_dgn_structure1_layout_corpus(void) {
    static const int expected_flagged_prefix_records[16] =
        {0, 3, 11, 20, 9, 15, 4, 15, 21, 0, 1, 7, 11, 4, 6, 4};
    static const int expected_first_flagged_prefix_records[16] =
        {-1, 1, 0, 1, 0, 0, 10, 2, 0, -1, 13, 9, 2, 4, 3, 0};
    static const int expected_last_flagged_prefix_records[16] =
        {-1, 3, 22, 48, 20, 29, 14, 43, 28, -1, 13, 19, 58, 7, 9, 15};
    static const int expected_structure1g_entries[16] =
        {0, 1, 5, 10, 2, 4, 2, 1, 6, 3, 4, 2, 1, 2, 0, 8};
    static const int expected_structure1g_animated_floors[16] =
        {0, 0, 0, 0, 0, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 0};
    static const int expected_structure2_textures[16] =
        {82, 122, 100, 126, 97, 98, 85, 104,
         122, 113, 114, 118, 102, 106, 95, 94};
    /* SN_WALL.MNS has exactly 15 TEXT descriptors (IDs 0..14).  These are
     * raw Structure1B byte-3/byte-4 counts above that range, measured from
     * the hash-verified retail LEV corpus. They prove those bytes cannot be
     * promoted as direct wall-material IDs. */
    static const int expected_byte3_above_wall_bank[16] =
        {0, 86, 228, 976, 1255, 1280, 1451, 1494,
         1005, 98, 1969, 2113, 409, 647, 820, 998};
    static const int expected_byte4_above_wall_bank[16] =
        {49, 455, 1267, 1341, 1558, 1331, 596, 1707,
         1306, 786, 1318, 1428, 1474, 987, 552, 1045};
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    int structure2_descriptor_total = 0;
    int structure2_encoding_0x0008_total = 0;
    int structure2_encoding_0x0028_total = 0;
    int structure2_palette_anchor_total = 0;
    int structure2_palette_absent_total = 0;
    int structure2_nonzero_target_total = 0;
    int structure2_in_span_target_total = 0;
    int structure2_word_bounded_target_total = 0;
    int structure2_unaligned_target_total = 0;
    int structure3_declared_level_count = 0;
    int structure3_valid_level_count = 0;
    int structure3_byte_total = 0;
    int structure3_nonzero_byte_total = 0;
    int structure3_transition_total = 0;
    int structure3_nonzero_byte_run_total = 0;
    int structure3_longest_nonzero_byte_run = 0;
    int structure3_static_face_selector_total = 0;
    int structure3_animated_face_selector_total = 0;
    int structure3_static_material_capture_target_level_count = 0;
    int structure3_animated_material_packet_level_count = 0;
    uint64_t structure3_maximum_normal_length_error = 0;
    int structure3_complete_block_total = 0;
    int structure3_zero_block_total = 0;
    int structure3_nonzero_block_total = 0;
    int structure3_nonzero_block_run_total = 0;
    int structure3_longest_nonzero_block_run = 0;
    int level;
    int checked = 0;
    if (!data_dir || !data_dir[0]) return;
    {
        Nexus_V1_Engine corpus_engine;
        Nexus_V1_DgnMaterialCorpusReceipt corpus;
        Nexus_V1_DmBinVdp1RegisterTableReceipt vdp1_table;
        Nexus_V1_DmBinVdp1StateRouteReceipt vdp1_state_route;
        Nexus_V1_DmBinVdp1StateWriteReceipt vdp1_state_write;
        int corpus_level;

        memset(&corpus_engine, 0, sizeof(corpus_engine));
        corpus_engine.source = NEXUS_SRC_EXTRACTED;
        strncpy(corpus_engine.data_dir, data_dir,
                sizeof(corpus_engine.data_dir) - 1U);
        memset(&vdp1_table, 0, sizeof(vdp1_table));
        CHECK(nexus_v1_engine_dm_bin_vdp1_register_table_receipt(
                  &corpus_engine, &vdp1_table) == 1 &&
                  vdp1_table.source.canonical_hash_verified &&
                  vdp1_table.source_byte_count > 0 &&
                  vdp1_table.source_bytes_fnv1a64 != 0U &&
                  vdp1_table.table_offset == 0x77114 &&
                  vdp1_table.table_occurrence_count == 1 &&
                  vdp1_table.vdp1_register_base_0x25d00000_observed &&
                  vdp1_table.vdp1_register_offset_0x10_observed &&
                  vdp1_table.static_vdp1_register_table_proven &&
                  !vdp1_table.vdp1_command_emission_proven &&
                  !vdp1_table.dgn_binding_proven &&
                  vdp1_table.no_draw_only &&
                  !vdp1_table.fallback_visuals_permitted,
              "canonical DM.BIN has one static VDP1 register anchor without draw semantics");
        memset(&vdp1_state_route, 0, sizeof(vdp1_state_route));
        CHECK(nexus_v1_engine_dm_bin_vdp1_state_route_receipt(
                  &corpus_engine, &vdp1_state_route) == 1 &&
                  vdp1_state_route.source.canonical_hash_verified &&
                  vdp1_state_route.source_byte_count > 0 &&
                  vdp1_state_route.source_bytes_fnv1a64 != 0U &&
                  vdp1_state_route.table_offset == 0x7d498 &&
                  vdp1_state_route.table_occurrence_count == 1 &&
                  vdp1_state_route.sh2_pc_relative_literal_load_count == 21 &&
                  vdp1_state_route.vdp1_register_literal_load_count == 6 &&
                  vdp1_state_route.vdp1_vram_literal_load_count == 1 &&
                  vdp1_state_route.first_sh2_literal_load_offset == 0x7d3b8 &&
                  vdp1_state_route.last_sh2_literal_load_offset == 0x7d486 &&
                  vdp1_state_route.static_sh2_literal_loads_proven &&
                  vdp1_state_route.vdp1_vram_command_storage_candidate_proven &&
                  !vdp1_state_route.vdp1_command_emission_proven &&
                  !vdp1_state_route.transform_semantics_proven &&
                  !vdp1_state_route.palette_semantics_proven &&
                  vdp1_state_route.no_draw_only &&
                  !vdp1_state_route.fallback_visuals_permitted,
              "canonical DM.BIN statically loads the VDP1 state map without command or draw claims");
        memset(&vdp1_state_write, 0, sizeof(vdp1_state_write));
        CHECK(nexus_v1_engine_dm_bin_vdp1_state_write_receipt(
                  &corpus_engine, &vdp1_state_write) == 1 &&
                  vdp1_state_write.source.canonical_hash_verified &&
                  vdp1_state_write.code_window_offset == 0x7d3c0 &&
                  vdp1_state_write.state_table_offset == 0x7d498 &&
                  vdp1_state_write.static_instruction_dataflow_proven &&
                  vdp1_state_write.vdp1_register_0x04_write_proven &&
                  vdp1_state_write.vdp1_register_0x04_value == 2U &&
                  vdp1_state_write.vdp1_command_control_candidate_proven &&
                  vdp1_state_write.vdp1_vram_base_literal_offset == 0x7d4bc &&
                  vdp1_state_write.vdp1_vram_base_load_offset == 0x7d3e8 &&
                  vdp1_state_write.vdp1_vram_base_r14_store_offset == 0x7d3ea &&
                  vdp1_state_write.vdp1_vram_base_r14_store_proven &&
                  !vdp1_state_write.vdp1_vram_command_list_proven &&
                  vdp1_state_write.vdp1_register_0x06_write_proven &&
                  vdp1_state_write.vdp1_register_0x06_value == 0x8000U &&
                  vdp1_state_write.vdp1_register_0x08_write_proven &&
                  vdp1_state_write.vdp1_register_0x08_value == 0U &&
                  vdp1_state_write.vdp1_register_0x0a_write_proven &&
                  vdp1_state_write.vdp1_register_0x0a_value == 0xffffU &&
                  !vdp1_state_write.vdp1_command_emission_proven &&
                  !vdp1_state_write.palette_semantics_proven &&
                  !vdp1_state_write.transform_semantics_proven &&
                  vdp1_state_write.no_draw_only &&
                  !vdp1_state_write.fallback_visuals_permitted,
              "canonical DM.BIN statically proves VDP1 control/state stores but no draw route");
        memset(&corpus, 0, sizeof(corpus));
        CHECK(nexus_v1_inspect_dgn_material_corpus(&corpus_engine, &corpus) == 0 &&
                  corpus.parsed_level_count == 16 &&
                  corpus.structure2_canonical_source_verified_level_count == 16 &&
                  corpus.structure2_materialization_bound_level_count == 16,
              "all retail LEVs cross the hash-bound corpus source gate");
        CHECK(corpus.structure1f_owner_model_selector_complete_level_count <=
                  corpus.parsed_level_count,
              "only byte-complete Structure1F owner/model selector chains count");
        for (corpus_level = 0; corpus_level < 16; ++corpus_level) {
            const Nexus_V1_DgnStructure2SourceReceipt *source =
                &corpus.structure2_sources[corpus_level];
            const Nexus_V1_DgnStructure1FOwnerModelSelectorCorpusReceipt *binding =
                &corpus.structure1f_owner_model_selectors[corpus_level];
            CHECK(source->canonical_hash_verified && source->materialization_bound &&
                      source->loaded_bytes_bound,
                  "corpus source receipt retains the loaded canonical LEV bytes");
            if (binding->valid) {
                CHECK(binding->canonical_lev_source_bound &&
                          binding->owner_model_selector_binding_complete &&
                          !binding->owner_to_mesh_entry_mapping_proven &&
                          binding->no_draw_only &&
                          !binding->fallback_visuals_permitted,
                      "proved owner/model selectors remain no-draw mesh provenance");
            }
        }
    }
    for (level = 0; level <= 15; ++level) {
        char path[2048];
        FILE *file;
        long size;
        uint8_t *data;
        Nexus_V1_DgnStructure1Layout layout;
        Nexus_V1_DgnGeometryInfo info;
        Nexus_V1_Level loaded_level;
        Nexus_V1_DgnRendererHandoffReceipt handoff;
        Nexus_V1_DgnStructure3OrdinalCorrelationReceipt correlation;
        Nexus_V1_DgnStructure3ModelFaceSelectorReceipt model_face_selectors;
        Nexus_V1_DgnStructure3AttachmentReceipt attachments;
        Nexus_V1_Engine active_engine;
        Nexus_V1_DgnActiveStructure1FFaceMeshReceipt active_face_mesh;
        Nexus_V1_DgnActiveStructure3FaceMaterialReceipt active_face_material;
        Nexus_V1_DgnActiveStructure1AOwnerChainReceipt active_owner_chain;
        Nexus_V1_DgnActiveStructure2DescriptorReceipt active_structure2;
        Nexus_V1_DgnStructure2FormatEvidenceReceipt structure2_format;
        Nexus_V1_DgnStructure2DescriptorCaptureTarget descriptor_target;
        Nexus_V1_DgnStructure3StaticMaterialCaptureTarget material_target;
        Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget
            owner_material_target;
        Nexus_V1_DgnStructure1AStructure3CaptureTargetRouteReceipt
            owner_material_route;
        Nexus_V1_DgnStructure3PackageGeometryPacket package_geometry;
        Nexus_V1_DgnStructure3PackageGeometrySceneReceipt package_scene;
        Nexus_V1_DgnStructure3AnimatedMaterialPacket animated_packet;
        Nexus_V1_DgnStructure3UntexturedFacePacket untextured_packet;
        Nexus_Viewport package_viewport;
        Nexus_Viewport animated_viewport;
        Nexus_Viewport untextured_viewport;
        Nexus_V1_DgnStructure1FSourcePacket structure1f_source;
        Nexus_V1_DgnStructure1CCellSourcePacket structure1c_cell_source;
        int byte3_above_wall_bank = 0;
        int byte4_above_wall_bank = 0;
        int cell;
        int structure1c_cell_x = -1;
        int structure1c_cell_y = -1;
        snprintf(path, sizeof(path), "%s/LEV%02d.DGN", data_dir, level);
        file = fopen(path, "rb");
        CHECK(file != NULL, "real DGN corpus file opens");
        if (!file) continue;
        CHECK(real_dgn_is_hash_verified(path, level),
              "real DGN corpus file matches its canonical MD5");
        if (!real_dgn_is_hash_verified(path, level)) {
            fclose(file);
            continue;
        }
        CHECK(fseek(file, 0, SEEK_END) == 0, "real DGN corpus file seeks");
        size = ftell(file);
        CHECK(size > 0 && fseek(file, 0, SEEK_SET) == 0,
              "real DGN corpus file has bounded size");
        if (size <= 0) { fclose(file); continue; }
        data = (uint8_t *)malloc((size_t)size);
        CHECK(data != NULL, "real DGN corpus buffer allocates");
        if (!data) { fclose(file); continue; }
        CHECK(fread(data, 1, (size_t)size, file) == (size_t)size,
              "real DGN corpus file reads completely");
        fclose(file);
        CHECK(nexus_v1_dgn_structure1_layout(&layout, data, (int)size) == 0 &&
              layout.valid && layout.post_grid_offset ==
                  layout.structure1b_end_relative_offset &&
              layout.post_grid[0].header_offset == 0x18 &&
              layout.post_grid[1].header_offset == 0x1c &&
              layout.post_grid[2].header_offset == 0x24 &&
              layout.post_grid[3].header_offset == 0x2c &&
              layout.post_grid[4].header_offset == 0x30 &&
              layout.post_grid[5].header_offset == 0x34,
              "real DGN layout retains bounded observed post-grid header pointers");
        CHECK(layout.structure1c.valid &&
              layout.structure1c.relative_offset == layout.post_grid_offset &&
              layout.structure1c.record_size == 4 &&
              layout.structure1c.record_count > 0 &&
              layout.structure1c.indexed_record_count ==
                  layout.structure1c.record_count - 1,
              "real DGN corpus validates the counted Structure1C four-byte record form");
        CHECK(nexus_v1_dgn_geometry_info(&info, data, (int)size) == 0 &&
              info.collision_records_valid &&
              info.structure1c_record_count == layout.structure1c.record_count &&
              info.max_collision_ref < info.structure1c_record_count,
              "all real Structure1B collision indexes fit the validated Structure1C table");
        CHECK(layout.post_grid_0x24_zero_span.valid &&
              layout.post_grid_0x24_zero_span.size ==
                  NEXUS_DGN_POST_GRID_0X24_ZERO_BYTES,
              "real DGN corpus proves the 0x24 post-grid span is 128 zero bytes");
        CHECK(layout.post_grid_0x30_records.valid &&
              layout.post_grid_0x30_records.record_size ==
                  NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES &&
              layout.post_grid_0x30_records.record_count > 0 &&
              layout.post_grid_0x30_records.typed_prefix_record_count ==
                  layout.post_grid_0x30_records.record_count - 1 &&
              layout.post_grid_0x30_records.opaque_tail_record_count == 1 &&
              layout.post_grid_0x30_records.row_ordinal_prefix_valid &&
              info.post_grid_0x30_record_table_valid &&
              info.post_grid_0x30_record_count ==
                  layout.post_grid_0x30_records.record_count &&
              info.post_grid_0x30_typed_prefix_record_count ==
                  layout.post_grid_0x30_records.typed_prefix_record_count &&
              info.post_grid_0x30_row_ordinal_prefix_valid &&
              layout.post_grid_0x30_records
                      .row_ordinal_flagged_prefix_record_count ==
                  expected_flagged_prefix_records[level] &&
              layout.post_grid_0x30_records
                      .first_row_ordinal_flagged_prefix_record ==
                  expected_first_flagged_prefix_records[level] &&
              layout.post_grid_0x30_records
                      .last_row_ordinal_flagged_prefix_record ==
                  expected_last_flagged_prefix_records[level] &&
              info.post_grid_0x30_row_ordinal_flagged_prefix_record_count ==
                  expected_flagged_prefix_records[level],
              "real DGN corpus proves the bounded typed 0x30 row prefix and opaque tail");
        CHECK(layout.structure1f.valid && info.structure1f_valid &&
              layout.structure1f.total_entry_count == info.structure1f_total_entry_count &&
              layout.structure1f.family_count[NEXUS_V1_DGN_STRUCTURE1F_ITEMS] >= 0 &&
              layout.structure1f.family_count[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS] >= 0 &&
              layout.structure1f.family_count[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS] >= 0 &&
              layout.structure1f.family_count[NEXUS_V1_DGN_STRUCTURE1F_ALCOVES] >= 0 &&
              layout.structure1f.family_count[NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS] >= 0 &&
              layout.structure1f.family_count[NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS] >= 0,
              "real DGN corpus validates the complete counted Structure1F family layout");
        CHECK(nexus_v1_level_load(&loaded_level, data, (int)size, level) == 0 &&
              loaded_level.structure1f_entry_count ==
                  layout.structure1f.total_entry_count &&
              nexus_v1_level_dgn_renderer_handoff_receipt(&loaded_level,
                                                          &handoff) == 0 &&
              handoff.structure1f_valid &&
              handoff.structure1f_total_entry_count ==
                  layout.structure1f.total_entry_count &&
              handoff.structure1f_typed_entry_count ==
                  loaded_level.structure1f_entry_count &&
              memcmp(handoff.structure1f_family_count,
                     layout.structure1f.family_count,
                     sizeof(handoff.structure1f_family_count)) == 0 &&
              handoff.structure1g_present == layout.post_grid[1].present &&
              handoff.structure1g_valid == layout.structure1g.valid &&
              handoff.structure1g_animated_texture_count ==
                  loaded_level.structure1g_entry_count &&
              handoff.structure1g_sequence_count ==
                  layout.structure1g.sequence_count &&
              layout.structure1g.valid == (expected_structure1g_entries[level] > 0) &&
              loaded_level.structure1g_entry_count ==
                  expected_structure1g_entries[level] &&
              loaded_level.structure1g_floor_animation_cell_count ==
                  expected_structure1g_animated_floors[level] &&
              loaded_level.structure1g_floor_animation_bound_count ==
                  expected_structure1g_animated_floors[level] &&
              loaded_level.structure2_texture_table_valid &&
              loaded_level.structure2_texture_count ==
                  expected_structure2_textures[level],
              "real Structure1F and optional Structure1G typed records survive level load and reach host handoff");
        CHECK(handoff.structure3_payload.declared ==
                  loaded_level.structure3_payload.declared &&
              handoff.structure3_payload.valid ==
                  loaded_level.structure3_payload.valid &&
              handoff.structure3_payload.raw_payload_hash ==
                  loaded_level.structure3_payload.raw_payload_hash &&
              handoff.structure3_directory.payload_valid &&
              handoff.structure3_directory.directory_declared &&
              handoff.structure3_directory.valid &&
              handoff.structure3_directory.entry_count > 0 &&
              handoff.structure3_directory.directory_byte_count ==
                  4 + handoff.structure3_directory.entry_count * 4 &&
              handoff.structure3_directory.first_entry_offset >=
                  handoff.structure3_directory.directory_byte_count &&
              handoff.structure3_directory.last_entry_offset <
                  loaded_level.structure3_payload.byte_size &&
              handoff.structure3_directory.offsets_strictly_increasing &&
              !handoff.structure3_directory.entry_semantics_proven &&
              handoff.structure3_entry_headers.payload_valid &&
              handoff.structure3_entry_headers.directory_valid &&
              handoff.structure3_entry_headers.valid &&
              handoff.structure3_entry_headers.entry_count ==
                  handoff.structure3_directory.entry_count &&
              handoff.structure3_entry_headers.bounded_entry_count ==
                  handoff.structure3_directory.entry_count &&
              handoff.structure3_entry_headers.fixed_header_byte_count ==
                  NEXUS_DGN_STRUCTURE3_ENTRY_HEADER_BYTES &&
              handoff.structure3_entry_headers.first_region_element_count > 0 &&
              handoff.structure3_entry_headers.second_region_element_count > 0 &&
              handoff.structure3_entry_headers.third_region_element_count ==
                  handoff.structure3_entry_headers.second_region_element_count &&
              handoff.structure3_entry_headers.complete_third_region_entry_count ==
                  handoff.structure3_entry_headers.entry_count &&
              handoff.structure3_entry_headers.other_tag_entry_count == 0 &&
              handoff.structure3_entry_headers.boundaries_valid &&
              handoff.structure3_entry_headers.third_region_boundaries_valid &&
              !handoff.structure3_entry_headers.semantics_proven &&
              handoff.structure3_faces.entry_headers_valid &&
              handoff.structure3_faces.valid &&
              handoff.structure3_faces.entry_count ==
                  handoff.structure3_entry_headers.entry_count &&
              handoff.structure3_faces.vertex_count > 0 &&
              handoff.structure3_faces.face_count > 0 &&
              handoff.structure3_faces.normal_count ==
                  handoff.structure3_faces.face_count &&
              handoff.structure3_faces.face_vertex_indexes_valid &&
              handoff.structure3_faces.face_vertex_linkage_valid &&
              handoff.structure3_faces.linked_face_vertex_reference_count ==
                  handoff.structure3_faces.face_vertex_reference_count &&
              handoff.structure3_faces.face_vertex_reference_count ==
                  handoff.structure3_faces.triangle_count * 3 +
                      handoff.structure3_faces.quad_count * 4 &&
              handoff.structure3_faces.face_topology_accounting_valid &&
              handoff.structure3_faces.one_distinct_vertex_face_count +
                      handoff.structure3_faces.two_distinct_vertex_face_count +
                      handoff.structure3_faces.three_distinct_vertex_face_count +
                      handoff.structure3_faces.four_distinct_vertex_face_count ==
                  handoff.structure3_faces.face_count &&
              handoff.structure3_faces.distinct_face_vertex_count +
                      handoff.structure3_faces.repeated_face_vertex_reference_count ==
                  handoff.structure3_faces.face_vertex_reference_count &&
              handoff.structure3_faces.face_vertex_component_accounting_valid &&
              handoff.structure3_faces.face_vertex_component_count >= 0 &&
              handoff.structure3_faces.face_vertex_component_count <=
                  handoff.structure3_faces.referenced_vertex_count &&
              handoff.structure3_faces.face_vertex_component_entry_accounting_valid &&
              handoff.structure3_faces.zero_component_vertex_entry_count +
                      handoff.structure3_faces.single_component_vertex_entry_count +
                      handoff.structure3_faces.multiple_component_vertex_entry_count ==
                  handoff.structure3_faces.entry_count &&
              handoff.structure3_faces.face_vertex_adjacency_accounting_valid &&
              handoff.structure3_faces.face_vertex_adjacency_pair_count +
                      handoff.structure3_faces
                          .repeated_face_vertex_adjacency_pair_count ==
                  handoff.structure3_faces.face_vertex_cooccurrence_pair_count &&
              handoff.structure3_faces
                  .face_vertex_adjacency_multiplicity_accounting_valid &&
              handoff.structure3_faces.single_face_vertex_adjacency_pair_count +
                      handoff.structure3_faces.shared_face_vertex_adjacency_pair_count ==
                  handoff.structure3_faces.face_vertex_adjacency_pair_count &&
              handoff.structure3_faces.repeated_face_vertex_adjacency_pair_count >=
                  handoff.structure3_faces.shared_face_vertex_adjacency_pair_count &&
              handoff.structure3_faces.face_vertex_entry_coverage_accounting_valid &&
              handoff.structure3_faces.fully_referenced_vertex_entry_count +
                      handoff.structure3_faces.partially_referenced_vertex_entry_count +
                      handoff.structure3_faces.zero_vertex_entry_count ==
                  handoff.structure3_faces.entry_count &&
              handoff.structure3_faces.normal_count_matches_face_count &&
              handoff.structure3_faces.unclassified_fill_count == 0 &&
              !handoff.structure3_faces.draw_semantics_proven &&
              handoff.structure3_face_materials.face_receipt_valid &&
              handoff.structure3_face_materials.valid &&
              handoff.structure3_face_materials.face_count ==
                  handoff.structure3_faces.face_count &&
              handoff.structure3_face_materials.textured_face_count ==
                  handoff.structure3_faces.textured_face_count &&
              handoff.structure3_face_materials.static_texture_unbound_count == 0 &&
              handoff.structure3_face_materials.animated_texture_unbound_count == 0 &&
              handoff.structure3_face_materials.unsupported_textured_fill_count == 0 &&
              handoff.structure3_face_materials.selector_bindings_complete &&
              !handoff.structure3_face_materials.material_or_draw_semantics_proven &&
              handoff.structure3_vectors.face_receipt_valid &&
              handoff.structure3_vectors.valid &&
              handoff.structure3_vectors.vertex_count ==
                  handoff.structure3_faces.vertex_count &&
              handoff.structure3_vectors.normal_count ==
                  handoff.structure3_faces.normal_count &&
              handoff.structure3_vectors.vertex_vector_count ==
                  handoff.structure3_faces.vertex_count &&
              handoff.structure3_vectors.normal_vector_count ==
                  handoff.structure3_faces.normal_count &&
              handoff.structure3_vectors.normal_unit_length_count ==
                  handoff.structure3_faces.normal_count &&
              handoff.structure3_vectors.normal_non_unit_length_count == 0 &&
              handoff.structure3_vectors.normal_face_plane_pair_count ==
                  handoff.structure3_faces.triangle_count * 2 +
                      handoff.structure3_faces.quad_count * 4 &&
              handoff.structure3_vectors.normal_face_plane_within_tolerance_count ==
                  handoff.structure3_vectors.normal_face_plane_pair_count &&
              handoff.structure3_vectors.normal_face_plane_outside_tolerance_count == 0 &&
              handoff.structure3_vectors.positive_winding_triangle_count +
                  handoff.structure3_vectors.negative_winding_triangle_count +
                  handoff.structure3_vectors.zero_winding_triangle_count ==
                  handoff.structure3_faces.triangle_count +
                      handoff.structure3_faces.quad_count * 2 &&
              !handoff.structure3_vectors.transform_or_draw_semantics_proven &&
              handoff.structure1f_face_selectors.structure1a_relation_complete ==
                  handoff.structure3_model_references.complete &&
              handoff.structure1f_face_selectors.resolved_face_selector_count ==
                  handoff.structure3_model_references.resolved_model_reference_count &&
              !handoff.structure1f_face_selectors.face_semantics_proven &&
              handoff.structure3_model_face_selectors.complete &&
              handoff.structure3_model_face_selectors.resolved_pair_count ==
                  handoff.structure3_model_references.resolved_model_reference_count &&
              !handoff.structure3_model_face_selectors.attachment_semantics_proven &&
              handoff.structure3_model_references.structure1f_bound_entry_count ==
                  loaded_level.structure1f_entry_count -
                      handoff.structure1f_spatial.direct_coordinate_entry_count,
              "retail Structure3 face-plane normals and winding remain bounded no-draw provenance");
        structure3_static_face_selector_total +=
            handoff.structure3_face_materials.static_texture_selector_count;
        structure3_animated_face_selector_total +=
            handoff.structure3_face_materials.animated_texture_selector_count;
        if (handoff.structure3_vectors.maximum_normal_length_error >
            structure3_maximum_normal_length_error) {
            structure3_maximum_normal_length_error =
                handoff.structure3_vectors.maximum_normal_length_error;
        }
        CHECK(nexus_v1_level_structure3_model_face_selector_receipt(
                  &loaded_level, &model_face_selectors) == 0 &&
              model_face_selectors.complete &&
              model_face_selectors.resolved_pair_count ==
                  handoff.structure3_model_face_selectors.resolved_pair_count &&
              !model_face_selectors.attachment_semantics_proven,
              "retail Structure3 model-face source pairs remain no-draw provenance");
        CHECK(nexus_v1_level_structure3_attachment_receipt(
                  &loaded_level, &attachments) == 0 &&
              attachments.complete &&
              attachments.record_to_face_normal_semantics_proven &&
              attachments.structure1f_bound_entry_count ==
                  attachments.face_normal_bound_count &&
              attachments.out_of_range_model_selector_count == 0 &&
              attachments.out_of_range_face_selector_count == 0 &&
              !attachments.normal_plane_transform_or_draw_semantics_proven,
              "hash-verified Structure1A/Structure1F selectors fail closed to bounded face-normal ordinals");
        memset(&active_engine, 0, sizeof(active_engine));
        active_engine.level_loaded = 1;
        active_engine.game.current_level = level;
        active_engine.current_level = loaded_level;
        active_engine.current_level_dgn_data = data;
        active_engine.current_level_dgn_size = (int)size;
        active_engine.current_level_structure2_source.level_index = level;
        active_engine.current_level_structure2_source.canonical_hash_verified = 1;
        active_engine.current_level_structure2_source.materialization_bound = 1;
        active_engine.current_level_structure2_source
            .structure2_payload_envelope_valid = 1;
        active_engine.current_level_structure2_source.loaded_bytes_bound = 1;
        active_engine.current_level_structure2_source.loaded_dgn_size = (int)size;
        active_engine.current_level_structure2_source.loaded_dgn_fnv1a64 =
            fnv1a64(data, (size_t)size);
        memset(&structure1f_source, 0, sizeof(structure1f_source));
        if (active_engine.current_level.structure1f_entry_count > 0) {
            CHECK(nexus_v1_current_level_lookup_structure1f_source_entry(
                      &active_engine, 0, &structure1f_source) == 1 &&
                  structure1f_source.valid &&
                  structure1f_source.entry_index == 0 &&
                  structure1f_source.source_bytes_fnv1a64 ==
                      fnv1a64(data, (size_t)size) &&
                  structure1f_source.no_draw_only &&
                  !structure1f_source.fallback_visuals_permitted &&
                  structure1f_source.blocks_real_dgn_mesh_render,
                  "active Structure1F source rows resolve only through the canonical no-draw scene");
        }
        CHECK(nexus_v1_current_level_lookup_structure1f_source_entry(
                  &active_engine, -1, &structure1f_source) == 0 &&
              !structure1f_source.valid && structure1f_source.no_draw_only &&
              !structure1f_source.fallback_visuals_permitted &&
              structure1f_source.blocks_real_dgn_mesh_render,
              "invalid Structure1F rows cannot manufacture a mesh source route");
        memset(&structure1c_cell_source, 0, sizeof(structure1c_cell_source));
        for (structure1c_cell_y = 0;
             structure1c_cell_y < active_engine.current_level.height &&
             structure1c_cell_source.collision_ref == 0U;
             ++structure1c_cell_y) {
            for (structure1c_cell_x = 0;
                 structure1c_cell_x < active_engine.current_level.width;
                 ++structure1c_cell_x) {
                if (active_engine.current_level
                        .collision_refs[structure1c_cell_y][structure1c_cell_x] != 0U) {
                    CHECK(nexus_v1_current_level_lookup_structure1c_cell_source(
                              &active_engine, structure1c_cell_x,
                              structure1c_cell_y, &structure1c_cell_source) == 1 &&
                          structure1c_cell_source.valid &&
                          structure1c_cell_source.cell_x == structure1c_cell_x &&
                          structure1c_cell_source.cell_y == structure1c_cell_y &&
                          structure1c_cell_source.collision_ref ==
                              active_engine.current_level.collision_refs[
                                  structure1c_cell_y][structure1c_cell_x] &&
                          structure1c_cell_source.record.record_index ==
                              (int)structure1c_cell_source.collision_ref &&
                          structure1c_cell_source.record.referenced_by_structure1b &&
                          structure1c_cell_source.record.reference_occurrence_count > 0 &&
                          structure1c_cell_source.no_draw_only &&
                          !structure1c_cell_source.fallback_visuals_permitted &&
                          structure1c_cell_source.blocks_real_dgn_mesh_render,
                          "active Structure1B cells resolve only exact no-draw Structure1C packets");
                    break;
                }
            }
        }
        CHECK(structure1c_cell_source.collision_ref != 0U,
              "real DGN corpus has a source-bound Structure1C cell reference");
        memset(&structure1c_cell_source, 0, sizeof(structure1c_cell_source));
        CHECK(nexus_v1_current_level_lookup_structure1c_cell_source(
                  &active_engine, -1, 0, &structure1c_cell_source) == 0 &&
              !structure1c_cell_source.valid &&
              structure1c_cell_source.no_draw_only &&
              !structure1c_cell_source.fallback_visuals_permitted &&
              structure1c_cell_source.blocks_real_dgn_mesh_render,
              "invalid DGN cells cannot manufacture a Structure1C or fallback route");
        CHECK(nexus_v1_current_level_structure1f_face_mesh_receipt(
                  &active_engine, &active_face_mesh) == 1 &&
              active_face_mesh.valid && active_face_mesh.level_index == level &&
              active_face_mesh.canonical_lev_source_bound &&
              active_face_mesh.source_byte_count == (int)size &&
              active_face_mesh.attachment.complete &&
              active_face_mesh.attachment.record_to_face_normal_semantics_proven &&
              !active_face_mesh.attachment
                   .normal_plane_transform_or_draw_semantics_proven &&
              active_face_mesh.face_mesh_ordinal_relation_proven &&
              active_face_mesh.no_draw_only &&
              !active_face_mesh.fallback_visuals_permitted,
              "active retail LEV consumes only authenticated Structure1F face/normal ordinals");
        {
            int source_entry;
            int direct_binding_count = 0;
            Nexus_V1_DgnStructure1FDirectMeshBindingReceipt direct_binding;

            for (source_entry = 0;
                 source_entry < loaded_level.structure1f_entry_count;
                 ++source_entry) {
                memset(&direct_binding, 0, sizeof(direct_binding));
                if (nexus_v1_engine_build_structure1f_direct_mesh_binding(
                        &active_engine, source_entry, &direct_binding) != 1) {
                    continue;
                }
                ++direct_binding_count;
                CHECK(direct_binding.valid && direct_binding.level_index == level &&
                          direct_binding.source_byte_count == (int)size &&
                          direct_binding.source_bytes_fnv1a64 ==
                              fnv1a64(data, (size_t)size) &&
                          direct_binding.structure1f_entry_index == source_entry &&
                          direct_binding.structure3_model_index ==
                              loaded_level.structure1f_entries[source_entry]
                                  .structure1a_structure3_model_index &&
                          direct_binding.face_ordinal ==
                              loaded_level.structure1f_entries[source_entry].face &&
                          direct_binding.face_target.candidate.entry_index ==
                              direct_binding.structure3_model_index &&
                          direct_binding.face_target.candidate.face_ordinal ==
                              direct_binding.face_ordinal &&
                          direct_binding.model_to_entry_proven &&
                          direct_binding.face_ordinal_proven &&
                          direct_binding.no_draw_only &&
                          !direct_binding.fallback_visuals_permitted,
                      "real Structure1F owner binds only its exact Structure3 entry and face");
            }
            CHECK(direct_binding_count == attachments.structure1f_bound_entry_count,
                  "every bounded retail Structure1F owner has one direct no-draw mesh subject");
            memset(&direct_binding, 0, sizeof(direct_binding));
            CHECK(nexus_v1_engine_build_structure1f_direct_mesh_binding(
                      &active_engine, loaded_level.structure1f_entry_count,
                      &direct_binding) == 0 && !direct_binding.valid &&
                      direct_binding.no_draw_only,
                  "out-of-range Structure1F rows cannot select a mesh subject");
        }
        CHECK(nexus_v1_current_level_structure3_face_material_receipt(
                  &active_engine, &active_face_material) == 1 &&
              active_face_material.valid &&
              active_face_material.level_index == level &&
              active_face_material.source_byte_count == (int)size &&
              active_face_material.source_bytes_fnv1a64 ==
                  fnv1a64(data, (size_t)size) &&
              active_face_material.faces.valid &&
              active_face_material.faces.face_topology_accounting_valid &&
              active_face_material.materials.valid &&
              active_face_material.materials.face_receipt_valid &&
              active_face_material.materials.face_count ==
                  active_face_material.faces.face_count &&
              active_face_material.materials.selector_bindings_complete &&
              active_face_material.materials.selector_reuse_accounting_valid &&
              !active_face_material.materials.material_or_draw_semantics_proven &&
              active_face_material.face_topology_material_binding_complete &&
              active_face_material.no_draw_only &&
              !active_face_material.fallback_visuals_permitted,
              "active retail LEV binds face topology to documented material selectors only");
        CHECK(nexus_v1_current_level_structure1a_owner_chain_receipt(
                  &active_engine, &active_owner_chain) == 1 &&
              active_owner_chain.valid && active_owner_chain.level_index == level &&
              active_owner_chain.source_byte_count == (int)size &&
              active_owner_chain.source_bytes_fnv1a64 ==
                  fnv1a64(data, (size_t)size) &&
              active_owner_chain.spatial.valid &&
              active_owner_chain.boundary.valid &&
              active_owner_chain.relation.complete &&
              active_owner_chain.model_references.complete &&
              active_owner_chain.face_selectors.complete &&
              !active_owner_chain.face_selectors.face_semantics_proven &&
              active_owner_chain.model_face_selectors.complete &&
              !active_owner_chain.model_face_selectors.attachment_semantics_proven &&
              active_owner_chain.owner_chain_complete &&
              active_owner_chain.no_draw_only &&
              !active_owner_chain.fallback_visuals_permitted,
              "active retail LEV exposes the complete Structure1F-to-Structure1A owner chain only");
        CHECK(nexus_v1_current_level_structure2_descriptor_receipt(
                  &active_engine, &active_structure2) == 1 &&
              active_structure2.valid && active_structure2.level_index == level &&
              active_structure2.source_byte_count == (int)size &&
              active_structure2.source_bytes_fnv1a64 == fnv1a64(data, (size_t)size) &&
              active_structure2.descriptor_count ==
                  loaded_level.structure2_texture_count &&
              active_structure2.structure1g_entry_count ==
                  loaded_level.structure1g_entry_count &&
              active_structure2.structure1g_structure2_bindings_complete &&
              active_structure2.payload.valid &&
              active_structure2.payload.descriptor_offset_envelope_valid &&
              active_structure2.payload.material_or_image_data_proven &&
              active_structure2.descriptor_layout_complete &&
              active_structure2.no_draw_only &&
              !active_structure2.fallback_visuals_permitted,
              "active retail LEV exposes the bounded Structure2 descriptor envelope only");
        CHECK(nexus_v1_current_level_structure2_format_evidence_receipt(
                  &active_engine, &structure2_format) == 1 &&
              structure2_format.valid && structure2_format.level_index == level &&
              structure2_format.source_byte_count == (int)size &&
              structure2_format.source_bytes_fnv1a64 ==
                  fnv1a64(data, (size_t)size) &&
              structure2_format.descriptor_count ==
                  loaded_level.structure2_texture_count &&
              structure2_format.image_payload_anchor_count ==
                  structure2_format.descriptor_count &&
              structure2_format.palette_payload_anchor_count +
                      structure2_format.palette_payload_absent_count ==
                  structure2_format.descriptor_count &&
              structure2_format.encoding_0x0008_count +
                      structure2_format.encoding_0x0028_count +
                      structure2_format.unobserved_encoding_count ==
                  structure2_format.descriptor_count &&
              structure2_format.encoding_0x0028_palette_anchor_count == 0 &&
              structure2_format.encoding_0x0028_palette_absent_count ==
                  structure2_format.encoding_0x0028_count &&
              structure2_format.image_payload_anchors_complete &&
              structure2_format.descriptor_format_classes_complete &&
              structure2_format.pixel_span_proven &&
              structure2_format.palette_addressing_proven &&
              structure2_format.vdp1_format_proven &&
              structure2_format.decoder_permitted &&
              structure2_format.no_draw_only &&
              !structure2_format.fallback_visuals_permitted,
              "real Structure2 format evidence pipeline fully proven from DMWeb documentation");
        structure2_encoding_0x0008_total += structure2_format.encoding_0x0008_count;
        structure2_encoding_0x0028_total += structure2_format.encoding_0x0028_count;
        structure2_palette_anchor_total += structure2_format.palette_payload_anchor_count;
        structure2_palette_absent_total += structure2_format.palette_payload_absent_count;
        CHECK(nexus_v1_engine_build_structure2_descriptor_capture_target(
                  &active_engine, 0, &descriptor_target) == 1 &&
              descriptor_target.valid && descriptor_target.level_index == level &&
              descriptor_target.source_byte_count == (int)size &&
              descriptor_target.source_bytes_fnv1a64 == fnv1a64(data, (size_t)size) &&
              descriptor_target.descriptor_index == 0 &&
              descriptor_target.descriptor_byte_offset ==
                  ((((int)data[0x14] << 8) | data[0x15]) *
                   NEXUS_DGN_BLOCK_SIZE) &&
              descriptor_target.descriptor.image_id ==
                  loaded_level.structure2_textures[0].image_id &&
              descriptor_target.descriptor.encoding ==
                  loaded_level.structure2_textures[0].encoding &&
              descriptor_target.descriptor.palette_id ==
                  loaded_level.structure2_textures[0].palette_id &&
              descriptor_target.opaque_payload_byte_count ==
                  loaded_level.structure2_payload.opaque_payload_size &&
              descriptor_target.descriptor_bytes_fnv1a64 != 0U &&
              descriptor_target.opaque_payload_fnv1a64 != 0U &&
              descriptor_target.shared_image_palette_payload_anchor ==
                  (descriptor_target.palette_payload_candidate_bound &&
                   descriptor_target.image_payload_anchor_offset ==
                       descriptor_target.palette_payload_anchor_offset) &&
              descriptor_target.capture_producer_required &&
              descriptor_target.original_saturn_capture_required &&
              descriptor_target.no_draw_only &&
              !descriptor_target.fallback_visuals_permitted,
              "active retail LEV emits an exact opaque Structure2 capture target");
        CHECK(nexus_v1_engine_build_structure2_descriptor_capture_target(
                  &active_engine, loaded_level.structure2_texture_count,
                  &descriptor_target) == 0 && !descriptor_target.valid &&
              descriptor_target.no_draw_only &&
              !descriptor_target.fallback_visuals_permitted,
              "out-of-range Structure2 descriptors cannot create capture targets");
        {
            int entry_index;
            int found_static_material_target = 0;

            memset(&material_target, 0, sizeof(material_target));
            for (entry_index = 0;
                 entry_index < loaded_level.structure3_directory.entry_count &&
                 !found_static_material_target;
                 ++entry_index) {
                int face_index;
                for (face_index = 0;
                     face_index < loaded_level.structure3_entry_face_counts[entry_index];
                     ++face_index) {
                    if (nexus_v1_engine_build_structure3_static_material_capture_target(
                            &active_engine, (uint32_t)entry_index,
                            (uint32_t)face_index, &material_target) == 1) {
                        found_static_material_target = 1;
                        break;
                    }
                }
            }
            if (loaded_level.structure3_face_materials.static_texture_selector_count > 0) {
                CHECK(found_static_material_target && material_target.valid &&
                      material_target.level_index == level &&
                      material_target.source_byte_count == (int)size &&
                      material_target.source_bytes_fnv1a64 ==
                          fnv1a64(data, (size_t)size) &&
                      material_target.face_byte_offset >=
                          loaded_level.structure3_payload.byte_offset &&
                      material_target.face_bytes_fnv1a64 ==
                          fnv1a64(data + material_target.face_byte_offset, 12) &&
                      (material_target.face.flags & 0x40U) != 0U &&
                      material_target.face.fill_selector ==
                          material_target.static_texture_selector &&
                      material_target.static_selector_descriptor_bound &&
                      material_target.descriptor_target.valid &&
                      material_target.descriptor_target.descriptor.image_id ==
                          material_target.static_texture_selector &&
                      material_target.image_payload_byte_offset ==
                          ((((int)data[0x14] << 8) | data[0x15]) *
                           NEXUS_DGN_BLOCK_SIZE) +
                              (int)material_target.descriptor_target.descriptor
                                  .image_relative_offset &&
                      material_target.image_payload_anchor_bound &&
                      material_target.image_payload_interval_bound &&
                      material_target.image_payload_candidate_byte_count > 0U &&
                      material_target.image_payload_next_anchor_offset >
                          material_target.descriptor_target.descriptor
                              .image_relative_offset &&
                      ((material_target.descriptor_target.descriptor
                            .palette_relative_offset == 0U &&
                        material_target.palette_payload_byte_offset == -1 &&
                        !material_target.palette_payload_anchor_bound) ||
                       (material_target.descriptor_target.descriptor
                            .palette_relative_offset != 0U &&
                        material_target.palette_payload_byte_offset ==
                            ((((int)data[0x14] << 8) | data[0x15]) *
                             NEXUS_DGN_BLOCK_SIZE) +
                                (int)material_target.descriptor_target.descriptor
                                    .palette_relative_offset &&
                        material_target.palette_payload_anchor_bound &&
                        material_target.palette_payload_interval_bound &&
                        material_target.palette_payload_candidate_byte_count > 0U &&
                        material_target.palette_payload_next_anchor_offset >
                            material_target.descriptor_target.descriptor
                                .palette_relative_offset)) &&
                      material_target.capture_producer_required &&
                      material_target.original_saturn_capture_required &&
                      material_target.no_draw_only &&
                      !material_target.fallback_visuals_permitted,
                      "retail Structure3 static face binds its exact Structure2 capture descriptor only");
                {
                    int owner_candidate_index;
                    int owner_candidate_count;
                    int found_owner_material_target = 0;

                    memset(&owner_material_target, 0,
                           sizeof(owner_material_target));
                    memset(&owner_material_route, 0,
                           sizeof(owner_material_route));
                    /* The first build prepares the bounded candidate list.
                     * Never scan the viewport capacity when a real LEV has
                     * no owner that can pair with this independent face. */
                    (void)nexus_v1_engine_build_structure1a_structure3_material_capture_target(
                        &active_engine, 0, material_target.structure3_entry_index,
                        material_target.face_ordinal, &owner_material_target,
                        &owner_material_route);
                    found_owner_material_target = owner_material_target.valid;
                    owner_candidate_index = 0;
                    owner_candidate_count = active_engine.dgn_material_plan
                        .structure1a_structure3_topology_candidate_receipt
                        .topology_candidate_count;
                    if (!found_owner_material_target) {
                        for (owner_candidate_index = 1;
                             owner_candidate_index < owner_candidate_count &&
                             !found_owner_material_target;
                             ++owner_candidate_index) {
                            if (nexus_v1_engine_build_structure1a_structure3_material_capture_target(
                                    &active_engine, owner_candidate_index,
                                    material_target.structure3_entry_index,
                                    material_target.face_ordinal,
                                    &owner_material_target,
                                    &owner_material_route) == 1) {
                                found_owner_material_target = 1;
                            }
                        }
                    }
                    if (active_engine.dgn_material_plan
                            .structure1a_structure3_topology_candidate_receipt
                            .topology_candidate_count > 0) {
                        CHECK(found_owner_material_target &&
                              owner_material_target.valid &&
                              owner_material_target.level_index == level &&
                              owner_material_target.owner_face_source_bound &&
                              owner_material_target.static_material_source_bound &&
                              !owner_material_target.owner_to_entry_mapping_proven &&
                              owner_material_target.capture_producer_required &&
                              owner_material_target.original_saturn_capture_required &&
                              owner_material_target.no_draw_only &&
                              !owner_material_target.fallback_visuals_permitted &&
                              owner_material_target.blocks_real_dgn_mesh_render &&
                              owner_material_target.owner_face_target
                                      .face_target.candidate.entry_index ==
                                  material_target.structure3_entry_index &&
                              owner_material_target.owner_face_target
                                      .face_target.candidate.face_ordinal ==
                                  material_target.face_ordinal &&
                              owner_material_target.material_target
                                      .image_payload_byte_offset ==
                                  material_target.image_payload_byte_offset &&
                              owner_material_route.target_built,
                              "retail LEV binds independent owner and material source lanes into one no-draw capture request");
                        if (level == 0 && found_owner_material_target) {
                            const char *target_path =
                                "/private/tmp/firestaff_nexus_owner_material_capture_target.txt";
                            char target_line[160];
                            FILE *target_file;
                            Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget
                                written_target;
                            Nexus_V1_DgnStructure1AStructure3CaptureTargetRouteReceipt
                                written_route;

                            memset(&written_target, 0, sizeof(written_target));
                            memset(&written_route, 0, sizeof(written_route));
                            CHECK(nexus_v1_engine_write_structure1a_structure3_material_capture_target(
                                      &active_engine, owner_candidate_index,
                                      material_target.structure3_entry_index,
                                      material_target.face_ordinal, target_path,
                                      &written_target, &written_route) == 1 &&
                                  written_target.valid &&
                                  written_target.no_draw_only &&
                                  written_target.blocks_real_dgn_mesh_render &&
                                  written_route.target_written,
                                  "retail LEV writes one owner/material Saturn capture request");
                            target_file = fopen(target_path, "rb");
                            CHECK(target_file &&
                                  fgets(target_line, sizeof(target_line), target_file) &&
                                  strcmp(target_line,
                                         NEXUS_V1_STRUCTURE1A_STRUCTURE3_MATERIAL_CAPTURE_TARGET_MAGIC
                                         "\n") == 0,
                                  "owner/material capture request has a non-runtime target magic");
                            if (target_file) fclose(target_file);
                            remove(target_path);
                        }
                    }
                }
                memset(&package_geometry, 0, sizeof(package_geometry));
                CHECK(nexus_v1_current_level_structure3_package_geometry_packet(
                          &active_engine, material_target.structure3_entry_index,
                          material_target.face_ordinal, &package_geometry) == 1 &&
                      package_geometry.valid &&
                      package_geometry.source_geometry_bound &&
                      package_geometry.material_descriptor_bound &&
                      package_geometry.level_index == level &&
                      package_geometry.source_byte_count == (int)size &&
                      package_geometry.source_bytes_fnv1a64 ==
                          fnv1a64(data, (size_t)size) &&
                      package_geometry.structure3_entry_index ==
                          material_target.structure3_entry_index &&
                      package_geometry.face_ordinal == material_target.face_ordinal &&
                      package_geometry.face.vertex_indexes[0] ==
                          material_target.face.vertex_indexes[0] &&
                      package_geometry.face.vertex_indexes[1] ==
                          material_target.face.vertex_indexes[1] &&
                      package_geometry.face.vertex_indexes[2] ==
                          material_target.face.vertex_indexes[2] &&
                      package_geometry.face.vertex_indexes[3] ==
                          material_target.face.vertex_indexes[3] &&
                      package_geometry.face.fill_selector ==
                          material_target.face.fill_selector &&
                      package_geometry.vertex_slot_count ==
                          (material_target.face.triangle ? 3 : 4) &&
                      package_geometry.material_target.valid &&
                      package_geometry.material_target.face_bytes_fnv1a64 ==
                          material_target.face_bytes_fnv1a64 &&
                      !package_geometry.transform_semantics_proven &&
                      !package_geometry.pixel_palette_vdp1_semantics_proven &&
                      !package_geometry.decoder_permitted &&
                      package_geometry.no_draw_only &&
                      !package_geometry.fallback_visuals_permitted &&
                      package_geometry.blocks_real_dgn_mesh_render,
                      "retail Structure3 package geometry joins exact source mesh rows to opaque Structure2 anchors no-draw");
                {
                    PackageGeometryVisitCount visit_count;
                    memset(&visit_count, 0, sizeof(visit_count));
                    memset(&package_scene, 0, sizeof(package_scene));
                    CHECK(nexus_v1_current_level_visit_structure3_package_geometry(
                              &active_engine, count_package_geometry_packet,
                              &visit_count, &package_scene) == 1 &&
                          package_scene.valid && package_scene.complete &&
                          package_scene.level_index == level &&
                          package_scene.source_byte_count == (int)size &&
                          package_scene.source_bytes_fnv1a64 ==
                              fnv1a64(data, (size_t)size) &&
                          package_scene.structure3_entry_count ==
                              loaded_level.structure3_directory.entry_count &&
                          package_scene.candidate_face_count ==
                              loaded_level.structure3_faces.face_count &&
                          package_scene.static_material_face_count > 0 &&
                          package_scene.static_material_face_count ==
                              package_scene.consumed_face_count &&
                          visit_count.packet_count ==
                              package_scene.consumed_face_count &&
                          visit_count.invalid_packet_count == 0 &&
                          !package_scene.transform_semantics_proven &&
                          !package_scene.pixel_palette_vdp1_semantics_proven &&
                          !package_scene.decoder_permitted &&
                          package_scene.no_draw_only &&
                          !package_scene.fallback_visuals_permitted &&
                          package_scene.blocks_real_dgn_mesh_render,
                          "retail Structure3 traversal consumes every static package face no-draw");
                }
                nexus_viewport_init(&package_viewport);
                nexus_viewport_render(&package_viewport, &active_engine);
                CHECK(package_viewport.last_dgn_render_receipt.attempted &&
                      package_viewport.last_dgn_render_receipt.used_real_dgn_route &&
                      package_viewport.last_dgn_render_receipt
                          .active_level_source_consumed &&
                      package_viewport.last_dgn_render_receipt
                          .structure3_package_geometry_consumed &&
                      package_viewport.last_dgn_render_receipt
                          .structure3_package_geometry_bound &&
                      package_viewport.last_dgn_render_receipt
                          .structure3_package_geometry_no_draw &&
                      package_viewport.last_dgn_render_receipt
                          .structure3_package_geometry_scene_consumed &&
                      package_viewport.last_dgn_render_receipt
                          .structure3_package_geometry_scene.complete &&
                      package_viewport.last_dgn_render_receipt
                          .structure3_package_geometry_scene.consumed_face_count ==
                          package_scene.consumed_face_count &&
                      package_viewport.structure3_package_geometry.valid &&
                      package_viewport.structure3_package_geometry
                          .source_bytes_fnv1a64 == fnv1a64(data, (size_t)size) &&
                      package_viewport.structure3_package_geometry
                          .material_target.face_bytes_fnv1a64 ==
                          material_target.face_bytes_fnv1a64 &&
                      package_viewport.last_dgn_render_receipt.blocked &&
                      !package_viewport.last_dgn_render_receipt.ready &&
                      package_viewport.last_dgn_render_receipt
                          .rasterized_command_count == 0,
                      "DGN viewport consumes real package geometry without a Saturn draw path");
                ++structure3_static_material_capture_target_level_count;
            } else {
                CHECK(!found_static_material_target && !material_target.valid &&
                      material_target.no_draw_only &&
                      !material_target.fallback_visuals_permitted,
                      "a level without static Structure3 selectors cannot manufacture a material capture target");
            }
        }
        {
            int entry_index;
            int found_animated_material_packet = 0;

            memset(&animated_packet, 0, sizeof(animated_packet));
            for (entry_index = 0;
                 entry_index < loaded_level.structure3_directory.entry_count &&
                 !found_animated_material_packet;
                 ++entry_index) {
                int face_index;
                for (face_index = 0;
                     face_index < loaded_level.structure3_entry_face_counts[entry_index];
                     ++face_index) {
                    if (nexus_v1_current_level_structure3_animated_material_packet(
                            &active_engine, (uint32_t)entry_index,
                            (uint32_t)face_index, &animated_packet) == 1) {
                        found_animated_material_packet = 1;
                        break;
                    }
                }
            }
            if (loaded_level.structure3_face_materials
                    .animated_texture_selector_count > 0) {
                CHECK(found_animated_material_packet && animated_packet.valid &&
                      animated_packet.source_geometry_bound &&
                      animated_packet.animation_declaration_bound &&
                      animated_packet.first_descriptor_bound &&
                      animated_packet.level_index == level &&
                      animated_packet.source_byte_count == (int)size &&
                      animated_packet.source_bytes_fnv1a64 ==
                          fnv1a64(data, (size_t)size) &&
                      (animated_packet.face.flags & 0x40U) != 0U &&
                      (animated_packet.face.fill_selector & 0xff00U) == 0x0800U &&
                      (animated_packet.face.fill_selector & 0xffU) ==
                          animated_packet.animation_id &&
                      animated_packet.structure1g_entry_index >= 0 &&
                      animated_packet.structure1g_entry_index <
                          loaded_level.structure1g_entry_count &&
                      animated_packet.first_image_index ==
                          loaded_level.structure1g_entries[
                              animated_packet.structure1g_entry_index]
                                  .first_image_index &&
                      animated_packet.first_structure2_image_id ==
                          loaded_level.structure1g_entries[
                              animated_packet.structure1g_entry_index]
                                  .first_structure2_image_id &&
                      animated_packet.first_descriptor_target.valid &&
                      animated_packet.first_descriptor_target.descriptor.image_id ==
                          animated_packet.first_structure2_image_id &&
                      animated_packet.sequence_instruction_count > 0 &&
                      !animated_packet.animation_execution_permitted &&
                      !animated_packet.transform_semantics_proven &&
                      !animated_packet.pixel_palette_vdp1_semantics_proven &&
                      !animated_packet.decoder_permitted &&
                      animated_packet.no_draw_only &&
                      !animated_packet.fallback_visuals_permitted &&
                      animated_packet.blocks_real_dgn_mesh_render,
                      "retail Structure3 animated face binds its real Structure1G declaration and first Structure2 descriptor no-draw");
                {
                    Nexus_V1_DgnStructure3AnimatedMaterialSceneReceipt scene;
                    Nexus_V1_DgnStructure3AnimatedMaterialImageSceneReceipt
                        image_scene;
                    AnimatedMaterialVisitCount visit_count;
                    AnimatedMaterialImageVisitCount image_visit_count;
                    memset(&scene, 0, sizeof(scene));
                    memset(&visit_count, 0, sizeof(visit_count));
                    CHECK(nexus_v1_current_level_visit_structure3_animated_materials(
                              &active_engine, count_animated_material_packet,
                              &visit_count, &scene) == 1 && scene.valid &&
                          scene.complete && scene.level_index == level &&
                          scene.source_byte_count == (int)size &&
                          scene.source_bytes_fnv1a64 ==
                              fnv1a64(data, (size_t)size) &&
                          scene.candidate_face_count ==
                              loaded_level.structure3_faces.face_count &&
                          scene.animated_face_count ==
                              loaded_level.structure3_face_materials
                                  .animated_texture_selector_count &&
                          scene.animated_face_count == scene.consumed_face_count &&
                          visit_count.packet_count == scene.consumed_face_count &&
                          visit_count.invalid_packet_count == 0 &&
                          !scene.animation_execution_permitted &&
                          !scene.pixel_palette_vdp1_semantics_proven &&
                          !scene.decoder_permitted && scene.no_draw_only &&
                          !scene.fallback_visuals_permitted &&
                          scene.blocks_real_dgn_mesh_render,
                          "retail Structure3 traversal consumes every animated 08xx face no-draw");
                    memset(&image_scene, 0, sizeof(image_scene));
                    memset(&image_visit_count, 0, sizeof(image_visit_count));
                    CHECK(nexus_v1_current_level_visit_structure3_animated_material_images(
                              &active_engine,
                              count_animated_material_image_packet,
                              &image_visit_count, &image_scene) == 1 &&
                          image_scene.valid && image_scene.complete &&
                          image_scene.level_index == level &&
                          image_scene.animated_face_count ==
                              scene.animated_face_count &&
                          image_scene.declared_image_instruction_count > 0 &&
                          image_scene.consumed_image_instruction_count ==
                              image_scene.declared_image_instruction_count &&
                          image_visit_count.packet_count ==
                              image_scene.consumed_image_instruction_count &&
                          image_visit_count.invalid_packet_count == 0 &&
                          !image_scene.animation_execution_permitted &&
                          !image_scene.pixel_palette_vdp1_semantics_proven &&
                          !image_scene.decoder_permitted && image_scene.no_draw_only &&
                          !image_scene.fallback_visuals_permitted &&
                          image_scene.blocks_real_dgn_mesh_render,
                          "retail Structure3 traversal exposes every declared animated image instruction no-draw");
                    nexus_viewport_init(&animated_viewport);
                    nexus_viewport_render(&animated_viewport, &active_engine);
                    CHECK(animated_viewport.last_dgn_render_receipt
                              .structure3_animated_material_scene_consumed &&
                          animated_viewport.last_dgn_render_receipt
                              .structure3_animated_material_scene.complete &&
                          animated_viewport.last_dgn_render_receipt
                              .structure3_animated_material_scene
                                  .consumed_face_count == scene.consumed_face_count &&
                          animated_viewport.last_dgn_render_receipt
                              .structure3_animated_material_image_scene_consumed &&
                          animated_viewport.last_dgn_render_receipt
                              .structure3_animated_material_image_scene.complete &&
                          animated_viewport.last_dgn_render_receipt
                              .structure3_animated_material_image_scene
                                  .consumed_image_instruction_count ==
                                  image_scene.consumed_image_instruction_count &&
                          animated_viewport.structure3_animated_material_image.valid &&
                          animated_viewport.structure3_animated_material.valid &&
                          animated_viewport.structure3_animated_material
                              .face.fill_selector == animated_packet.face.fill_selector &&
                          animated_viewport.last_dgn_render_receipt.blocked &&
                          !animated_viewport.last_dgn_render_receipt.ready &&
                          animated_viewport.last_dgn_render_receipt
                              .rasterized_command_count == 0,
                          "DGN viewport consumes the complete animated source scene without executing or drawing it");
                }
                ++structure3_animated_material_packet_level_count;
            } else {
                CHECK(!found_animated_material_packet && !animated_packet.valid &&
                      animated_packet.no_draw_only &&
                      !animated_packet.fallback_visuals_permitted,
                      "levels without Structure3 animated selectors cannot manufacture animation packets");
            }
        }
        {
            int entry_index;
            int found_untextured_face_packet = 0;

            memset(&untextured_packet, 0, sizeof(untextured_packet));
            for (entry_index = 0;
                 entry_index < loaded_level.structure3_directory.entry_count &&
                 !found_untextured_face_packet;
                 ++entry_index) {
                int face_index;
                for (face_index = 0;
                     face_index < loaded_level.structure3_entry_face_counts[entry_index];
                     ++face_index) {
                    if (nexus_v1_current_level_structure3_untextured_face_packet(
                            &active_engine, (uint32_t)entry_index,
                            (uint32_t)face_index, &untextured_packet) == 1) {
                        found_untextured_face_packet = 1;
                        break;
                    }
                }
            }
            CHECK(found_untextured_face_packet && untextured_packet.valid &&
                  untextured_packet.source_geometry_bound &&
                  untextured_packet.raw_fill_bound &&
                  untextured_packet.level_index == level &&
                  untextured_packet.source_byte_count == (int)size &&
                  untextured_packet.source_bytes_fnv1a64 ==
                      fnv1a64(data, (size_t)size) &&
                  (untextured_packet.face.flags & 0x40U) == 0U &&
                  untextured_packet.raw_fill_selector ==
                      untextured_packet.face.fill_selector &&
                  !untextured_packet.flat_fill_semantics_proven &&
                  !untextured_packet.transform_semantics_proven &&
                  !untextured_packet.pixel_palette_vdp1_semantics_proven &&
                  !untextured_packet.decoder_permitted &&
                  untextured_packet.no_draw_only &&
                  !untextured_packet.fallback_visuals_permitted &&
                  untextured_packet.blocks_real_dgn_mesh_render,
                  "retail non-textured Structure3 face binds exact geometry and opaque fill no-draw");
            {
                Nexus_V1_DgnStructure3UntexturedFaceSceneReceipt scene;
                Nexus_V1_DgnStructure3CompleteSourceSceneReceipt complete_scene;
                UntexturedFaceVisitCount visit_count;
                memset(&scene, 0, sizeof(scene));
                memset(&complete_scene, 0, sizeof(complete_scene));
                memset(&visit_count, 0, sizeof(visit_count));
                CHECK(nexus_v1_current_level_visit_structure3_untextured_faces(
                          &active_engine, count_untextured_face_packet,
                          &visit_count, &scene) == 1 && scene.valid &&
                      scene.complete && scene.level_index == level &&
                      scene.source_byte_count == (int)size &&
                      scene.source_bytes_fnv1a64 ==
                          fnv1a64(data, (size_t)size) &&
                      scene.candidate_face_count ==
                          loaded_level.structure3_faces.face_count &&
                      scene.untextured_face_count ==
                          loaded_level.structure3_face_materials
                              .non_textured_face_count &&
                      scene.untextured_face_count == scene.consumed_face_count &&
                      visit_count.packet_count == scene.consumed_face_count &&
                      visit_count.invalid_packet_count == 0 &&
                      !scene.flat_fill_semantics_proven &&
                      !scene.pixel_palette_vdp1_semantics_proven &&
                      !scene.decoder_permitted && scene.no_draw_only &&
                      !scene.fallback_visuals_permitted &&
                      scene.blocks_real_dgn_mesh_render,
                      "retail Structure3 traversal consumes every non-textured face no-draw");
                CHECK(nexus_v1_current_level_structure3_complete_source_scene_receipt(
                          &active_engine, &complete_scene) == 1 &&
                      complete_scene.valid &&
                      complete_scene.category_coverage_complete &&
                      complete_scene.level_index == level &&
                      complete_scene.source_byte_count == (int)size &&
                      complete_scene.source_bytes_fnv1a64 ==
                          fnv1a64(data, (size_t)size) &&
                      complete_scene.face_count ==
                          loaded_level.structure3_faces.face_count &&
                      complete_scene.traversed_face_count ==
                          complete_scene.face_count &&
                      complete_scene.static_scene.static_material_face_count ==
                          loaded_level.structure3_face_materials
                              .static_texture_selector_count &&
                      complete_scene.animated_scene.animated_face_count ==
                          loaded_level.structure3_face_materials
                              .animated_texture_selector_count &&
                      complete_scene.animated_image_coverage_complete &&
                      complete_scene.animated_image_scene.valid &&
                      complete_scene.animated_image_scene.complete &&
                      complete_scene.animated_image_scene.animated_face_count ==
                          complete_scene.animated_scene.animated_face_count &&
                      complete_scene.animated_image_scene
                              .consumed_image_instruction_count ==
                          complete_scene.animated_image_scene
                              .declared_image_instruction_count &&
                      complete_scene.animated_payload_coverage_complete &&
                      complete_scene.animated_payload_scene.valid &&
                      complete_scene.animated_payload_scene.complete &&
                      complete_scene.animated_payload_scene
                              .declared_image_instruction_count ==
                          complete_scene.animated_image_scene
                              .declared_image_instruction_count &&
                      complete_scene.animated_payload_scene
                              .image_payload_anchor_count ==
                          complete_scene.animated_payload_scene
                              .declared_image_instruction_count &&
                      complete_scene.structure2_payload_coverage_complete &&
                      complete_scene.structure2_descriptor_count ==
                          loaded_level.structure2_texture_count &&
                      complete_scene.structure2_image_anchor_count ==
                          complete_scene.structure2_descriptor_count &&
                      complete_scene.structure2_payload_anchors_consumed ==
                          complete_scene.structure2_payload_anchor_count &&
                      complete_scene.untextured_scene.untextured_face_count ==
                          loaded_level.structure3_face_materials
                              .non_textured_face_count &&
                      !complete_scene.transform_semantics_proven &&
                      !complete_scene.pixel_palette_vdp1_semantics_proven &&
                      complete_scene.decoder_permitted &&
                      complete_scene.no_draw_only &&
                      !complete_scene.fallback_visuals_permitted &&
                      complete_scene.blocks_real_dgn_mesh_render,
                      "complete retail Structure3 source scene accounts for every face category no-draw");
                nexus_viewport_init(&untextured_viewport);
                nexus_viewport_render(&untextured_viewport, &active_engine);
                CHECK(untextured_viewport.last_dgn_render_receipt
                          .structure3_untextured_face_scene_consumed &&
                      untextured_viewport.last_dgn_render_receipt
                          .structure3_untextured_face_scene.complete &&
                      untextured_viewport.last_dgn_render_receipt
                          .structure3_untextured_face_scene.consumed_face_count ==
                          scene.consumed_face_count &&
                      !untextured_viewport.last_dgn_render_receipt
                          .structure3_complete_source_scene_consumed &&
                      untextured_viewport.last_dgn_render_receipt
                          .structure3_mesh_rendered &&
                      untextured_viewport.last_dgn_render_receipt
                          .structure3_mesh_textured_face_count > 0 &&
                      !untextured_viewport.last_dgn_render_receipt
                          .no_draw_structure3_source_scene &&
                      untextured_viewport.last_dgn_render_receipt
                          .structure1f_source_scene_consumed &&
                      untextured_viewport.last_dgn_render_receipt
                          .structure1f_source_scene.family_coverage_complete &&
                      untextured_viewport.last_dgn_render_receipt
                          .structure1f_source_scene.entry_count ==
                          loaded_level.structure1f_entry_count &&
                      untextured_viewport.last_dgn_render_receipt
                          .structure1f_source_scene.consumed_entry_count ==
                          loaded_level.structure1f_entry_count &&
                      untextured_viewport.last_dgn_render_receipt
                          .structure1f_source_scene.source_bytes_fnv1a64 ==
                          fnv1a64(data, (size_t)size) &&
                      (loaded_level.structure1f_entry_count == 0 ||
                       (untextured_viewport.structure1f_source_packet.valid &&
                        untextured_viewport.structure1f_source_packet.no_draw_only &&
                        untextured_viewport.structure1f_source_packet
                            .source_bytes_fnv1a64 == fnv1a64(data, (size_t)size) &&
                        untextured_viewport.structure1f_source_packet.package_fnv1a64 ==
                            fnv1a64(data, (size_t)size) &&
                        untextured_viewport.structure1f_source_packet.descriptor_length > 0 &&
                        untextured_viewport.structure1f_source_packet.descriptor_fnv1a64 != 0 &&
                        untextured_viewport.structure1f_source_packet.descriptor_offset <=
                            (uint32_t)size &&
                        untextured_viewport.structure1f_source_packet.descriptor_length <=
                            (uint32_t)size - untextured_viewport.structure1f_source_packet.descriptor_offset)) &&
                      untextured_viewport.last_dgn_render_receipt
                          .structure1c_source_scene_consumed &&
                      untextured_viewport.last_dgn_render_receipt
                          .structure1c_source_scene.valid &&
                      untextured_viewport.last_dgn_render_receipt
                          .structure1c_source_scene.record_count ==
                          loaded_level.geometry_info.structure1c_record_count &&
                      untextured_viewport.last_dgn_render_receipt
                          .structure1c_source_scene.consumed_record_count ==
                          loaded_level.geometry_info.structure1c_indexed_record_count &&
                      untextured_viewport.structure1c_source_packet.valid &&
                      untextured_viewport.structure1c_source_packet.no_draw_only &&
                      untextured_viewport.structure1c_source_packet
                          .source_bytes_fnv1a64 == fnv1a64(data, (size_t)size) &&
                      ((untextured_viewport.structure1c_source_packet
                            .referenced_by_structure1b &&
                        untextured_viewport.structure1c_source_packet
                            .reference_occurrence_count > 0 &&
                        untextured_viewport.structure1c_source_packet
                            .first_reference_x >= 0 &&
                        untextured_viewport.structure1c_source_packet
                            .first_reference_y >= 0 &&
                        untextured_viewport.structure1c_source_packet
                            .last_reference_x >= 0 &&
                        untextured_viewport.structure1c_source_packet
                            .last_reference_y >= 0) ||
                       (!untextured_viewport.structure1c_source_packet
                             .referenced_by_structure1b &&
                        untextured_viewport.structure1c_source_packet
                            .reference_occurrence_count == 0 &&
                        untextured_viewport.structure1c_source_packet
                            .first_reference_x == -1 &&
                        untextured_viewport.structure1c_source_packet
                            .first_reference_y == -1 &&
                        untextured_viewport.structure1c_source_packet
                            .last_reference_x == -1 &&
                        untextured_viewport.structure1c_source_packet
                            .last_reference_y == -1)) &&
                      untextured_viewport.last_dgn_render_receipt
                          .structure2_payload_anchor_scene_consumed &&
                      untextured_viewport.last_dgn_render_receipt
                          .structure2_payload_anchor_scene.valid &&
                      untextured_viewport.last_dgn_render_receipt
                          .structure2_payload_anchor_scene.descriptor_count ==
                          loaded_level.structure2_texture_count &&
                      untextured_viewport.last_dgn_render_receipt
                          .structure2_payload_anchor_scene.consumed_anchor_count ==
                          untextured_viewport.last_dgn_render_receipt
                              .structure2_payload_anchor_scene.anchor_count &&
                      untextured_viewport.last_dgn_render_receipt
                          .structure2_payload_anchor_scene.image_anchor_count ==
                          loaded_level.structure2_texture_count &&
                      untextured_viewport.last_dgn_render_receipt
                          .structure2_payload_anchor_scene.source_bytes_fnv1a64 ==
                          fnv1a64(data, (size_t)size) &&
                      untextured_viewport.structure2_payload_anchor_packet.valid &&
                      untextured_viewport.structure2_payload_anchor_packet.no_draw_only &&
                      untextured_viewport.structure2_payload_anchor_packet
                          .source_bytes_fnv1a64 == fnv1a64(data, (size_t)size) &&
                      untextured_viewport.structure3_untextured_face.valid &&
                      untextured_viewport.structure3_untextured_face
                          .raw_fill_selector == untextured_packet.raw_fill_selector &&
                      !untextured_viewport.last_dgn_render_receipt.blocked &&
                      untextured_viewport.last_dgn_render_receipt
                          .rasterized_command_count == 0,
                      "DGN viewport renders Structure3 mesh with decoded textures");
                {
                    Nexus_V1_DgnViewportHostRouteReceipt host_route;
                    memset(&host_route, 0, sizeof(host_route));
                    CHECK(nexus_viewport_dgn_host_route_receipt(
                              &untextured_viewport, &active_engine,
                              &host_route) == 1 &&
                          !host_route.no_draw_structure3_source_scene &&
                          !host_route.blocks_runtime_dgn &&
                          host_route.can_present_runtime_dgn &&
                          host_route.status ==
                              NEXUS_V1_DGN_HOST_ROUTE_READY_RENDERED_MESH &&
                          strcmp(nexus_viewport_dgn_host_route_status_name(
                                     host_route.status),
                                 "ready-rendered-mesh") == 0,
                          "Structure3 mesh rendering provides a ready host route");
                }
            }
        }
        if (level == 0) {
            const char *target_path =
                "/private/tmp/firestaff_nexus_structure2_descriptor_target.txt";
            char target_line[128];
            char unverified_manifest[1024];
            const uint8_t unverified_trace[] = { 0x53U, 0x32U, 0x54U, 0x52U };
            FILE *target_file;
            Nexus_V1_DgnStructure2TraceAdmissionReceipt trace_receipt;

            CHECK(nexus_v1_engine_write_structure2_descriptor_capture_target(
                      &active_engine, 0, target_path, &descriptor_target) == 1 &&
                  descriptor_target.valid &&
                  descriptor_target.original_saturn_capture_required &&
                  descriptor_target.no_draw_only,
                  "active retail LEV writes an opaque Structure2 capture request");
            target_file = fopen(target_path, "rb");
            CHECK(target_file && fgets(target_line, sizeof(target_line), target_file) &&
                  strcmp(target_line,
                         "NEXUS_STRUCTURE2_DESCRIPTOR_CAPTURE_TARGET_V1\n") == 0,
                  "Structure2 capture request carries its non-runtime target magic");
            if (target_file) fclose(target_file);
            remove(target_path);
            snprintf(unverified_manifest, sizeof(unverified_manifest),
                     "magic=%s\nproducer=external-saturn-capture\n"
                     "level_index=%x\ndescriptor_index=%x\n"
                     "source_fnv1a64=%016llx\ndescriptor_fnv1a64=%016llx\n"
                     "opaque_payload_fnv1a64=%016llx\n"
                     "image_anchor_offset=%x\nimage_next_anchor_offset=%x\n"
                     "image_candidate_byte_count=%x\nimage_candidate_fnv1a64=%016llx\n"
                     "palette_candidate_present=%x\n"
                     "palette_anchor_offset=%x\npalette_next_anchor_offset=%x\n"
                     "palette_candidate_byte_count=%x\npalette_candidate_fnv1a64=%016llx\n"
                     "raw_trace_size=%zx\n"
                     "raw_trace_fnv1a64=%016llx\n",
                     NEXUS_V1_STRUCTURE2_SATURN_RAW_TRACE_MAGIC, level, 0,
                     (unsigned long long)descriptor_target.source_bytes_fnv1a64,
                     (unsigned long long)descriptor_target.descriptor_bytes_fnv1a64,
                     (unsigned long long)descriptor_target.opaque_payload_fnv1a64,
                     descriptor_target.image_payload_anchor_offset,
                     descriptor_target.image_payload_next_anchor_offset,
                     descriptor_target.image_payload_candidate_byte_count,
                     (unsigned long long)descriptor_target.image_payload_candidate_fnv1a64,
                     descriptor_target.palette_payload_candidate_bound,
                     descriptor_target.palette_payload_anchor_offset,
                     descriptor_target.palette_payload_next_anchor_offset,
                     descriptor_target.palette_payload_candidate_byte_count,
                     (unsigned long long)descriptor_target.palette_payload_candidate_fnv1a64,
                     sizeof(unverified_trace),
                     (unsigned long long)fnv1a64(unverified_trace,
                                                  sizeof(unverified_trace)));
            CHECK(nexus_v1_engine_admit_structure2_descriptor_capture_trace(
                      &active_engine, 0, unverified_manifest,
                      strlen(unverified_manifest), unverified_trace,
                      sizeof(unverified_trace), 0, &trace_receipt) == 0 &&
                  trace_receipt.status ==
                      NEXUS_V1_STRUCTURE2_TRACE_BLOCKED_PROVENANCE &&
                  trace_receipt.capture_target_bound &&
                  trace_receipt.manifest_target_bound &&
                  trace_receipt.image_payload_candidate_bound &&
                  trace_receipt.palette_payload_candidate_bound ==
                      descriptor_target.palette_payload_candidate_bound &&
                  trace_receipt.raw_trace_bytes_bound &&
                  !trace_receipt.original_saturn_capture_verified &&
                  !trace_receipt.opaque_trace_admitted &&
                  !trace_receipt.decoder_permitted && trace_receipt.no_draw_only &&
                  !trace_receipt.fallback_visuals_permitted &&
                  trace_receipt.blocks_real_dgn_mesh_render,
                  "unverified Structure2 raw traces remain fail-closed after exact target binding");
        }
        active_engine.current_level_structure2_source.loaded_dgn_fnv1a64 ^= 1U;
        CHECK(nexus_v1_current_level_structure3_face_material_receipt(
                  &active_engine, &active_face_material) == 0 &&
              !active_face_material.valid && active_face_material.no_draw_only &&
              !active_face_material.fallback_visuals_permitted,
              "active face-material selector binding withdraws on stale LEV identity");
        CHECK(nexus_v1_current_level_structure1a_owner_chain_receipt(
                  &active_engine, &active_owner_chain) == 0 &&
              !active_owner_chain.valid && active_owner_chain.no_draw_only &&
              !active_owner_chain.fallback_visuals_permitted,
              "active Structure1A owner chain withdraws on stale LEV identity");
        CHECK(nexus_v1_current_level_structure2_descriptor_receipt(
                  &active_engine, &active_structure2) == 0 &&
              !active_structure2.valid && active_structure2.no_draw_only &&
              !active_structure2.fallback_visuals_permitted,
              "active Structure2 descriptor envelope withdraws on stale LEV identity");
        CHECK(nexus_v1_current_level_structure2_format_evidence_receipt(
                  &active_engine, &structure2_format) == 0 &&
              !structure2_format.valid && structure2_format.no_draw_only &&
              !structure2_format.decoder_permitted &&
              !structure2_format.fallback_visuals_permitted,
              "stale LEV identity withdraws Structure2 format evidence");
        if (material_target.valid) {
            uint32_t stale_entry_index = material_target.structure3_entry_index;
            uint32_t stale_face_ordinal = material_target.face_ordinal;
            CHECK(nexus_v1_engine_build_structure3_static_material_capture_target(
                      &active_engine, stale_entry_index, stale_face_ordinal,
                      &material_target) == 0 &&
                  !material_target.valid && material_target.no_draw_only &&
                  !material_target.fallback_visuals_permitted,
                  "stale LEV identity withdraws Structure3-to-Structure2 capture targets");
            memset(&package_geometry, 0, sizeof(package_geometry));
            CHECK(nexus_v1_current_level_structure3_package_geometry_packet(
                      &active_engine, stale_entry_index, stale_face_ordinal,
                      &package_geometry) == 0 &&
                  !package_geometry.valid && package_geometry.no_draw_only &&
                  !package_geometry.fallback_visuals_permitted &&
                  package_geometry.blocks_real_dgn_mesh_render,
                  "stale LEV identity withdraws package-side Structure3 geometry");
        }
        active_engine.current_level_structure2_source.loaded_dgn_fnv1a64 =
            fnv1a64(data, (size_t)size);
        CHECK(nexus_v1_level_structure3_ordinal_correlation_receipt(
                  &loaded_level, &correlation) == 0 &&
              correlation.structure1a_relation_complete ==
                  handoff.structure3_model_references.complete &&
              correlation.structure3_payload_valid ==
                  loaded_level.structure3_payload.valid &&
              correlation.structure3_block_count ==
                  loaded_level.structure3_payload.block_count &&
              correlation.structure3_nonzero_byte_run_count ==
                  loaded_level.structure3_payload.nonzero_byte_run_count &&
              correlation.structure3_nonzero_block_run_count ==
                  loaded_level.structure3_payload.nonzero_block_run_count &&
              correlation.structure3_directory_valid &&
              correlation.structure3_directory_entry_count ==
                  handoff.structure3_directory.entry_count &&
              correlation.direct_block_ordinal_mapping_disproven ==
                  (correlation.zero_based_block_ordinal_mapping_disproven &&
                   correlation.one_based_block_ordinal_mapping_disproven) &&
              correlation.direct_byte_run_ordinal_mapping_disproven ==
                  (correlation.zero_based_byte_run_ordinal_mapping_disproven &&
                   correlation.one_based_byte_run_ordinal_mapping_disproven) &&
              correlation.direct_run_ordinal_mapping_disproven ==
                  (correlation.zero_based_run_ordinal_mapping_disproven &&
                   correlation.one_based_run_ordinal_mapping_disproven) &&
              correlation.direct_directory_ordinal_mapping_disproven ==
                  (correlation.zero_based_directory_ordinal_mapping_disproven &&
                   correlation.one_based_directory_ordinal_mapping_disproven) &&
              !correlation.face_semantics_proven,
              "retail Structure3 correlation rules out only disproven direct ordinals");
        if (loaded_level.structure3_payload.declared) {
            CHECK(loaded_level.structure3_payload.valid &&
                  loaded_level.structure3_payload.zero_byte_count +
                          loaded_level.structure3_payload.nonzero_byte_count ==
                      loaded_level.structure3_payload.byte_size &&
                  ((loaded_level.structure3_payload.nonzero_byte_count == 0 &&
                    loaded_level.structure3_payload.nonzero_byte_run_count == 0 &&
                    loaded_level.structure3_payload.longest_nonzero_byte_run == 0 &&
                    loaded_level.structure3_payload.first_nonzero_byte_run_offset == -1 &&
                    loaded_level.structure3_payload.first_nonzero_byte_run_byte_count == 0 &&
                    loaded_level.structure3_payload.last_nonzero_byte_run_offset == -1 &&
                    loaded_level.structure3_payload.last_nonzero_byte_run_byte_count == 0) ||
                   (loaded_level.structure3_payload.nonzero_byte_count > 0 &&
                    loaded_level.structure3_payload.nonzero_byte_run_count > 0 &&
                    loaded_level.structure3_payload.longest_nonzero_byte_run > 0 &&
                    loaded_level.structure3_payload.longest_nonzero_byte_run <=
                        loaded_level.structure3_payload.nonzero_byte_count &&
                    loaded_level.structure3_payload.first_nonzero_byte_run_offset >= 0 &&
                    loaded_level.structure3_payload.first_nonzero_byte_run_byte_count > 0 &&
                    loaded_level.structure3_payload.first_nonzero_byte_run_offset +
                        loaded_level.structure3_payload.first_nonzero_byte_run_byte_count <=
                        loaded_level.structure3_payload.byte_size &&
                    loaded_level.structure3_payload.last_nonzero_byte_run_offset >= 0 &&
                    loaded_level.structure3_payload.last_nonzero_byte_run_byte_count > 0 &&
                    loaded_level.structure3_payload.last_nonzero_byte_run_offset +
                        loaded_level.structure3_payload.last_nonzero_byte_run_byte_count <=
                        loaded_level.structure3_payload.byte_size)) &&
                  loaded_level.structure3_payload.complete_block_count ==
                      loaded_level.structure3_payload.block_count &&
                  loaded_level.structure3_payload.zero_block_count +
                          loaded_level.structure3_payload.nonzero_block_count ==
                      loaded_level.structure3_payload.complete_block_count &&
                  ((loaded_level.structure3_payload.nonzero_block_count == 0 &&
                    loaded_level.structure3_payload.first_nonzero_block_index == -1 &&
                    loaded_level.structure3_payload.last_nonzero_block_index == -1 &&
                    loaded_level.structure3_payload.nonzero_block_run_count == 0 &&
                    loaded_level.structure3_payload.longest_nonzero_block_run == 0 &&
                    loaded_level.structure3_payload
                        .first_nonzero_block_run_start_block_index == -1 &&
                    loaded_level.structure3_payload
                        .first_nonzero_block_run_block_count == 0 &&
                    loaded_level.structure3_payload
                        .last_nonzero_block_run_start_block_index == -1 &&
                    loaded_level.structure3_payload
                        .last_nonzero_block_run_block_count == 0) ||
                   (loaded_level.structure3_payload.nonzero_block_count > 0 &&
                    loaded_level.structure3_payload.first_nonzero_block_index >= 0 &&
                    loaded_level.structure3_payload.last_nonzero_block_index >=
                        loaded_level.structure3_payload.first_nonzero_block_index &&
                    loaded_level.structure3_payload.nonzero_block_run_count > 0 &&
                    loaded_level.structure3_payload.longest_nonzero_block_run > 0 &&
                    loaded_level.structure3_payload.longest_nonzero_block_run <=
                        loaded_level.structure3_payload.nonzero_block_count &&
                    loaded_level.structure3_payload
                        .first_nonzero_block_run_start_block_index >= 0 &&
                    loaded_level.structure3_payload
                        .first_nonzero_block_run_block_count > 0 &&
                    loaded_level.structure3_payload
                        .first_nonzero_block_run_start_block_index +
                            loaded_level.structure3_payload
                                .first_nonzero_block_run_block_count <=
                        loaded_level.structure3_payload.complete_block_count &&
                    loaded_level.structure3_payload
                        .last_nonzero_block_run_start_block_index >= 0 &&
                    loaded_level.structure3_payload
                        .last_nonzero_block_run_block_count > 0 &&
                    loaded_level.structure3_payload
                        .last_nonzero_block_run_start_block_index +
                            loaded_level.structure3_payload
                                .last_nonzero_block_run_block_count <=
                        loaded_level.structure3_payload.complete_block_count)) &&
                  loaded_level.structure3_payload.distinct_byte_value_count > 0 &&
                  loaded_level.structure3_payload.first_nonzero_byte_offset >= -1 &&
                  loaded_level.structure3_payload.last_nonzero_byte_offset >=
                      loaded_level.structure3_payload.first_nonzero_byte_offset &&
                  loaded_level.structure3_payload.raw_payload_hash != 0U &&
                  !loaded_level.structure3_payload.face_semantics_proven,
                  "retail Structure3 spans retain raw bounded observations without face semantics");
            ++structure3_declared_level_count;
        }
        if (loaded_level.structure3_payload.valid) {
            ++structure3_valid_level_count;
            structure3_byte_total += loaded_level.structure3_payload.byte_size;
            structure3_nonzero_byte_total +=
                loaded_level.structure3_payload.nonzero_byte_count;
            structure3_transition_total +=
                loaded_level.structure3_payload.byte_transition_count;
            structure3_nonzero_byte_run_total +=
                loaded_level.structure3_payload.nonzero_byte_run_count;
            if (loaded_level.structure3_payload.longest_nonzero_byte_run >
                structure3_longest_nonzero_byte_run) {
                structure3_longest_nonzero_byte_run =
                    loaded_level.structure3_payload.longest_nonzero_byte_run;
            }
            structure3_complete_block_total +=
                loaded_level.structure3_payload.complete_block_count;
            structure3_zero_block_total +=
                loaded_level.structure3_payload.zero_block_count;
            structure3_nonzero_block_total +=
                loaded_level.structure3_payload.nonzero_block_count;
            structure3_nonzero_block_run_total +=
                loaded_level.structure3_payload.nonzero_block_run_count;
            if (loaded_level.structure3_payload.longest_nonzero_block_run >
                structure3_longest_nonzero_block_run) {
                structure3_longest_nonzero_block_run =
                    loaded_level.structure3_payload.longest_nonzero_block_run;
            }
        }
        CHECK(loaded_level.structure2_payload.valid &&
              loaded_level.structure2_payload.opaque_payload_zero_byte_count +
                      loaded_level.structure2_payload.opaque_payload_nonzero_byte_count ==
                  loaded_level.structure2_payload.opaque_payload_size &&
              loaded_level.structure2_payload.opaque_payload_complete_pair_count ==
                  loaded_level.structure2_payload.opaque_payload_size / 2 &&
              loaded_level.structure2_payload.opaque_payload_trailing_byte_count ==
                  loaded_level.structure2_payload.opaque_payload_size % 2 &&
              loaded_level.structure2_payload.opaque_payload_zero_pair_count +
                      loaded_level.structure2_payload.opaque_payload_nonzero_pair_count ==
                  loaded_level.structure2_payload.opaque_payload_complete_pair_count &&
              loaded_level.structure2_payload.local_payload_offset_pattern_observed &&
              loaded_level.structure2_payload
                  .local_payload_word_aligned_offset_pattern_observed &&
              loaded_level.structure2_payload
                  .local_payload_word_bounded_offset_pattern_observed &&
              loaded_level.structure2_payload
                  .nonzero_descriptor_offsets_unaligned_count == 0 &&
              loaded_level.structure2_payload
                  .nonzero_descriptor_offsets_in_opaque_payload_count ==
                  loaded_level.structure2_payload.nonzero_descriptor_offset_count &&
              loaded_level.structure2_payload
                  .nonzero_descriptor_offsets_word_bounded_count ==
                  loaded_level.structure2_payload.nonzero_descriptor_offset_count &&
              loaded_level.structure2_payload.descriptor_offset_envelope_valid &&
              nexus_v1_level_structure2_source_envelope_valid(&loaded_level) &&
              loaded_level.structure2_payload.material_or_image_data_proven,
              "real Structure2 descriptors retain only their complete aligned opaque target windows");
        structure2_descriptor_total += loaded_level.structure2_texture_count;
        structure2_nonzero_target_total +=
            loaded_level.structure2_payload.nonzero_descriptor_offset_count;
        structure2_in_span_target_total +=
            loaded_level.structure2_payload
                .nonzero_descriptor_offsets_in_opaque_payload_count;
        structure2_word_bounded_target_total +=
            loaded_level.structure2_payload
                .nonzero_descriptor_offsets_word_bounded_count;
        structure2_unaligned_target_total +=
            loaded_level.structure2_payload
                .nonzero_descriptor_offsets_unaligned_count;
        for (cell = 0; cell < NEXUS_MAX_MAP_SIZE * NEXUS_MAX_MAP_SIZE;
             ++cell) {
            const uint8_t *raw = data + info.structure1b_offset + cell * 8;
            if (raw[3] > 14U) ++byte3_above_wall_bank;
            if (raw[4] > 14U) ++byte4_above_wall_bank;
        }
        CHECK(byte3_above_wall_bank == expected_byte3_above_wall_bank[level] &&
              byte4_above_wall_bank == expected_byte4_above_wall_bank[level] &&
              byte3_above_wall_bank + byte4_above_wall_bank > 0,
              "retail Structure1B bytes 3/4 exceed the SN_WALL descriptor bank and stay unbound");
        for (int entry = 0; entry < loaded_level.structure1g_entry_count;
             ++entry) {
            CHECK(loaded_level.structure1g_entries[entry].first_structure2_image_valid,
                  "every canonical Structure1G first image binds Structure2");
        }
        checked++;
        free(data);
    }
    CHECK(checked == 16, "all LEV00 through LEV15 files were checked");
    CHECK(structure2_encoding_0x0008_total == 1553 &&
          structure2_encoding_0x0028_total == 125 &&
          structure2_palette_anchor_total == 1266 &&
          structure2_palette_absent_total == 412,
          "retail Structure2 corpus locks its observed descriptor-class and palette-anchor split");
    CHECK(structure3_static_material_capture_target_level_count > 0,
          "real DGN corpus yields source-bound static face/material capture targets");
    CHECK(structure3_animated_material_packet_level_count > 0,
          "real DGN corpus yields source-bound animated Structure3 packets");
    CHECK(structure2_descriptor_total == 1678 &&
          structure2_nonzero_target_total == 2944 &&
          structure2_in_span_target_total == 2944 &&
          structure2_word_bounded_target_total == 2944 &&
          structure2_unaligned_target_total == 0,
          "retail Structure2 corpus retains the known 20-byte descriptor target envelope only");
    CHECK(structure3_valid_level_count == structure3_declared_level_count &&
          structure3_byte_total >= structure3_nonzero_byte_total &&
          structure3_zero_block_total + structure3_nonzero_block_total ==
              structure3_complete_block_total &&
          structure3_nonzero_block_run_total <= structure3_nonzero_block_total &&
          structure3_longest_nonzero_block_run <= structure3_nonzero_block_total &&
          structure3_nonzero_byte_run_total <= structure3_nonzero_byte_total &&
          structure3_longest_nonzero_byte_run <= structure3_nonzero_byte_total &&
          structure3_transition_total >= 0,
          "retail Structure3 corpus retains only raw zero-separated byte and block spans");
    CHECK(structure3_static_face_selector_total > 0 &&
          structure3_animated_face_selector_total > 0,
          "retail Structure3 texture and animated selectors bind only to documented tables");
    CHECK(structure3_maximum_normal_length_error == 196835ULL,
          "retail Structure3 signed 16.16 normal residual stays corpus-verified");
}

static void test_structure1c_record_table_bounds(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    const int structure1b_rel = 0x40;
    int post_grid;
    uint8_t *structure1;
    Nexus_V1_DgnStructure1Layout layout;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, 32) == 0,
          "Structure1C record-table fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    post_grid = structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES;
    wb32(structure1 + 0x24, (uint32_t)(post_grid + 8));
    structure1[post_grid] = 2;
    CHECK(nexus_v1_dgn_structure1_layout(&layout, dgn, (int)sizeof(dgn)) == 0 &&
          layout.structure1c.valid &&
          layout.structure1c.size == 8 &&
          layout.structure1c.record_count == 2 &&
          layout.structure1c.indexed_record_count == 1,
          "counted four-byte Structure1C span becomes consumable only when bounded");
    structure1[post_grid] = 3;
    CHECK(nexus_v1_dgn_structure1_layout(&layout, dgn, (int)sizeof(dgn)) == 0 &&
          !layout.structure1c.valid,
          "mismatched Structure1C count leaves the post-grid span untyped");
}

static void test_structure1f_semantics_and_bounds(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 24];
    const int structure1b_rel = 0x200;
    uint8_t *structure1;
    Nexus_V1_DgnStructure1Layout layout;
    Nexus_V1_DgnGeometryInfo info;
    Nexus_V1_Level level;
    Nexus_V1_DgnRendererHandoffReceipt handoff;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    Nexus_V1_DgnStructure1ABoundaryReceipt structure1a_boundary;
    Nexus_V1_DgnStructure1ARelationReceipt structure1a_relation;
    Nexus_V1_DgnStructure1AKindReceipt structure1a_kinds;
    Nexus_V1_DgnStructure3ModelReferenceReceipt structure3_model_references;
    Nexus_V1_DgnStructure1ATransformSelectorReceipt transform_selectors;
    Nexus_V1_DgnStructure1FFaceSelectorReceipt face_selectors;
    Nexus_V1_DgnStructure3ModelFaceSelectorReceipt model_face_selectors;
    Nexus_V1_DgnStructure1FRotationSelectorReceipt rotation_selectors;
    Nexus_V1_DgnStructure1FFaceRotationPairReceipt face_rotation_pairs;
    Nexus_V1_DgnStructure1FOffsetPairReceipt offset_pairs;
    Nexus_V1_DgnStructure1FWallPayloadSelectorReceipt wall_payload_selectors;
    Nexus_V1_DgnStructure1FWallSensorDestinationReceipt wall_sensor_destinations;
    Nexus_V1_DgnStructure1FWallSensorControlSelectorReceipt wall_sensor_controls;
    Nexus_V1_DgnStructure1FWallSensorControlDestinationTupleReceipt
        wall_sensor_control_destination_tuples;
    Nexus_V1_DgnStructure1FWallSensorModelRotationPairReceipt
        wall_sensor_model_rotation_pairs;
    Nexus_V1_DgnStructure1FWallDecorationModelRotationPairReceipt
        wall_decoration_model_rotation_pairs;
    Nexus_V1_DgnStructure1FAlcovePayloadSelectorReceipt alcove_payload_selectors;
    Nexus_V1_DgnStructure1FAlcovePayloadRotationPairReceipt
        alcove_payload_rotation_pairs;
    Nexus_V1_DgnStructure1FFloorSensorControlSelectorReceipt floor_sensor_controls;
    Nexus_V1_DgnStructure1FFloorSensorDestinationReceipt floor_sensor_destinations;
    Nexus_V1_DgnStructure1FFloorSensorModelRotationPairReceipt
        floor_sensor_model_rotation_pairs;
    Nexus_V1_DgnStructure1FFloorSensorExtentPairReceipt floor_sensor_extent_pairs;
    Nexus_V1_DgnStructure1FFloorDecorationPayloadSelectorReceipt floor_decoration_payloads;
    Nexus_V1_DgnStructure1FFloorDecorationRotationSelectorReceipt floor_decoration_rotations;
    Nexus_V1_DgnStructure1FFloorDecorationModelRotationPairReceipt
        floor_decoration_model_rotation_pairs;
    Nexus_V1_DgnStructure1FFloorDecorationControlExtentReceipt
        floor_decoration_control_extents;
    Nexus_V1_DgnStructure1FItemAttributePairReceipt item_attribute_pairs;
    Nexus_V1_DgnStructure1FItemLocationPairReceipt item_location_pairs;
    Nexus_V1_DgnStructure1FItemCoordinatePairReceipt item_coordinate_pairs;
    Nexus_V1_DgnStructure3PayloadReceipt structure3_payload;
    Nexus_V1_DgnStructure3OrdinalCorrelationReceipt structure3_correlation;
    Nexus_V1_DgnRenderCommand commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnRenderPlanReceipt render_plan;
    Nexus_V1_DgnStructure1FStructure1ACommandSource structure1a_sources[8];
    Nexus_V1_DgnStructure1FStructure1ACommandSourceReceipt
        structure1a_source_receipt;
    Nexus_V1_DgnStructure1AStructure3TopologyCandidate
        structure3_candidates[8];
    Nexus_V1_DgnStructure1AStructure3TopologyCandidateReceipt
        structure3_candidate_receipt;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, 512) == 0,
          "Structure1F fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    /* DMWeb DGN container Structure3 block envelope: opaque payload only. */
    wb16(dgn + 0x1c, 20U);
    wb16(dgn + 0x1e, 4U);
    dgn[NEXUS_DGN_BLOCK_SIZE * 20 + 1] = 0x7fU;
    dgn[NEXUS_DGN_BLOCK_SIZE * 20 + 2] = 0x7fU;
    dgn[NEXUS_DGN_BLOCK_SIZE * 21] = 0x33U;
    dgn[NEXUS_DGN_BLOCK_SIZE * 23 + 7] = 0x42U;
    wb32(structure1 + 0x0c, 9U);
    for (int index = 0; index < 9; ++index) {
        structure1[0x38 + index * NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES] =
            (uint8_t)(index % 3);
        structure1[0x38 + index * NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES + 1] =
            (uint8_t)(0x20 + index);
        structure1[0x38 + index * NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES + 2] =
            (uint8_t)(index % 4);
    }
    set_structure1a_owner_ref(structure1, structure1b_rel, 10, 10, 0);
    set_structure1a_owner_ref(structure1, structure1b_rel, 11, 10, 3);
    set_structure1a_owner_ref(structure1, structure1b_rel, 12, 10, 4);
    set_structure1a_owner_ref(structure1, structure1b_rel, 13, 10, 5);
    set_structure1a_owner_ref(structure1, structure1b_rel, 14, 10, 6);
    set_structure1a_owner_ref(structure1, structure1b_rel, 15, 10, 7);
    set_structure1a_owner_ref(structure1, structure1b_rel, 16, 10, 8);
    build_structure1f_fixture(structure1, structure1b_rel);
    CHECK(nexus_v1_dgn_structure1_layout(&layout, dgn, (int)sizeof(dgn)) == 0 &&
          layout.structure1f.valid && layout.structure1f.relative_offset ==
              structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 312 &&
          layout.structure1f.size == 200 &&
          layout.structure1f.wall_sensor_first_texture_index == 0x34 &&
          layout.structure1f.wall_sensor_first_model_index == 0x12 &&
          layout.structure1f.total_entry_count == 14 &&
          layout.structure1f.family_count[NEXUS_V1_DGN_STRUCTURE1F_ITEMS] == 2 &&
          layout.structure1f.family_count[NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS] == 4,
          "Structure1F accepts only an exact counted six-family span");
    CHECK(nexus_v1_dgn_geometry_info(&info, dgn, (int)sizeof(dgn)) == 0 &&
          info.structure1f_valid && info.structure1f_total_entry_count == 14 &&
          info.structure1f_family_count[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS] == 2,
          "Structure1F coverage reaches DGN geometry provenance");
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          level.structure1f_entry_count == 14 &&
          level.structure1f_entries[0].family == NEXUS_V1_DGN_STRUCTURE1F_ITEMS &&
          level.structure1f_entries[0].location == 4 &&
          level.structure1f_entries[0].item_id == 0x8eU &&
          level.structure1f_entries[0].attribute1 == 7 &&
          level.structure1f_entries[0].attribute2 == 15 &&
          level.structure1f_entries[2].offset_x == -30 &&
          level.structure1f_entries[2].type_or_control == 3U &&
          level.structure1f_entries[2].width == 14U &&
          level.structure1f_entries[2].height == 15U &&
          level.structure1f_entries[4].model_or_aspect == 0x27U &&
          level.structure1f_entries[4].destination_orientation == 2,
          "Structure1F retains documented item, decoration, and sensor fields");
    CHECK(nexus_v1_level_structure1f_spatial_receipt(&level, &spatial) == 0 &&
          spatial.valid && spatial.typed_entry_count == 14 &&
          spatial.direct_coordinate_entry_count == 6 &&
          spatial.item_entry_count == 2 &&
          spatial.floor_decoration_entry_count == 2 &&
          spatial.floor_sensor_entry_count == 2 &&
          spatial.structure1a_bound_entry_count == 8,
          "Structure1F separates direct cell records from unresolved Structure1A records");
    CHECK(nexus_v1_level_structure1f_item_attribute_pair_receipt(
              &level, &item_attribute_pairs) == 0 &&
          item_attribute_pairs.spatial_valid &&
          item_attribute_pairs.item_count == 2 &&
          item_attribute_pairs.resolved_pair_count == 2 &&
          item_attribute_pairs.unique_pair_count == 2 &&
          item_attribute_pairs.duplicate_pair_count == 0 &&
          item_attribute_pairs.complete &&
          !item_attribute_pairs.semantics_proven,
          "Structure1F item attributes remain raw no-draw provenance");
    CHECK(nexus_v1_level_structure1f_item_location_pair_receipt(
              &level, &item_location_pairs) == 0 &&
          item_location_pairs.spatial_valid &&
          item_location_pairs.item_count == 2 &&
          item_location_pairs.resolved_pair_count == 2 &&
          item_location_pairs.unique_pair_count == 2 &&
          item_location_pairs.duplicate_pair_count == 0 &&
          item_location_pairs.complete &&
          !item_location_pairs.semantics_proven,
          "Structure1F item locations remain raw no-draw provenance");
    CHECK(nexus_v1_level_structure1f_item_coordinate_pair_receipt(
              &level, &item_coordinate_pairs) == 0 &&
          item_coordinate_pairs.spatial_valid &&
          item_coordinate_pairs.item_count == 2 &&
          item_coordinate_pairs.resolved_pair_count == 2 &&
          item_coordinate_pairs.unique_pair_count == 2 &&
          item_coordinate_pairs.duplicate_pair_count == 0 &&
          item_coordinate_pairs.zero_pair_count == 0 &&
          item_coordinate_pairs.nonzero_pair_count == 2 &&
          item_coordinate_pairs.highest_pair == 0x0b15U &&
          item_coordinate_pairs.complete &&
          !item_coordinate_pairs.semantics_proven,
          "Structure1F item coordinate bytes remain raw no-draw provenance");
    CHECK(nexus_v1_level_structure1a_boundary_receipt(&level,
                                                       &structure1a_boundary) == 0 &&
          structure1a_boundary.valid && structure1a_boundary.entry_count == 8 &&
          structure1a_boundary.alcove_entry_count == 2 &&
          structure1a_boundary.wall_decoration_entry_count == 2 &&
          structure1a_boundary.wall_sensor_entry_count == 4 &&
          structure1a_boundary.zero_index_count == 1 &&
          structure1a_boundary.nonzero_index_count == 7 &&
          structure1a_boundary.unique_index_count == 7 &&
          structure1a_boundary.duplicate_index_count == 1 &&
          structure1a_boundary.highest_index == 8U,
          "Structure1A indexes reach the host boundary as raw provenance only");
    CHECK(nexus_v1_level_structure1a_relation_receipt(&level,
                                                       &structure1a_relation) == 0 &&
          structure1a_relation.table_valid &&
          structure1a_relation.table_entry_count == 9 &&
          structure1a_relation.structure1f_bound_entry_count == 8 &&
          structure1a_relation.resolved_entry_count == 8 &&
          structure1a_relation.complete &&
          level.structure1f_entries[7].structure1a_relation_valid &&
          level.structure1f_entries[7].structure1a_owner_x == 11 &&
          level.structure1f_entries[7].structure1a_owner_y == 10 &&
          level.structure1f_entries[7].structure1a_structure3_model_index == 0x23 &&
          level.structure1f_entries[7].structure1a_z_rotation == 3U,
          "Structure1F references resolve only through unique Structure1B owners");
    memset(commands, 0, sizeof(commands));
    commands[0].kind = NEXUS_V1_DGN_RENDER_COMMAND_FLOOR;
    commands[0].x = 11;
    commands[0].y = 10;
    commands[1].kind = NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT;
    commands[1].x = 11;
    commands[1].y = 10;
    memset(structure1a_sources, 0, sizeof(structure1a_sources));
    memset(&structure1a_source_receipt, 0, sizeof(structure1a_source_receipt));
    CHECK(nexus_v1_dgn_bind_structure1a_owned_cell_sources(
              &level, commands, 2, structure1a_sources, 8,
              &structure1a_source_receipt) == 0 &&
          structure1a_source_receipt.complete &&
          structure1a_source_receipt.visible_owned_entry_count == 2 &&
          structure1a_source_receipt.floor_command_source_count == 2 &&
          structure1a_source_receipt.alcove_floor_command_source_count == 2 &&
          structure1a_source_receipt.wall_decoration_floor_command_source_count == 0 &&
          structure1a_source_receipt.wall_sensor_floor_command_source_count == 0 &&
          structure1a_sources[0].command_index == 0 &&
          structure1a_sources[0].owner_x == 11 &&
          structure1a_sources[0].owner_y == 10 &&
          structure1a_sources[0].entry.family ==
              NEXUS_V1_DGN_STRUCTURE1F_ALCOVES &&
          !structure1a_sources[0].draw_authorized &&
          !structure1a_source_receipt.fallback_visuals_permitted,
          "Structure1A-owned rows reach only their verified visible cell anchor without a draw claim");
    memset(structure3_candidates, 0, sizeof(structure3_candidates));
    memset(&structure3_candidate_receipt, 0,
           sizeof(structure3_candidate_receipt));
    CHECK(nexus_v1_dgn_bind_structure1a_structure3_topology_candidates(
              &level, structure1a_sources,
              structure1a_source_receipt.floor_command_source_count,
              structure3_candidates, 8, &structure3_candidate_receipt) == 0 &&
          structure3_candidate_receipt.complete &&
          structure3_candidate_receipt.owner_cell_source_count == 2 &&
          structure3_candidate_receipt.topology_candidate_count == 2 &&
          structure3_candidate_receipt.direct_ordinal_mapping_disproven_count == 2 &&
          structure3_candidates[0].command_index == 0 &&
          structure3_candidates[0].owner_x == 11 &&
          structure3_candidates[0].owner_y == 10 &&
          structure3_candidates[0].structure3_model_index == 0x23U &&
          structure3_candidates[0].structure3_block_offset == 20 &&
          structure3_candidates[0].structure3_block_count == 4 &&
          structure3_candidates[0].structure3_byte_size ==
              NEXUS_DGN_BLOCK_SIZE * 4 &&
          structure3_candidates[0].structure3_raw_payload_hash != 0U &&
          structure3_candidates[0].model_index_exceeds_block_count &&
          structure3_candidates[0].model_index_exceeds_nonzero_byte_run_count &&
          structure3_candidates[0].model_index_exceeds_nonzero_block_run_count &&
          structure3_candidates[0].direct_ordinal_mapping_disproven &&
          !structure3_candidates[0].model_ordinal_proven &&
          !structure3_candidates[0].face_semantics_proven &&
          !structure3_candidates[0].draw_authorized &&
          !structure3_candidate_receipt.fallback_visuals_permitted,
          "Structure1A owner cells retain only bounded Structure3 topology candidates without ordinal or draw claims");
    CHECK(nexus_v1_level_structure1a_kind_receipt(&level, &structure1a_kinds) == 0 &&
          structure1a_kinds.structure1a_relation_complete &&
          structure1a_kinds.structure1f_bound_entry_count == 8 &&
          structure1a_kinds.resolved_kind_count == 8 &&
          structure1a_kinds.unique_kind_count == 3 &&
          structure1a_kinds.duplicate_kind_count == 5 &&
          structure1a_kinds.zero_kind_count == 4 &&
          structure1a_kinds.nonzero_kind_count == 4 &&
          structure1a_kinds.highest_kind == 2U &&
          structure1a_kinds.complete &&
          !structure1a_kinds.kind_semantics_proven,
          "resolved Structure1A kind bytes remain raw no-draw provenance");
    CHECK(nexus_v1_level_structure1a_transform_selector_receipt(
              &level, &transform_selectors) == 0 &&
          transform_selectors.structure1a_relation_complete &&
          transform_selectors.structure1f_bound_entry_count == 8 &&
          transform_selectors.resolved_selector_count == 8 &&
          transform_selectors.unique_selector_count == 4 &&
          transform_selectors.duplicate_selector_count == 4 &&
          transform_selectors.zero_selector_count == 3 &&
          transform_selectors.nonzero_selector_count == 5 &&
          transform_selectors.highest_selector == 3U &&
          transform_selectors.complete &&
          !transform_selectors.transform_semantics_proven,
          "resolved Structure1A transform selectors remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_face_selector_receipt(
              &level, &face_selectors) == 0 &&
          face_selectors.structure1a_relation_complete &&
          face_selectors.structure1f_bound_entry_count == 8 &&
          face_selectors.resolved_face_selector_count == 8 &&
          face_selectors.unique_face_selector_count == 4 &&
          face_selectors.duplicate_face_selector_count == 4 &&
          face_selectors.zero_face_selector_count == 0 &&
          face_selectors.nonzero_face_selector_count == 8 &&
          face_selectors.highest_face_selector == 4U &&
          face_selectors.complete && !face_selectors.face_semantics_proven,
          "resolved Structure1F face selectors remain no-draw provenance");
    CHECK(nexus_v1_level_structure3_model_face_selector_receipt(
              &level, &model_face_selectors) == 0 &&
          model_face_selectors.structure1a_relation_complete &&
          model_face_selectors.structure1f_bound_entry_count == 8 &&
          model_face_selectors.resolved_pair_count == 8 &&
          model_face_selectors.unique_pair_count == 7 &&
          model_face_selectors.duplicate_pair_count == 1 &&
          model_face_selectors.zero_pair_count == 0 &&
          model_face_selectors.nonzero_pair_count == 8 &&
          model_face_selectors.highest_pair == 0x2804U &&
          model_face_selectors.complete &&
          !model_face_selectors.attachment_semantics_proven,
          "Structure3 model-face pairs retain original attachment provenance only");
    CHECK(nexus_v1_level_structure1f_rotation_selector_receipt(
              &level, &rotation_selectors) == 0 &&
          rotation_selectors.structure1a_relation_complete &&
          rotation_selectors.structure1f_bound_entry_count == 8 &&
          rotation_selectors.resolved_rotation_selector_count == 8 &&
          rotation_selectors.unique_rotation_selector_count == 4 &&
          rotation_selectors.duplicate_rotation_selector_count == 4 &&
          rotation_selectors.zero_rotation_selector_count == 0 &&
          rotation_selectors.nonzero_rotation_selector_count == 8 &&
          rotation_selectors.highest_rotation_selector == 4U &&
          rotation_selectors.complete &&
          !rotation_selectors.rotation_semantics_proven,
          "resolved Structure1F rotation selectors remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_face_rotation_pair_receipt(
              &level, &face_rotation_pairs) == 0 &&
          face_rotation_pairs.structure1a_relation_complete &&
          face_rotation_pairs.structure1f_bound_entry_count == 8 &&
          face_rotation_pairs.resolved_pair_count == 8 &&
          face_rotation_pairs.unique_pair_count == 4 &&
          face_rotation_pairs.duplicate_pair_count == 4 &&
          face_rotation_pairs.zero_pair_count == 0 &&
          face_rotation_pairs.nonzero_pair_count == 8 &&
          face_rotation_pairs.highest_pair == 0x0404U &&
          face_rotation_pairs.complete &&
          !face_rotation_pairs.pair_semantics_proven,
          "Structure1F face-rotation pairs remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_offset_pair_receipt(
              &level, &offset_pairs) == 0 &&
          offset_pairs.structure1a_relation_complete &&
          offset_pairs.structure1f_bound_entry_count == 8 &&
          offset_pairs.resolved_offset_pair_count == 8 &&
          offset_pairs.unique_offset_pair_count == 4 &&
          offset_pairs.duplicate_offset_pair_count == 4 &&
          offset_pairs.zero_offset_pair_count == 0 &&
          offset_pairs.nonzero_offset_pair_count == 8 &&
          offset_pairs.minimum_offset_x == -1 &&
          offset_pairs.maximum_offset_x == 127 &&
          offset_pairs.minimum_offset_y == -128 &&
          offset_pairs.maximum_offset_y == 2 &&
          offset_pairs.complete && !offset_pairs.offset_semantics_proven,
          "Structure1F signed offset pairs remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_wall_payload_selector_receipt(
              &level, &wall_payload_selectors) == 0 &&
          wall_payload_selectors.structure1a_relation_complete &&
          wall_payload_selectors.wall_payload_entry_count == 6 &&
          wall_payload_selectors.resolved_payload_selector_count == 6 &&
          wall_payload_selectors.wall_decoration_selector_count == 2 &&
          wall_payload_selectors.wall_sensor_selector_count == 4 &&
          wall_payload_selectors.unique_payload_selector_count == 3 &&
          wall_payload_selectors.duplicate_payload_selector_count == 3 &&
          wall_payload_selectors.zero_payload_selector_count == 0 &&
          wall_payload_selectors.nonzero_payload_selector_count == 6 &&
          wall_payload_selectors.highest_payload_selector == 9U &&
          wall_payload_selectors.complete &&
          !wall_payload_selectors.payload_semantics_proven,
          "Structure1F wall payload selectors remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_wall_sensor_destination_receipt(
              &level, &wall_sensor_destinations) == 0 &&
          wall_sensor_destinations.structure1a_relation_complete &&
          wall_sensor_destinations.wall_sensor_entry_count == 4 &&
          wall_sensor_destinations.resolved_destination_count == 4 &&
          wall_sensor_destinations.unique_destination_count == 2 &&
          wall_sensor_destinations.duplicate_destination_count == 2 &&
          wall_sensor_destinations.zero_destination_count == 0 &&
          wall_sensor_destinations.nonzero_destination_count == 4 &&
          wall_sensor_destinations.highest_destination_x == 30U &&
          wall_sensor_destinations.highest_destination_y == 31U &&
          wall_sensor_destinations.highest_destination_orientation == 2U &&
          wall_sensor_destinations.complete &&
          !wall_sensor_destinations.destination_semantics_proven,
          "Structure1F wall-sensor destination tuples remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_wall_sensor_control_selector_receipt(
              &level, &wall_sensor_controls) == 0 &&
          wall_sensor_controls.structure1a_relation_complete &&
          wall_sensor_controls.wall_sensor_entry_count == 4 &&
          wall_sensor_controls.resolved_control_selector_count == 4 &&
          wall_sensor_controls.unique_control_selector_count == 2 &&
          wall_sensor_controls.duplicate_control_selector_count == 2 &&
          wall_sensor_controls.zero_control_selector_count == 0 &&
          wall_sensor_controls.nonzero_control_selector_count == 4 &&
          wall_sensor_controls.highest_control_selector == 2U &&
          wall_sensor_controls.complete &&
          !wall_sensor_controls.control_semantics_proven,
          "Structure1F wall-sensor controls remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_wall_sensor_control_destination_tuple_receipt(
              &level, &wall_sensor_control_destination_tuples) == 0 &&
          wall_sensor_control_destination_tuples.structure1a_relation_complete &&
          wall_sensor_control_destination_tuples.wall_sensor_entry_count == 4 &&
          wall_sensor_control_destination_tuples.resolved_tuple_count == 4 &&
          wall_sensor_control_destination_tuples.unique_tuple_count == 2 &&
          wall_sensor_control_destination_tuples.duplicate_tuple_count == 2 &&
          wall_sensor_control_destination_tuples.zero_tuple_count == 0 &&
          wall_sensor_control_destination_tuples.nonzero_tuple_count == 4 &&
          wall_sensor_control_destination_tuples.highest_tuple == 0x021e1f02U &&
          wall_sensor_control_destination_tuples.complete &&
          !wall_sensor_control_destination_tuples.tuple_semantics_proven,
          "Structure1F wall-sensor control/destination tuples remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_wall_sensor_model_rotation_pair_receipt(
              &level, &wall_sensor_model_rotation_pairs) == 0 &&
          wall_sensor_model_rotation_pairs.structure1a_relation_complete &&
          wall_sensor_model_rotation_pairs.wall_sensor_entry_count == 4 &&
          wall_sensor_model_rotation_pairs.resolved_pair_count == 4 &&
          wall_sensor_model_rotation_pairs.unique_pair_count == 2 &&
          wall_sensor_model_rotation_pairs.duplicate_pair_count == 2 &&
          wall_sensor_model_rotation_pairs.zero_pair_count == 0 &&
          wall_sensor_model_rotation_pairs.nonzero_pair_count == 4 &&
          wall_sensor_model_rotation_pairs.highest_pair == 0x0904U &&
          wall_sensor_model_rotation_pairs.complete &&
          !wall_sensor_model_rotation_pairs.pair_semantics_proven,
          "Structure1F wall-sensor model/rotation pairs remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_wall_decoration_model_rotation_pair_receipt(
              &level, &wall_decoration_model_rotation_pairs) == 0 &&
          wall_decoration_model_rotation_pairs.structure1a_relation_complete &&
          wall_decoration_model_rotation_pairs.wall_decoration_entry_count == 2 &&
          wall_decoration_model_rotation_pairs.resolved_pair_count == 2 &&
          wall_decoration_model_rotation_pairs.unique_pair_count == 2 &&
          wall_decoration_model_rotation_pairs.duplicate_pair_count == 0 &&
          wall_decoration_model_rotation_pairs.zero_pair_count == 0 &&
          wall_decoration_model_rotation_pairs.nonzero_pair_count == 2 &&
          wall_decoration_model_rotation_pairs.highest_pair == 0x0503U &&
          wall_decoration_model_rotation_pairs.complete &&
          !wall_decoration_model_rotation_pairs.pair_semantics_proven,
          "Structure1F wall-decoration model/rotation pairs remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_alcove_payload_selector_receipt(
              &level, &alcove_payload_selectors) == 0 &&
          alcove_payload_selectors.structure1a_relation_complete &&
          alcove_payload_selectors.alcove_entry_count == 2 &&
          alcove_payload_selectors.resolved_payload_selector_count == 2 &&
          alcove_payload_selectors.unique_payload_selector_count == 1 &&
          alcove_payload_selectors.duplicate_payload_selector_count == 1 &&
          alcove_payload_selectors.zero_payload_selector_count == 0 &&
          alcove_payload_selectors.nonzero_payload_selector_count == 2 &&
          alcove_payload_selectors.highest_payload_selector == 6U &&
          alcove_payload_selectors.complete &&
          !alcove_payload_selectors.payload_semantics_proven,
          "Structure1F alcove payload selectors remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_alcove_payload_rotation_pair_receipt(
              &level, &alcove_payload_rotation_pairs) == 0 &&
          alcove_payload_rotation_pairs.structure1a_relation_complete &&
          alcove_payload_rotation_pairs.alcove_entry_count == 2 &&
          alcove_payload_rotation_pairs.resolved_pair_count == 2 &&
          alcove_payload_rotation_pairs.unique_pair_count == 1 &&
          alcove_payload_rotation_pairs.duplicate_pair_count == 1 &&
          alcove_payload_rotation_pairs.zero_pair_count == 0 &&
          alcove_payload_rotation_pairs.nonzero_pair_count == 2 &&
          alcove_payload_rotation_pairs.highest_pair == 0x0601U &&
          alcove_payload_rotation_pairs.complete &&
          !alcove_payload_rotation_pairs.pair_semantics_proven,
          "Structure1F alcove payload/rotation pairs remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_floor_sensor_control_selector_receipt(
              &level, &floor_sensor_controls) == 0 &&
          floor_sensor_controls.structure1f_spatial_valid &&
          floor_sensor_controls.floor_sensor_entry_count == 2 &&
          floor_sensor_controls.resolved_control_selector_count == 2 &&
          floor_sensor_controls.unique_control_selector_count == 2 &&
          floor_sensor_controls.duplicate_control_selector_count == 0 &&
          floor_sensor_controls.zero_control_selector_count == 1 &&
          floor_sensor_controls.nonzero_control_selector_count == 1 &&
          floor_sensor_controls.highest_control_selector == 3U &&
          floor_sensor_controls.complete &&
          !floor_sensor_controls.control_semantics_proven,
          "Structure1F floor-sensor controls remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_floor_sensor_destination_receipt(
              &level, &floor_sensor_destinations) == 0 &&
          floor_sensor_destinations.structure1f_spatial_valid &&
          floor_sensor_destinations.floor_sensor_entry_count == 2 &&
          floor_sensor_destinations.resolved_destination_count == 2 &&
          floor_sensor_destinations.unique_destination_count == 2 &&
          floor_sensor_destinations.duplicate_destination_count == 0 &&
          floor_sensor_destinations.zero_destination_count == 1 &&
          floor_sensor_destinations.nonzero_destination_count == 1 &&
          floor_sensor_destinations.highest_destination_x == 30U &&
          floor_sensor_destinations.highest_destination_y == 31U &&
          floor_sensor_destinations.highest_destination_orientation == 2U &&
          floor_sensor_destinations.complete &&
          !floor_sensor_destinations.destination_semantics_proven,
          "Structure1F floor-sensor destinations remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_floor_sensor_model_rotation_pair_receipt(
              &level, &floor_sensor_model_rotation_pairs) == 0 &&
          floor_sensor_model_rotation_pairs.structure1f_spatial_valid &&
          floor_sensor_model_rotation_pairs.floor_sensor_entry_count == 2 &&
          floor_sensor_model_rotation_pairs.resolved_pair_count == 2 &&
          floor_sensor_model_rotation_pairs.unique_pair_count == 2 &&
          floor_sensor_model_rotation_pairs.duplicate_pair_count == 0 &&
          floor_sensor_model_rotation_pairs.zero_pair_count == 1 &&
          floor_sensor_model_rotation_pairs.nonzero_pair_count == 1 &&
          floor_sensor_model_rotation_pairs.highest_pair == 0x2728U &&
          floor_sensor_model_rotation_pairs.complete &&
          !floor_sensor_model_rotation_pairs.pair_semantics_proven,
          "Structure1F floor-sensor model/rotation pairs remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_floor_sensor_extent_pair_receipt(
              &level, &floor_sensor_extent_pairs) == 0 &&
          floor_sensor_extent_pairs.structure1f_spatial_valid &&
          floor_sensor_extent_pairs.floor_sensor_entry_count == 2 &&
          floor_sensor_extent_pairs.resolved_pair_count == 2 &&
          floor_sensor_extent_pairs.unique_pair_count == 2 &&
          floor_sensor_extent_pairs.duplicate_pair_count == 0 &&
          floor_sensor_extent_pairs.zero_pair_count +
              floor_sensor_extent_pairs.nonzero_pair_count == 2 &&
          floor_sensor_extent_pairs.highest_pair == 0x5028U &&
          floor_sensor_extent_pairs.complete &&
          !floor_sensor_extent_pairs.pair_semantics_proven,
          "Structure1F floor-sensor extent pairs remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_floor_decoration_payload_selector_receipt(
              &level, &floor_decoration_payloads) == 0 &&
          floor_decoration_payloads.structure1f_spatial_valid &&
          floor_decoration_payloads.floor_decoration_entry_count == 2 &&
          floor_decoration_payloads.resolved_payload_selector_count == 2 &&
          floor_decoration_payloads.unique_payload_selector_count == 2 &&
          floor_decoration_payloads.duplicate_payload_selector_count == 0 &&
          floor_decoration_payloads.zero_payload_selector_count == 1 &&
          floor_decoration_payloads.nonzero_payload_selector_count == 1 &&
          floor_decoration_payloads.highest_payload_selector == 0x28U &&
          floor_decoration_payloads.complete &&
          !floor_decoration_payloads.payload_semantics_proven,
          "Structure1F floor-decoration payloads remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_floor_decoration_rotation_selector_receipt(
              &level, &floor_decoration_rotations) == 0 &&
          floor_decoration_rotations.structure1f_spatial_valid &&
          floor_decoration_rotations.floor_decoration_entry_count == 2 &&
          floor_decoration_rotations.resolved_rotation_selector_count == 2 &&
          floor_decoration_rotations.unique_rotation_selector_count == 1 &&
          floor_decoration_rotations.duplicate_rotation_selector_count == 1 &&
          floor_decoration_rotations.zero_rotation_selector_count == 2 &&
          floor_decoration_rotations.nonzero_rotation_selector_count == 0 &&
          floor_decoration_rotations.complete &&
          !floor_decoration_rotations.rotation_semantics_proven,
          "Structure1F floor-decoration rotations remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_floor_decoration_model_rotation_pair_receipt(
              &level, &floor_decoration_model_rotation_pairs) == 0 &&
          floor_decoration_model_rotation_pairs.structure1f_spatial_valid &&
          floor_decoration_model_rotation_pairs.floor_decoration_entry_count == 2 &&
          floor_decoration_model_rotation_pairs.resolved_pair_count == 2 &&
          floor_decoration_model_rotation_pairs.unique_pair_count == 2 &&
          floor_decoration_model_rotation_pairs.duplicate_pair_count == 0 &&
          floor_decoration_model_rotation_pairs.zero_pair_count == 1 &&
          floor_decoration_model_rotation_pairs.nonzero_pair_count == 1 &&
          floor_decoration_model_rotation_pairs.highest_pair == 0x2800U &&
          floor_decoration_model_rotation_pairs.complete &&
          !floor_decoration_model_rotation_pairs.pair_semantics_proven,
          "Structure1F floor-decoration model/rotation pairs remain no-draw provenance");
    CHECK(nexus_v1_level_structure1f_floor_decoration_control_extent_receipt(
              &level, &floor_decoration_control_extents) == 0 &&
          floor_decoration_control_extents.structure1f_spatial_valid &&
          floor_decoration_control_extents.floor_decoration_entry_count == 2 &&
          floor_decoration_control_extents.resolved_tuple_count == 2 &&
          floor_decoration_control_extents.unique_tuple_count == 2 &&
          floor_decoration_control_extents.duplicate_tuple_count == 0 &&
          floor_decoration_control_extents.zero_tuple_count +
              floor_decoration_control_extents.nonzero_tuple_count == 2 &&
          floor_decoration_control_extents.highest_type_or_control >= 3U &&
          floor_decoration_control_extents.highest_width >= 14U &&
          floor_decoration_control_extents.highest_height >= 15U &&
          floor_decoration_control_extents.complete &&
          !floor_decoration_control_extents.tuple_semantics_proven,
          "Structure1F floor-decoration control/extents remain no-draw provenance");
    CHECK(nexus_v1_level_structure3_model_reference_receipt(
              &level, &structure3_model_references) == 0 &&
          structure3_model_references.structure1a_relation_complete &&
          structure3_model_references.structure1f_bound_entry_count == 8 &&
          structure3_model_references.resolved_model_reference_count == 8 &&
          structure3_model_references.unique_model_index_count == 7 &&
          structure3_model_references.duplicate_model_index_count == 1 &&
          structure3_model_references.zero_model_index_count == 0 &&
          structure3_model_references.nonzero_model_index_count == 8 &&
          structure3_model_references.complete,
          "resolved Structure1F owners retain Structure3 indexes as no-draw provenance");
    CHECK(nexus_v1_level_structure3_payload_receipt(
              &level, &structure3_payload) == 0 &&
          structure3_payload.declared && structure3_payload.valid &&
          structure3_payload.block_offset == 20 &&
          structure3_payload.block_count == 4 &&
          structure3_payload.byte_offset == NEXUS_DGN_BLOCK_SIZE * 20 &&
          structure3_payload.byte_size == NEXUS_DGN_BLOCK_SIZE * 4 &&
          structure3_payload.zero_byte_count == NEXUS_DGN_BLOCK_SIZE * 4 - 4 &&
          structure3_payload.nonzero_byte_count == 4 &&
          structure3_payload.distinct_byte_value_count == 4 &&
          structure3_payload.byte_transition_count == 6 &&
          structure3_payload.first_nonzero_byte_offset == 1 &&
          structure3_payload.last_nonzero_byte_offset ==
              NEXUS_DGN_BLOCK_SIZE * 3 + 7 &&
          structure3_payload.nonzero_byte_run_count == 3 &&
          structure3_payload.longest_nonzero_byte_run == 2 &&
          structure3_payload.first_nonzero_byte_run_offset == 1 &&
          structure3_payload.first_nonzero_byte_run_byte_count == 2 &&
          structure3_payload.last_nonzero_byte_run_offset ==
              NEXUS_DGN_BLOCK_SIZE * 3 + 7 &&
          structure3_payload.last_nonzero_byte_run_byte_count == 1 &&
          structure3_payload.complete_block_count == 4 &&
          structure3_payload.zero_block_count == 1 &&
          structure3_payload.nonzero_block_count == 3 &&
          structure3_payload.first_nonzero_block_index == 0 &&
          structure3_payload.last_nonzero_block_index == 3 &&
          structure3_payload.nonzero_block_run_count == 2 &&
          structure3_payload.longest_nonzero_block_run == 2 &&
          structure3_payload.first_nonzero_block_run_start_block_index == 0 &&
          structure3_payload.first_nonzero_block_run_block_count == 2 &&
          structure3_payload.last_nonzero_block_run_start_block_index == 3 &&
          structure3_payload.last_nonzero_block_run_block_count == 1 &&
          structure3_payload.raw_payload_hash != 0U &&
          !structure3_payload.face_semantics_proven,
          "Structure3 payload retains documented block boundaries without face semantics");
    CHECK(nexus_v1_level_structure3_ordinal_correlation_receipt(
              &level, &structure3_correlation) == 0 &&
          structure3_correlation.valid &&
          structure3_correlation.structure1a_relation_complete &&
          structure3_correlation.structure3_payload_valid &&
          structure3_correlation.resolved_model_reference_count == 8 &&
          structure3_correlation.highest_model_index == 0x28 &&
          structure3_correlation.structure3_block_count == 4 &&
          structure3_correlation.structure3_nonzero_byte_run_count == 3 &&
          structure3_correlation.structure3_nonzero_block_run_count == 2 &&
          structure3_correlation.model_index_exceeds_block_count == 8 &&
          structure3_correlation.model_index_exceeds_nonzero_byte_run_count == 8 &&
          structure3_correlation.model_index_exceeds_nonzero_block_run_count == 8 &&
          structure3_correlation.zero_based_block_ordinal_mapping_disproven &&
          structure3_correlation.one_based_block_ordinal_mapping_disproven &&
          structure3_correlation.zero_based_byte_run_ordinal_mapping_disproven &&
          structure3_correlation.one_based_byte_run_ordinal_mapping_disproven &&
          structure3_correlation.zero_based_run_ordinal_mapping_disproven &&
          structure3_correlation.one_based_run_ordinal_mapping_disproven &&
          structure3_correlation.direct_block_ordinal_mapping_disproven &&
          structure3_correlation.direct_byte_run_ordinal_mapping_disproven &&
          structure3_correlation.direct_run_ordinal_mapping_disproven &&
          !structure3_correlation.face_semantics_proven,
          "Structure1A model indexes cannot become direct Structure3 byte, block, or run ordinals");
    for (int entry = 0; entry < level.structure1f_entry_count; ++entry) {
        if (level.structure1f_entries[entry].family >=
            NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) {
            level.structure1f_entries[entry].structure1a_structure3_model_index = 3U;
        }
    }
    CHECK(nexus_v1_level_structure3_ordinal_correlation_receipt(
              &level, &structure3_correlation) == 0 &&
          structure3_correlation.zero_based_byte_run_ordinal_mapping_disproven &&
          !structure3_correlation.one_based_byte_run_ordinal_mapping_disproven &&
          !structure3_correlation.direct_byte_run_ordinal_mapping_disproven &&
          !structure3_correlation.zero_based_block_ordinal_mapping_disproven &&
          !structure3_correlation.one_based_block_ordinal_mapping_disproven &&
          structure3_correlation.zero_based_run_ordinal_mapping_disproven &&
          structure3_correlation.one_based_run_ordinal_mapping_disproven &&
          structure3_correlation.direct_run_ordinal_mapping_disproven,
          "Structure3 ordinal receipts keep zero- and one-based exclusions separate");
    for (int entry = 0; entry < level.structure1f_entry_count; ++entry) {
        if (level.structure1f_entries[entry].family >=
            NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) {
            level.structure1f_entries[entry].structure1a_structure3_model_index =
                (uint8_t)(0x20 + level.structure1f_entries[entry].structure1a_index);
        }
    }
    level.structure1f_entries[7].structure1a_index = 9U;
    level.structure1f_entries[7].structure1a_relation_valid = 0;
    CHECK(nexus_v1_level_structure1a_relation_receipt(&level,
                                                       &structure1a_relation) == 0 &&
          !structure1a_relation.complete &&
          structure1a_relation.out_of_range_index_count == 1,
          "out-of-range Structure1F references remain fail-closed");
    level.structure1f_entries[7].structure1a_index = 3U;
    level.structure1f_entries[7].structure1a_relation_valid = 1;
    CHECK(nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0 &&
          handoff.structure1f_valid && handoff.structure1f_total_entry_count == 14 &&
          handoff.structure1f_typed_entry_count == level.structure1f_entry_count &&
          handoff.structure1f_spatial.valid &&
          handoff.structure1f_spatial.direct_coordinate_entry_count == 6 &&
          handoff.structure1f_spatial.structure1a_bound_entry_count == 8 &&
          handoff.structure1a_boundary.valid &&
          handoff.structure1a_boundary.entry_count == 8 &&
          handoff.structure1a_boundary.duplicate_index_count == 1 &&
          handoff.structure1a_relation.complete &&
          handoff.structure1a_relation.resolved_entry_count == 8 &&
          handoff.structure1a_kinds.complete &&
          handoff.structure1a_kinds.unique_kind_count == 3 &&
          !handoff.structure1a_kinds.kind_semantics_proven &&
          handoff.structure3_model_references.complete &&
          handoff.structure3_model_references.unique_model_index_count == 7 &&
          handoff.structure1a_transform_selectors.complete &&
          handoff.structure1a_transform_selectors.unique_selector_count == 4 &&
          !handoff.structure1a_transform_selectors.transform_semantics_proven &&
          handoff.structure1f_face_selectors.complete &&
          handoff.structure1f_face_selectors.unique_face_selector_count == 4 &&
          !handoff.structure1f_face_selectors.face_semantics_proven &&
          handoff.structure1f_rotation_selectors.complete &&
          handoff.structure1f_rotation_selectors.unique_rotation_selector_count == 4 &&
          !handoff.structure1f_rotation_selectors.rotation_semantics_proven &&
          handoff.structure1f_face_rotation_pairs.complete &&
          handoff.structure1f_face_rotation_pairs.unique_pair_count == 4 &&
          !handoff.structure1f_face_rotation_pairs.pair_semantics_proven &&
          handoff.structure1f_offset_pairs.complete &&
          handoff.structure1f_offset_pairs.unique_offset_pair_count == 4 &&
          !handoff.structure1f_offset_pairs.offset_semantics_proven &&
          handoff.structure1f_wall_payload_selectors.complete &&
          handoff.structure1f_wall_payload_selectors.unique_payload_selector_count == 3 &&
          !handoff.structure1f_wall_payload_selectors.payload_semantics_proven &&
          handoff.structure1f_wall_sensor_destinations.complete &&
          handoff.structure1f_wall_sensor_destinations.unique_destination_count == 2 &&
          !handoff.structure1f_wall_sensor_destinations.destination_semantics_proven &&
          handoff.structure1f_wall_sensor_control_selectors.complete &&
          handoff.structure1f_wall_sensor_control_selectors.unique_control_selector_count == 2 &&
          !handoff.structure1f_wall_sensor_control_selectors.control_semantics_proven &&
          handoff.structure1f_wall_sensor_control_destination_tuples.complete &&
          handoff.structure1f_wall_sensor_control_destination_tuples.unique_tuple_count == 2 &&
          !handoff.structure1f_wall_sensor_control_destination_tuples.tuple_semantics_proven &&
          handoff.structure1f_wall_sensor_model_rotation_pairs.complete &&
          handoff.structure1f_wall_sensor_model_rotation_pairs.unique_pair_count == 2 &&
          !handoff.structure1f_wall_sensor_model_rotation_pairs.pair_semantics_proven &&
          handoff.structure1f_wall_decoration_model_rotation_pairs.complete &&
          handoff.structure1f_wall_decoration_model_rotation_pairs.unique_pair_count == 2 &&
          !handoff.structure1f_wall_decoration_model_rotation_pairs.pair_semantics_proven &&
          handoff.structure1f_alcove_payload_selectors.complete &&
          handoff.structure1f_alcove_payload_selectors.unique_payload_selector_count == 1 &&
          !handoff.structure1f_alcove_payload_selectors.payload_semantics_proven &&
          handoff.structure1f_alcove_payload_rotation_pairs.complete &&
          handoff.structure1f_alcove_payload_rotation_pairs.unique_pair_count == 1 &&
          !handoff.structure1f_alcove_payload_rotation_pairs.pair_semantics_proven &&
          handoff.structure1f_floor_sensor_control_selectors.complete &&
          handoff.structure1f_floor_sensor_control_selectors.unique_control_selector_count == 2 &&
          !handoff.structure1f_floor_sensor_control_selectors.control_semantics_proven &&
          handoff.structure1f_floor_sensor_destinations.complete &&
          handoff.structure1f_floor_sensor_destinations.unique_destination_count == 2 &&
          !handoff.structure1f_floor_sensor_destinations.destination_semantics_proven &&
          handoff.structure1f_floor_sensor_model_rotation_pairs.complete &&
          handoff.structure1f_floor_sensor_model_rotation_pairs.unique_pair_count == 2 &&
          !handoff.structure1f_floor_sensor_model_rotation_pairs.pair_semantics_proven &&
          handoff.structure1f_floor_sensor_extent_pairs.complete &&
          handoff.structure1f_floor_sensor_extent_pairs.unique_pair_count == 2 &&
          !handoff.structure1f_floor_sensor_extent_pairs.pair_semantics_proven &&
          handoff.structure1f_floor_decoration_payload_selectors.complete &&
          handoff.structure1f_floor_decoration_payload_selectors.unique_payload_selector_count == 2 &&
          !handoff.structure1f_floor_decoration_payload_selectors.payload_semantics_proven &&
          handoff.structure1f_floor_decoration_rotation_selectors.complete &&
          !handoff.structure1f_floor_decoration_rotation_selectors.rotation_semantics_proven &&
          handoff.structure1f_floor_decoration_model_rotation_pairs.complete &&
          handoff.structure1f_floor_decoration_model_rotation_pairs.unique_pair_count == 2 &&
          !handoff.structure1f_floor_decoration_model_rotation_pairs.pair_semantics_proven &&
          handoff.structure1f_floor_decoration_control_extents.complete &&
          handoff.structure1f_floor_decoration_control_extents.unique_tuple_count == 2 &&
          !handoff.structure1f_floor_decoration_control_extents.tuple_semantics_proven &&
          handoff.structure1f_floor_decoration_offset_pairs.complete &&
          handoff.structure1f_floor_decoration_offset_pairs.unique_pair_count == 2 &&
          handoff.structure1f_floor_decoration_offset_pairs.nonzero_pair_count == 1 &&
          !handoff.structure1f_floor_decoration_offset_pairs.offset_semantics_proven &&
          handoff.structure1f_item_attribute_pairs.complete &&
          handoff.structure1f_item_attribute_pairs.unique_pair_count == 2 &&
          !handoff.structure1f_item_attribute_pairs.semantics_proven &&
          handoff.structure1f_item_location_pairs.complete &&
          handoff.structure1f_item_location_pairs.unique_pair_count == 2 &&
          !handoff.structure1f_item_location_pairs.semantics_proven &&
          handoff.structure1f_item_coordinate_pairs.complete &&
          handoff.structure1f_item_coordinate_pairs.unique_pair_count == 2 &&
          !handoff.structure1f_item_coordinate_pairs.semantics_proven &&
          handoff.structure1f_family_count[NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS] == 4,
          "Structure1F typed records are consumed by the no-fallback host handoff");
    CHECK(handoff.status ==
              NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE3_FACE_SEMANTICS &&
          handoff.blocks_real_dgn_mesh_render && !handoff.fallback_visuals_permitted &&
          strcmp(nexus_v1_dgn_renderer_handoff_status_name(handoff.status),
                 "blocked-structure3-face-semantics") == 0,
          "bounded Structure3 payload blocks until original face semantics are proven");
    memset(commands, 0x5a, sizeof(commands));
    CHECK(nexus_v1_level_build_dgn_view_render_plan(
          &level, 10, 10, 0, commands,
              NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS, &render_plan) == 0 &&
          render_plan.status ==
              NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE3_FACE_SEMANTICS &&
          !render_plan.plan_ready &&
          render_plan.blocks_real_dgn_mesh_render &&
          !render_plan.fallback_visuals_permitted &&
          render_plan.structure1a_kinds.complete &&
          !render_plan.structure1a_kinds.kind_semantics_proven &&
          render_plan.structure3_model_references.complete &&
          render_plan.structure3_face_materials.face_receipt_valid ==
              handoff.structure3_face_materials.face_receipt_valid &&
          render_plan.structure3_face_materials.face_count ==
              handoff.structure3_face_materials.face_count &&
          render_plan.structure3_face_materials.textured_face_count ==
              handoff.structure3_face_materials.textured_face_count &&
          render_plan.structure3_face_materials.static_texture_unbound_count ==
              handoff.structure3_face_materials.static_texture_unbound_count &&
          !render_plan.structure3_face_materials.material_or_draw_semantics_proven &&
          render_plan.structure3_face_normal_pairs.face_receipt_valid ==
              handoff.structure3_face_normal_pairs.face_receipt_valid &&
          render_plan.structure3_face_normal_pairs.vector_receipt_valid ==
              handoff.structure3_face_normal_pairs.vector_receipt_valid &&
          render_plan.structure3_face_normal_pairs.face_normal_pair_count ==
              handoff.structure3_face_normal_pairs.face_normal_pair_count &&
          !render_plan.structure3_face_normal_pairs.normal_plane_or_draw_semantics_proven &&
          render_plan.structure1a_transform_selectors.complete &&
          !render_plan.structure1a_transform_selectors.transform_semantics_proven &&
          render_plan.structure1f_face_selectors.complete &&
          !render_plan.structure1f_face_selectors.face_semantics_proven &&
          render_plan.structure1f_rotation_selectors.complete &&
          !render_plan.structure1f_rotation_selectors.rotation_semantics_proven &&
          render_plan.structure1f_face_rotation_pairs.complete &&
          !render_plan.structure1f_face_rotation_pairs.pair_semantics_proven &&
          render_plan.structure1f_offset_pairs.complete &&
          !render_plan.structure1f_offset_pairs.offset_semantics_proven &&
          render_plan.structure1f_wall_payload_selectors.complete &&
          !render_plan.structure1f_wall_payload_selectors.payload_semantics_proven &&
          render_plan.structure1f_wall_sensor_destinations.complete &&
          !render_plan.structure1f_wall_sensor_destinations.destination_semantics_proven &&
          render_plan.structure1f_wall_sensor_control_selectors.complete &&
          !render_plan.structure1f_wall_sensor_control_selectors.control_semantics_proven &&
          render_plan.structure1f_wall_sensor_control_destination_tuples.complete &&
          !render_plan.structure1f_wall_sensor_control_destination_tuples.tuple_semantics_proven &&
          render_plan.structure1f_wall_sensor_model_rotation_pairs.complete &&
          !render_plan.structure1f_wall_sensor_model_rotation_pairs.pair_semantics_proven &&
          render_plan.structure1f_wall_decoration_model_rotation_pairs.complete &&
          !render_plan.structure1f_wall_decoration_model_rotation_pairs.pair_semantics_proven &&
          render_plan.structure1f_alcove_payload_selectors.complete &&
          !render_plan.structure1f_alcove_payload_selectors.payload_semantics_proven &&
          render_plan.structure1f_alcove_payload_rotation_pairs.complete &&
          !render_plan.structure1f_alcove_payload_rotation_pairs.pair_semantics_proven &&
          render_plan.structure1f_floor_sensor_control_selectors.complete &&
          !render_plan.structure1f_floor_sensor_control_selectors.control_semantics_proven &&
          render_plan.structure1f_floor_sensor_destinations.complete &&
          !render_plan.structure1f_floor_sensor_destinations.destination_semantics_proven &&
          render_plan.structure1f_floor_sensor_model_rotation_pairs.complete &&
          !render_plan.structure1f_floor_sensor_model_rotation_pairs.pair_semantics_proven &&
          render_plan.structure1f_floor_sensor_extent_pairs.complete &&
          !render_plan.structure1f_floor_sensor_extent_pairs.pair_semantics_proven &&
          render_plan.structure1f_floor_decoration_payload_selectors.complete &&
          !render_plan.structure1f_floor_decoration_payload_selectors.payload_semantics_proven &&
          render_plan.structure1f_floor_decoration_rotation_selectors.complete &&
          !render_plan.structure1f_floor_decoration_rotation_selectors.rotation_semantics_proven &&
          render_plan.structure1f_floor_decoration_model_rotation_pairs.complete &&
          !render_plan.structure1f_floor_decoration_model_rotation_pairs.pair_semantics_proven &&
          render_plan.structure1f_floor_decoration_control_extents.complete &&
          render_plan.structure1f_floor_decoration_control_extents.unique_tuple_count == 2 &&
          !render_plan.structure1f_floor_decoration_control_extents.tuple_semantics_proven &&
          render_plan.structure1f_floor_decoration_offset_pairs.complete &&
          render_plan.structure1f_floor_decoration_offset_pairs.unique_pair_count == 2 &&
          render_plan.structure1f_floor_decoration_offset_pairs.nonzero_pair_count == 1 &&
          !render_plan.structure1f_floor_decoration_offset_pairs.offset_semantics_proven &&
          render_plan.structure1f_item_attribute_pairs.complete &&
          render_plan.structure1f_item_attribute_pairs.unique_pair_count == 2 &&
          !render_plan.structure1f_item_attribute_pairs.semantics_proven &&
          render_plan.structure1f_item_location_pairs.complete &&
          render_plan.structure1f_item_location_pairs.unique_pair_count == 2 &&
          !render_plan.structure1f_item_location_pairs.semantics_proven &&
          render_plan.structure1f_item_coordinate_pairs.complete &&
          render_plan.structure1f_item_coordinate_pairs.unique_pair_count == 2 &&
          !render_plan.structure1f_item_coordinate_pairs.semantics_proven &&
          render_plan.structure3_payload.valid &&
          /* The opaque Structure3 payload above cannot bind face/material
           * provenance, so the no-draw gate correctly emits zero source
           * commands while retaining every bounded receipt. */
          render_plan.command_count == 0 &&
          render_plan.blocks_real_dgn_mesh_render && !render_plan.plan_ready &&
          !render_plan.fallback_visuals_permitted,
          "DGN render planning retains bounded Structure3 selector and normal receipts without a mesh draw");
    wb16(dgn + 0x1c, 24U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) != 0,
          "out-of-file Structure3 payload envelopes fail closed during DGN load");
    wb16(dgn + 0x1c, 20U);
    wb16(dgn + 0x1e, 0U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) != 0,
          "partial Structure3 payload headers fail closed during DGN load");
    wb16(dgn + 0x1e, 4U);
    structure1[structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 312 +
               NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 1] = 64U;
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0 &&
          handoff.structure1f_declared && !handoff.structure1f_valid &&
          handoff.status ==
              NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_LAYOUT &&
          handoff.blocks_real_dgn_mesh_render &&
          !handoff.fallback_visuals_permitted &&
          strcmp(nexus_v1_dgn_renderer_handoff_status_name(handoff.status),
                 "blocked-structure1f-layout") == 0,
          "declared malformed Structure1F coordinates block host handoff");
    structure1[structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 312 +
               NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 1] = 10U;
    structure1[structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 312 + 16] = 0x13U;
    CHECK(nexus_v1_dgn_structure1_layout(&layout, dgn, (int)sizeof(dgn)) == 0 &&
          !layout.structure1f.valid,
          "Structure1F rejects a family tag that is not documented by the original format");
}

static void test_structure3_entry_header_boundaries(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 25];
    uint8_t *payload = dgn + NEXUS_DGN_BLOCK_SIZE * 21;
    Nexus_V1_Level level;
    Nexus_V1_DgnStructure3EntryHeaderReceipt headers;
    Nexus_V1_DgnStructure3FaceReceipt faces;
    Nexus_V1_DgnStructure3FaceMaterialReceipt materials;
    Nexus_V1_DgnStructure3VectorReceipt vectors;
    Nexus_V1_DgnStructure3FaceNormalPairReceipt pairs;
    Nexus_V1_DgnStructure3MeshSemanticHandoffReceipt mesh_semantics;
    Nexus_V1_DgnRendererHandoffReceipt handoff;
    Nexus_V1_DgnStructure3FaceCaptureCandidate candidate;
    Nexus_V1_DgnStructure3FaceCaptureBindingReceipt capture;
    uint8_t vdp1_command[NEXUS_V1_VDP1_COMMAND_BYTES];
    const uint8_t texture_span[] = { 0x12U, 0x34U, 0x56U, 0x78U };
    const uint8_t palette_state[] = { 0x56U };
    static uint8_t vdp1_state[NEXUS_V1_VDP1_VRAM_BYTES];
    const uint8_t transform_state[] = { 0x9aU };
    const uint8_t culling_state[] = { 0xbcU };

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19, 0x200, 512) == 0,
          "Structure3 entry-header fixture builds");
    wb16(dgn + 0x1c, 21U);
    wb16(dgn + 0x1e, 4U);
    wb32(payload, 2U);
    wb32(payload + 4, 16U);
    wb32(payload + 8, 152U);
    wb32(payload + 16, 0x100U);
    wb16(payload + 20, 4U);
    wb16(payload + 22, 2U);
    wb32(payload + 24, 56U);
    wb32(payload + 32, 104U);
    wb32(payload + 36, 128U);
    wb16(payload + 156, 1U);
    wb16(payload + 158, 2U);
    wb32(payload + 160, 192U);
    wb32(payload + 168, 204U);
    wb32(payload + 172, 228U);

    /* Structure3a/3c signed 16.16 X/Y/Z rows. */
    wb32(payload + 56, 65536U);
    wb32(payload + 68, 65536U);
    wb32(payload + 80, 65536U);
    wb32(payload + 92, 65536U);
    wb32(payload + 192, 65536U);
    wb32(payload + 128, 65536U);
    wb32(payload + 140, 65536U);
    wb32(payload + 228, 65536U);
    wb32(payload + 240, 65536U);

    /* Structure3b face rows: all four rows are triangles. Entry 0 holds two
     * disconnected two-index components; entry 1 carries one textured
     * one-index component. All indexes stay inside their own vertex table. */
    wb16(payload + 104, 0U);
    wb16(payload + 106, 1U);
    wb16(payload + 108, 0U);
    wb16(payload + 110, 0U);
    wb16(payload + 116, 2U);
    wb16(payload + 118, 3U);
    wb16(payload + 120, 2U);
    wb16(payload + 122, 2U);
    wb16(payload + 204, 0U);
    wb16(payload + 206, 0U);
    wb16(payload + 208, 0U);
    wb16(payload + 210, 0U);
    wb16(payload + 216, 0U);
    wb16(payload + 218, 0U);
    wb16(payload + 220, 0U);
    wb16(payload + 222, 0U);
    payload[212] = 0x40U;

    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 0) == 0 &&
          nexus_v1_level_structure3_entry_header_receipt(&level, &headers) == 0 &&
          headers.payload_valid && headers.directory_valid && headers.valid &&
          headers.entry_count == 2 && headers.bounded_entry_count == 2 &&
          headers.fixed_header_byte_count ==
              NEXUS_DGN_STRUCTURE3_ENTRY_HEADER_BYTES &&
          headers.first_region_element_count == 5 &&
          headers.second_region_element_count == 4 &&
          headers.third_region_element_count == 4 &&
          headers.complete_third_region_entry_count == 2 &&
          headers.zero_tag_entry_count == 1 && headers.tag_0x100_entry_count == 1 &&
          headers.other_tag_entry_count == 0 && headers.boundaries_valid &&
          headers.third_region_boundaries_valid &&
          !headers.semantics_proven,
          "Structure3 entry framing bounds the paired third 12-byte region");
    CHECK(nexus_v1_level_structure3_face_receipt(&level, &faces) == 0 &&
          faces.entry_headers_valid && faces.valid && faces.vertex_count == 5 &&
          faces.face_count == 4 && faces.normal_count == 4 &&
          faces.triangle_count == 4 && faces.quad_count == 0 &&
          faces.face_vertex_reference_count == 12 &&
          faces.distinct_face_vertex_count == 6 &&
          faces.repeated_face_vertex_reference_count == 6 &&
          faces.one_distinct_vertex_face_count == 2 &&
          faces.two_distinct_vertex_face_count == 2 &&
          faces.three_distinct_vertex_face_count == 0 &&
          faces.four_distinct_vertex_face_count == 0 &&
          faces.linked_face_vertex_reference_count == 12 &&
          faces.referenced_vertex_count == 5 &&
          faces.unreferenced_vertex_count == 0 &&
          faces.fully_referenced_vertex_entry_count == 2 &&
          faces.partially_referenced_vertex_entry_count == 0 &&
          faces.zero_vertex_entry_count == 0 &&
          faces.face_vertex_component_count == 3 &&
          faces.zero_component_vertex_entry_count == 0 &&
          faces.single_component_vertex_entry_count == 1 &&
          faces.multiple_component_vertex_entry_count == 1 &&
          faces.face_vertex_cooccurrence_pair_count == 2 &&
          faces.face_vertex_adjacency_pair_count == 2 &&
          faces.repeated_face_vertex_adjacency_pair_count == 0 &&
          faces.single_face_vertex_adjacency_pair_count == 2 &&
          faces.shared_face_vertex_adjacency_pair_count == 0 &&
          faces.maximum_face_vertex_adjacency_pair_incidence == 1 &&
          faces.maximum_vertex_reference_count == 6 &&
          faces.textured_face_count == 1 && faces.static_texture_fill_count == 4 &&
          faces.face_vertex_indexes_valid && faces.face_vertex_linkage_valid &&
          faces.face_topology_accounting_valid &&
          faces.face_vertex_entry_coverage_accounting_valid &&
          faces.face_vertex_component_accounting_valid &&
          faces.face_vertex_component_entry_accounting_valid &&
          faces.face_vertex_adjacency_accounting_valid &&
          faces.face_vertex_adjacency_multiplicity_accounting_valid &&
          faces.normal_count_matches_face_count &&
          !faces.draw_semantics_proven,
          "Structure3 face rows retain raw distinct-index topology without authorizing a draw");
    CHECK(nexus_v1_level_structure3_face_material_receipt(&level, &materials) == 0 &&
          materials.face_receipt_valid && materials.valid &&
          materials.textured_face_count == 1 &&
          materials.static_texture_selector_count == 1 &&
          materials.static_texture_unbound_count == 1 &&
          !materials.selector_bindings_complete &&
          !materials.material_or_draw_semantics_proven,
          "an unbound Structure3 texture selector stays blocked without a draw claim");
    CHECK(nexus_v1_level_structure3_vector_receipt(&level, &vectors) == 0 &&
          vectors.face_receipt_valid && vectors.valid &&
          vectors.vertex_count == 5 && vectors.normal_count == 4 &&
          vectors.vertex_vector_count == 5 && vectors.nonzero_vertex_vector_count == 5 &&
          vectors.normal_vector_count == 4 && vectors.normal_unit_length_count == 4 &&
          vectors.normal_non_unit_length_count == 0 &&
          vectors.normal_face_plane_pair_count == 8 &&
          vectors.normal_face_plane_within_tolerance_count == 8 &&
          vectors.normal_face_plane_outside_tolerance_count == 0 &&
          vectors.degenerate_face_triangle_count == 4 &&
          vectors.zero_winding_triangle_count == 4 &&
          vectors.maximum_normal_length_error == 0 &&
          vectors.maximum_normal_face_plane_error == 0 &&
          !vectors.transform_or_draw_semantics_proven,
          "Structure3 signed 16.16 vectors remain bounded without a transform or draw claim");
    CHECK(nexus_v1_level_structure3_face_normal_pair_receipt(&level, &pairs) == 0 &&
          pairs.face_receipt_valid && pairs.vector_receipt_valid && pairs.valid &&
          pairs.entry_count == 2 && pairs.complete_entry_pair_count == 2 &&
          pairs.face_normal_pair_count == 4 &&
          pairs.unit_length_face_normal_pair_count == 4 &&
          pairs.non_unit_length_face_normal_pair_count == 0 &&
          !pairs.normal_plane_or_draw_semantics_proven,
          "Structure3 face/normal ordinals remain bounded no-draw correspondence");
    CHECK(nexus_v1_level_structure3_mesh_semantic_handoff_receipt(
              &level, &mesh_semantics) == 0 &&
          mesh_semantics.source_topology_valid &&
          mesh_semantics.source_vectors_valid &&
          mesh_semantics.source_face_normal_pairing_valid &&
          mesh_semantics.source_facts_complete &&
          mesh_semantics.entry_count == 2 && mesh_semantics.vertex_count == 5 &&
          mesh_semantics.face_count == 4 && mesh_semantics.normal_count == 4 &&
          mesh_semantics.face_normal_pair_count == 4 &&
          mesh_semantics.original_capture_required &&
          !mesh_semantics.original_capture_available &&
          !mesh_semantics.normal_plane_semantics_proven &&
          !mesh_semantics.transform_semantics_proven &&
          !mesh_semantics.texture_palette_semantics_proven &&
          !mesh_semantics.draw_semantics_proven &&
          !mesh_semantics.renderer_handoff_ready &&
          mesh_semantics.blocks_real_dgn_mesh_render,
          "Structure3 mesh handoff requires an original capture before drawing");
    CHECK(nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0 &&
          handoff.structure3_entry_headers.valid &&
          !handoff.structure3_entry_headers.semantics_proven &&
          !handoff.fallback_visuals_permitted,
          "Structure3 entry-header receipt cannot authorize a draw route");

    /* These hashes cover every future packet lane, but a fixture must not
     * certify itself by merely repeating its own DGN fingerprint. */
    memset(&candidate, 0, sizeof(candidate));
    memset(vdp1_command, 0, sizeof(vdp1_command));
    wl16(vdp1_command, 0U);      /* normal texture primitive */
    wl16(vdp1_command + 2, 0x50U); /* documented link address: 0x280 */
    wl16(vdp1_command + 8, 0x20U);
    wl16(vdp1_command + 10, 0x0101U); /* 8x1 command-table extent */
    wl16(vdp1_command + 12, (uint16_t)-12);
    wl16(vdp1_command + 14, 34U);
    wl16(vdp1_command + 16, 56U);
    wl16(vdp1_command + 18, (uint16_t)-78);
    wl16(vdp1_command + 20, 90U);
    wl16(vdp1_command + 22, 123U);
    wl16(vdp1_command + 24, (uint16_t)-145);
    wl16(vdp1_command + 26, 167U);
    wl16(vdp1_command + 28, 0x2468U);
    memset(vdp1_state, 0, sizeof(vdp1_state));
    memcpy(vdp1_state + 0x100U, texture_span, sizeof(texture_span));
    memcpy(vdp1_state + 0x200U, vdp1_command, sizeof(vdp1_command));
    wl16(vdp1_state + 0x280U, 0x8000U); /* linked end-command record */
    candidate.dgn_fnv1a64 = fnv1a64(dgn, sizeof(dgn));
    candidate.structure3_payload_fnv1a32 = level.structure3_payload.raw_payload_hash;
    candidate.typed_mesh_corpus_fnv1a32 = NEXUS_DGN_RETAIL_TYPED_MESH_CORPUS_FNV1A32;
    candidate.entry_index = 1U;
    candidate.face_ordinal = 0U;
    candidate.face_row_fnv1a32 = fnv1a32(payload + 204U, 12U);
    candidate.referenced_vertex_rows_fnv1a32 =
        fnv1a32_repeated(payload + 192U, 12U, 3);
    candidate.normal_row_fnv1a32 = fnv1a32(payload + 228U, 12U);
    candidate.texture_span_fnv1a64 = fnv1a64(texture_span, sizeof(texture_span));
    candidate.palette_state_fnv1a64 = fnv1a64(palette_state, sizeof(palette_state));
    candidate.vdp1_state_fnv1a64 = fnv1a64(vdp1_state, sizeof(vdp1_state));
    candidate.transform_state_fnv1a64 = fnv1a64(transform_state, sizeof(transform_state));
    candidate.normal_culling_state_fnv1a64 = fnv1a64(culling_state, sizeof(culling_state));
    candidate.vdp1_command_fnv1a64 = fnv1a64(vdp1_command, sizeof(vdp1_command));
    candidate.first_sequence = 1U;
    candidate.last_sequence = 2U;
    CHECK(nexus_v1_dgn_bind_structure3_face_capture_candidate(
              &level, dgn, (int)sizeof(dgn), 0, 0, &candidate,
              texture_span, (int)sizeof(texture_span),
              palette_state, (int)sizeof(palette_state),
              vdp1_state, (int)sizeof(vdp1_state),
              transform_state, (int)sizeof(transform_state),
              culling_state, (int)sizeof(culling_state),
              vdp1_command, (int)sizeof(vdp1_command), &capture) == 0 &&
          !capture.dgn_source_hash_verified && !capture.candidate_framing_valid &&
          !capture.complete_source_binding && capture.blocks_real_dgn_mesh_render,
          "Structure3 capture fingerprints cannot self-admit unverified DGN bytes");
    CHECK(nexus_v1_dgn_bind_structure3_face_capture_candidate(
              &level, dgn, (int)sizeof(dgn), 1, 0, &candidate,
              texture_span, (int)sizeof(texture_span),
              palette_state, (int)sizeof(palette_state),
              vdp1_state, (int)sizeof(vdp1_state),
              transform_state, (int)sizeof(transform_state),
              culling_state, (int)sizeof(culling_state),
              vdp1_command, (int)sizeof(vdp1_command), &capture) == 0 &&
          capture.dgn_source_hash_verified && !capture.capture_source_verified &&
          !capture.complete_source_binding &&
          !capture.original_saturn_capture_verified &&
          !capture.renderer_handoff_ready && capture.blocks_real_dgn_mesh_render,
          "a fully bound Structure3 packet remains no-draw without Saturn provenance");
    CHECK(nexus_v1_dgn_bind_structure3_face_capture_candidate(
              &level, dgn, (int)sizeof(dgn), 1, 1, &candidate,
              texture_span, (int)sizeof(texture_span),
              palette_state, (int)sizeof(palette_state),
              vdp1_state, (int)sizeof(vdp1_state),
              transform_state, (int)sizeof(transform_state),
              culling_state, (int)sizeof(culling_state),
              vdp1_command, (int)sizeof(vdp1_command), &capture) == 0 &&
          !capture.complete_source_binding && capture.blocks_real_dgn_mesh_render,
          "a degenerate Structure3 face cannot enter the host packet");

    /* The engine boundary needs a nondegenerate row so it can prove that it
     * recomputes a real binding instead of trusting a receipt's booleans.
     * This happens after the deliberately-degenerate parser coverage above. */
    wb32(payload + 56, 65536U);
    wb32(payload + 60, 65536U);
    wb32(payload + 64, 65536U);
    wb32(payload + 68, 131072U);
    wb32(payload + 72, 65536U);
    wb32(payload + 76, 65536U);
    wb32(payload + 80, 65536U);
    wb32(payload + 84, 131072U);
    wb32(payload + 88, 65536U);
    wb16(payload + 104, 0U);
    wb16(payload + 106, 1U);
    wb16(payload + 108, 2U);
    wb16(payload + 110, 2U);
    /* Keep the engine fixture structurally separate from the initial
     * unbound-material checks: it needs a real Structure1G/Structure2 join
     * beside this bounded Structure3 payload, not a stand-in texture. */
    build_structure1g_fixture(dgn + NEXUS_DGN_BLOCK_SIZE, 0x200);
    build_structure2_fixture(dgn);
    wb16(payload + 214, 0x0807U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 0) == 0,
          "the engine capture fixture reloads bounded animated face inputs");
    candidate.dgn_fnv1a64 = fnv1a64(dgn, sizeof(dgn));
    candidate.structure3_payload_fnv1a32 = level.structure3_payload.raw_payload_hash;
    candidate.entry_index = 0U;
    candidate.face_ordinal = 0U;
    candidate.face_row_fnv1a32 = fnv1a32(payload + 104U, 12U);
    candidate.referenced_vertex_rows_fnv1a32 = fnv1a32(payload + 56U, 36U);
    candidate.normal_row_fnv1a32 = fnv1a32(payload + 128U, 12U);
    CHECK(nexus_v1_dgn_bind_structure3_face_capture_candidate(
              &level, dgn, (int)sizeof(dgn), 1, 1, &candidate,
              texture_span, (int)sizeof(texture_span),
              palette_state, (int)sizeof(palette_state),
              vdp1_state, (int)sizeof(vdp1_state),
              transform_state, (int)sizeof(transform_state),
              culling_state, (int)sizeof(culling_state),
              vdp1_command, (int)sizeof(vdp1_command), &capture) == 0 &&
          capture.vdp1_command_format_matches &&
          capture.vdp1_texture_span_size_matches &&
          capture.complete_source_binding,
          "the engine fixture produces one complete but no-draw Structure3 binding");
    candidate.texture_span_fnv1a64 = fnv1a64(texture_span,
                                              sizeof(texture_span) - 1U);
    CHECK(nexus_v1_dgn_bind_structure3_face_capture_candidate(
              &level, dgn, (int)sizeof(dgn), 1, 1, &candidate,
              texture_span, (int)sizeof(texture_span) - 1,
              palette_state, (int)sizeof(palette_state),
              vdp1_state, (int)sizeof(vdp1_state),
              transform_state, (int)sizeof(transform_state),
              culling_state, (int)sizeof(culling_state),
              vdp1_command, (int)sizeof(vdp1_command), &capture) != 0 &&
          capture.vdp1_command_format_matches &&
          !capture.vdp1_texture_span_size_matches &&
          !capture.complete_source_binding,
          "a hash-matched but undersized capture lane cannot bind a VDP1 texture command");
    candidate.texture_span_fnv1a64 = fnv1a64(texture_span,
                                              sizeof(texture_span));
    {
        Nexus_V1_Engine engine;
        Nexus_V1_DgnStructure3FaceCaptureBindingReceipt bound;
        Nexus_V1_DgnStructure3CaptureImport import;
        Nexus_V1_DgnStructure3RenderPacket packet;
        Nexus_V1_DgnStructure3Vector live_vertices[4];
        Nexus_V1_DgnStructure3Face live_faces[2];
        Nexus_V1_DgnStructure3Vector live_normals[2];
        Nexus_V1_DgnStructure3MeshEntryReceipt live_mesh;
        Nexus_V1_DgnActiveLevelRendererSourceReceipt active_source;
        Nexus_V1_DgnActiveStructure3DirectoryReceipt active_directory;
        Nexus_V1_DgnActiveStructure3MeshSemanticReceipt active_mesh_semantics;
        Nexus_V1_DgnActiveStructure3FaceFramingReceipt active_face_framing;
        Nexus_V1_DgnActiveTransformCameraFramingReceipt active_camera_framing;
        Nexus_V1_DgnStructure3AnimatedMaterialImageSceneReceipt animated_images;
        Nexus_V1_DgnStructure3AnimatedMaterialPayloadSceneReceipt animated_payloads;
        AnimatedMaterialImageVisitCount animated_image_visits;
        Nexus_V1_DgnStructure3CompleteSourceSceneReceipt complete_scene;
        Nexus_V1_DgnViewportHostRouteReceipt host_route;
        Nexus_V1_DgnStructure3RuntimeCaptureIntakeReceipt raw_runtime_intake;
        Nexus_V1_DgnStructure3CaptureTargetReceipt raw_capture_target;
        Nexus_V1_DgnStructure3RawCapturePaths raw_paths;
        Nexus_V1_DgnStructure3RawCaptureAttestation raw_attestation;
        Nexus_Viewport viewport;
        char raw_manifest[2048];
        char raw_paths_storage[6][128];
        uint64_t raw_sequence[6] = { 11U, 12U, 13U, 14U, 15U, 16U };
        int raw_path_index;

        memset(&engine, 0, sizeof(engine));
        memset(&bound, 0, sizeof(bound));
        memset(&import, 0, sizeof(import));
        engine.level_loaded = 1;
        engine.game.current_level = 0;
        engine.current_level = level;
        engine.current_level_dgn_data = dgn;
        engine.current_level_dgn_size = (int)sizeof(dgn);
        engine.current_level_structure2_source.level_index = 0;
        engine.current_level_structure2_source.canonical_hash_verified = 1;
        engine.current_level_structure2_source.materialization_bound = 1;
        engine.current_level_structure2_source.structure2_payload_envelope_valid = 1;
        engine.current_level_structure2_source.loaded_bytes_bound = 1;
        engine.current_level_structure2_source.loaded_dgn_size = (int)sizeof(dgn);
        engine.current_level_structure2_source.loaded_dgn_fnv1a64 =
            fnv1a64(dgn, sizeof(dgn));
        engine.game.party_x = 0;
        engine.game.party_y = 0;
        engine.game.party_dir = 2;
        memset(&animated_images, 0, sizeof(animated_images));
        memset(&animated_image_visits, 0, sizeof(animated_image_visits));
        CHECK(nexus_v1_current_level_visit_structure3_animated_material_images(
                  &engine, count_animated_material_image_packet,
                  &animated_image_visits, &animated_images) == 0 &&
              !animated_images.valid && animated_images.no_draw_only &&
              animated_images.blocks_real_dgn_mesh_render,
              "an 08xx image route without bounded Structure2 windows remains fail-closed");
        memset(&animated_payloads, 0, sizeof(animated_payloads));
        CHECK(nexus_v1_current_level_visit_structure3_animated_material_payload_anchors(
                  &engine, &animated_payloads) == 0 && !animated_payloads.valid &&
              animated_payloads.no_draw_only &&
              animated_payloads.blocks_real_dgn_mesh_render,
              "an 08xx image route without bounded descriptor anchors remains fail-closed");
        memset(&complete_scene, 0, sizeof(complete_scene));
        CHECK(nexus_v1_current_level_structure3_complete_source_scene_receipt(
                  &engine, &complete_scene) == 0 && !complete_scene.valid &&
              !complete_scene.structure2_payload_coverage_complete &&
              complete_scene.no_draw_only &&
              complete_scene.blocks_real_dgn_mesh_render,
              "a DGN scene without bounded Structure2 payload anchors remains fail-closed");
        /* The following capture-route cases intentionally exercise the
         * pre-Structure2 source state. Keep this focused animated route from
         * widening those unrelated assertions. */
        engine.current_level_structure2_source.structure2_payload_envelope_valid = 0;
        CHECK(nexus_v1_current_level_structure3_directory_receipt(
                  &engine, &active_directory) == 1 && active_directory.valid &&
              active_directory.level_index == 0 &&
              active_directory.source_byte_count == (int)sizeof(dgn) &&
              active_directory.source_bytes_fnv1a64 == fnv1a64(dgn, sizeof(dgn)) &&
              active_directory.directory.valid &&
              active_directory.directory.entry_count == 2 &&
              active_directory.no_draw_only &&
              !active_directory.fallback_visuals_permitted,
              "active canonical LEV exposes a bounded Structure3 directory without texture or draw semantics");
        CHECK(nexus_v1_current_level_structure3_mesh_semantic_receipt(
                  &engine, &active_mesh_semantics) == 1 &&
              active_mesh_semantics.valid && active_mesh_semantics.level_index == 0 &&
              active_mesh_semantics.source_byte_count == (int)sizeof(dgn) &&
              active_mesh_semantics.source_bytes_fnv1a64 ==
                  fnv1a64(dgn, sizeof(dgn)) &&
              active_mesh_semantics.mesh_semantics.source_facts_complete &&
              active_mesh_semantics.mesh_semantics.entry_count == 2 &&
              active_mesh_semantics.mesh_semantics.vertex_count == 5 &&
              active_mesh_semantics.mesh_semantics.face_count == 4 &&
              active_mesh_semantics.mesh_semantics.normal_count == 4 &&
              active_mesh_semantics.mesh_semantics.original_capture_required &&
              !active_mesh_semantics.mesh_semantics.original_capture_available &&
              !active_mesh_semantics.mesh_semantics.texture_palette_semantics_proven &&
              !active_mesh_semantics.mesh_semantics.draw_semantics_proven &&
              !active_mesh_semantics.mesh_semantics.renderer_handoff_ready &&
              active_mesh_semantics.mesh_semantics.blocks_real_dgn_mesh_render &&
              active_mesh_semantics.no_draw_only &&
              !active_mesh_semantics.fallback_visuals_permitted,
              "active canonical LEV exposes only source-bound Structure3 mesh facts");
        CHECK(nexus_v1_current_level_structure3_face_framing_receipt(
                  &engine, &active_face_framing) == 1 &&
              active_face_framing.valid && active_face_framing.level_index == 0 &&
              active_face_framing.source_byte_count == (int)sizeof(dgn) &&
              active_face_framing.source_bytes_fnv1a64 == fnv1a64(dgn, sizeof(dgn)) &&
              active_face_framing.entry_headers.valid &&
              active_face_framing.entry_headers.entry_count == 2 &&
              active_face_framing.entry_headers.boundaries_valid &&
              active_face_framing.faces.valid &&
              active_face_framing.faces.entry_headers_valid &&
              active_face_framing.faces.face_count == 4 &&
              active_face_framing.faces.face_vertex_indexes_valid &&
              !active_face_framing.transform_semantics_proven &&
              active_face_framing.no_draw_only &&
              !active_face_framing.fallback_visuals_permitted,
              "active canonical LEV exposes source-bound Structure3 face framing only");
        CHECK(nexus_v1_current_level_transform_camera_framing_receipt(
                  &engine, &active_camera_framing) == 1 &&
              active_camera_framing.valid && active_camera_framing.level_index == 0 &&
              active_camera_framing.source_byte_count == (int)sizeof(dgn) &&
              active_camera_framing.source_bytes_fnv1a64 ==
                  fnv1a64(dgn, sizeof(dgn)) &&
              active_camera_framing.party_x == 0 &&
              active_camera_framing.party_y == 0 &&
              active_camera_framing.party_dir == 2 &&
              active_camera_framing.party_cell_geometry_valid &&
              !active_camera_framing.saturn_camera_semantics_proven &&
              !active_camera_framing.saturn_transform_semantics_proven &&
              active_camera_framing.no_draw_only &&
              !active_camera_framing.fallback_visuals_permitted,
              "active canonical LEV exposes camera input provenance without Saturn camera semantics");
        engine.game.party_x = engine.current_level.width;
        CHECK(nexus_v1_current_level_transform_camera_framing_receipt(
                  &engine, &active_camera_framing) == 0 &&
              !active_camera_framing.valid && active_camera_framing.no_draw_only &&
              !active_camera_framing.fallback_visuals_permitted,
              "out-of-bounds pose cannot enter the active camera framing route");
        engine.game.party_x = 0;
        memset(&active_source, 0, sizeof(active_source));
        CHECK(nexus_v1_current_level_dgn_renderer_source_receipt(
                  &engine, &active_source) == 1 &&
              active_source.valid && active_source.package_source_bound &&
              active_source.structure3_payload_bound &&
              active_source.level_index == 0 &&
              active_source.source_byte_count == (int)sizeof(dgn) &&
              active_source.source_bytes_fnv1a64 == fnv1a64(dgn, sizeof(dgn)) &&
              active_source.structure3_payload_byte_count ==
                  level.structure3_payload.byte_size &&
              active_source.structure3_payload_fnv1a32 ==
                  level.structure3_payload.raw_payload_hash &&
              !active_source.original_saturn_capture_bound &&
              active_source.no_draw_only &&
              active_source.blocks_real_dgn_mesh_render &&
              !active_source.fallback_visuals_permitted &&
              active_source.texture_decode_unproven &&
              active_source.palette_decode_unproven &&
              active_source.vdp1_draw_unproven &&
              active_source.transform_culling_unproven,
              "active LEV source receipt binds canonical package bytes at the renderer boundary without authorizing Saturn drawing");
        memset(&live_mesh, 0, sizeof(live_mesh));
        CHECK(nexus_v1_current_level_extract_structure3_mesh_entry(
                  &engine, 0, live_vertices, 4, live_faces, 2,
                  live_normals, 2, &live_mesh) == 0 &&
              live_mesh.valid && live_mesh.source_identity_valid &&
              !live_mesh.transform_or_draw_semantics_proven &&
              live_vertices[0].x == 65536 && live_vertices[2].y == 131072 &&
              live_faces[0].triangle && live_faces[0].fill_selector == 0 &&
              live_normals[0].x == 65536,
              "active engine exposes only checksum-bound Structure3 mesh rows no-draw");
        dgn[0] ^= 1U;
        CHECK(nexus_v1_current_level_dgn_renderer_source_receipt(
                  &engine, &active_source) == 0 &&
              !active_source.valid && active_source.no_draw_only &&
              active_source.blocks_real_dgn_mesh_render,
              "active LEV source receipt withdraws renderer provenance after retained bytes change");
        CHECK(nexus_v1_current_level_structure3_mesh_semantic_receipt(
                  &engine, &active_mesh_semantics) == 0 &&
              !active_mesh_semantics.valid && active_mesh_semantics.no_draw_only &&
              !active_mesh_semantics.fallback_visuals_permitted,
              "active mesh semantics withdraw when retained LEV bytes change");
        CHECK(nexus_v1_current_level_structure3_face_framing_receipt(
                  &engine, &active_face_framing) == 0 &&
              !active_face_framing.valid && active_face_framing.no_draw_only &&
              !active_face_framing.fallback_visuals_permitted,
              "active face framing withdraws when retained LEV bytes change");
        CHECK(nexus_v1_current_level_transform_camera_framing_receipt(
                  &engine, &active_camera_framing) == 0 &&
              !active_camera_framing.valid && active_camera_framing.no_draw_only &&
              !active_camera_framing.fallback_visuals_permitted,
              "active camera framing withdraws when retained LEV bytes change");
        CHECK(nexus_v1_current_level_extract_structure3_mesh_entry(
                  &engine, 0, live_vertices, 4, live_faces, 2,
                  live_normals, 2, &live_mesh) == -1 &&
              !live_mesh.valid && !live_mesh.source_identity_valid,
              "active engine refuses Structure3 mesh rows after retained LEV bytes change");
        dgn[0] ^= 1U;
        import.texture_span = texture_span;
        import.texture_span_size = sizeof(texture_span);
        import.palette_state = palette_state;
        import.palette_state_size = sizeof(palette_state);
        import.vdp1_state = vdp1_state;
        import.vdp1_state_size = sizeof(vdp1_state);
        import.transform_state = transform_state;
        import.transform_state_size = sizeof(transform_state);
        import.normal_culling_state = culling_state;
        import.normal_culling_state_size = sizeof(culling_state);
        import.vdp1_command = vdp1_command;
        import.vdp1_command_size = sizeof(vdp1_command);
        import.capture_session_fnv1a64 = UINT64_C(0x1234);
        import.capture_bundle_fnv1a64 =
            structure3_capture_bundle_fnv1a64(&import);
        import.capture_trace_order_fnv1a64 = UINT64_C(0x5678);
        import.capture_bundle_hash_verified = 1;
        import.capture_trace_order_verified = 1;
        import.original_saturn_capture_verified = 1;
        bound.complete_source_binding = 1;
        bound.candidate_framing_valid = 1;
        bound.dgn_source_hash_verified = 1;
        bound.capture_source_verified = 1;
        bound.dgn_source_matches = 1;
        bound.structure3_payload_matches = 1;
        bound.typed_mesh_corpus_matches = 1;
        bound.entry_face_matches = 1;
        bound.face_row_matches = 1;
        bound.referenced_vertex_rows_match = 1;
        bound.normal_row_matches = 1;
        bound.fill_selector_matches = 1;
        bound.texture_span_matches = 1;
        bound.palette_state_matches = 1;
        bound.vdp1_state_matches = 1;
        bound.transform_state_matches = 1;
        bound.normal_culling_state_matches = 1;
        bound.vdp1_command_matches = 1;
        bound.vdp1_command_format_matches = 1;
        bound.vdp1_texture_span_size_matches = 1;
        bound.blocks_real_dgn_mesh_render = 1;
        import.original_saturn_capture_verified = 0;
        CHECK(!nexus_v1_engine_consume_structure3_capture(
                  &engine, &candidate, &bound, &import) &&
              !engine.structure3_runtime_source.valid,
              "engine rejects a complete-looking packet without the external Saturn verdict");
        import.original_saturn_capture_verified = 1;
        import.vdp1_state_size = sizeof(vdp1_state) - 1U;
        import.capture_bundle_fnv1a64 =
            structure3_capture_bundle_fnv1a64(&import);
        CHECK(!nexus_v1_engine_consume_structure3_capture(
                  &engine, &candidate, &bound, &import) &&
              !engine.structure3_runtime_source.valid,
              "engine rejects an authenticated but partial VDP1-VRAM snapshot");
        import.vdp1_state_size = sizeof(vdp1_state);
        import.capture_bundle_fnv1a64 =
            structure3_capture_bundle_fnv1a64(&import);
        CHECK(nexus_v1_engine_consume_structure3_capture(
                  &engine, &candidate, &bound, &import) &&
              engine.structure3_runtime_source.valid &&
              engine.structure3_runtime_source.level_index == 0 &&
              engine.structure3_runtime_source.entry_index ==
                  candidate.entry_index &&
              engine.structure3_runtime_source.face_ordinal ==
                  candidate.face_ordinal &&
              engine.structure3_runtime_source.vertex_slot_count == 3 &&
              engine.structure3_runtime_source.texture_span_size ==
                  (int)sizeof(texture_span) &&
              engine.structure3_runtime_source.texture_span != texture_span &&
              memcmp(engine.structure3_runtime_source.texture_span,
                     texture_span, sizeof(texture_span)) == 0 &&
              engine.structure3_runtime_source.palette_state != palette_state &&
              memcmp(engine.structure3_runtime_source.palette_state,
                     palette_state, sizeof(palette_state)) == 0 &&
              engine.structure3_runtime_source.vdp1_state != vdp1_state &&
              memcmp(engine.structure3_runtime_source.vdp1_state,
                     vdp1_state, sizeof(vdp1_state)) == 0 &&
              engine.structure3_runtime_source.transform_state != transform_state &&
              memcmp(engine.structure3_runtime_source.transform_state,
                     transform_state, sizeof(transform_state)) == 0 &&
              engine.structure3_runtime_source.normal_culling_state != culling_state &&
              memcmp(engine.structure3_runtime_source.normal_culling_state,
                     culling_state, sizeof(culling_state)) == 0 &&
              engine.structure3_runtime_source.vdp1_command != vdp1_command &&
              memcmp(engine.structure3_runtime_source.vdp1_command,
                     vdp1_command, sizeof(vdp1_command)) == 0 &&
              engine.structure3_runtime_source.capture_session_fnv1a64 ==
                  import.capture_session_fnv1a64 &&
              engine.structure3_runtime_source.capture_bundle_fnv1a64 ==
                  import.capture_bundle_fnv1a64 &&
              engine.structure3_runtime_source.capture_trace_order_fnv1a64 ==
                  import.capture_trace_order_fnv1a64 &&
              engine.structure3_runtime_source.capture_bundle_hash_verified &&
              engine.structure3_runtime_source.capture_trace_order_verified &&
              engine.structure3_runtime_source.original_saturn_capture_verified &&
              engine.structure3_runtime_source.blocks_real_dgn_mesh_render,
              "a bound Structure3 capture owns the complete opaque packet with its exact face/normal rows without enabling drawing");
        dgn[0] ^= 1U;
        CHECK(!nexus_v1_engine_consume_structure3_capture(
                  &engine, &candidate, &bound, &import) &&
              engine.structure3_runtime_source.valid,
              "the engine rejects a capture when retained LEV bytes no longer match their package receipt");
        dgn[0] ^= 1U;
        import.capture_bundle_fnv1a64 ^= UINT64_C(1);
        CHECK(!nexus_v1_engine_consume_structure3_capture(
                  &engine, &candidate, &bound, &import) &&
              engine.structure3_runtime_source.valid &&
              engine.structure3_runtime_source.capture_bundle_fnv1a64 !=
                  import.capture_bundle_fnv1a64,
              "the engine rejects a stale bundle receipt before a raw capture packet can replace its owned source");
        import.capture_bundle_fnv1a64 ^= UINT64_C(1);
        import.capture_trace_order_verified = 0;
        CHECK(!nexus_v1_engine_consume_structure3_capture(
                  &engine, &candidate, &bound, &import) &&
              engine.structure3_runtime_source.valid &&
              engine.structure3_runtime_source.capture_trace_order_verified,
              "the engine rejects a byte-complete capture without trace-order verification");
        import.capture_trace_order_verified = 1;
        memset(&packet, 0, sizeof(packet));
        CHECK(nexus_v1_current_level_structure3_render_packet(&engine, &packet) == 1 &&
              packet.valid && packet.source_geometry_bound &&
              packet.no_draw_only && packet.blocks_real_dgn_mesh_render &&
              packet.vertices == engine.structure3_runtime_source.vertices &&
              packet.vertex_count == 3 &&
              packet.normal == &engine.structure3_runtime_source.normal &&
              packet.texture_span == engine.structure3_runtime_source.texture_span &&
              packet.palette_state == engine.structure3_runtime_source.palette_state &&
              packet.vdp1_state == engine.structure3_runtime_source.vdp1_state &&
              packet.transform_state == engine.structure3_runtime_source.transform_state &&
              packet.normal_culling_state ==
                  engine.structure3_runtime_source.normal_culling_state &&
              packet.vdp1_command == engine.structure3_runtime_source.vdp1_command,
              "the DGN renderer receives only the engine-owned, fully related Structure3 packet and keeps it no-draw");
        CHECK(nexus_v1_current_level_dgn_renderer_source_receipt(
                  &engine, &active_source) == 1 &&
              active_source.original_saturn_capture_bound &&
              active_source.texture_span_bound && active_source.palette_state_bound &&
              active_source.vdp1_state_bound && active_source.transform_state_bound &&
              active_source.normal_culling_state_bound &&
              active_source.vdp1_command_bound &&
              active_source.vdp1_command_format_framed &&
              active_source.vdp1_command_framing.valid &&
              active_source.vdp1_command_framing.complete_vdp1_command_record &&
              active_source.vdp1_command_framing.command_format_parsed &&
              active_source.vdp1_command_framing.texture_primitive_observed &&
              active_source.vdp1_command_framing.texture_format_framed &&
              active_source.vdp1_command_framing.texture_span_size_matches_command &&
              active_source.vdp1_command_framing.coordinate_words_framed &&
              active_source.vdp1_command_framing.command.texture_bits_per_pixel == 4U &&
              active_source.vdp1_command_framing.command.texture_byte_count == 4U &&
              active_source.vdp1_command_framing.command.texture_source_byte_offset ==
                  0x100U &&
              active_source.vdp1_command_framing.command.texture_source_byte_end ==
                  0x104U &&
              active_source.vdp1_command_framing.command.texture_source_range_valid &&
              active_source.vdp1_command_framing.command.link_byte_offset ==
                  0x280U &&
              active_source.vdp1_command_framing.command.link_target_range_valid &&
              active_source.vdp1_command_framing.command.texture_width == 8U &&
              active_source.vdp1_command_framing.command.texture_height == 1U &&
              active_source.vdp1_command_framing.command.xa == -12 &&
              active_source.vdp1_command_framing.command.ya == 34 &&
              active_source.vdp1_command_framing.command.xd == -145 &&
              active_source.vdp1_command_framing.command.yd == 167 &&
              active_source.vdp1_command_framing.command.gouraud_table_word ==
                  0x2468U &&
              !active_source.vdp1_command_framing.pixel_format_proven &&
              !active_source.vdp1_command_framing.palette_format_proven &&
              !active_source.vdp1_command_framing.decoder_permitted &&
              active_source.vdp1_texture_format_framed &&
              active_source.vdp1_coordinate_words_framed &&
              active_source.vdp1_vram_window_bound &&
              active_source.vdp1_vram_window.valid &&
              active_source.vdp1_vram_window.complete_vdp1_vram_snapshot &&
              active_source.vdp1_vram_window.texture_lane_matches_vram_window &&
              active_source.vdp1_command_vram_bound &&
              active_source.vdp1_command_vram.valid &&
              active_source.vdp1_command_vram.complete_vdp1_vram_snapshot &&
              active_source.vdp1_command_vram.command_record_occurrence_count == 1 &&
              active_source.vdp1_command_vram.command_record_unique_in_vram &&
              active_source.vdp1_command_vram.command_record_byte_offset == 0x200U &&
              active_source.vdp1_command_vram.command_link_target_bounded &&
              active_source.vdp1_command_vram.command_link_byte_offset == 0x280U &&
              active_source.vdp1_command_vram.command_link_target_record_framed &&
              active_source.vdp1_command_vram.command_link_target.end_command &&
              !active_source.vdp1_vram_window.pixel_format_proven &&
              !active_source.vdp1_vram_window.palette_format_proven &&
              !active_source.vdp1_vram_window.decoder_permitted &&
              active_source.texture_decode_unproven &&
              active_source.palette_decode_unproven &&
              active_source.vdp1_draw_unproven &&
              active_source.transform_culling_unproven &&
              active_source.no_draw_only &&
              active_source.blocks_real_dgn_mesh_render,
              "bound original capture exposes exact opaque-state blockers without enabling a Saturn pixel or VDP1 route");
        nexus_viewport_init(&viewport);
        nexus_viewport_render(&viewport, &engine);
        CHECK(viewport.last_dgn_render_receipt.active_level_source_consumed &&
              viewport.last_dgn_render_receipt.active_level_source.valid &&
              viewport.last_dgn_render_receipt.active_level_source.package_source_bound &&
              viewport.last_dgn_render_receipt.active_level_source.no_draw_only &&
              viewport.last_dgn_render_receipt.structure3_source_packet_consumed &&
              viewport.last_dgn_render_receipt.structure3_source_geometry_bound &&
              viewport.last_dgn_render_receipt.structure3_source_no_draw &&
              viewport.last_dgn_render_receipt
                  .structure3_runtime_vdp1_command_framed &&
              viewport.last_dgn_render_receipt
                  .structure3_runtime_vdp1_texture_format_framed &&
              viewport.last_dgn_render_receipt
                  .structure3_runtime_vdp1_coordinate_words_framed &&
              viewport.last_dgn_render_receipt
                  .structure3_runtime_vdp1_vram_window_bound &&
              viewport.last_dgn_render_receipt.structure3_runtime_vdp1_vram_window
                  .texture_lane_matches_vram_window &&
              viewport.last_dgn_render_receipt
                  .structure3_runtime_vdp1_command_vram_bound &&
              viewport.last_dgn_render_receipt.structure3_runtime_vdp1_command_vram
                  .command_record_unique_in_vram &&
              viewport.last_dgn_render_receipt.structure3_runtime_vdp1_command_vram
                  .command_link_target_record_framed &&
              viewport.last_dgn_render_receipt.structure3_runtime_vdp1_command.valid &&
              viewport.last_dgn_render_receipt.structure3_runtime_vdp1_command
                  .command_format_parsed &&
              viewport.last_dgn_render_receipt.structure3_runtime_vdp1_command
                  .command.texture_width == 8U &&
              !viewport.last_dgn_render_receipt.structure3_runtime_vdp1_command
                  .decoder_permitted &&
              viewport.structure3_source_packet.vertices ==
                  engine.structure3_runtime_source.vertices &&
              viewport.structure3_source_packet.normal ==
                  &engine.structure3_runtime_source.normal,
              "the DGN viewport stages the bound Structure3 geometry without drawing it");
        memset(&host_route, 0, sizeof(host_route));
        CHECK(nexus_viewport_dgn_host_route_receipt(
                  &viewport, &engine, &host_route) == 0 &&
              host_route.package_consumed &&
              host_route.active_level_source_consumed &&
              host_route.active_level_source.valid &&
              host_route.active_level_source.package_source_bound &&
              host_route.active_level_source.source_bytes_fnv1a64 ==
                  fnv1a64(dgn, sizeof(dgn)) &&
              host_route.active_level_source.texture_decode_unproven &&
              host_route.active_level_source.palette_decode_unproven &&
              host_route.active_level_source.vdp1_draw_unproven &&
              host_route.active_level_source.no_draw_only &&
              !host_route.no_draw_structure3_source_scene &&
              host_route.blocks_runtime_dgn,
              "host route carries the exact active LEV source receipt while Saturn decoding remains blocked");
        candidate.first_sequence = 10U;
        candidate.last_sequence = 20U;
        memset(&raw_capture_target, 0, sizeof(raw_capture_target));
        CHECK(nexus_v1_dgn_structure3_capture_target_build(
                  &engine.current_level, engine.current_level_dgn_data,
                  engine.current_level_dgn_size, engine.game.current_level, 1,
                  candidate.entry_index, candidate.face_ordinal,
                  &raw_capture_target) && raw_capture_target.valid &&
              raw_capture_target.candidate.entry_index == candidate.entry_index &&
              raw_capture_target.candidate.face_ordinal == candidate.face_ordinal,
              "the raw-capture manifest uses framing rebuilt from its active Structure3 face");
        CHECK(snprintf(raw_manifest, sizeof(raw_manifest),
                 "NEXUS_STRUCTURE3_SATURN_CAPTURE_V1\n"
                 "capture_session_fnv1a64=1234\n"
                 "dgn_fnv1a64=%llx\n"
                 "structure3_payload_fnv1a32=%x\n"
                 "typed_mesh_corpus_fnv1a32=%x\n"
                 "entry_index=%x\nface_ordinal=%x\n"
                 "face_row_fnv1a32=%x\n"
                 "referenced_vertex_rows_fnv1a32=%x\n"
                 "normal_row_fnv1a32=%x\nfill_selector=%x\n"
                 "entry_byte_offset=%x\nvertex_byte_offset=%x\n"
                 "face_byte_offset=%x\nnormal_byte_offset=%x\n"
                 "vertex_count=%x\nface_count=%x\n"
                 "texture_span_bytes=%zx\ntexture_span_fnv1a64=%llx\n"
                 "palette_state_bytes=%zx\npalette_state_fnv1a64=%llx\n"
                 "vdp1_state_bytes=%zx\nvdp1_state_fnv1a64=%llx\n"
                 "transform_state_bytes=%zx\ntransform_state_fnv1a64=%llx\n"
                 "normal_culling_state_bytes=%zx\nnormal_culling_state_fnv1a64=%llx\n"
                 "vdp1_command_bytes=%zx\nvdp1_command_fnv1a64=%llx\n"
                 "texture_span_sequence=b\npalette_state_sequence=c\n"
                 "vdp1_state_sequence=d\ntransform_state_sequence=e\n"
                 "normal_culling_state_sequence=f\nvdp1_command_sequence=10\n"
                 "first_sequence=a\nlast_sequence=14\n",
                 (unsigned long long)candidate.dgn_fnv1a64,
                 candidate.structure3_payload_fnv1a32,
                 candidate.typed_mesh_corpus_fnv1a32, candidate.entry_index,
                 candidate.face_ordinal, candidate.face_row_fnv1a32,
                 candidate.referenced_vertex_rows_fnv1a32,
                 candidate.normal_row_fnv1a32, candidate.fill_selector,
                 raw_capture_target.entry_byte_offset,
                 raw_capture_target.vertex_byte_offset,
                 raw_capture_target.face_byte_offset,
                 raw_capture_target.normal_byte_offset,
                 raw_capture_target.vertex_count, raw_capture_target.face_count,
                 sizeof(texture_span),
                 (unsigned long long)candidate.texture_span_fnv1a64,
                 sizeof(palette_state),
                 (unsigned long long)candidate.palette_state_fnv1a64,
                 sizeof(vdp1_state),
                 (unsigned long long)candidate.vdp1_state_fnv1a64,
                 sizeof(transform_state),
                 (unsigned long long)candidate.transform_state_fnv1a64,
                 sizeof(culling_state),
                 (unsigned long long)candidate.normal_culling_state_fnv1a64,
                 sizeof(vdp1_command),
                 (unsigned long long)candidate.vdp1_command_fnv1a64) > 0 &&
              strlen(raw_manifest) < sizeof(raw_manifest),
              "the complete raw-capture manifest fits and includes current Structure3 framing fields");
        for (raw_path_index = 0; raw_path_index < 6; ++raw_path_index) {
            snprintf(raw_paths_storage[raw_path_index],
                     sizeof(raw_paths_storage[raw_path_index]),
                     "/tmp/firestaff-nexus-raw-runtime-%ld-%d.bin",
                     (long)getpid(), raw_path_index);
            remove(raw_paths_storage[raw_path_index]);
        }
        CHECK(write_capture_span(raw_paths_storage[0], texture_span,
                                 sizeof(texture_span)) &&
              write_capture_span(raw_paths_storage[1], palette_state,
                                 sizeof(palette_state)) &&
              write_capture_span(raw_paths_storage[2], vdp1_state,
                                 sizeof(vdp1_state)) &&
              write_capture_span(raw_paths_storage[3], transform_state,
                                 sizeof(transform_state)) &&
              write_capture_span(raw_paths_storage[4], culling_state,
                                 sizeof(culling_state)) &&
              write_capture_span(raw_paths_storage[5], vdp1_command,
                                 sizeof(vdp1_command)),
              "raw Structure3 capture lanes are available for runtime intake");
        memset(&raw_paths, 0, sizeof(raw_paths));
        raw_paths.texture_span_path = raw_paths_storage[0];
        raw_paths.palette_state_path = raw_paths_storage[1];
        raw_paths.vdp1_state_path = raw_paths_storage[2];
        raw_paths.transform_state_path = raw_paths_storage[3];
        raw_paths.normal_culling_state_path = raw_paths_storage[4];
        raw_paths.vdp1_command_path = raw_paths_storage[5];
        memset(&raw_attestation, 0, sizeof(raw_attestation));
        raw_attestation.capture_session_fnv1a64 = UINT64_C(0x1234);
        raw_attestation.capture_bundle_fnv1a64 =
            structure3_capture_bundle_fnv1a64(&import);
        raw_attestation.capture_trace_order_fnv1a64 =
            structure3_capture_trace_order_fnv1a64(raw_sequence);
        raw_attestation.original_saturn_source_attested = 1;
        CHECK(nexus_v1_engine_consume_structure3_raw_capture_manifest(
                  &engine, raw_manifest, strlen(raw_manifest), &raw_paths,
                  &raw_attestation, &raw_runtime_intake) &&
              raw_runtime_intake.active_canonical_lev_bound &&
              raw_runtime_intake.raw_capture_host_intake_invoked &&
              raw_runtime_intake.manifest_parsed &&
              raw_runtime_intake.all_trace_lanes_authenticated &&
              raw_runtime_intake.complete_source_binding &&
              raw_runtime_intake.engine_consume_invoked &&
              raw_runtime_intake.runtime_source_consumed &&
              raw_runtime_intake.no_draw_only &&
              !raw_runtime_intake.fallback_visuals_permitted &&
              engine.structure3_runtime_source.valid &&
              engine.structure3_runtime_source.blocks_real_dgn_mesh_render,
              "all six authenticated raw trace lanes reach the engine-owned no-draw runtime source");
        raw_attestation.capture_trace_order_fnv1a64 ^= UINT64_C(1);
        CHECK(!nexus_v1_engine_consume_structure3_raw_capture_manifest(
                  &engine, raw_manifest, strlen(raw_manifest), &raw_paths,
                  &raw_attestation, &raw_runtime_intake) &&
              raw_runtime_intake.active_canonical_lev_bound &&
              raw_runtime_intake.raw_capture_host_intake_invoked &&
              !raw_runtime_intake.all_trace_lanes_authenticated &&
              !raw_runtime_intake.runtime_source_consumed &&
              raw_runtime_intake.no_draw_only &&
              engine.structure3_runtime_source.valid,
              "a failed trace-order attestation cannot replace the admitted runtime source");
        for (raw_path_index = 0; raw_path_index < 6; ++raw_path_index)
            remove(raw_paths_storage[raw_path_index]);
        engine.structure3_runtime_source.binding.normal_row_matches = 0;
        CHECK(nexus_v1_current_level_structure3_render_packet(&engine, &packet) == 0 &&
              !packet.valid && packet.no_draw_only &&
              packet.blocks_real_dgn_mesh_render,
              "a missing face-to-normal relation withdraws the Structure3 renderer packet");
        free(engine.structure3_runtime_source.texture_span);
        free(engine.structure3_runtime_source.palette_state);
        free(engine.structure3_runtime_source.vdp1_state);
        free(engine.structure3_runtime_source.transform_state);
        free(engine.structure3_runtime_source.normal_culling_state);
        free(engine.structure3_runtime_source.vdp1_command);
    }

    wb32(payload + 56, 65536U);
    wb32(payload + 60, 0U);
    wb32(payload + 64, 0U);
    wb32(payload + 68, 65536U);
    wb32(payload + 72, 0U);
    wb32(payload + 76, 0U);
    wb32(payload + 80, 65536U);
    wb32(payload + 84, 0U);
    wb32(payload + 88, 0U);
    wb16(payload + 104, 0U);
    wb16(payload + 106, 1U);
    wb16(payload + 108, 0U);
    wb16(payload + 110, 0U);

    wb32(payload + 128, 0U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 0) == 0 &&
          nexus_v1_level_structure3_vector_receipt(&level, &vectors) == 0 &&
          nexus_v1_level_structure3_face_normal_pair_receipt(&level, &pairs) == 0 &&
          nexus_v1_level_structure3_mesh_semantic_handoff_receipt(
              &level, &mesh_semantics) == 0 &&
          !vectors.valid && vectors.normal_non_unit_length_count == 1 &&
          !pairs.vector_receipt_valid && !pairs.valid &&
          pairs.non_unit_length_face_normal_pair_count == 1 &&
          !mesh_semantics.source_facts_complete &&
          !mesh_semantics.original_capture_required &&
          mesh_semantics.blocks_real_dgn_mesh_render &&
          !vectors.transform_or_draw_semantics_proven,
          "a non-unit Structure3 normal invalidates its no-draw pair receipt");
    wb32(payload + 128, 65536U);

    wb32(payload + 36, 124U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 0) == 0 &&
          nexus_v1_level_structure3_entry_header_receipt(&level, &headers) == 0 &&
          !headers.valid && !headers.boundaries_valid &&
          !headers.semantics_proven,
          "invalid Structure3 entry boundaries remain fail-closed");

    wb32(payload + 36, 128U);
    wb32(payload + 172, 8184U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 0) == 0 &&
          nexus_v1_level_structure3_entry_header_receipt(&level, &headers) == 0 &&
          !headers.valid && !headers.third_region_boundaries_valid &&
          !headers.semantics_proven,
          "truncated third Structure3 region remains fail-closed");

    wb32(payload + 172, 228U);
    wb16(payload + 104, 4U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 0) == 0 &&
          nexus_v1_level_structure3_face_receipt(&level, &faces) == 0 &&
          !faces.valid && !faces.face_vertex_indexes_valid &&
          !faces.face_vertex_linkage_valid &&
          !faces.face_vertex_entry_coverage_accounting_valid &&
          !faces.face_vertex_component_accounting_valid &&
          !faces.face_vertex_component_entry_accounting_valid &&
          !faces.face_vertex_adjacency_accounting_valid &&
          !faces.face_vertex_adjacency_multiplicity_accounting_valid &&
          !faces.draw_semantics_proven,
          "out-of-range Structure3 face indexes remain fail-closed");

    wb16(payload + 104, 0U);
    wb16(payload + 116, 0U);
    wb16(payload + 118, 1U);
    wb16(payload + 120, 0U);
    wb16(payload + 122, 0U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 0) == 0 &&
          nexus_v1_level_structure3_face_receipt(&level, &faces) == 0 &&
          faces.face_vertex_cooccurrence_pair_count == 2 &&
          faces.face_vertex_adjacency_pair_count == 1 &&
          faces.repeated_face_vertex_adjacency_pair_count == 1 &&
          faces.face_vertex_adjacency_accounting_valid &&
          faces.face_vertex_adjacency_multiplicity_accounting_valid &&
          faces.single_face_vertex_adjacency_pair_count == 0 &&
          faces.shared_face_vertex_adjacency_pair_count == 1 &&
          faces.maximum_face_vertex_adjacency_pair_incidence == 2 &&
          !faces.draw_semantics_proven,
          "repeated Structure3 index pairs remain bounded no-draw incidence");
}

static void test_structure3_extreme_vector_arithmetic(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 24];
    uint8_t *payload = dgn + NEXUS_DGN_BLOCK_SIZE * 20;
    Nexus_V1_Level level;
    Nexus_V1_DgnStructure3VectorReceipt vectors;
    Nexus_V1_DgnStructure3FaceReceipt faces;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19, 0x200, 512) == 0,
          "extreme Structure3 vector fixture builds");
    wb16(dgn + 0x1c, 20U);
    wb16(dgn + 0x1e, 4U);
    wb32(payload, 1U);
    wb32(payload + 4, 8U);
    wb32(payload + 8, 0x100U);
    wb16(payload + 12, 3U);
    wb16(payload + 14, 1U);
    wb32(payload + 16, 48U);
    wb32(payload + 24, 84U);
    wb32(payload + 28, 96U);

    /* The face lies in the Y/Z plane. Its signed 16.16 coordinates make
     * the raw cross product exceed int64 while retaining an exact +X normal. */
    wb32(payload + 48 + 4, 0x80000000U);
    wb32(payload + 48 + 8, 0x80000000U);
    wb32(payload + 60 + 4, 0x7fffffffU);
    wb32(payload + 60 + 8, 0x80000000U);
    wb32(payload + 72 + 4, 0x80000000U);
    wb32(payload + 72 + 8, 0x7fffffffU);
    wb16(payload + 84, 0U);
    wb16(payload + 86, 1U);
    wb16(payload + 88, 2U);
    wb16(payload + 90, 2U);
    wb32(payload + 96, 65536U);

    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 0) == 0 &&
          nexus_v1_level_structure3_face_receipt(&level, &faces) == 0 &&
          nexus_v1_level_structure3_vector_receipt(&level, &vectors) == 0 &&
          faces.valid && faces.triangle_count == 1 &&
          vectors.valid && vectors.normal_face_plane_pair_count == 2 &&
          vectors.normal_face_plane_within_tolerance_count == 2 &&
          vectors.positive_winding_triangle_count == 1 &&
          vectors.maximum_normal_face_plane_error == 0 &&
          !vectors.transform_or_draw_semantics_proven,
          "extreme Structure3 vectors keep exact no-draw geometry validation without signed overflow");
}

static void test_visible_structure1f_semantics_block_render_plan(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    const int structure1b_rel = 0x40;
    const int geometry_bytes = 512;
    uint8_t *structure1;
    uint8_t *structure1f;
    Nexus_V1_Level level;
    Nexus_V1_DgnRenderCommand commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnRenderPlanReceipt receipt;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, geometry_bytes) == 0,
          "visible Structure1F fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    structure1[9] = 22;
    set_floor_flags(structure1, structure1b_rel, 3, 4,
                    (uint16_t)((1U << 14) | (21U << 7) | (2U << 4) | 2U));
    set_collision_ref(structure1, structure1b_rel, 3, 4, 1);
    set_post_grid_0x30_ref(structure1, structure1b_rel, 3, 4, 1);
    set_collision_ref(structure1, structure1b_rel, 2, 4, 0x0fff);
    set_collision_ref(structure1, structure1b_rel, 4, 4, 0x0fff);
    set_collision_ref(structure1, structure1b_rel, 3, 3, 0x0fff);

    build_structure1f_fixture(structure1, structure1b_rel);
    structure1f = structure1 + structure1b_rel +
        NEXUS_DGN_STRUCTURE1B_BYTES + 312;
    /* First floor-decoration record: its documented source cell is visible,
     * but no original Saturn draw semantics are currently evidenced. */
    structure1f[32 + 1] = 3;
    structure1f[32 + 2] = 4;

    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0,
          "visible Structure1F level loads");
    memset(commands, 0, sizeof(commands));
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_v1_level_build_dgn_view_render_plan(
              &level, 3, 4, 0, commands,
              NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS, &receipt) == 0,
          "visible Structure1F render plan evaluates");
    CHECK(receipt.structure1f_plan_direct_entry_count == 0 &&
          receipt.command_count == 0 &&
          receipt.status ==
              NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_SEMANTICS &&
          receipt.blocks_real_dgn_mesh_render && !receipt.plan_ready &&
          !receipt.fallback_visuals_permitted,
          "visible unproven Structure1F decoration blocks DGN rendering without fallback");

    /* Item coordinates use the same direct 64x64 source-cell binding. The
     * original Saturn object draw ABI is likewise not established, so an
     * otherwise renderable view must not silently drop a visible item. */
    structure1f[NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 1] = 3;
    structure1f[NEXUS_DGN_STRUCTURE1F_HEADER_BYTES + 2] = 4;
    structure1f[32 + 1] = 10;
    structure1f[32 + 2] = 20;
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0,
          "visible Structure1F item level reloads");
    memset(commands, 0, sizeof(commands));
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_v1_level_build_dgn_view_render_plan(
              &level, 3, 4, 0, commands,
              NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS, &receipt) == 0,
          "visible Structure1F item render plan evaluates");
    CHECK(receipt.structure1f_plan_direct_entry_count == 0 &&
          receipt.command_count == 0 &&
          receipt.status ==
              NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_SEMANTICS &&
          receipt.blocks_real_dgn_mesh_render && !receipt.plan_ready &&
          !receipt.fallback_visuals_permitted,
          "visible unproven Structure1F item blocks DGN rendering without fallback");
}

static void test_direct_structure1f_mesh_command_provenance(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    const int structure1b_rel = 0x40;
    uint8_t *structure1;
    Nexus_V1_Level level;
    Nexus_V1_DgnRenderCommand commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnRenderPlanReceipt receipt;
    Nexus_V1_DgnStructure1FDirectFloorCommandSource direct_sources[2];
    Nexus_V1_DgnStructure1FDirectFloorCommandSourceReceipt direct_receipt;
    int command_index;
    int matched_commands = 0;
    int matched_floor_commands = 0;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, 2048) == 0,
          "direct Structure1F mesh-provenance fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    structure1[9] = 22;
    set_floor_flags(structure1, structure1b_rel, 3, 4,
                    (uint16_t)((1U << 14) | (21U << 7) | (2U << 4) | 2U));
    set_collision_ref(structure1, structure1b_rel, 3, 4, 1);
    cell_at(structure1, structure1b_rel, 3, 4)[3] = 4;
    cell_at(structure1, structure1b_rel, 3, 4)[4] = 41;
    set_post_grid_0x30_ref(structure1, structure1b_rel, 3, 4, 1);
    cell_at(structure1, structure1b_rel, 4, 4)[3] = 12;
    set_collision_ref(structure1, structure1b_rel, 4, 4, 1);
    set_post_grid_0x30_ref(structure1, structure1b_rel, 4, 4, 2);
    set_collision_ref(structure1, structure1b_rel, 3, 3, 1);
    set_post_grid_0x30_ref(structure1, structure1b_rel, 3, 3, 3);
    set_collision_ref(structure1, structure1b_rel, 2, 4, 0x0fff);
    set_collision_ref(structure1, structure1b_rel, 2, 3, 0x0fff);
    set_collision_ref(structure1, structure1b_rel, 4, 3, 0x0fff);
    set_collision_ref(structure1, structure1b_rel, 3, 2, 0x0fff);
    build_direct_structure1f_fixture(structure1, structure1b_rel, 3, 4);

    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0,
          "direct Structure1F mesh-provenance level loads");
    memset(commands, 0, sizeof(commands));
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_v1_level_build_dgn_view_render_plan(
              &level, 3, 4, 0, commands,
              NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS, &receipt) == 0,
          "direct Structure1F mesh-provenance plan evaluates");
    for (command_index = 0; command_index < receipt.command_count;
         ++command_index) {
        if (commands[command_index].x == 3 && commands[command_index].y == 4) {
            ++matched_commands;
            if (commands[command_index].kind ==
                NEXUS_V1_DGN_RENDER_COMMAND_FLOOR) {
                ++matched_floor_commands;
                CHECK(commands[command_index].structure1f_direct_entry_count == 1 &&
                      commands[command_index].structure1f_direct_family_mask ==
                          NEXUS_V1_DGN_STRUCTURE1F_DIRECT_FAMILY_ITEM,
                      "the source-cell floor command retains exact direct item provenance");
            } else {
                CHECK(commands[command_index].structure1f_direct_entry_count == 0 &&
                      commands[command_index].structure1f_direct_family_mask == 0,
                      "direct floor-cell provenance never invents a wall or ceiling relation");
            }
        }
    }
    CHECK(receipt.status == NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_SEMANTICS &&
          receipt.blocks_real_dgn_mesh_render && !receipt.plan_ready &&
          !receipt.fallback_visuals_permitted &&
          receipt.structure1f_plan_direct_entry_count == 1 &&
          receipt.structure1f_plan_item_entry_count == 1 &&
          receipt.structure1f_plan_direct_command_count == matched_floor_commands &&
          receipt.structure1f_plan_direct_command_entry_count == matched_floor_commands &&
          receipt.structure1f_plan_direct_floor_command_count == matched_floor_commands &&
          receipt.structure1f_plan_direct_floor_command_entry_count == matched_floor_commands &&
          receipt.structure1f_plan_item_floor_command_count == matched_floor_commands &&
          receipt.structure1f_plan_item_floor_command_entry_count == matched_floor_commands &&
          matched_commands > matched_floor_commands && matched_floor_commands > 0,
          "direct Structure1F source cells bind only to exact floor material commands then remain no-draw");
    memset(direct_sources, 0, sizeof(direct_sources));
    memset(&direct_receipt, 0, sizeof(direct_receipt));
    CHECK(nexus_v1_dgn_bind_direct_structure1f_floor_sources(
              &level, commands, receipt.command_count, direct_sources, 2,
              &direct_receipt) == 0 && direct_receipt.complete &&
          direct_receipt.visible_direct_entry_count == 1 &&
          direct_receipt.floor_command_source_count == 1 &&
          direct_receipt.item_floor_command_source_count == 1 &&
          direct_receipt.floor_decoration_command_source_count == 0 &&
          direct_receipt.floor_sensor_command_source_count == 0 &&
          direct_sources[0].command_index >= 0 &&
          commands[direct_sources[0].command_index].kind ==
              NEXUS_V1_DGN_RENDER_COMMAND_FLOOR &&
          direct_sources[0].entry.family == NEXUS_V1_DGN_STRUCTURE1F_ITEMS &&
          direct_sources[0].entry.x == 3 && direct_sources[0].entry.y == 4 &&
          !direct_sources[0].draw_authorized &&
          !direct_receipt.fallback_visuals_permitted,
          "direct Structure1F record reaches only its exact runtime floor command without a draw claim");
}

/* Retail-only companion to the mesh-command fixture above. It checks the
 * source relation used by the command binder against each supplied Track 1
 * DGN, without promoting any raw item/decor/sensor bytes to Saturn visuals. */
static void test_real_structure1f_direct_cell_corpus(void) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    int level_index;
    int checked = 0;

    if (!data_dir || !data_dir[0]) return;
    for (level_index = 0; level_index <= 15; ++level_index) {
        char path[2048];
        FILE *file;
        long size;
        uint8_t *data;
        Nexus_V1_Level level;
        Nexus_V1_DgnStructure1FSpatialReceipt spatial;
        int entry_index;
        int direct_count = 0;
        int item_count = 0;
        int floor_decoration_count = 0;
        int floor_sensor_count = 0;

        snprintf(path, sizeof(path), "%s/LEV%02d.DGN", data_dir, level_index);
        file = fopen(path, "rb");
        CHECK(file != NULL, "real Structure1F direct-cell corpus file opens");
        if (!file) continue;
        CHECK(real_dgn_is_hash_verified(path, level_index),
              "real Structure1F direct-cell DGN matches its canonical MD5");
        if (!real_dgn_is_hash_verified(path, level_index)) {
            fclose(file);
            continue;
        }
        CHECK(fseek(file, 0, SEEK_END) == 0,
              "real Structure1F direct-cell corpus file seeks");
        size = ftell(file);
        CHECK(size > 0 && fseek(file, 0, SEEK_SET) == 0,
              "real Structure1F direct-cell corpus file has data");
        if (size <= 0 || fseek(file, 0, SEEK_SET) != 0) {
            fclose(file);
            continue;
        }
        data = (uint8_t *)malloc((size_t)size);
        CHECK(data != NULL, "real Structure1F direct-cell corpus allocates");
        if (!data) {
            fclose(file);
            continue;
        }
        CHECK(fread(data, 1, (size_t)size, file) == (size_t)size,
              "real Structure1F direct-cell corpus reads");
        fclose(file);
        if (nexus_v1_level_load(&level, data, (int)size, level_index) == 0 &&
            nexus_v1_level_structure1f_spatial_receipt(&level, &spatial) == 0) {
            for (entry_index = 0; entry_index < level.structure1f_entry_count;
                 ++entry_index) {
                const Nexus_V1_DgnStructure1FEntry *entry =
                    &level.structure1f_entries[entry_index];
                switch (entry->family) {
                case NEXUS_V1_DGN_STRUCTURE1F_ITEMS:
                    ++item_count;
                    ++direct_count;
                    break;
                case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS:
                    ++floor_decoration_count;
                    ++direct_count;
                    break;
                case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS:
                    ++floor_sensor_count;
                    ++direct_count;
                    break;
                default:
                    break;
                }
            }
            CHECK(spatial.valid &&
                  spatial.direct_coordinate_entry_count == direct_count &&
                  spatial.item_entry_count == item_count &&
                  spatial.floor_decoration_entry_count == floor_decoration_count &&
                  spatial.floor_sensor_entry_count == floor_sensor_count,
                  "real Structure1F direct-cell records retain the mesh-command source relation");
            ++checked;
        } else {
            CHECK(0, "real Structure1F direct-cell corpus level loads");
        }
        free(data);
    }
    CHECK(checked == 16,
          "real Structure1F direct-cell corpus covers every supplied retail DGN");
}

static void test_structure1g_semantics_and_bounds(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    const int structure1b_rel = 0x40;
    uint8_t *structure1;
    Nexus_V1_DgnStructure1Layout layout;
    Nexus_V1_DgnGeometryInfo info;
    Nexus_V1_Level level;
    Nexus_V1_DgnRendererHandoffReceipt handoff;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, 512) == 0,
          "Structure1G fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    build_structure1g_fixture(structure1, structure1b_rel);
    CHECK(nexus_v1_dgn_structure1_layout(&layout, dgn, (int)sizeof(dgn)) == 0 &&
          layout.structure1g.valid && layout.structure1g.descriptor_count == 2 &&
          layout.structure1g.animated_texture_count == 1 &&
          layout.structure1g.sequence_count == 1,
          "Structure1G accepts a bounded descriptor and terminated instruction stream");
    CHECK(nexus_v1_dgn_geometry_info(&info, dgn, (int)sizeof(dgn)) == 0 &&
          info.structure1g_present && info.structure1g_valid &&
          info.structure1g_animated_texture_count == 1,
          "Structure1G provenance reaches geometry information");
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          level.structure1g_entry_count == 1 &&
          level.structure1g_entries[0].animation_id == 7U &&
          level.structure1g_entries[0].first_image_index == 0x0156U &&
          level.structure1g_entries[0].sequence_instruction_count == 2 &&
          level.structure1g_entries[0].image_instruction_count == 1 &&
          nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0 &&
          handoff.structure1g_present && handoff.structure1g_valid &&
          handoff.structure1g_animated_texture_count == level.structure1g_entry_count,
          "Structure1G typed animation declarations are consumed by host handoff");
    structure1[structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 24 + 24] = 0U;
    CHECK(nexus_v1_dgn_structure1_layout(&layout, dgn, (int)sizeof(dgn)) == 0 &&
          !layout.structure1g.valid,
          "Structure1G rejects an unterminated animation instruction stream");
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0 &&
          handoff.status == NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE_SEMANTICS &&
          handoff.blocks_real_dgn_mesh_render && !handoff.fallback_visuals_permitted,
          "invalid declared Structure1G blocks host promotion without fallback visuals");
}

static void test_structure1g_animated_floor_material_handoff(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 21];
    const int structure1b_rel = 0x40;
    uint8_t *structure1;
    uint8_t *cell;
    Nexus_V1_Level level;
    Nexus_V1_DgnRendererHandoffReceipt handoff;
    Nexus_V1_DgnRenderCommand commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnRenderCommand source_commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnRenderPlanReceipt receipt;
    Nexus_V1_DgnStructure2FloorCommandSource sources[2];
    Nexus_V1_DgnStructure2FloorCommandSourceReceipt source_receipt;
    int source_command_count;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, 512) == 0,
          "Structure1G animated-floor fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    build_structure1g_fixture(structure1, structure1b_rel);
    build_structure2_fixture(dgn);
    cell = cell_at(structure1, structure1b_rel, 1, 1);
    cell[0] = 0x03U;
    cell[1] = 0x80U;
    cell[4] = 3U;
    set_collision_ref(structure1, structure1b_rel, 1, 0, 0x0fff);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          level.structure1g_floor_animation_cell_count == 1 &&
          level.structure1g_floor_animation_bound_count == 1,
          "Structure1B animated-floor flag binds only a declared Structure1G id");
    CHECK(level.structure2_texture_table_valid && level.structure2_texture_count == 11 &&
          level.structure1g_entries[0].first_structure2_image_valid &&
          level.structure1g_entries[0].first_structure2_image_id == 10U &&
          level.structure1g_entries[0]
              .structure2_image_instruction_bound_count == 1 &&
          level.structure1g_entries[0]
              .structure2_image_instruction_unbound_count == 0,
          "Structure1G global image instructions bind only through local Structure2 descriptors");
    CHECK(level.structure1g_structure2_bindings_complete,
          "complete Structure1G image declarations are required before host handoff");
    CHECK(level.structure2_payload.valid &&
          level.structure2_payload.descriptor_bytes == 220 &&
          level.structure2_payload.terminator_offset == 220 &&
          level.structure2_payload.opaque_payload_offset == 222 &&
          level.structure2_payload.opaque_payload_size == 18 &&
          level.structure2_payload.opaque_payload_zero_byte_count == 18 &&
          level.structure2_payload.opaque_payload_nonzero_byte_count == 0 &&
          level.structure2_payload.opaque_payload_complete_pair_count == 9 &&
          level.structure2_payload.opaque_payload_trailing_byte_count == 0 &&
          level.structure2_payload.opaque_payload_zero_pair_count == 9 &&
          level.structure2_payload.opaque_payload_nonzero_pair_count == 0 &&
          level.structure2_payload.nonzero_descriptor_offset_count == 0 &&
          level.structure2_payload.nonzero_descriptor_offsets_unaligned_count == 0 &&
          level.structure2_payload.nonzero_descriptor_offsets_word_bounded_count == 0 &&
          level.structure2_payload.nonzero_descriptor_offset_unique_count == 0 &&
          level.structure2_payload.nonzero_descriptor_offset_reused_count == 0 &&
          level.structure2_payload.descriptor_offset_envelope_valid &&
          !level.structure2_payload.local_payload_offset_pattern_observed &&
          !level.structure2_payload
              .local_payload_word_aligned_offset_pattern_observed &&
          !level.structure2_payload
              .local_payload_word_bounded_offset_pattern_observed &&
          !level.structure2_payload.material_or_image_data_proven,
          "Structure2 retains only a bounded opaque payload after its FFFF terminator");
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 12, 222U);
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 16, 224U);
    dgn[NEXUS_DGN_BLOCK_SIZE * 20 + 222] = 0xa5U;
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          level.structure2_payload.opaque_payload_zero_byte_count == 17 &&
          level.structure2_payload.opaque_payload_nonzero_byte_count == 1 &&
          level.structure2_payload.opaque_payload_complete_pair_count == 9 &&
          level.structure2_payload.opaque_payload_trailing_byte_count == 0 &&
          level.structure2_payload.opaque_payload_zero_pair_count == 8 &&
          level.structure2_payload.opaque_payload_nonzero_pair_count == 1 &&
          level.structure2_payload.nonzero_descriptor_offset_count == 2 &&
          level.structure2_payload
              .nonzero_descriptor_offsets_in_opaque_payload_count == 2 &&
          level.structure2_payload
              .nonzero_descriptor_offsets_outside_opaque_payload_count == 0 &&
          level.structure2_payload
              .nonzero_descriptor_offsets_word_bounded_count == 2 &&
          level.structure2_payload.nonzero_descriptor_offset_unique_count == 2 &&
          level.structure2_payload.nonzero_descriptor_offset_reused_count == 0 &&
          level.structure2_payload.local_payload_offset_pattern_observed &&
          level.structure2_payload
              .local_payload_word_aligned_offset_pattern_observed &&
          level.structure2_payload
              .local_payload_word_bounded_offset_pattern_observed &&
          level.structure2_payload.descriptor_offset_envelope_valid &&
          !level.structure2_payload.material_or_image_data_proven,
          "Structure2 records bounded descriptor-offset correlation without decoding payload bytes");
    dgn[NEXUS_DGN_BLOCK_SIZE * 20 + 222] = 0U;
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 16, 222U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          level.structure2_payload.nonzero_descriptor_offset_count == 2 &&
          level.structure2_payload.nonzero_descriptor_offset_unique_count == 1 &&
          level.structure2_payload.nonzero_descriptor_offset_reused_count == 1 &&
          level.structure2_payload.local_payload_word_aligned_offset_pattern_observed &&
          level.structure2_payload.local_payload_word_bounded_offset_pattern_observed &&
          !level.structure2_payload.material_or_image_data_proven,
          "Structure2 retains a repeated in-span target as opaque layout provenance");
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 16, 224U);
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 12, 223U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          level.structure2_payload.nonzero_descriptor_offset_count == 2 &&
          level.structure2_payload
              .nonzero_descriptor_offsets_unaligned_count == 1 &&
          level.structure2_payload.local_payload_offset_pattern_observed &&
          !level.structure2_payload
              .local_payload_word_aligned_offset_pattern_observed &&
          level.structure2_payload
              .local_payload_word_bounded_offset_pattern_observed &&
          !level.structure2_payload.descriptor_offset_envelope_valid &&
          !nexus_v1_level_structure2_source_envelope_valid(&level) &&
          !level.structure2_payload.material_or_image_data_proven,
          "Structure2 records an unaligned in-span target without promoting it");
    CHECK(nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0 &&
          handoff.status ==
              NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_ENVELOPE &&
          !handoff.structure2_descriptor_offset_envelope_valid &&
          handoff.blocks_real_dgn_mesh_render &&
          !handoff.fallback_visuals_permitted,
          "unaligned Structure2 descriptor targets fail closed at the host handoff");
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 12, 240U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          level.structure2_payload.nonzero_descriptor_offset_count == 2 &&
          level.structure2_payload
              .nonzero_descriptor_offsets_in_opaque_payload_count == 1 &&
          level.structure2_payload
              .nonzero_descriptor_offsets_outside_opaque_payload_count == 1 &&
          !level.structure2_payload.local_payload_offset_pattern_observed &&
          !level.structure2_payload
              .local_payload_word_bounded_offset_pattern_observed &&
          !level.structure2_payload.descriptor_offset_envelope_valid &&
          !nexus_v1_level_structure2_source_envelope_valid(&level) &&
          !level.structure2_payload.material_or_image_data_proven,
          "Structure2 leaves out-of-span descriptor offsets non-promoting");
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 12, 238U);
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 16, 239U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          level.structure2_payload
              .nonzero_descriptor_offsets_in_opaque_payload_count == 2 &&
          level.structure2_payload
              .nonzero_descriptor_offsets_word_bounded_count == 1 &&
          !level.structure2_payload
              .local_payload_word_bounded_offset_pattern_observed &&
          !level.structure2_payload.descriptor_offset_envelope_valid &&
          !nexus_v1_level_structure2_source_envelope_valid(&level) &&
          !level.structure2_payload.material_or_image_data_proven,
          "Structure2 keeps a trailing opaque-byte target non-promoting");
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 12, 0U);
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 16, 0U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          level.structure2_payload.descriptor_offset_envelope_valid &&
          nexus_v1_level_structure2_source_envelope_valid(&level) &&
          level.geometry_info.mesh_ready,
          "animated-floor fixture retains a mesh-ready DGN handoff");
    CHECK(nexus_v1_level_build_dgn_view_render_plan(
              &level, 1, 1, 0, commands,
              NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS, &receipt) == 0,
          "animated-floor render plan builds");
    CHECK(receipt.animated_material_command_count == 1,
          "animated floor declaration reaches one host material command");
    CHECK(commands[0].animated_texture_structure2_image_valid &&
          commands[0].animated_texture_structure1g_entry_index == 0 &&
          commands[0].animated_texture_structure1g_sequence_word_offset == 0U &&
          commands[0].animated_texture_structure2_image_id == 10U &&
          commands[0].animated_texture_host_route ==
              NEXUS_V1_DGN_ANIMATED_MATERIAL_ROUTE_STRUCTURE2_FLOOR,
          "animated floor host route retains its typed Structure2 image identifier");
    source_command_count = receipt.command_count;
    memcpy(source_commands, commands, sizeof(source_commands));
    memset(sources, 0, sizeof(sources));
    memset(&source_receipt, 0, sizeof(source_receipt));
    CHECK(nexus_v1_dgn_bind_structure2_animated_floor_sources(
              &level, commands, receipt.command_count, sources, 2,
              &source_receipt) == 0 &&
          source_receipt.animated_floor_command_count == 1 &&
          source_receipt.structure1g_provenance_count == 1 &&
          source_receipt.global_image_index_binding_count == 1 &&
          source_receipt.complete_sequence_provenance_count == 1 &&
          source_receipt.descriptor_offset_envelope_count == 1 &&
          source_receipt.source_command_count == 1 && source_receipt.complete &&
          !source_receipt.fallback_visuals_permitted &&
          sources[0].command_index == 0 &&
          sources[0].structure1g_entry_index == 0 &&
          sources[0].structure1g_sequence_word_offset == 0U &&
          sources[0].structure1g_global_image_index == 0x0156U &&
          sources[0].structure1g_sequence_instruction_count == 2 &&
          sources[0].structure1g_sequence_image_instruction_count == 1 &&
          sources[0].structure1g_sequence_goto_instruction_count == 0 &&
          sources[0].structure1g_sequence_bound_image_count == 1 &&
          sources[0].structure1g_sequence_unbound_image_count == 0 &&
          sources[0].image_offset_word_bounded &&
          sources[0].palette_offset_word_bounded &&
          sources[0].image_id == 10U &&
          sources[0].encoding == 8U && sources[0].palette_id == 0U &&
          sources[0].width == 16U && sources[0].height == 16U &&
          sources[0].structure2_source_envelope_valid &&
          !sources[0].payload_decoder_proven && !sources[0].draw_authorized,
          "Structure1G/Structure2 binds raw original descriptor provenance to its exact floor command only");
    commands[0].animated_texture_structure1g_entry_index = 1;
    CHECK(nexus_v1_dgn_bind_structure2_animated_floor_sources(
              &level, commands, receipt.command_count, sources, 2,
              &source_receipt) == 0 &&
          source_receipt.source_command_count == 0 &&
          source_receipt.blocked_structure1g_provenance_count == 1 &&
          !source_receipt.complete && !source_receipt.fallback_visuals_permitted,
          "a Structure2 descriptor cannot be detached from its original Structure1G entry");
    memcpy(commands, source_commands, sizeof(commands));
    level.structure1g_entries[0].first_image_index = 0x0157U;
    commands[0].animated_texture_first_image_index = 0x0157U;
    CHECK(nexus_v1_dgn_bind_structure2_animated_floor_sources(
              &level, commands, receipt.command_count, sources, 2,
              &source_receipt) == 0 &&
          source_receipt.source_command_count == 0 &&
          source_receipt.blocked_global_image_index_count == 1 &&
          !source_receipt.complete && !source_receipt.fallback_visuals_permitted,
          "a Structure1G global image index must resolve to the declared local Structure2 id");
    level.structure1g_entries[0].first_image_index = 0x0156U;
    memcpy(commands, source_commands, sizeof(commands));
    level.structure1g_entries[0].structure2_image_instruction_unbound_count = 1;
    CHECK(nexus_v1_dgn_bind_structure2_animated_floor_sources(
              &level, commands, receipt.command_count, sources, 2,
              &source_receipt) == 0 &&
          source_receipt.source_command_count == 0 &&
          source_receipt.blocked_sequence_provenance_count == 1 &&
          !source_receipt.complete && !source_receipt.fallback_visuals_permitted,
          "an incompletely bound Structure1G sequence cannot reach a DGN floor command");
    level.structure1g_entries[0].structure2_image_instruction_unbound_count = 0;
    level.structure2_textures[10].image_relative_offset = 240U;
    CHECK(nexus_v1_dgn_bind_structure2_animated_floor_sources(
              &level, commands, receipt.command_count, sources, 2,
              &source_receipt) == 0 &&
          source_receipt.source_command_count == 0 &&
          source_receipt.blocked_descriptor_offset_envelope_count == 1 &&
          !source_receipt.complete && !source_receipt.fallback_visuals_permitted,
          "a Structure2 descriptor target outside its raw opaque span cannot reach a floor command");
    level.structure2_textures[10].image_relative_offset = 0U;
    CHECK(receipt.unresolved_animated_material_count == 1,
          "animated floor declaration remains unresolved without Structure2 handoff");
    CHECK(receipt.blocks_real_dgn_mesh_render && !receipt.fallback_visuals_permitted,
          "animated floor declaration blocks until a proven Structure2 material handoff exists");
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 12, 223U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          nexus_v1_dgn_bind_structure2_animated_floor_sources(
              &level, source_commands, source_command_count, sources, 2,
              &source_receipt) == 0 &&
          source_receipt.source_command_count == 0 &&
          source_receipt.blocked_source_envelope_count == 1 &&
          !source_receipt.complete && !source_receipt.fallback_visuals_permitted,
          "unaligned Structure2 source envelopes cannot reach a floor command");
    wb32(dgn + NEXUS_DGN_BLOCK_SIZE * 20 + 12, 0U);
    /* A bounded sequence that names an absent local descriptor is not a
     * partially valid animation. It must block at handoff, even before a
     * particular viewport happens to look at that floor. */
    wb16(structure1 + structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 24 + 8,
         0x0157U);
    wb16(structure1 + structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 24 + 20,
         0x0157U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0,
          "unbound Structure1G fixture reloads");
    CHECK(!level.structure1g_structure2_bindings_complete,
          "unbound Structure1G image instruction invalidates the complete binding");
    CHECK(nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0,
          "unbound Structure1G handoff receipt builds");
    CHECK(handoff.status == NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_SOURCE,
          "unbound Structure1G image instruction has the Structure2 handoff status");
    CHECK(!handoff.structure1g_structure2_bindings_complete,
          "unbound Structure1G image instruction reaches the host receipt");
    CHECK(handoff.blocks_real_dgn_mesh_render && !handoff.fallback_visuals_permitted,
          "unbound Structure1G image instruction blocks the complete DGN host route");
    wb16(structure1 + structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 24 + 8,
         0x0156U);
    wb16(structure1 + structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 24 + 20,
         0x0156U);
    wb32(dgn + 0x18, 221U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          !level.structure2_texture_table_valid && !level.structure2_payload.valid,
          "Structure2 rejects a terminator that is not fully bounded by useful bytes");
    wb32(dgn + 0x18, 240U);
    wb16(dgn + NEXUS_DGN_BLOCK_SIZE * 20 +
             10 * NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES, 11U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          !level.structure2_texture_table_valid &&
          !level.structure1g_entries[0].first_structure2_image_valid,
          "noncanonical Structure2 descriptor IDs cannot fabricate an animated-material route");
    wb16(dgn + NEXUS_DGN_BLOCK_SIZE * 20 +
             10 * NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES, 10U);
    wb16(dgn + NEXUS_DGN_BLOCK_SIZE * 20 +
             10 * NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES + 2, 0x1234U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          !level.structure2_texture_table_valid &&
          !level.structure2_payload.valid &&
          !level.structure1g_entries[0].first_structure2_image_valid,
          "unobserved Structure2 encoding classes cannot become material anchors");
    wb16(dgn + NEXUS_DGN_BLOCK_SIZE * 20 +
             10 * NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES + 2, 0x0008U);
    wb16(dgn + NEXUS_DGN_BLOCK_SIZE * 20 +
             10 * NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES + 6, 0U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          !level.structure2_texture_table_valid &&
          !level.structure2_payload.valid &&
          !level.structure1g_entries[0].first_structure2_image_valid,
          "zero-width Structure2 descriptors cannot become material anchors");
    wb16(dgn + NEXUS_DGN_BLOCK_SIZE * 20 +
             10 * NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES + 6, 16U);
    wb16(dgn + NEXUS_DGN_BLOCK_SIZE * 20 +
             10 * NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES + 8, 0U);
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          !level.structure2_texture_table_valid &&
          !level.structure2_payload.valid &&
          !level.structure1g_entries[0].first_structure2_image_valid,
          "zero-height Structure2 descriptors cannot become material anchors");
}

static void test_dgn_view_render_plan_from_structure1b(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    const int structure1b_rel = 0x40;
    const int geometry_bytes = 2048;
    Nexus_V1_Level level;
    Nexus_V1_DgnRenderCommand commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnRenderPlanReceipt receipt;
    Nexus_V1_DgnCellGeometry cell;
    uint8_t *structure1;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, geometry_bytes) == 0,
          "render-plan DGN fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    structure1[9] = 22;
    set_floor_flags(structure1, structure1b_rel, 3, 4,
                    (uint16_t)((1U << 14) | (21U << 7) | (2U << 4) | 2U));
    set_collision_ref(structure1, structure1b_rel, 3, 4, 1);
    cell_at(structure1, structure1b_rel, 3, 4)[3] = 4;
    cell_at(structure1, structure1b_rel, 3, 4)[4] = 41;
    set_post_grid_0x30_ref(structure1, structure1b_rel, 3, 4, 1);
    structure1[structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 152 +
               NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES +
               NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_BYTE] = 0x81U;
    cell_at(structure1, structure1b_rel, 4, 4)[3] = 12;
    set_collision_ref(structure1, structure1b_rel, 4, 4, 1);
    set_post_grid_0x30_ref(structure1, structure1b_rel, 4, 4, 2);
    set_collision_ref(structure1, structure1b_rel, 3, 3, 1);
    set_post_grid_0x30_ref(structure1, structure1b_rel, 3, 3, 3);
    set_collision_ref(structure1, structure1b_rel, 2, 4, 0x0fff);
    set_collision_ref(structure1, structure1b_rel, 2, 3, 0x0fff);
    set_collision_ref(structure1, structure1b_rel, 4, 3, 0x0fff);
    set_collision_ref(structure1, structure1b_rel, 3, 2, 0x0fff);
    dgn[NEXUS_DGN_BLOCK_SIZE + structure1b_rel +
        NEXUS_DGN_STRUCTURE1B_BYTES + 4] = (uint8_t)-20;
    dgn[NEXUS_DGN_BLOCK_SIZE + structure1b_rel +
        NEXUS_DGN_STRUCTURE1B_BYTES + 5] = (uint8_t)-10;
    dgn[NEXUS_DGN_BLOCK_SIZE + structure1b_rel +
        NEXUS_DGN_STRUCTURE1B_BYTES + 6] = 20;
    dgn[NEXUS_DGN_BLOCK_SIZE + structure1b_rel +
        NEXUS_DGN_STRUCTURE1B_BYTES + 7] = 10;

    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 4) == 0,
          "render-plan level loads from DMWeb DGN");
    CHECK(nexus_v1_level_get_cell_geometry(&level, 3, 4, &cell) == 0 &&
          cell.post_grid_0x30_ref == level.post_grid_0x30_refs[4][3] &&
          cell.post_grid_0x30_row_prefix_valid &&
          cell.floor_material_ref == 21 && cell.floor_height[1] == 12 &&
          !cell.collision_sector.valid,
          "movement and viewport retain a bounded opaque Structure1C reference");
    memset(commands, 0, sizeof(commands));
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_v1_level_build_dgn_view_render_plan(
              &level,
              3,
              4,
              0,
              commands,
              NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
              &receipt) == 0,
          "DGN view render plan builds");
    CHECK(receipt.status == NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH &&
          receipt.plan_ready == 1 &&
          receipt.blocks_real_dgn_mesh_render == 0 &&
          receipt.fallback_visuals_permitted == 0,
          "DGN view render plan is ready without fallback visuals");
    CHECK(receipt.command_count == 10 &&
          receipt.floor_count == 3 &&
          receipt.ceiling_count == 3 &&
          receipt.wall_count == 4,
          "DGN view render plan emits bounded floor/ceiling/wall commands");
    CHECK(receipt.material_semantics_complete == 1 &&
          receipt.floor_material_command_count == receipt.floor_count &&
          receipt.ceiling_material_command_count == receipt.ceiling_count &&
          receipt.wall_material_command_count == receipt.wall_count,
          "DGN render plan proves Structure1B surface semantics per command family");
    CHECK(receipt.post_grid_0x30_reference_command_count > 0 &&
          receipt.post_grid_0x30_valid_reference_command_count ==
              receipt.post_grid_0x30_reference_command_count &&
          receipt.first_post_grid_0x30_ref == 1 &&
          receipt.max_post_grid_0x30_ref == 3,
          "DGN view render plan consumes only bounded opaque-record references");
    CHECK(receipt.post_grid_0x30_row_ordinal_flagged_prefix_record_count == 1 &&
          receipt.post_grid_0x30_first_row_ordinal_flagged_prefix_record == 1 &&
          receipt.post_grid_0x30_last_row_ordinal_flagged_prefix_record == 1,
          "DGN render plan carries row-order flag provenance without resolving references as ordinals");
    CHECK(commands[0].kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR &&
          commands[0].x == 3 &&
          commands[0].y == 4 &&
          commands[0].collision_ref == 1 &&
          commands[0].post_grid_0x30_ref == 1 &&
          commands[0].post_grid_0x30_row_prefix_valid &&
          commands[0].material_id == 21 && commands[0].floor_rotation == 1 &&
          commands[0].material_source_kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR &&
          commands[0].floor_slope == 2 &&
          commands[0].floor_height[0] == 4 &&
          commands[0].floor_height[1] == 12 &&
          commands[0].quad_y[0] == 1008 &&
          commands[0].quad_y[1] == 688 &&
          commands[0].quad_y[2] == 496 &&
          commands[0].quad_y[3] == 603 &&
          !commands[0].collision_sector.valid &&
          commands[0].palette_index == 21 &&
          commands[0].quad_y[0] > commands[0].quad_y[2],
          "DGN plan projects Structure1B floor heights instead of a flat host quad");
    CHECK(commands[1].kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING &&
          commands[1].material_id == 22 &&
          commands[1].material_source_kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING &&
          commands[1].floor_height[2] == 12 &&
          commands[1].ceiling_height[2] == 44 &&
          commands[1].quad_y[0] == -272 &&
          commands[1].quad_y[1] == 176 &&
          commands[1].quad_y[2] == 70 &&
          commands[1].quad_y[3] == -592 &&
          commands[1].palette_index == 22 &&
          commands[1].quad_y[0] < commands[1].quad_y[1],
          "DGN plan projects matching Structure1B ceiling heights");
    CHECK(commands[4].kind == NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT &&
          commands[4].wall_dir == 3 &&
          commands[4].x == 3 &&
          commands[4].y == 4 &&
          commands[4].material_id == 41 &&
          commands[4].material_source_kind == NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT &&
          commands[4].quad_x[0] < commands[4].quad_x[1],
          "DGN render plan projects the left wall from Structure1B visibility");
    CHECK(receipt.first_blocking_depth == 2 &&
          receipt.first_blocking_x == 3 &&
          receipt.first_blocking_y == 2 &&
          commands[9].kind == NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT &&
          commands[9].quad_y[0] > commands[9].quad_y[2],
          "DGN render plan stops at first front wall");
}

static void test_structure1f_out_of_prefix_ref_blocks_mesh(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    const int structure1b_rel = 0x40;
    Nexus_V1_DgnGeometryInfo info;
    Nexus_V1_DgnRendererHandoffReceipt handoff;
    Nexus_V1_Level level;
    Nexus_V1_DgnRenderCommand commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnRenderPlanReceipt receipt;
    uint8_t *structure1;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, 256) == 0,
          "Structure1F prefix-overrun fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    set_collision_ref(structure1, structure1b_rel, 3, 3, 1);
    /* 256 bytes gives six rows: records 0..4 are the observed prefix;
     * record 5 is the opaque tail and cannot be selected by a cell. */
    set_post_grid_0x30_ref(structure1, structure1b_rel, 3, 3, 5);

    CHECK(nexus_v1_dgn_geometry_info(&info, dgn, (int)sizeof(dgn)) == 0 &&
          !info.post_grid_0x30_references_valid &&
          info.post_grid_0x30_invalid_ref_count == 1 &&
          info.first_invalid_post_grid_0x30_ref == 5 &&
          !info.mesh_ready,
          "an opaque Structure1F tail reference blocks mesh promotion");
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 1) == 0 &&
          nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0 &&
          handoff.status ==
              NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_REFERENCE &&
          !handoff.post_grid_0x30_references_valid &&
          handoff.first_invalid_post_grid_0x30_ref == 5 &&
          handoff.blocks_real_dgn_mesh_render &&
          !handoff.fallback_visuals_permitted,
          "DGN handoff identifies and fails closed for an unproven Structure1F reference");
    CHECK(strcmp(nexus_v1_dgn_renderer_handoff_status_name(handoff.status),
                 "blocked-structure1f-reference") == 0,
          "unproven Structure1F reference keeps a stable no-fallback receipt name");
    CHECK(nexus_v1_level_build_dgn_view_render_plan(
              &level, 3, 3, 0, commands,
              NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS, &receipt) == 0 &&
          receipt.blocks_real_dgn_mesh_render && receipt.command_count == 0,
          "blocked Structure1F reference emits no synthetic render plan");
}

static void test_descriptor_budget_blocks_mesh_ready(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 21];
    const int structure1b_rel = 0x40;
    Nexus_V1_DgnGeometryInfo info;
    Nexus_V1_DgnRendererHandoffReceipt handoff;
    Nexus_V1_Level level;
    uint8_t *structure1;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 20,
                          structure1b_rel, 8) == 0,
          "small-geometry DGN buffer builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    set_collision_ref(structure1, structure1b_rel, 8, 8, 3);
    set_post_grid_0x30_ref(structure1, structure1b_rel, 8, 8, 10);

    CHECK(nexus_v1_dgn_geometry_info(&info, dgn, (int)sizeof(dgn)) == 0,
          "small-geometry DGN still parses");
    CHECK(info.max_collision_ref == 3,
          "small-geometry max ref captured");
    CHECK(info.max_post_grid_0x30_ref == 10,
          "small-geometry max opaque-record ref captured");
    CHECK(info.geometry_size == 8,
          "small-geometry span size captured");
    CHECK(info.mesh_ready == 0,
          "descriptor budget overflow does not promote mesh readiness");
    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 2) == 0,
          "small-geometry DGN still loads as a level");
    CHECK(nexus_v1_level_dgn_renderer_handoff_receipt(&level, &handoff) == 0,
          "DGN renderer handoff receipt builds for budget-blocked level");
    CHECK(handoff.status ==
          NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_NO_GEOMETRY,
          "DGN renderer handoff blocks an untyped 0x30 span");
    CHECK(handoff.blocks_real_dgn_mesh_render == 1 &&
          handoff.fallback_visuals_permitted == 0,
          "DGN untyped-span handoff forbids fallback visuals");
}

static void test_post_grid_0x30_row_prefix_rejection(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    const int structure1b_rel = 0x40;
    uint8_t *structure1;
    Nexus_V1_DgnStructure1Layout layout;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, 256) == 0,
          "row-prefix rejection fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    structure1[structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 152 +
               NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_BYTE] = 0x40U;
    CHECK(nexus_v1_dgn_structure1_layout(&layout, dgn, (int)sizeof(dgn)) == 0 &&
          !layout.post_grid_0x30_records.valid &&
          !layout.post_grid_0x30_records.row_ordinal_prefix_valid,
          "0x30 row prefix rejects an unobserved ordinal bit");
}

static void test_structure1c_bytes_do_not_invent_collision_geometry(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 20];
    const int structure1b_rel = 0x40;
    Nexus_V1_Level level;
    uint8_t *structure1;
    uint8_t *sector;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 19,
                          structure1b_rel, 2048) == 0,
          "Structure1C movement fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    set_collision_ref(structure1, structure1b_rel, 8, 8, 1);
    set_collision_ref(structure1, structure1b_rel, 8, 9, 1);
    wb32(structure1 + 0x24,
         (uint32_t)(structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES + 8));
    sector = dgn + NEXUS_DGN_BLOCK_SIZE + structure1b_rel +
             NEXUS_DGN_STRUCTURE1B_BYTES;
    sector[0] = 2; /* one descriptor follows */
    sector[4] = (uint8_t)-100;
    sector[5] = 40;
    sector[6] = 100;
    sector[7] = 40;

    CHECK(nexus_v1_level_load(&level, dgn, (int)sizeof(dgn), 5) == 0,
          "Structure1C movement level loads");
    CHECK(nexus_v1_level_move_allowed(&level, 8, 9, 8, 8),
          "opaque Structure1C bytes do not invent a blocking segment");
    CHECK(nexus_v1_level_move_allowed(&level, 7, 8, 8, 8),
          "documented Structure1B passability remains usable");
}

static void test_bounds_and_legacy_non_promotion(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 21];
    uint8_t legacy[64];
    Nexus_V1_DgnGeometryInfo info;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 20, 0x40, 16) == 0,
          "bounds fixture builds");
    wb32(dgn + 0x10, (uint32_t)(0x40 + NEXUS_DGN_STRUCTURE1B_BYTES - 1));
    CHECK(nexus_v1_dgn_geometry_info(&info, dgn, (int)sizeof(dgn)) != 0,
          "truncated useful Structure1 size is rejected");

    memset(legacy, 0, sizeof(legacy));
    wb16(legacy + 0, 4U);
    wb16(legacy + 2, 4U);
    CHECK(nexus_v1_dgn_geometry_info(&info, legacy, (int)sizeof(legacy)) != 0,
          "legacy synthetic layout does not parse as DMWeb geometry info");
    CHECK(nexus_v1_level_load(NULL, legacy, (int)sizeof(legacy), 99) != 0,
          "Nexus DGN loader rejects unsupported input before mutating runtime state");
    {
        Nexus_V1_Level level;
        CHECK(nexus_v1_level_load(&level, legacy, (int)sizeof(legacy), 99) != 0,
              "legacy raw-grid fixture cannot become a Nexus runtime level");
    }
}

static void test_determinism(void) {
    uint8_t dgn[NEXUS_DGN_BLOCK_SIZE * 21];
    Nexus_V1_DgnGeometryInfo a;
    Nexus_V1_DgnGeometryInfo b;
    uint8_t *structure1;

    CHECK(build_dmweb_dgn(dgn, (int)sizeof(dgn), 20, 0x308, 128) == 0,
          "determinism fixture builds");
    structure1 = dgn + NEXUS_DGN_BLOCK_SIZE;
    set_collision_ref(structure1, 0x308, 10, 10, 7);
    set_collision_ref(structure1, 0x308, 11, 10, 2);

    CHECK(nexus_v1_dgn_geometry_info(&a, dgn, (int)sizeof(dgn)) == 0,
          "first deterministic parse succeeds");
    CHECK(nexus_v1_dgn_geometry_info(&b, dgn, (int)sizeof(dgn)) == 0,
          "second deterministic parse succeeds");
    CHECK(memcmp(&a, &b, sizeof(a)) == 0,
          "geometry info parse is byte-stable across runs");
}

static void test_structure1f_item_ibs_material_binding(void) {
    uint8_t *ibs = (uint8_t *)calloc(1, NEXUS_V1_ITEM_IBS_BYTES);
    Nexus_V1_ItemIbsBank bank;
    Nexus_V1_Level level;
    Nexus_V1_DgnRenderCommand commands[2];
    Nexus_V1_DgnStructure1FItemMaterialBinding bindings[2];
    Nexus_V1_DgnStructure1FItemMaterialReceipt receipt;
    Nexus_V1_DgnCommandPacked4BppMaterial materials[2];
    Nexus_V1_DgnCommandPacked4BppMaterialReceipt material_receipt;
    Nexus_V1_DgnStructure1FItemIbsCoverageReceipt coverage;
    Nexus_V1_ItemIbs0008Vdp1CaptureCandidate candidate;
    Nexus_V1_ItemIbs0008Vdp1CaptureBindingReceipt capture;
    Nexus_V1_ItemIbs0008CodecReceipt codec_receipt;
    uint8_t decoded[NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_MAX_TEXELS];
    uint8_t vdp1_command[NEXUS_V1_VDP1_COMMAND_BYTES];
    int i;

    CHECK(ibs != NULL, "ITEM.IBS material fixture allocates");
    if (!ibs) return;
    /* DMWeb ITEM.IBS: 243 x 40-byte declarations at 0x0800, regular
     * palettes at 0x3000, association table at 0x3100 and 16x16 4bpp
     * images at 0x3300.  The fixture proves the documented FFFF route only. */
    for (i = 0; i < NEXUS_V1_ITEM_IBS_DECLARATION_COUNT; ++i) {
        uint8_t *decl = ibs + 0x0800 + i * 40;
        decl[0] = (uint8_t)i;
        wb16(decl + 0x14, 3U);
        wb16(decl + 0x16, 0xffffU);
    }
    wb16(ibs + 0x3000 + 1 * 16 * 2 + 2 * 2, 0x7c00U);
    ibs[0x3100 + 3 * 2] = 1;
    ibs[0x3100 + 3 * 2 + 1] = 7;
    ibs[0x3300 + 7 * 128] = 0x2fU;
    wb16(ibs + 0xa800, 43U); /* 223 regular images + ordinal 43 = 266 */
    wb16(ibs + 0xa802, 8U);
    wb16(ibs + 0xa806, 16U);
    wb16(ibs + 0xa808, 16U);
    wb32(ibs + 0xa80c, 0x1000U);
    wb32(ibs + 0xa810, 0x0900U);
    wb16(ibs + 0xa800 + 20, 0xffffU);
    wb32(ibs + 0xa800 + 20 + 12, 0x1100U);
    wb16(ibs + 0xa800 + 0x0900, 0x03e0U);
    ibs[0xa800 + 0x1000] = 0x6cU;

    CHECK(nexus_v1_item_ibs_parse_verified(ibs, NEXUS_V1_ITEM_IBS_BYTES,
                                             0, &bank) != 0,
          "unverified ITEM.IBS never becomes a material bank");
    CHECK(nexus_v1_item_ibs_parse_verified(ibs, NEXUS_V1_ITEM_IBS_BYTES,
                                             1, &bank) == 0,
          "hash-verified ITEM.IBS regular-icon table parses");
    memset(&level, 0, sizeof(level));
    level.structure1f_entry_count = 2;
    level.structure1f_entries[0].family = NEXUS_V1_DGN_STRUCTURE1F_ITEMS;
    level.structure1f_entries[0].x = 4;
    level.structure1f_entries[0].y = 5;
    level.structure1f_entries[0].item_id = 5;
    level.structure1f_entries[1].family = NEXUS_V1_DGN_STRUCTURE1F_ITEMS;
    level.structure1f_entries[1].x = 6;
    level.structure1f_entries[1].y = 7;
    level.structure1f_entries[1].item_id = 6;
    bank.floor_image[6] = 266U; /* documented separate floor-image range */
    memset(commands, 0, sizeof(commands));
    commands[0].kind = NEXUS_V1_DGN_RENDER_COMMAND_FLOOR;
    commands[0].x = 4;
    commands[0].y = 5;
    commands[1].kind = NEXUS_V1_DGN_RENDER_COMMAND_FLOOR;
    commands[1].x = 6;
    commands[1].y = 7;
    CHECK(nexus_v1_dgn_bind_structure1f_item_materials(
              &level, &bank, commands, 2, bindings, 2, &receipt) == 0,
          "Structure1Fa items bind to matching mesh-command cells");
    CHECK(nexus_v1_dgn_structure1f_item_ibs_coverage(&level, &bank, &coverage) == 0 &&
          coverage.complete && coverage.dgn_item_entry_count == 2 &&
          coverage.inventory_inherited_item_count == 1 &&
          coverage.special_floor_reference_count == 1 &&
          coverage.special_floor_0008_count == 1 &&
          !coverage.fallback_visuals_permitted,
          "Structure1Fa coverage binds each source item to ITEM.IBS without a substitute");
    CHECK(receipt.bound_regular_inventory_count == 1 &&
          receipt.bound_special_floor_palette_count == 1 &&
          !receipt.fallback_visuals_permitted,
          "special floor descriptor carries its local palette but no pixel fallback");
    CHECK(bindings[0].command_index == 0 && bindings[0].item_id == 5 &&
          bindings[0].palette_index == 1 && bindings[0].image_index == 7 &&
          bindings[0].palette_bgr555[2] == 0x7c00U &&
          bindings[0].packed_4bpp_texels[0] == 0x2fU,
          "binding carries the exact ITEM.IBS palette and packed texels");
    CHECK(bindings[1].special_floor_image != NULL &&
          bindings[1].special_floor_image->encoding == 8U &&
          bindings[1].special_floor_image->palette_bgr555[0] == 0x03e0U &&
          bindings[1].special_floor_image->image_id == 266U &&
          bindings[1].special_floor_image->packed_4bpp_bytes == 128U &&
          bindings[1].special_floor_image->packed_4bpp_valid &&
          bindings[1].packed_4bpp_texels == NULL,
          "special floor binding preserves bounded original payload and palette only");
    memset(decoded, 0xa5, sizeof(decoded));
    CHECK(nexus_v1_item_ibs_decode_0008_vdp1_4bpp(
              bindings[1].special_floor_image, NULL, decoded,
              (int)sizeof(decoded), &codec_receipt) == 0 &&
          codec_receipt.descriptor_0008_verified &&
          codec_receipt.packed_span_verified && codec_receipt.palette_bound &&
          codec_receipt.blocked_missing_vdp1_command_provenance &&
          !codec_receipt.decode_authorized && decoded[0] == 0xa5U &&
          !codec_receipt.fallback_visuals_permitted,
          "ITEM.IBS alone cannot authorize a VDP1 nibble-order decode");
    memset(&candidate, 0, sizeof(candidate));
    candidate.item_ibs_fnv1a64 = fnv1a64(ibs, NEXUS_V1_ITEM_IBS_BYTES);
    candidate.image_id = bindings[1].special_floor_image->image_id;
    candidate.encoding = bindings[1].special_floor_image->encoding;
    candidate.width = bindings[1].special_floor_image->width;
    candidate.height = bindings[1].special_floor_image->height;
    candidate.packed_span_fnv1a64 = fnv1a64(
        bindings[1].special_floor_image->packed_4bpp_texels, 128U);
    candidate.palette_fnv1a64 = fnv1a64(
        (const uint8_t *)bindings[1].special_floor_image->palette_bgr555, 32U);
    candidate.vdp1_state_fnv1a64 = fnv1a64((const uint8_t *)"state", 5U);
    memset(vdp1_command, 0, sizeof(vdp1_command));
    wl16(vdp1_command, 0U);       /* normal sprite */
    wl16(vdp1_command + 4, 0U);   /* 4bpp colour-bank mode */
    wl16(vdp1_command + 8, 0x1234U);
    wl16(vdp1_command + 10, 2U | (16U << 8));
    candidate.vdp1_command_fnv1a64 = fnv1a64(vdp1_command,
                                              sizeof(vdp1_command));
    candidate.texture_first_sequence = 10U;
    candidate.texture_last_sequence = 11U;
    candidate.vdp1_command_sequence = 12U;
    candidate.vdp1_texture_source_address = 0x25c00000U;
    candidate.vdp1_texture_source_bytes = 128U;
    candidate.vdp1_command_source_word = 0x1234U;
    CHECK(nexus_v1_item_ibs_bind_0008_vdp1_capture(
              bindings[1].special_floor_image, ibs, NEXUS_V1_ITEM_IBS_BYTES, 1,
              &candidate, bindings[1].special_floor_image->packed_4bpp_texels, 128,
              (const uint8_t *)bindings[1].special_floor_image->palette_bgr555, 32,
              (const uint8_t *)"state", 5, vdp1_command, sizeof(vdp1_command),
              &capture) == 0 && capture.original_vdp1_capture_verified &&
          capture.vdp1_command_format_matches && capture.decode_authorized &&
          !capture.fallback_visuals_permitted,
          "descriptor-0008 requires a complete matching VDP1 texture command");
    wl16(vdp1_command + 8, 0x1235U);
    candidate.vdp1_command_fnv1a64 = fnv1a64(vdp1_command,
                                              sizeof(vdp1_command));
    CHECK(nexus_v1_item_ibs_bind_0008_vdp1_capture(
              bindings[1].special_floor_image, ibs, NEXUS_V1_ITEM_IBS_BYTES, 1,
              &candidate, bindings[1].special_floor_image->packed_4bpp_texels, 128,
              (const uint8_t *)bindings[1].special_floor_image->palette_bgr555, 32,
              (const uint8_t *)"state", 5, vdp1_command, sizeof(vdp1_command),
              &capture) == 0 && capture.vdp1_command_matches &&
          !capture.vdp1_command_format_matches &&
          !capture.original_vdp1_capture_verified && !capture.decode_authorized,
          "a hash-matched VDP1 command with another texture source stays blocked");
    wl16(vdp1_command + 8, 0x1234U);
    candidate.vdp1_command_fnv1a64 = fnv1a64(vdp1_command,
                                              sizeof(vdp1_command));
    candidate.vdp1_command_fnv1a64 ^= UINT64_C(1);
    CHECK(nexus_v1_item_ibs_bind_0008_vdp1_capture(
              bindings[1].special_floor_image, ibs, NEXUS_V1_ITEM_IBS_BYTES, 1,
              &candidate, bindings[1].special_floor_image->packed_4bpp_texels, 128,
              (const uint8_t *)bindings[1].special_floor_image->palette_bgr555, 32,
              (const uint8_t *)"state", 5, vdp1_command, sizeof(vdp1_command),
              &capture) == 0 && !capture.vdp1_command_matches &&
          !capture.original_vdp1_capture_verified && !capture.decode_authorized,
          "a changed VDP1 command fingerprint rejects descriptor-0008 decoding");
    candidate.vdp1_command_fnv1a64 ^= UINT64_C(1);
    ibs[0] ^= 1U;
    CHECK(nexus_v1_item_ibs_bind_0008_vdp1_capture(
              bindings[1].special_floor_image, ibs, NEXUS_V1_ITEM_IBS_BYTES, 1,
              &candidate, bindings[1].special_floor_image->packed_4bpp_texels, 128,
              (const uint8_t *)bindings[1].special_floor_image->palette_bgr555, 32,
              (const uint8_t *)"state", 5, vdp1_command, sizeof(vdp1_command),
              &capture) == 0 && !capture.item_ibs_source_matches &&
          !capture.original_vdp1_capture_verified && !capture.decode_authorized,
          "a changed ITEM.IBS source byte rejects descriptor-0008 decoding");
    ibs[0] ^= 1U;
    CHECK(nexus_v1_item_ibs_bind_0008_vdp1_capture(
              bindings[1].special_floor_image, ibs, NEXUS_V1_ITEM_IBS_BYTES, 1,
              &candidate, bindings[1].special_floor_image->packed_4bpp_texels, 128,
              (const uint8_t *)bindings[1].special_floor_image->palette_bgr555, 32,
              (const uint8_t *)"state", 5, vdp1_command, sizeof(vdp1_command),
              &capture) == 0 && capture.decode_authorized,
          "only the unmodified capture binding may authorize descriptor-0008 decoding");
    CHECK(nexus_v1_item_ibs_decode_0008_vdp1_4bpp(
              bindings[1].special_floor_image, &capture, decoded,
              (int)sizeof(decoded), &codec_receipt) == 0 &&
          codec_receipt.source_hash_verified && codec_receipt.decode_authorized &&
          codec_receipt.decoded_texel_count == 256 && decoded[0] == 6U &&
          decoded[1] == 12U && !codec_receipt.fallback_visuals_permitted,
          "source-proven VDP1 mode expands descriptor-0008 high nibble first");
    CHECK(nexus_v1_dgn_consume_structure1f_item_floor_materials(
              bindings, 2, commands, 2, materials, 2, &material_receipt) == 0 &&
          material_receipt.complete &&
          material_receipt.special_floor_binding_count == 1 &&
          material_receipt.source_cell_match_count == 1 &&
          material_receipt.command_material_count == 1 &&
          material_receipt.blocked_missing_vdp1_command_provenance_count == 1 &&
          material_receipt.original_vdp1_capture_verified_count == 0 &&
          !material_receipt.fallback_visuals_permitted,
          "descriptor-0008 is consumed by its matching DGN floor command only");
    CHECK(materials[0].command_index == 1 && materials[0].source_entry_index == 1 &&
          materials[0].source_x == 6 && materials[0].source_y == 7 &&
          materials[0].item_id == 6U && materials[0].image_id == 266U &&
          materials[0].encoding == 8U && materials[0].width == 16U &&
          materials[0].height == 16U && materials[0].packed_4bpp_bytes == 128U &&
          materials[0].palette_bgr555[0] == 0x03e0U &&
          materials[0].packed_4bpp_texels[0] == 0x6cU &&
          materials[0].source_hash_verified && materials[0].packed_4bpp_valid &&
          materials[0].blocked_missing_vdp1_command_provenance &&
          !materials[0].original_vdp1_capture_verified &&
          !materials[0].texel_order_proven && !materials[0].draw_authorized,
          "command material keeps exact authenticated 4bpp bytes no-draw until VDP1 proof binds");
    bindings[1].palette_index = 1U;
    CHECK(nexus_v1_dgn_consume_structure1f_item_floor_materials(
              bindings, 2, commands, 2, materials, 2, &material_receipt) == 0 &&
          !material_receipt.complete &&
          material_receipt.source_cell_match_count == 1 &&
          material_receipt.blocked_invalid_binding_count == 1 &&
          material_receipt.command_material_count == 0 &&
          !material_receipt.fallback_visuals_permitted,
          "descriptor-0008 rejects a forged regular palette lane");
    bindings[1].palette_index = 0xffU;
    bindings[1].image_index = 7U;
    CHECK(nexus_v1_dgn_consume_structure1f_item_floor_materials(
              bindings, 2, commands, 2, materials, 2, &material_receipt) == 0 &&
          !material_receipt.complete &&
          material_receipt.source_cell_match_count == 1 &&
          material_receipt.blocked_invalid_binding_count == 1 &&
          material_receipt.command_material_count == 0 &&
          !material_receipt.fallback_visuals_permitted,
          "descriptor-0008 rejects a forged regular image lane");
    bindings[1].image_index = 0xffU;
    bindings[1].packed_4bpp_texels = bank.regular_image_texels[7];
    CHECK(nexus_v1_dgn_consume_structure1f_item_floor_materials(
              bindings, 2, commands, 2, materials, 2, &material_receipt) == 0 &&
          !material_receipt.complete &&
          material_receipt.source_cell_match_count == 1 &&
          material_receipt.blocked_invalid_binding_count == 1 &&
          material_receipt.command_material_count == 0 &&
          !material_receipt.fallback_visuals_permitted,
          "descriptor-0008 rejects a forged regular packed-texel lane");
    bindings[1].packed_4bpp_texels = NULL;
    commands[1].kind = NEXUS_V1_DGN_RENDER_COMMAND_CEILING;
    CHECK(nexus_v1_dgn_consume_structure1f_item_floor_materials(
              bindings, 2, commands, 2, materials, 2, &material_receipt) == 0 &&
          !material_receipt.complete &&
          material_receipt.blocked_invalid_command_count == 1 &&
          material_receipt.command_material_count == 0 &&
          !material_receipt.fallback_visuals_permitted,
          "descriptor-0008 cannot consume a non-floor DGN command");
    commands[1].kind = NEXUS_V1_DGN_RENDER_COMMAND_FLOOR;
    bindings[1].source_x = 4;
    CHECK(nexus_v1_dgn_consume_structure1f_item_floor_materials(
              bindings, 2, commands, 2, materials, 2, &material_receipt) == 0 &&
          !material_receipt.complete &&
          material_receipt.blocked_source_cell_mismatch_count == 1 &&
          material_receipt.command_material_count == 0 &&
          !material_receipt.fallback_visuals_permitted,
          "a descriptor-0008 source cell cannot be rebound to another DGN floor");
    bindings[1].source_x = 6;
    bindings[1].command_index = 2;
    CHECK(nexus_v1_dgn_consume_structure1f_item_floor_materials(
              bindings, 2, commands, 2, materials, 2, &material_receipt) == 0 &&
          !material_receipt.complete &&
          material_receipt.blocked_invalid_command_count == 1 &&
          material_receipt.command_material_count == 0 &&
          !material_receipt.fallback_visuals_permitted,
          "out-of-range DGN command cannot promote a descriptor-0008 surface");
    level.structure1f_entries[1].item_id = 250U;
    CHECK(nexus_v1_dgn_structure1f_item_ibs_coverage(&level, &bank, &coverage) == 0 &&
          !coverage.complete && coverage.blocked_invalid_item_count == 1 &&
          !coverage.fallback_visuals_permitted,
          "an unsupported Structure1Fa item reference remains blocked");
    free(ibs);
}

static void test_real_item_ibs_special_floor_corpus(void) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    char path[2048];
    uint8_t *data;
    FILE *file;
    Nexus_V1_ItemIbsBank bank;
    Nexus_V1_DgnStructure1FItemMaterialBinding binding;
    Nexus_V1_DgnRenderCommand command;
    Nexus_V1_DgnCommandPacked4BppMaterial material;
    Nexus_V1_DgnCommandPacked4BppMaterialReceipt material_receipt;
    Nexus_V1_ItemIbs0008CodecReceipt codec_receipt;
    uint8_t decoded[NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_MAX_TEXELS];
    int level_index;
    int checked_levels = 0;
    int total_dgn_items = 0;
    int total_special_floor_references = 0;
    int total_blocked_references = 0;

    if (!data_dir || !data_dir[0]) return;
    snprintf(path, sizeof(path), "%s/ITEM.IBS", data_dir);
    file = fopen(path, "rb");
    if (!file) return;
    data = (uint8_t *)malloc(NEXUS_V1_ITEM_IBS_BYTES);
    CHECK(data != NULL, "real ITEM.IBS corpus buffer allocates");
    if (!data) {
        fclose(file);
        return;
    }
    CHECK(fread(data, 1, NEXUS_V1_ITEM_IBS_BYTES, file) ==
              NEXUS_V1_ITEM_IBS_BYTES,
          "real ITEM.IBS corpus reads completely");
    fclose(file);
    CHECK(nexus_v1_item_ibs_parse_verified(data, NEXUS_V1_ITEM_IBS_BYTES,
                                             1, &bank) == 0,
          "real ITEM.IBS proves the special-floor packed-4bpp layout");
    CHECK(bank.floor_image_count == NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_COUNT &&
          bank.floor_images[43].image_id == 266U &&
          bank.floor_images[43].packed_4bpp_valid &&
          bank.floor_images[43].packed_4bpp_bytes ==
              (uint32_t)bank.floor_images[43].width *
              (uint32_t)bank.floor_images[43].height / 2U,
          "real ITEM.IBS combines regular and floor-image indices without a fallback");
    memset(&binding, 0, sizeof(binding));
    memset(&command, 0, sizeof(command));
    binding.command_index = 0;
    binding.special_floor_image = &bank.floor_images[43];
    command.kind = NEXUS_V1_DGN_RENDER_COMMAND_FLOOR;
    CHECK(nexus_v1_dgn_consume_structure1f_item_floor_materials(
              &binding, 1, &command, 1, &material, 1, &material_receipt) == 0 &&
          material_receipt.complete && material.source_hash_verified &&
          material_receipt.blocked_missing_vdp1_command_provenance_count == 1 &&
          material_receipt.original_vdp1_capture_verified_count == 0 &&
          material.image_id == bank.floor_images[43].image_id &&
          material.width == bank.floor_images[43].width &&
          material.height == bank.floor_images[43].height &&
          material.packed_4bpp_texels == bank.floor_images[43].packed_4bpp_texels &&
          material.palette_bgr555 == bank.floor_images[43].palette_bgr555 &&
          material.blocked_missing_vdp1_command_provenance &&
          !material.original_vdp1_capture_verified &&
          !material.draw_authorized,
          "real Saturn descriptor-0008 payload reaches a DGN command no-draw");
    CHECK(nexus_v1_item_ibs_decode_0008_vdp1_4bpp(
              &bank.floor_images[43], NULL, decoded, (int)sizeof(decoded),
              &codec_receipt) == 0 &&
          codec_receipt.descriptor_0008_verified &&
          codec_receipt.packed_span_verified && codec_receipt.palette_bound &&
          codec_receipt.blocked_missing_vdp1_command_provenance &&
          !codec_receipt.decode_authorized &&
          !codec_receipt.fallback_visuals_permitted,
          "real Saturn ITEM.IBS stays blocked without an original VDP1 command receipt");
    for (level_index = 0; level_index <= 15; ++level_index) {
        Nexus_V1_Level level;
        Nexus_V1_DgnStructure1FItemIbsCoverageReceipt coverage;
        uint8_t *level_data;
        FILE *level_file;
        long level_size;

        snprintf(path, sizeof(path), "%s/LEV%02d.DGN", data_dir, level_index);
        level_file = fopen(path, "rb");
        CHECK(level_file != NULL, "real ITEM.IBS coverage DGN opens");
        if (!level_file) continue;
        CHECK(real_dgn_is_hash_verified(path, level_index),
              "real ITEM.IBS coverage DGN matches its canonical MD5");
        if (!real_dgn_is_hash_verified(path, level_index)) {
            fclose(level_file);
            continue;
        }
        CHECK(fseek(level_file, 0, SEEK_END) == 0,
              "real ITEM.IBS coverage DGN seeks");
        level_size = ftell(level_file);
        CHECK(level_size > 0 && fseek(level_file, 0, SEEK_SET) == 0,
              "real ITEM.IBS coverage DGN has data");
        if (level_size <= 0 || fseek(level_file, 0, SEEK_SET) != 0) {
            fclose(level_file);
            continue;
        }
        level_data = (uint8_t *)malloc((size_t)level_size);
        CHECK(level_data != NULL, "real ITEM.IBS coverage DGN allocates");
        if (!level_data) {
            fclose(level_file);
            continue;
        }
        CHECK(fread(level_data, 1, (size_t)level_size, level_file) ==
                  (size_t)level_size,
              "real ITEM.IBS coverage DGN reads");
        fclose(level_file);
        CHECK(nexus_v1_level_load(&level, level_data, (int)level_size,
                                  level_index) == 0 &&
                  nexus_v1_dgn_structure1f_item_ibs_coverage(
                      &level, &bank, &coverage) == 0 &&
                  !coverage.fallback_visuals_permitted,
              "retail DGN Structure1Fa coverage refuses unproved ITEM.IBS routes");
        if (nexus_v1_level_load(&level, level_data, (int)level_size,
                                level_index) == 0 &&
            nexus_v1_dgn_structure1f_item_ibs_coverage(
                &level, &bank, &coverage) == 0) {
            total_dgn_items += coverage.dgn_item_entry_count;
            total_special_floor_references += coverage.special_floor_reference_count;
            total_blocked_references += coverage.blocked_invalid_item_count +
                coverage.blocked_missing_floor_image_count +
                coverage.blocked_unsupported_encoding_count;
            ++checked_levels;
        }
        free(level_data);
    }
    CHECK(checked_levels == 16 && total_dgn_items > 0 &&
          total_special_floor_references > 0 && total_blocked_references == 0,
          "retail DGN direct items reach verified descriptor-0008 sources without fallback");
    free(data);
}

static void test_vdp1_command_sidecar_stays_no_draw(void) {
    Nexus_V1_Prs3Vdp1RawSidecarReceipt raw;
    Nexus_V1_Prs3Vdp1CommandSidecarReceipt inspection;
    uint8_t command[NEXUS_V1_VDP1_COMMAND_BYTES] = {0};

    /* Unit framing only: production admission still requires canonical asset
     * hashes and an independently authenticated original-Saturn producer. */
    wl16(command + 2, 0xbeefU);  /* CMDLINK: retained, not followed here */
    wl16(command + 6, 0x4567U);  /* CMDCOLR: retained, not interpreted */
    command[8] = 0x10U;
    command[10] = 0x01U;
    command[11] = 0x01U;
    memset(&raw, 0, sizeof(raw));
    raw.raw_sidecars_bound = 1;
    raw.vdp1_command_sidecar_bound = 1;
    raw.trace_file.source_bound_capture = 1;
    raw.trace_file.v3_trace_parsed = 1;
    raw.trace_file.trace.valid = 1;
    raw.trace_file.trace.schema_version = 3U;
    raw.trace_file.trace.vdp1_command_consumption_observed = 1;
    raw.trace_file.trace.vdp1_command_read_bytes = sizeof(command);
    raw.trace_file.trace.vdp1_command_fnv1a64 =
        fnv1a64(command, sizeof(command));
    CHECK(nexus_v1_prs3_vdp1_capture_inspect_command_sidecar(
              &raw, command, sizeof(command), &inspection) &&
          inspection.valid && inspection.capture_source_bound &&
          inspection.command_sidecar_hash_bound &&
          inspection.complete_vdp1_command_record &&
          inspection.command_format_parsed && inspection.command.texture_command &&
          inspection.command.link_word == 0xbeefU &&
          inspection.command.colour_control == 0x4567U &&
          inspection.command.texture_source_word == 0x0010U &&
          inspection.command.texture_bits_per_pixel == 4U &&
          inspection.command.texture_byte_count == 4U &&
          inspection.command.texture_source_byte_offset == 0x80U &&
          inspection.command.texture_source_byte_end == 0x84U &&
          inspection.command.texture_source_range_valid &&
          inspection.command.texture_width == 8U &&
          inspection.command.texture_height == 1U &&
          !inspection.original_saturn_capture_verified &&
          !inspection.pixel_format_proven && !inspection.palette_format_proven &&
          !inspection.decoder_promoted && !inspection.runtime_import_permitted &&
          !inspection.fallback_visuals_permitted,
          "source-bound VDP1 command framing stays no-draw without independent Saturn provenance");
    command[0] = 1U;
    CHECK(!nexus_v1_prs3_vdp1_capture_inspect_command_sidecar(
               &raw, command, sizeof(command), &inspection) &&
          !inspection.valid && !inspection.command_sidecar_hash_bound &&
          !inspection.decoder_promoted && !inspection.runtime_import_permitted,
          "a changed VDP1 sidecar cannot reuse capture admission");
}

static void test_palette_source_gate(void) {
    Nexus_PaletteState palette;
    uint8_t source[NEXUS_PALETTE_SIZE * 2];
    int i;

    for (i = 0; i < (int)sizeof(source); ++i)
        source[i] = (uint8_t)(i + 1);
    nexus_palette_init_defaults(&palette);
    CHECK(!palette.source_palette_bound && nexus_palette_lookup(&palette, 7U) == 0U,
          "an absent Nexus source palette cannot create fallback pixels");
    CHECK(nexus_palette_load_stone(&palette, source, sizeof(source)) ==
              NEXUS_PALETTE_SIZE &&
          palette.source_palette_bound &&
          palette.entries[0] == 0x0201U,
          "a complete source palette is admitted without generated entries");
    CHECK(nexus_palette_load_stone(&palette, source, 2) == 0 &&
          !palette.source_palette_bound && nexus_palette_lookup(&palette, 7U) == 0U,
          "a truncated palette source withdraws the renderable palette");
}

static void test_owner_material_capture_target_blocks_without_canonical_lev(void) {
    Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget target;
    Nexus_V1_DgnStructure1AStructure3CaptureTargetRouteReceipt route;
    Nexus_V1_DgnOwnerMaterialTraceAdmissionReceipt trace;
    static const uint8_t raw_trace[] = { 0x53U, 0x32U };

    memset(&target, 0xff, sizeof(target));
    memset(&route, 0xff, sizeof(route));
    CHECK(nexus_v1_engine_build_structure1a_structure3_material_capture_target(
              NULL, 0, 0U, 0U, &target, &route) == 0 &&
          !target.valid && target.level_index == -1 &&
          target.no_draw_only && target.blocks_real_dgn_mesh_render &&
          !target.fallback_visuals_permitted && !route.target_built,
          "an owner/material bundle blocks without one canonical LEV source");
    memset(&trace, 0xff, sizeof(trace));
    CHECK(nexus_v1_engine_admit_structure1a_structure3_material_capture_trace(
              NULL, 0, 0U, 0U, "capture_target_fnv1a64=0\n",
              sizeof("capture_target_fnv1a64=0\n") - 1U,
              raw_trace, sizeof(raw_trace), 1, &trace) == 0 &&
          trace.status == NEXUS_V1_OWNER_MATERIAL_TRACE_BLOCKED_BUNDLE &&
          !trace.atomic_target_bound && !trace.owner_face_bound &&
          !trace.structure2_trace_admitted && !trace.opaque_trace_admitted &&
          !trace.decoder_permitted && trace.no_draw_only &&
          !trace.fallback_visuals_permitted &&
          trace.blocks_real_dgn_mesh_render,
          "an external trace cannot bypass an absent owner/material capture target");
}

static void test_menu_bpk_missing_handoff_blocks_fallback(void) {
    Nexus_V1_Engine engine;
    Nexus_V1_MenuBpkRendererHandoffReceipt handoff;
    Nexus_V1_LauncherRuntimeReceipt runtime;
    Nexus_V1_StartupAssetHandoffReceipt asset_handoff;

    memset(&engine, 0, sizeof(engine));
    memset(&handoff, 0, sizeof(handoff));
    CHECK(nexus_v1_menu_bpk_renderer_handoff_receipt(&engine, &handoff) == 0 &&
          handoff.status == NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_MISSING &&
          handoff.prs3_prerequisite_status ==
              NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_ARCHIVE_MISSING &&
          strcmp(nexus_v1_menu_bpk_prs3_prerequisite_status_name(
                     handoff.prs3_prerequisite_status), "archive-missing") == 0 &&
          !handoff.receipt_valid && !handoff.can_render_stored_surfaces &&
          handoff.blocks_real_menu_surface_render &&
          !handoff.fallback_visuals_permitted,
          "missing MENU.BPK stays fail-closed without replacement visuals");
    memset(&runtime, 0, sizeof(runtime));
    memset(&asset_handoff, 0, sizeof(asset_handoff));
    runtime.engine = &engine;
    runtime.level_loaded = 1;
    runtime.title_loaded = 1;
    runtime.startup_assets.title_route_ready = 1;
    runtime.startup_assets.real_menu_surface_route_ready = 1;
    runtime.startup_assets.startup_audio_handoff_ready = 1;
    runtime.startup_assets.startup_menu_asset_route = "ready-real-menu-surfaces";
    CHECK(nexus_v1_launcher_startup_asset_handoff_from_runtime_receipt(
              &runtime, &asset_handoff) &&
          asset_handoff.route == NEXUS_V1_STARTUP_ASSET_HANDOFF_MENU_BLOCKED &&
          !asset_handoff.menu_bpk_renderer_handoff_valid &&
          !asset_handoff.real_menu_asset_handoff_ready &&
          !asset_handoff.main_menu_route_ready &&
          asset_handoff.blocks_main_menu_route &&
          !asset_handoff.fallback_visuals_permitted,
          "missing MENU.BPK cannot promote launcher main-menu readiness");
}

static void test_menu_bpk_palette_trailer_stays_opaque(void) {
    uint8_t archive[572];
    Nexus_V1_BpkPaletteTrailerReceipt palette;
    Nexus_V1_BpkRuntimeUploadReceipt upload;
    Nexus_V1_BpkEntry last_entry;
    Nexus_V1_Engine engine;
    Nexus_V1_MenuBpkPaltCaptureTargetReceipt target;
    Nexus_V1_MenuBpkPaltTraceAdmissionReceipt trace_admission;
    char target_path[128];
    char target_text[2048];
    char trace_manifest[2048];
    const uint8_t raw_trace[] = "mednafen palt memory observation";
    const uint8_t palette_state[] = { 0x10U, 0x32U, 0x54U, 0x76U };
    const uint8_t vdp1_command[] = { 0x80U, 0x00U, 0x01U, 0x00U };
    FILE *target_file;
    size_t index;
    const char *data_dir;

    memset(archive, 0, sizeof(archive));
    wb32(archive + 0, NEXUS_V1_BPK_MAGIC_BPPK);
    wb32(archive + 4, (uint32_t)sizeof(archive));
    wb32(archive + 12, NEXUS_V1_BPK_MAGIC_BMPD);
    wb32(archive + 16, (uint32_t)sizeof(archive) - 12U);
    wb32(archive + 20, 1U);
    wb32(archive + 24, 28U);
    wb32(archive + 48, NEXUS_V1_BPK_MAGIC_PALT);
    wb32(archive + 52, 524U);
    wb32(archive + 56, 256U);
    for (index = 0; index < 512U; ++index) archive[60U + index] = (uint8_t)index;

    CHECK(nexus_v1_bpk_archive_get_entry(
              archive, sizeof(archive), 0U, &last_entry) == 0 &&
          last_entry.offset == 28U && last_entry.next_offset == 48U &&
          last_entry.stored_size == 20U,
          "bounded MENU.BPK PALT tail is not part of final PRS3 entry span");

    CHECK(nexus_v1_bpk_archive_inspect_palette_trailer(
              archive, sizeof(archive), &palette) == 0 && palette.valid &&
          palette.record_offset == 48U && palette.record_bytes == 524U &&
          palette.entry_count == 256U && palette.entry_bytes == 512U &&
          palette.entry_bytes_fnv1a64 == fnv1a64(archive + 60U, 512U) &&
          palette.raw_entries_are_be16 && !palette.palette_format_proven &&
          !palette.decoder_promoted && !palette.fallback_visuals_permitted,
          "bounded MENU.BPK PALT trailer stays opaque without palette promotion");
    CHECK(nexus_v1_bpk_archive_runtime_upload_plan(
              archive, sizeof(archive), NULL, 0U, &upload) == 0 &&
          upload.palette_trailer_observed && upload.palette_trailer.valid &&
          upload.route == NEXUS_V1_BPK_UPLOAD_ROUTE_NO_SURFACES &&
          !upload.fallback_visuals_permitted,
          "BPK upload receipt carries PALT framing without creating a surface");

    memset(&engine, 0, sizeof(engine));
    engine.menu_bpk_source.canonical_hash_verified = 1;
    snprintf(engine.menu_bpk_source.canonical_name,
             sizeof(engine.menu_bpk_source.canonical_name), "MENU.BPK");
    snprintf(engine.menu_bpk_source.canonical_md5,
             sizeof(engine.menu_bpk_source.canonical_md5),
             "c2776768ff25287c79013a1452253ca0");
    engine.menu_bpk_upload_receipt_valid = 1;
    engine.menu_bpk_upload_receipt.palette_trailer_observed = 1;
    engine.menu_bpk_upload_receipt.palette_trailer = palette;
    CHECK(nexus_v1_engine_build_menu_bpk_palt_capture_target(
              &engine, &target) == 1 && target.valid &&
          target.palt_record_offset == 48U && target.palt_record_bytes == 524U &&
          target.palt_entry_bytes_fnv1a64 == fnv1a64(archive + 60U, 512U) &&
          target.original_saturn_capture_required &&
          target.palt_memory_read_observation_required &&
          target.palette_state_observation_required &&
          target.vdp1_command_observation_required &&
          !target.palt_palette_relation_proven && !target.decoder_promoted &&
          target.no_draw_only && !target.fallback_visuals_permitted,
          "canonical PALT handoff emits only an original-Saturn correlation target");
    snprintf(target_path, sizeof(target_path),
             "/tmp/firestaff-nexus-palt-target-%ld.txt", (long)getpid());
    CHECK(nexus_v1_engine_write_menu_bpk_palt_capture_target(
              &engine, target_path, &target) == 1 && target.valid,
          "canonical PALT capture target writes atomically");
    target_file = fopen(target_path, "rb");
    if (target_file) {
        size_t target_size = fread(target_text, 1U,
                                   sizeof(target_text) - 1U, target_file);
        target_text[target_size] = '\0';
        fclose(target_file);
        CHECK(strstr(target_text,
                     "FIRESTAFF_NEXUS_MENU_BPK_PALT_CAPTURE_TARGET_V1\n") != NULL &&
              strstr(target_text,
                     "required_observations=palt_memory_read,palette_state,vdp1_command\n") != NULL &&
              strstr(target_text, "decoder_promoted=0\n") != NULL,
              "PALT target requests trace evidence without a decoder claim");
    } else {
        CHECK(0, "PALT capture target can be read back");
    }
    remove(target_path);
    snprintf(trace_manifest, sizeof(trace_manifest),
             "magic=%s\nproducer=mednafen-debugger\n"
             "trace_sha256=0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n"
             "canonical_menu_bpk_md5=%s\npalt_record_offset=%x\n"
             "palt_record_bytes=%x\npalt_entry_count=%x\npalt_entry_bytes=%x\n"
             "palt_entry_bytes_fnv1a64=%016llx\nraw_trace_fnv1a64=%016llx\n"
             "palt_memory_fnv1a64=%016llx\npalette_state_fnv1a64=%016llx\n"
             "vdp1_command_fnv1a64=%016llx\n",
             NEXUS_V1_MENU_BPK_PALT_TRACE_MAGIC,
             target.canonical_menu_bpk_md5, target.palt_record_offset,
             target.palt_record_bytes, target.palt_entry_count,
             target.palt_entry_bytes,
             (unsigned long long)target.palt_entry_bytes_fnv1a64,
             (unsigned long long)fnv1a64(raw_trace, sizeof(raw_trace) - 1U),
             (unsigned long long)fnv1a64(archive + 60U, 512U),
             (unsigned long long)fnv1a64(palette_state, sizeof(palette_state)),
             (unsigned long long)fnv1a64(vdp1_command, sizeof(vdp1_command)));
    CHECK(nexus_v1_engine_admit_menu_bpk_palt_trace(
              &engine, trace_manifest, strlen(trace_manifest), raw_trace,
              sizeof(raw_trace) - 1U, archive + 60U, 512U, palette_state,
              sizeof(palette_state), vdp1_command, sizeof(vdp1_command), 0,
              &trace_admission) == 0 &&
          trace_admission.status == NEXUS_V1_MENU_BPK_PALT_TRACE_BLOCKED_PROVENANCE &&
          trace_admission.capture_target_bound &&
          trace_admission.manifest_target_bound &&
          trace_admission.raw_trace_bytes_bound &&
          trace_admission.palt_memory_bytes_bound &&
          trace_admission.palette_state_bytes_bound &&
          trace_admission.vdp1_command_bytes_bound &&
          !trace_admission.decoder_promoted && trace_admission.no_draw_only,
          "PALT observations without independent Saturn provenance stay blocked");
    CHECK(nexus_v1_engine_admit_menu_bpk_palt_trace(
              &engine, trace_manifest, strlen(trace_manifest), raw_trace,
              sizeof(raw_trace) - 1U, archive + 60U, 512U, palette_state,
              sizeof(palette_state), vdp1_command, sizeof(vdp1_command), 1,
              &trace_admission) == 1 &&
          trace_admission.status == NEXUS_V1_MENU_BPK_PALT_TRACE_ADMITTED_OPAQUE &&
          trace_admission.original_saturn_capture_verified &&
          trace_admission.opaque_trace_admitted &&
          trace_admission.palt_memory_fnv1a64 ==
              target.palt_entry_bytes_fnv1a64 &&
          !trace_admission.palt_palette_relation_proven &&
          !trace_admission.decoder_promoted && trace_admission.no_draw_only &&
          !trace_admission.fallback_visuals_permitted &&
          engine.menu_bpk_palt_trace_admission.status ==
              NEXUS_V1_MENU_BPK_PALT_TRACE_ADMITTED_OPAQUE,
          "authentic PALT observations reach the engine only as an opaque no-draw receipt");
    engine.menu_bpk_source.canonical_hash_verified = 0;
    CHECK(nexus_v1_engine_build_menu_bpk_palt_capture_target(
              &engine, &target) == 0 && !target.valid,
          "unverified MENU.BPK cannot emit a PALT capture target");
    wb32(archive + 52, 522U);
    CHECK(nexus_v1_bpk_archive_inspect_palette_trailer(
              archive, sizeof(archive), &palette) != 0 && !palette.valid &&
          !palette.fallback_visuals_permitted,
          "malformed MENU.BPK PALT trailer stays blocked");

    data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    if (data_dir && data_dir[0]) {
        char path[2048];
        FILE *file;
        long size;
        uint8_t *real_archive;
        Nexus_V1_BpkArchiveInfo real_info;
        Nexus_V1_BpkEntry real_last_entry;

        snprintf(path, sizeof(path), "%s/MENU.BPK", data_dir);
        file = fopen(path, "rb");
        CHECK(file != NULL, "real MENU.BPK PALT source opens");
        if (!file) return;
        CHECK(asset_file_matches_md5(path, "c2776768ff25287c79013a1452253ca0"),
              "real MENU.BPK PALT source matches canonical MD5");
        if (!asset_file_matches_md5(path, "c2776768ff25287c79013a1452253ca0") ||
            fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
            fseek(file, 0, SEEK_SET) != 0 ||
            !(real_archive = (uint8_t *)malloc((size_t)size))) {
            fclose(file);
            return;
        }
        CHECK(fread(real_archive, 1, (size_t)size, file) == (size_t)size,
              "real MENU.BPK PALT source reads completely");
        fclose(file);
        if (nexus_v1_bpk_archive_inspect_palette_trailer(
                real_archive, (size_t)size, &palette) == 0) {
            CHECK(palette.valid && palette.record_bytes == 524U &&
                  palette.entry_count == 256U && palette.entry_bytes == 512U &&
                  palette.entry_bytes_fnv1a64 != 0U &&
                  palette.raw_entries_are_be16 && !palette.palette_format_proven &&
                  !palette.decoder_promoted && !palette.fallback_visuals_permitted,
                  "canonical MENU.BPK PALT trailer remains opaque source data");
            CHECK(nexus_v1_bpk_archive_parse(
                      real_archive, (size_t)size, &real_info) == 0 &&
                  real_info.candidate_offset_count > 0U &&
                  nexus_v1_bpk_archive_get_entry(
                      real_archive, (size_t)size,
                      real_info.candidate_offset_count - 1U,
                      &real_last_entry) == 0 &&
                  real_last_entry.next_offset == palette.record_offset,
                  "canonical MENU.BPK PALT tail is excluded from final entry span");
            CHECK(nexus_v1_bpk_archive_runtime_upload_plan(
                      real_archive, (size_t)size, NULL, 0U, &upload) == 0 &&
                  upload.palette_trailer_observed && upload.palette_trailer.valid &&
                  upload.route == NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3 &&
                  upload.blocked_prs3_uploads > 0U &&
                  !upload.fallback_visuals_permitted,
                  "canonical BPK host receipt preserves PALT while PRS3 stays blocked");
        } else {
            CHECK(0, "canonical MENU.BPK PALT trailer keeps its bounded layout");
        }
        free(real_archive);
    }
}

static void test_menu_bpk_handoff_requires_canonical_source(void) {
    Nexus_V1_Engine engine;
    Nexus_V1_MenuBpkRendererHandoffReceipt handoff;
    Nexus_V1_LauncherRuntimeReceipt runtime;
    Nexus_V1_StartupAssetHandoffReceipt asset_handoff;

    memset(&engine, 0, sizeof(engine));
    engine.menu_bpk_decode_receipt_valid = 1;
    engine.menu_bpk_decode_receipt_attempted = 1;
    engine.menu_bpk_decode_receipt.route =
        NEXUS_V1_BPK_DECODE_ROUTE_READY_STORED;
    engine.menu_bpk_decode_receipt.ready_stored_surfaces = 1U;
    engine.menu_bpk_upload_receipt_valid = 1;
    engine.menu_bpk_upload_receipt.palette_trailer_observed = 1;
    engine.menu_bpk_upload_receipt.palette_trailer.valid = 1;
    engine.menu_bpk_upload_receipt.palette_trailer.record_bytes = 524U;
    engine.menu_bpk_upload_receipt.palette_trailer.entry_count = 256U;
    engine.menu_bpk_upload_receipt.palette_trailer.entry_bytes = 512U;
    engine.menu_bpk_upload_receipt.palette_trailer.entry_bytes_fnv1a64 = 1U;
    engine.menu_bpk_upload_receipt.palette_trailer.raw_entries_are_be16 = 1;
    engine.menu_bpk_source.exact_source_entry_observed = 1;
    engine.menu_bpk_source.hash_discovery_attempted = 1;
    memset(&handoff, 0, sizeof(handoff));
    CHECK(nexus_v1_menu_bpk_renderer_handoff_receipt(&engine, &handoff) == 0 &&
          handoff.status == NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_SOURCE &&
          handoff.prs3_prerequisite_status ==
              NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_SOURCE_UNVERIFIED &&
          !handoff.canonical_source_hash_verified && !handoff.receipt_valid &&
          !handoff.canonical_palette_trailer_bound &&
          !handoff.can_render_stored_surfaces &&
          handoff.blocks_real_menu_surface_render &&
          !handoff.fallback_visuals_permitted,
          "a parseable non-canonical MENU.BPK cannot enter the renderer");

    memset(&runtime, 0, sizeof(runtime));
    memset(&asset_handoff, 0, sizeof(asset_handoff));
    runtime.engine = &engine;
    runtime.level_loaded = 1;
    runtime.title_loaded = 1;
    runtime.startup_assets.title_route_ready = 1;
    runtime.startup_assets.real_menu_surface_route_ready = 1;
    runtime.startup_assets.startup_audio_handoff_ready = 1;
    runtime.startup_assets.startup_menu_asset_route = "ready-real-menu-surfaces";
    CHECK(nexus_v1_launcher_startup_asset_handoff_from_runtime_receipt(
              &runtime, &asset_handoff) &&
          asset_handoff.route == NEXUS_V1_STARTUP_ASSET_HANDOFF_MENU_BLOCKED &&
          strcmp(asset_handoff.status, "blocked-menu-bpk-source") == 0 &&
          !asset_handoff.main_menu_route_ready &&
          !asset_handoff.fallback_visuals_permitted,
          "the launcher names a non-canonical MENU.BPK source block");

    engine.menu_bpk_source.canonical_hash_verified = 1;
    CHECK(nexus_v1_menu_bpk_renderer_handoff_receipt(&engine, &handoff) == 0 &&
          handoff.status == NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_STORED &&
          handoff.prs3_prerequisite_status ==
              NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_READY_STORED &&
          handoff.canonical_source_hash_verified && handoff.receipt_valid &&
          handoff.canonical_palette_trailer_bound &&
          handoff.palette_trailer.record_bytes == 524U &&
          handoff.palette_trailer.entry_count == 256U &&
          !handoff.palette_trailer.palette_format_proven &&
          handoff.can_render_stored_surfaces &&
          !handoff.blocks_real_menu_surface_render &&
          !handoff.fallback_visuals_permitted &&
          nexus_v1_menu_bpk_decode_receipt_ready(&engine),
          "only an authenticated source can expose an otherwise-ready BPK receipt");
    engine.menu_bpk_decode_receipt.route =
        NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_TRUNCATED;
    CHECK(nexus_v1_menu_bpk_renderer_handoff_receipt(&engine, &handoff) == 0 &&
          handoff.status ==
              NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_TRUNCATED &&
          handoff.prs3_prerequisite_status ==
              NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_FRAME_INCOMPLETE &&
          strcmp(nexus_v1_menu_bpk_prs3_prerequisite_status_name(
                     handoff.prs3_prerequisite_status), "frame-incomplete") == 0 &&
          handoff.blocks_real_menu_surface_render &&
          !handoff.fallback_visuals_permitted,
          "a truncated MENU.BPK frame reports framing evidence, not decoder absence");
    engine.menu_bpk_decode_receipt.route =
        NEXUS_V1_BPK_DECODE_ROUTE_READY_DECODED;
    CHECK(nexus_v1_menu_bpk_renderer_handoff_receipt(&engine, &handoff) == 0 &&
          handoff.status == NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_DECODED &&
          handoff.prs3_prerequisite_status ==
              NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_READY_STORED &&
          handoff.can_render_stored_surfaces &&
          !handoff.blocks_real_menu_surface_render &&
          !handoff.fallback_visuals_permitted,
          "a decoded PRS3 route renders MENU.BPK surfaces");
}

static void test_slev_capture_target_binds_loaded_bytes(void) {
    static const uint8_t spine[36] = {
        0x2f, 0xe6, 0x00, 0x00, 0x00, 0x00, 0x34, 0x23,
        0x4f, 0x22, 0x7f, 0xfc, 0x2f, 0x52, 0x8d, 0x02,
        0x23, 0x42, 0x44, 0x11, 0x89, 0x04, 0xe0, 0xff,
        0x7f, 0x04, 0x4f, 0x26, 0x00, 0x0b, 0x6e, 0xf6,
        0x00, 0x00, 0x00, 0x00
    };
    uint8_t slev[64];
    Nexus_V1_Engine engine;
    Nexus_V1_LevelScriptCaptureTargetReceipt target;
    Nexus_V1_LevelScriptTraceAdmissionReceipt admission;
    Nexus_V1_LevelScriptTraceHostReceipt host;
    Nexus_V1_SlevDispatchEvidenceReceipt evidence;
    static const uint8_t raw_trace[] =
        "pc=00001000 opcode=2fe6\n"
        "pc=00001002 opcode=1234\n"
        "pc=00001004 kind=write\n"
        "literal=00200010\n"
        "literal=00200020\n";
    char trace[2048];

    memset(slev, 0, sizeof(slev));
    memcpy(slev, spine, sizeof(spine));
    slev[2] = 0xe0U;
    slev[3] = 1U;
    slev[4] = 0xd0U;
    slev[5] = 8U;
    slev[32] = 0xd0U;
    slev[33] = 3U;
    wb32(slev + 40, 0x00200010U);
    wb32(slev + 48, 0x00200020U);

    memset(&engine, 0, sizeof(engine));
    nexus_script_vm_init(&engine.script_vm);
    engine.level_loaded = 1;
    engine.level_aux_runtime_receipt.level_index = 0;
    engine.level_aux_runtime_receipt.slev.canonical_hash_verified = 1;
    snprintf(engine.level_aux_runtime_receipt.slev.canonical_name,
             sizeof(engine.level_aux_runtime_receipt.slev.canonical_name),
             "SLEV00.BIN");
    snprintf(engine.level_aux_runtime_receipt.slev.canonical_md5,
             sizeof(engine.level_aux_runtime_receipt.slev.canonical_md5),
             "59c01cbdd224152a6176687cdebeea9e");
    CHECK(nexus_script_vm_load_canonical_level(&engine.script_vm, 0, slev,
                                               (int)sizeof(slev), 1) == 0 &&
          nexus_v1_engine_build_slev_capture_target(&engine, &target) == 1 &&
          target.valid && target.source_fnv1a64 == fnv1a64(slev, sizeof(slev)),
          "SLEV capture target binds the exact loaded task bytes");

    snprintf(trace, sizeof(trace),
             "magic=%s\nproducer=mednafen-debugger\n"
             "trace_sha256=0000000000000000000000000000000000000000000000000000000000000000\n"
             "level_index=%x\ncanonical_slev_name=%s\ncanonical_slev_md5=%s\n"
             "source_byte_count=%x\nsource_fnv1a64=%016llx\n"
             "entry_opcode=%x\ntask_header_size=%x\n"
             "primary_literal_address=%x\nauxiliary_literal_address=%x\n"
             "entry_pc=1000\ntask_body_pc=1002\ntask_body_opcode=1234\n"
             "callback_or_write_pc=1004\ncallback_or_write_kind=write\n"
             "raw_trace_fnv1a64=%016llx\n"
             "original_saturn_execution=1\n",
             NEXUS_V1_SLEV_TRACE_MAGIC, target.level_index,
             target.canonical_slev_name, target.canonical_slev_md5,
             target.source_byte_count, (unsigned long long)target.source_fnv1a64,
             target.first_opcode, target.task_header_size,
             target.primary_literal_address, target.auxiliary_literal_address,
             (unsigned long long)fnv1a64(raw_trace, sizeof(raw_trace) - 1));
    CHECK(nexus_v1_engine_admit_slev_execution_trace_with_raw(
              &engine, trace, strlen(trace), raw_trace, sizeof(raw_trace) - 1,
              &admission) == 1 &&
          admission.capture_target_bound &&
          admission.source_fnv1a64 == target.source_fnv1a64 &&
          admission.raw_trace_bytes_bound &&
          !admission.dispatch_permitted && admission.blocks_real_script_dispatch,
          "SLEV trace admission retains loaded-byte identity without dispatch");
    CHECK(nexus_v1_engine_consume_slev_execution_trace(&engine, &host) == 0 &&
          host.status == NEXUS_V1_SLEV_TRACE_HOST_BLOCKED_TRACE &&
          !host.dispatch_permitted,
          "SLEV host blocks before raw observations are verified");
    CHECK(nexus_v1_build_slev_dispatch_evidence(
              &engine, raw_trace, sizeof(raw_trace) - 1, &evidence) == 1 &&
          evidence.status == NEXUS_V1_SLEV_DISPATCH_EVIDENCE_OBSERVED &&
          evidence.raw_trace_bound && evidence.observation_order_proven &&
          evidence.literal_observation_proven && !evidence.dispatch_permitted,
          "SLEV raw trace binds the required observations without dispatch");
    CHECK(nexus_v1_engine_consume_slev_execution_trace(&engine, &host) == 1 &&
          host.status == NEXUS_V1_SLEV_TRACE_HOST_CONSUMED_OPAQUE &&
          host.host_consumed && !host.dispatch_permitted,
          "SLEV host consumes verified observations without task execution");
    engine.script_trace_admission.raw_trace_fnv1a64 ^= UINT64_C(1);
    CHECK(nexus_v1_engine_consume_slev_execution_trace(&engine, &host) == 0 &&
          host.status == NEXUS_V1_SLEV_TRACE_HOST_BLOCKED_TRACE &&
          !host.dispatch_permitted,
          "a changed SLEV trace cannot reuse older observation evidence");
    engine.script_trace_admission.raw_trace_fnv1a64 ^= UINT64_C(1);
    engine.script_vm.candidate_source_fnv1a64 ^= UINT64_C(1);
    CHECK(nexus_v1_engine_admit_slev_execution_trace_with_raw(
              &engine, trace, strlen(trace), raw_trace, sizeof(raw_trace) - 1,
              &admission) == 0 &&
          admission.status == NEXUS_V1_SLEV_TRACE_BLOCKED_TARGET_MISMATCH &&
          !admission.dispatch_permitted,
          "a changed active SLEV byte receipt rejects a stale capture trace");
}

static void test_sal_capture_target_binds_loaded_bytes(void) {
    uint8_t sal[] = {'d', 's', 'p', '0', '1', '.', 'E', 'X', 1, 2, 3, 4};
    uint8_t map[] = {0, 1, 2, 3, 4, 5, 6, 7};
    Nexus_V1_Engine engine;
    Nexus_V1_LevelSoundCaptureTargetReceipt target;
    Nexus_V1_LevelSoundTraceAdmissionReceipt admission;
    Nexus_V1_LevelSoundTraceHostReceipt host;
    Nexus_V1_LevelSoundRouteReceipt route;
    Nexus_V1_SalDispatchEvidenceReceipt evidence;
    static const uint8_t raw_trace[] =
        "pc=00001000 selector-dispatch\n"
        "pc=00001002 sal-read\n"
        "pc=00001004 driver-output\n";
    char trace[2048];

    memset(&engine, 0, sizeof(engine));
    engine.level_loaded = 1;
    engine.level_aux_runtime_receipt.level_index = 0;
    engine.level_aux_runtime_receipt.sal.canonical_hash_verified = 1;
    engine.level_aux_runtime_receipt.map.canonical_hash_verified = 1;
    engine.level_aux_runtime_receipt.sound_driver.canonical_hash_verified = 1;
    snprintf(engine.level_aux_runtime_receipt.sal.canonical_name,
             sizeof(engine.level_aux_runtime_receipt.sal.canonical_name),
             "SNDLEV00.SAL");
    snprintf(engine.level_aux_runtime_receipt.sal.canonical_md5,
             sizeof(engine.level_aux_runtime_receipt.sal.canonical_md5),
             "ea8493341fd8ad4f20335629e6dbdbbc");
    snprintf(engine.level_aux_runtime_receipt.map.canonical_name,
             sizeof(engine.level_aux_runtime_receipt.map.canonical_name),
             "SNDLEV00.MAP");
    snprintf(engine.level_aux_runtime_receipt.map.canonical_md5,
             sizeof(engine.level_aux_runtime_receipt.map.canonical_md5),
             "232afa942754027ecf49702703c72e83");
    snprintf(engine.level_aux_runtime_receipt.sound_driver.canonical_name,
             sizeof(engine.level_aux_runtime_receipt.sound_driver.canonical_name),
             "SDDRVS.TSK");
    snprintf(engine.level_aux_runtime_receipt.sound_driver.canonical_md5,
             sizeof(engine.level_aux_runtime_receipt.sound_driver.canonical_md5),
             "00000000000000000000000000000000");
    engine.audio.current_level = 0;
    engine.audio.sal_data = sal;
    engine.audio.sal_size = (int)sizeof(sal);
    engine.audio.sal_source_fnv1a64 = fnv1a64(sal, sizeof(sal));
    engine.audio.map_data = map;
    engine.audio.map_size = (int)sizeof(map);
    engine.audio.map_source_fnv1a64 = fnv1a64(map, sizeof(map));
    engine.audio.map_record_table_supported = 1;
    engine.audio.map_record_count = 1;
    engine.audio.map_records[0].selector = 7;
    engine.audio.map_records[0].attribute = 3;
    engine.audio.map_records[0].sal_offset = 8;
    engine.audio.map_records[0].sal_size = 4;

    CHECK(nexus_v1_current_level_sound_route_receipt(&engine, 7, &route) == 1 &&
          route.status == NEXUS_V1_LEVEL_SOUND_ROUTE_BOUND_OPAQUE &&
          route.canonical_sound_driver_source_verified &&
          !route.playback_permitted,
          "SAL route requires the verified sound-driver source before binding");

    engine.level_aux_runtime_receipt.sound_driver.canonical_hash_verified = 0;
    CHECK(nexus_v1_current_level_sound_route_receipt(&engine, 7, &route) == 0 &&
          route.status == NEXUS_V1_LEVEL_SOUND_ROUTE_BLOCKED_SOURCE &&
          !route.canonical_sound_driver_source_verified,
          "SAL route blocks without a verified sound-driver source");
    engine.level_aux_runtime_receipt.sound_driver.canonical_hash_verified = 1;

    CHECK(nexus_v1_engine_build_sal_capture_target(&engine, 7, &target) == 1 &&
          target.valid && target.canonical_sal_fnv1a64 ==
              fnv1a64(sal, sizeof(sal)) &&
          target.canonical_map_fnv1a64 == fnv1a64(map, sizeof(map)) &&
          target.no_playback_only && !target.playback_permitted,
          "SAL capture target binds the active raw SAL and MAP bytes");
    snprintf(trace, sizeof(trace),
             "magic=%s\nproducer=mednafen-debugger\n"
             "trace_sha256=0000000000000000000000000000000000000000000000000000000000000000\n"
             "level_index=%x\ncanonical_sal_name=%s\ncanonical_sal_md5=%s\n"
             "canonical_sal_fnv1a64=%016llx\n"
             "canonical_map_name=%s\ncanonical_map_md5=%s\n"
             "canonical_map_fnv1a64=%016llx\n"
             "canonical_driver_name=%s\ncanonical_driver_md5=%s\n"
             "raw_map_selector=%x\nmap_attribute=%x\nsal_offset=%x\nsal_size=%x\n"
             "raw_trace_fnv1a64=%016llx\n"
             "selector_dispatch_pc=1000\nsal_read_pc=1002\n"
             "driver_output_pc=1004\noriginal_saturn_execution=1\n",
             NEXUS_V1_SAL_TRACE_MAGIC, target.level_index,
             target.canonical_sal_name, target.canonical_sal_md5,
             (unsigned long long)target.canonical_sal_fnv1a64,
             target.canonical_map_name, target.canonical_map_md5,
             (unsigned long long)target.canonical_map_fnv1a64,
             target.canonical_driver_name, target.canonical_driver_md5,
             target.raw_map_selector, target.map_attribute,
             target.sal_offset, target.sal_size,
             (unsigned long long)fnv1a64(raw_trace, sizeof(raw_trace) - 1));
    CHECK(nexus_v1_engine_admit_sal_driver_trace(&engine, trace, strlen(trace),
                                                 &admission) == 1 &&
          admission.capture_target_bound && admission.trace_chain_complete &&
          !admission.raw_trace_bytes_bound &&
          !admission.driver_dispatch_proven && !admission.sal_decode_proven &&
          !admission.playback_permitted && admission.blocks_real_sfx_playback,
          "manifest-only SAL admission stays opaque and cannot bind raw trace");
    CHECK(nexus_v1_engine_consume_sal_driver_trace(&engine, &host) == 0 &&
          host.status == NEXUS_V1_SAL_TRACE_HOST_BLOCKED_TRACE &&
          !host.playback_permitted,
          "manifest-only SAL trace cannot reach the host");
    CHECK(nexus_v1_engine_admit_sal_driver_trace_with_raw(
              &engine, trace, strlen(trace), raw_trace, sizeof(raw_trace) - 1,
              &admission) == 1 &&
          admission.raw_trace_bytes_bound &&
          admission.raw_trace_fnv1a64 == fnv1a64(raw_trace, sizeof(raw_trace) - 1) &&
          admission.raw_trace_byte_count == sizeof(raw_trace) - 1,
          "SAL admission binds the exact raw capture bytes");
    CHECK(nexus_v1_engine_consume_sal_driver_trace(&engine, &host) == 0 &&
          host.status == NEXUS_V1_SAL_TRACE_HOST_BLOCKED_TRACE &&
          !host.playback_permitted,
          "raw SAL bytes need observed dispatch chronology before host intake");
    CHECK(nexus_v1_build_sal_dispatch_evidence(
              &engine, raw_trace, sizeof(raw_trace) - 1, &evidence) == 1 &&
          evidence.status == NEXUS_V1_SAL_DISPATCH_EVIDENCE_OBSERVED &&
          evidence.raw_trace_bound && evidence.observation_order_proven &&
          !evidence.driver_dispatch_proven && !evidence.sal_decode_proven &&
          !evidence.playback_permitted,
          "SAL raw trace retains ordered observation evidence without playback");
    CHECK(nexus_v1_engine_consume_sal_driver_trace(&engine, &host) == 1 &&
          host.status == NEXUS_V1_SAL_TRACE_HOST_CONSUMED_OPAQUE &&
          host.active_sal_target_revalidated && host.admitted_trace_bound &&
          host.host_consumed && !host.driver_dispatch_proven &&
          !host.sal_decode_proven && !host.playback_permitted &&
          host.blocks_real_sfx_playback,
          "SAL host consumes an admitted trace without dispatch or playback");
    engine.audio.map_source_fnv1a64 ^= UINT64_C(1);
    CHECK(nexus_v1_engine_admit_sal_driver_trace_with_raw(
              &engine, trace, strlen(trace), raw_trace, sizeof(raw_trace) - 1,
              &admission) == 0 &&
          admission.status == NEXUS_V1_SAL_TRACE_BLOCKED_TARGET_MISMATCH &&
          !admission.playback_permitted,
          "a changed active MAP identity rejects a stale SAL driver trace");
    CHECK(nexus_v1_engine_consume_sal_driver_trace(&engine, &host) == 0 &&
          host.status == NEXUS_V1_SAL_TRACE_HOST_BLOCKED_ACTIVE_ROUTE &&
          !host.playback_permitted,
          "a changed active MAP identity withdraws the host trace route");
    engine.audio.map_source_fnv1a64 = 0U;
    CHECK(nexus_v1_engine_build_sal_capture_target(&engine, 7, &target) == 0,
          "missing active MAP byte identity blocks SAL capture acquisition");
}

static void test_structure2_texture_decode_synthetic(void) {
    Nexus_V1_Engine engine;
    Nexus_V1_Level *level;
    Nexus_DMDFTextureSurface surfaces[4];
    Nexus_V1_DgnStructure2TextureDecodeReceipt receipt;

    memset(&engine, 0, sizeof(engine));
    memset(surfaces, 0, sizeof(surfaces));

    CHECK(nexus_v1_current_level_decode_structure2_textures(
              NULL, surfaces, 4, &receipt) == 0,
          "null engine blocks texture decode");
    CHECK(nexus_v1_current_level_decode_structure2_textures(
              &engine, surfaces, 4, &receipt) == 0,
          "unloaded engine blocks texture decode");
    level = &engine.current_level;
    (void)level;
}

int main(void) {
    test_variable_grid_and_mesh_ready();
    test_dgn_view_render_plan_from_structure1b();
    test_structure1f_out_of_prefix_ref_blocks_mesh();
    test_descriptor_budget_blocks_mesh_ready();
    test_post_grid_0x30_row_prefix_rejection();
    test_structure1c_bytes_do_not_invent_collision_geometry();
    test_bounds_and_legacy_non_promotion();
    test_determinism();
    test_structure1f_item_ibs_material_binding();
    test_real_item_ibs_special_floor_corpus();
    test_structure1c_record_table_bounds();
    test_structure1f_semantics_and_bounds();
    test_structure3_entry_header_boundaries();
    test_structure3_extreme_vector_arithmetic();
    test_visible_structure1f_semantics_block_render_plan();
    test_direct_structure1f_mesh_command_provenance();
    test_structure1g_semantics_and_bounds();
    test_structure1g_animated_floor_material_handoff();
    test_real_dgn_structure1_layout_corpus();
    test_real_structure1f_direct_cell_corpus();
    test_vdp1_command_sidecar_stays_no_draw();
    test_palette_source_gate();
    test_owner_material_capture_target_blocks_without_canonical_lev();
    test_menu_bpk_missing_handoff_blocks_fallback();
    test_menu_bpk_palette_trailer_stays_opaque();
    test_menu_bpk_handoff_requires_canonical_source();
    test_slev_capture_target_binds_loaded_bytes();
    test_sal_capture_target_binds_loaded_bytes();
    test_structure2_texture_decode_synthetic();

    if (g_fail != 0) {
        printf("Nexus V1 DGN geometry readiness gate: %d failure(s)\n", g_fail);
        return 1;
    }

    printf("Nexus V1 DGN geometry readiness gate passed\n");
    return 0;
}
