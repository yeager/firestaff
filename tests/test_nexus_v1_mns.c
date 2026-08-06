#include "nexus_v1_mns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

static uint8_t *load_file(const char *path, int *out_size) {
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    long sz;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = (uint8_t *)malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    if ((long)fread(buf, 1, (size_t)sz, f) != sz) {
        free(buf); fclose(f); return NULL;
    }
    fclose(f);
    *out_size = (int)sz;
    return buf;
}

static void write_be32(uint8_t *p, uint32_t value) {
    p[0] = (uint8_t)(value >> 24);
    p[1] = (uint8_t)(value >> 16);
    p[2] = (uint8_t)(value >> 8);
    p[3] = (uint8_t)value;
}

static int test_synthetic(void) {
    Nexus_V1_MnsDecodeResult r;
    uint8_t bad[64];
    memset(bad, 0, sizeof(bad));
    if (nexus_v1_mns_decode(bad, 64, &r)) return 1;
    if (nexus_v1_mns_decode(NULL, 0, &r)) return 1;
    /* A declared joint table is part of the DMDF envelope, not an optional
     * hint. A truncated table must not be accepted as a valid model. */
    memset(bad, 0, sizeof(bad));
    bad[0] = 'D'; bad[1] = 'M'; bad[2] = 'D'; bad[3] = 'F';
    bad[0x28 + 3] = 1;
    bad[0x1C + 3] = 0x34;
    if (nexus_v1_mns_decode(bad, 0x34, &r)) return 1;
    /* DMWeb DMDF+04 is the exact block/file size, not an advisory value. */
    memset(bad, 0, sizeof(bad));
    bad[0] = 'D'; bad[1] = 'M'; bad[2] = 'D'; bad[3] = 'F';
    write_be32(bad + 4, 0x34U - 1U);
    if (nexus_v1_mns_decode(bad, (int)sizeof(bad), &r)) return 1;
    /* A declared skeleton beyond the bounded host representation must not be
     * accepted as a truncated prefix. */
    memset(bad, 0, sizeof(bad));
    bad[0] = 'D'; bad[1] = 'M'; bad[2] = 'D'; bad[3] = 'F';
    write_be32(bad + 0x1C, 0x34U);
    write_be32(bad + 0x28, NEXUS_MNS_MAX_JOINTS + 1U);
    if (nexus_v1_mns_decode(bad, (int)sizeof(bad), &r)) return 1;
    printf("  PASS synthetic\n");
    return 0;
}

static int test_all_mns(void) {
    const char *home = getenv("HOME");
    char dirpath[512];
    DIR *d;
    struct dirent *ent;
    int decoded = 0, rendered = 0, fail = 0;
    int scorpion_joints = 0, rockpile_joints = 0;
    int vexirk_textures = 0, d_gold_tables = 0;

    if (!home) { printf("  SKIP all_mns (no HOME)\n"); return 0; }
    snprintf(dirpath, sizeof(dirpath), "%s/.firestaff/data/nexus", home);
    d = opendir(dirpath);
    if (!d) { printf("  SKIP all_mns (no data dir)\n"); return 0; }

    while ((ent = readdir(d)) != NULL) {
        char path[768];
        const char *name = ent->d_name;
        int len = (int)strlen(name);
        uint8_t *data;
        int size = 0;
        Nexus_V1_MnsDecodeResult result;

        if (len < 5 || strcmp(name + len - 4, ".MNS") != 0) continue;

        snprintf(path, sizeof(path), "%s/%s", dirpath, name);
        data = load_file(path, &size);
        if (!data) continue;

        if (!nexus_v1_mns_decode(data, size, &result)) {
            printf("  FAIL %s: decode failed\n", name);
            free(data);
            ++fail;
            continue;
        }

        printf("  PASS %-14s joints=%d verts=%d faces=%d tex=%d",
               name, result.joint_count, result.total_vertices,
               result.total_faces, result.texture_count);
        if (result.texture_count > 0) {
            int texture_index;
            printf(" tex0=%dx%d hash=0x%08X",
                   result.textures[0].width, result.textures[0].height,
                   result.textures[0].pixel_hash);
            for (texture_index = 0; texture_index < result.texture_count;
                 ++texture_index) {
                uint32_t *pixels = (uint32_t *)malloc(
                    (size_t)result.textures[texture_index].pixel_count *
                    sizeof(*pixels));
                if (!pixels || !nexus_v1_mns_render_texture(
                        data, size, &result, texture_index, pixels,
                        result.textures[texture_index].pixel_count)) {
                    ++fail;
                } else {
                    ++rendered;
                }
                free(pixels);
            }
            printf(" render=%s", fail ? "CHECKED" : "PASS");
        }
        printf("\n");
        if (strcmp(name, "SCORPION.MNS") == 0) scorpion_joints = result.joint_count;
        if (strcmp(name, "ROCKPILE.MNS") == 0) rockpile_joints = result.joint_count;
        if (strcmp(name, "VEXIRK.MNS") == 0) vexirk_textures = result.texture_count;
        if (strcmp(name, "D_GOLD.MNS") == 0) d_gold_tables = result.motn.table_count;
        decoded++;
        free(data);
    }
    closedir(d);

    printf("  decoded %d MNS files, rendered %d source textures\n",
           decoded, rendered);
    if (decoded > 0) {
        if (scorpion_joints != 33) {
            printf("  FAIL SCORPION.MNS: joints=%d (expected retail 33)\n",
                   scorpion_joints);
            ++fail;
        } else {
            printf("  PASS SCORPION.MNS: retained 33 retail joints\n");
        }
        if (rockpile_joints != 37) {
            printf("  FAIL ROCKPILE.MNS: joints=%d (expected retail 37)\n",
                   rockpile_joints);
            ++fail;
        } else {
            printf("  PASS ROCKPILE.MNS: retained 37 retail joints\n");
        }
        if (vexirk_textures != 64) {
            printf("  FAIL VEXIRK.MNS: textures=%d (expected retail 64)\n",
                   vexirk_textures);
            ++fail;
        } else {
            printf("  PASS VEXIRK.MNS: retained 64 retail textures\n");
        }
        if (d_gold_tables != 11) {
            printf("  FAIL D_GOLD.MNS: MOTN tables=%d (expected retail 11)\n",
                   d_gold_tables);
            ++fail;
        } else {
            printf("  PASS D_GOLD.MNS: retained 11 retail MOTN tables\n");
        }
    }
    if (decoded == 0) printf("  SKIP (no MNS files found)\n");
    return fail;
}

