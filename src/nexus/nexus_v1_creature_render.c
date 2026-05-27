#include "nexus_v1_rendering.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* ── Creature billboard (fallback for missing DMDF) ─────────── */

static const uint8_t g_creature_fallback_colors[NEXUS_MAX_CREATURE_TYPES] = {
    /* Based on creature type index */
    10, /* Scorpion — brown */
    10, /* Mummy — brown */
    12, /* Dragon — firebrick */
    7,  /* Skeleton — light gray */
    4,  /* Ghost — purple */
    10, /* Worm — brown */
    6,  /* Golem — medium gray */
    10, /* Spider — brown */
};

/* Build a billboard quad facing the camera for a creature.
 * A billboard quad has 4 vertices forming a rectangle perpendicular
 * to the view direction, always facing the camera. */
static void build_creature_billboard(Nexus_RasterVertex *v,
    float world_x, float world_y,
    float height, float width,
    const Nexus_Camera *cam)
{
    Vec3 center = {world_x + 0.5f, height * 0.5f, world_y + 0.5f};
    Vec3 to_cam;
    Vec3 right, up;
    float hw = width * 0.5f;

    /* Direction from creature to camera */
    to_cam = v3_normalize(v3_sub(cam->pos, center));

    /* Up is world up */
    up = (Vec3){0, 1, 0};

    /* Right = camera-right vector (perpendicular to to_cam and up) */
    right = v3_normalize(v3_cross(to_cam, up));

    /* Build quad: center +/(-) right * hw +/(-) up * height */
    v[0].position = v3_sub(v3_sub(center, v3_scale(right, hw)), v3_scale(up, height * 0.5f));
    v[1].position = v3_add(v3_sub(center, v3_scale(right, hw)), v3_scale(up, height * 0.5f));
    v[2].position = v3_add(v3_add(center, v3_scale(right, hw)), v3_scale(up, height * 0.5f));
    v[3].position = v3_sub(v3_add(center, v3_scale(right, hw)), v3_scale(up, height * 0.5f));

    /* UV coordinates for full texture */
    v[0].uv = (Vec2){0, 1};
    v[1].uv = (Vec2){0, 0};
    v[2].uv = (Vec2){1, 0};
    v[3].uv = (Vec2){1, 1};
}

void nexus_render_creature(Nexus_Framebuffer *fb,
    const Nexus_Camera *cam,
    const Nexus_V1_Model *model,
    float world_x, float world_y,
    int anim_frame,
    int facing,
    uint8_t light_level)
{
    int i;

    if (!fb || !cam) return;

    if (model && model->vertex_count > 0 && model->faces) {
        /* Render DMDF model — actual 3D mesh */
        /* Transform and project each triangle */
        Mat4 model_mat = m4_identity();
        Mat4 world_mat;

        /* Apply creature position */
        model_mat = m4_multiply(
            m4_translate(world_x + 0.5f, 0.0f, world_y + 0.5f),
            m4_rotate_y((float)(facing) * 3.14159265f * 0.5f));

        /* For each triangle in the model */
        for (i = 0; i + 2 < model->face_count * 3; i += 3) {
            Nexus_RasterVertex tv[3];
            Vec3 verts[3];
            int vi0, vi1, vi2;
            uint8_t color;

            vi0 = model->faces[i];
            vi1 = model->faces[i + 1];
            vi2 = model->faces[i + 2];

            if (vi0 >= model->vertex_count ||
                vi1 >= model->vertex_count ||
                vi2 >= model->vertex_count)
                continue;

            /* Get model-space vertices */
            verts[0] = (Vec3){
                (float)model->vertices[vi0].x / 4096.0f,
                (float)model->vertices[vi0].y / 4096.0f,
                (float)model->vertices[vi0].z / 4096.0f
            };
            verts[1] = (Vec3){
                (float)model->vertices[vi1].x / 4096.0f,
                (float)model->vertices[vi1].y / 4096.0f,
                (float)model->vertices[vi1].z / 4096.0f
            };
            verts[2] = (Vec3){
                (float)model->vertices[vi2].x / 4096.0f,
                (float)model->vertices[vi2].y / 4096.0f,
                (float)model->vertices[vi2].z / 4096.0f
            };

            /* Apply model matrix */
            {
                Vec4 mv0 = m4_transform(model_mat, (Vec4){verts[0].x, verts[0].y, verts[0].z, 1});
                Vec4 mv1 = m4_transform(model_mat, (Vec4){verts[1].x, verts[1].y, verts[1].z, 1});
                Vec4 mv2 = m4_transform(model_mat, (Vec4){verts[2].x, verts[2].y, verts[2].z, 1});
                tv[0].position = (Vec3){mv0.x, mv0.y, mv0.z};
                tv[1].position = (Vec3){mv1.x, mv1.y, mv1.z};
                tv[2].position = (Vec3){mv2.x, mv2.y, mv2.z};
            }

            /* Color from UV or light level */
            color = light_level;
            tv[0].color = tv[1].color = tv[2].color = color;

            /* UV from model */
            tv[0].uv = (Vec2){
                (float)model->vertices[vi0].u / 255.0f,
                (float)model->vertices[vi0].v / 255.0f
            };
            tv[1].uv = (Vec2){
                (float)model->vertices[vi1].u / 255.0f,
                (float)model->vertices[vi1].v / 255.0f
            };
            tv[2].uv = (Vec2){
                (float)model->vertices[vi2].u / 255.0f,
                (float)model->vertices[vi2].v / 255.0f
            };

            nexus_raster_triangle(fb, tv[0], tv[1], tv[2], cam);
        }
    } else {
        /* Fallback: billboard quad with procedural color */
        Nexus_RasterVertex v[4];
        uint8_t color;

        /* Select fallback color by creature type (deterministic) */
        color = 10; /* default brown */
        if (model && model->name) {
            /* Try to match name to known creature types */
            const char *n = model->name;
            if (strstr(n, "dragon")) color = 12;
            else if (strstr(n, "ghost")) color = 4;
            else if (strstr(n, "skeleton")) color = 7;
            else if (strstr(n, "golem")) color = 6;
            else if (strstr(n, "worm")) color = 10;
            else if (strstr(n, "scorpion")) color = 10;
            else if (strstr(n, "spider")) color = 10;
            else if (strstr(n, "mummy")) color = 10;
        }

        build_creature_billboard(v, world_x, world_y, 1.0f, 0.8f, cam);
        for (i = 0; i < 4; i++) v[i].color = color + (light_level >> 2);
        nexus_raster_quad(fb, v[0], v[1], v[2], v[3], cam);
    }
}
