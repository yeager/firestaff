
#include "nexus_v1_dungeon.h"
#include "nexus_v1_dgn_mesh.h"
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

static uint32_t rb32(const uint8_t *p) {
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
}
static uint16_t rb16(const uint8_t *p) { return ((uint16_t)p[0]<<8)|p[1]; }
static uint16_t rl16(const uint8_t *p) { return (uint16_t)p[0]|((uint16_t)p[1]<<8); }

static uint32_t nexus_v1_fnv1a32(const uint8_t *data, int size)
{
    uint32_t hash = 2166136261u;
    int index;

    if (!data || size <= 0) return 0U;
    for (index = 0; index < size; ++index) {
        hash ^= data[index];
        hash *= 16777619u;
    }
    return hash;
}

static uint64_t nexus_v1_fnv1a64(const uint8_t *data, int size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    int index;

    if (!data || size <= 0) return 0U;
    for (index = 0; index < size; ++index) {
        hash ^= (uint64_t)data[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static int32_t rbs32(const uint8_t *p) {
    uint32_t value = rb32(p);
    return value <= INT32_MAX ? (int32_t)value :
        (int32_t)((int64_t)value - 0x100000000LL);
}

static uint64_t nexus_v1_fixed_vector_length_squared(int32_t x, int32_t y,
                                                       int32_t z)
{
    const int64_t components[3] = { x, y, z };
    uint64_t sum = 0;
    int component;

    for (component = 0; component < 3; ++component) {
        uint64_t magnitude = components[component] < 0
            ? (uint64_t)-components[component]
            : (uint64_t)components[component];
        uint64_t square = magnitude * magnitude;
        if (sum > UINT64_MAX - square) return UINT64_MAX;
        sum += square;
    }
    return sum;
}

static uint64_t nexus_v1_abs_i32(int32_t value)
{
    return value < 0 ? (uint64_t)-(int64_t)value : (uint64_t)value;
}

static uint64_t nexus_v1_dgn_structure3_edge_key(uint32_t first,
                                                 uint32_t second)
{
    uint32_t lo = first < second ? first : second;
    uint32_t hi = first < second ? second : first;
    return ((uint64_t)lo << 32) | (uint64_t)hi;
}

static int nexus_v1_dgn_structure3_edge_compare(const void *left,
                                                const void *right)
{
    uint64_t a = *(const uint64_t *)left;
    uint64_t b = *(const uint64_t *)right;
    return (a > b) - (a < b);
}

static int nexus_v1_structure3_face_has_noncollinear_vertices(
    const uint8_t *vertices, const uint16_t indexes[4], int slot_count)
{
    int first;

    for (first = 0; first < slot_count - 2; ++first) {
        int second;
        for (second = first + 1; second < slot_count - 1; ++second) {
            int third;
            for (third = second + 1; third < slot_count; ++third) {
                const uint8_t *a = vertices + indexes[first] * 12;
                const uint8_t *b = vertices + indexes[second] * 12;
                const uint8_t *c = vertices + indexes[third] * 12;
                int64_t ux = (int64_t)rbs32(b) - rbs32(a);
                int64_t uy = (int64_t)rbs32(b + 4) - rbs32(a + 4);
                int64_t uz = (int64_t)rbs32(b + 8) - rbs32(a + 8);
                int64_t vx = (int64_t)rbs32(c) - rbs32(a);
                int64_t vy = (int64_t)rbs32(c + 4) - rbs32(a + 4);
                int64_t vz = (int64_t)rbs32(c + 8) - rbs32(a + 8);

                if (uy * vz != uz * vy || uz * vx != ux * vz ||
                    ux * vy != uy * vx) return 1;
            }
        }
    }
    return 0;
}

static int nexus_v1_compare_u32(const void *left, const void *right)
{
    uint32_t a = *(const uint32_t *)left;
    uint32_t b = *(const uint32_t *)right;

    return a < b ? -1 : a > b;
}

static int nexus_v1_compare_u64(const void *left, const void *right)
{
    uint64_t a = *(const uint64_t *)left;
    uint64_t b = *(const uint64_t *)right;

    return a < b ? -1 : a > b;
}

static uint64_t nexus_v1_fixed_vector_dot(int64_t ax, int64_t ay, int64_t az,
                                          int64_t bx, int64_t by, int64_t bz)
{
#if defined(__SIZEOF_INT128__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    __int128 value = (__int128)ax * bx + (__int128)ay * by +
        (__int128)az * bz;
    __uint128_t magnitude = value < 0 ? (__uint128_t)-value :
        (__uint128_t)value;
#pragma GCC diagnostic pop
    return magnitude > UINT64_MAX ? UINT64_MAX : (uint64_t)magnitude;
#else
    long double value = (long double)ax * bx + (long double)ay * by +
        (long double)az * bz;
    long double magnitude = value < 0.0L ? -value : value;

    return magnitude > (long double)UINT64_MAX ? UINT64_MAX :
        (uint64_t)magnitude;
#endif
}

static int nexus_v1_fixed_face_winding_sign(const int32_t *a,
                                            const int32_t *b,
                                            const int32_t *c,
                                            const int32_t *normal)
{
    int64_t abx = (int64_t)b[0] - a[0];
    int64_t aby = (int64_t)b[1] - a[1];
    int64_t abz = (int64_t)b[2] - a[2];
    int64_t acx = (int64_t)c[0] - a[0];
    int64_t acy = (int64_t)c[1] - a[1];
    int64_t acz = (int64_t)c[2] - a[2];
#if defined(__SIZEOF_INT128__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    __int128 cross_x = (__int128)aby * acz - (__int128)abz * acy;
    __int128 cross_y = (__int128)abz * acx - (__int128)abx * acz;
    __int128 cross_z = (__int128)abx * acy - (__int128)aby * acx;
    __int128 winding = cross_x * normal[0] +
        (__int128)cross_y * normal[1] + (__int128)cross_z * normal[2];
#pragma GCC diagnostic pop

    return winding > 0 ? 1 : winding < 0 ? -1 : 0;
#else
    long double cross_x = (long double)aby * acz - (long double)abz * acy;
    long double cross_y = (long double)abz * acx - (long double)abx * acz;
    long double cross_z = (long double)abx * acy - (long double)aby * acx;
    long double winding = cross_x * normal[0] +
        (long double)cross_y * normal[1] + (long double)cross_z * normal[2];

    return winding > 0.0L ? 1 : winding < 0.0L ? -1 : 0;
#endif
}

static uint64_t nexus_v1_fixed_normal_plane_tolerance(const int32_t *a,
                                                        const int32_t *b)
{
    uint64_t dx = (uint64_t)llabs((long long)b[0] - a[0]);
    uint64_t dy = (uint64_t)llabs((long long)b[1] - a[1]);
    uint64_t dz = (uint64_t)llabs((long long)b[2] - a[2]);

    /* A 16.16 normal component is rounded by at most half of one raw unit.
     * The exact plane dot-product is zero, leaving this L1 edge bound. */
    return (dx + dy + dz + 1U) / 2U;
}

static const Nexus_V1_DgnStructure2Texture *
nexus_v1_level_get_structure2_texture(const Nexus_V1_Level *level,
                                      uint16_t image_id)
{
    int index;
    if (!level || !level->structure2_texture_table_valid) return NULL;
    for (index = 0; index < level->structure2_texture_count; ++index) {
        if (level->structure2_textures[index].image_id == image_id)
            return &level->structure2_textures[index];
    }
    return NULL;
}

static int nexus_v1_level_find_structure2_texture(
    const Nexus_V1_Level *level, uint16_t image_id)
{
    return nexus_v1_level_get_structure2_texture(level, image_id) != NULL;
}

int nexus_v1_level_structure2_source_envelope_valid(
    const Nexus_V1_Level *level)
{
    if (!level) return 0;
    return level->structure2_payload.valid &&
           level->structure2_payload.descriptor_offset_envelope_valid;
}

static int nexus_v1_level_copy_structure3_payload(
    Nexus_V1_Level *level, const uint8_t *data, int size)
{
    uint16_t block_offset;
    uint16_t block_count;
    int byte_offset;
    int byte_size;
    unsigned char seen[UINT8_MAX + 1U];
    uint32_t hash = 2166136261u;
    int block_index;
    int nonzero_block_run_start = -1;
    int nonzero_block_run_length = 0;
    int nonzero_byte_run_start = -1;
    int nonzero_byte_run_length = 0;
    int byte_index;
    Nexus_V1_DgnStructure3DirectoryReceipt directory;
    Nexus_V1_DgnStructure3EntryHeaderReceipt entry_headers;
    Nexus_V1_DgnStructure3FaceReceipt faces;
    Nexus_V1_DgnStructure3EdgeReceipt edges;
    Nexus_V1_DgnStructure3VectorReceipt vectors;
    Nexus_V1_DgnStructure3FaceGeometryReceipt face_geometry;
    Nexus_V1_DgnStructure3FaceEdgeReceipt face_edges;
    Nexus_V1_DgnStructure3FaceNormalPairReceipt face_normal_pairs;
    Nexus_V1_DgnStructure3FaceNormalGeometryReceipt face_normal_geometry;

    if (!level || !data || size < NEXUS_DGN_BLOCK_SIZE) return -1;
    /* DMWeb DGN container: Structure3's block offset/count follow the
     * Structure2 header pair. No field inside the resulting span is decoded. */
    block_offset = rb16(data + 0x1c);
    block_count = rb16(data + 0x1e);
    if (block_offset == 0U && block_count == 0U) return 0;
    if (block_offset == 0U || block_count == 0U ||
        block_offset > (uint16_t)(INT_MAX / NEXUS_DGN_BLOCK_SIZE) ||
        block_count > (uint16_t)(INT_MAX / NEXUS_DGN_BLOCK_SIZE)) return -1;
    byte_offset = (int)block_offset * NEXUS_DGN_BLOCK_SIZE;
    byte_size = (int)block_count * NEXUS_DGN_BLOCK_SIZE;
    if (byte_offset > size || byte_size > size - byte_offset) return -1;
    level->structure3_payload.declared = 1;
    level->structure3_payload.block_offset = (int)block_offset;
    level->structure3_payload.block_count = (int)block_count;
    level->structure3_payload.byte_offset = byte_offset;
    level->structure3_payload.byte_size = byte_size;
    level->structure3_payload.first_nonzero_byte_offset = -1;
    level->structure3_payload.last_nonzero_byte_offset = -1;
    level->structure3_payload.first_nonzero_byte_run_offset = -1;
    level->structure3_payload.last_nonzero_byte_run_offset = -1;
    level->structure3_payload.first_nonzero_block_index = -1;
    level->structure3_payload.last_nonzero_block_index = -1;
    level->structure3_payload.first_nonzero_block_run_start_block_index = -1;
    level->structure3_payload.last_nonzero_block_run_start_block_index = -1;
    level->structure3_payload.complete_block_count = (int)block_count;
    memset(&directory, 0, sizeof(directory));
    memset(&entry_headers, 0, sizeof(entry_headers));
    memset(&faces, 0, sizeof(faces));
    memset(&edges, 0, sizeof(edges));
    memset(&vectors, 0, sizeof(vectors));
    memset(&face_geometry, 0, sizeof(face_geometry));
    memset(&face_edges, 0, sizeof(face_edges));
    memset(&face_normal_pairs, 0, sizeof(face_normal_pairs));
    memset(&face_normal_geometry, 0, sizeof(face_normal_geometry));
    directory.payload_valid = 1;
    memset(seen, 0, sizeof(seen));
    for (block_index = 0; block_index < (int)block_count; ++block_index) {
        int block_nonzero = 0;
        int block_byte;
        for (block_byte = 0; block_byte < NEXUS_DGN_BLOCK_SIZE; ++block_byte) {
            uint8_t value;
            byte_index = block_index * NEXUS_DGN_BLOCK_SIZE + block_byte;
            value = data[byte_offset + byte_index];
            hash ^= value;
            hash *= 16777619u;
            if (value == 0U) {
                ++level->structure3_payload.zero_byte_count;
                if (nonzero_byte_run_length > 0) {
                    if (level->structure3_payload
                            .first_nonzero_byte_run_byte_count == 0) {
                        level->structure3_payload
                            .first_nonzero_byte_run_byte_count =
                            nonzero_byte_run_length;
                    }
                    level->structure3_payload.last_nonzero_byte_run_offset =
                        nonzero_byte_run_start;
                    level->structure3_payload.last_nonzero_byte_run_byte_count =
                        nonzero_byte_run_length;
                }
                nonzero_byte_run_start = -1;
                nonzero_byte_run_length = 0;
            } else {
                ++block_nonzero;
                ++level->structure3_payload.nonzero_byte_count;
                if (level->structure3_payload.first_nonzero_byte_offset < 0) {
                    level->structure3_payload.first_nonzero_byte_offset = byte_index;
                }
                level->structure3_payload.last_nonzero_byte_offset = byte_index;
                if (nonzero_byte_run_length == 0) {
                    ++level->structure3_payload.nonzero_byte_run_count;
                    nonzero_byte_run_start = byte_index;
                    if (level->structure3_payload
                            .first_nonzero_byte_run_offset < 0) {
                        level->structure3_payload.first_nonzero_byte_run_offset =
                            byte_index;
                    }
                }
                ++nonzero_byte_run_length;
                if (nonzero_byte_run_length >
                    level->structure3_payload.longest_nonzero_byte_run) {
                    level->structure3_payload.longest_nonzero_byte_run =
                        nonzero_byte_run_length;
                }
            }
            if (!seen[value]) {
                seen[value] = 1U;
                ++level->structure3_payload.distinct_byte_value_count;
            }
            if (byte_index > 0 && value != data[byte_offset + byte_index - 1]) {
                ++level->structure3_payload.byte_transition_count;
            }
        }
        if (block_nonzero == 0) {
            ++level->structure3_payload.zero_block_count;
            if (nonzero_block_run_length > 0) {
                if (level->structure3_payload.first_nonzero_block_run_block_count ==
                    0) {
                    level->structure3_payload
                        .first_nonzero_block_run_block_count =
                        nonzero_block_run_length;
                }
                level->structure3_payload.last_nonzero_block_run_start_block_index =
                    nonzero_block_run_start;
                level->structure3_payload.last_nonzero_block_run_block_count =
                    nonzero_block_run_length;
            }
            nonzero_block_run_start = -1;
            nonzero_block_run_length = 0;
        } else {
            ++level->structure3_payload.nonzero_block_count;
            if (level->structure3_payload.first_nonzero_block_index < 0) {
                level->structure3_payload.first_nonzero_block_index = block_index;
            }
            level->structure3_payload.last_nonzero_block_index = block_index;
            if (nonzero_block_run_length == 0) {
                ++level->structure3_payload.nonzero_block_run_count;
                nonzero_block_run_start = block_index;
                if (level->structure3_payload
                        .first_nonzero_block_run_start_block_index < 0) {
                    level->structure3_payload
                        .first_nonzero_block_run_start_block_index = block_index;
                }
            }
            ++nonzero_block_run_length;
            if (nonzero_block_run_length >
                level->structure3_payload.longest_nonzero_block_run) {
                level->structure3_payload.longest_nonzero_block_run =
                    nonzero_block_run_length;
            }
        }
    }
    if (nonzero_byte_run_length > 0) {
        if (level->structure3_payload.first_nonzero_byte_run_byte_count == 0) {
            level->structure3_payload.first_nonzero_byte_run_byte_count =
                nonzero_byte_run_length;
        }
        level->structure3_payload.last_nonzero_byte_run_offset =
            nonzero_byte_run_start;
        level->structure3_payload.last_nonzero_byte_run_byte_count =
            nonzero_byte_run_length;
    }
    if (nonzero_block_run_length > 0) {
        if (level->structure3_payload.first_nonzero_block_run_block_count == 0) {
            level->structure3_payload.first_nonzero_block_run_block_count =
                nonzero_block_run_length;
        }
        level->structure3_payload.last_nonzero_block_run_start_block_index =
            nonzero_block_run_start;
        level->structure3_payload.last_nonzero_block_run_block_count =
            nonzero_block_run_length;
    }
    level->structure3_payload.raw_payload_hash = hash ? hash : 1U;
    level->structure3_payload.valid = 1;
    level->structure3_payload.face_semantics_proven = 0;
    if (byte_size >= 4) {
        uint32_t count = rb32(data + byte_offset);
        uint64_t directory_bytes = 4U + (uint64_t)count * 4U;
        int entry;
        uint32_t previous = 0;

        directory.directory_declared = 1;
        if (count > 0U && count <= (uint32_t)INT_MAX &&
            directory_bytes <= (uint64_t)byte_size) {
            directory.entry_count = (int)count;
            directory.directory_byte_count = (int)directory_bytes;
            directory.offsets_strictly_increasing = 1;
            for (entry = 0; entry < directory.entry_count; ++entry) {
                uint32_t offset = rb32(data + byte_offset + 4 + entry * 4);
                if (entry == 0) directory.first_entry_offset = (int)offset;
                directory.last_entry_offset = (int)offset;
                if (offset < (uint32_t)directory.directory_byte_count ||
                    offset >= (uint32_t)byte_size ||
                    (entry > 0 && offset <= previous)) {
                    directory.offsets_strictly_increasing = 0;
                    break;
                }
                previous = offset;
            }
            directory.valid = directory.offsets_strictly_increasing;
        }
    }
    /* The directory bounds entry spans, but no entry grammar has yet been
     * recovered from original Saturn execution or capture evidence. */
    directory.entry_semantics_proven = 0;
    level->structure3_directory = directory;
    entry_headers.payload_valid = 1;
    entry_headers.directory_valid = directory.valid;
    entry_headers.fixed_header_byte_count =
        NEXUS_DGN_STRUCTURE3_ENTRY_HEADER_BYTES;
    if (directory.valid) {
        int entry;

        entry_headers.entry_count = directory.entry_count;
        entry_headers.boundaries_valid = 1;
        for (entry = 0; entry < directory.entry_count; ++entry) {
            uint32_t entry_offset = rb32(data + byte_offset + 4 + entry * 4);
            uint32_t entry_end = (entry + 1 < directory.entry_count)
                ? rb32(data + byte_offset + 4 + (entry + 1) * 4)
                : (uint32_t)byte_size;
            const uint8_t *header;
            uint32_t tag;
            uint16_t first_count;
            uint16_t second_count;
            uint32_t first_boundary;
            uint32_t second_boundary;
            uint32_t third_boundary;
            uint64_t expected_second;
            uint64_t expected_third;
            uint64_t expected_entry_end;

            if (entry_end < entry_offset ||
                entry_end - entry_offset <
                    NEXUS_DGN_STRUCTURE3_ENTRY_HEADER_BYTES) {
                entry_headers.boundaries_valid = 0;
                break;
            }
            header = data + byte_offset + entry_offset;
            tag = rb32(header);
            first_count = rb16(header + 4);
            second_count = rb16(header + 6);
            first_boundary = rb32(header + 8);
            second_boundary = rb32(header + 16);
            third_boundary = rb32(header + 20);
            expected_second = (uint64_t)first_boundary +
                (uint64_t)first_count * 12U;
            expected_third = expected_second +
                (uint64_t)second_count * 12U;
            expected_entry_end = expected_third +
                (uint64_t)second_count * 12U;
            if (tag == 0U) ++entry_headers.zero_tag_entry_count;
            else if (tag == 0x100U) ++entry_headers.tag_0x100_entry_count;
            else ++entry_headers.other_tag_entry_count;
            if (first_boundary != entry_offset +
                    NEXUS_DGN_STRUCTURE3_ENTRY_HEADER_BYTES ||
                expected_second > UINT32_MAX ||
                expected_third > UINT32_MAX ||
                expected_entry_end > UINT32_MAX ||
                second_boundary != (uint32_t)expected_second ||
                third_boundary != (uint32_t)expected_third ||
                first_boundary > second_boundary ||
                second_boundary > third_boundary ||
                expected_entry_end > entry_end) {
                entry_headers.boundaries_valid = 0;
                entry_headers.third_region_boundaries_valid = 0;
                break;
            }
            ++entry_headers.bounded_entry_count;
            entry_headers.first_region_element_count += first_count;
            entry_headers.second_region_element_count += second_count;
            entry_headers.third_region_element_count += second_count;
            ++entry_headers.complete_third_region_entry_count;
        }
        entry_headers.third_region_boundaries_valid =
            entry_headers.boundaries_valid &&
            entry_headers.complete_third_region_entry_count ==
                entry_headers.entry_count;
        entry_headers.valid = entry_headers.boundaries_valid &&
            entry_headers.bounded_entry_count == entry_headers.entry_count;
    }
    /* The fixed framing and 12-byte regions are not yet a face, mesh, or
     * texture grammar. Keep the renderer on its existing fail-closed gate. */
    entry_headers.semantics_proven = 0;
    level->structure3_entry_headers = entry_headers;
    faces.entry_headers_valid = entry_headers.valid;
    if (entry_headers.valid) {
        int entry;
        int face_vertex_linkage_measurement_complete = 1;
        int face_vertex_entry_coverage_measurement_complete = 1;
        int face_vertex_component_measurement_complete = 1;
        int face_vertex_component_entry_measurement_complete = 1;
        int face_vertex_adjacency_measurement_complete = 1;

        faces.entry_count = entry_headers.entry_count;
        faces.face_vertex_indexes_valid = 1;
        faces.face_vertex_linkage_valid = 0;
        faces.normal_count_matches_face_count = 1;
        for (entry = 0; entry < directory.entry_count; ++entry) {
            uint32_t entry_offset = rb32(data + byte_offset + 4 + entry * 4);
            const uint8_t *header = data + byte_offset + entry_offset;
            uint16_t vertex_count = rb16(header + 4);
            uint16_t face_count = rb16(header + 6);
            uint32_t face_offset = rb32(header + 16);
            uint32_t normal_offset = rb32(header + 20);
            int face_index;
            int *vertex_reference_counts = NULL;
            int *vertex_component_parents = NULL;
            uint32_t *vertex_adjacency_pairs = NULL;
            int vertex_adjacency_pair_count = 0;
            int vertex_adjacency_pair_capacity = 0;

            faces.vertex_count += vertex_count;
            faces.face_count += face_count;
            faces.normal_count += face_count;
            if ((uint64_t)normal_offset + (uint64_t)face_count * 12U >
                (uint64_t)byte_size) {
                faces.normal_count_matches_face_count = 0;
                break;
            }
            if (vertex_count > 0) {
                vertex_reference_counts = (int *)calloc(
                    (size_t)vertex_count, sizeof(*vertex_reference_counts));
                if (!vertex_reference_counts) {
                    face_vertex_linkage_measurement_complete = 0;
                }
                vertex_component_parents = (int *)malloc(
                    (size_t)vertex_count * sizeof(*vertex_component_parents));
                if (!vertex_component_parents) {
                    face_vertex_component_measurement_complete = 0;
                } else {
                    int vertex;
                    for (vertex = 0; vertex < (int)vertex_count; ++vertex) {
                        vertex_component_parents[vertex] = vertex;
                    }
                }
            }
            if (face_count > 0) {
                vertex_adjacency_pair_capacity = (int)face_count * 6;
                vertex_adjacency_pairs = (uint32_t *)malloc(
                    (size_t)vertex_adjacency_pair_capacity *
                    sizeof(*vertex_adjacency_pairs));
                if (!vertex_adjacency_pairs) {
                    face_vertex_adjacency_measurement_complete = 0;
                }
            }
            for (face_index = 0; face_index < (int)face_count; ++face_index) {
                const uint8_t *face = data + byte_offset + face_offset +
                    face_index * 12;
                uint16_t indexes[4] = {
                    rb16(face), rb16(face + 2), rb16(face + 4), rb16(face + 6)
                };
                uint16_t index2 = indexes[2];
                uint16_t index3 = indexes[3];
                uint16_t fill = rb16(face + 10);
                int slot_count = index2 == index3 ? 3 : 4;
                int distinct_count = 0;
                int index_valid = 1;
                int slot;

                for (slot = 0; slot < slot_count; ++slot) {
                    int earlier;
                    if (indexes[slot] >= vertex_count) {
                        index_valid = 0;
                        break;
                    }
                    for (earlier = 0; earlier < slot; ++earlier) {
                        if (indexes[earlier] == indexes[slot]) break;
                    }
                    if (earlier == slot) ++distinct_count;
                }
                if (!index_valid) {
                    faces.face_vertex_indexes_valid = 0;
                } else {
                    uint16_t distinct_indexes[4];
                    int distinct_index_count = 0;

                    for (slot = 0; slot < slot_count; ++slot) {
                        int earlier;

                        for (earlier = 0; earlier < slot; ++earlier) {
                            if (indexes[earlier] == indexes[slot]) break;
                        }
                        if (earlier == slot) {
                            distinct_indexes[distinct_index_count++] =
                                indexes[slot];
                        }
                    }
                    faces.distinct_face_vertex_count += distinct_count;
                    faces.repeated_face_vertex_reference_count +=
                        slot_count - distinct_count;
                    if (distinct_count == 1) ++faces.one_distinct_vertex_face_count;
                    else if (distinct_count == 2) ++faces.two_distinct_vertex_face_count;
                    else if (distinct_count == 3) ++faces.three_distinct_vertex_face_count;
                    else if (distinct_count == 4) ++faces.four_distinct_vertex_face_count;
                    faces.face_vertex_cooccurrence_pair_count +=
                        distinct_index_count * (distinct_index_count - 1) / 2;
                    if (vertex_adjacency_pairs) {
                        int left;

                        for (left = 0; left < distinct_index_count; ++left) {
                            int right;

                            for (right = left + 1; right < distinct_index_count;
                                 ++right) {
                                uint16_t low = distinct_indexes[left] <
                                    distinct_indexes[right]
                                    ? distinct_indexes[left]
                                    : distinct_indexes[right];
                                uint16_t high = distinct_indexes[left] <
                                    distinct_indexes[right]
                                    ? distinct_indexes[right]
                                    : distinct_indexes[left];

                                if (vertex_adjacency_pair_count >=
                                    vertex_adjacency_pair_capacity) {
                                    face_vertex_adjacency_measurement_complete =
                                        0;
                                } else {
                                    vertex_adjacency_pairs[
                                        vertex_adjacency_pair_count++] =
                                        ((uint32_t)low << 16) | high;
                                }
                            }
                        }
                    }
                    if (vertex_component_parents) {
                        int root = indexes[0];

                        while (vertex_component_parents[root] != root) {
                            root = vertex_component_parents[root];
                        }
                        for (slot = 1; slot < slot_count; ++slot) {
                            int other = indexes[slot];
                            int other_root;

                            while (vertex_component_parents[other] != other) {
                                other = vertex_component_parents[other];
                            }
                            other_root = other;
                            if (other_root != root) {
                                vertex_component_parents[other_root] = root;
                            }
                        }
                    }
                }
                if (vertex_reference_counts) {
                    if (indexes[0] < vertex_count)
                        ++vertex_reference_counts[indexes[0]];
                    if (indexes[1] < vertex_count)
                        ++vertex_reference_counts[indexes[1]];
                    if (index2 < vertex_count)
                        ++vertex_reference_counts[index2];
                    if (index3 != index2 && index3 < vertex_count)
                        ++vertex_reference_counts[index3];
                }
                if (index2 == index3) ++faces.triangle_count;
                else ++faces.quad_count;
                faces.face_vertex_reference_count +=
                    index2 == index3 ? 3 : 4;
                if ((face[8] & 0x40U) != 0U) ++faces.textured_face_count;
                if ((face[8] & 0x01U) != 0U) ++faces.mesh_transparent_face_count;
                if ((fill & 0x8000U) != 0U) ++faces.one_off_color_fill_count;
                else if ((fill & 0xff00U) == 0x0800U)
                    ++faces.animated_texture_fill_count;
                else if ((fill & 0xff00U) == 0U)
                    ++faces.static_texture_fill_count;
                else ++faces.unclassified_fill_count;
            }
            if (vertex_adjacency_pairs) {
                int pair;
                int unique_pair_count = 0;
                int single_pair_count = 0;
                int shared_pair_count = 0;

                qsort(vertex_adjacency_pairs,
                      (size_t)vertex_adjacency_pair_count,
                      sizeof(*vertex_adjacency_pairs), nexus_v1_compare_u32);
                for (pair = 0; pair < vertex_adjacency_pair_count; ++pair) {
                    if (pair == 0 || vertex_adjacency_pairs[pair] !=
                        vertex_adjacency_pairs[pair - 1]) {
                        int incidence = 1;

                        ++unique_pair_count;
                        while (pair + incidence < vertex_adjacency_pair_count &&
                               vertex_adjacency_pairs[pair + incidence] ==
                                   vertex_adjacency_pairs[pair]) {
                            ++incidence;
                        }
                        if (incidence == 1) ++single_pair_count;
                        else ++shared_pair_count;
                        if (incidence > faces.maximum_face_vertex_adjacency_pair_incidence) {
                            faces.maximum_face_vertex_adjacency_pair_incidence =
                                incidence;
                        }
                    }
                }
                faces.face_vertex_adjacency_pair_count += unique_pair_count;
                faces.repeated_face_vertex_adjacency_pair_count +=
                    vertex_adjacency_pair_count - unique_pair_count;
                faces.single_face_vertex_adjacency_pair_count += single_pair_count;
                faces.shared_face_vertex_adjacency_pair_count += shared_pair_count;
            }
            for (face_index = 0; face_index < (int)vertex_count; ++face_index) {
                int references = vertex_reference_counts
                    ? vertex_reference_counts[face_index] : 0;

                if (references > 0) ++faces.referenced_vertex_count;
                else ++faces.unreferenced_vertex_count;
                faces.linked_face_vertex_reference_count += references;
                if (references > faces.maximum_vertex_reference_count) {
                    faces.maximum_vertex_reference_count = references;
                }
            }
            if (vertex_component_parents && vertex_reference_counts) {
                int vertex;
                int component_count = 0;
                int referenced_vertex_count = 0;

                for (vertex = 0; vertex < (int)vertex_count; ++vertex) {
                    int root = vertex;
                    if (vertex_reference_counts[vertex] == 0) continue;
                    ++referenced_vertex_count;
                    while (vertex_component_parents[root] != root) {
                        root = vertex_component_parents[root];
                    }
                    if (root == vertex) ++component_count;
                }
                faces.face_vertex_component_count += component_count;
                if (referenced_vertex_count == (int)vertex_count)
                    ++faces.fully_referenced_vertex_entry_count;
                else
                    ++faces.partially_referenced_vertex_entry_count;
                if (component_count == 0)
                    ++faces.zero_component_vertex_entry_count;
                else if (component_count == 1)
                    ++faces.single_component_vertex_entry_count;
                else
                    ++faces.multiple_component_vertex_entry_count;
            } else if (vertex_count == 0) {
                ++faces.zero_vertex_entry_count;
                ++faces.zero_component_vertex_entry_count;
            } else {
                face_vertex_entry_coverage_measurement_complete = 0;
                face_vertex_component_entry_measurement_complete = 0;
            }
            free(vertex_reference_counts);
            free(vertex_component_parents);
            free(vertex_adjacency_pairs);
        }
        faces.face_vertex_linkage_valid =
            face_vertex_linkage_measurement_complete &&
            faces.referenced_vertex_count + faces.unreferenced_vertex_count ==
                faces.vertex_count &&
            faces.linked_face_vertex_reference_count ==
                faces.face_vertex_reference_count;
        faces.face_topology_accounting_valid =
            faces.face_vertex_indexes_valid &&
            faces.one_distinct_vertex_face_count +
                    faces.two_distinct_vertex_face_count +
                    faces.three_distinct_vertex_face_count +
                    faces.four_distinct_vertex_face_count == faces.face_count &&
            faces.distinct_face_vertex_count +
                    faces.repeated_face_vertex_reference_count ==
                faces.face_vertex_reference_count;
        faces.face_vertex_entry_coverage_accounting_valid =
            face_vertex_entry_coverage_measurement_complete &&
            faces.face_vertex_indexes_valid &&
            faces.fully_referenced_vertex_entry_count +
                    faces.partially_referenced_vertex_entry_count +
                    faces.zero_vertex_entry_count ==
                faces.entry_count;
        faces.face_vertex_component_accounting_valid =
            face_vertex_component_measurement_complete &&
            face_vertex_linkage_measurement_complete &&
            faces.face_vertex_indexes_valid &&
            faces.face_vertex_component_count >= 0 &&
            faces.face_vertex_component_count <= faces.referenced_vertex_count;
        faces.face_vertex_component_entry_accounting_valid =
            face_vertex_component_entry_measurement_complete &&
            faces.face_vertex_component_accounting_valid &&
            faces.zero_component_vertex_entry_count +
                    faces.single_component_vertex_entry_count +
                    faces.multiple_component_vertex_entry_count ==
                faces.entry_count;
        faces.face_vertex_adjacency_accounting_valid =
            face_vertex_adjacency_measurement_complete &&
            faces.face_vertex_indexes_valid &&
            faces.face_vertex_adjacency_pair_count +
                    faces.repeated_face_vertex_adjacency_pair_count ==
                faces.face_vertex_cooccurrence_pair_count;
        faces.face_vertex_adjacency_multiplicity_accounting_valid =
            faces.face_vertex_adjacency_accounting_valid &&
            faces.single_face_vertex_adjacency_pair_count +
                    faces.shared_face_vertex_adjacency_pair_count ==
                faces.face_vertex_adjacency_pair_count &&
            faces.repeated_face_vertex_adjacency_pair_count >=
                faces.shared_face_vertex_adjacency_pair_count &&
            ((faces.face_vertex_adjacency_pair_count == 0 &&
              faces.maximum_face_vertex_adjacency_pair_incidence == 0) ||
             (faces.face_vertex_adjacency_pair_count > 0 &&
              faces.maximum_face_vertex_adjacency_pair_incidence >= 1 &&
              faces.maximum_face_vertex_adjacency_pair_incidence <=
                  faces.face_vertex_cooccurrence_pair_count));
        faces.valid = faces.face_vertex_indexes_valid &&
            faces.face_vertex_linkage_valid &&
            faces.face_topology_accounting_valid &&
            faces.face_vertex_entry_coverage_accounting_valid &&
            faces.face_vertex_component_accounting_valid &&
            faces.face_vertex_component_entry_accounting_valid &&
            faces.face_vertex_adjacency_accounting_valid &&
            faces.face_vertex_adjacency_multiplicity_accounting_valid &&
            faces.normal_count_matches_face_count &&
            faces.unclassified_fill_count == 0 &&
            faces.face_vertex_reference_count ==
                faces.triangle_count * 3 + faces.quad_count * 4;
    }
    /* Record grammar alone cannot select material data or issue a draw. */
    faces.draw_semantics_proven = 0;
    level->structure3_faces = faces;
    edges.face_receipt_valid = faces.valid;
    edges.entry_count = directory.entry_count;
    if (faces.valid) {
        uint64_t *edge_keys;
        int edge_cursor = 0;
        int entry;
        int expected_edge_count = faces.triangle_count * 3 +
            faces.quad_count * 4;

        edge_keys = (uint64_t *)calloc((size_t)expected_edge_count,
                                       sizeof(*edge_keys));
        if (edge_keys) {
            for (entry = 0; entry < directory.entry_count; ++entry) {
                uint32_t entry_offset = rb32(data + byte_offset + 4 + entry * 4);
                const uint8_t *header = data + byte_offset + entry_offset;
                uint16_t face_count = rb16(header + 6);
                uint32_t face_offset = rb32(header + 16);
                int face_index;

                for (face_index = 0; face_index < (int)face_count; ++face_index) {
                    const uint8_t *face = data + byte_offset + face_offset +
                        face_index * 12;
                    uint16_t indexes[4] = {
                        rb16(face), rb16(face + 2), rb16(face + 4), rb16(face + 6)
                    };
                    int endpoint_count = indexes[2] == indexes[3] ? 3 : 4;
                    int endpoint;

                    for (endpoint = 0; endpoint < endpoint_count; ++endpoint) {
                        uint32_t first = ((uint32_t)entry << 16) | indexes[endpoint];
                        uint32_t second = ((uint32_t)entry << 16) |
                            indexes[(endpoint + 1) % endpoint_count];
                        if (first == second) ++edges.degenerate_edge_count;
                        edge_keys[edge_cursor++] =
                            nexus_v1_dgn_structure3_edge_key(first, second);
                    }
                }
            }
            qsort(edge_keys, (size_t)edge_cursor, sizeof(*edge_keys),
                  nexus_v1_dgn_structure3_edge_compare);
            for (int edge = 0; edge < edge_cursor;) {
                int uses = 1;
                while (edge + uses < edge_cursor &&
                       edge_keys[edge] == edge_keys[edge + uses]) {
                    ++uses;
                }
                ++edges.unique_edge_count;
                if (uses == 1) ++edges.single_use_edge_count;
                else if (uses == 2) ++edges.shared_edge_count;
                else ++edges.nonmanifold_edge_count;
                if (uses > edges.maximum_edge_use_count) {
                    edges.maximum_edge_use_count = uses;
                }
                edge += uses;
            }
            edges.edge_count = edge_cursor;
            edges.topology_measurement_complete =
                edge_cursor == expected_edge_count;
            free(edge_keys);
        }
    }
    edges.valid = edges.face_receipt_valid &&
        edges.topology_measurement_complete &&
        edges.edge_count == faces.triangle_count * 3 + faces.quad_count * 4 &&
        edges.unique_edge_count == edges.single_use_edge_count +
            edges.shared_edge_count + edges.nonmanifold_edge_count;
    edges.topology_semantics_proven = 0;
    level->structure3_edges = edges;
    vectors.face_receipt_valid = faces.valid;
    vectors.vertex_count = faces.vertex_count;
    vectors.normal_count = faces.normal_count;
    vectors.fixed_point_vectors_valid = faces.valid;
    if (faces.valid) {
        const uint64_t unit_length_squared = 65536ULL * 65536ULL;
        /* The documented unit vector is quantized to three signed 16.16
         * components. Keep a conservative fixed-point error envelope; the
         * corpus receipt retains the actual maximum for audit. */
        const uint64_t rounding_tolerance = 4ULL * 65536ULL;
        int entry;

        for (entry = 0; entry < directory.entry_count; ++entry) {
            uint32_t entry_offset = rb32(data + byte_offset + 4 + entry * 4);
            const uint8_t *header = data + byte_offset + entry_offset;
            uint16_t vertex_count = rb16(header + 4);
            uint16_t face_count = rb16(header + 6);
            uint32_t vertex_offset = rb32(header + 8);
            uint32_t face_offset = rb32(header + 16);
            uint32_t normal_offset = rb32(header + 20);
            int vector_index;
            int entry_pairs_unit_length = 1;

            for (vector_index = 0; vector_index < (int)vertex_count;
                 ++vector_index) {
                const uint8_t *vertex = data + byte_offset + vertex_offset +
                    vector_index * 12;
                uint64_t component_abs;

                component_abs = nexus_v1_abs_i32(rbs32(vertex));
                if (component_abs > (uint64_t)face_geometry
                        .maximum_component_absolute_value)
                    face_geometry.maximum_component_absolute_value =
                        component_abs > INT_MAX ? INT_MAX : (int)component_abs;
                component_abs = nexus_v1_abs_i32(rbs32(vertex + 4));
                if (component_abs > (uint64_t)face_geometry
                        .maximum_component_absolute_value)
                    face_geometry.maximum_component_absolute_value =
                        component_abs > INT_MAX ? INT_MAX : (int)component_abs;
                component_abs = nexus_v1_abs_i32(rbs32(vertex + 8));
                if (component_abs > (uint64_t)face_geometry
                        .maximum_component_absolute_value)
                    face_geometry.maximum_component_absolute_value =
                        component_abs > INT_MAX ? INT_MAX : (int)component_abs;
                if (rbs32(vertex) != 0 || rbs32(vertex + 4) != 0 ||
                    rbs32(vertex + 8) != 0) {
                    ++vectors.nonzero_vertex_vector_count;
                }
                ++vectors.vertex_vector_count;
            }
            for (vector_index = 0; vector_index < (int)face_count;
                 ++vector_index) {
                const uint8_t *normal = data + byte_offset + normal_offset +
                    vector_index * 12;
                const uint8_t *face = data + byte_offset + face_offset +
                    vector_index * 12;
                uint16_t indexes[4] = {
                    rb16(face), rb16(face + 2), rb16(face + 4), rb16(face + 6)
                };
                int32_t vertices[4][3];
                int32_t normal_vector[3] = {
                    rbs32(normal), rbs32(normal + 4), rbs32(normal + 8)
                };
                uint64_t length_squared = nexus_v1_fixed_vector_length_squared(
                    normal_vector[0], normal_vector[1], normal_vector[2]);
                uint64_t error = length_squared > unit_length_squared
                    ? length_squared - unit_length_squared
                    : unit_length_squared - length_squared;

                if (error > vectors.maximum_normal_length_error) {
                    vectors.maximum_normal_length_error = error;
                }
                if (error <= rounding_tolerance) {
                    ++vectors.normal_unit_length_count;
                    ++face_normal_pairs.unit_length_face_normal_pair_count;
                } else {
                    ++vectors.normal_non_unit_length_count;
                    ++face_normal_pairs.non_unit_length_face_normal_pair_count;
                    vectors.fixed_point_vectors_valid = 0;
                    entry_pairs_unit_length = 0;
                }
                ++vectors.normal_vector_count;
                ++face_normal_pairs.face_normal_pair_count;

                for (int index = 0; index < 4; ++index) {
                    const uint8_t *vertex = data + byte_offset + vertex_offset +
                        indexes[index] * 12;
                    vertices[index][0] = rbs32(vertex);
                    vertices[index][1] = rbs32(vertex + 4);
                    vertices[index][2] = rbs32(vertex + 8);
                }
                for (int triangle = 0;
                     triangle < (indexes[2] == indexes[3] ? 1 : 2);
                     ++triangle) {
                    int second = triangle == 0 ? 1 : 2;
                    int third = triangle == 0 ? 2 : 3;
                    int winding = nexus_v1_fixed_face_winding_sign(
                        vertices[0], vertices[second], vertices[third],
                        normal_vector);

                    for (int edge = 0; edge < 2; ++edge) {
                        int endpoint = edge == 0 ? second : third;
                        int64_t edge_x = (int64_t)vertices[endpoint][0] -
                            vertices[0][0];
                        int64_t edge_y = (int64_t)vertices[endpoint][1] -
                            vertices[0][1];
                        int64_t edge_z = (int64_t)vertices[endpoint][2] -
                            vertices[0][2];
                        uint64_t plane_error = nexus_v1_fixed_vector_dot(
                            edge_x, edge_y, edge_z, normal_vector[0],
                            normal_vector[1], normal_vector[2]);
                        uint64_t tolerance =
                            nexus_v1_fixed_normal_plane_tolerance(
                                vertices[0], vertices[endpoint]);

                        ++vectors.normal_face_plane_pair_count;
                        if (plane_error <= tolerance) {
                            ++vectors.normal_face_plane_within_tolerance_count;
                        } else {
                            /* Retail quads are not always planar: the stored
                             * normal is then a source fact measured outside
                             * the exact fixed-point plane envelope. Keep the
                             * measurement without revoking vector validity. */
                            ++vectors.normal_face_plane_outside_tolerance_count;
                        }
                        if (plane_error > vectors.maximum_normal_face_plane_error) {
                            vectors.maximum_normal_face_plane_error = plane_error;
                        }
                    }
                    if (winding > 0) ++vectors.positive_winding_triangle_count;
                    else if (winding < 0) ++vectors.negative_winding_triangle_count;
                    else {
                        ++vectors.zero_winding_triangle_count;
                        ++vectors.degenerate_face_triangle_count;
                    }
                }
            }
            if (entry_pairs_unit_length) {
                ++face_normal_pairs.complete_entry_pair_count;
            }
        }
    }
    vectors.valid = vectors.fixed_point_vectors_valid &&
        vectors.vertex_vector_count == vectors.vertex_count &&
        vectors.normal_vector_count == vectors.normal_count &&
        vectors.normal_unit_length_count == vectors.normal_count &&
        vectors.normal_face_plane_pair_count ==
            faces.triangle_count * 2 + faces.quad_count * 4;
    vectors.transform_or_draw_semantics_proven = 0;
    level->structure3_vectors = vectors;
    face_geometry.face_receipt_valid = faces.valid;
    face_geometry.vector_receipt_valid = vectors.valid;
    face_geometry.face_count = faces.face_count;
    face_geometry.cross_product_measurement_safe =
        face_geometry.maximum_component_absolute_value <=
        NEXUS_DGN_STRUCTURE3_GEOMETRY_MEASUREMENT_MAX_COMPONENT_ABS;
    if (faces.valid && vectors.valid &&
        face_geometry.cross_product_measurement_safe) {
        int entry;

        for (entry = 0; entry < directory.entry_count; ++entry) {
            uint32_t entry_offset = rb32(data + byte_offset + 4 + entry * 4);
            const uint8_t *header = data + byte_offset + entry_offset;
            const uint8_t *entry_vertices = data + byte_offset + rb32(header + 8);
            uint16_t face_count = rb16(header + 6);
            uint32_t face_offset = rb32(header + 16);
            int face_index;

            for (face_index = 0; face_index < (int)face_count; ++face_index) {
                const uint8_t *face = data + byte_offset + face_offset +
                    face_index * 12;
                uint16_t indexes[4] = {
                    rb16(face), rb16(face + 2), rb16(face + 4), rb16(face + 6)
                };
                int slot_count = indexes[2] == indexes[3] ? 3 : 4;

                ++face_geometry.measurement_face_count;
                if (nexus_v1_structure3_face_has_noncollinear_vertices(
                        entry_vertices, indexes, slot_count)) {
                    ++face_geometry.nondegenerate_face_count;
                } else {
                    ++face_geometry.degenerate_face_count;
                }
            }
        }
    }
    face_geometry.accounting_valid =
        face_geometry.measurement_face_count == face_geometry.face_count &&
        face_geometry.nondegenerate_face_count +
                face_geometry.degenerate_face_count ==
            face_geometry.measurement_face_count;
    face_geometry.valid = face_geometry.face_receipt_valid &&
        face_geometry.vector_receipt_valid &&
        face_geometry.cross_product_measurement_safe &&
        face_geometry.accounting_valid;
    face_geometry.surface_or_draw_semantics_proven = 0;
    level->structure3_face_geometry = face_geometry;
    face_edges.face_receipt_valid = faces.valid;
    face_edges.entry_count = faces.entry_count;
    face_edges.face_count = faces.face_count;
    if (faces.valid) {
        int entry;
        int measurement_complete = 1;

        for (entry = 0; entry < directory.entry_count; ++entry) {
            uint32_t entry_offset = rb32(data + byte_offset + 4 + entry * 4);
            const uint8_t *header = data + byte_offset + entry_offset;
            uint16_t face_count = rb16(header + 6);
            uint32_t face_offset = rb32(header + 16);
            uint64_t *edges = NULL;
            int edge_capacity = (int)face_count * 4;
            int edge_count = 0;
            int face_index;

            if (edge_capacity > 0) {
                edges = (uint64_t *)malloc((size_t)edge_capacity *
                                           sizeof(*edges));
                if (!edges) measurement_complete = 0;
            }
            for (face_index = 0; face_index < (int)face_count; ++face_index) {
                const uint8_t *face = data + byte_offset + face_offset +
                    face_index * 12;
                uint16_t indexes[4] = {
                    rb16(face), rb16(face + 2), rb16(face + 4), rb16(face + 6)
                };
                int slot_count = indexes[2] == indexes[3] ? 3 : 4;
                int slot;

                face_edges.face_edge_slot_count += slot_count;
                for (slot = 0; slot < slot_count; ++slot) {
                    uint16_t from = indexes[slot];
                    uint16_t to = indexes[(slot + 1) % slot_count];
                    uint16_t low;
                    uint16_t high;
                    uint64_t key;
                    int direction;

                    if (from == to) {
                        ++face_edges.degenerate_face_edge_reference_count;
                        continue;
                    }
                    ++face_edges.nondegenerate_face_edge_reference_count;
                    low = from < to ? from : to;
                    high = from < to ? to : from;
                    direction = from == low ? 0 : 1;
                    key = ((uint64_t)low << 16) | high;
                    if (!edges || edge_count >= edge_capacity) {
                        measurement_complete = 0;
                    } else {
                        edges[edge_count++] = (key << 1) | (uint64_t)direction;
                    }
                }
            }
            if (edges) {
                int edge;

                qsort(edges, (size_t)edge_count, sizeof(*edges),
                      nexus_v1_compare_u64);
                for (edge = 0; edge < edge_count;) {
                    uint64_t key = edges[edge] >> 1;
                    int incidence = 0;
                    int direction_count[2] = { 0, 0 };

                    do {
                        ++direction_count[edges[edge] & 1U];
                        ++incidence;
                        ++edge;
                    } while (edge < edge_count && (edges[edge] >> 1) == key);
                    ++face_edges.unique_face_edge_count;
                    if (incidence == 1) ++face_edges.boundary_face_edge_count;
                    else if (incidence == 2) {
                        ++face_edges.paired_face_edge_count;
                        if (direction_count[0] == 1 && direction_count[1] == 1)
                            ++face_edges.opposite_direction_paired_face_edge_count;
                        else
                            ++face_edges.same_direction_paired_face_edge_count;
                    } else {
                        ++face_edges.multi_incident_face_edge_count;
                    }
                    if (incidence > face_edges.maximum_face_edge_incidence)
                        face_edges.maximum_face_edge_incidence = incidence;
                }
            }
            free(edges);
        }
        face_edges.accounting_valid = measurement_complete &&
            face_edges.nondegenerate_face_edge_reference_count +
                    face_edges.degenerate_face_edge_reference_count ==
                face_edges.face_edge_slot_count &&
            face_edges.boundary_face_edge_count +
                    face_edges.paired_face_edge_count +
                    face_edges.multi_incident_face_edge_count ==
                face_edges.unique_face_edge_count &&
            face_edges.opposite_direction_paired_face_edge_count +
                    face_edges.same_direction_paired_face_edge_count ==
                face_edges.paired_face_edge_count;
    }
    face_edges.valid = face_edges.face_receipt_valid &&
        face_edges.accounting_valid;
    face_edges.winding_or_draw_semantics_proven = 0;
    level->structure3_face_edges = face_edges;
    face_normal_pairs.face_receipt_valid = faces.valid;
    face_normal_pairs.vector_receipt_valid = vectors.valid;
    face_normal_pairs.entry_count = faces.entry_count;
    face_normal_pairs.pairing_valid = faces.valid &&
        face_normal_pairs.face_normal_pair_count == faces.face_count &&
        face_normal_pairs.face_normal_pair_count == vectors.normal_vector_count &&
        face_normal_pairs.unit_length_face_normal_pair_count +
                face_normal_pairs.non_unit_length_face_normal_pair_count ==
            face_normal_pairs.face_normal_pair_count;
    face_normal_pairs.valid = face_normal_pairs.pairing_valid &&
        face_normal_pairs.complete_entry_pair_count == faces.entry_count &&
        face_normal_pairs.non_unit_length_face_normal_pair_count == 0;
    face_normal_pairs.normal_plane_or_draw_semantics_proven = 0;
    level->structure3_face_normal_pairs = face_normal_pairs;
    face_normal_geometry.face_receipt_valid = faces.valid;
    face_normal_geometry.vector_receipt_valid = vectors.valid;
    face_normal_geometry.face_normal_pairing_valid = face_normal_pairs.valid;
    face_normal_geometry.face_count = faces.face_count;
    /* 0x80000 keeps every cross/dot intermediate within signed int64 while
     * covering the measured retail coordinate range (max 450560). */
    face_normal_geometry.arithmetic_envelope_safe =
        face_geometry.maximum_component_absolute_value <= 0x80000;
    if (face_normal_geometry.face_receipt_valid &&
        face_normal_geometry.vector_receipt_valid &&
        face_normal_geometry.face_normal_pairing_valid &&
        face_normal_geometry.arithmetic_envelope_safe) {
        int entry;

        for (entry = 0; entry < directory.entry_count; ++entry) {
            uint32_t entry_offset = rb32(data + byte_offset + 4 + entry * 4);
            const uint8_t *header = data + byte_offset + entry_offset;
            const uint8_t *entry_vertices = data + byte_offset + rb32(header + 8);
            uint16_t face_count = rb16(header + 6);
            uint32_t face_offset = rb32(header + 16);
            uint32_t normal_offset = rb32(header + 20);
            int face_index;

            for (face_index = 0; face_index < (int)face_count; ++face_index) {
                const uint8_t *face = data + byte_offset + face_offset +
                    face_index * 12;
                const uint8_t *normal = data + byte_offset + normal_offset +
                    face_index * 12;
                uint16_t indexes[4] = {
                    rb16(face), rb16(face + 2), rb16(face + 4), rb16(face + 6)
                };
                int slot_count = indexes[2] == indexes[3] ? 3 : 4;
                const uint8_t *origin = entry_vertices + indexes[0] * 12;
                int32_t nx = rbs32(normal);
                int32_t ny = rbs32(normal + 4);
                int32_t nz = rbs32(normal + 8);
                int face_orthogonal = 1;
                int first, second, third;

                ++face_normal_geometry.measured_face_count;
                for (first = 1; first < slot_count; ++first) {
                    const uint8_t *vertex = entry_vertices + indexes[first] * 12;
                    int64_t dx = (int64_t)rbs32(vertex) - rbs32(origin);
                    int64_t dy = (int64_t)rbs32(vertex + 4) - rbs32(origin + 4);
                    int64_t dz = (int64_t)rbs32(vertex + 8) - rbs32(origin + 8);
                    int64_t dot = dx * nx + dy * ny + dz * nz;

                    ++face_normal_geometry.edge_test_count;
                    if (dot == 0) ++face_normal_geometry.orthogonal_edge_test_count;
                    else face_orthogonal = 0;
                }
                if (face_orthogonal) ++face_normal_geometry.orthogonal_face_count;
                else ++face_normal_geometry.nonorthogonal_face_count;
                for (first = 0; first < slot_count - 2; ++first) {
                    for (second = first + 1; second < slot_count - 1; ++second) {
                        for (third = second + 1; third < slot_count; ++third) {
                            const uint8_t *a = entry_vertices + indexes[first] * 12;
                            const uint8_t *b = entry_vertices + indexes[second] * 12;
                            const uint8_t *c = entry_vertices + indexes[third] * 12;
                            int64_t ux = (int64_t)rbs32(b) - rbs32(a);
                            int64_t uy = (int64_t)rbs32(b + 4) - rbs32(a + 4);
                            int64_t uz = (int64_t)rbs32(b + 8) - rbs32(a + 8);
                            int64_t vx = (int64_t)rbs32(c) - rbs32(a);
                            int64_t vy = (int64_t)rbs32(c + 4) - rbs32(a + 4);
                            int64_t vz = (int64_t)rbs32(c + 8) - rbs32(a + 8);
                            int64_t cross_x = uy * vz - uz * vy;
                            int64_t cross_y = uz * vx - ux * vz;
                            int64_t cross_z = ux * vy - uy * vx;
                            int64_t dot = cross_x * nx + cross_y * ny + cross_z * nz;

                            if (cross_x == 0 && cross_y == 0 && cross_z == 0)
                                continue;
                            if (dot > 0) ++face_normal_geometry.positive_cross_normal_dot_count;
                            else if (dot < 0) ++face_normal_geometry.negative_cross_normal_dot_count;
                            else ++face_normal_geometry.zero_cross_normal_dot_count;
                            first = slot_count;
                            second = slot_count;
                            break;
                        }
                    }
                }
            }
        }
    }
    face_normal_geometry.accounting_valid =
        face_normal_geometry.measured_face_count == face_normal_geometry.face_count &&
        face_normal_geometry.orthogonal_face_count +
                face_normal_geometry.nonorthogonal_face_count ==
            face_normal_geometry.measured_face_count &&
        face_normal_geometry.orthogonal_edge_test_count <=
            face_normal_geometry.edge_test_count &&
        face_normal_geometry.positive_cross_normal_dot_count +
                face_normal_geometry.negative_cross_normal_dot_count +
                face_normal_geometry.zero_cross_normal_dot_count ==
            face_normal_geometry.measured_face_count;
    face_normal_geometry.valid = face_normal_geometry.face_receipt_valid &&
        face_normal_geometry.vector_receipt_valid &&
        face_normal_geometry.face_normal_pairing_valid &&
        face_normal_geometry.arithmetic_envelope_safe &&
        face_normal_geometry.accounting_valid;
    face_normal_geometry.normal_plane_or_draw_semantics_proven = 0;
    level->structure3_face_normal_geometry = face_normal_geometry;
    return 0;
}

static int nexus_v1_level_copy_structure2_textures(Nexus_V1_Level *level,
                                                    const uint8_t *data,
                                                    int size)
{
    uint16_t structure2_block;
    uint16_t structure2_blocks;
    uint32_t structure2_useful;
    int structure2_offset;
    int structure2_size;
    int cursor;

    if (!level || !data || size < NEXUS_DGN_BLOCK_SIZE) return -1;
    structure2_block = rb16(data + 0x14);
    structure2_blocks = rb16(data + 0x16);
    structure2_useful = rb32(data + 0x18);
    if (structure2_block == 0U && structure2_blocks == 0U &&
        structure2_useful == 0U) return 0;
    if (structure2_block == 0U || structure2_blocks == 0U ||
        structure2_useful < NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES ||
        structure2_useful > (uint32_t)INT_MAX) return -1;
    structure2_offset = (int)structure2_block * NEXUS_DGN_BLOCK_SIZE;
    structure2_size = (int)structure2_blocks * NEXUS_DGN_BLOCK_SIZE;
    if (structure2_offset > size || structure2_size > size - structure2_offset ||
        structure2_useful > (uint32_t)structure2_size) return -1;

    /* DMWeb establishes only this envelope: Descriptor[20]... FFFF followed
     * by raw palette/image bytes. The bytes after FFFF have no corpus-proven
     * record grammar yet, so keep their span bounded and prohibit promotion. */
    for (cursor = 0; cursor <= (int)structure2_useful - 2;) {
        const uint8_t *src = data + structure2_offset + cursor;
        Nexus_V1_DgnStructure2Texture *dst;
        uint16_t image_id = rb16(src);
        if (image_id == 0xffffU) {
            int descriptor_index;
            int opaque_offset = cursor + 2;
            int opaque_index;
            uint32_t observed_offsets[NEXUS_DGN_MAX_STRUCTURE2_TEXTURES * 2];
            int observed_offset_count = 0;
            level->structure2_texture_table_valid = 1;
            level->structure2_payload.descriptor_bytes = cursor;
            level->structure2_payload.terminator_offset = cursor;
            level->structure2_payload.opaque_payload_offset = opaque_offset;
            level->structure2_payload.opaque_payload_size =
                (int)structure2_useful - opaque_offset;
            for (opaque_index = opaque_offset;
                 opaque_index < (int)structure2_useful;
                 ++opaque_index) {
                if (data[structure2_offset + opaque_index] == 0U) {
                    ++level->structure2_payload.opaque_payload_zero_byte_count;
                } else {
                    ++level->structure2_payload.opaque_payload_nonzero_byte_count;
                }
            }
            for (opaque_index = opaque_offset;
                 opaque_index + 1 < (int)structure2_useful;
                 opaque_index += 2) {
                ++level->structure2_payload.opaque_payload_complete_pair_count;
                if (data[structure2_offset + opaque_index] == 0U &&
                    data[structure2_offset + opaque_index + 1] == 0U) {
                    ++level->structure2_payload.opaque_payload_zero_pair_count;
                } else {
                    ++level->structure2_payload.opaque_payload_nonzero_pair_count;
                }
            }
            level->structure2_payload.opaque_payload_trailing_byte_count =
                level->structure2_payload.opaque_payload_size & 1;
            for (descriptor_index = 0;
                 descriptor_index < level->structure2_texture_count;
                 ++descriptor_index) {
                const Nexus_V1_DgnStructure2Texture *descriptor =
                    &level->structure2_textures[descriptor_index];
                uint32_t offsets[2];
                int offset_index;

                offsets[0] = descriptor->image_relative_offset;
                offsets[1] = descriptor->palette_relative_offset;
                for (offset_index = 0; offset_index < 2; ++offset_index) {
                    uint32_t relative_offset = offsets[offset_index];
                    int observed_index;
                    int already_observed = 0;
                    if (relative_offset == 0U) continue;
                    ++level->structure2_payload.nonzero_descriptor_offset_count;
                    for (observed_index = 0;
                         observed_index < observed_offset_count;
                         ++observed_index) {
                        if (observed_offsets[observed_index] == relative_offset) {
                            already_observed = 1;
                            break;
                        }
                    }
                    if (already_observed) {
                        ++level->structure2_payload
                              .nonzero_descriptor_offset_reused_count;
                    } else {
                        observed_offsets[observed_offset_count++] = relative_offset;
                        ++level->structure2_payload
                              .nonzero_descriptor_offset_unique_count;
                    }
                    if (relative_offset >= (uint32_t)opaque_offset &&
                        relative_offset < structure2_useful) {
                        ++level->structure2_payload
                            .nonzero_descriptor_offsets_in_opaque_payload_count;
                    } else {
                        ++level->structure2_payload
                            .nonzero_descriptor_offsets_outside_opaque_payload_count;
                    }
                    if (relative_offset >= (uint32_t)opaque_offset &&
                        relative_offset <= structure2_useful - 2U) {
                        ++level->structure2_payload
                            .nonzero_descriptor_offsets_word_bounded_count;
                    }
                    if ((relative_offset & 1U) != 0U) {
                        ++level->structure2_payload
                            .nonzero_descriptor_offsets_unaligned_count;
                    }
                }
            }
            level->structure2_payload.local_payload_offset_pattern_observed =
                level->structure2_payload.nonzero_descriptor_offset_count > 0 &&
                level->structure2_payload
                    .nonzero_descriptor_offsets_outside_opaque_payload_count == 0;
            level->structure2_payload
                .local_payload_word_aligned_offset_pattern_observed =
                level->structure2_payload.local_payload_offset_pattern_observed &&
                level->structure2_payload
                    .nonzero_descriptor_offsets_unaligned_count == 0;
            level->structure2_payload
                .local_payload_word_bounded_offset_pattern_observed =
                level->structure2_payload.local_payload_offset_pattern_observed &&
                level->structure2_payload
                    .nonzero_descriptor_offsets_word_bounded_count ==
                    level->structure2_payload.nonzero_descriptor_offset_count;
            /* Preserve a format-envelope gate separately from the measured
             * corpus patterns above. Zero offsets are allowed; each present
             * target must remain aligned and fully bounded in its descriptor
             * envelope before Structure1G can hand it to a host. */
            level->structure2_payload.descriptor_offset_envelope_valid =
                level->structure2_payload
                    .nonzero_descriptor_offsets_outside_opaque_payload_count == 0 &&
                level->structure2_payload
                    .nonzero_descriptor_offsets_unaligned_count == 0 &&
                level->structure2_payload
                    .nonzero_descriptor_offsets_word_bounded_count ==
                    level->structure2_payload.nonzero_descriptor_offset_count;
            level->structure2_payload.valid = 1;
            {
                int proven = level->structure2_payload
                    .descriptor_offset_envelope_valid;
                for (descriptor_index = 0;
                     proven && descriptor_index < level->structure2_texture_count;
                     ++descriptor_index) {
                    const Nexus_V1_DgnStructure2Texture *tex =
                        &level->structure2_textures[descriptor_index];
                    uint32_t image_bytes;
                    if (tex->encoding == 0x0008U)
                        image_bytes = (uint32_t)tex->width * tex->height / 2U;
                    else if (tex->encoding == 0x0028U)
                        image_bytes = (uint32_t)tex->width * tex->height * 2U;
                    else { proven = 0; break; }
                    if (image_bytes == 0U ||
                        tex->image_relative_offset < (uint32_t)opaque_offset ||
                        tex->image_relative_offset + image_bytes > structure2_useful)
                    { proven = 0; break; }
                    if (tex->palette_relative_offset != 0U &&
                        (tex->palette_relative_offset < (uint32_t)opaque_offset ||
                         tex->palette_relative_offset + 32U > structure2_useful))
                    { proven = 0; break; }
                }
                level->structure2_payload.material_or_image_data_proven = proven;
            }
            return 0;
        }
        if (cursor > (int)structure2_useful -
                NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES ||
            level->structure2_texture_count >= NEXUS_DGN_MAX_STRUCTURE2_TEXTURES ||
            image_id != (uint16_t)level->structure2_texture_count) return -1;
        dst = &level->structure2_textures[level->structure2_texture_count];
        dst->image_id = image_id;
        dst->encoding = rb16(src + 2);
        dst->palette_id = rb16(src + 4);
        dst->width = rb16(src + 6);
        dst->height = rb16(src + 8);
        dst->image_relative_offset = rb32(src + 12);
        dst->palette_relative_offset = rb32(src + 16);
        if ((dst->encoding != 0x0008U && dst->encoding != 0x0028U) ||
            dst->width == 0U || dst->height == 0U) return -1;
        level->structure2_texture_count++;
        cursor += NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES;
    }
    return -1;
}

static int nexus_v1_dgn_parse_structure1g(
    Nexus_V1_DgnStructure1GTable *out_table,
    const uint8_t *data,
    const Nexus_V1_DgnStructure1Layout *layout)
{
    const Nexus_V1_DgnStructure1PostGridPointer *span;
    Nexus_V1_DgnStructure1GTable table;
    const uint8_t *src;
    int descriptor;

    if (out_table) memset(out_table, 0, sizeof(*out_table));
    if (!out_table || !data || !layout) return -1;
    span = &layout->post_grid[1];
    if (!span->present) return 0;
    if (!span->bounded || span->relative_offset < 0 ||
        span->relative_offset > layout->useful_size - NEXUS_DGN_STRUCTURE1G_HEADER_BYTES)
        return -1;

    memset(&table, 0, sizeof(table));
    table.relative_offset = span->relative_offset;
    /* Structure1 header pointers are not address ordered. Structure1G ends
     * at its own FF FF instruction, so its enclosing bound is useful
     * Structure1 data, never the next named header pointer. */
    table.size = layout->useful_size - span->relative_offset;
    src = data + layout->structure1_offset + span->relative_offset;
    table.descriptor_count = (int)rb16(src);
    table.animation_data_relative_offset = (int)rb16(src + 2);
    if (table.descriptor_count < 1 ||
        table.descriptor_count > NEXUS_DGN_MAX_STRUCTURE1G_ENTRIES ||
        table.animation_data_relative_offset !=
            NEXUS_DGN_STRUCTURE1G_HEADER_BYTES +
            table.descriptor_count * NEXUS_DGN_STRUCTURE1G_DESCRIPTOR_BYTES ||
        table.animation_data_relative_offset > table.size) return -1;

    for (descriptor = 0; descriptor < table.descriptor_count; ++descriptor) {
        const uint8_t *entry = src + NEXUS_DGN_STRUCTURE1G_HEADER_BYTES +
            descriptor * NEXUS_DGN_STRUCTURE1G_DESCRIPTOR_BYTES;
        uint16_t sequence_word_offset;
        int next_sequence_word_offset;
        int sequence_byte_offset;
        int cursor;
        int terminated = 0;
        if (descriptor == table.descriptor_count - 1) {
            if (entry[0] != 0xffU) return -1;
            continue;
        }
        if (entry[0] == 0xffU || entry[1] != 0U || rb16(entry + 2) != 1U)
            return -1;
        sequence_word_offset = rb16(entry + 6);
        next_sequence_word_offset = descriptor + 1 < table.descriptor_count - 1
            ? (int)rb16(entry + NEXUS_DGN_STRUCTURE1G_DESCRIPTOR_BYTES + 6)
            : (table.size - table.animation_data_relative_offset) / 4;
        sequence_byte_offset = table.animation_data_relative_offset +
            (int)sequence_word_offset * 4;
        if (sequence_byte_offset < table.animation_data_relative_offset ||
            sequence_word_offset >= (uint16_t)next_sequence_word_offset ||
            sequence_byte_offset > table.size - 4 ||
            rb16(src + sequence_byte_offset) != rb16(entry + 4)) return -1;
        for (cursor = sequence_byte_offset;
             cursor < table.animation_data_relative_offset +
                 next_sequence_word_offset * 4;
             cursor += 4) {
            uint16_t instruction = rb16(src + cursor);
            if (instruction == 0xffffU) {
                terminated = 1;
                break;
            }
            if (instruction == 0xfffeU) {
                int target = (int)(int16_t)rb16(src + cursor + 2);
                int instruction_index =
                    (cursor - sequence_byte_offset) / 4;
                if (target >= 0 || -target > instruction_index) return -1;
                table.goto_instruction_count++;
                continue;
            }
            if (instruction < NEXUS_DGN_STRUCTURE1G_FIRST_IMAGE_INDEX)
                return -1;
            table.image_instruction_count++;
        }
        if (!terminated) return -1;
        table.animated_texture_count++;
        table.sequence_count++;
    }
    table.valid = 1;
    *out_table = table;
    return 0;
}

static int nexus_v1_dgn_parse_structure1a(
    Nexus_V1_DgnStructure1ATable *out_table,
    const uint8_t *data,
    const Nexus_V1_DgnStructure1Layout *layout)
{
    const uint8_t *src;
    Nexus_V1_DgnStructure1ATable table;
    uint32_t count;
    uint32_t relative_offset;

    if (out_table) memset(out_table, 0, sizeof(*out_table));
    if (!out_table || !data || !layout) return -1;
    src = data + layout->structure1_offset;
    count = rb32(src + 0x0c);
    relative_offset = rb32(src + 0x10);
    if (relative_offset != 0x38U ||
        count > NEXUS_DGN_MAX_STRUCTURE1A_ENTRIES ||
        count > (uint32_t)(INT_MAX / NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES)) {
        return -1;
    }
    memset(&table, 0, sizeof(table));
    table.relative_offset = (int)relative_offset;
    table.entry_count = (int)count;
    table.size = table.entry_count * NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES;
    if (table.relative_offset > layout->structure1b_relative_offset ||
        table.size > layout->structure1b_relative_offset - table.relative_offset) {
        return -1;
    }
    table.valid = 1;
    *out_table = table;
    return 0;
}

static int nexus_v1_dgn_parse_structure1f(
    Nexus_V1_DgnStructure1FTable *out_table,
    const uint8_t *data,
    const Nexus_V1_DgnStructure1Layout *layout)
{
    static const int record_sizes[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT] =
        {8, 12, 16, 12, 12, 16};
    static const uint8_t tags[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT] =
        {0x10U, 0x11U, 0x12U, 0x20U, 0x21U, 0x22U};
    const Nexus_V1_DgnStructure1PostGridPointer *span;
    Nexus_V1_DgnStructure1FTable table;
    const uint8_t *src;
    int cursor;
    int family;

    if (out_table) memset(out_table, 0, sizeof(*out_table));
    if (!out_table || !data || !layout) return -1;
    span = &layout->post_grid[5];
    if (!span->present || !span->bounded ||
        span->size_to_next < NEXUS_DGN_STRUCTURE1F_HEADER_BYTES) return -1;

    memset(&table, 0, sizeof(table));
    table.relative_offset = span->relative_offset;
    table.size = span->size_to_next;
    src = data + layout->structure1_offset + span->relative_offset;
    table.wall_sensor_first_texture_index = rb16(src);
    table.wall_sensor_first_model_index = rb16(src + 2);
    cursor = NEXUS_DGN_STRUCTURE1F_HEADER_BYTES;
    for (family = 0; family < NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT; ++family) {
        int count = (int)rb16(src + 4 + family * 2);
        int bytes;
        if (count > NEXUS_DGN_MAX_STRUCTURE1F_ENTRIES ||
            count > (INT_MAX - cursor) / record_sizes[family]) return -1;
        bytes = count * record_sizes[family];
        if (bytes > table.size - cursor ||
            table.total_entry_count > NEXUS_DGN_MAX_STRUCTURE1F_ENTRIES - count)
            return -1;
        table.family_count[family] = count;
        table.family_offset[family] = span->relative_offset + cursor;
        table.family_record_size[family] = record_sizes[family];
        table.total_entry_count += count;
        cursor += bytes;
    }
    /* Structure1F is the final useful Structure1 span. Its six counts must
     * account for the complete span: padding belongs outside useful data. */
    if (cursor != table.size) return -1;
    for (family = 0; family < NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT; ++family) {
        int record;
        const uint8_t *records = data + layout->structure1_offset +
            table.family_offset[family];
        for (record = 0; record < table.family_count[family]; ++record) {
            const uint8_t *entry = records + record * record_sizes[family];
            if (entry[0] != tags[family]) return -1;
        }
    }
    /* Exact final-span counts and the six source tags are enough to retain
     * a Structure1F declaration.  Keep it even when direct-cell validation
     * below fails so host handoff cannot silently lose an original record. */
    table.declared = 1;
    for (family = 0; family < NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT; ++family) {
        int record;
        const uint8_t *records = data + layout->structure1_offset +
            table.family_offset[family];
        for (record = 0; record < table.family_count[family]; ++record) {
            const uint8_t *entry = records + record * record_sizes[family];
            /* Structure1Fa through Structure1Fc carry documented 64x64
             * coordinates. Alcove and wall records bind through Structure1A. */
            if (family <= NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS &&
                (entry[1] >= NEXUS_MAX_MAP_SIZE || entry[2] >= NEXUS_MAX_MAP_SIZE)) {
                *out_table = table;
                return 0;
            }
        }
    }
    table.valid = 1;
    *out_table = table;
    return 0;
}

static int nexus_v1_decode_structure1b_collision_ref(const uint8_t *cell) {
    if (!cell) {
        return 0;
    }
    return (int)((((unsigned)cell[6] & 0x0FU) << 8) | (unsigned)cell[7]);
}

static uint8_t nexus_v1_decode_structure1b_wall_material(
    const uint8_t *cell, int wall_dir) {
    /* DMWeb DGN Structure1B: byte 3 holds north/east surface ids and
     * byte 4 holds south/west ids. The directional bit is the missing link
     * between the level's material references and the mesh command. */
    return cell[(wall_dir & 3) < 2 ? 3 : 4];
}

static uint16_t nexus_v1_decode_structure1b_post_grid_0x30_ref(
    const uint8_t *cell) {
    return (uint16_t)((((unsigned)cell[5] << 4) |
                       ((unsigned)cell[6] >> 4)) & 0x0fffU);
}

static int nexus_v1_decode_structure1b_structure1a_ref(
    const uint8_t *cell, uint16_t *out_ref)
{
    if (!cell || !out_ref || (cell[4] & 0x80U) == 0U) return 0;
    *out_ref = (uint16_t)((((unsigned)cell[5] << 4) |
                           ((unsigned)cell[6] >> 4)) & 0x0fffU);
    return 1;
}

static int nexus_v1_post_grid_0x30_ref_is_bounded(
    const Nexus_V1_DgnStructure1Layout *layout, int ref) {
    if (ref == 0 || ref == 0x0fff) {
        return 1;
    }
    return layout && layout->post_grid_0x30_records.valid &&
        ref >= 0 &&
        ref < layout->post_grid_0x30_records.typed_prefix_record_count;
}

static uint8_t nexus_v1_decode_structure1b_floor_material(const uint8_t *cell) {
    return (uint8_t)((rb16(cell) >> 7) & 0x1fU);
}

static int nexus_v1_decode_structure1b_floor_animation_id(
    const uint8_t *cell, uint8_t *out_id) {
    if (!cell || !out_id || (cell[4] & 0x0fU) != 3U) return 0;
    *out_id = nexus_v1_decode_structure1b_floor_material(cell);
    return 1;
}

static uint8_t nexus_v1_decode_structure1b_ceiling_material(
    const uint8_t *header, const uint8_t *cell) {
    unsigned selection = (rb16(cell) >> 1) & 3U;
    return selection == 0U ? 0U : header[8 + selection];
}

static int nexus_v1_decode_structure1b_cell(const uint8_t *cell) {
    uint16_t flags;
    unsigned collision;
    if (!cell) {
        return 0;
    }
    flags = rb16(cell);
    collision = (unsigned)nexus_v1_decode_structure1b_collision_ref(cell);
    if (collision == 0x0FFFU) {
        return 0; /* wall / cannot enter */
    }
    if ((flags & 0x0001U) != 0) {
        return 8; /* door present */
    }
    return 1; /* free corridor/floor */
}

int nexus_v1_dgn_structure1_layout(Nexus_V1_DgnStructure1Layout *out_layout,
                                   const uint8_t *data,
                                   int size) {
    static const int header_offsets[NEXUS_DGN_STRUCTURE1_POST_GRID_POINTER_COUNT] =
        {0x18, 0x1c, 0x24, 0x2c, 0x30, 0x34};
    Nexus_V1_DgnStructure1Layout layout;
    uint16_t block;
    uint16_t blocks;
    uint32_t useful;
    int i;

    if (out_layout) memset(out_layout, 0, sizeof(*out_layout));
    if (!out_layout || !data || size < NEXUS_DGN_BLOCK_SIZE) return -1;
    block = rb16(data + 0x0c);
    blocks = rb16(data + 0x0e);
    useful = rb32(data + 0x10);
    if (block == 0 || blocks == 0 ||
        block > (uint16_t)(INT_MAX / NEXUS_DGN_BLOCK_SIZE) ||
        blocks > (uint16_t)(INT_MAX / NEXUS_DGN_BLOCK_SIZE)) return -1;
    memset(&layout, 0, sizeof(layout));
    layout.structure1_offset = (int)block * NEXUS_DGN_BLOCK_SIZE;
    if (layout.structure1_offset + 0x38 > size ||
        (int)blocks * NEXUS_DGN_BLOCK_SIZE > size - layout.structure1_offset ||
        useful > (uint32_t)((int)blocks * NEXUS_DGN_BLOCK_SIZE)) return -1;
    layout.useful_size = (int)useful;
    layout.structure1b_relative_offset =
        (int)rb32(data + layout.structure1_offset + 0x14);
    if (layout.structure1b_relative_offset < 0 ||
        layout.structure1b_relative_offset > layout.useful_size ||
        layout.structure1b_relative_offset + NEXUS_DGN_STRUCTURE1B_BYTES >
            layout.useful_size) return -1;
    layout.structure1b_end_relative_offset =
        layout.structure1b_relative_offset + NEXUS_DGN_STRUCTURE1B_BYTES;
    layout.post_grid_offset = layout.structure1b_end_relative_offset;
    layout.post_grid_size = layout.useful_size - layout.post_grid_offset;
    for (i = 0; i < NEXUS_DGN_STRUCTURE1_POST_GRID_POINTER_COUNT; ++i) {
        Nexus_V1_DgnStructure1PostGridPointer *pointer = &layout.post_grid[i];
        int next = layout.useful_size;
        int j;
        pointer->header_offset = header_offsets[i];
        pointer->relative_offset =
            (int)rb32(data + layout.structure1_offset + pointer->header_offset);
        pointer->present = pointer->relative_offset != 0;
        if (!pointer->present) continue;
        if (pointer->relative_offset < layout.post_grid_offset ||
            pointer->relative_offset > layout.useful_size) return -1;
        for (j = 0; j < NEXUS_DGN_STRUCTURE1_POST_GRID_POINTER_COUNT; ++j) {
            int candidate = (int)rb32(data + layout.structure1_offset +
                                      header_offsets[j]);
            if (candidate > pointer->relative_offset && candidate < next)
                next = candidate;
        }
        pointer->size_to_next = next - pointer->relative_offset;
        pointer->bounded = 1;
    }
    {
        const Nexus_V1_DgnStructure1PostGridPointer *collision_span =
            &layout.post_grid[0];
        int record_count;

        if (collision_span->present && collision_span->bounded &&
            collision_span->size_to_next > 0 &&
            collision_span->size_to_next %
                NEXUS_DGN_GEOMETRY_DESCRIPTOR_MIN_BYTES == 0) {
            record_count = collision_span->size_to_next /
                           NEXUS_DGN_GEOMETRY_DESCRIPTOR_MIN_BYTES;
            if (data[layout.structure1_offset +
                     collision_span->relative_offset] ==
                    (uint8_t)record_count) {
                layout.structure1c.relative_offset =
                    collision_span->relative_offset;
                layout.structure1c.size = collision_span->size_to_next;
                layout.structure1c.record_size =
                    NEXUS_DGN_GEOMETRY_DESCRIPTOR_MIN_BYTES;
                layout.structure1c.record_count = record_count;
                layout.structure1c.indexed_record_count = record_count - 1;
                layout.structure1c.valid = 1;
            }
        }
    }
    {
        const Nexus_V1_DgnStructure1PostGridPointer *zero_span =
            &layout.post_grid[2];
        const Nexus_V1_DgnStructure1PostGridPointer *records =
            &layout.post_grid[4];
        int i;

        if (zero_span->present && zero_span->bounded &&
            zero_span->size_to_next == NEXUS_DGN_POST_GRID_0X24_ZERO_BYTES) {
            int all_zero = 1;
            for (i = 0; i < zero_span->size_to_next; ++i) {
                if (data[layout.structure1_offset + zero_span->relative_offset + i] != 0) {
                    all_zero = 0;
                    break;
                }
            }
            if (all_zero) {
                layout.post_grid_0x24_zero_span.relative_offset =
                    zero_span->relative_offset;
                layout.post_grid_0x24_zero_span.size = zero_span->size_to_next;
                layout.post_grid_0x24_zero_span.valid = 1;
            }
        }
        if (records->present && records->bounded && records->size_to_next > 0 &&
            records->size_to_next % NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES == 0) {
            unsigned char values[NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES][256];
            int record;
            int byte;
            layout.post_grid_0x30_records.relative_offset = records->relative_offset;
            layout.post_grid_0x30_records.size = records->size_to_next;
            layout.post_grid_0x30_records.record_size =
                NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES;
            layout.post_grid_0x30_records.record_count =
                records->size_to_next / NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES;
            layout.post_grid_0x30_records.opaque_tail_record_count = 1;
            layout.post_grid_0x30_records.typed_prefix_record_count =
                layout.post_grid_0x30_records.record_count - 1;
            layout.post_grid_0x30_records.first_row_ordinal_flagged_prefix_record =
                -1;
            layout.post_grid_0x30_records.last_row_ordinal_flagged_prefix_record =
                -1;
            memset(values, 0, sizeof(values));
            for (record = 0;
                 record < layout.post_grid_0x30_records.record_count;
                 ++record) {
                const uint8_t *src = data + layout.structure1_offset +
                    records->relative_offset +
                    record * NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES;
                for (byte = 0; byte < NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES;
                     ++byte) {
                    if (!values[byte][src[byte]]) {
                        values[byte][src[byte]] = 1U;
                        layout.post_grid_0x30_records
                            .field_distinct_value_count[byte]++;
                    }
                }
            }
            if (layout.post_grid_0x30_records.typed_prefix_record_count > 0) {
                int ordinal_valid = 1;
                for (record = 0;
                     record < layout.post_grid_0x30_records.typed_prefix_record_count;
                     ++record) {
                    const uint8_t *src = data + layout.structure1_offset +
                        records->relative_offset +
                        record * NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES;
                    uint8_t ordinal =
                        src[NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_BYTE];
                    if ((ordinal & NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_MASK) !=
                            (uint8_t)record ||
                        (ordinal & ~(NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_MASK |
                                     NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_FLAG_MASK)) != 0U) {
                        ordinal_valid = 0;
                        break;
                    }
                    if ((ordinal & NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_FLAG_MASK) !=
                        0U) {
                        layout.post_grid_0x30_records
                            .row_ordinal_flagged_prefix_record_count++;
                        if (layout.post_grid_0x30_records
                                .first_row_ordinal_flagged_prefix_record < 0) {
                            layout.post_grid_0x30_records
                                .first_row_ordinal_flagged_prefix_record = record;
                        }
                        layout.post_grid_0x30_records
                            .last_row_ordinal_flagged_prefix_record = record;
                    }
                }
                layout.post_grid_0x30_records.row_ordinal_prefix_valid =
                    ordinal_valid;
            }
            layout.post_grid_0x30_records.valid =
                layout.post_grid_0x30_records.row_ordinal_prefix_valid;
        }
    }
    /* DMWeb DGN files: pointer 0x34 is Structure1F, not an opaque tail.
     * Its counted families are decoded only when the entire final useful span
     * has an exact, source-backed layout. */
    (void)nexus_v1_dgn_parse_structure1a(&layout.structure1a, data, &layout);
    (void)nexus_v1_dgn_parse_structure1f(&layout.structure1f, data, &layout);
    (void)nexus_v1_dgn_parse_structure1g(&layout.structure1g, data, &layout);
    layout.valid = 1;
    *out_layout = layout;
    return 0;
}

int nexus_v1_dgn_geometry_info(Nexus_V1_DgnGeometryInfo *out_info,
                               const uint8_t *data,
                               int size) {
    Nexus_V1_DgnGeometryInfo info;
    uint16_t structure1_block;
    uint16_t structure1_blocks;
    uint32_t structure1_useful;
    int structure1_offset;
    int structure1_size;
    const uint8_t *structure1;
    uint32_t structure1b_rel;
    int structure1b_offset;
    int geometry_offset;
    int geometry_size;
    Nexus_V1_DgnStructure1Layout layout;
    unsigned char seen_refs[4096];
    unsigned char seen_post_grid_0x30_refs[4096];
    int y;
    int x;

    if (out_info) {
        memset(out_info, 0, sizeof(*out_info));
    }
    if (!out_info || !data || size < NEXUS_DGN_BLOCK_SIZE) {
        return -1;
    }

    memset(&info, 0, sizeof(info));
    structure1_block = rb16(data + 0x0C);
    structure1_blocks = rb16(data + 0x0E);
    structure1_useful = rb32(data + 0x10);
    if (structure1_block == 0 || structure1_blocks == 0 ||
        structure1_blocks > (uint16_t)(INT_MAX / NEXUS_DGN_BLOCK_SIZE) ||
        structure1_block > (uint16_t)(INT_MAX / NEXUS_DGN_BLOCK_SIZE)) {
        return -1;
    }

    structure1_offset = (int)structure1_block * NEXUS_DGN_BLOCK_SIZE;
    structure1_size = (int)structure1_blocks * NEXUS_DGN_BLOCK_SIZE;
    if (structure1_offset < NEXUS_DGN_BLOCK_SIZE ||
        structure1_offset + 0x38 > size ||
        structure1_size <= 0 ||
        structure1_offset + structure1_size > size ||
        structure1_useful > (uint32_t)structure1_size) {
        return -1;
    }

    structure1 = data + structure1_offset;
    structure1b_rel = rb32(structure1 + 0x14);
    if (structure1[2] != 0x40 || structure1[3] != 0x40 ||
        structure1b_rel > (uint32_t)structure1_size ||
        structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES > structure1_useful) {
        return -1;
    }

    structure1b_offset = structure1_offset + (int)structure1b_rel;
    geometry_offset = structure1b_offset + NEXUS_DGN_STRUCTURE1B_BYTES;
    geometry_size = (int)structure1_useful -
                    ((int)structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES);
    if (structure1b_offset < structure1_offset ||
        geometry_offset > size ||
        geometry_size < 0 ||
        geometry_offset + geometry_size > size) {
        return -1;
    }
    if (nexus_v1_dgn_structure1_layout(&layout, data, size) != 0) {
        return -1;
    }

    /*
     * DMWeb DGN Structure1B source-lock:
     * bytes 5..7 pack two 12-bit values; Firestaff's current renderer needs
     * the low collision descriptor reference to be bounded before a real
     * Structure1C/mesh reader can replace the procedural fallback.
     */
    memset(seen_refs, 0, sizeof(seen_refs));
    memset(seen_post_grid_0x30_refs, 0, sizeof(seen_post_grid_0x30_refs));
    for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
        for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
            int off = structure1b_offset +
                      ((y * NEXUS_MAX_MAP_SIZE + x) *
                       NEXUS_DGN_STRUCTURE1B_CELL_BYTES);
            int ref = nexus_v1_decode_structure1b_collision_ref(data + off);
            int post_grid_0x30_ref =
                nexus_v1_decode_structure1b_post_grid_0x30_ref(data + off);
            if (ref != 0 && ref != 0x0FFF) {
                info.collision_ref_count++;
                if (!seen_refs[ref]) {
                    seen_refs[ref] = 1U;
                    info.collision_ref_unique_count++;
                }
                if (ref > info.max_collision_ref) {
                    info.max_collision_ref = ref;
                }
            }
            if (post_grid_0x30_ref != 0 && post_grid_0x30_ref != 0x0FFF) {
                info.post_grid_0x30_ref_count++;
                if (!seen_post_grid_0x30_refs[post_grid_0x30_ref]) {
                    seen_post_grid_0x30_refs[post_grid_0x30_ref] = 1U;
                    info.post_grid_0x30_ref_unique_count++;
                }
                if (post_grid_0x30_ref > info.max_post_grid_0x30_ref) {
                    info.max_post_grid_0x30_ref = post_grid_0x30_ref;
                }
                if (!nexus_v1_post_grid_0x30_ref_is_bounded(
                        &layout, post_grid_0x30_ref)) {
                    info.post_grid_0x30_invalid_ref_count++;
                    if (info.first_invalid_post_grid_0x30_ref == 0) {
                        info.first_invalid_post_grid_0x30_ref =
                            post_grid_0x30_ref;
                    }
                }
            }
        }
    }

    info.dmweb_container = 1;
    info.structure1_offset = structure1_offset;
    info.structure1_size = structure1_size;
    info.structure1_useful_size = (int)structure1_useful;
    info.structure1b_offset = structure1b_offset;
    info.structure1b_size = NEXUS_DGN_STRUCTURE1B_BYTES;
    info.geometry_offset = geometry_offset;
    info.geometry_size = geometry_size;
    if (layout.structure1c.valid) {
        info.structure1c_offset = structure1_offset +
                                  layout.structure1c.relative_offset;
        info.structure1c_size = layout.structure1c.size;
        info.structure1c_record_count = layout.structure1c.record_count;
        info.structure1c_indexed_record_count =
            layout.structure1c.indexed_record_count;
        info.collision_records_valid =
            info.max_collision_ref < layout.structure1c.record_count;
    }
    info.post_grid_0x24_zero_span_valid =
        layout.post_grid_0x24_zero_span.valid;
    info.post_grid_0x24_zero_span_size = layout.post_grid_0x24_zero_span.size;
    info.post_grid_0x30_record_table_valid =
        layout.post_grid_0x30_records.valid;
    info.post_grid_0x30_record_count =
        layout.post_grid_0x30_records.record_count;
    info.post_grid_0x30_typed_prefix_record_count =
        layout.post_grid_0x30_records.typed_prefix_record_count;
    info.post_grid_0x30_opaque_tail_record_count =
        layout.post_grid_0x30_records.opaque_tail_record_count;
    info.post_grid_0x30_row_ordinal_prefix_valid =
        layout.post_grid_0x30_records.row_ordinal_prefix_valid;
    info.post_grid_0x30_row_ordinal_flagged_prefix_record_count =
        layout.post_grid_0x30_records.row_ordinal_flagged_prefix_record_count;
    info.post_grid_0x30_first_row_ordinal_flagged_prefix_record =
        layout.post_grid_0x30_records.first_row_ordinal_flagged_prefix_record;
    info.post_grid_0x30_last_row_ordinal_flagged_prefix_record =
        layout.post_grid_0x30_records.last_row_ordinal_flagged_prefix_record;
    info.post_grid_0x30_ref_value_count = info.post_grid_0x30_ref_count;
    info.structure1f_declared = layout.structure1f.declared;
    info.structure1f_valid = layout.structure1f.valid;
    info.structure1f_total_entry_count = layout.structure1f.total_entry_count;
    memcpy(info.structure1f_family_count, layout.structure1f.family_count,
           sizeof(info.structure1f_family_count));
    info.structure1g_present = layout.post_grid[1].present;
    info.structure1g_valid = layout.structure1g.valid;
    info.structure1g_animated_texture_count =
        layout.structure1g.animated_texture_count;
    info.structure1g_sequence_count = layout.structure1g.sequence_count;
    info.post_grid_0x30_references_valid =
        info.post_grid_0x30_record_table_valid &&
        info.post_grid_0x30_invalid_ref_count == 0;
    if (info.collision_records_valid &&
        info.post_grid_0x30_record_table_valid &&
        info.post_grid_0x30_row_ordinal_prefix_valid &&
        info.post_grid_0x30_references_valid) {
        info.mesh_ready = 1;
    }

    *out_info = info;
    return 0;
}

static void nexus_v1_level_copy_structure1g_entries(
    Nexus_V1_Level *level,
    const uint8_t *data,
    const Nexus_V1_DgnStructure1Layout *layout)
{
    const Nexus_V1_DgnStructure1GTable *table;
    const uint8_t *src;
    int descriptor;
    int output = 0;
    if (!level || !data || !layout || !layout->structure1g.valid) return;
    table = &layout->structure1g;
    src = data + layout->structure1_offset + table->relative_offset;
    for (descriptor = 0; descriptor < table->descriptor_count - 1; ++descriptor) {
        const uint8_t *entry = src + NEXUS_DGN_STRUCTURE1G_HEADER_BYTES +
            descriptor * NEXUS_DGN_STRUCTURE1G_DESCRIPTOR_BYTES;
        const int sequence_offset = table->animation_data_relative_offset +
            (int)rb16(entry + 6) * 4;
        int cursor;
        Nexus_V1_DgnStructure1GEntry *dst = &level->structure1g_entries[output++];
        dst->animation_id = entry[0];
        dst->first_image_index = rb16(entry + 4);
        if (dst->first_image_index >= NEXUS_DGN_STRUCTURE1G_FIRST_IMAGE_INDEX) {
            dst->first_structure2_image_id = (uint16_t)(
                dst->first_image_index - NEXUS_DGN_STRUCTURE1G_FIRST_IMAGE_INDEX);
            dst->first_structure2_image_valid =
                nexus_v1_level_find_structure2_texture(
                    level, dst->first_structure2_image_id);
        }
        dst->sequence_word_offset = rb16(entry + 6);
        for (cursor = sequence_offset; cursor <= table->size - 4; cursor += 4) {
            uint16_t instruction = rb16(src + cursor);
            dst->sequence_instruction_count++;
            if (instruction == 0xffffU) break;
            if (instruction == 0xfffeU) dst->goto_instruction_count++;
            else {
                uint16_t local_image_id;
                dst->image_instruction_count++;
                /* The parser accepts an instruction only after validating
                 * the Structure1G image-space lower bound. Keep a second
                 * defensive check here because level-load must never turn a
                 * malformed instruction into a local-table lookup. */
                if (instruction < NEXUS_DGN_STRUCTURE1G_FIRST_IMAGE_INDEX) {
                    dst->structure2_image_instruction_unbound_count++;
                    continue;
                }
                local_image_id = (uint16_t)(
                    instruction - NEXUS_DGN_STRUCTURE1G_FIRST_IMAGE_INDEX);
                if (nexus_v1_level_find_structure2_texture(level,
                                                            local_image_id)) {
                    dst->structure2_image_instruction_bound_count++;
                } else {
                    dst->structure2_image_instruction_unbound_count++;
                }
            }
        }
    }
    level->structure1g_entry_count = output;
}

static void nexus_v1_level_finalize_structure1g_structure2_bindings(
    Nexus_V1_Level *level)
{
    int entry;

    if (!level) return;
    if (!level->geometry_info.structure1g_present) {
        level->structure1g_structure2_bindings_complete = 1;
        return;
    }

    /* DMWeb DGN Structure1G uses the global image space while Structure2
     * owns the local descriptor table. Keep the already proven
     * global-to-local relation whole: one unresolved instruction means this
     * level cannot claim a usable animated-material declaration. This does
     * not decode the opaque payload or make any material drawable. */
    level->structure1g_structure2_bindings_complete =
        level->structure2_texture_table_valid &&
        level->structure2_payload.valid &&
        level->structure2_payload.descriptor_offset_envelope_valid &&
        level->structure1g_entry_count > 0;
    for (entry = 0;
         entry < level->structure1g_entry_count &&
         level->structure1g_structure2_bindings_complete;
         ++entry) {
        const Nexus_V1_DgnStructure1GEntry *declaration =
            &level->structure1g_entries[entry];
        if (!declaration->first_structure2_image_valid ||
            declaration->structure2_image_instruction_unbound_count != 0) {
            level->structure1g_structure2_bindings_complete = 0;
        }
    }
}

static void nexus_v1_level_bind_structure3_face_materials(
    Nexus_V1_Level *level, const uint8_t *data, int size)
{
    Nexus_V1_DgnStructure3FaceMaterialReceipt receipt;
    uint8_t static_selector_seen[256];
    uint8_t animated_selector_seen[256];
    int entry;

    if (!level || !data || size <= 0) return;
    memset(&receipt, 0, sizeof(receipt));
    memset(static_selector_seen, 0, sizeof(static_selector_seen));
    memset(animated_selector_seen, 0, sizeof(animated_selector_seen));
    receipt.face_receipt_valid = level->structure3_faces.valid;
    receipt.face_count = level->structure3_faces.face_count;
    receipt.valid = receipt.face_receipt_valid &&
        level->structure3_directory.valid &&
        level->structure3_entry_headers.valid &&
        level->structure3_payload.valid;
    if (!receipt.valid) {
        level->structure3_face_materials = receipt;
        return;
    }

    /* DMWeb DGN Structure3b: flags bit 6 selects texture filling; 00xx
     * selects a local Structure2 descriptor and 08xx selects a Structure1G
     * animated-texture declaration. This is a bounded identifier join only. */
    for (entry = 0; entry < level->structure3_directory.entry_count; ++entry) {
        uint32_t entry_offset = rb32(data + level->structure3_payload.byte_offset +
                                     4 + entry * 4);
        const uint8_t *header = data + level->structure3_payload.byte_offset +
            entry_offset;
        uint16_t face_count = rb16(header + 6);
        uint32_t face_offset = rb32(header + 16);
        int face_index;

        for (face_index = 0; face_index < (int)face_count; ++face_index) {
            const uint8_t *face = data + level->structure3_payload.byte_offset +
                face_offset + face_index * 12;
            uint16_t fill = rb16(face + 10);

            if ((face[8] & 0x40U) == 0U) {
                ++receipt.non_textured_face_count;
            } else if ((fill & 0xff00U) == 0U) {
                uint8_t selector = (uint8_t)(fill & 0xffU);

                ++receipt.textured_face_count;
                ++receipt.static_texture_selector_count;
                if (static_selector_seen[selector])
                    ++receipt.static_texture_reused_selector_count;
                else {
                    static_selector_seen[selector] = 1;
                    ++receipt.static_texture_unique_selector_count;
                }
                if (nexus_v1_level_find_structure2_texture(level, selector))
                    ++receipt.static_texture_bound_count;
                else
                    ++receipt.static_texture_unbound_count;
            } else if ((fill & 0xff00U) == 0x0800U) {
                uint8_t selector = (uint8_t)(fill & 0xffU);
                int animation;
                int bound = 0;

                ++receipt.textured_face_count;
                ++receipt.animated_texture_selector_count;
                if (animated_selector_seen[selector])
                    ++receipt.animated_texture_reused_selector_count;
                else {
                    animated_selector_seen[selector] = 1;
                    ++receipt.animated_texture_unique_selector_count;
                }
                for (animation = 0; animation < level->structure1g_entry_count;
                     ++animation) {
                    if (level->structure1g_entries[animation].animation_id ==
                        selector) {
                        bound = 1;
                        break;
                    }
                }
                if (bound) ++receipt.animated_texture_bound_count;
                else ++receipt.animated_texture_unbound_count;
            } else {
                ++receipt.textured_face_count;
                ++receipt.unsupported_textured_fill_count;
            }
        }
    }
    receipt.selector_bindings_complete =
        receipt.textured_face_count == level->structure3_faces.textured_face_count &&
        receipt.static_texture_unbound_count == 0 &&
        receipt.animated_texture_unbound_count == 0 &&
        receipt.unsupported_textured_fill_count == 0;
    receipt.selector_reuse_accounting_valid =
        receipt.static_texture_unique_selector_count +
                receipt.static_texture_reused_selector_count ==
            receipt.static_texture_selector_count &&
        receipt.animated_texture_unique_selector_count +
                receipt.animated_texture_reused_selector_count ==
            receipt.animated_texture_selector_count;
    receipt.material_or_draw_semantics_proven = 0;
    level->structure3_face_materials = receipt;
}

static void nexus_v1_level_copy_structure1f_entries(
    Nexus_V1_Level *level,
    const uint8_t *data,
    const Nexus_V1_DgnStructure1Layout *layout)
{
    int family;
    int output = 0;
    if (!level || !data || !layout || !layout->structure1f.valid) return;
    for (family = 0; family < NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT; ++family) {
        const int count = layout->structure1f.family_count[family];
        const int record_size = layout->structure1f.family_record_size[family];
        const uint8_t *records = data + layout->structure1_offset +
            layout->structure1f.family_offset[family];
        int record;
        for (record = 0; record < count; ++record) {
            const uint8_t *src = records + record * record_size;
            Nexus_V1_DgnStructure1FEntry *dst =
                &level->structure1f_entries[output++];
            const size_t raw_offset = (size_t)(src - data);
            if (raw_offset > UINT32_MAX || record_size <= 0 ||
                raw_offset > SIZE_MAX - (size_t)record_size) {
                --output;
                level->structure1f_entry_count = 0;
                return;
            }
            dst->family = (Nexus_V1_DgnStructure1FFamily)family;
            dst->raw_record_offset = (uint32_t)raw_offset;
            dst->raw_record_length = (uint32_t)record_size;
            dst->raw_record_fnv1a64 = nexus_v1_fnv1a64(src, record_size);
            dst->tag = src[0];
            switch (family) {
            case NEXUS_V1_DGN_STRUCTURE1F_ITEMS:
                dst->x = src[1]; dst->y = src[2]; dst->location = src[3];
                dst->item_id = src[4]; dst->attribute1 = src[5];
                dst->attribute2 = src[7];
                break;
            case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS:
                dst->x = src[1]; dst->y = src[2];
                dst->offset_x = (int8_t)src[3]; dst->offset_y = (int8_t)src[4];
                dst->model_or_aspect = src[5]; dst->rotation = src[6];
                dst->type_or_control = src[7]; dst->width = src[8];
                dst->height = src[9];
                break;
            case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS:
                dst->x = src[1]; dst->y = src[2];
                dst->model_or_aspect = src[5]; dst->rotation = src[6];
                dst->width = src[10]; dst->height = src[11];
                dst->type_or_control = src[12]; dst->destination_x = src[13];
                dst->destination_y = src[14]; dst->destination_orientation = src[15];
                break;
            case NEXUS_V1_DGN_STRUCTURE1F_ALCOVES:
                dst->face = src[1]; dst->structure1a_index = rb16(src + 2);
                dst->rotation = src[4]; dst->offset_x = (int8_t)src[5];
                dst->offset_y = (int8_t)src[6]; dst->item_id = src[7];
                break;
            case NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS:
                dst->face = src[1]; dst->structure1a_index = rb16(src + 2);
                dst->rotation = src[4]; dst->offset_x = (int8_t)src[5];
                dst->offset_y = (int8_t)src[6]; dst->model_or_aspect = src[7];
                break;
            case NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS:
                dst->face = src[1]; dst->structure1a_index = rb16(src + 2);
                dst->rotation = src[4]; dst->offset_x = (int8_t)src[5];
                dst->offset_y = (int8_t)src[6]; dst->model_or_aspect = src[7];
                dst->type_or_control = src[12]; dst->destination_x = src[13];
                dst->destination_y = src[14]; dst->destination_orientation = src[15];
                break;
            }
        }
    }
    level->structure1f_entry_count = output;
}

static void nexus_v1_level_copy_structure1a_models(
    Nexus_V1_Level *level,
    const uint8_t *data,
    const Nexus_V1_DgnStructure1Layout *layout)
{
    int index;

    if (!level || !data || !layout || !layout->structure1a.valid) return;
    for (index = 0; index < layout->structure1a.entry_count; ++index) {
        const uint8_t *src = data + layout->structure1_offset +
            layout->structure1a.relative_offset +
            index * NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES;
        level->structure1a_models[index].kind = src[0];
        level->structure1a_models[index].structure3_model_index = src[1];
        level->structure1a_models[index].z_rotation = src[2];
    }
    level->structure1a_model_count = layout->structure1a.entry_count;
    level->structure1a_table_valid = 1;
}

static void nexus_v1_level_resolve_structure1a_relations(Nexus_V1_Level *level)
{
    int owner_count[NEXUS_DGN_MAX_STRUCTURE1A_ENTRIES];
    int owner_x[NEXUS_DGN_MAX_STRUCTURE1A_ENTRIES];
    int owner_y[NEXUS_DGN_MAX_STRUCTURE1A_ENTRIES];
    int x;
    int y;
    int entry;

    if (!level || !level->structure1a_table_valid) return;
    memset(owner_count, 0, sizeof(owner_count));
    memset(owner_x, 0, sizeof(owner_x));
    memset(owner_y, 0, sizeof(owner_y));
    for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
        for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
            uint16_t ref;
            if (!level->structure1a_owner_ref_valid[y][x]) continue;
            ref = level->structure1a_owner_refs[y][x];
            if (ref >= (uint16_t)level->structure1a_model_count) continue;
            ++owner_count[ref];
            owner_x[ref] = x;
            owner_y[ref] = y;
        }
    }
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        Nexus_V1_DgnStructure1FEntry *record = &level->structure1f_entries[entry];
        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES ||
            record->structure1a_index >= (uint16_t)level->structure1a_model_count ||
            owner_count[record->structure1a_index] != 1) continue;
        record->structure1a_relation_valid = 1;
        record->structure1a_owner_x = owner_x[record->structure1a_index];
        record->structure1a_owner_y = owner_y[record->structure1a_index];
        record->structure1a_structure3_model_index =
            level->structure1a_models[record->structure1a_index]
                .structure3_model_index;
        record->structure1a_z_rotation =
            level->structure1a_models[record->structure1a_index].z_rotation;
    }
}

