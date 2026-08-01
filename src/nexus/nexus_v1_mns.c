#include "nexus_v1_mns.h"
#include <string.h>

static uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static uint16_t read_be16(const uint8_t *p) {
    return (uint16_t)((p[0] << 8) | p[1]);
}

static int32_t read_be32s(const uint8_t *p) {
    return (int32_t)read_be32(p);
}

static uint32_t bgr555_to_rgba(uint16_t c) {
    int r = (c & 0x1F) << 3;
    int g = ((c >> 5) & 0x1F) << 3;
    int b = ((c >> 10) & 0x1F) << 3;
    return 0xFF000000U | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

static uint32_t fnv1a_pixels(const uint32_t *px, int count) {
    uint32_t h = 0x811C9DC5U;
    int i;
    for (i = 0; i < count; ++i) {
        uint32_t v = px[i];
        h = (h ^ (v & 0xFF)) * 0x01000193U;
        h = (h ^ ((v >> 8) & 0xFF)) * 0x01000193U;
        h = (h ^ ((v >> 16) & 0xFF)) * 0x01000193U;
        h = (h ^ ((v >> 24) & 0xFF)) * 0x01000193U;
    }
    return h;
}

static int parse_mesh(const uint8_t *data, int data_size,
                       uint32_t mesh_off, Nexus_V1_MnsMesh *m) {
    const uint8_t *md;
    int i;

    if ((int)(mesh_off + NEXUS_MNS_MESH_DESC_SIZE) > data_size) return 0;
    md = data + mesh_off;

    m->vertex_count = read_be16(md + 4);
    m->face_count = read_be16(md + 6);
    m->vertex_offset = read_be32(md + 8);
    m->face_offset = read_be32(md + 16);

    if (m->vertex_count > NEXUS_MNS_MAX_VERTICES) m->vertex_count = NEXUS_MNS_MAX_VERTICES;
    if (m->face_count > NEXUS_MNS_MAX_FACES) m->face_count = NEXUS_MNS_MAX_FACES;

    if ((int)(m->vertex_offset + m->vertex_count * NEXUS_MNS_VERTEX_SIZE) > data_size) return 0;
    if ((int)(m->face_offset + m->face_count * NEXUS_MNS_FACE_SIZE) > data_size) return 0;

    for (i = 0; i < m->vertex_count; ++i) {
        const uint8_t *v = data + m->vertex_offset + i * NEXUS_MNS_VERTEX_SIZE;
        m->vertices[i].x = read_be32s(v);
        m->vertices[i].y = read_be32s(v + 4);
        m->vertices[i].z = read_be32s(v + 8);
    }

    for (i = 0; i < m->face_count; ++i) {
        const uint8_t *f = data + m->face_offset + i * NEXUS_MNS_FACE_SIZE;
        m->faces[i].v0 = read_be16(f);
        m->faces[i].v1 = read_be16(f + 2);
        m->faces[i].v2 = read_be16(f + 4);
        m->faces[i].v3 = read_be16(f + 6);
        m->faces[i].flags = read_be16(f + 8);
        m->faces[i].misc = read_be16(f + 10);
        m->faces[i].is_quad = (m->faces[i].v2 != m->faces[i].v3);
        m->faces[i].flip_y = (m->faces[i].misc & 0x8000) != 0;
        m->faces[i].flip_x = (m->faces[i].misc & 0x4000) != 0;
        m->faces[i].texture_index = m->faces[i].misc & 0xFF;
    }
    return 1;
}

int nexus_v1_mns_decode(const uint8_t *data, int data_size,
                         Nexus_V1_MnsDecodeResult *out) {
    uint32_t joints_offset;
    int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || data_size < 0x34) return 0;
    if (read_be32(data) != NEXUS_MNS_MAGIC) return 0;

    out->file_size = read_be32(data + 4);
    out->motn_offset = read_be32(data + 0x20);
    out->text_offset = read_be32(data + 0x24);
    out->joint_count = (int)read_be32(data + 0x28);
    joints_offset = read_be32(data + 0x1C);

    if (out->joint_count > NEXUS_MNS_MAX_JOINTS) out->joint_count = NEXUS_MNS_MAX_JOINTS;

    for (i = 0; i < out->joint_count; ++i) {
        uint32_t joff = joints_offset + (uint32_t)i * NEXUS_MNS_JOINT_SIZE;
        const uint8_t *j;
        Nexus_V1_MnsJoint *jt = &out->joints[i];

        if ((int)(joff + NEXUS_MNS_JOINT_SIZE) > data_size) break;
        j = data + joff;

        jt->index = (int)read_be32(j);
        jt->origin_x = read_be32s(j + 0x0C);
        jt->origin_y = read_be32s(j + 0x10);
        jt->origin_z = read_be32s(j + 0x14);
        jt->mesh_offset = read_be32(j + 0x18);
        jt->sibling_offset = read_be32(j + 0x1C);
        jt->child_offset = read_be32(j + 0x20);

        if (jt->mesh_offset && jt->mesh_offset < (uint32_t)data_size) {
            jt->has_mesh = parse_mesh(data, data_size, jt->mesh_offset, &jt->mesh);
            if (jt->has_mesh) {
                out->total_vertices += jt->mesh.vertex_count;
                out->total_faces += jt->mesh.face_count;
            }
        }
    }

    if (out->text_offset && (int)(out->text_offset + 8) <= data_size) {
        const uint8_t *text = data + out->text_offset;
        if (read_be32(text) == NEXUS_MNS_TEXT_MAGIC) {
            uint32_t text_size = read_be32(text + 4);
            int tex_count = (int)read_be32(text + 8);
            uint32_t desc_start = out->text_offset + 0x24;

            if (tex_count > NEXUS_MNS_MAX_TEXTURES) tex_count = NEXUS_MNS_MAX_TEXTURES;
            (void)text_size;

            for (i = 0; i < tex_count; ++i) {
                uint32_t doff = desc_start + (uint32_t)i * NEXUS_MNS_TEXT_DESC_SIZE;
                Nexus_V1_MnsTextureDesc *td;

                if ((int)(doff + NEXUS_MNS_TEXT_DESC_SIZE) > data_size) break;
                td = &out->textures[i];
                td->image_id = read_be16(data + doff);
                td->encoding = read_be16(data + doff + 2);
                td->width = read_be16(data + doff + 6);
                td->height = read_be16(data + doff + 8);
                td->image_offset = read_be32(data + doff + 12);

                if (td->image_id == 0xFFFF) break;

                td->pixel_count = td->width * td->height;
                if (td->pixel_count > 0 && td->image_offset) {
                    uint32_t abs_off = out->text_offset + td->image_offset;
                    int byte_count = td->pixel_count * 2;
                    if ((int)(abs_off + byte_count) <= data_size) {
                        uint32_t rgba[16384];
                        int j;
                        if (td->pixel_count <= 16384) {
                            const uint8_t *px = data + abs_off;
                            for (j = 0; j < td->pixel_count; ++j) {
                                uint16_t c = read_be16(px + j * 2);
                                rgba[j] = bgr555_to_rgba(c);
                            }
                            td->pixel_hash = fnv1a_pixels(rgba, td->pixel_count);
                        }
                    }
                }
                out->texture_count++;
            }
        }
    }

    /* MOTN section: animation keyframes (DMWeb StrucMotnHeader) */
    if (out->motn_offset && (int)(out->motn_offset + 0x20) <= data_size) {
        const uint8_t *motn = data + out->motn_offset;
        if (read_be32(motn) == NEXUS_MNS_MOTN_MAGIC) {
            int table_count = (int)read_be32(motn + 8);
            int motn_joints = (int)read_be32(motn + 0x0C);
            int t;
            if (table_count > NEXUS_MNS_MAX_MOTN_TABLES)
                table_count = NEXUS_MNS_MAX_MOTN_TABLES;
            out->motn.joint_count_motn = motn_joints;
            out->motn.table_count = table_count;
            for (t = 0; t < table_count; ++t) {
                uint32_t tbl_off_pos = out->motn_offset + 0x10 + (uint32_t)t * 4;
                uint32_t tbl_rel;
                int f_idx = 0;
                if ((int)(tbl_off_pos + 4) > data_size) break;
                tbl_rel = read_be32(data + tbl_off_pos);
                if (tbl_rel == 0 || tbl_rel == 0xFFFFFFFF) break;
                /* Walk frame table offsets */
                {
                    uint32_t idx_base = out->motn_offset + 0x10 + (uint32_t)table_count * 4 + 4;
                    /* Table has N offsets + terminator (0 or 0xFFFFFFFF) */
                    uint32_t entry_pos = out->motn_offset + tbl_rel;
                    while (f_idx < NEXUS_MNS_MAX_MOTN_FRAMES) {
                        uint32_t frame_off;
                        const uint8_t *fp;
                        int frame_size, j;
                        Nexus_V1_MnsMotnFrame *frm;

                        if ((int)(entry_pos + 4) > data_size) break;
                        frame_off = read_be32(data + entry_pos);
                        if (frame_off == 0xFFFFFFFF || frame_off == 0) break;

                        /* Frame data is at idx_base + frame_off */
                        {
                            uint32_t abs_frame = idx_base + frame_off;
                            uint16_t flags;
                            if ((int)(abs_frame + 4) > data_size) break;
                            fp = data + abs_frame;
                            frm = &out->motn.tables[t].frames[f_idx];
                            frm->duration = read_be16(fp);
                            frm->flags = read_be16(fp + 2);
                            flags = frm->flags;
                            frame_size = 20 + motn_joints * 8;
                            if (flags == 2) frame_size += 4;
                            else if (flags == 3) frame_size += 8;
                            else if (flags >= 4) frame_size += 12;
                            if ((int)(abs_frame + frame_size) > data_size) break;
                            for (j = 0; j < motn_joints && j < NEXUS_MNS_MAX_JOINTS; ++j) {
                                int roff = 20 + j * 8;
                                frm->rotation[j][0] = (int16_t)read_be16(fp + roff);
                                frm->rotation[j][1] = (int16_t)read_be16(fp + roff + 2);
                                frm->rotation[j][2] = (int16_t)read_be16(fp + roff + 4);
                            }
                        }
                        f_idx++;
                        entry_pos += 4;
                    }
                    out->motn.tables[t].frame_count = f_idx;
                }
            }
            out->motn.valid = 1;
        }
    }

    out->valid = 1;
    return 1;
}

int nexus_v1_mns_render_texture(const uint8_t *data, int data_size,
                                 const Nexus_V1_MnsDecodeResult *result,
                                 int texture_index,
                                 uint32_t *rgba_out, int rgba_capacity) {
    const Nexus_V1_MnsTextureDesc *td;
    uint32_t abs_off;
    int j;

    if (!result || !result->valid || !rgba_out) return 0;
    if (texture_index < 0 || texture_index >= result->texture_count) return 0;

    td = &result->textures[texture_index];
    if (td->pixel_count <= 0 || rgba_capacity < td->pixel_count) return 0;

    abs_off = result->text_offset + td->image_offset;
    if ((int)(abs_off + td->pixel_count * 2) > data_size) return 0;

    for (j = 0; j < td->pixel_count; ++j) {
        uint16_t c = read_be16(data + abs_off + j * 2);
        rgba_out[j] = bgr555_to_rgba(c);
    }
    return 1;
}
