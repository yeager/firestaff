#include "nexus_v1_mns.h"
#include <limits.h>
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

static int range_in_file(uint32_t offset, uint64_t length, int data_size) {
    return data_size >= 0 && (uint64_t)offset <= (uint64_t)data_size &&
           length <= (uint64_t)data_size - (uint64_t)offset;
}

static uint32_t bgr555_to_rgba(uint16_t c) {
    int r = ((c >> 10) & 0x1F) << 3;
    int g = ((c >> 5) & 0x1F) << 3;
    int b = (c & 0x1F) << 3;
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

    if (!range_in_file(mesh_off, NEXUS_MNS_MESH_DESC_SIZE, data_size)) return 0;
    md = data + mesh_off;

    m->vertex_count = read_be16(md + 4);
    m->face_count = read_be16(md + 6);
    m->vertex_offset = read_be32(md + 8);
    m->face_offset = read_be32(md + 16);

    /* DMWeb's declared counts are part of the DMDF source envelope. Never
     * accept a prefix by silently clipping a retail declaration to the host
     * buffer; an unrepresentable mesh must remain unavailable. */
    if (m->vertex_count > NEXUS_MNS_MAX_VERTICES ||
        m->face_count > NEXUS_MNS_MAX_FACES) return 0;

    if (!range_in_file(m->vertex_offset,
                       (uint64_t)m->vertex_count * NEXUS_MNS_VERTEX_SIZE,
                       data_size) ||
        !range_in_file(m->face_offset,
                       (uint64_t)m->face_count * NEXUS_MNS_FACE_SIZE,
                       data_size)) return 0;

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
    uint32_t declared_joint_count;
    int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || data_size < 0x34) return 0;
    if (read_be32(data) != NEXUS_MNS_MAGIC) return 0;

    out->file_size = read_be32(data + 4);
    /* DMWeb MNS study: DMDF+04 is the block size and is equal to the file
     * size.  Do not decode a truncated prefix (or an appended synthetic
     * tail) as a retail model. */
    if (out->file_size != (uint32_t)data_size) return 0;
    out->motn_offset = read_be32(data + 0x20);
    out->text_offset = read_be32(data + 0x24);
    declared_joint_count = read_be32(data + 0x28);
    out->joint_count = (int)declared_joint_count;
    joints_offset = read_be32(data + 0x1C);

    /* Preserve source identity: a declared skeleton larger than the bounded
     * host representation is invalid for this route, not a request to drop
     * the tail joints. */
    if (declared_joint_count > NEXUS_MNS_MAX_JOINTS) return 0;

    for (i = 0; i < out->joint_count; ++i) {
        uint32_t joff = joints_offset + (uint32_t)i * NEXUS_MNS_JOINT_SIZE;
        const uint8_t *j;
        Nexus_V1_MnsJoint *jt = &out->joints[i];

        if (!range_in_file(joff, NEXUS_MNS_JOINT_SIZE, data_size)) return 0;
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

    /* DMWeb StrucTextHeader contains the magic, section size and texture
     * count. The count is at +0x08, so the complete 12-byte prefix must be
     * present before any of those fields are read. */
    if (out->text_offset &&
        range_in_file(out->text_offset, 12U, data_size)) {
        const uint8_t *text = data + out->text_offset;
        if (read_be32(text) == NEXUS_MNS_TEXT_MAGIC) {
            uint32_t text_size = read_be32(text + 4);
            uint32_t declared_tex_count = read_be32(text + 8);
            int tex_count;
            uint32_t desc_start = out->text_offset + 0x24;

            /* DMWeb StrucTextHeader: +04 is the complete TEXT section size. */
            if (text_size < 12 ||
                !range_in_file(out->text_offset, text_size, data_size))
                return 0;
            if (declared_tex_count > NEXUS_MNS_MAX_TEXTURES) return 0;
            tex_count = (int)declared_tex_count;
            for (i = 0; i < tex_count; ++i) {
                uint32_t doff = desc_start + (uint32_t)i * NEXUS_MNS_TEXT_DESC_SIZE;
                Nexus_V1_MnsTextureDesc *td;

                if ((uint64_t)doff + NEXUS_MNS_TEXT_DESC_SIZE <=
                        (uint64_t)out->text_offset + text_size &&
                    range_in_file(doff, NEXUS_MNS_TEXT_DESC_SIZE,
                                  data_size)) {
                td = &out->textures[i];
                td->image_id = read_be16(data + doff);
                td->encoding = read_be16(data + doff + 2);
                td->width = read_be16(data + doff + 6);
                td->height = read_be16(data + doff + 8);
                td->image_offset = read_be32(data + doff + 12);

                if (td->image_id == 0xFFFF) break;

                if ((uint64_t)td->width * td->height > INT_MAX) {
                    td->pixel_count = 0;
                    out->texture_count++;
                    continue;
                }
                td->pixel_count = td->width * td->height;
                if (td->pixel_count > 0 && td->image_offset) {
                    uint32_t abs_off = out->text_offset + td->image_offset;
                    int byte_count = td->pixel_count * 2;
                    if ((uint64_t)td->image_offset + (uint64_t)byte_count <=
                            (uint64_t)text_size &&
                        range_in_file(abs_off, (uint64_t)byte_count,
                                      data_size)) {
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
                } else {
                    return 0;
                }
            }
        }
    }

    /* MOTN section: animation keyframes (DMWeb StrucMotnHeader).  Its
     * declared section size is part of the source envelope; validate that
     * envelope before the bounded offset walk below. */
    if (out->motn_offset && (int)(out->motn_offset + 0x20) <= data_size) {
        const uint8_t *motn = data + out->motn_offset;
        if (read_be32(motn) == NEXUS_MNS_MOTN_MAGIC) {
            uint32_t motn_size = read_be32(motn + 4);
            uint32_t declared_table_count = read_be32(motn + 8);
            uint32_t declared_motn_joints = read_be32(motn + 0x0C);
            int table_count;
            int motn_joints;
            int t;
            if (motn_size < 0x20 ||
                !range_in_file(out->motn_offset, motn_size, data_size))
                return 0;
            if (declared_table_count > NEXUS_MNS_MAX_MOTN_TABLES ||
                declared_motn_joints > NEXUS_MNS_MAX_JOINTS) return 0;
            table_count = (int)declared_table_count;
            motn_joints = (int)declared_motn_joints;
            out->motn.joint_count_motn = motn_joints;
            out->motn.table_count = table_count;
            for (t = 0; t < table_count; ++t) {
                uint32_t tbl_off_pos = out->motn_offset + 0x10 + (uint32_t)t * 4;
                uint32_t tbl_rel;
                int f_idx = 0;
                if (!range_in_file(tbl_off_pos, 4U, data_size)) break;
                tbl_rel = read_be32(data + tbl_off_pos);
                if (tbl_rel == 0 || tbl_rel == 0xFFFFFFFF) break;
                /* Walk frame offset list (DMWeb: each table is N offsets
                 * relative to MOTN section start, terminated by
                 * 0xFFFFFFFF or 0x00000000) */
                {
                    uint32_t entry_pos = out->motn_offset + tbl_rel;
                    while (f_idx < NEXUS_MNS_MAX_MOTN_FRAMES) {
                        uint32_t frame_rel;
                        uint32_t abs_frame;
                        const uint8_t *fp;
                        int frame_size, j;
                        Nexus_V1_MnsMotnFrame *frm;
                        uint16_t flags;

                        if (!range_in_file(entry_pos, 4U, data_size)) break;
                        frame_rel = read_be32(data + entry_pos);
                        if (frame_rel == 0xFFFFFFFF || frame_rel == 0) break;

                        abs_frame = out->motn_offset + frame_rel;
                        if (!range_in_file(abs_frame, 4U, data_size)) break;
                        fp = data + abs_frame;
                        frm = &out->motn.tables[t].frames[f_idx];
                        frm->duration = read_be16(fp);
                        frm->flags = read_be16(fp + 2);
                        flags = frm->flags;
                        frame_size = 20 + motn_joints * 8;
                        if (flags == 2) frame_size += 4;
                        else if (flags == 3) frame_size += 8;
                        else if (flags >= 4) frame_size += 12;
                        if (!range_in_file(abs_frame, (uint64_t)frame_size,
                                           data_size)) break;
                        for (j = 0; j < motn_joints && j < NEXUS_MNS_MAX_JOINTS; ++j) {
                            int roff = 20 + j * 8;
                            frm->rotation[j][0] = (int16_t)read_be16(fp + roff);
                            frm->rotation[j][1] = (int16_t)read_be16(fp + roff + 2);
                            frm->rotation[j][2] = (int16_t)read_be16(fp + roff + 4);
                        }
                        f_idx++;
                        entry_pos += 4;
                    }
                    if (f_idx == NEXUS_MNS_MAX_MOTN_FRAMES) {
                        uint32_t next_frame;
                        if (!range_in_file(entry_pos, 4U, data_size)) return 0;
                        next_frame = read_be32(data + entry_pos);
                        if (next_frame != 0U && next_frame != 0xFFFFFFFFU)
                            return 0;
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

/* ═══════════════════════════════════════════════════════════════════
 * Animation playback
 * DMWeb: rotation unit ≈ value / 150.0 → degrees
 * ═══════════════════════════════════════════════════════════════════ */

void nexus_v1_mns_anim_init(Nexus_V1_MnsAnimState *state) {
    if (!state) return;
    state->table_index = -1;
    state->frame_index = 0;
    state->tick_accumulator = 0;
    state->looping = 0;
    state->finished = 0;
}

void nexus_v1_mns_anim_play(Nexus_V1_MnsAnimState *state,
                             int table_index, int loop) {
    if (!state) return;
    state->table_index = table_index;
    state->frame_index = 0;
    state->tick_accumulator = 0;
    state->looping = loop;
    state->finished = 0;
}

int nexus_v1_mns_anim_tick(Nexus_V1_MnsAnimState *state,
                            const Nexus_V1_MnsMotnResult *motn) {
    const Nexus_V1_MnsMotnTable *tbl;
    const Nexus_V1_MnsMotnFrame *frm;

    if (!state || !motn || !motn->valid) return 0;
    if (state->table_index < 0 || state->table_index >= motn->table_count)
        return 0;
    if (state->finished) return 0;

    tbl = &motn->tables[state->table_index];
    if (tbl->frame_count == 0) return 0;

    frm = &tbl->frames[state->frame_index];
    state->tick_accumulator++;

    if (state->tick_accumulator >= (int)frm->duration) {
        state->tick_accumulator = 0;
        state->frame_index++;
        if (state->frame_index >= tbl->frame_count) {
            if (state->looping) {
                state->frame_index = 0;
            } else {
                state->frame_index = tbl->frame_count - 1;
                state->finished = 1;
            }
        }
        return 1;
    }
    return 0;
}

static int16_t lerp_i16(int16_t a, int16_t b, int num, int den) {
    if (den <= 0) return a;
    return (int16_t)(a + ((int)(b - a) * num) / den);
}

int nexus_v1_mns_anim_sample(const Nexus_V1_MnsDecodeResult *result,
                              const Nexus_V1_MnsAnimState *state,
                              Nexus_V1_MnsPose *out) {
    const Nexus_V1_MnsMotnTable *tbl;
    const Nexus_V1_MnsMotnFrame *cur, *nxt;
    int i, jcount, next_idx;

    if (!result || !result->valid || !state || !out) return 0;
    memset(out, 0, sizeof(*out));
    out->joint_count = result->joint_count;

    /* No animation: return rest pose (origins only, zero rotation) */
    if (state->table_index < 0 || !result->motn.valid ||
        state->table_index >= result->motn.table_count) {
        for (i = 0; i < result->joint_count; ++i) {
            out->joints[i].world_x = result->joints[i].origin_x;
            out->joints[i].world_y = result->joints[i].origin_y;
            out->joints[i].world_z = result->joints[i].origin_z;
        }
        return 1;
    }

    tbl = &result->motn.tables[state->table_index];
    if (tbl->frame_count == 0) return 0;

    cur = &tbl->frames[state->frame_index];
    jcount = result->motn.joint_count_motn;
    if (jcount > result->joint_count) jcount = result->joint_count;

    /* Determine next frame for interpolation */
    next_idx = state->frame_index + 1;
    if (next_idx >= tbl->frame_count)
        next_idx = state->looping ? 0 : tbl->frame_count - 1;
    nxt = &tbl->frames[next_idx];

    /* Interpolate rotations between current and next frame */
    for (i = 0; i < result->joint_count; ++i) {
        out->joints[i].world_x = result->joints[i].origin_x;
        out->joints[i].world_y = result->joints[i].origin_y;
        out->joints[i].world_z = result->joints[i].origin_z;
        if (i < jcount) {
            int dur = (int)cur->duration;
            out->joints[i].rot_x = lerp_i16(cur->rotation[i][0],
                nxt->rotation[i][0], state->tick_accumulator, dur);
            out->joints[i].rot_y = lerp_i16(cur->rotation[i][1],
                nxt->rotation[i][1], state->tick_accumulator, dur);
            out->joints[i].rot_z = lerp_i16(cur->rotation[i][2],
                nxt->rotation[i][2], state->tick_accumulator, dur);
        }
    }
    return 1;
}

/* Fixed-point sine/cosine (Q15). Input: raw MOTN rotation unit.
 * DMWeb: value / 150.0 ≈ degrees. We convert to a 16-bit angle
 * and use a small lookup approximation. */
static int32_t mns_sin_q15(int16_t motn_rot) {
    /* Convert MOTN units to radians: motn_rot / 150.0 * PI / 180.0
     * ≈ motn_rot * 0.000116355  (very small per unit)
     * For fixed-point: multiply by (2^15 * sin_factor) */
    /* Use Taylor approx for small-to-medium angles */
    int32_t angle_q15;
    int32_t x, x2, x3;
    /* radians * 2^15 = motn_rot * (PI/180/150) * 32768
     *                = motn_rot * 3.812  (approx) */
    angle_q15 = (int32_t)motn_rot * 3812 / 1000;
    x = angle_q15;
    x2 = (x * x) >> 15;
    x3 = (x2 * x) >> 15;
    /* sin(x) ≈ x - x³/6 */
    return x - x3 / 6;
}

static int32_t mns_cos_q15(int16_t motn_rot) {
    int32_t angle_q15;
    int32_t x2, x4;
    angle_q15 = (int32_t)motn_rot * 3812 / 1000;
    x2 = (angle_q15 * angle_q15) >> 15;
    x4 = (x2 * x2) >> 15;
    /* cos(x) ≈ 1 - x²/2 + x⁴/24 */
    return 32768 - x2 / 2 + x4 / 24;
}

/* Rotate vertex (x,y,z) by Euler angles (rx,ry,rz) using XYZ order.
 * All math in Q15 fixed-point. */
static void rotate_vertex(int32_t *x, int32_t *y, int32_t *z,
                           int16_t rx, int16_t ry, int16_t rz) {
    int32_t cx, sx, cy, sy, cz, sz;
    int32_t nx, ny, nz, tx, ty;

    /* Rotate around X */
    cx = mns_cos_q15(rx); sx = mns_sin_q15(rx);
    ny = ((*y) * cx - (*z) * sx) >> 15;
    nz = ((*y) * sx + (*z) * cx) >> 15;
    *y = ny; *z = nz;

    /* Rotate around Y */
    cy = mns_cos_q15(ry); sy = mns_sin_q15(ry);
    nx = ((*x) * cy + (*z) * sy) >> 15;
    nz = (-(*x) * sy + (*z) * cy) >> 15;
    *x = nx; *z = nz;

    /* Rotate around Z */
    cz = mns_cos_q15(rz); sz = mns_sin_q15(rz);
    tx = ((*x) * cz - (*y) * sz) >> 15;
    ty = ((*x) * sz + (*y) * cz) >> 15;
    *x = tx; *y = ty;
}

int nexus_v1_mns_anim_transform_vertices(
    const Nexus_V1_MnsDecodeResult *result,
    const Nexus_V1_MnsPose *pose,
    Nexus_V1_MnsVertex *verts_out, int capacity) {
    int total = 0, ji;

    if (!result || !result->valid || !pose || !verts_out) return 0;

    for (ji = 0; ji < result->joint_count && ji < pose->joint_count; ++ji) {
        const Nexus_V1_MnsJoint *jt = &result->joints[ji];
        const Nexus_V1_MnsJointPose *jp = &pose->joints[ji];
        int vi;

        if (!jt->has_mesh) continue;

        for (vi = 0; vi < jt->mesh.vertex_count; ++vi) {
            int32_t vx, vy, vz;
            if (total >= capacity) return total;

            vx = jt->mesh.vertices[vi].x;
            vy = jt->mesh.vertices[vi].y;
            vz = jt->mesh.vertices[vi].z;

            rotate_vertex(&vx, &vy, &vz, jp->rot_x, jp->rot_y, jp->rot_z);

            verts_out[total].x = vx + jp->world_x;
            verts_out[total].y = vy + jp->world_y;
            verts_out[total].z = vz + jp->world_z;
            total++;
        }
    }
    return total;
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
    if (!range_in_file(abs_off, (uint64_t)td->pixel_count * 2,
                       data_size)) return 0;

    for (j = 0; j < td->pixel_count; ++j) {
        uint16_t c = read_be16(data + abs_off + j * 2);
        rgba_out[j] = bgr555_to_rgba(c);
    }
    return 1;
}