int nexus_v1_level_load(Nexus_V1_Level *level, const uint8_t *data, int size, int level_index) {
    if (!level || !data || size < 64) return -1;
    memset(level, 0, sizeof(*level));

    /*
     * DMWeb source-lock:
     *   http://dmweb.free.fr/community/documentation/dungeon-master-nexus/dgn-files/
     *   DGN files are 2048-byte block containers. Header offsets at 0x0C,
     *   0x0E and 0x10 locate Structure1; Structure1 offset 0x14 locates
     *   Structure1B, always 0x8000 bytes: 64x64 cells, 8 bytes each.
     */

    if (size >= NEXUS_DGN_BLOCK_SIZE) {
        Nexus_V1_DgnGeometryInfo info;
        if (nexus_v1_dgn_geometry_info(&info, data, size) == 0) {
            int y, x;
            Nexus_V1_DgnStructure1Layout layout;
            level->width = NEXUS_MAX_MAP_SIZE;
            level->height = NEXUS_MAX_MAP_SIZE;
            for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
                for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
                    int off = info.structure1b_offset +
                              ((y * NEXUS_MAX_MAP_SIZE + x) *
                               NEXUS_DGN_STRUCTURE1B_CELL_BYTES);
                    int ref = nexus_v1_decode_structure1b_collision_ref(data + off);
                    level->squares[y][x] = (uint8_t)nexus_v1_decode_structure1b_cell(data + off);
                    level->collision_refs[y][x] = (uint16_t)ref;
                    level->floor_material_refs[y][x] =
                        nexus_v1_decode_structure1b_floor_material(data + off);
                    level->floor_animation_ids[y][x] = 0xffU;
                    if (nexus_v1_decode_structure1b_floor_animation_id(
                            data + off, &level->floor_animation_ids[y][x])) {
                        int entry;
                        level->structure1g_floor_animation_cell_count++;
                        for (entry = 0; entry < level->structure1g_entry_count;
                             ++entry) {
                            if (level->structure1g_entries[entry].animation_id ==
                                level->floor_animation_ids[y][x]) {
                                level->structure1g_floor_animation_bound_count++;
                                break;
                            }
                        }
                    }
                    level->ceiling_material_refs[y][x] =
                        nexus_v1_decode_structure1b_ceiling_material(
                            data + info.structure1_offset, data + off);
                    level->wall_material_refs[y][x][0] =
                        nexus_v1_decode_structure1b_wall_material(data + off, 0);
                    level->wall_material_refs[y][x][1] =
                        nexus_v1_decode_structure1b_wall_material(data + off, 1);
                    level->wall_material_refs[y][x][2] =
                        nexus_v1_decode_structure1b_wall_material(data + off, 2);
                    level->wall_material_refs[y][x][3] =
                        nexus_v1_decode_structure1b_wall_material(data + off, 3);
                    level->floor_heights[y][x] = (int8_t)data[off + 3];
                    level->floor_slopes[y][x] =
                        (uint8_t)((rb16(data + off) >> 4) & 3U);
                    level->floor_rotations[y][x] =
                        (uint8_t)(rb16(data + off) >> 14);
                    level->post_grid_0x30_refs[y][x] =
                        nexus_v1_decode_structure1b_post_grid_0x30_ref(data + off);
                    level->structure1a_owner_ref_valid[y][x] =
                        nexus_v1_decode_structure1b_structure1a_ref(
                            data + off, &level->structure1a_owner_refs[y][x]) ? 1U : 0U;
                }
            }
            /* DMWeb proves Structure1B's 12-bit reference into a bounded
             * Structure1C record table, but not the four record bytes as
             * line/circle coordinates. Keep those original references for
             * later evidence; never turn opaque bytes into collision shapes. */
            level->has_3d_geometry = 1;
            level->geometry_offset = info.geometry_offset;
            /* Structure1's useful-size field ends the DGN geometry span.
             * Later container blocks can hold Structure2/3 data and must not
             * be exposed through the Structure1 geometry boundary. */
            level->geometry_size = info.geometry_size;
            level->geometry_info = info;
            if (nexus_v1_dgn_structure1_layout(&layout, data, size) == 0) {
                if (nexus_v1_level_copy_structure2_textures(level, data, size) != 0) {
                    level->structure2_texture_count = 0;
                    level->structure2_texture_table_valid = 0;
                }
                if (nexus_v1_level_copy_structure3_payload(level, data, size) != 0) {
                    return -1;
                }
                if (level->structure3_directory.valid &&
                    level->structure3_directory.entry_count >= 0 &&
                    level->structure3_directory.entry_count <=
                        NEXUS_DGN_MAX_STRUCTURE3_ENTRIES) {
                    int structure3_entry;
                    for (structure3_entry = 0;
                         structure3_entry < level->structure3_directory.entry_count;
                         ++structure3_entry) {
                        uint32_t entry_offset = rb32(data +
                            level->structure3_payload.byte_offset + 4 +
                            structure3_entry * 4);
                        level->structure3_entry_face_counts[structure3_entry] =
                            rb16(data + level->structure3_payload.byte_offset +
                                 entry_offset + 6);
                    }
                }
                nexus_v1_level_copy_structure1a_models(level, data, &layout);
                nexus_v1_level_copy_structure1f_entries(level, data, &layout);
                nexus_v1_level_resolve_structure1a_relations(level);
                nexus_v1_level_copy_structure1g_entries(level, data, &layout);
                nexus_v1_level_finalize_structure1g_structure2_bindings(level);
                nexus_v1_level_bind_structure3_face_materials(level, data, size);
                level->structure1g_floor_animation_bound_count = 0;
                for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
                    for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
                        int entry;
                        if (level->floor_animation_ids[y][x] == 0xffU) continue;
                        for (entry = 0; entry < level->structure1g_entry_count;
                             ++entry) {
                            if (level->structure1g_entries[entry].animation_id ==
                                level->floor_animation_ids[y][x]) {
                                level->structure1g_floor_animation_bound_count++;
                                break;
                            }
                        }
                    }
                }
            }
            printf("Nexus level %d: 64x64 Structure1B, payload=%d bytes, mesh_span=%d bytes, refs=%d/%d [DMWeb DGN]\n",
                   level_index, level->geometry_size, info.geometry_size,
                   info.collision_ref_unique_count, info.collision_ref_count);
            return 0;
        }
    }

    printf("Nexus level %d: could not parse DGN header (size=%d)\n",
           level_index, size);
    return -1;
}