static int test_anim_init(void) {
    Nexus_V1_MnsAnimState state;
    nexus_v1_mns_anim_init(&state);
    if (state.table_index != -1) return 1;
    if (state.finished != 0) return 1;
    printf("  PASS anim_init\n");
    return 0;
}

static int test_anim_synthetic(void) {
    Nexus_V1_MnsAnimState state;
    Nexus_V1_MnsMotnResult motn;
    Nexus_V1_MnsMotnFrame frames[3];
    int changed, i;

    memset(&motn, 0, sizeof(motn));
    memset(frames, 0, sizeof(frames));
    motn.valid = 1;
    motn.table_count = 1;
    motn.joint_count_motn = 2;
    motn.tables[0].frame_count = 3;
    frames[0].duration = 5;
    frames[0].rotation[0][0] = 0;
    frames[0].rotation[0][1] = 0;
    frames[0].rotation[0][2] = 0;
    frames[1].duration = 5;
    frames[1].rotation[0][0] = 150;
    frames[1].rotation[0][1] = 0;
    frames[1].rotation[0][2] = 0;
    frames[2].duration = 5;
    frames[2].rotation[0][0] = 300;
    frames[2].rotation[0][1] = 0;
    frames[2].rotation[0][2] = 0;
    memcpy(motn.tables[0].frames, frames, sizeof(frames));

    nexus_v1_mns_anim_init(&state);
    nexus_v1_mns_anim_play(&state, 0, 0);
    if (state.frame_index != 0) return 1;

    /* Tick through frame 0 (duration=5) */
    for (i = 0; i < 4; ++i) {
        changed = nexus_v1_mns_anim_tick(&state, &motn);
        if (changed) return 1;
    }
    changed = nexus_v1_mns_anim_tick(&state, &motn);
    if (!changed) return 1;
    if (state.frame_index != 1) return 1;

    /* Tick through frame 1 */
    for (i = 0; i < 5; ++i)
        nexus_v1_mns_anim_tick(&state, &motn);
    if (state.frame_index != 2) return 1;

    /* Tick through frame 2 — should stop (non-looping) */
    for (i = 0; i < 5; ++i)
        nexus_v1_mns_anim_tick(&state, &motn);
    if (!state.finished) return 1;

    /* Test looping */
    nexus_v1_mns_anim_play(&state, 0, 1);
    for (i = 0; i < 15; ++i)
        nexus_v1_mns_anim_tick(&state, &motn);
    if (state.finished) return 1;
    if (state.frame_index != 0) return 1;

    printf("  PASS anim_synthetic\n");
    return 0;
}

static int test_anim_sample_rest(void) {
    Nexus_V1_MnsDecodeResult result;
    Nexus_V1_MnsAnimState state;
    Nexus_V1_MnsPose pose;

    memset(&result, 0, sizeof(result));
    result.valid = 1;
    result.joint_count = 2;
    result.joints[0].origin_x = 100;
    result.joints[0].origin_y = 200;
    result.joints[0].origin_z = 300;
    result.joints[1].origin_x = 400;
    result.joints[1].origin_y = 500;
    result.joints[1].origin_z = 600;

    nexus_v1_mns_anim_init(&state);
    if (!nexus_v1_mns_anim_sample(&result, &state, &pose)) return 1;
    if (pose.joint_count != 2) return 1;
    if (pose.joints[0].world_x != 100) return 1;
    if (pose.joints[1].world_z != 600) return 1;
    if (pose.joints[0].rot_x != 0) return 1;

    printf("  PASS anim_sample_rest\n");
    return 0;
}

