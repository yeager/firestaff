#include "nexus_v1_dgn.h"
#include <string.h>

static uint16_t read_be16(const uint8_t *p) {
    return (uint16_t)((p[0] << 8) | p[1]);
}

static uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

int nexus_v1_dgn_decode(const uint8_t *data, int data_size,
                         Nexus_V1_DgnDecodeResult *out) {
    uint32_t s1_abs, s2_abs, s3_abs;
    const uint8_t *s1;
    uint32_t s1b_abs;
    int i;
    uint32_t fnv;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || data_size < NEXUS_DGN_BLOCK_SIZE) return 0;
    if (data[0] != 0x01) return 0;

    out->level_number = data[1];
    out->s1_block_offset = read_be16(data + 0x0C);
    out->s1_block_count = read_be16(data + 0x0E);
    out->s1_data_size = read_be32(data + 0x10);
    out->s2_block_offset = read_be16(data + 0x14);
    out->s2_block_count = read_be16(data + 0x16);
    out->s2_data_size = read_be32(data + 0x18);
    out->s3_block_offset = read_be16(data + 0x1C);
    out->s3_block_count = read_be16(data + 0x1E);
    out->s3_data_size = read_be32(data + 0x20);

    s1_abs = (uint32_t)out->s1_block_offset * NEXUS_DGN_BLOCK_SIZE;
    s2_abs = (uint32_t)out->s2_block_offset * NEXUS_DGN_BLOCK_SIZE;
    s3_abs = (uint32_t)out->s3_block_offset * NEXUS_DGN_BLOCK_SIZE;

    if ((int)(s1_abs + out->s1_data_size) > data_size) return 0;
    if ((int)(s2_abs + out->s2_data_size) > data_size) return 0;
    if ((int)(s3_abs + out->s3_data_size) > data_size) return 0;

    s1 = data + s1_abs;
    out->ceiling_tex[0] = s1[0x09];
    out->ceiling_tex[1] = s1[0x0A];
    out->ceiling_tex[2] = s1[0x0B];
    out->model_ref_count = (int)read_be32(s1 + 0x0C);
    out->s1b_offset = read_be32(s1 + 0x14);
    out->s1c_offset = read_be32(s1 + 0x18);
    out->s1e_offset = read_be32(s1 + 0x30);
    out->s1f_offset = read_be32(s1 + 0x34);
    out->s1g_offset = read_be32(s1 + 0x1C);

    s1b_abs = s1_abs + out->s1b_offset;
    if ((int)(s1b_abs + NEXUS_DGN_GRID_BYTES) > data_size) return 0;

    fnv = 0x811C9DC5U;
    for (i = 0; i < NEXUS_DGN_GRID_CELLS; ++i) {
        const uint8_t *cell = data + s1b_abs + i * NEXUS_DGN_CELL_SIZE;
        uint16_t fw = read_be16(cell);
        uint8_t b2 = cell[2];
        int floor_tex = (fw >> 7) & 0x1F;
        int j;

        if (b2 == 0x01 || b2 == 0x02 || b2 == 0x03 || b2 == 0x0B) {
            out->wall_cell_count++;
        } else if (floor_tex > 0) {
            out->open_cell_count++;
        }

        for (j = 0; j < NEXUS_DGN_CELL_SIZE; ++j) {
            fnv = (fnv ^ cell[j]) * 0x01000193U;
        }
    }
    out->grid_hash = fnv;

    {
        const uint8_t *s2 = data + s2_abs;
        int tex_count = 0;
        uint32_t s2fnv = 0x811C9DC5U;

        for (i = 0; i < NEXUS_DGN_MAX_TEXTURES; ++i) {
            int off = i * NEXUS_DGN_TEX_DESC_SIZE;
            uint16_t img_id;
            if ((uint32_t)(off + NEXUS_DGN_TEX_DESC_SIZE) > out->s2_data_size) break;
            img_id = read_be16(s2 + off);
            if (img_id == 0xFFFF) break;
            tex_count++;
        }
        out->texture_count = tex_count;

        for (i = 0; (uint32_t)i < out->s2_data_size; ++i) {
            s2fnv = (s2fnv ^ s2[i]) * 0x01000193U;
        }
        out->s2_hash = s2fnv;
    }

    if (out->s1e_offset && s1_abs + out->s1e_offset + 16 <= (uint32_t)data_size) {
        const uint8_t *doors = data + s1_abs + out->s1e_offset;
        int dc = 0;
        for (i = 0; i < NEXUS_DGN_MAX_DOORS; ++i) {
            if (doors[i * 16] == 0xFF) break;
            dc++;
        }
        out->door_count = dc;
    }

    out->valid = 1;
    return 1;
}