int nexus_v1_level_get_square(const Nexus_V1_Level *level, int x, int y) {
    if (!level || x < 0 || x >= level->width || y < 0 || y >= level->height)
        return 0; /* wall */
    return level->squares[y][x];
}

int nexus_v1_level_get_collision_ref(const Nexus_V1_Level *level, int x, int y) {
    if (!level || x < 0 || x >= level->width || y < 0 || y >= level->height)
        return 0x0fff;
    return (int)level->collision_refs[y][x];
}

int nexus_v1_level_get_material_ref(const Nexus_V1_Level *level, int x, int y,
                                    Nexus_V1_DgnRenderCommandKind kind,
                                    int wall_dir) {
    if (!level || x < 0 || x >= level->width || y < 0 || y >= level->height)
        return -1;
    if (kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR ||
        kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING) {
        if (kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING)
            return level->ceiling_material_refs[y][x];
        return level->floor_material_refs[y][x];
    }
    return level->wall_material_refs[y][x][wall_dir & 3];
}

int nexus_v1_level_get_cell_geometry(const Nexus_V1_Level *level, int x, int y,
                                     Nexus_V1_DgnCellGeometry *out_cell) {
    Nexus_V1_DgnCellGeometry cell;
    int corner;

    if (!out_cell) return -1;
    memset(out_cell, 0, sizeof(*out_cell));
    if (!level || x < 0 || x >= level->width || y < 0 || y >= level->height)
        return -1;

    memset(&cell, 0, sizeof(cell));
    cell.square_type = level->squares[y][x];
    cell.collision_ref = level->collision_refs[y][x];
    cell.post_grid_0x30_ref = level->post_grid_0x30_refs[y][x];
    cell.floor_material_ref = level->floor_material_refs[y][x];
    cell.ceiling_material_ref = level->ceiling_material_refs[y][x];
    memcpy(cell.wall_material_refs, level->wall_material_refs[y][x],
           sizeof(cell.wall_material_refs));
    cell.floor_slope = level->floor_slopes[y][x];
    cell.floor_rotation = level->floor_rotations[y][x];
    for (corner = 0; corner < 4; ++corner)
        cell.floor_height[corner] = level->floor_heights[y][x];
    if (cell.floor_slope == 2U && x + 1 < level->width) {
        cell.floor_height[1] = cell.floor_height[2] =
            level->floor_heights[y][x + 1];
    } else if (cell.floor_slope == 3U && y + 1 < level->height) {
        cell.floor_height[2] = cell.floor_height[3] =
            level->floor_heights[y + 1][x];
    }
    for (corner = 0; corner < 4; ++corner)
        cell.ceiling_height[corner] = (int8_t)(cell.floor_height[corner] + 32);
    if (cell.collision_ref < NEXUS_DGN_MAX_COLLISION_SECTORS)
        cell.collision_sector = level->collision_sectors[cell.collision_ref];
    cell.post_grid_0x30_row_prefix_valid =
        level->geometry_info.post_grid_0x30_row_ordinal_prefix_valid &&
        (cell.post_grid_0x30_ref == 0 ||
         cell.post_grid_0x30_ref == 0x0fffU ||
         cell.post_grid_0x30_ref <
             (uint16_t)level->geometry_info
                 .post_grid_0x30_typed_prefix_record_count);
    *out_cell = cell;
    return 0;
}