static int test_anim_transform(void) {
    Nexus_V1_MnsDecodeResult result;
    Nexus_V1_MnsPose pose;
    Nexus_V1_MnsVertex verts[64];
    int count;

    memset(&result, 0, sizeof(result));
    result.valid = 1;
    result.joint_count = 1;
    result.joints[0].has_mesh = 1;
    result.joints[0].mesh.vertex_count = 3;
    result.joints[0].mesh.vertices[0].x = 10;
    result.joints[0].mesh.vertices[0].y = 20;
    result.joints[0].mesh.vertices[0].z = 30;
    result.joints[0].mesh.vertices[1].x = -10;
    result.joints[0].mesh.vertices[1].y = -20;
    result.joints[0].mesh.vertices[1].z = -30;
    result.joints[0].mesh.vertices[2].x = 0;
    result.joints[0].mesh.vertices[2].y = 0;
    result.joints[0].mesh.vertices[2].z = 0;

    memset(&pose, 0, sizeof(pose));
    pose.joint_count = 1;
    pose.joints[0].world_x = 1000;
    pose.joints[0].world_y = 2000;
    pose.joints[0].world_z = 3000;
    /* Zero rotation = identity */

    count = nexus_v1_mns_anim_transform_vertices(&result, &pose, verts, 64);
    if (count != 3) return 1;
    /* With zero rotation, output = local + world offset */
    if (verts[0].x != 1010 || verts[0].y != 2020 || verts[0].z != 3030)
        return 1;
    if (verts[1].x != 990 || verts[1].y != 1980 || verts[1].z != 2970)
        return 1;
    if (verts[2].x != 1000 || verts[2].y != 2000 || verts[2].z != 3000)
        return 1;

    printf("  PASS anim_transform\n");
    return 0;
}

static int test_anim_real_mns(void) {
    const char *home = getenv("HOME");
    char path[512];
    uint8_t *data;
    int size = 0;
    Nexus_V1_MnsDecodeResult result;
    Nexus_V1_MnsAnimState state;
    Nexus_V1_MnsPose pose;
    Nexus_V1_MnsVertex verts[2048];
    int count, i;

    if (!home) { printf("  SKIP anim_real (no HOME)\n"); return 0; }
    snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/OBAKE.MNS", home);
    data = load_file(path, &size);
    if (!data) { printf("  SKIP anim_real (no OBAKE.MNS)\n"); return 0; }

    if (!nexus_v1_mns_decode(data, size, &result)) {
        printf("  FAIL anim_real: decode failed\n");
        free(data);
        return 1;
    }

    printf("  OBAKE.MNS: joints=%d motn_valid=%d tables=%d motn_joints=%d\n",
           result.joint_count, result.motn.valid,
           result.motn.table_count, result.motn.joint_count_motn);

    if (result.motn.valid && result.motn.table_count > 0) {
        int t;
        for (t = 0; t < result.motn.table_count; ++t) {
            printf("    table[%d]: %d frames", t,
                   result.motn.tables[t].frame_count);
            if (result.motn.tables[t].frame_count > 0)
                printf(" dur0=%d flags0=%d",
                       result.motn.tables[t].frames[0].duration,
                       result.motn.tables[t].frames[0].flags);
            printf("\n");
        }

        /* Play table 0 and sample */
        nexus_v1_mns_anim_init(&state);
        nexus_v1_mns_anim_play(&state, 0, 1);

        for (i = 0; i < 30; ++i)
            nexus_v1_mns_anim_tick(&state, &result.motn);

        if (!nexus_v1_mns_anim_sample(&result, &state, &pose)) {
            printf("  FAIL anim_real: sample failed\n");
            free(data);
            return 1;
        }

        count = nexus_v1_mns_anim_transform_vertices(&result, &pose,
                                                      verts, 2048);
        printf("  anim_real: frame=%d tick=%d transformed=%d verts\n",
               state.frame_index, state.tick_accumulator, count);

        if (count <= 0) {
            printf("  FAIL anim_real: no transformed vertices\n");
            free(data);
            return 1;
        }
    }

    printf("  PASS anim_real\n");
    free(data);
    return 0;
}

int main(void) {
    int fail = 0;
    printf("=== Nexus V1 MNS Creature Model Decoder ===\n");
    fail += test_synthetic();
    fail += test_all_mns();
    fail += test_anim_init();
    fail += test_anim_synthetic();
    fail += test_anim_sample_rest();
    fail += test_anim_transform();
    fail += test_anim_real_mns();
    printf("summary: fail=%d\n", fail);
    return fail ? 1 : 0;
}