int nexus_v1_level_structure1f_spatial_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FSpatialReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FSpatialReceipt receipt;
    unsigned char direct_cell_seen[NEXUS_MAX_MAP_SIZE * NEXUS_MAX_MAP_SIZE];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(direct_cell_seen, 0, sizeof(direct_cell_seen));
    if (!level || !level->geometry_info.structure1f_valid ||
        level->structure1f_entry_count < 0 ||
        level->structure1f_entry_count !=
            level->geometry_info.structure1f_total_entry_count) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.typed_entry_count = level->structure1f_entry_count;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        switch (record->family) {
        case NEXUS_V1_DGN_STRUCTURE1F_ITEMS:
            ++receipt.item_entry_count;
            break;
        case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS:
            ++receipt.floor_decoration_entry_count;
            break;
        case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS:
            ++receipt.floor_sensor_entry_count;
            break;
        case NEXUS_V1_DGN_STRUCTURE1F_ALCOVES:
        case NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS:
        case NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS:
            ++receipt.structure1a_bound_entry_count;
            continue;
        default:
            *out_receipt = receipt;
            return 0;
        }
        if (record->x >= (uint8_t)level->width ||
            record->y >= (uint8_t)level->height) {
            *out_receipt = receipt;
            return 0;
        }
        {
            int cell = (int)record->y * NEXUS_MAX_MAP_SIZE + record->x;
            if (direct_cell_seen[cell]) {
                ++receipt.direct_coordinate_duplicate_cell_count;
            } else {
                direct_cell_seen[cell] = 1;
                ++receipt.direct_coordinate_unique_cell_count;
            }
        }
        ++receipt.direct_coordinate_entry_count;
    }
    receipt.valid = receipt.typed_entry_count ==
        receipt.direct_coordinate_entry_count +
            receipt.structure1a_bound_entry_count;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1a_boundary_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1ABoundaryReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1ABoundaryReceipt receipt;
    unsigned char index_seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(index_seen, 0, sizeof(index_seen));
    if (!level || !level->geometry_info.structure1f_valid ||
        level->structure1f_entry_count < 0 ||
        level->structure1f_entry_count !=
            level->geometry_info.structure1f_total_entry_count) {
        *out_receipt = receipt;
        return 0;
    }
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        switch (record->family) {
        case NEXUS_V1_DGN_STRUCTURE1F_ALCOVES:
            ++receipt.alcove_entry_count;
            break;
        case NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS:
            ++receipt.wall_decoration_entry_count;
            break;
        case NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS:
            ++receipt.wall_sensor_entry_count;
            break;
        default:
            continue;
        }
        ++receipt.entry_count;
        if (record->structure1a_index == 0U) {
            ++receipt.zero_index_count;
        } else {
            ++receipt.nonzero_index_count;
        }
        if (index_seen[record->structure1a_index]) {
            ++receipt.duplicate_index_count;
        } else {
            index_seen[record->structure1a_index] = 1;
            ++receipt.unique_index_count;
        }
        if (record->structure1a_index > receipt.highest_index) {
            receipt.highest_index = record->structure1a_index;
        }
    }
    receipt.valid = receipt.entry_count ==
        receipt.alcove_entry_count + receipt.wall_decoration_entry_count +
        receipt.wall_sensor_entry_count &&
        receipt.entry_count == receipt.zero_index_count +
        receipt.nonzero_index_count &&
        receipt.entry_count == receipt.unique_index_count +
        receipt.duplicate_index_count;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1a_relation_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1ARelationReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1ARelationReceipt receipt;
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    if (!level || !level->geometry_info.structure1f_valid) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.table_entry_count = level->structure1a_model_count;
    receipt.table_valid = level->structure1a_table_valid &&
        receipt.table_entry_count >= 0 &&
        receipt.table_entry_count <= NEXUS_DGN_MAX_STRUCTURE1A_ENTRIES;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.structure1f_bound_entry_count;
        if (!receipt.table_valid ||
            record->structure1a_index >= (uint16_t)receipt.table_entry_count) {
            ++receipt.out_of_range_index_count;
        } else {
            int x;
            int y;
            int owners = 0;
            int owner_matches_cached_relation = 0;
            for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
                for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
                    if (level->structure1a_owner_ref_valid[y][x] &&
                        level->structure1a_owner_refs[y][x] ==
                            record->structure1a_index) ++owners;
                }
            }
            if (record->structure1a_relation_valid &&
                record->structure1a_owner_x >= 0 &&
                record->structure1a_owner_x < NEXUS_MAX_MAP_SIZE &&
                record->structure1a_owner_y >= 0 &&
                record->structure1a_owner_y < NEXUS_MAX_MAP_SIZE &&
                level->structure1a_owner_ref_valid
                    [record->structure1a_owner_y]
                    [record->structure1a_owner_x] &&
                level->structure1a_owner_refs
                    [record->structure1a_owner_y]
                    [record->structure1a_owner_x] ==
                    record->structure1a_index &&
                level->structure1a_models[record->structure1a_index]
                    .structure3_model_index ==
                    record->structure1a_structure3_model_index &&
                level->structure1a_models[record->structure1a_index]
                    .z_rotation == record->structure1a_z_rotation) {
                owner_matches_cached_relation = 1;
            }
            if (owners == 1 && owner_matches_cached_relation) {
                ++receipt.resolved_entry_count;
            } else if (owners == 0) ++receipt.missing_owner_entry_count;
            else ++receipt.ambiguous_owner_entry_count;
        }
    }
    receipt.complete = receipt.table_valid &&
        receipt.structure1f_bound_entry_count == receipt.resolved_entry_count;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1a_kind_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1AKindReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1AKindReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint8_t kind;

        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.structure1f_bound_entry_count;
        if (!record->structure1a_relation_valid ||
            record->structure1a_index >=
                (uint16_t)level->structure1a_model_count) {
            continue;
        }
        kind = level->structure1a_models[record->structure1a_index].kind;
        ++receipt.resolved_kind_count;
        if (kind == 0U) ++receipt.zero_kind_count;
        else ++receipt.nonzero_kind_count;
        if (kind > receipt.highest_kind) receipt.highest_kind = kind;
        if (seen[kind]) ++receipt.duplicate_kind_count;
        else {
            seen[kind] = 1U;
            ++receipt.unique_kind_count;
        }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_kind_count == receipt.structure1f_bound_entry_count;
    receipt.kind_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure3_model_reference_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3ModelReferenceReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3ModelReferenceReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint8_t model_index;

        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.structure1f_bound_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_model_reference_count;
        model_index = record->structure1a_structure3_model_index;
        if (model_index == 0U) ++receipt.zero_model_index_count;
        else ++receipt.nonzero_model_index_count;
        if (seen[model_index]) {
            ++receipt.duplicate_model_index_count;
        } else {
            seen[model_index] = 1U;
            ++receipt.unique_model_index_count;
        }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_model_reference_count ==
            receipt.structure1f_bound_entry_count;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1a_transform_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1ATransformSelectorReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1ATransformSelectorReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint8_t selector;
        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.structure1f_bound_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_selector_count;
        selector = record->structure1a_z_rotation;
        if (selector == 0U) ++receipt.zero_selector_count;
        else ++receipt.nonzero_selector_count;
        if (selector > receipt.highest_selector) receipt.highest_selector = selector;
        if (seen[selector]) ++receipt.duplicate_selector_count;
        else { seen[selector] = 1U; ++receipt.unique_selector_count; }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_selector_count == receipt.structure1f_bound_entry_count;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1a_transform_table_receipt(
    const Nexus_V1_Level *level, const uint8_t *dgn_data, int dgn_size,
    Nexus_V1_DgnStructure1ATransformTableReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1ATransformTableReceipt receipt;
    const uint8_t *table;
    int structure1_offset;
    uint32_t count;
    uint32_t relative_offset;
    int byte_count;
    int index;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    if (!level || !dgn_data || dgn_size <= 0 || !level->structure1a_table_valid ||
        level->structure1a_model_count < 0 ||
        level->structure1a_model_count > NEXUS_DGN_MAX_STRUCTURE1A_ENTRIES) {
        *out_receipt = receipt;
        return 0;
    }
    structure1_offset = level->geometry_info.structure1_offset;
    if (structure1_offset < 0 || structure1_offset > dgn_size - 0x14) {
        *out_receipt = receipt;
        return 0;
    }
    count = rb32(dgn_data + structure1_offset + 0x0c);
    relative_offset = rb32(dgn_data + structure1_offset + 0x10);
    if (relative_offset != 0x38U ||
        count != (uint32_t)level->structure1a_model_count ||
        count > (uint32_t)(INT_MAX / NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES)) {
        *out_receipt = receipt;
        return 0;
    }
    byte_count = (int)count * NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES;
    if ((int)relative_offset > dgn_size - structure1_offset - byte_count) {
        *out_receipt = receipt;
        return 0;
    }
    table = dgn_data + structure1_offset + relative_offset;
    for (index = 0; index < (int)count; ++index) {
        const uint8_t *row = table + index * NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES;
        const Nexus_V1_DgnStructure1AModel *model =
            &level->structure1a_models[index];
        if (row[0] != model->kind ||
            row[1] != model->structure3_model_index ||
            row[2] != model->z_rotation) {
            *out_receipt = receipt;
            return 0;
        }
    }
    if (nexus_v1_level_structure1a_relation_receipt(level, &receipt.relation) != 0 ||
        nexus_v1_level_structure1a_transform_selector_receipt(
            level, &receipt.selectors) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.table_byte_offset = structure1_offset + (int)relative_offset;
    receipt.entry_count = (int)count;
    receipt.table_byte_count = byte_count;
    receipt.raw_table_fnv1a64 = nexus_v1_fnv1a64(table, byte_count);
    receipt.selector_column_fnv1a64 = UINT64_C(1469598103934665603);
    for (index = 0; index < (int)count; ++index) {
        receipt.selector_column_fnv1a64 ^=
            table[index * NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES + 2];
        receipt.selector_column_fnv1a64 *= UINT64_C(1099511628211);
    }
    receipt.parsed_model_rows_match = 1;
    receipt.source_table_bound = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_level_structure1f_face_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFaceSelectorReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFaceSelectorReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint8_t selector;

        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.structure1f_bound_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_face_selector_count;
        selector = record->face;
        if (selector == 0U) ++receipt.zero_face_selector_count;
        else ++receipt.nonzero_face_selector_count;
        if (selector > receipt.highest_face_selector)
            receipt.highest_face_selector = selector;
        if (seen[selector]) ++receipt.duplicate_face_selector_count;
        else { seen[selector] = 1U; ++receipt.unique_face_selector_count; }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_face_selector_count == receipt.structure1f_bound_entry_count;
    receipt.face_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure3_model_face_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3ModelFaceSelectorReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3ModelFaceSelectorReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint16_t pair;

        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.structure1f_bound_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_pair_count;
        pair = (uint16_t)(((uint16_t)record->structure1a_structure3_model_index
                           << 8) | record->face);
        if (pair == 0U) ++receipt.zero_pair_count;
        else ++receipt.nonzero_pair_count;
        if (pair > receipt.highest_pair) receipt.highest_pair = pair;
        if (seen[pair]) ++receipt.duplicate_pair_count;
        else { seen[pair] = 1U; ++receipt.unique_pair_count; }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_pair_count == receipt.structure1f_bound_entry_count;
    receipt.attachment_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_rotation_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FRotationSelectorReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FRotationSelectorReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint8_t selector;

        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.structure1f_bound_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_rotation_selector_count;
        selector = record->rotation;
        if (selector == 0U) ++receipt.zero_rotation_selector_count;
        else ++receipt.nonzero_rotation_selector_count;
        if (selector > receipt.highest_rotation_selector)
            receipt.highest_rotation_selector = selector;
        if (seen[selector]) ++receipt.duplicate_rotation_selector_count;
        else { seen[selector] = 1U; ++receipt.unique_rotation_selector_count; }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_rotation_selector_count == receipt.structure1f_bound_entry_count;
    receipt.rotation_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_face_rotation_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFaceRotationPairReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFaceRotationPairReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint16_t pair;

        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.structure1f_bound_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_pair_count;
        pair = (uint16_t)(((uint16_t)record->face << 8) | record->rotation);
        if (pair == 0U) ++receipt.zero_pair_count;
        else ++receipt.nonzero_pair_count;
        if (pair > receipt.highest_pair) receipt.highest_pair = pair;
        if (seen[pair]) ++receipt.duplicate_pair_count;
        else { seen[pair] = 1U; ++receipt.unique_pair_count; }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_pair_count == receipt.structure1f_bound_entry_count;
    receipt.pair_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_offset_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FOffsetPairReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FOffsetPairReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint16_t pair;

        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.structure1f_bound_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_offset_pair_count;
        pair = (uint16_t)(((uint16_t)(uint8_t)record->offset_x << 8) |
                          (uint8_t)record->offset_y);
        if (record->offset_x == 0 && record->offset_y == 0)
            ++receipt.zero_offset_pair_count;
        else
            ++receipt.nonzero_offset_pair_count;
        if (receipt.resolved_offset_pair_count == 1) {
            receipt.minimum_offset_x = receipt.maximum_offset_x = record->offset_x;
            receipt.minimum_offset_y = receipt.maximum_offset_y = record->offset_y;
        } else {
            if (record->offset_x < receipt.minimum_offset_x)
                receipt.minimum_offset_x = record->offset_x;
            if (record->offset_x > receipt.maximum_offset_x)
                receipt.maximum_offset_x = record->offset_x;
            if (record->offset_y < receipt.minimum_offset_y)
                receipt.minimum_offset_y = record->offset_y;
            if (record->offset_y > receipt.maximum_offset_y)
                receipt.maximum_offset_y = record->offset_y;
        }
        if (seen[pair]) ++receipt.duplicate_offset_pair_count;
        else { seen[pair] = 1U; ++receipt.unique_offset_pair_count; }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_offset_pair_count == receipt.structure1f_bound_entry_count;
    receipt.offset_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_wall_payload_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FWallPayloadSelectorReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FWallPayloadSelectorReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint8_t selector;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS &&
            record->family != NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS) continue;
        ++receipt.wall_payload_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_payload_selector_count;
        if (record->family == NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS)
            ++receipt.wall_decoration_selector_count;
        else
            ++receipt.wall_sensor_selector_count;
        selector = record->model_or_aspect;
        if (selector == 0U) ++receipt.zero_payload_selector_count;
        else ++receipt.nonzero_payload_selector_count;
        if (selector > receipt.highest_payload_selector)
            receipt.highest_payload_selector = selector;
        if (seen[selector]) ++receipt.duplicate_payload_selector_count;
        else { seen[selector] = 1U; ++receipt.unique_payload_selector_count; }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_payload_selector_count == receipt.wall_payload_entry_count;
    receipt.payload_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_wall_sensor_destination_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FWallSensorDestinationReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FWallSensorDestinationReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        int previous;
        int duplicate = 0;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS) continue;
        ++receipt.wall_sensor_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_destination_count;
        if (record->destination_x == 0U && record->destination_y == 0U &&
            record->destination_orientation == 0U) {
            ++receipt.zero_destination_count;
        } else {
            ++receipt.nonzero_destination_count;
        }
        if (record->destination_x > receipt.highest_destination_x)
            receipt.highest_destination_x = record->destination_x;
        if (record->destination_y > receipt.highest_destination_y)
            receipt.highest_destination_y = record->destination_y;
        if (record->destination_orientation >
            receipt.highest_destination_orientation) {
            receipt.highest_destination_orientation =
                record->destination_orientation;
        }
        for (previous = 0; previous < entry; ++previous) {
            const Nexus_V1_DgnStructure1FEntry *prior =
                &level->structure1f_entries[previous];
            if (prior->family == NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS &&
                prior->structure1a_relation_valid &&
                prior->destination_x == record->destination_x &&
                prior->destination_y == record->destination_y &&
                prior->destination_orientation == record->destination_orientation) {
                duplicate = 1;
                break;
            }
        }
        if (duplicate) ++receipt.duplicate_destination_count;
        else ++receipt.unique_destination_count;
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_destination_count == receipt.wall_sensor_entry_count;
    receipt.destination_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_wall_sensor_control_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FWallSensorControlSelectorReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FWallSensorControlSelectorReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint8_t selector;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS) continue;
        ++receipt.wall_sensor_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_control_selector_count;
        selector = record->type_or_control;
        if (selector == 0U) ++receipt.zero_control_selector_count;
        else ++receipt.nonzero_control_selector_count;
        if (selector > receipt.highest_control_selector)
            receipt.highest_control_selector = selector;
        if (seen[selector]) ++receipt.duplicate_control_selector_count;
        else { seen[selector] = 1U; ++receipt.unique_control_selector_count; }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_control_selector_count == receipt.wall_sensor_entry_count;
    receipt.control_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_wall_sensor_control_destination_tuple_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FWallSensorControlDestinationTupleReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FWallSensorControlDestinationTupleReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint32_t tuple;
        int previous;
        int duplicate = 0;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS) {
            continue;
        }
        ++receipt.wall_sensor_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_tuple_count;
        tuple = ((uint32_t)record->type_or_control << 24) |
            ((uint32_t)record->destination_x << 16) |
            ((uint32_t)record->destination_y << 8) |
            record->destination_orientation;
        if (tuple == 0U) ++receipt.zero_tuple_count;
        else ++receipt.nonzero_tuple_count;
        if (tuple > receipt.highest_tuple) receipt.highest_tuple = tuple;
        for (previous = 0; previous < entry; ++previous) {
            const Nexus_V1_DgnStructure1FEntry *prior =
                &level->structure1f_entries[previous];
            if (prior->family == NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS &&
                prior->structure1a_relation_valid &&
                prior->type_or_control == record->type_or_control &&
                prior->destination_x == record->destination_x &&
                prior->destination_y == record->destination_y &&
                prior->destination_orientation == record->destination_orientation) {
                duplicate = 1;
                break;
            }
        }
        if (duplicate) ++receipt.duplicate_tuple_count;
        else ++receipt.unique_tuple_count;
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_tuple_count == receipt.wall_sensor_entry_count;
    receipt.tuple_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_wall_sensor_model_rotation_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FWallSensorModelRotationPairReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FWallSensorModelRotationPairReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint16_t pair;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS) {
            continue;
        }
        ++receipt.wall_sensor_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_pair_count;
        pair = (uint16_t)(((uint16_t)record->model_or_aspect << 8) |
                          record->rotation);
        if (pair == 0U) ++receipt.zero_pair_count;
        else ++receipt.nonzero_pair_count;
        if (pair > receipt.highest_pair) receipt.highest_pair = pair;
        if (seen[pair]) ++receipt.duplicate_pair_count;
        else {
            seen[pair] = 1U;
            ++receipt.unique_pair_count;
        }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_pair_count == receipt.wall_sensor_entry_count;
    receipt.pair_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_wall_decoration_model_rotation_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FWallDecorationModelRotationPairReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FWallDecorationModelRotationPairReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint16_t pair;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS) {
            continue;
        }
        ++receipt.wall_decoration_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_pair_count;
        pair = (uint16_t)(((uint16_t)record->model_or_aspect << 8) |
                          record->rotation);
        if (pair == 0U) ++receipt.zero_pair_count;
        else ++receipt.nonzero_pair_count;
        if (pair > receipt.highest_pair) receipt.highest_pair = pair;
        if (seen[pair]) ++receipt.duplicate_pair_count;
        else {
            seen[pair] = 1U;
            ++receipt.unique_pair_count;
        }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_pair_count == receipt.wall_decoration_entry_count;
    receipt.pair_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_alcove_payload_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FAlcovePayloadSelectorReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FAlcovePayloadSelectorReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint8_t selector;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.alcove_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_payload_selector_count;
        selector = record->item_id;
        if (selector == 0U) ++receipt.zero_payload_selector_count;
        else ++receipt.nonzero_payload_selector_count;
        if (selector > receipt.highest_payload_selector)
            receipt.highest_payload_selector = selector;
        if (seen[selector]) ++receipt.duplicate_payload_selector_count;
        else { seen[selector] = 1U; ++receipt.unique_payload_selector_count; }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_payload_selector_count == receipt.alcove_entry_count;
    receipt.payload_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_alcove_payload_rotation_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FAlcovePayloadRotationPairReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FAlcovePayloadRotationPairReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint16_t pair;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.alcove_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_pair_count;
        pair = (uint16_t)(((uint16_t)record->item_id << 8) |
                          record->rotation);
        if (pair == 0U) ++receipt.zero_pair_count;
        else ++receipt.nonzero_pair_count;
        if (pair > receipt.highest_pair) receipt.highest_pair = pair;
        if (seen[pair]) ++receipt.duplicate_pair_count;
        else {
            seen[pair] = 1U;
            ++receipt.unique_pair_count;
        }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_pair_count == receipt.alcove_entry_count;
    receipt.pair_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_floor_sensor_control_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorSensorControlSelectorReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFloorSensorControlSelectorReceipt receipt;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&spatial, 0, sizeof(spatial));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &spatial) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1f_spatial_valid = spatial.valid;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint8_t selector;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS) continue;
        ++receipt.floor_sensor_entry_count;
        if (!spatial.valid) continue;
        ++receipt.resolved_control_selector_count;
        selector = record->type_or_control;
        if (selector == 0U) ++receipt.zero_control_selector_count;
        else ++receipt.nonzero_control_selector_count;
        if (selector > receipt.highest_control_selector)
            receipt.highest_control_selector = selector;
        if (seen[selector]) ++receipt.duplicate_control_selector_count;
        else { seen[selector] = 1U; ++receipt.unique_control_selector_count; }
    }
    receipt.complete = receipt.structure1f_spatial_valid &&
        receipt.resolved_control_selector_count == receipt.floor_sensor_entry_count;
    receipt.control_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_floor_sensor_destination_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorSensorDestinationReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFloorSensorDestinationReceipt receipt;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&spatial, 0, sizeof(spatial));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &spatial) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1f_spatial_valid = spatial.valid;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record = &level->structure1f_entries[entry];
        int previous;
        int duplicate = 0;
        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS) continue;
        ++receipt.floor_sensor_entry_count;
        if (!spatial.valid) continue;
        ++receipt.resolved_destination_count;
        if (record->destination_x == 0U && record->destination_y == 0U &&
            record->destination_orientation == 0U) ++receipt.zero_destination_count;
        else ++receipt.nonzero_destination_count;
        if (record->destination_x > receipt.highest_destination_x)
            receipt.highest_destination_x = record->destination_x;
        if (record->destination_y > receipt.highest_destination_y)
            receipt.highest_destination_y = record->destination_y;
        if (record->destination_orientation > receipt.highest_destination_orientation)
            receipt.highest_destination_orientation = record->destination_orientation;
        for (previous = 0; previous < entry; ++previous) {
            const Nexus_V1_DgnStructure1FEntry *prior = &level->structure1f_entries[previous];
            if (prior->family == NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS &&
                prior->destination_x == record->destination_x &&
                prior->destination_y == record->destination_y &&
                prior->destination_orientation == record->destination_orientation) {
                duplicate = 1;
                break;
            }
        }
        if (duplicate) ++receipt.duplicate_destination_count;
        else ++receipt.unique_destination_count;
    }
    receipt.complete = receipt.structure1f_spatial_valid &&
        receipt.resolved_destination_count == receipt.floor_sensor_entry_count;
    receipt.destination_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_floor_sensor_model_rotation_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorSensorModelRotationPairReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFloorSensorModelRotationPairReceipt receipt;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    unsigned char seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&spatial, 0, sizeof(spatial));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &spatial) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.structure1f_spatial_valid = spatial.valid;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint16_t pair;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS) {
            continue;
        }
        ++receipt.floor_sensor_entry_count;
        if (!spatial.valid) continue;
        ++receipt.resolved_pair_count;
        pair = (uint16_t)(((uint16_t)record->model_or_aspect << 8) |
                          record->rotation);
        if (pair == 0U) ++receipt.zero_pair_count;
        else ++receipt.nonzero_pair_count;
        if (pair > receipt.highest_pair) receipt.highest_pair = pair;
        if (seen[pair]) ++receipt.duplicate_pair_count;
        else {
            seen[pair] = 1U;
            ++receipt.unique_pair_count;
        }
    }
    receipt.complete = receipt.structure1f_spatial_valid &&
        receipt.resolved_pair_count == receipt.floor_sensor_entry_count;
    receipt.pair_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_floor_sensor_extent_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorSensorExtentPairReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFloorSensorExtentPairReceipt receipt;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    unsigned char seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&spatial, 0, sizeof(spatial));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &spatial) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.structure1f_spatial_valid = spatial.valid;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint16_t pair;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS) {
            continue;
        }
        ++receipt.floor_sensor_entry_count;
        if (!spatial.valid) continue;
        ++receipt.resolved_pair_count;
        pair = (uint16_t)(((uint16_t)record->width << 8) | record->height);
        if (pair == 0U) ++receipt.zero_pair_count;
        else ++receipt.nonzero_pair_count;
        if (pair > receipt.highest_pair) receipt.highest_pair = pair;
        if (seen[pair]) ++receipt.duplicate_pair_count;
        else {
            seen[pair] = 1U;
            ++receipt.unique_pair_count;
        }
    }
    receipt.complete = receipt.structure1f_spatial_valid &&
        receipt.resolved_pair_count == receipt.floor_sensor_entry_count;
    receipt.pair_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_floor_decoration_payload_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorDecorationPayloadSelectorReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFloorDecorationPayloadSelectorReceipt receipt;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;
    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&spatial, 0, sizeof(spatial));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &spatial) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1f_spatial_valid = spatial.valid;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record = &level->structure1f_entries[entry];
        uint8_t selector;
        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS) continue;
        ++receipt.floor_decoration_entry_count;
        if (!spatial.valid) continue;
        ++receipt.resolved_payload_selector_count;
        selector = record->model_or_aspect;
        if (selector == 0U) ++receipt.zero_payload_selector_count;
        else ++receipt.nonzero_payload_selector_count;
        if (selector > receipt.highest_payload_selector)
            receipt.highest_payload_selector = selector;
        if (seen[selector]) ++receipt.duplicate_payload_selector_count;
        else { seen[selector] = 1U; ++receipt.unique_payload_selector_count; }
    }
    receipt.complete = receipt.structure1f_spatial_valid &&
        receipt.resolved_payload_selector_count == receipt.floor_decoration_entry_count;
    receipt.payload_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_floor_decoration_rotation_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorDecorationRotationSelectorReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFloorDecorationRotationSelectorReceipt receipt;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;
    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&spatial, 0, sizeof(spatial));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &spatial) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1f_spatial_valid = spatial.valid;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record = &level->structure1f_entries[entry];
        uint8_t selector;
        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS) continue;
        ++receipt.floor_decoration_entry_count;
        if (!spatial.valid) continue;
        ++receipt.resolved_rotation_selector_count;
        selector = record->rotation;
        if (selector == 0U) ++receipt.zero_rotation_selector_count;
        else ++receipt.nonzero_rotation_selector_count;
        if (selector > receipt.highest_rotation_selector)
            receipt.highest_rotation_selector = selector;
        if (seen[selector]) ++receipt.duplicate_rotation_selector_count;
        else { seen[selector] = 1U; ++receipt.unique_rotation_selector_count; }
    }
    receipt.complete = receipt.structure1f_spatial_valid &&
        receipt.resolved_rotation_selector_count == receipt.floor_decoration_entry_count;
    receipt.rotation_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_floor_decoration_model_rotation_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorDecorationModelRotationPairReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFloorDecorationModelRotationPairReceipt receipt;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    unsigned char seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&spatial, 0, sizeof(spatial));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &spatial) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.structure1f_spatial_valid = spatial.valid;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint16_t pair;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS) {
            continue;
        }
        ++receipt.floor_decoration_entry_count;
        if (!spatial.valid) continue;
        ++receipt.resolved_pair_count;
        pair = (uint16_t)(((uint16_t)record->model_or_aspect << 8) |
                          record->rotation);
        if (pair == 0U) ++receipt.zero_pair_count;
        else ++receipt.nonzero_pair_count;
        if (pair > receipt.highest_pair) receipt.highest_pair = pair;
        if (seen[pair]) ++receipt.duplicate_pair_count;
        else {
            seen[pair] = 1U;
            ++receipt.unique_pair_count;
        }
    }
    receipt.complete = receipt.structure1f_spatial_valid &&
        receipt.resolved_pair_count == receipt.floor_decoration_entry_count;
    receipt.pair_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_floor_decoration_control_extent_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorDecorationControlExtentReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFloorDecorationControlExtentReceipt receipt;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&spatial, 0, sizeof(spatial));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &spatial) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.structure1f_spatial_valid = spatial.valid;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        int previous;
        int duplicate = 0;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS) {
            continue;
        }
        ++receipt.floor_decoration_entry_count;
        if (!spatial.valid) continue;
        ++receipt.resolved_tuple_count;
        if (record->type_or_control == 0U && record->width == 0U &&
            record->height == 0U) {
            ++receipt.zero_tuple_count;
        } else {
            ++receipt.nonzero_tuple_count;
        }
        if (record->type_or_control > receipt.highest_type_or_control) {
            receipt.highest_type_or_control = record->type_or_control;
        }
        if (record->width > receipt.highest_width) {
            receipt.highest_width = record->width;
        }
        if (record->height > receipt.highest_height) {
            receipt.highest_height = record->height;
        }
        for (previous = 0; previous < entry; ++previous) {
            const Nexus_V1_DgnStructure1FEntry *prior =
                &level->structure1f_entries[previous];
            if (prior->family == NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS &&
                prior->type_or_control == record->type_or_control &&
                prior->width == record->width && prior->height == record->height) {
                duplicate = 1;
                break;
            }
        }
        if (duplicate) ++receipt.duplicate_tuple_count;
        else ++receipt.unique_tuple_count;
    }
    receipt.complete = receipt.structure1f_spatial_valid &&
        receipt.resolved_tuple_count == receipt.floor_decoration_entry_count;
    receipt.tuple_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_item_attribute_pair_receipt(const Nexus_V1_Level *l, Nexus_V1_DgnStructure1FItemAttributePairReceipt *o) { Nexus_V1_DgnStructure1FItemAttributePairReceipt r; Nexus_V1_DgnStructure1FSpatialReceipt s; unsigned char seen[UINT16_MAX + 1U]; int i; if (!o) return -1; memset(&r,0,sizeof(r)); memset(&s,0,sizeof(s)); memset(seen,0,sizeof(seen)); if (!l || nexus_v1_level_structure1f_spatial_receipt(l,&s)) {*o=r;return 0;} r.spatial_valid=s.valid; for(i=0;i<l->structure1f_entry_count;i++){const Nexus_V1_DgnStructure1FEntry *e=&l->structure1f_entries[i];uint16_t p;if(e->family!=NEXUS_V1_DGN_STRUCTURE1F_ITEMS)continue;++r.item_count;if(!s.valid)continue;++r.resolved_pair_count;p=((uint16_t)e->attribute1<<8)|e->attribute2;if(seen[p])++r.duplicate_pair_count;else{seen[p]=1;++r.unique_pair_count;}}r.complete=r.spatial_valid&&r.item_count==r.resolved_pair_count;*o=r;return 0;}
int nexus_v1_level_structure1f_item_location_pair_receipt(const Nexus_V1_Level *l, Nexus_V1_DgnStructure1FItemLocationPairReceipt *o) { Nexus_V1_DgnStructure1FItemLocationPairReceipt r; Nexus_V1_DgnStructure1FSpatialReceipt s; unsigned char seen[UINT16_MAX + 1U]; int i; if (!o) return -1; memset(&r,0,sizeof(r)); memset(&s,0,sizeof(s)); memset(seen,0,sizeof(seen)); if (!l || nexus_v1_level_structure1f_spatial_receipt(l,&s)) {*o=r;return 0;} r.spatial_valid=s.valid; for(i=0;i<l->structure1f_entry_count;i++){const Nexus_V1_DgnStructure1FEntry *e=&l->structure1f_entries[i];uint16_t p;if(e->family!=NEXUS_V1_DGN_STRUCTURE1F_ITEMS)continue;++r.item_count;if(!s.valid)continue;++r.resolved_pair_count;p=((uint16_t)e->location<<8)|e->item_id;if(seen[p])++r.duplicate_pair_count;else{seen[p]=1;++r.unique_pair_count;}}r.complete=r.spatial_valid&&r.item_count==r.resolved_pair_count;*o=r;return 0;}

int nexus_v1_level_structure1f_item_coordinate_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FItemCoordinatePairReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FItemCoordinatePairReceipt receipt;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    unsigned char seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&spatial, 0, sizeof(spatial));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &spatial) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.spatial_valid = spatial.valid;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint16_t pair;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_ITEMS) continue;
        ++receipt.item_count;
        if (!spatial.valid) continue;
        ++receipt.resolved_pair_count;
        pair = (uint16_t)(((uint16_t)record->x << 8) | record->y);
        if (pair == 0U) ++receipt.zero_pair_count;
        else ++receipt.nonzero_pair_count;
        if (pair > receipt.highest_pair) receipt.highest_pair = pair;
        if (seen[pair]) ++receipt.duplicate_pair_count;
        else {
            seen[pair] = 1U;
            ++receipt.unique_pair_count;
        }
    }
    receipt.complete = receipt.spatial_valid &&
        receipt.resolved_pair_count == receipt.item_count;
    receipt.semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_floor_decoration_offset_pair_receipt(const Nexus_V1_Level *level, Nexus_V1_DgnStructure1FFloorDecorationOffsetPairReceipt *out)
{
    Nexus_V1_DgnStructure1FFloorDecorationOffsetPairReceipt r;
    Nexus_V1_DgnStructure1FSpatialReceipt s;
    unsigned char seen[UINT16_MAX + 1U];
    int i;
    if (!out) return -1;
    memset(&r, 0, sizeof(r));
    memset(&s, 0, sizeof(s));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &s) != 0) {
        *out = r;
        return 0;
    }
    r.structure1f_spatial_valid = s.valid;
    for (i = 0; i < level->structure1f_entry_count; ++i) {
        const Nexus_V1_DgnStructure1FEntry *e = &level->structure1f_entries[i];
        uint16_t p;
        if (e->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS) continue;
        ++r.entry_count;
        if (!s.valid) continue;
        ++r.resolved_pair_count;
        p = (uint16_t)(((uint16_t)(uint8_t)e->offset_x << 8) | (uint8_t)e->offset_y);
        if (p) ++r.nonzero_pair_count; else ++r.zero_pair_count;
        if (seen[p]) ++r.duplicate_pair_count; else { seen[p] = 1; ++r.unique_pair_count; }
    }
    r.complete = r.structure1f_spatial_valid && r.entry_count == r.resolved_pair_count;
    *out = r;
    return 0;
}

int nexus_v1_level_structure3_payload_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3PayloadReceipt *out_receipt)
{
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (level) *out_receipt = level->structure3_payload;
    return 0;
}

int nexus_v1_level_structure3_directory_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3DirectoryReceipt *out_receipt)
{
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (level) *out_receipt = level->structure3_directory;
    return 0;
}

int nexus_v1_level_structure3_entry_header_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3EntryHeaderReceipt *out_receipt)
{
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (level) *out_receipt = level->structure3_entry_headers;
    return 0;
}

int nexus_v1_level_structure3_face_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3FaceReceipt *out_receipt)
{
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (level) *out_receipt = level->structure3_faces;
    return 0;
}

int nexus_v1_level_structure3_face_material_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3FaceMaterialReceipt *out_receipt)
{
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (level) *out_receipt = level->structure3_face_materials;
    return 0;
}

int nexus_v1_level_collect_structure3_face_material_bindings(
    const Nexus_V1_Level *level, const uint8_t *data, int size,
    Nexus_V1_DgnFaceMaterialBinding *out_bindings, int max_bindings,
    int *out_binding_count)
{
    const Nexus_V1_DgnStructure3PayloadReceipt *payload;
    int entry;
    int output = 0;

    if (out_binding_count) *out_binding_count = 0;
    if (!level || !data || size <= 0 || !out_bindings || max_bindings <= 0 ||
        !out_binding_count || !level->structure3_payload.valid ||
        !level->structure3_directory.valid ||
        !level->structure3_entry_headers.valid ||
        !level->structure3_faces.valid ||
        !level->structure3_face_materials.valid ||
        !level->structure3_face_materials.selector_bindings_complete) {
        return -1;
    }
    payload = &level->structure3_payload;
    if (payload->byte_offset < 0 || payload->byte_size <= 0 ||
        payload->byte_offset > size || payload->byte_size > size - payload->byte_offset ||
        level->structure3_directory.entry_count <= 0 ||
        level->structure3_directory.directory_byte_count < 4 ||
        level->structure3_directory.directory_byte_count > payload->byte_size) {
        return -1;
    }

    for (entry = 0; entry < level->structure3_directory.entry_count; ++entry) {
        uint32_t entry_offset = rb32(data + payload->byte_offset + 4 + entry * 4);
        uint32_t entry_end = entry + 1 < level->structure3_directory.entry_count
            ? rb32(data + payload->byte_offset + 4 + (entry + 1) * 4)
            : (uint32_t)payload->byte_size;
        const uint8_t *header;
        uint16_t face_count;
        uint32_t face_offset;
        int face_index;

        if (entry_offset >= (uint32_t)payload->byte_size || entry_end < entry_offset ||
            entry_end - entry_offset < NEXUS_DGN_STRUCTURE3_ENTRY_HEADER_BYTES) {
            return -1;
        }
        header = data + payload->byte_offset + entry_offset;
        face_count = rb16(header + 6);
        face_offset = rb32(header + 16);
        if (face_offset < entry_offset || face_offset > entry_end ||
            (uint64_t)face_offset + (uint64_t)face_count * 12U > entry_end ||
            output > max_bindings - (int)face_count) {
            return -1;
        }
        for (face_index = 0; face_index < (int)face_count; ++face_index) {
            const uint8_t *face = data + payload->byte_offset + face_offset +
                face_index * 12;
            uint16_t fill = rb16(face + 10);
            Nexus_V1_DgnFaceMaterialBinding *binding = &out_bindings[output];

            if ((face[8] & 0x40U) == 0U) {
                continue;
            }
            binding->face_ordinal = (uint16_t)output;
            binding->material_selector = 0;
            if ((fill & 0xff00U) == 0U) {
                binding->selector_kind = NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_STATIC;
                binding->material_selector = (uint16_t)(fill & 0xffU);
            } else if ((fill & 0xff00U) == 0x0800U) {
                binding->selector_kind = NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_ANIMATED;
                binding->material_selector = (uint16_t)(fill & 0xffU);
            } else {
                return -1;
            }
            ++output;
        }
    }
    if (output <= 0 || output > level->structure3_faces.face_count) return -1;
    *out_binding_count = output;
    return 0;
}

int nexus_v1_level_structure3_edge_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3EdgeReceipt *out_receipt)
{
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (level) *out_receipt = level->structure3_edges;
    return 0;
}

int nexus_v1_level_structure3_vector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3VectorReceipt *out_receipt)
{
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (level) *out_receipt = level->structure3_vectors;
    return 0;
}

int nexus_v1_level_structure3_face_geometry_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3FaceGeometryReceipt *out_receipt)
{
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (level) *out_receipt = level->structure3_face_geometry;
    return 0;
}

int nexus_v1_level_structure3_face_edge_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3FaceEdgeReceipt *out_receipt)
{
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (level) *out_receipt = level->structure3_face_edges;
    return 0;
}

int nexus_v1_level_structure3_face_normal_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3FaceNormalPairReceipt *out_receipt)
{
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (level) *out_receipt = level->structure3_face_normal_pairs;
    return 0;
}

int nexus_v1_level_structure3_face_normal_geometry_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3FaceNormalGeometryReceipt *out_receipt)
{
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (level) *out_receipt = level->structure3_face_normal_geometry;
    return 0;
}

int nexus_v1_level_structure3_mesh_semantic_handoff_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3MeshSemanticHandoffReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3MeshSemanticHandoffReceipt receipt;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    if (!level) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.source_topology_valid = level->structure3_faces.valid;
    receipt.source_vectors_valid = level->structure3_vectors.valid;
    receipt.source_face_geometry_valid = level->structure3_face_geometry.valid;
    receipt.source_face_normal_pairing_valid =
        level->structure3_face_normal_pairs.valid;
    receipt.source_face_normal_geometry_valid =
        level->structure3_face_normal_geometry.valid;
    receipt.entry_count = level->structure3_faces.entry_count;
    receipt.vertex_count = level->structure3_faces.vertex_count;
    receipt.face_count = level->structure3_faces.face_count;
    receipt.normal_count = level->structure3_faces.normal_count;
    receipt.face_normal_pair_count =
        level->structure3_face_normal_pairs.face_normal_pair_count;
    receipt.source_facts_complete = receipt.source_topology_valid &&
        receipt.source_vectors_valid && receipt.source_face_geometry_valid &&
        receipt.source_face_normal_pairing_valid &&
        receipt.face_count == receipt.normal_count &&
        receipt.face_count == receipt.face_normal_pair_count;
    receipt.original_capture_required = receipt.source_facts_complete;
    /* No original Saturn trace is represented by the DGN corpus alone. */
    receipt.original_capture_available = 0;
    receipt.normal_plane_semantics_proven = 0;
    receipt.transform_semantics_proven = 0;
    receipt.texture_palette_semantics_proven = 0;
    receipt.draw_semantics_proven = 0;
    receipt.renderer_handoff_ready = 0;
    receipt.blocks_real_dgn_mesh_render = 1;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_extract_structure3_mesh_entry(
    const Nexus_V1_Level *level, const uint8_t *data, int size,
    int entry_index, Nexus_V1_DgnStructure3Vector *out_vertices,
    int max_vertices, Nexus_V1_DgnStructure3Face *out_faces, int max_faces,
    Nexus_V1_DgnStructure3Vector *out_normals, int max_normals,
    Nexus_V1_DgnStructure3MeshEntryReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3MeshEntryReceipt receipt;
    const Nexus_V1_DgnStructure3PayloadReceipt *payload;
    int payload_offset;
    int payload_size;
    uint32_t payload_hash = 2166136261u;
    uint32_t entry_offset;
    const uint8_t *header;
    uint16_t vertex_count;
    uint16_t face_count;
    uint16_t normal_count;
    uint32_t vertex_offset;
    uint32_t face_offset;
    uint32_t normal_offset;
    int index;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.entry_index = entry_index;
    *out_receipt = receipt;
    if (!level || !data || size <= 0 || entry_index < 0 ||
        entry_index >= level->structure3_directory.entry_count ||
        max_vertices < 0 || max_faces < 0 || max_normals < 0) return -1;

    payload = &level->structure3_payload;
    payload_offset = payload->byte_offset;
    payload_size = payload->byte_size;
    if (!payload->valid || !level->structure3_directory.valid ||
        !level->structure3_entry_headers.valid ||
        !level->structure3_faces.valid || !level->structure3_vectors.valid ||
        !level->structure3_face_normal_pairs.valid || payload_offset < 0 ||
        payload_size < 4 || payload_offset > size || payload_size > size - payload_offset)
        return -1;
    for (index = 0; index < payload_size; ++index) {
        payload_hash ^= data[payload_offset + index];
        payload_hash *= 16777619u;
    }
    if (payload_hash != payload->raw_payload_hash) return -1;
    receipt.source_identity_valid = 1;
    entry_offset = rb32(data + payload_offset + 4 + entry_index * 4);
    if (entry_offset > (uint32_t)payload_size ||
        (uint32_t)payload_size - entry_offset < 40U) return -1;
    header = data + payload_offset + entry_offset;
    vertex_count = rb16(header + 4);
    face_count = rb16(header + 6);
    /* The documented entry framing stores one normal row per face; its third
     * region is bounded at header+20 rather than carrying a third count. */
    normal_count = face_count;
    vertex_offset = rb32(header + 8);
    face_offset = rb32(header + 16);
    normal_offset = rb32(header + 20);
    if (vertex_offset > (uint32_t)payload_size ||
        face_offset > (uint32_t)payload_size || normal_offset > (uint32_t)payload_size ||
        (uint32_t)vertex_count > ((uint32_t)payload_size - vertex_offset) / 12U ||
        (uint32_t)face_count > ((uint32_t)payload_size - face_offset) / 12U ||
        (uint32_t)normal_count > ((uint32_t)payload_size - normal_offset) / 12U)
        return -1;
    receipt.vertex_count = (int)vertex_count;
    receipt.face_count = (int)face_count;
    receipt.normal_count = (int)normal_count;
    receipt.vertex_capacity_sufficient = max_vertices >= (int)vertex_count;
    receipt.face_capacity_sufficient = max_faces >= (int)face_count;
    receipt.normal_capacity_sufficient = max_normals >= (int)normal_count;
    if (!receipt.vertex_capacity_sufficient || !receipt.face_capacity_sufficient ||
        !receipt.normal_capacity_sufficient || (vertex_count && !out_vertices) ||
        (face_count && !out_faces) || (normal_count && !out_normals)) {
        *out_receipt = receipt;
        return -1;
    }
    for (index = 0; index < (int)vertex_count; ++index) {
        const uint8_t *row = data + payload_offset + vertex_offset + index * 12;
        out_vertices[index].x = rbs32(row);
        out_vertices[index].y = rbs32(row + 4);
        out_vertices[index].z = rbs32(row + 8);
    }
    for (index = 0; index < (int)face_count; ++index) {
        const uint8_t *row = data + payload_offset + face_offset + index * 12;
        Nexus_V1_DgnStructure3Face *face = &out_faces[index];
        face->vertex_indexes[0] = rb16(row);
        face->vertex_indexes[1] = rb16(row + 2);
        face->vertex_indexes[2] = rb16(row + 4);
        face->vertex_indexes[3] = rb16(row + 6);
        face->flags = row[8];
        face->raw_byte_9 = row[9];
        face->fill_selector = rb16(row + 10);
        face->triangle = face->vertex_indexes[2] == face->vertex_indexes[3];
    }
    for (index = 0; index < (int)normal_count; ++index) {
        const uint8_t *row = data + payload_offset + normal_offset + index * 12;
        out_normals[index].x = rbs32(row);
        out_normals[index].y = rbs32(row + 4);
        out_normals[index].z = rbs32(row + 8);
    }
    receipt.valid = 1;
    receipt.transform_or_draw_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure3_mesh_geometry_ready(
    const Nexus_V1_Level *level, const uint8_t *data, int size,
    int *out_face_total)
{
    Nexus_V1_DgnStructure3FaceReceipt faces;
    int entry_index;
    int face_total = 0;

    if (out_face_total) *out_face_total = 0;
    if (!level || !data || size <= 0) return 0;
    memset(&faces, 0, sizeof(faces));
    if (nexus_v1_level_structure3_face_receipt(level, &faces) != 0 ||
            !faces.valid || faces.entry_count <= 0) {
        return 0;
    }
    for (entry_index = 0; entry_index < faces.entry_count; ++entry_index) {
        Nexus_V1_DgnStructure3MeshEntryReceipt mesh_entry;
        Nexus_V1_DgnStructure3Vector *vertices = NULL;
        Nexus_V1_DgnStructure3Face *extracted_faces = NULL;
        Nexus_V1_DgnStructure3Vector *normals = NULL;
        Nexus_V1_DgnMeshFixedVertex *fixed_vertices = NULL;
        Nexus_V1_DgnMeshSourceFace *source_faces = NULL;
        Nexus_V1_DgnMesh *mesh = NULL;
        Nexus_V1_DgnMeshInput mesh_input;
        int vertex_count;
        int face_count;
        int normal_count;
        int index;
        int ok;

        memset(&mesh_entry, 0, sizeof(mesh_entry));
        /* The bounded-requirements query fills the receipt even when the
         * copy arrays are NULL; only the receipt fields are checked. */
        (void)nexus_v1_level_extract_structure3_mesh_entry(
            level, data, size, entry_index, NULL, 0, NULL, 0,
            NULL, 0, &mesh_entry);
        if (!mesh_entry.source_identity_valid ||
                mesh_entry.vertex_count <= 0 || mesh_entry.face_count <= 0 ||
                mesh_entry.normal_count != mesh_entry.face_count) {
            return 0;
        }
        vertex_count = mesh_entry.vertex_count;
        face_count = mesh_entry.face_count;
        normal_count = mesh_entry.normal_count;
        vertices = (Nexus_V1_DgnStructure3Vector *)calloc(
            (size_t)vertex_count, sizeof(*vertices));
        extracted_faces = (Nexus_V1_DgnStructure3Face *)calloc(
            (size_t)face_count, sizeof(*extracted_faces));
        normals = (Nexus_V1_DgnStructure3Vector *)calloc(
            (size_t)normal_count, sizeof(*normals));
        fixed_vertices = (Nexus_V1_DgnMeshFixedVertex *)calloc(
            (size_t)vertex_count, sizeof(*fixed_vertices));
        source_faces = (Nexus_V1_DgnMeshSourceFace *)calloc(
            (size_t)face_count, sizeof(*source_faces));
        mesh = (Nexus_V1_DgnMesh *)calloc(1U, sizeof(*mesh));
        if (!vertices || !extracted_faces || !normals || !fixed_vertices ||
                !source_faces || !mesh) {
            free(vertices); free(extracted_faces); free(normals);
            free(fixed_vertices); free(source_faces); free(mesh);
            return 0;
        }
        memset(&mesh_entry, 0, sizeof(mesh_entry));
        ok = nexus_v1_level_extract_structure3_mesh_entry(
                level, data, size, entry_index, vertices, vertex_count,
                extracted_faces, face_count, normals, normal_count,
                &mesh_entry) == 0 &&
            mesh_entry.valid && mesh_entry.source_identity_valid &&
            mesh_entry.vertex_count == vertex_count &&
            mesh_entry.face_count == face_count &&
            mesh_entry.normal_count == normal_count &&
            !mesh_entry.transform_or_draw_semantics_proven;
        if (ok) {
            for (index = 0; index < vertex_count; ++index) {
                fixed_vertices[index].x = vertices[index].x;
                fixed_vertices[index].y = vertices[index].y;
                fixed_vertices[index].z = vertices[index].z;
            }
            for (index = 0; index < face_count; ++index) {
                source_faces[index].vertex_index[0] =
                    extracted_faces[index].vertex_indexes[0];
                source_faces[index].vertex_index[1] =
                    extracted_faces[index].vertex_indexes[1];
                source_faces[index].vertex_index[2] =
                    extracted_faces[index].vertex_indexes[2];
                source_faces[index].vertex_index[3] =
                    extracted_faces[index].vertex_indexes[3];
                source_faces[index].flags = extracted_faces[index].flags;
                source_faces[index].fill_selector =
                    extracted_faces[index].fill_selector;
            }
            memset(&mesh_input, 0, sizeof(mesh_input));
            mesh_input.vertices = fixed_vertices;
            mesh_input.vertex_count = vertex_count;
            mesh_input.faces = source_faces;
            mesh_input.face_count = face_count;
            mesh_input.canonical_source_verified = 1;
            mesh_input.topology_receipt_valid = 1;
            mesh_input.fixed_point_vectors_valid = 1;
            ok = nexus_v1_dgn_mesh_build(&mesh_input, mesh) == 1 &&
                mesh->status == NEXUS_V1_DGN_MESH_READY_GEOMETRY &&
                mesh->can_submit_geometry &&
                !mesh->can_submit_textured_raster &&
                !mesh->permits_fallback_visuals &&
                mesh->face_count == face_count;
        }
        if (ok) face_total += face_count;
        free(vertices); free(extracted_faces); free(normals);
        free(fixed_vertices); free(source_faces); free(mesh);
        if (!ok) return 0;
    }
    if (face_total != faces.face_count) return 0;
    if (out_face_total) *out_face_total = face_total;
    return 1;
}

int nexus_v1_level_structure3_attachment_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3AttachmentReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3AttachmentReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    receipt.structure3_directory_valid = level->structure3_directory.valid &&
        level->structure3_directory.entry_count >= 0 &&
        level->structure3_directory.entry_count <= NEXUS_DGN_MAX_STRUCTURE3_ENTRIES;
    receipt.structure3_faces_valid = level->structure3_faces.valid;
    receipt.structure3_face_normal_pairing_valid =
        level->structure3_face_normal_pairs.valid;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint32_t model_index;
        uint32_t face_count;

        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.structure1f_bound_entry_count;
        if (!record->structure1a_relation_valid) continue;
        model_index = record->structure1a_structure3_model_index;
        if (!receipt.structure3_directory_valid ||
            model_index >= (uint32_t)level->structure3_directory.entry_count) {
            ++receipt.out_of_range_model_selector_count;
            continue;
        }
        ++receipt.model_entry_bound_count;
        /* Entry headers are independently bounded before their face count is
         * consulted here; the face selector is an ordinal, never a draw key. */
        face_count = level->structure3_entry_face_counts[model_index];
        if (record->face >= face_count) {
            ++receipt.out_of_range_face_selector_count;
            continue;
        }
        ++receipt.face_normal_bound_count;
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.structure3_directory_valid && receipt.structure3_faces_valid &&
        receipt.structure3_face_normal_pairing_valid &&
        receipt.structure1f_bound_entry_count == receipt.model_entry_bound_count &&
        receipt.structure1f_bound_entry_count == receipt.face_normal_bound_count &&
        receipt.out_of_range_model_selector_count == 0 &&
        receipt.out_of_range_face_selector_count == 0;
    receipt.record_to_face_normal_semantics_proven = receipt.complete;
    receipt.normal_plane_transform_or_draw_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_dgn_bind_structure3_face_capture_candidate(
    const Nexus_V1_Level *level, const uint8_t *dgn_data, int dgn_size,
    int dgn_source_hash_verified, int capture_source_verified,
    const Nexus_V1_DgnStructure3FaceCaptureCandidate *candidate,
    const uint8_t *captured_texture_span, int captured_texture_span_size,
    const uint8_t *captured_palette_state, int captured_palette_state_size,
    const uint8_t *captured_vdp1_state, int captured_vdp1_state_size,
    const uint8_t *captured_transform_state, int captured_transform_state_size,
    const uint8_t *captured_normal_culling_state,
    int captured_normal_culling_state_size,
    const uint8_t *captured_vdp1_command, int captured_vdp1_command_size,
    Nexus_V1_DgnStructure3FaceCaptureBindingReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3FaceCaptureBindingReceipt receipt;
    Nexus_V1_Vdp1TextureCommand vdp1_command;
    const Nexus_V1_DgnStructure3PayloadReceipt *payload;
    const uint8_t *header;
    const uint8_t *face;
    const uint8_t *normal;
    uint32_t entry_offset;
    uint32_t vertex_offset;
    uint32_t face_offset;
    uint32_t normal_offset;
    uint16_t vertex_count;
    uint16_t face_count;
    uint16_t indexes[4];
    uint32_t vertex_hash = 2166136261u;
    int slot_count;
    int slot;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.blocks_real_dgn_mesh_render = 1;
    *out_receipt = receipt;
    if (!level || !dgn_data || dgn_size <= 0 || !candidate ||
        captured_texture_span_size <= 0 || captured_palette_state_size <= 0 ||
        captured_vdp1_state_size <= 0 || captured_transform_state_size <= 0 ||
        captured_normal_culling_state_size <= 0 || captured_vdp1_command_size <= 0 ||
        !captured_texture_span || !captured_palette_state || !captured_vdp1_state ||
        !captured_transform_state || !captured_normal_culling_state ||
        !captured_vdp1_command) return -1;
    payload = &level->structure3_payload;
    if (!payload->valid || !level->structure3_directory.valid ||
        !level->structure3_entry_headers.valid || !level->structure3_faces.valid ||
        !level->structure3_vectors.valid || !level->structure3_face_normal_pairs.valid ||
        payload->byte_offset < 0 || payload->byte_size < 4 ||
        payload->byte_offset > dgn_size ||
        payload->byte_size > dgn_size - payload->byte_offset ||
        candidate->entry_index >= (uint32_t)level->structure3_directory.entry_count ||
        candidate->dgn_fnv1a64 == 0U ||
        candidate->structure3_payload_fnv1a32 == 0U ||
        candidate->typed_mesh_corpus_fnv1a32 !=
            NEXUS_DGN_RETAIL_TYPED_MESH_CORPUS_FNV1A32 ||
        candidate->face_row_fnv1a32 == 0U ||
        candidate->referenced_vertex_rows_fnv1a32 == 0U ||
        candidate->normal_row_fnv1a32 == 0U ||
        candidate->texture_span_fnv1a64 == 0U ||
        candidate->palette_state_fnv1a64 == 0U ||
        candidate->vdp1_state_fnv1a64 == 0U ||
        candidate->transform_state_fnv1a64 == 0U ||
        candidate->normal_culling_state_fnv1a64 == 0U ||
        candidate->vdp1_command_fnv1a64 == 0U ||
        candidate->first_sequence == 0U ||
        candidate->first_sequence >= candidate->last_sequence) return -1;
    /* The caller owns canonical-source admission. Do not let arbitrary data
     * self-admit by echoing its fingerprint inside a capture packet. */
    receipt.dgn_source_hash_verified = dgn_source_hash_verified != 0;
    receipt.capture_source_verified = capture_source_verified != 0;
    receipt.candidate_framing_valid = receipt.dgn_source_hash_verified &&
        receipt.capture_source_verified;
    if (!receipt.candidate_framing_valid) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.dgn_source_matches = nexus_v1_fnv1a64(dgn_data, dgn_size) ==
        candidate->dgn_fnv1a64;
    receipt.structure3_payload_matches =
        payload->raw_payload_hash == candidate->structure3_payload_fnv1a32;
    receipt.typed_mesh_corpus_matches = 1;
    if (!receipt.dgn_source_matches || !receipt.structure3_payload_matches) {
        *out_receipt = receipt;
        return 0;
    }
    entry_offset = rb32(dgn_data + payload->byte_offset + 4 +
                        candidate->entry_index * 4U);
    if (entry_offset > (uint32_t)payload->byte_size ||
        (uint32_t)payload->byte_size - entry_offset < 40U) {
        *out_receipt = receipt;
        return 0;
    }
    header = dgn_data + payload->byte_offset + entry_offset;
    vertex_count = rb16(header + 4);
    face_count = rb16(header + 6);
    vertex_offset = rb32(header + 8);
    face_offset = rb32(header + 16);
    normal_offset = rb32(header + 20);
    if (candidate->face_ordinal >= (uint32_t)face_count ||
        vertex_offset > (uint32_t)payload->byte_size ||
        face_offset > (uint32_t)payload->byte_size ||
        normal_offset > (uint32_t)payload->byte_size ||
        (uint32_t)face_count > ((uint32_t)payload->byte_size - face_offset) / 12U ||
        (uint32_t)face_count > ((uint32_t)payload->byte_size - normal_offset) / 12U ||
        (uint32_t)vertex_count > ((uint32_t)payload->byte_size - vertex_offset) / 12U) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.entry_face_matches = 1;
    face = dgn_data + payload->byte_offset + face_offset +
        candidate->face_ordinal * 12U;
    normal = dgn_data + payload->byte_offset + normal_offset +
        candidate->face_ordinal * 12U;
    receipt.face_row_matches = nexus_v1_fnv1a32(face, 12) ==
        candidate->face_row_fnv1a32;
    receipt.normal_row_matches = nexus_v1_fnv1a32(normal, 12) ==
        candidate->normal_row_fnv1a32;
    for (slot = 0; slot < 4; ++slot) indexes[slot] = rb16(face + slot * 2);
    slot_count = indexes[2] == indexes[3] ? 3 : 4;
    for (slot = 0; slot < slot_count; ++slot) {
        const uint8_t *vertex;
        int byte;
        if (indexes[slot] >= vertex_count) {
            *out_receipt = receipt;
            return 0;
        }
        vertex = dgn_data + payload->byte_offset + vertex_offset + indexes[slot] * 12U;
        for (byte = 0; byte < 12; ++byte) {
            vertex_hash ^= vertex[byte];
            vertex_hash *= 16777619u;
        }
    }
    /* A capture can identify a row without making it drawable. Do not bind a
     * zero-area Structure3 face into the host packet, even with matching
     * original-capture fingerprints. */
    if (!nexus_v1_structure3_face_has_noncollinear_vertices(
            dgn_data + payload->byte_offset + vertex_offset, indexes,
            slot_count)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.referenced_vertex_rows_match = vertex_hash ==
        candidate->referenced_vertex_rows_fnv1a32;
    receipt.fill_selector_matches = rb16(face + 10) == candidate->fill_selector;
    receipt.texture_span_matches = nexus_v1_fnv1a64(captured_texture_span,
        captured_texture_span_size) == candidate->texture_span_fnv1a64;
    receipt.palette_state_matches = nexus_v1_fnv1a64(captured_palette_state,
        captured_palette_state_size) == candidate->palette_state_fnv1a64;
    receipt.vdp1_state_matches = nexus_v1_fnv1a64(captured_vdp1_state,
        captured_vdp1_state_size) == candidate->vdp1_state_fnv1a64;
    receipt.transform_state_matches = nexus_v1_fnv1a64(captured_transform_state,
        captured_transform_state_size) == candidate->transform_state_fnv1a64;
    receipt.normal_culling_state_matches = nexus_v1_fnv1a64(
        captured_normal_culling_state, captured_normal_culling_state_size) ==
        candidate->normal_culling_state_fnv1a64;
    receipt.vdp1_command_matches = nexus_v1_fnv1a64(captured_vdp1_command,
        captured_vdp1_command_size) == candidate->vdp1_command_fnv1a64;
    receipt.vdp1_command_format_matches =
        nexus_v1_vdp1_texture_command_parse(captured_vdp1_command,
            captured_vdp1_command_size, &vdp1_command) == 0 &&
        vdp1_command.texture_command &&
        vdp1_command.colour_mode_documented &&
        vdp1_command.texture_byte_count > 0U &&
        vdp1_command.texture_source_range_valid;
    receipt.vdp1_texture_span_size_matches =
        receipt.vdp1_command_format_matches &&
        captured_texture_span_size == (int)vdp1_command.texture_byte_count;
    receipt.complete_source_binding = receipt.dgn_source_hash_verified &&
        receipt.capture_source_verified &&
        receipt.dgn_source_matches &&
        receipt.structure3_payload_matches && receipt.typed_mesh_corpus_matches &&
        receipt.entry_face_matches && receipt.face_row_matches &&
        receipt.referenced_vertex_rows_match && receipt.normal_row_matches &&
        receipt.fill_selector_matches && receipt.texture_span_matches &&
        receipt.palette_state_matches && receipt.vdp1_state_matches &&
        receipt.transform_state_matches && receipt.normal_culling_state_matches &&
        receipt.vdp1_command_matches && receipt.vdp1_command_format_matches &&
        receipt.vdp1_texture_span_size_matches;
    *out_receipt = receipt;
    return receipt.complete_source_binding ? 0 : -1;
}

int nexus_v1_level_structure3_ordinal_correlation_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3OrdinalCorrelationReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3OrdinalCorrelationReceipt receipt;
    Nexus_V1_DgnStructure3ModelReferenceReceipt model_references;
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.highest_model_index = -1;
    if (!level) {
        *out_receipt = receipt;
        return 0;
    }
    (void)nexus_v1_level_structure3_model_reference_receipt(
        level, &model_references);
    receipt.structure1a_relation_complete = model_references.complete;
    receipt.structure3_payload_valid = level->structure3_payload.valid;
    receipt.structure3_block_count = level->structure3_payload.block_count;
    receipt.structure3_nonzero_byte_run_count =
        level->structure3_payload.nonzero_byte_run_count;
    receipt.structure3_nonzero_block_run_count =
        level->structure3_payload.nonzero_block_run_count;
    receipt.structure3_directory_valid = level->structure3_directory.valid;
    receipt.structure3_directory_entry_count =
        level->structure3_directory.entry_count;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        int model_index;
        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES ||
            !record->structure1a_relation_valid) continue;
        model_index = (int)record->structure1a_structure3_model_index;
        ++receipt.resolved_model_reference_count;
        if (model_index > receipt.highest_model_index) {
            receipt.highest_model_index = model_index;
        }
        if (model_index > receipt.structure3_block_count) {
            ++receipt.model_index_exceeds_block_count;
        }
        if (model_index > receipt.structure3_nonzero_byte_run_count) {
            ++receipt.model_index_exceeds_nonzero_byte_run_count;
        }
        if (model_index > receipt.structure3_nonzero_block_run_count) {
            ++receipt.model_index_exceeds_nonzero_block_run_count;
        }
        if (model_index >= receipt.structure3_block_count) {
            receipt.zero_based_block_ordinal_mapping_disproven = 1;
        }
        if (model_index == 0 || model_index > receipt.structure3_block_count) {
            receipt.one_based_block_ordinal_mapping_disproven = 1;
        }
        if (model_index >= receipt.structure3_nonzero_byte_run_count) {
            receipt.zero_based_byte_run_ordinal_mapping_disproven = 1;
        }
        if (model_index == 0 ||
            model_index > receipt.structure3_nonzero_byte_run_count) {
            receipt.one_based_byte_run_ordinal_mapping_disproven = 1;
        }
        if (model_index >= receipt.structure3_nonzero_block_run_count) {
            receipt.zero_based_run_ordinal_mapping_disproven = 1;
        }
        if (model_index == 0 ||
            model_index > receipt.structure3_nonzero_block_run_count) {
            receipt.one_based_run_ordinal_mapping_disproven = 1;
        }
        if (model_index >= receipt.structure3_directory_entry_count) {
            receipt.zero_based_directory_ordinal_mapping_disproven = 1;
        }
        if (model_index == 0 ||
            model_index > receipt.structure3_directory_entry_count) {
            receipt.one_based_directory_ordinal_mapping_disproven = 1;
        }
    }
    receipt.direct_block_ordinal_mapping_disproven =
        receipt.resolved_model_reference_count > 0 &&
        receipt.zero_based_block_ordinal_mapping_disproven &&
        receipt.one_based_block_ordinal_mapping_disproven;
    receipt.direct_byte_run_ordinal_mapping_disproven =
        receipt.resolved_model_reference_count > 0 &&
        receipt.zero_based_byte_run_ordinal_mapping_disproven &&
        receipt.one_based_byte_run_ordinal_mapping_disproven;
    receipt.direct_run_ordinal_mapping_disproven =
        receipt.resolved_model_reference_count > 0 &&
        receipt.zero_based_run_ordinal_mapping_disproven &&
        receipt.one_based_run_ordinal_mapping_disproven;
    receipt.direct_directory_ordinal_mapping_disproven =
        receipt.resolved_model_reference_count > 0 &&
        receipt.zero_based_directory_ordinal_mapping_disproven &&
        receipt.one_based_directory_ordinal_mapping_disproven;
    receipt.valid = receipt.structure1a_relation_complete &&
        receipt.structure3_payload_valid;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_dgn_structure1_host_provenance_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1HostProvenanceReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1HostProvenanceReceipt receipt;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_MISSING;
    if (!level || level->width <= 0 || level->height <= 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.structure1f_declared = level->geometry_info.structure1f_declared;
    receipt.structure1f_valid = level->geometry_info.structure1f_valid;
    receipt.structure1f_typed_entry_count = level->structure1f_entry_count;
    (void)nexus_v1_level_structure1f_spatial_receipt(
        level, &receipt.structure1f_spatial);
    (void)nexus_v1_level_structure1a_boundary_receipt(
        level, &receipt.structure1a_boundary);
    (void)nexus_v1_level_structure1a_relation_receipt(
        level, &receipt.structure1a_relation);

    if (!receipt.structure1f_declared) {
        receipt.status = NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_READY_ABSENT;
        receipt.can_prepare_runtime_dgn = 1;
    } else if (!receipt.structure1f_valid ||
               !receipt.structure1f_spatial.valid ||
               !receipt.structure1a_boundary.valid) {
        receipt.status =
            NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_BLOCKED_STRUCTURE1F_LAYOUT;
    } else if (receipt.structure1f_spatial.structure1a_bound_entry_count > 0 ||
               receipt.structure1a_boundary.entry_count > 0) {
        if (receipt.structure1a_relation.complete) {
            receipt.status =
                NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_READY_RESOLVED_STRUCTURE1A;
            receipt.can_prepare_runtime_dgn = 1;
        } else {
            receipt.status =
                NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_BLOCKED_STRUCTURE1A_RELATION;
        }
    } else {
        receipt.status = NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_READY_DIRECT;
        receipt.can_prepare_runtime_dgn = 1;
    }
    receipt.blocks_real_dgn_mesh_render =
        receipt.can_prepare_runtime_dgn ? 0 : 1;
    *out_receipt = receipt;
    return 0;
}

const char *nexus_v1_dgn_structure1_host_provenance_status_name(
    Nexus_V1_DgnStructure1HostProvenanceStatus status)
{
    switch (status) {
    case NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_READY_ABSENT:
        return "ready-no-structure1f";
    case NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_READY_DIRECT:
        return "ready-direct-structure1f";
    case NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_READY_RESOLVED_STRUCTURE1A:
        return "ready-resolved-structure1a";
    case NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_BLOCKED_STRUCTURE1F_LAYOUT:
        return "blocked-structure1f-layout";
    case NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_BLOCKED_STRUCTURE1A_RELATION:
        return "blocked-structure1a-relation";
    case NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_MISSING:
    default:
        return "missing";
    }
}

int nexus_v1_level_move_allowed(const Nexus_V1_Level *level,
                                int from_x, int from_y,
                                int to_x, int to_y) {
    Nexus_V1_DgnCellGeometry cell;

    (void)from_x;
    (void)from_y;

    if (nexus_v1_level_get_cell_geometry(level, to_x, to_y, &cell) != 0 ||
        cell.square_type == 0 || cell.collision_ref == 0x0fffU)
        return 0;
    return 1;
}

int nexus_v1_level_dgn_renderer_handoff_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnRendererHandoffReceipt *out_receipt) {
    const Nexus_V1_DgnGeometryInfo *info;

    if (!out_receipt) {
        return -1;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status = NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING;
    out_receipt->fallback_visuals_permitted = 0;

    if (!level || level->width <= 0 || level->height <= 0) {
        return 0;
    }

    info = &level->geometry_info;
    out_receipt->width = level->width;
    out_receipt->height = level->height;
    out_receipt->dmweb_container = info->dmweb_container;
    out_receipt->mesh_ready = info->mesh_ready;
    out_receipt->geometry_offset = info->geometry_offset;
    out_receipt->geometry_size = info->geometry_size;
    out_receipt->collision_ref_count = info->collision_ref_count;
    out_receipt->collision_ref_unique_count =
        info->collision_ref_unique_count;
    out_receipt->max_collision_ref = info->max_collision_ref;
    out_receipt->post_grid_0x30_ref_count =
        info->post_grid_0x30_ref_count;
    out_receipt->post_grid_0x30_ref_unique_count =
        info->post_grid_0x30_ref_unique_count;
    out_receipt->max_post_grid_0x30_ref =
        info->max_post_grid_0x30_ref;
    out_receipt->post_grid_0x30_references_valid =
        info->post_grid_0x30_references_valid;
    out_receipt->post_grid_0x30_invalid_ref_count =
        info->post_grid_0x30_invalid_ref_count;
    out_receipt->first_invalid_post_grid_0x30_ref =
        info->first_invalid_post_grid_0x30_ref;
    out_receipt->post_grid_0x30_ref_value_count =
        info->post_grid_0x30_ref_value_count;
    out_receipt->post_grid_0x24_zero_span_valid =
        info->post_grid_0x24_zero_span_valid;
    out_receipt->post_grid_0x30_record_table_valid =
        info->post_grid_0x30_record_table_valid;
    out_receipt->post_grid_0x30_record_count =
        info->post_grid_0x30_record_count;
    out_receipt->post_grid_0x30_typed_prefix_record_count =
        info->post_grid_0x30_typed_prefix_record_count;
    out_receipt->post_grid_0x30_opaque_tail_record_count =
        info->post_grid_0x30_opaque_tail_record_count;
    out_receipt->post_grid_0x30_row_ordinal_prefix_valid =
        info->post_grid_0x30_row_ordinal_prefix_valid;
    out_receipt->post_grid_0x30_row_ordinal_flagged_prefix_record_count =
        info->post_grid_0x30_row_ordinal_flagged_prefix_record_count;
    out_receipt->post_grid_0x30_first_row_ordinal_flagged_prefix_record =
        info->post_grid_0x30_first_row_ordinal_flagged_prefix_record;
    out_receipt->post_grid_0x30_last_row_ordinal_flagged_prefix_record =
        info->post_grid_0x30_last_row_ordinal_flagged_prefix_record;
    out_receipt->structure1f_declared = info->structure1f_declared;
    out_receipt->structure1f_valid = info->structure1f_valid;
    out_receipt->structure1f_total_entry_count =
        info->structure1f_total_entry_count;
    memcpy(out_receipt->structure1f_family_count,
           info->structure1f_family_count,
           sizeof(out_receipt->structure1f_family_count));
    out_receipt->structure1f_typed_entry_count = level->structure1f_entry_count;
    (void)nexus_v1_level_structure1f_spatial_receipt(
        level, &out_receipt->structure1f_spatial);
    (void)nexus_v1_level_structure1a_boundary_receipt(
        level, &out_receipt->structure1a_boundary);
    (void)nexus_v1_level_structure1a_relation_receipt(
        level, &out_receipt->structure1a_relation);
    (void)nexus_v1_level_structure1a_kind_receipt(
        level, &out_receipt->structure1a_kinds);
    (void)nexus_v1_level_structure3_model_reference_receipt(
        level, &out_receipt->structure3_model_references);
    (void)nexus_v1_level_structure3_directory_receipt(
        level, &out_receipt->structure3_directory);
    (void)nexus_v1_level_structure3_entry_header_receipt(
        level, &out_receipt->structure3_entry_headers);
    (void)nexus_v1_level_structure3_face_receipt(
        level, &out_receipt->structure3_faces);
    (void)nexus_v1_level_structure3_face_material_receipt(
        level, &out_receipt->structure3_face_materials);
    (void)nexus_v1_level_structure3_edge_receipt(
        level, &out_receipt->structure3_edges);
    (void)nexus_v1_level_structure3_vector_receipt(
        level, &out_receipt->structure3_vectors);
    (void)nexus_v1_level_structure3_face_geometry_receipt(
        level, &out_receipt->structure3_face_geometry);
    (void)nexus_v1_level_structure3_face_edge_receipt(
        level, &out_receipt->structure3_face_edges);
    (void)nexus_v1_level_structure3_face_normal_pair_receipt(
        level, &out_receipt->structure3_face_normal_pairs);
    (void)nexus_v1_level_structure3_face_normal_geometry_receipt(
        level, &out_receipt->structure3_face_normal_geometry);
    (void)nexus_v1_level_structure3_mesh_semantic_handoff_receipt(
        level, &out_receipt->structure3_mesh_semantics);
    (void)nexus_v1_level_structure3_attachment_receipt(
        level, &out_receipt->structure3_attachments);
    (void)nexus_v1_level_structure1a_transform_selector_receipt(
        level, &out_receipt->structure1a_transform_selectors);
    (void)nexus_v1_level_structure1f_face_selector_receipt(
        level, &out_receipt->structure1f_face_selectors);
    (void)nexus_v1_level_structure3_model_face_selector_receipt(
        level, &out_receipt->structure3_model_face_selectors);
    (void)nexus_v1_level_structure1f_rotation_selector_receipt(
        level, &out_receipt->structure1f_rotation_selectors);
    (void)nexus_v1_level_structure1f_face_rotation_pair_receipt(
        level, &out_receipt->structure1f_face_rotation_pairs);
    (void)nexus_v1_level_structure1f_offset_pair_receipt(
        level, &out_receipt->structure1f_offset_pairs);
    (void)nexus_v1_level_structure1f_wall_payload_selector_receipt(
        level, &out_receipt->structure1f_wall_payload_selectors);
    (void)nexus_v1_level_structure1f_wall_sensor_destination_receipt(
        level, &out_receipt->structure1f_wall_sensor_destinations);
    (void)nexus_v1_level_structure1f_wall_sensor_control_selector_receipt(
        level, &out_receipt->structure1f_wall_sensor_control_selectors);
    (void)nexus_v1_level_structure1f_wall_sensor_control_destination_tuple_receipt(
        level, &out_receipt->structure1f_wall_sensor_control_destination_tuples);
    (void)nexus_v1_level_structure1f_wall_sensor_model_rotation_pair_receipt(
        level, &out_receipt->structure1f_wall_sensor_model_rotation_pairs);
    (void)nexus_v1_level_structure1f_wall_decoration_model_rotation_pair_receipt(
        level, &out_receipt->structure1f_wall_decoration_model_rotation_pairs);
    (void)nexus_v1_level_structure1f_alcove_payload_selector_receipt(
        level, &out_receipt->structure1f_alcove_payload_selectors);
    (void)nexus_v1_level_structure1f_alcove_payload_rotation_pair_receipt(
        level, &out_receipt->structure1f_alcove_payload_rotation_pairs);
    (void)nexus_v1_level_structure1f_floor_sensor_control_selector_receipt(
        level, &out_receipt->structure1f_floor_sensor_control_selectors);
    (void)nexus_v1_level_structure1f_floor_sensor_destination_receipt(
        level, &out_receipt->structure1f_floor_sensor_destinations);
    (void)nexus_v1_level_structure1f_floor_sensor_model_rotation_pair_receipt(
        level, &out_receipt->structure1f_floor_sensor_model_rotation_pairs);
    (void)nexus_v1_level_structure1f_floor_sensor_extent_pair_receipt(
        level, &out_receipt->structure1f_floor_sensor_extent_pairs);
    (void)nexus_v1_level_structure1f_floor_decoration_payload_selector_receipt(
        level, &out_receipt->structure1f_floor_decoration_payload_selectors);
    (void)nexus_v1_level_structure1f_floor_decoration_rotation_selector_receipt(
        level, &out_receipt->structure1f_floor_decoration_rotation_selectors);
    (void)nexus_v1_level_structure1f_floor_decoration_model_rotation_pair_receipt(
        level, &out_receipt->structure1f_floor_decoration_model_rotation_pairs);
    (void)nexus_v1_level_structure1f_floor_decoration_control_extent_receipt(
        level, &out_receipt->structure1f_floor_decoration_control_extents);
    (void)nexus_v1_level_structure1f_item_attribute_pair_receipt(
        level, &out_receipt->structure1f_item_attribute_pairs);
    (void)nexus_v1_level_structure1f_item_location_pair_receipt(
        level, &out_receipt->structure1f_item_location_pairs);
    (void)nexus_v1_level_structure1f_item_coordinate_pair_receipt(
        level, &out_receipt->structure1f_item_coordinate_pairs);
    (void)nexus_v1_level_structure1f_floor_decoration_offset_pair_receipt(
        level, &out_receipt->structure1f_floor_decoration_offset_pairs);
    (void)nexus_v1_level_structure3_payload_receipt(
        level, &out_receipt->structure3_payload);
    out_receipt->structure1g_present = info->structure1g_present;
    out_receipt->structure1g_valid = info->structure1g_valid;
    out_receipt->structure1g_animated_texture_count =
        info->structure1g_animated_texture_count;
    out_receipt->structure1g_sequence_count = info->structure1g_sequence_count;
    out_receipt->structure1g_floor_animation_cell_count =
        level->structure1g_floor_animation_cell_count;
    out_receipt->structure1g_floor_animation_bound_count =
        level->structure1g_floor_animation_bound_count;
    for (int entry = 0; entry < level->structure1g_entry_count; ++entry) {
        out_receipt->structure1g_image_instruction_count +=
            level->structure1g_entries[entry].image_instruction_count;
        out_receipt->structure1g_goto_instruction_count +=
            level->structure1g_entries[entry].goto_instruction_count;
        out_receipt->structure1g_structure2_image_instruction_bound_count +=
            level->structure1g_entries[entry]
                .structure2_image_instruction_bound_count;
        out_receipt->structure1g_structure2_image_instruction_unbound_count +=
            level->structure1g_entries[entry]
                .structure2_image_instruction_unbound_count;
    }
    out_receipt->structure1g_structure2_bindings_complete =
        level->structure1g_structure2_bindings_complete;
    out_receipt->structure2_descriptor_offset_envelope_valid =
        level->structure2_payload.descriptor_offset_envelope_valid;

    if (!info->dmweb_container) {
        out_receipt->status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_LEGACY_FALLBACK;
    } else if (info->structure1g_present && !info->structure1g_valid) {
        out_receipt->status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE_SEMANTICS;
    } else if (info->structure1g_present &&
               level->structure2_texture_table_valid &&
               level->structure2_payload.valid &&
               !level->structure2_payload.descriptor_offset_envelope_valid) {
        /* A descriptor ID alone is not an admissible original source if one
         * of its raw targets crosses the only proven Structure2 envelope. */
        out_receipt->status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_ENVELOPE;
    } else if (info->structure1g_present &&
               !level->structure1g_structure2_bindings_complete) {
        /* A syntactically bounded Structure1G program still cannot be
         * promoted when any real image instruction misses Structure2. */
        out_receipt->status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_SOURCE;
    } else if (info->structure1f_declared && !info->structure1f_valid) {
        out_receipt->status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_LAYOUT;
    } else if (info->structure1f_valid &&
               out_receipt->structure1f_spatial.valid &&
               out_receipt->structure1f_spatial.structure1a_bound_entry_count > 0) {
        /* A resolved Structure1A owner names only a Structure3 model index.
         * Structure3's mesh/face payload grammar is still unparsed, so do
         * not convert this receipt into a draw or omit it from the scene. */
        if (!out_receipt->structure3_model_references.complete) {
            out_receipt->status =
                NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_SEMANTICS;
        } else if (!out_receipt->structure3_payload.valid) {
            out_receipt->status =
                NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE3_MESH;
        } else {
            out_receipt->status =
                NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE3_FACE_SEMANTICS;
        }
    } else if (info->mesh_ready) {
        out_receipt->status = NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH;
        out_receipt->can_render_dgn_mesh = 1;
    } else if (!info->post_grid_0x30_record_table_valid) {
        out_receipt->status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_NO_GEOMETRY;
    } else if (!info->post_grid_0x30_references_valid) {
        /* The retail corpus proves only the ordinal-typed prefix. A packed
         * Structure1B reference into the opaque tail cannot be presented as
         * a generic descriptor shortage or substituted with fallback art. */
        out_receipt->status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_REFERENCE;
    } else {
        out_receipt->status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_DESCRIPTOR_BUDGET;
    }

    out_receipt->blocks_real_dgn_mesh_render =
        out_receipt->can_render_dgn_mesh ? 0 : 1;
    return 0;
}

const char *nexus_v1_dgn_renderer_handoff_status_name(
    Nexus_V1_DgnRendererHandoffStatus status) {
    switch (status) {
    case NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING: return "missing";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH: return "ready-mesh";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_NO_GEOMETRY:
        return "blocked-no-geometry";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_DESCRIPTOR_BUDGET:
        return "blocked-descriptor-budget";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_LEGACY_FALLBACK:
        return "blocked-legacy-fallback";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE_SEMANTICS:
        return "blocked-structure-semantics";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_SOURCE:
        return "blocked-structure2-source";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_SEMANTICS:
        return "blocked-structure1f-semantics";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_REFERENCE:
        return "blocked-structure1f-reference";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_ENVELOPE:
        return "blocked-structure2-envelope";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_LAYOUT:
        return "blocked-structure1f-layout";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE3_MESH:
        return "blocked-structure3-mesh";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE3_FACE_SEMANTICS:
        return "blocked-structure3-face-semantics";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1B_SELECTOR:
        return "blocked-structure1b-selector";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_CANONICAL_SOURCE:
        return "blocked-canonical-source";
    default: return "unknown";
    }
}

static void nexus_v1_dgn_plan_project_quad(Nexus_V1_DgnRenderCommand *command);

static int nexus_v1_dgn_plan_push(
    Nexus_V1_DgnRenderCommand *commands,
    int max_commands,
    Nexus_V1_DgnRenderPlanReceipt *receipt,
    Nexus_V1_DgnRenderCommand command) {
    if (!receipt) {
        return -1;
    }
    if (!commands || receipt->command_count >= max_commands) {
        receipt->blocks_real_dgn_mesh_render = 1;
        receipt->plan_ready = 0;
        return -1;
    }
    nexus_v1_dgn_plan_project_quad(&command);
    commands[receipt->command_count++] = command;
    receipt->source_cell_count++;
    if (command.kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR) {
        receipt->floor_count++;
        if (command.material_source_kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR) {
            receipt->floor_material_command_count++;
        }
    } else if (command.kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING) {
        receipt->ceiling_count++;
        if (command.material_source_kind ==
            NEXUS_V1_DGN_RENDER_COMMAND_CEILING) {
            receipt->ceiling_material_command_count++;
        }
    } else {
        receipt->wall_count++;
        if (command.material_source_kind != NEXUS_V1_DGN_RENDER_COMMAND_FLOOR &&
            command.material_source_kind != NEXUS_V1_DGN_RENDER_COMMAND_CEILING) {
            receipt->wall_material_command_count++;
        }
    }
    if (command.post_grid_0x30_ref != 0U &&
        command.post_grid_0x30_ref != 0x0FFFU) {
        receipt->post_grid_0x30_reference_command_count++;
        if (command.post_grid_0x30_row_prefix_valid)
            receipt->post_grid_0x30_valid_reference_command_count++;
        if (receipt->first_post_grid_0x30_ref == 0) {
            receipt->first_post_grid_0x30_ref = command.post_grid_0x30_ref;
        }
        if ((int)command.post_grid_0x30_ref >
            receipt->max_post_grid_0x30_ref) {
            receipt->max_post_grid_0x30_ref = command.post_grid_0x30_ref;
        }
    }
    return 0;
}

static Nexus_V1_DgnRenderCommand nexus_v1_dgn_plan_command(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnRenderCommandKind kind,
    int x,
    int y,
    int depth,
    int lateral,
    int wall_dir) {
    Nexus_V1_DgnRenderCommand command;
    memset(&command, 0, sizeof(command));
    command.animated_texture_structure1g_entry_index = -1;
    command.kind = kind;
    command.x = x;
    command.y = y;
    command.depth = depth;
    command.lateral = lateral;
    Nexus_V1_DgnCellGeometry cell;
    (void)nexus_v1_level_get_cell_geometry(level, x, y, &cell);
    command.square_type = cell.square_type;
    command.wall_dir = wall_dir & 3;
    command.collision_ref = cell.collision_ref;
    command.post_grid_0x30_ref = cell.post_grid_0x30_ref;
    command.collision_sector = cell.collision_sector;
    command.post_grid_0x30_row_prefix_valid =
        cell.post_grid_0x30_row_prefix_valid;
    command.floor_rotation = cell.floor_rotation;
    command.floor_slope = cell.floor_slope;
    memcpy(command.floor_height, cell.floor_height, sizeof(command.floor_height));
    memcpy(command.ceiling_height, cell.ceiling_height,
           sizeof(command.ceiling_height));
    command.material_id = (uint8_t)nexus_v1_level_get_material_ref(
        level, x, y, kind, wall_dir);
    if (kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR &&
        level->floor_animation_ids[y][x] != 0xffU) {
        int entry;
        command.animated_texture_declared = 1;
        command.animated_texture_id = level->floor_animation_ids[y][x];
        for (entry = 0; entry < level->structure1g_entry_count; ++entry) {
            if (level->structure1g_entries[entry].animation_id ==
                command.animated_texture_id) {
                command.animated_texture_structure1g_entry_index = entry;
                command.animated_texture_structure1g_sequence_word_offset =
                    level->structure1g_entries[entry].sequence_word_offset;
                command.animated_texture_first_image_index =
                    level->structure1g_entries[entry].first_image_index;
                command.animated_texture_structure2_image_id =
                    level->structure1g_entries[entry].first_structure2_image_id;
                command.animated_texture_structure2_image_valid =
                    level->structure1g_entries[entry].first_structure2_image_valid;
                command.animated_texture_host_route =
                    NEXUS_V1_DGN_ANIMATED_MATERIAL_ROUTE_STRUCTURE2_FLOOR;
                break;
            }
        }
    }
    switch (kind) {
    case NEXUS_V1_DGN_RENDER_COMMAND_FLOOR:
        command.material_source_kind = NEXUS_V1_DGN_RENDER_COMMAND_FLOOR;
        command.palette_index = command.material_id;
        command.draw_order = (uint8_t)(32 - depth);
        break;
    case NEXUS_V1_DGN_RENDER_COMMAND_CEILING:
        command.material_source_kind = NEXUS_V1_DGN_RENDER_COMMAND_CEILING;
        command.palette_index = command.material_id;
        command.draw_order = (uint8_t)(16 - depth);
        break;
    default:
        command.material_source_kind = kind;
        command.palette_index = command.material_id;
        command.draw_order = (uint8_t)(48 - depth);
        break;
    }
    return command;
}

static void nexus_v1_dgn_plan_bind_direct_structure1f(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnRenderCommand *commands,
    Nexus_V1_DgnRenderPlanReceipt *receipt)
{
    int entry_index;

    if (!level || !commands || !receipt ||
        !receipt->structure1f_spatial.valid) {
        return;
    }
    for (entry_index = 0; entry_index < level->structure1f_entry_count;
         ++entry_index) {
        const Nexus_V1_DgnStructure1FEntry *entry =
            &level->structure1f_entries[entry_index];
        int command_index;
        int visible = 0;
        uint8_t family_mask;

        if (entry->family != NEXUS_V1_DGN_STRUCTURE1F_ITEMS &&
            entry->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS &&
            entry->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS) {
            continue;
        }
        switch (entry->family) {
        case NEXUS_V1_DGN_STRUCTURE1F_ITEMS:
            family_mask = NEXUS_V1_DGN_STRUCTURE1F_DIRECT_FAMILY_ITEM;
            break;
        case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS:
            family_mask =
                NEXUS_V1_DGN_STRUCTURE1F_DIRECT_FAMILY_FLOOR_DECORATION;
            break;
        case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS:
            family_mask =
                NEXUS_V1_DGN_STRUCTURE1F_DIRECT_FAMILY_FLOOR_SENSOR;
            break;
        default:
            continue;
        }
        for (command_index = 0; command_index < receipt->command_count;
             ++command_index) {
            if (commands[command_index].x == entry->x &&
                commands[command_index].y == entry->y) {
                visible = 1;
                /* DMWeb's Structure1F families here carry a direct floor
                 * cell coordinate. Retain their provenance on the exact
                 * FLOOR command only; applying it to co-located walls or a
                 * ceiling would invent a material/mesh relation. The visible
                 * record remains a no-draw gate below even if a plan has no
                 * floor command for its source cell. */
                if (commands[command_index].kind !=
                    NEXUS_V1_DGN_RENDER_COMMAND_FLOOR) {
                    continue;
                }
                const int item_already_attached =
                    (commands[command_index].structure1f_direct_family_mask &
                     NEXUS_V1_DGN_STRUCTURE1F_DIRECT_FAMILY_ITEM) != 0;
                if (commands[command_index].structure1f_direct_entry_count ==
                    0) {
                    ++receipt->structure1f_plan_direct_command_count;
                    ++receipt->structure1f_plan_direct_floor_command_count;
                }
                ++commands[command_index].structure1f_direct_entry_count;
                commands[command_index].structure1f_direct_family_mask |=
                    family_mask;
                ++receipt->structure1f_plan_direct_command_entry_count;
                ++receipt->structure1f_plan_direct_floor_command_entry_count;
                if (entry->family == NEXUS_V1_DGN_STRUCTURE1F_ITEMS) {
                    ++receipt->structure1f_plan_item_floor_command_entry_count;
                    if (!item_already_attached) {
                        ++receipt->structure1f_plan_item_floor_command_count;
                    }
                }
            }
        }
        if (!visible) {
            continue;
        }
        ++receipt->structure1f_plan_direct_entry_count;
        switch (entry->family) {
        case NEXUS_V1_DGN_STRUCTURE1F_ITEMS:
            ++receipt->structure1f_plan_item_entry_count;
            break;
        case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS:
            ++receipt->structure1f_plan_floor_decoration_entry_count;
            break;
        case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS:
            ++receipt->structure1f_plan_floor_sensor_entry_count;
            break;
        default:
            break;
        }
    }
}

static int16_t nexus_v1_dgn_view_clamp(int value) {
    if (value < -NEXUS_V1_DGN_VIEWPORT_UNITS) return -NEXUS_V1_DGN_VIEWPORT_UNITS;
    if (value > NEXUS_V1_DGN_VIEWPORT_UNITS * 2) return NEXUS_V1_DGN_VIEWPORT_UNITS * 2;
    return (int16_t)value;
}

static int nexus_v1_dgn_view_x(int lateral_half, int z_half) {
    return (NEXUS_V1_DGN_VIEWPORT_UNITS / 2) +
        (lateral_half * NEXUS_V1_DGN_VIEWPORT_UNITS / 2) / z_half;
}

static int nexus_v1_dgn_view_floor_y(int z_half) {
    return 400 + (768 / z_half);
}

/* Structure1B byte 3 stores signed 1/32 world-unit floor heights. Keep the
 * copied host plan on the same vertical projection as the material viewport:
 * a 32-unit ceiling over a zero-height floor reaches the old ceiling baseline. */
static int nexus_v1_dgn_view_height_y(int z_half, int8_t height) {
    return nexus_v1_dgn_view_floor_y(z_half) -
        ((int)height * 1280) / (32 * z_half);
}

static void nexus_v1_dgn_plan_set_quad(Nexus_V1_DgnRenderCommand *command,
                                       int x0, int y0, int x1, int y1,
                                       int x2, int y2, int x3, int y3) {
    command->quad_x[0] = nexus_v1_dgn_view_clamp(x0);
    command->quad_y[0] = nexus_v1_dgn_view_clamp(y0);
    command->quad_x[1] = nexus_v1_dgn_view_clamp(x1);
    command->quad_y[1] = nexus_v1_dgn_view_clamp(y1);
    command->quad_x[2] = nexus_v1_dgn_view_clamp(x2);
    command->quad_y[2] = nexus_v1_dgn_view_clamp(y2);
    command->quad_x[3] = nexus_v1_dgn_view_clamp(x3);
    command->quad_y[3] = nexus_v1_dgn_view_clamp(y3);
}

static void nexus_v1_dgn_plan_project_quad(Nexus_V1_DgnRenderCommand *command) {
    int near_z = command->depth * 2 + 1;
    int far_z = near_z + 2;
    int left_half = command->lateral * 2 - 1;
    int right_half = left_half + 2;
    int near_left = nexus_v1_dgn_view_x(left_half, near_z);
    int near_right = nexus_v1_dgn_view_x(right_half, near_z);
    int far_left = nexus_v1_dgn_view_x(left_half, far_z);
    int far_right = nexus_v1_dgn_view_x(right_half, far_z);

    switch (command->kind) {
    case NEXUS_V1_DGN_RENDER_COMMAND_FLOOR:
        nexus_v1_dgn_plan_set_quad(command,
            near_left, nexus_v1_dgn_view_height_y(near_z,
                                                   command->floor_height[0]),
            near_right, nexus_v1_dgn_view_height_y(near_z,
                                                    command->floor_height[1]),
            far_right, nexus_v1_dgn_view_height_y(far_z,
                                                   command->floor_height[2]),
            far_left, nexus_v1_dgn_view_height_y(far_z,
                                                  command->floor_height[3]));
        break;
    case NEXUS_V1_DGN_RENDER_COMMAND_CEILING:
        nexus_v1_dgn_plan_set_quad(command,
            near_left, nexus_v1_dgn_view_height_y(near_z,
                                                   command->ceiling_height[0]),
            far_left, nexus_v1_dgn_view_height_y(far_z,
                                                  command->ceiling_height[3]),
            far_right, nexus_v1_dgn_view_height_y(far_z,
                                                   command->ceiling_height[2]),
            near_right, nexus_v1_dgn_view_height_y(near_z,
                                                    command->ceiling_height[1]));
        break;
    case NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT:
        nexus_v1_dgn_plan_set_quad(command,
            near_left, nexus_v1_dgn_view_height_y(near_z,
                                                   command->floor_height[0]),
            near_right, nexus_v1_dgn_view_height_y(near_z,
                                                    command->floor_height[1]),
            near_right, nexus_v1_dgn_view_height_y(near_z,
                                                    command->ceiling_height[1]),
            near_left, nexus_v1_dgn_view_height_y(near_z,
                                                   command->ceiling_height[0]));
        break;
    case NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT:
        nexus_v1_dgn_plan_set_quad(command,
            near_left, nexus_v1_dgn_view_height_y(near_z,
                                                   command->floor_height[0]),
            far_left, nexus_v1_dgn_view_height_y(far_z,
                                                  command->floor_height[3]),
            far_left, nexus_v1_dgn_view_height_y(far_z,
                                                  command->ceiling_height[3]),
            near_left, nexus_v1_dgn_view_height_y(near_z,
                                                   command->ceiling_height[0]));
        break;
    case NEXUS_V1_DGN_RENDER_COMMAND_WALL_RIGHT:
        nexus_v1_dgn_plan_set_quad(command,
            far_right, nexus_v1_dgn_view_height_y(far_z,
                                                   command->floor_height[2]),
            near_right, nexus_v1_dgn_view_height_y(near_z,
                                                    command->floor_height[1]),
            near_right, nexus_v1_dgn_view_height_y(near_z,
                                                    command->ceiling_height[1]),
            far_right, nexus_v1_dgn_view_height_y(far_z,
                                                   command->ceiling_height[2]));
        break;
    default:
        break;
    }
}

static int nexus_v1_dgn_structure3_face_materials_plan_bound(
    const Nexus_V1_DgnRendererHandoffReceipt *handoff)
{
    const Nexus_V1_DgnStructure3FaceReceipt *faces;
    const Nexus_V1_DgnStructure3FaceMaterialReceipt *materials;
    const Nexus_V1_DgnStructure3EdgeReceipt *edges;
    const Nexus_V1_DgnStructure3VectorReceipt *vectors;

    if (!handoff) return 0;
    faces = &handoff->structure3_faces;
    materials = &handoff->structure3_face_materials;
    edges = &handoff->structure3_edges;
    vectors = &handoff->structure3_vectors;
    return materials->face_receipt_valid && materials->valid &&
        faces->valid && materials->face_count == faces->face_count &&
        materials->textured_face_count == faces->textured_face_count &&
        edges->face_receipt_valid && edges->valid &&
        edges->edge_count == faces->triangle_count * 3 +
            faces->quad_count * 4 &&
        edges->unique_edge_count == edges->single_use_edge_count +
            edges->shared_edge_count + edges->nonmanifold_edge_count &&
        !edges->topology_semantics_proven &&
        vectors->face_receipt_valid && vectors->valid &&
        vectors->vertex_count == faces->vertex_count &&
        vectors->normal_count == faces->normal_count &&
        vectors->vertex_vector_count == faces->vertex_count &&
        vectors->normal_vector_count == faces->normal_count &&
        vectors->normal_unit_length_count == faces->normal_count &&
        vectors->normal_non_unit_length_count == 0 &&
        vectors->normal_face_plane_pair_count ==
            faces->triangle_count * 2 + faces->quad_count * 4 &&
        !vectors->transform_or_draw_semantics_proven &&
        materials->static_texture_selector_count ==
            materials->static_texture_bound_count &&
        materials->animated_texture_selector_count ==
            materials->animated_texture_bound_count &&
        materials->static_texture_unbound_count == 0 &&
        materials->animated_texture_unbound_count == 0 &&
        materials->unsupported_textured_fill_count == 0 &&
        materials->selector_bindings_complete &&
        !materials->material_or_draw_semantics_proven;
}

int nexus_v1_level_build_dgn_view_render_plan(
    const Nexus_V1_Level *level,
    int party_x,
    int party_y,
    int party_dir,
    Nexus_V1_DgnRenderCommand *commands,
    int max_commands,
    Nexus_V1_DgnRenderPlanReceipt *out_receipt) {
    static const int dir_dx[4] = {0, 1, 0, -1};
    static const int dir_dy[4] = {-1, 0, 1, 0};
    static const int left_dx[4] = {-1, 0, 1, 0};
    static const int left_dy[4] = {0, -1, 0, 1};
    Nexus_V1_DgnRendererHandoffReceipt handoff;
    Nexus_V1_DgnRenderPlanReceipt receipt;
    int structure3_topology_no_draw;
    int pdir;
    int depth;

    if (!out_receipt) {
        return -1;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING;
    receipt.first_blocking_x = -1;
    receipt.first_blocking_y = -1;
    receipt.first_blocking_depth = -1;
    receipt.fallback_visuals_permitted = 0;
    if (commands && max_commands > 0) {
        memset(commands, 0,
               (size_t)max_commands * sizeof(Nexus_V1_DgnRenderCommand));
    }

    if (!level || max_commands < NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS) {
        receipt.blocks_real_dgn_mesh_render = 1;
        *out_receipt = receipt;
        return 0;
    }
    if (nexus_v1_level_dgn_renderer_handoff_receipt(level, &handoff) != 0) {
        receipt.blocks_real_dgn_mesh_render = 1;
        *out_receipt = receipt;
        return 0;
    }
    receipt.status = handoff.status;
    receipt.post_grid_0x30_row_ordinal_flagged_prefix_record_count =
        handoff.post_grid_0x30_row_ordinal_flagged_prefix_record_count;
    receipt.post_grid_0x30_first_row_ordinal_flagged_prefix_record =
        handoff.post_grid_0x30_first_row_ordinal_flagged_prefix_record;
    receipt.post_grid_0x30_last_row_ordinal_flagged_prefix_record =
        handoff.post_grid_0x30_last_row_ordinal_flagged_prefix_record;
    receipt.structure1f_declared = handoff.structure1f_declared;
    receipt.structure1f_valid = handoff.structure1f_valid;
    receipt.structure1f_total_entry_count = handoff.structure1f_total_entry_count;
    memcpy(receipt.structure1f_family_count, handoff.structure1f_family_count,
           sizeof(receipt.structure1f_family_count));
    receipt.structure1f_typed_entry_count = handoff.structure1f_typed_entry_count;
    receipt.structure1f_spatial = handoff.structure1f_spatial;
    receipt.structure1a_boundary = handoff.structure1a_boundary;
    receipt.structure1a_relation = handoff.structure1a_relation;
    receipt.structure1a_kinds = handoff.structure1a_kinds;
    receipt.structure3_model_references = handoff.structure3_model_references;
    receipt.structure3_directory = handoff.structure3_directory;
    receipt.structure3_entry_headers = handoff.structure3_entry_headers;
    receipt.structure3_faces = handoff.structure3_faces;
    receipt.structure3_face_materials = handoff.structure3_face_materials;
    receipt.structure3_vectors = handoff.structure3_vectors;
    receipt.structure3_face_geometry = handoff.structure3_face_geometry;
    receipt.structure3_face_edges = handoff.structure3_face_edges;
    receipt.structure3_face_normal_pairs = handoff.structure3_face_normal_pairs;
    receipt.structure3_face_normal_geometry = handoff.structure3_face_normal_geometry;
    receipt.structure3_attachments = handoff.structure3_attachments;
    receipt.structure1a_transform_selectors = handoff.structure1a_transform_selectors;
    receipt.structure1f_face_selectors = handoff.structure1f_face_selectors;
    receipt.structure3_model_face_selectors =
        handoff.structure3_model_face_selectors;
    receipt.structure1f_rotation_selectors = handoff.structure1f_rotation_selectors;
    receipt.structure1f_face_rotation_pairs = handoff.structure1f_face_rotation_pairs;
    receipt.structure1f_offset_pairs = handoff.structure1f_offset_pairs;
    receipt.structure1f_wall_payload_selectors =
        handoff.structure1f_wall_payload_selectors;
    receipt.structure1f_wall_sensor_destinations =
        handoff.structure1f_wall_sensor_destinations;
    receipt.structure1f_wall_sensor_control_selectors =
        handoff.structure1f_wall_sensor_control_selectors;
    receipt.structure1f_wall_sensor_control_destination_tuples =
        handoff.structure1f_wall_sensor_control_destination_tuples;
    receipt.structure1f_wall_sensor_model_rotation_pairs =
        handoff.structure1f_wall_sensor_model_rotation_pairs;
    receipt.structure1f_wall_decoration_model_rotation_pairs =
        handoff.structure1f_wall_decoration_model_rotation_pairs;
    receipt.structure1f_alcove_payload_selectors =
        handoff.structure1f_alcove_payload_selectors;
    receipt.structure1f_alcove_payload_rotation_pairs =
        handoff.structure1f_alcove_payload_rotation_pairs;
    receipt.structure1f_floor_sensor_control_selectors =
        handoff.structure1f_floor_sensor_control_selectors;
    receipt.structure1f_floor_sensor_destinations =
        handoff.structure1f_floor_sensor_destinations;
    receipt.structure1f_floor_sensor_model_rotation_pairs =
        handoff.structure1f_floor_sensor_model_rotation_pairs;
    receipt.structure1f_floor_sensor_extent_pairs =
        handoff.structure1f_floor_sensor_extent_pairs;
    receipt.structure1f_floor_decoration_payload_selectors =
        handoff.structure1f_floor_decoration_payload_selectors;
    receipt.structure1f_floor_decoration_rotation_selectors =
        handoff.structure1f_floor_decoration_rotation_selectors;
    receipt.structure1f_floor_decoration_model_rotation_pairs =
        handoff.structure1f_floor_decoration_model_rotation_pairs;
    receipt.structure1f_floor_decoration_control_extents =
        handoff.structure1f_floor_decoration_control_extents;
    receipt.structure1f_item_attribute_pairs = handoff.structure1f_item_attribute_pairs;
    receipt.structure1f_item_location_pairs = handoff.structure1f_item_location_pairs;
    receipt.structure1f_item_coordinate_pairs =
        handoff.structure1f_item_coordinate_pairs;
    receipt.structure1f_floor_decoration_offset_pairs =
        handoff.structure1f_floor_decoration_offset_pairs;
    receipt.structure3_payload = handoff.structure3_payload;
    receipt.structure1g_present = handoff.structure1g_present;
    receipt.structure1g_valid = handoff.structure1g_valid;
    receipt.structure1g_animated_texture_count =
        handoff.structure1g_animated_texture_count;
    receipt.structure1g_sequence_count = handoff.structure1g_sequence_count;
    receipt.structure1g_floor_animation_cell_count =
        handoff.structure1g_floor_animation_cell_count;
    receipt.structure1g_floor_animation_bound_count =
        handoff.structure1g_floor_animation_bound_count;
    receipt.structure1g_image_instruction_count =
        handoff.structure1g_image_instruction_count;
    receipt.structure1g_goto_instruction_count =
        handoff.structure1g_goto_instruction_count;
    receipt.structure1g_structure2_image_instruction_bound_count =
        handoff.structure1g_structure2_image_instruction_bound_count;
    receipt.structure1g_structure2_image_instruction_unbound_count =
        handoff.structure1g_structure2_image_instruction_unbound_count;
    receipt.structure1g_structure2_bindings_complete =
        handoff.structure1g_structure2_bindings_complete;
    /* Preserve bounded source commands for Structure3 only after every
     * parser-validated face fill remains joined to its declared source. This
     * is a no-draw consumer: it retains neither mesh geometry nor pixel,
     * palette, transform, or raster semantics. */
    structure3_topology_no_draw =
        handoff.status ==
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE3_FACE_SEMANTICS &&
        nexus_v1_dgn_structure3_face_materials_plan_bound(&handoff);
    if ((handoff.status != NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH &&
         !structure3_topology_no_draw) ||
        (!handoff.can_render_dgn_mesh && !structure3_topology_no_draw)) {
        receipt.blocks_real_dgn_mesh_render = 1;
        *out_receipt = receipt;
        return 0;
    }

    pdir = party_dir & 3;
    for (depth = 0; depth < NEXUS_V1_DGN_VIEW_DISTANCE; ++depth) {
        int cx = party_x + dir_dx[pdir] * depth;
        int cy = party_y + dir_dy[pdir] * depth;
        int lx = cx + left_dx[pdir];
        int ly = cy + left_dy[pdir];
        int rx = cx - left_dx[pdir];
        int ry = cy - left_dy[pdir];
        int sq = nexus_v1_level_get_square(level, cx, cy);
        int sq_l = nexus_v1_level_get_square(level, lx, ly);
        int sq_r = nexus_v1_level_get_square(level, rx, ry);

        if (sq != 0) {
            if (nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_FLOOR,
                        cx, cy, depth, 0, pdir)) != 0) {
                break;
            }
            if (nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_CEILING,
                        cx, cy, depth, 0, pdir)) != 0) {
                break;
            }
            if (sq_l != 0) {
                (void)nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_FLOOR,
                        lx, ly, depth, -1, pdir));
                (void)nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_CEILING,
                        lx, ly, depth, -1, pdir));
            }
            if (sq_r != 0) {
                (void)nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_FLOOR,
                        rx, ry, depth, 1, pdir));
                (void)nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_CEILING,
                        rx, ry, depth, 1, pdir));
            }
            if (sq_l == 0) {
                (void)nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT,
                        cx, cy, depth, -1, (pdir + 3) & 3));
            }
            if (sq_r == 0) {
                (void)nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_WALL_RIGHT,
                        cx, cy, depth, 1, (pdir + 1) & 3));
            }
        } else {
            if (nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT,
                        cx, cy, depth, 0, (pdir + 2) & 3)) == 0) {
                receipt.first_blocking_x = cx;
                receipt.first_blocking_y = cy;
                receipt.first_blocking_depth = depth;
            }
            break;
        }
        if (receipt.blocks_real_dgn_mesh_render) {
            break;
        }
    }

    if (structure3_topology_no_draw) {
        /* Structure1A owner cells and the bounded Structure3 envelope may be
         * retained by the engine, but the original face grammar remains
         * absent. Keep this command plan source-only. */
        receipt.status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE3_FACE_SEMANTICS;
        receipt.plan_ready = 0;
        receipt.blocks_real_dgn_mesh_render = 1;
        receipt.fallback_visuals_permitted = 0;
        *out_receipt = receipt;
        return 0;
    }
    if (!receipt.blocks_real_dgn_mesh_render) {
        int command_index;
        nexus_v1_dgn_plan_bind_direct_structure1f(level, commands, &receipt);
        /* DMWeb DGN Structure1F documents direct 64x64 source cells for
         * items, floor decorations, and sensors, but not their Saturn draw
         * or trigger ABI. Do not render the surrounding DGN as a complete
         * runtime scene while one is visible: that would silently omit a
         * real record. */
        if (receipt.structure1f_plan_item_entry_count > 0 ||
            receipt.structure1f_plan_floor_decoration_entry_count > 0 ||
            receipt.structure1f_plan_floor_sensor_entry_count > 0) {
            receipt.status =
                NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_SEMANTICS;
            receipt.blocks_real_dgn_mesh_render = 1;
            receipt.fallback_visuals_permitted = 0;
            *out_receipt = receipt;
            return 0;
        }
        for (command_index = 0; command_index < receipt.command_count;
             ++command_index) {
            if (commands[command_index].animated_texture_declared) {
                receipt.animated_material_command_count++;
                /* DMWeb DGN files, Structure1B byte4 / Structure1G / Structure2:
                 * animated-floor IDs route through the local Structure2 image
                 * descriptor (global ID - 0x14c). The host still has no verified
                 * Structure2 payload decoder, so a descriptor is provenance,
                 * not a drawable DMDF/BPK surface or static substitution. */
                receipt.unresolved_animated_material_count++;
            }
        }
        /* A Structure2 descriptor is not a pixel/palette decoder. Keep all
         * declared animated commands no-draw until the engine can supply a
         * separately evidenced image route. Static Structure1B MNS commands
         * do not set this counter and remain eligible for their own route. */
        if (receipt.unresolved_animated_material_count > 0) {
            receipt.status =
                NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_SOURCE;
            receipt.blocks_real_dgn_mesh_render = 1;
            receipt.fallback_visuals_permitted = 0;
            *out_receipt = receipt;
            return 0;
        }
        receipt.material_semantics_complete =
            receipt.floor_material_command_count == receipt.floor_count &&
            receipt.ceiling_material_command_count == receipt.ceiling_count &&
            receipt.wall_material_command_count == receipt.wall_count;
        receipt.plan_ready =
            receipt.command_count > 0 && receipt.material_semantics_complete
                ? 1 : 0;
        receipt.blocks_real_dgn_mesh_render =
            receipt.plan_ready ? 0 : 1;
    }
    *out_receipt = receipt;
    return 0;
}

static int nexus_v1_dgn_structure2_offset_word_bounded(
    const Nexus_V1_Level *level, uint32_t relative_offset)
{
    const Nexus_V1_DgnStructure2Payload *payload;
    uint32_t opaque_start;
    uint32_t opaque_end;

    if (!level) return 0;
    payload = &level->structure2_payload;
    if (!payload->valid || !payload->descriptor_offset_envelope_valid)
        return 0;
    if (relative_offset == 0U) return 1;
    if ((relative_offset & 1U) != 0U || payload->opaque_payload_size < 2)
        return 0;
    opaque_start = (uint32_t)payload->opaque_payload_offset;
    opaque_end = opaque_start + (uint32_t)payload->opaque_payload_size;
    return relative_offset >= opaque_start && relative_offset <= opaque_end - 2U;
}

int nexus_v1_dgn_bind_structure2_animated_floor_sources(
    const Nexus_V1_Level *level, const Nexus_V1_DgnRenderCommand *commands,
    int command_count, Nexus_V1_DgnStructure2FloorCommandSource *out_sources,
    int max_sources, Nexus_V1_DgnStructure2FloorCommandSourceReceipt *out_receipt)
{
    Nexus_V1_DgnStructure2FloorCommandSourceReceipt receipt;
    int command_index;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.fallback_visuals_permitted = 0;
    if (!level || !commands || command_count < 0 || !out_sources ||
        max_sources < 0) {
        *out_receipt = receipt;
        return -1;
    }
    for (command_index = 0; command_index < command_count; ++command_index) {
        const Nexus_V1_DgnRenderCommand *command = &commands[command_index];
        const Nexus_V1_DgnStructure1GEntry *structure1g;
        const Nexus_V1_DgnStructure2Texture *texture;
        Nexus_V1_DgnStructure2FloorCommandSource *source;

        if (!command->animated_texture_declared) continue;
        ++receipt.animated_floor_command_count;
        if (command->kind != NEXUS_V1_DGN_RENDER_COMMAND_FLOOR ||
            command->animated_texture_host_route !=
                NEXUS_V1_DGN_ANIMATED_MATERIAL_ROUTE_STRUCTURE2_FLOOR ||
            !command->animated_texture_structure2_image_valid) {
            ++receipt.blocked_invalid_command_count;
            continue;
        }
        if (command->animated_texture_structure1g_entry_index < 0 ||
            command->animated_texture_structure1g_entry_index >=
                level->structure1g_entry_count) {
            ++receipt.blocked_structure1g_provenance_count;
            continue;
        }
        structure1g = &level->structure1g_entries[
            command->animated_texture_structure1g_entry_index];
        if (structure1g->animation_id != command->animated_texture_id ||
            structure1g->sequence_word_offset !=
                command->animated_texture_structure1g_sequence_word_offset ||
            structure1g->first_image_index !=
                command->animated_texture_first_image_index ||
            !structure1g->first_structure2_image_valid ||
            structure1g->first_structure2_image_id !=
                command->animated_texture_structure2_image_id) {
            ++receipt.blocked_structure1g_provenance_count;
            continue;
        }
        ++receipt.structure1g_provenance_count;
        if (command->animated_texture_first_image_index <
                NEXUS_DGN_STRUCTURE1G_FIRST_IMAGE_INDEX ||
            (uint16_t)(command->animated_texture_first_image_index -
                NEXUS_DGN_STRUCTURE1G_FIRST_IMAGE_INDEX) !=
                command->animated_texture_structure2_image_id) {
            ++receipt.blocked_global_image_index_count;
            continue;
        }
        ++receipt.global_image_index_binding_count;
        if (structure1g->sequence_instruction_count <= 0 ||
            structure1g->structure2_image_instruction_bound_count +
                structure1g->structure2_image_instruction_unbound_count !=
                    structure1g->image_instruction_count ||
            structure1g->structure2_image_instruction_unbound_count != 0) {
            ++receipt.blocked_sequence_provenance_count;
            continue;
        }
        ++receipt.complete_sequence_provenance_count;
        if (!nexus_v1_level_structure2_source_envelope_valid(level) ||
            !level->structure1g_structure2_bindings_complete) {
            ++receipt.blocked_source_envelope_count;
            continue;
        }
        texture = nexus_v1_level_get_structure2_texture(
            level, command->animated_texture_structure2_image_id);
        if (!texture) {
            ++receipt.blocked_missing_descriptor_count;
            continue;
        }
        if (!nexus_v1_dgn_structure2_offset_word_bounded(
                level, texture->image_relative_offset) ||
            !nexus_v1_dgn_structure2_offset_word_bounded(
                level, texture->palette_relative_offset)) {
            ++receipt.blocked_descriptor_offset_envelope_count;
            continue;
        }
        ++receipt.descriptor_offset_envelope_count;
        if (receipt.source_command_count >= max_sources) {
            ++receipt.blocked_invalid_command_count;
            continue;
        }
        source = &out_sources[receipt.source_command_count++];
        memset(source, 0, sizeof(*source));
        source->command_index = command_index;
        source->structure1g_entry_index =
            command->animated_texture_structure1g_entry_index;
        source->structure1g_sequence_word_offset =
            command->animated_texture_structure1g_sequence_word_offset;
        source->structure1g_global_image_index =
            command->animated_texture_first_image_index;
        source->structure1g_sequence_instruction_count =
            structure1g->sequence_instruction_count;
        source->structure1g_sequence_image_instruction_count =
            structure1g->image_instruction_count;
        source->structure1g_sequence_goto_instruction_count =
            structure1g->goto_instruction_count;
        source->structure1g_sequence_bound_image_count =
            structure1g->structure2_image_instruction_bound_count;
        source->structure1g_sequence_unbound_image_count =
            structure1g->structure2_image_instruction_unbound_count;
        source->image_id = texture->image_id;
        source->encoding = texture->encoding;
        source->palette_id = texture->palette_id;
        source->width = texture->width;
        source->height = texture->height;
        source->image_relative_offset = texture->image_relative_offset;
        source->palette_relative_offset = texture->palette_relative_offset;
        source->image_offset_word_bounded = 1;
        source->palette_offset_word_bounded = 1;
        source->structure2_source_envelope_valid = 1;
        source->payload_decoder_proven = 0;
        source->draw_authorized = 0;
    }
    receipt.complete = receipt.animated_floor_command_count > 0 &&
        receipt.structure1g_provenance_count ==
            receipt.animated_floor_command_count &&
        receipt.global_image_index_binding_count ==
            receipt.animated_floor_command_count &&
        receipt.complete_sequence_provenance_count ==
            receipt.animated_floor_command_count &&
        receipt.descriptor_offset_envelope_count ==
            receipt.animated_floor_command_count &&
        receipt.source_command_count == receipt.animated_floor_command_count &&
        receipt.blocked_invalid_command_count == 0 &&
        receipt.blocked_structure1g_provenance_count == 0 &&
        receipt.blocked_global_image_index_count == 0 &&
        receipt.blocked_sequence_provenance_count == 0 &&
        receipt.blocked_descriptor_offset_envelope_count == 0 &&
        receipt.blocked_missing_descriptor_count == 0 &&
        receipt.blocked_source_envelope_count == 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_dgn_bind_direct_structure1f_floor_sources(
    const Nexus_V1_Level *level, const Nexus_V1_DgnRenderCommand *commands,
    int command_count, Nexus_V1_DgnStructure1FDirectFloorCommandSource *out_sources,
    int max_sources,
    Nexus_V1_DgnStructure1FDirectFloorCommandSourceReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FDirectFloorCommandSourceReceipt receipt;
    int entry_index;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.fallback_visuals_permitted = 0;
    if (!level || !commands || command_count < 0 || !out_sources ||
        max_sources < 0) {
        *out_receipt = receipt;
        return -1;
    }
    for (entry_index = 0; entry_index < level->structure1f_entry_count;
         ++entry_index) {
        const Nexus_V1_DgnStructure1FEntry *entry =
            &level->structure1f_entries[entry_index];
        int command_index;

        if (entry->family != NEXUS_V1_DGN_STRUCTURE1F_ITEMS &&
            entry->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS &&
            entry->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS) {
            continue;
        }
        for (command_index = 0; command_index < command_count;
             ++command_index) {
            Nexus_V1_DgnStructure1FDirectFloorCommandSource *source;
            if (commands[command_index].kind !=
                    NEXUS_V1_DGN_RENDER_COMMAND_FLOOR ||
                commands[command_index].x != entry->x ||
                commands[command_index].y != entry->y) {
                continue;
            }
            ++receipt.visible_direct_entry_count;
            if (receipt.floor_command_source_count >= max_sources) {
                ++receipt.blocked_capacity_count;
                continue;
            }
            source = &out_sources[receipt.floor_command_source_count++];
            memset(source, 0, sizeof(*source));
            source->command_index = command_index;
            source->entry_index = entry_index;
            source->entry = *entry;
            source->draw_authorized = 0;
            if (entry->family == NEXUS_V1_DGN_STRUCTURE1F_ITEMS) {
                ++receipt.item_floor_command_source_count;
            } else if (entry->family ==
                       NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS) {
                ++receipt.floor_decoration_command_source_count;
            } else {
                ++receipt.floor_sensor_command_source_count;
            }
        }
    }
    receipt.complete = receipt.visible_direct_entry_count > 0 &&
        receipt.floor_command_source_count == receipt.visible_direct_entry_count &&
        receipt.blocked_capacity_count == 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_dgn_bind_structure1a_owned_cell_sources(
    const Nexus_V1_Level *level, const Nexus_V1_DgnRenderCommand *commands,
    int command_count,
    Nexus_V1_DgnStructure1FStructure1ACommandSource *out_sources,
    int max_sources,
    Nexus_V1_DgnStructure1FStructure1ACommandSourceReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FStructure1ACommandSourceReceipt receipt;
    int entry_index;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.fallback_visuals_permitted = 0;
    if (!level || !commands || command_count < 0 || !out_sources ||
        max_sources < 0) {
        *out_receipt = receipt;
        return -1;
    }
    for (entry_index = 0; entry_index < level->structure1f_entry_count;
         ++entry_index) {
        const Nexus_V1_DgnStructure1FEntry *entry =
            &level->structure1f_entries[entry_index];
        int command_index;

        if (entry->family != NEXUS_V1_DGN_STRUCTURE1F_ALCOVES &&
            entry->family != NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS &&
            entry->family != NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS) {
            continue;
        }
        if (!entry->structure1a_relation_valid) {
            ++receipt.blocked_missing_relation_count;
            continue;
        }
        for (command_index = 0; command_index < command_count;
             ++command_index) {
            Nexus_V1_DgnStructure1FStructure1ACommandSource *source;

            if (commands[command_index].kind !=
                    NEXUS_V1_DGN_RENDER_COMMAND_FLOOR ||
                commands[command_index].x != entry->structure1a_owner_x ||
                commands[command_index].y != entry->structure1a_owner_y) {
                continue;
            }
            ++receipt.visible_owned_entry_count;
            if (receipt.floor_command_source_count >= max_sources) {
                ++receipt.blocked_capacity_count;
                continue;
            }
            source = &out_sources[receipt.floor_command_source_count++];
            memset(source, 0, sizeof(*source));
            source->command_index = command_index;
            source->entry_index = entry_index;
            source->entry = *entry;
            source->owner_x = entry->structure1a_owner_x;
            source->owner_y = entry->structure1a_owner_y;
            source->structure3_model_index =
                entry->structure1a_structure3_model_index;
            source->z_rotation = entry->structure1a_z_rotation;
            source->draw_authorized = 0;
            if (entry->family == NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) {
                ++receipt.alcove_floor_command_source_count;
            } else if (entry->family ==
                       NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS) {
                ++receipt.wall_decoration_floor_command_source_count;
            } else {
                ++receipt.wall_sensor_floor_command_source_count;
            }
        }
    }
    receipt.complete = receipt.visible_owned_entry_count > 0 &&
        receipt.floor_command_source_count == receipt.visible_owned_entry_count &&
        receipt.blocked_missing_relation_count == 0 &&
        receipt.blocked_capacity_count == 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_dgn_bind_structure1a_structure3_topology_candidates(
    const Nexus_V1_Level *level,
    const Nexus_V1_DgnStructure1FStructure1ACommandSource *sources,
    int source_count,
    Nexus_V1_DgnStructure1AStructure3TopologyCandidate *out_candidates,
    int max_candidates,
    Nexus_V1_DgnStructure1AStructure3TopologyCandidateReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1AStructure3TopologyCandidateReceipt receipt;
    int source_index;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.fallback_visuals_permitted = 0;
    if (!level || !sources || source_count < 0 || !out_candidates ||
        max_candidates < 0) {
        *out_receipt = receipt;
        return -1;
    }
    if (!level->structure3_payload.declared ||
        !level->structure3_payload.valid ||
        level->structure3_payload.block_count <= 0 ||
        level->structure3_payload.byte_size <= 0 ||
        level->structure3_payload.complete_block_count !=
            level->structure3_payload.block_count) {
        receipt.blocked_payload_count = source_count;
        *out_receipt = receipt;
        return 0;
    }
    for (source_index = 0; source_index < source_count; ++source_index) {
        const Nexus_V1_DgnStructure1FStructure1ACommandSource *source =
            &sources[source_index];
        const Nexus_V1_DgnStructure1FEntry *entry;
        Nexus_V1_DgnStructure1AStructure3TopologyCandidate *candidate;

        ++receipt.owner_cell_source_count;
        if (source->command_index < 0 || source->entry_index < 0 ||
            source->entry_index >= level->structure1f_entry_count ||
            !source->entry.structure1a_relation_valid ||
            source->owner_x != source->entry.structure1a_owner_x ||
            source->owner_y != source->entry.structure1a_owner_y ||
            source->structure3_model_index !=
                source->entry.structure1a_structure3_model_index ||
            source->z_rotation != source->entry.structure1a_z_rotation) {
            ++receipt.blocked_invalid_source_count;
            continue;
        }
        entry = &level->structure1f_entries[source->entry_index];
        if (entry->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES ||
            entry->family != source->entry.family ||
            entry->tag != source->entry.tag ||
            entry->face != source->entry.face ||
            entry->structure1a_index != source->entry.structure1a_index ||
            !entry->structure1a_relation_valid ||
            entry->structure1a_owner_x != source->owner_x ||
            entry->structure1a_owner_y != source->owner_y ||
            entry->structure1a_structure3_model_index !=
                source->structure3_model_index ||
            entry->structure1a_z_rotation != source->z_rotation) {
            ++receipt.blocked_invalid_source_count;
            continue;
        }
        if (!level->structure1a_table_valid ||
            entry->structure1a_index >=
                (uint16_t)level->structure1a_model_count ||
            level->structure1a_models[entry->structure1a_index]
                    .structure3_model_index != source->structure3_model_index ||
            level->structure1a_models[entry->structure1a_index].z_rotation !=
                source->z_rotation) {
            ++receipt.blocked_invalid_source_count;
            continue;
        }
        if (receipt.topology_candidate_count >= max_candidates) {
            ++receipt.blocked_invalid_source_count;
            continue;
        }
        candidate = &out_candidates[receipt.topology_candidate_count++];
        memset(candidate, 0, sizeof(*candidate));
        candidate->command_index = source->command_index;
        candidate->entry_index = source->entry_index;
        candidate->owner_x = source->owner_x;
        candidate->owner_y = source->owner_y;
        candidate->structure1f_family = entry->family;
        candidate->structure1f_tag = entry->tag;
        candidate->structure1f_face_selector = entry->face;
        candidate->structure1f_structure1a_index = entry->structure1a_index;
        candidate->structure1f_binding_proven = 1;
        candidate->structure1f_face_selector_semantics_proven = 0;
        ++receipt.structure1f_binding_count;
        candidate->structure1a_kind =
            level->structure1a_models[entry->structure1a_index].kind;
        candidate->structure1a_row_binding_proven = 1;
        candidate->structure1a_kind_semantics_proven = 0;
        ++receipt.structure1a_row_binding_count;
        candidate->structure1a_structure3_model_index =
            level->structure1a_models[entry->structure1a_index]
                .structure3_model_index;
        candidate->structure1a_z_rotation =
            level->structure1a_models[entry->structure1a_index].z_rotation;
        candidate->structure1a_model_rotation_binding_proven = 1;
        candidate->structure1a_model_rotation_semantics_proven = 0;
        ++receipt.structure1a_model_rotation_binding_count;
        candidate->structure3_model_index = source->structure3_model_index;
        candidate->z_rotation = source->z_rotation;
        candidate->structure3_block_offset =
            level->structure3_payload.block_offset;
        candidate->structure3_block_count =
            level->structure3_payload.block_count;
        candidate->structure3_byte_size = level->structure3_payload.byte_size;
        candidate->structure3_raw_payload_hash =
            level->structure3_payload.raw_payload_hash;
        candidate->model_index_exceeds_block_count =
            (int)candidate->structure3_model_index >=
                level->structure3_payload.block_count;
        candidate->model_index_exceeds_nonzero_byte_run_count =
            (int)candidate->structure3_model_index >=
                level->structure3_payload.nonzero_byte_run_count;
        candidate->model_index_exceeds_nonzero_block_run_count =
            (int)candidate->structure3_model_index >=
                level->structure3_payload.nonzero_block_run_count;
        candidate->direct_ordinal_mapping_disproven =
            candidate->model_index_exceeds_block_count &&
            candidate->model_index_exceeds_nonzero_byte_run_count &&
            candidate->model_index_exceeds_nonzero_block_run_count;
        if (candidate->direct_ordinal_mapping_disproven) {
            ++receipt.direct_ordinal_mapping_disproven_count;
        }
        candidate->model_ordinal_proven = 0;
        candidate->face_semantics_proven = 0;
        candidate->draw_authorized = 0;
    }
    receipt.complete = receipt.owner_cell_source_count > 0 &&
        receipt.topology_candidate_count == receipt.owner_cell_source_count &&
        receipt.blocked_invalid_source_count == 0 &&
        receipt.blocked_payload_count == 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_item_ibs_parse_verified(const uint8_t *data, int size,
                                     int source_hash_verified,
                                     Nexus_V1_ItemIbsBank *out_bank)
{
    enum {
        ITEM_DECLARATIONS = 0x0800,
        ITEM_DECLARATION_BYTES = 40,
        ITEM_PALETTES = 0x3000,
        ITEM_ASSOCIATIONS = 0x3100,
        ITEM_IMAGES = 0x3300,
        ITEM_IMAGE_BYTES = 128,
        ITEM_FLOOR_DECLARATIONS = 0xa800,
        ITEM_FLOOR_DECLARATION_BYTES = 20,
        ITEM_FLOOR_PAYLOAD_START = 0xb098
    };
    int i;
    if (!out_bank) return -1;
    memset(out_bank, 0, sizeof(*out_bank));
    /* DMWeb Nexus ITEM.IBS: 0x18800-byte table.  The source verifier owns
     * identity; the decoder refuses to make an unverified byte stream live. */
    if (!data || size != NEXUS_V1_ITEM_IBS_BYTES || !source_hash_verified)
        return -1;
    for (i = 0; i < NEXUS_V1_ITEM_IBS_DECLARATION_COUNT; ++i) {
        const uint8_t *decl = data + ITEM_DECLARATIONS +
            i * ITEM_DECLARATION_BYTES;
        if (decl[0] != (uint8_t)i) return -1;
        out_bank->inventory_association[i] = rb16(decl + 0x14);
        out_bank->floor_image[i] = rb16(decl + 0x16);
        out_bank->item_category[i] = decl[1];
        out_bank->item_weight[i] = decl[8];
        out_bank->item_name_string[i] = rb16(decl + 0x18);
        out_bank->item_desc_string[i] = rb16(decl + 0x1a);
        out_bank->item_action1_string[i] = rb16(decl + 0x1c);
        out_bank->item_action2_string[i] = rb16(decl + 0x1e);
        out_bank->item_action3_string[i] = rb16(decl + 0x20);
        /* DMWeb records 19 item declarations without an inventory image as
         * FFFF.  They are valid source records, but cannot become an icon
         * material or a substitute for a floor image. */
        if (out_bank->inventory_association[i] >= 256U &&
            out_bank->inventory_association[i] != 0xffffU) return -1;
    }
    for (i = 0; i < NEXUS_V1_ITEM_IBS_PALETTE_COUNT * 16; ++i) {
        out_bank->palette_bgr555[i / 16][i % 16] =
            rb16(data + ITEM_PALETTES + i * 2);
    }
    for (i = 0; i < 256; ++i) {
        uint8_t palette = data[ITEM_ASSOCIATIONS + i * 2];
        uint8_t image = data[ITEM_ASSOCIATIONS + i * 2 + 1];
        /* FF00 denotes an unused association, not a valid surface. */
        if (palette == 0xffU && image == 0x00U) {
            out_bank->association_palette[i] = 0xffU;
            out_bank->association_image[i] = 0xffU;
        } else if (palette < NEXUS_V1_ITEM_IBS_PALETTE_COUNT &&
                   image < NEXUS_V1_ITEM_IBS_REGULAR_IMAGE_COUNT) {
            out_bank->association_palette[i] = palette;
            out_bank->association_image[i] = image;
        } else {
            return -1;
        }
    }
    memcpy(out_bank->regular_image_texels, data + ITEM_IMAGES,
           sizeof(out_bank->regular_image_texels));
    {
        uint16_t inherited_palette[16] = {0};
        int have_inherited_palette = 0;
        int terminator_offset = -1;
        for (i = 0; i <= NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_COUNT; ++i) {
            const uint8_t *decl = data + ITEM_FLOOR_DECLARATIONS +
                i * ITEM_FLOOR_DECLARATION_BYTES;
            uint16_t image_ordinal = rb16(decl);
            uint32_t image_offset = rb32(decl + 12);
            uint32_t palette_offset = rb32(decl + 16);
            Nexus_V1_ItemIbsFloorImage *floor;
            int color;
            if (image_ordinal == 0xffffU) {
                terminator_offset = (int)image_offset;
                break;
            }
            if (i == NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_COUNT ||
                image_ordinal >= NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_COUNT ||
                out_bank->floor_image_count >= NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_COUNT ||
                image_offset < (uint32_t)(ITEM_FLOOR_PAYLOAD_START - ITEM_FLOOR_DECLARATIONS) ||
                image_offset >= (uint32_t)(NEXUS_V1_ITEM_IBS_BYTES - ITEM_FLOOR_DECLARATIONS))
                return -1;
            floor = &out_bank->floor_images[out_bank->floor_image_count++];
            /* The on-disc declaration uses a 0..108 ordinal.  Item
             * declarations address the combined image space, so its special
             * floor image is regular-image-count + this ordinal.  This is
             * confirmed by the canonical ITEM.IBS where floor refs are
             * 266..331 while these declarations are 43..108. */
            floor->image_id = (uint16_t)(
                NEXUS_V1_ITEM_IBS_REGULAR_IMAGE_COUNT + image_ordinal);
            floor->encoding = rb16(decl + 2);
            floor->palette_id = rb16(decl + 4);
            floor->width = rb16(decl + 6);
            floor->height = rb16(decl + 8);
            floor->image_offset = image_offset;
            if (!floor->width || !floor->height || floor->encoding != 8U)
                return -1;
            if (palette_offset != 0U) {
                if (palette_offset > (uint32_t)(NEXUS_V1_ITEM_IBS_BYTES -
                                                  ITEM_FLOOR_DECLARATIONS - 32))
                    return -1;
                for (color = 0; color < 16; ++color) {
                    inherited_palette[color] = rb16(data + ITEM_FLOOR_DECLARATIONS +
                                                     palette_offset + color * 2);
                }
                have_inherited_palette = 1;
            }
            if (!have_inherited_palette) return -1;
            memcpy(floor->palette_bgr555, inherited_palette,
                   sizeof(floor->palette_bgr555));
            floor->palette_bound = 1;
        }
        if (terminator_offset < ITEM_FLOOR_PAYLOAD_START - ITEM_FLOOR_DECLARATIONS ||
            terminator_offset > NEXUS_V1_ITEM_IBS_BYTES - ITEM_FLOOR_DECLARATIONS)
            return -1;
        for (i = 0; i < out_bank->floor_image_count; ++i) {
            uint32_t next = (uint32_t)terminator_offset;
            int j;
            for (j = 0; j < out_bank->floor_image_count; ++j) {
                uint32_t candidate = out_bank->floor_images[j].image_offset;
                if (candidate > out_bank->floor_images[i].image_offset && candidate < next)
                    next = candidate;
            }
            uint32_t packed_bytes;
            uint32_t expected_packed_bytes;
            if (next <= out_bank->floor_images[i].image_offset) return -1;
            out_bank->floor_images[i].image_bytes =
                next - out_bank->floor_images[i].image_offset;
            expected_packed_bytes =
                ((uint32_t)out_bank->floor_images[i].width *
                 (uint32_t)out_bank->floor_images[i].height + 1U) / 2U;
            /* The verified retail corpus establishes encoding 0008 as a
             * packed 4bpp payload: every descriptor has at least ceil(WxH/2)
             * bytes before the next image, with any remaining bytes being a
             * following 32-byte palette record.  Do not promote a different
             * encoding or a truncated surface. */
            if (out_bank->floor_images[i].encoding != 8U ||
                expected_packed_bytes == 0U ||
                expected_packed_bytes >
                    NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_MAX_PACKED_BYTES ||
                out_bank->floor_images[i].image_bytes < expected_packed_bytes)
                return -1;
            packed_bytes = expected_packed_bytes;
            memcpy(out_bank->floor_images[i].packed_4bpp_texels,
                   data + ITEM_FLOOR_DECLARATIONS +
                       out_bank->floor_images[i].image_offset,
                   packed_bytes);
            out_bank->floor_images[i].packed_4bpp_bytes = packed_bytes;
            out_bank->floor_images[i].packed_4bpp_valid = 1;
        }
    }
    out_bank->source_hash_verified = 1;
    out_bank->valid = 1;
    return 0;
}

int nexus_v1_dgn_bind_structure1f_item_materials(
    const Nexus_V1_Level *level, const Nexus_V1_ItemIbsBank *bank,
    const Nexus_V1_DgnRenderCommand *commands, int command_count,
    Nexus_V1_DgnStructure1FItemMaterialBinding *out_bindings,
    int max_bindings, Nexus_V1_DgnStructure1FItemMaterialReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FItemMaterialReceipt receipt;
    int i;
    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.fallback_visuals_permitted = 0;
    if (!level || !bank || !bank->valid || !bank->source_hash_verified ||
        !commands || command_count < 0 || !out_bindings || max_bindings < 0) {
        *out_receipt = receipt;
        return -1;
    }
    receipt.source_hash_verified = 1;
    for (i = 0; i < level->structure1f_entry_count; ++i) {
        const Nexus_V1_DgnStructure1FEntry *entry =
            &level->structure1f_entries[i];
        int command_index = -1;
        int j;
        uint16_t association;
        uint16_t floor_image;
        uint8_t palette;
        uint8_t image;
        if (entry->family != NEXUS_V1_DGN_STRUCTURE1F_ITEMS) continue;
        ++receipt.item_entry_count;
        if (entry->item_id >= NEXUS_V1_ITEM_IBS_DECLARATION_COUNT) {
            ++receipt.blocked_invalid_item_count;
            continue;
        }
        for (j = 0; j < command_count; ++j) {
            if (commands[j].x == entry->x && commands[j].y == entry->y &&
                commands[j].kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR) {
                command_index = j;
                break;
            }
        }
        if (command_index < 0) {
            ++receipt.blocked_missing_command_count;
            continue;
        }
        ++receipt.command_candidate_count;
        floor_image = bank->floor_image[entry->item_id];
        /* DMWeb ITEM.IBS explicitly defines FFFF as "same as inventory".
         * A separate floor image has a different, still-unproved codec. */
        if (floor_image != 0xffffU) {
            const Nexus_V1_ItemIbsFloorImage *special = NULL;
            for (j = 0; j < bank->floor_image_count; ++j) {
                if (bank->floor_images[j].image_id == floor_image) {
                    special = &bank->floor_images[j];
                    break;
                }
            }
            if (!special || !special->palette_bound ||
                !special->packed_4bpp_valid) {
                ++receipt.blocked_special_floor_image_count;
                continue;
            }
            if (receipt.bound_regular_inventory_count +
                    receipt.bound_special_floor_palette_count >= max_bindings) {
                ++receipt.blocked_invalid_item_count;
                continue;
            }
            j = receipt.bound_regular_inventory_count +
                receipt.bound_special_floor_palette_count;
            out_bindings[j].entry_index = i;
            out_bindings[j].command_index = command_index;
            out_bindings[j].source_x = entry->x;
            out_bindings[j].source_y = entry->y;
            out_bindings[j].item_id = entry->item_id;
            out_bindings[j].palette_index = 0xffU;
            out_bindings[j].image_index = 0xffU;
            out_bindings[j].palette_bgr555 = special->palette_bgr555;
            out_bindings[j].packed_4bpp_texels = NULL;
            out_bindings[j].special_floor_image = special;
            ++receipt.bound_special_floor_palette_count;
            ++receipt.bound_special_floor_texture_count;
            continue;
        }
        association = bank->inventory_association[entry->item_id];
        if (association == 0xffffU) {
            ++receipt.blocked_invalid_item_count;
            continue;
        }
        palette = bank->association_palette[association];
        image = bank->association_image[association];
        if (palette >= NEXUS_V1_ITEM_IBS_PALETTE_COUNT ||
            image >= NEXUS_V1_ITEM_IBS_REGULAR_IMAGE_COUNT ||
            receipt.bound_regular_inventory_count >= max_bindings) {
            ++receipt.blocked_invalid_item_count;
            continue;
        }
        j = receipt.bound_regular_inventory_count +
            receipt.bound_special_floor_palette_count;
        out_bindings[j].entry_index = i;
        out_bindings[j].command_index =
            command_index;
        out_bindings[j].source_x = entry->x;
        out_bindings[j].source_y = entry->y;
        out_bindings[j].item_id =
            entry->item_id;
        out_bindings[j].palette_index =
            palette;
        out_bindings[j].image_index = image;
        out_bindings[j].palette_bgr555 =
            bank->palette_bgr555[palette];
        out_bindings[j].packed_4bpp_texels =
            bank->regular_image_texels[image];
        out_bindings[j].special_floor_image = NULL;
        ++receipt.bound_regular_inventory_count;
    }
    receipt.complete = receipt.item_entry_count > 0 &&
        receipt.blocked_special_floor_image_count == 0 &&
        receipt.blocked_missing_command_count == 0 &&
        receipt.blocked_invalid_item_count == 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_dgn_consume_structure1f_item_floor_materials(
    const Nexus_V1_DgnStructure1FItemMaterialBinding *bindings,
    int binding_count, const Nexus_V1_DgnRenderCommand *commands,
    int command_count, Nexus_V1_DgnCommandPacked4BppMaterial *out_materials,
    int max_materials, Nexus_V1_DgnCommandPacked4BppMaterialReceipt *out_receipt)
{
    Nexus_V1_DgnCommandPacked4BppMaterialReceipt receipt;
    int i;
    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.fallback_visuals_permitted = 0;
    if (!bindings || binding_count < 0 || !commands || command_count < 0 ||
        !out_materials || max_materials < 0) {
        *out_receipt = receipt;
        return -1;
    }
    for (i = 0; i < binding_count; ++i) {
        const Nexus_V1_DgnStructure1FItemMaterialBinding *binding =
            &bindings[i];
        const Nexus_V1_ItemIbsFloorImage *floor =
            binding->special_floor_image;
        Nexus_V1_DgnCommandPacked4BppMaterial *material;
        uint32_t expected_bytes;
        if (!floor) continue;
        ++receipt.special_floor_binding_count;
        if (binding->command_index < 0 || binding->command_index >= command_count ||
            commands[binding->command_index].kind !=
                NEXUS_V1_DGN_RENDER_COMMAND_FLOOR) {
            ++receipt.blocked_invalid_command_count;
            continue;
        }
        if (commands[binding->command_index].x != binding->source_x ||
            commands[binding->command_index].y != binding->source_y) {
            ++receipt.blocked_source_cell_mismatch_count;
            continue;
        }
        ++receipt.source_cell_match_count;
        if (binding->palette_index != 0xffU ||
            binding->image_index != 0xffU ||
            binding->packed_4bpp_texels != NULL) {
            ++receipt.blocked_invalid_binding_count;
            continue;
        }
        expected_bytes = ((uint32_t)floor->width * (uint32_t)floor->height) / 2U;
        if (!floor->palette_bound || !floor->packed_4bpp_valid ||
            floor->encoding != 8U || !floor->width || !floor->height ||
            expected_bytes == 0U || floor->packed_4bpp_bytes != expected_bytes ||
            receipt.command_material_count >= max_materials) {
            ++receipt.blocked_invalid_binding_count;
            continue;
        }
        material = &out_materials[receipt.command_material_count++];
        memset(material, 0, sizeof(*material));
        material->command_index = binding->command_index;
        material->source_entry_index = binding->entry_index;
        material->source_x = binding->source_x;
        material->source_y = binding->source_y;
        material->item_id = binding->item_id;
        material->image_id = floor->image_id;
        material->encoding = floor->encoding;
        material->width = floor->width;
        material->height = floor->height;
        material->packed_4bpp_bytes = floor->packed_4bpp_bytes;
        material->palette_bgr555 = floor->palette_bgr555;
        material->packed_4bpp_texels = floor->packed_4bpp_texels;
        /* The parser can only be reached through a hash-verified bank.
         * Retain that provenance at the DGN command boundary. */
        material->source_hash_verified = 1;
        material->packed_4bpp_valid = 1;
        material->blocked_missing_vdp1_command_provenance = 1;
        material->original_vdp1_capture_verified = 0;
        material->texel_order_proven = 0;
        material->draw_authorized = 0;
        ++receipt.blocked_missing_vdp1_command_provenance_count;
    }
    receipt.source_hash_verified = receipt.command_material_count > 0;
    receipt.original_vdp1_capture_verified_count = 0;
    receipt.complete = receipt.special_floor_binding_count > 0 &&
        receipt.source_cell_match_count == receipt.special_floor_binding_count &&
        receipt.command_material_count == receipt.special_floor_binding_count &&
        receipt.blocked_missing_vdp1_command_provenance_count ==
            receipt.command_material_count &&
        receipt.original_vdp1_capture_verified_count == 0 &&
        receipt.blocked_invalid_binding_count == 0 &&
        receipt.blocked_invalid_command_count == 0 &&
        receipt.blocked_source_cell_mismatch_count == 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_dgn_structure1f_item_ibs_coverage(
    const Nexus_V1_Level *level, const Nexus_V1_ItemIbsBank *bank,
    Nexus_V1_DgnStructure1FItemIbsCoverageReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FItemIbsCoverageReceipt receipt;
    int i;
    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.fallback_visuals_permitted = 0;
    if (!level || !bank || !bank->valid || !bank->source_hash_verified) {
        *out_receipt = receipt;
        return -1;
    }
    receipt.source_hash_verified = 1;
    for (i = 0; i < level->structure1f_entry_count; ++i) {
        const Nexus_V1_DgnStructure1FEntry *entry =
            &level->structure1f_entries[i];
        uint16_t floor_image;
        int j;
        if (entry->family != NEXUS_V1_DGN_STRUCTURE1F_ITEMS) continue;
        ++receipt.dgn_item_entry_count;
        if (entry->item_id >= NEXUS_V1_ITEM_IBS_DECLARATION_COUNT) {
            ++receipt.blocked_invalid_item_count;
            continue;
        }
        floor_image = bank->floor_image[entry->item_id];
        if (floor_image == 0xffffU) {
            ++receipt.inventory_inherited_item_count;
            continue;
        }
        ++receipt.special_floor_reference_count;
        for (j = 0; j < bank->floor_image_count; ++j) {
            const Nexus_V1_ItemIbsFloorImage *floor = &bank->floor_images[j];
            if (floor->image_id != floor_image) continue;
            if (floor->encoding != 8U || !floor->palette_bound ||
                !floor->packed_4bpp_valid) {
                ++receipt.blocked_unsupported_encoding_count;
            } else {
                ++receipt.special_floor_0008_count;
            }
            break;
        }
        if (j == bank->floor_image_count)
            ++receipt.blocked_missing_floor_image_count;
    }
    receipt.complete = receipt.dgn_item_entry_count > 0 &&
        receipt.special_floor_reference_count == receipt.special_floor_0008_count &&
        receipt.blocked_invalid_item_count == 0 &&
        receipt.blocked_missing_floor_image_count == 0 &&
        receipt.blocked_unsupported_encoding_count == 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_vdp1_texture_command_parse(
    const uint8_t *command, int command_size,
    Nexus_V1_Vdp1TextureCommand *out_command)
{
    Nexus_V1_Vdp1TextureCommand parsed;

    if (!out_command) return -1;
    memset(&parsed, 0, sizeof(parsed));
    if (!command || command_size != NEXUS_V1_VDP1_COMMAND_BYTES) {
        *out_command = parsed;
        return -1;
    }
    /* Sega Saturn VDP1 User Manual, command-table Figure 8.3: CMDCTRL,
     * CMDLINK, CMDPMOD, CMDCOLR, CMDSRCA and CMDSIZE are at +00,+02,+04,
     * +06,+08,+0A. SH-2 command-table memory is little-endian; CMDSIZE
     * stores width in eight-pixel units. CMDCOLR remains an opaque field:
     * parsing its position does not prove a palette or CLUT relation. */
    parsed.control = rl16(command);
    parsed.link_word = rl16(command + 2);
    parsed.draw_mode = rl16(command + 4);
    parsed.colour_control = rl16(command + 6);
    parsed.texture_source_word = rl16(command + 8);
    parsed.link_byte_offset = (uint32_t)parsed.link_word * 8U;
    parsed.xa = (int16_t)rl16(command + 12);
    parsed.ya = (int16_t)rl16(command + 14);
    parsed.xb = (int16_t)rl16(command + 16);
    parsed.yb = (int16_t)rl16(command + 18);
    parsed.xc = (int16_t)rl16(command + 20);
    parsed.yc = (int16_t)rl16(command + 22);
    parsed.xd = (int16_t)rl16(command + 24);
    parsed.yd = (int16_t)rl16(command + 26);
    parsed.gouraud_table_word = rl16(command + 28);
    parsed.command_type = (uint8_t)(parsed.control & 0x000fU);
    parsed.colour_mode = (uint8_t)((parsed.draw_mode >> 3) & 0x0007U);
    parsed.texture_width = (uint16_t)((rl16(command + 10) & 0x003fU) * 8U);
    parsed.texture_height = (uint16_t)(rl16(command + 10) >> 8);
    parsed.end_command = (parsed.control & 0x8000U) != 0U;
    parsed.texture_command = !parsed.end_command && parsed.command_type <= 2U;
    parsed.colour_mode_documented = parsed.colour_mode <= 5U;
    if (parsed.texture_command && parsed.colour_mode_documented &&
        parsed.texture_width != 0U && parsed.texture_height != 0U) {
        if (parsed.colour_mode <= 1U) parsed.texture_bits_per_pixel = 4U;
        else if (parsed.colour_mode <= 4U) parsed.texture_bits_per_pixel = 8U;
        else parsed.texture_bits_per_pixel = 16U;
        parsed.texture_byte_count = ((uint32_t)parsed.texture_width *
            (uint32_t)parsed.texture_height *
            (uint32_t)parsed.texture_bits_per_pixel) / 8U;
        /* CMDSRCA is the character-pattern address divided by eight. The
         * resulting range is only a VDP1-VRAM-local bound; no host buffer or
         * captured byte lane is assigned to it without separate trace proof. */
        parsed.texture_source_byte_offset =
            (uint32_t)parsed.texture_source_word * 8U;
        if (parsed.texture_source_byte_offset <= NEXUS_V1_VDP1_VRAM_BYTES &&
            parsed.texture_byte_count <= NEXUS_V1_VDP1_VRAM_BYTES -
                parsed.texture_source_byte_offset) {
            parsed.texture_source_byte_end = parsed.texture_source_byte_offset +
                parsed.texture_byte_count;
            parsed.texture_source_range_valid = 1;
        }
    }
    parsed.four_bpp_colour_bank = parsed.texture_command &&
        parsed.colour_mode == 0U;
    parsed.link_target_range_valid =
        parsed.link_byte_offset <= NEXUS_V1_VDP1_VRAM_BYTES -
            NEXUS_V1_VDP1_COMMAND_BYTES;
    parsed.coordinate_words_framed = parsed.texture_command;
    *out_command = parsed;
    return 0;
}

int nexus_v1_vdp1_lookup_colour_codes_match(
    const uint16_t *decoded, size_t decoded_count,
    const uint16_t *expected, size_t expected_count)
{
    if (!decoded || !expected || decoded_count == 0U ||
        decoded_count != expected_count) return 0;
    return memcmp(decoded, expected, decoded_count * sizeof(*decoded)) == 0;
}

int nexus_v1_vdp1_decode_mode1_lookup_texture(
    const uint8_t *command, int command_size,
    const uint8_t *vdp1_vram, int vdp1_vram_size,
    const uint8_t *texture_span, int texture_span_size,
    uint16_t *out_colour_codes, size_t out_colour_code_count,
    Nexus_V1_Vdp1LookupDecodeReceipt *out_receipt)
{
    Nexus_V1_Vdp1LookupDecodeReceipt receipt;
    Nexus_V1_Vdp1TextureCommand parsed;
    uint32_t lookup_offset;
    uint32_t pixel_count;
    uint32_t pixel;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    if (!command || !vdp1_vram || !texture_span || !out_colour_codes ||
        command_size != NEXUS_V1_VDP1_COMMAND_BYTES ||
        vdp1_vram_size != (int)NEXUS_V1_VDP1_VRAM_BYTES ||
        nexus_v1_vdp1_texture_command_parse(command, command_size, &parsed) != 0 ||
        !parsed.texture_command || parsed.colour_mode != 1U ||
        !parsed.colour_mode_documented || !parsed.texture_source_range_valid ||
        !parsed.texture_byte_count ||
        texture_span_size != (int)parsed.texture_byte_count ||
        parsed.texture_byte_count > (uint32_t)INT_MAX ||
        (parsed.colour_control & 0x0003U) != 0U) {
        *out_receipt = receipt;
        return 0;
    }
    lookup_offset = (uint32_t)parsed.colour_control * 8U;
    pixel_count = (uint32_t)parsed.texture_width *
        (uint32_t)parsed.texture_height;
    if (lookup_offset > NEXUS_V1_VDP1_VRAM_BYTES - 32U ||
        parsed.texture_source_byte_end > NEXUS_V1_VDP1_VRAM_BYTES ||
        pixel_count == 0U || pixel_count > out_colour_code_count) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.command_mode1_lookup = 1;
    receipt.complete_vdp1_vram_snapshot = 1;
    receipt.lookup_table_byte_offset = lookup_offset;
    receipt.lookup_table_in_vram = 1;
    receipt.texture_high_nibble_first = 1;
    receipt.output_pixel_count = pixel_count;
    receipt.output_byte_count = (int)(pixel_count * sizeof(*out_colour_codes));
    receipt.texture_lane_matches_vram = memcmp(texture_span,
        vdp1_vram + parsed.texture_source_byte_offset,
        (size_t)texture_span_size) == 0;
    if (!receipt.texture_lane_matches_vram) {
        *out_receipt = receipt;
        return 0;
    }
    for (pixel = 0U; pixel < pixel_count; ++pixel) {
        uint8_t packed = texture_span[pixel >> 1U];
        uint8_t index = (pixel & 1U) == 0U ? (uint8_t)(packed >> 4U) :
            (uint8_t)(packed & 0x0fU);
        out_colour_codes[pixel] = rl16(vdp1_vram + lookup_offset +
                                        (uint32_t)index * 2U);
    }
    receipt.valid = 1;
    *out_receipt = receipt;
    return 1;
}

static uint8_t nexus_v1_vdp_rgb5_to_u8(uint16_t value, unsigned shift)
{
    return (uint8_t)(((value >> shift) & 0x1fU) << 3);
}

int nexus_v1_vdp1_mode1_palette_pixels_match(
    const Nexus_V1_Vdp1Mode1PalettePixel *resolved, size_t resolved_count,
    const Nexus_V1_Vdp1Mode1PalettePixel *expected, size_t expected_count)
{
    if (!resolved || !expected || resolved_count == 0U ||
        resolved_count != expected_count) return 0;
    return memcmp(resolved, expected,
                  resolved_count * sizeof(*resolved)) == 0;
}

int nexus_v1_vdp1_resolve_mode1_palette_capture(
    const uint8_t *command, int command_size,
    const uint8_t *vdp1_vram, int vdp1_vram_size,
    const uint8_t *texture_span, int texture_span_size,
    const Nexus_V1_Vdp2ColourRamCapture *vdp2_capture,
    Nexus_V1_Vdp1Mode1PalettePixel *out_pixels, size_t out_pixel_count,
    Nexus_V1_Vdp1Mode1PaletteResolveReceipt *out_receipt)
{
    Nexus_V1_Vdp1Mode1PaletteResolveReceipt receipt;
    Nexus_V1_Vdp1LookupDecodeReceipt lookup;
    Nexus_V1_Vdp1TextureCommand parsed;
    uint16_t *raw_codes = NULL;
    uint16_t ramctl;
    uint16_t craofb;
    uint8_t cram_mode;
    uint8_t sprite_offset;
    uint32_t pixel_count;
    uint32_t pixel;
    int current_row_ended = 0;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    if (!command || !vdp1_vram || !texture_span || !vdp2_capture ||
        !out_pixels || command_size != NEXUS_V1_VDP1_COMMAND_BYTES ||
        vdp1_vram_size != (int)NEXUS_V1_VDP1_VRAM_BYTES ||
        !vdp2_capture->colour_ram ||
        vdp2_capture->colour_ram_size != NEXUS_V1_VDP2_CRAM_BYTES ||
        !vdp2_capture->registers ||
        vdp2_capture->registers_size <
            NEXUS_V1_VDP2_CAPTURE_REGISTERS_MIN_BYTES ||
        !vdp2_capture->original_saturn_capture_verified ||
        nexus_v1_vdp1_texture_command_parse(command, command_size, &parsed) != 0 ||
        !parsed.texture_command || parsed.colour_mode != 1U ||
        !parsed.texture_source_range_valid || !parsed.texture_byte_count ||
        parsed.texture_byte_count > (uint32_t)INT_MAX) {
        *out_receipt = receipt;
        return 0;
    }
    pixel_count = (uint32_t)parsed.texture_width *
        (uint32_t)parsed.texture_height;
    if (pixel_count == 0U || pixel_count > out_pixel_count ||
        pixel_count > (uint32_t)(SIZE_MAX / sizeof(*raw_codes))) {
        *out_receipt = receipt;
        return 0;
    }
    ramctl = rl16(vdp2_capture->registers + NEXUS_V1_VDP2_RAMCTL_OFFSET);
    craofb = rl16(vdp2_capture->registers + NEXUS_V1_VDP2_CRAOFB_OFFSET);
    cram_mode = (uint8_t)((ramctl >> 12) & 0x3U);
    sprite_offset = (uint8_t)((craofb >> 4) & 0x7U);
    /* VDP2 RAMCTL CRMD=3 is expressly prohibited by Sega's VDP2 manual. */
    if (cram_mode == 3U) {
        *out_receipt = receipt;
        return 0;
    }
    raw_codes = (uint16_t *)malloc((size_t)pixel_count * sizeof(*raw_codes));
    if (!raw_codes || nexus_v1_vdp1_decode_mode1_lookup_texture(
                          command, command_size, vdp1_vram, vdp1_vram_size,
                          texture_span, texture_span_size, raw_codes,
                          pixel_count, &lookup) != 1 || !lookup.valid) {
        free(raw_codes);
        *out_receipt = receipt;
        return 0;
    }
    receipt.original_saturn_capture_verified = 1;
    receipt.mode1_lookup_bound = 1;
    receipt.vdp2_cram_bound = 1;
    receipt.vdp2_registers_bound = 1;
    receipt.vdp2_cram_mode = cram_mode;
    receipt.vdp2_sprite_colour_ram_offset = sprite_offset;
    receipt.source_index_zero_transparent = (parsed.draw_mode & 0x0040U) == 0U;
    receipt.source_index_f_end_code = (parsed.draw_mode & 0x0080U) == 0U;
    receipt.direct_rgb555_proven = 1;
    receipt.vdp2_cram_address_proven = 1;
    receipt.vdp2_cram_rgb_proven = 1;
    receipt.output_pixel_count = pixel_count;
    receipt.colour_ram_fnv1a64 = nexus_v1_fnv1a64(
        vdp2_capture->colour_ram, (int)vdp2_capture->colour_ram_size);
    receipt.registers_fnv1a64 = nexus_v1_fnv1a64(
        vdp2_capture->registers, (int)vdp2_capture->registers_size);
    for (pixel = 0U; pixel < pixel_count; ++pixel) {
        Nexus_V1_Vdp1Mode1PalettePixel resolved;
        uint8_t packed = texture_span[pixel >> 1U];
        uint8_t index = (pixel & 1U) == 0U ? (uint8_t)(packed >> 4U) :
            (uint8_t)(packed & 0x0fU);
        uint32_t address;

        if (pixel % parsed.texture_width == 0U) current_row_ended = 0;
        memset(&resolved, 0, sizeof(resolved));
        resolved.texture_index = index;
        resolved.raw_colour_code = raw_codes[pixel];
        if (current_row_ended) {
            resolved.kind = NEXUS_V1_VDP1_MODE1_PIXEL_SUPPRESSED_AFTER_END;
        } else if (index == 0U && receipt.source_index_zero_transparent) {
            resolved.kind = NEXUS_V1_VDP1_MODE1_PIXEL_TRANSPARENT;
        } else if (index == 0x0fU && receipt.source_index_f_end_code) {
            resolved.kind = NEXUS_V1_VDP1_MODE1_PIXEL_END_CODE;
            current_row_ended = 1;
        } else if ((resolved.raw_colour_code & 0x8000U) != 0U) {
            /* VDP1 RGB-code format: bit 15 selects direct RGB; B:G:R are
             * 5:5:5 at bits 14..10, 9..5, and 4..0. */
            resolved.kind = NEXUS_V1_VDP1_MODE1_PIXEL_RGB555;
            resolved.red = nexus_v1_vdp_rgb5_to_u8(resolved.raw_colour_code, 0U);
            resolved.green = nexus_v1_vdp_rgb5_to_u8(resolved.raw_colour_code, 5U);
            resolved.blue = nexus_v1_vdp_rgb5_to_u8(resolved.raw_colour_code, 10U);
        } else {
            /* VDP2 sprite colour-bank output supplies 11 dot-colour bits.
             * SPCAOS is added at the high three address bits; CRMD modes 0
             * and 2 mirror/ignore bit 10 as documented. */
            address = (uint32_t)(resolved.raw_colour_code & 0x07ffU) +
                ((uint32_t)sprite_offset << 8);
            if (cram_mode == 0U || cram_mode == 2U) address &= 0x03ffU;
            else address &= 0x07ffU;
            resolved.colour_ram_address = (uint16_t)address;
            if (cram_mode == 2U) {
                const uint8_t *entry = vdp2_capture->colour_ram + address * 4U;
                resolved.kind = NEXUS_V1_VDP1_MODE1_PIXEL_VDP2_CRAM_RGB888;
                resolved.red = entry[0];
                resolved.green = entry[1];
                resolved.blue = entry[2];
            } else {
                uint16_t colour = rl16(vdp2_capture->colour_ram + address * 2U);
                resolved.kind = NEXUS_V1_VDP1_MODE1_PIXEL_VDP2_CRAM_RGB555;
                resolved.red = nexus_v1_vdp_rgb5_to_u8(colour, 0U);
                resolved.green = nexus_v1_vdp_rgb5_to_u8(colour, 5U);
                resolved.blue = nexus_v1_vdp_rgb5_to_u8(colour, 10U);
            }
        }
        out_pixels[pixel] = resolved;
    }
    free(raw_codes);
    receipt.valid = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_item_ibs_bind_0008_vdp1_capture(
    const Nexus_V1_ItemIbsFloorImage *floor,
    const uint8_t *item_ibs, int item_ibs_size, int item_ibs_hash_verified,
    const Nexus_V1_ItemIbs0008Vdp1CaptureCandidate *candidate,
    const uint8_t *captured_texture_span, int captured_texture_span_size,
    const uint8_t *captured_palette_state, int captured_palette_state_size,
    const uint8_t *captured_vdp1_state, int captured_vdp1_state_size,
    const uint8_t *captured_vdp1_command, int captured_vdp1_command_size,
    Nexus_V1_ItemIbs0008Vdp1CaptureBindingReceipt *out_receipt)
{
    Nexus_V1_ItemIbs0008Vdp1CaptureBindingReceipt receipt;
    Nexus_V1_Vdp1TextureCommand command;
    uint32_t expected_bytes;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.fallback_visuals_permitted = 0;
    if (!floor || !item_ibs || item_ibs_size != NEXUS_V1_ITEM_IBS_BYTES ||
        !item_ibs_hash_verified || !candidate || !captured_texture_span ||
        !captured_palette_state || !captured_vdp1_state ||
        !captured_vdp1_command || captured_texture_span_size <= 0 ||
        captured_palette_state_size <= 0 || captured_vdp1_state_size <= 0 ||
        captured_vdp1_command_size <= 0) {
        *out_receipt = receipt;
        return -1;
    }
    expected_bytes = ((uint32_t)floor->width * (uint32_t)floor->height) / 2U;
    receipt.candidate_framing_valid = candidate->item_ibs_fnv1a64 != 0U &&
        candidate->packed_span_fnv1a64 != 0U &&
        candidate->palette_fnv1a64 != 0U && candidate->vdp1_state_fnv1a64 != 0U &&
        candidate->vdp1_command_fnv1a64 != 0U &&
        candidate->vdp1_texture_source_address != 0U &&
        candidate->vdp1_texture_source_bytes == expected_bytes &&
        candidate->texture_first_sequence != 0U &&
        candidate->texture_first_sequence <= candidate->texture_last_sequence &&
        candidate->texture_last_sequence < candidate->vdp1_command_sequence;
    if (!receipt.candidate_framing_valid || floor->encoding != 8U ||
        !floor->palette_bound || !floor->packed_4bpp_valid ||
        floor->packed_4bpp_bytes != expected_bytes ||
        captured_texture_span_size != (int)expected_bytes ||
        captured_palette_state_size != 32) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.item_ibs_source_matches = nexus_v1_fnv1a64(item_ibs,
        (size_t)item_ibs_size) == candidate->item_ibs_fnv1a64;
    receipt.floor_descriptor_matches = candidate->image_id == floor->image_id &&
        candidate->encoding == floor->encoding && candidate->width == floor->width &&
        candidate->height == floor->height;
    receipt.packed_span_matches = nexus_v1_fnv1a64(captured_texture_span,
        (size_t)captured_texture_span_size) == candidate->packed_span_fnv1a64 &&
        memcmp(captured_texture_span, floor->packed_4bpp_texels, expected_bytes) == 0;
    receipt.palette_matches = nexus_v1_fnv1a64(captured_palette_state,
        (size_t)captured_palette_state_size) == candidate->palette_fnv1a64 &&
        memcmp(captured_palette_state, floor->palette_bgr555, 32U) == 0;
    receipt.vdp1_state_matches = nexus_v1_fnv1a64(captured_vdp1_state,
        (size_t)captured_vdp1_state_size) == candidate->vdp1_state_fnv1a64;
    receipt.vdp1_command_matches = nexus_v1_fnv1a64(captured_vdp1_command,
        (size_t)captured_vdp1_command_size) == candidate->vdp1_command_fnv1a64;
    receipt.vdp1_command_format_matches =
        nexus_v1_vdp1_texture_command_parse(captured_vdp1_command,
            captured_vdp1_command_size, &command) == 0 &&
        command.four_bpp_colour_bank &&
        command.texture_source_word == candidate->vdp1_command_source_word &&
        command.texture_width == floor->width && command.texture_height == floor->height;
    receipt.sequence_order_valid = candidate->texture_first_sequence <=
        candidate->texture_last_sequence && candidate->texture_last_sequence <
        candidate->vdp1_command_sequence;
    receipt.original_vdp1_capture_verified = receipt.candidate_framing_valid &&
        receipt.item_ibs_source_matches && receipt.floor_descriptor_matches &&
        receipt.packed_span_matches && receipt.palette_matches &&
        receipt.vdp1_state_matches && receipt.vdp1_command_matches &&
        receipt.vdp1_command_format_matches &&
        receipt.sequence_order_valid;
    receipt.decode_authorized = receipt.original_vdp1_capture_verified;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_item_ibs_decode_0008_vdp1_4bpp(
    const Nexus_V1_ItemIbsFloorImage *floor,
    const Nexus_V1_ItemIbs0008Vdp1CaptureBindingReceipt *capture,
    uint8_t *out_texels, int max_texels,
    Nexus_V1_ItemIbs0008CodecReceipt *out_receipt)
{
    Nexus_V1_ItemIbs0008CodecReceipt receipt;
    uint32_t expected_bytes;
    int texel_count;
    int i;
    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.fallback_visuals_permitted = 0;
    if (!floor || !out_texels || max_texels < 0) {
        *out_receipt = receipt;
        return -1;
    }
    expected_bytes = ((uint32_t)floor->width * (uint32_t)floor->height) / 2U;
    receipt.descriptor_0008_verified = floor->encoding == 8U;
    receipt.packed_span_verified = expected_bytes > 0U &&
        expected_bytes <= NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_MAX_PACKED_BYTES &&
        floor->packed_4bpp_valid && floor->packed_4bpp_bytes == expected_bytes;
    receipt.palette_bound = floor->palette_bound;
    texel_count = (int)(expected_bytes * 2U);
    if (!capture || !capture->original_vdp1_capture_verified ||
        !capture->decode_authorized || capture->fallback_visuals_permitted) {
        receipt.blocked_missing_vdp1_command_provenance = 1;
        *out_receipt = receipt;
        return 0;
    }
    receipt.source_hash_verified = 1;
    if (!receipt.descriptor_0008_verified || !receipt.packed_span_verified ||
        !receipt.palette_bound || texel_count > max_texels) {
        *out_receipt = receipt;
        return 0;
    }
    /* Mednafen Saturn VDP1 TexFetch(), colour modes 0/1: x=0 consumes the
     * high nibble of the first byte.  This is reachable only through the
     * original-command provenance gate above, never from ITEM.IBS alone. */
    for (i = 0; i < (int)expected_bytes; ++i) {
        uint8_t packed = floor->packed_4bpp_texels[i];
        out_texels[i * 2] = (uint8_t)(packed >> 4);
        out_texels[i * 2 + 1] = (uint8_t)(packed & 0x0fU);
    }
    receipt.decoded_texel_count = texel_count;
    receipt.decode_authorized = 1;
    *out_receipt = receipt;
    return 0;
}
