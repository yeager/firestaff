
#include "nexus_v1_math3d.h"
#include <math.h>
#include <string.h>

Vec3 v3_add(Vec3 a, Vec3 b) { return (Vec3){a.x+b.x, a.y+b.y, a.z+b.z}; }
Vec3 v3_sub(Vec3 a, Vec3 b) { return (Vec3){a.x-b.x, a.y-b.y, a.z-b.z}; }
Vec3 v3_scale(Vec3 v, float s) { return (Vec3){v.x*s, v.y*s, v.z*s}; }
float v3_dot(Vec3 a, Vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

Vec3 v3_cross(Vec3 a, Vec3 b) {
    return (Vec3){a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
}

float v3_length(Vec3 v) { return sqrtf(v3_dot(v, v)); }

Vec3 v3_normalize(Vec3 v) {
    float len = v3_length(v);
    if (len < 0.0001f) return (Vec3){0,0,0};
    return v3_scale(v, 1.0f / len);
}

Mat4 m4_identity(void) {
    Mat4 m; memset(&m, 0, sizeof(m));
    m.m[0][0] = m.m[1][1] = m.m[2][2] = m.m[3][3] = 1.0f;
    return m;
}

Mat4 m4_multiply(Mat4 a, Mat4 b) {
    Mat4 r; int i, j, k;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++) {
            r.m[i][j] = 0;
            for (k = 0; k < 4; k++)
                r.m[i][j] += a.m[i][k] * b.m[k][j];
        }
    return r;
}

Mat4 m4_translate(float x, float y, float z) {
    Mat4 m = m4_identity();
    m.m[0][3] = x; m.m[1][3] = y; m.m[2][3] = z;
    return m;
}

Mat4 m4_rotate_y(float rad) {
    Mat4 m = m4_identity();
    float c = cosf(rad), s = sinf(rad);
    m.m[0][0] = c;  m.m[0][2] = s;
    m.m[2][0] = -s; m.m[2][2] = c;
    return m;
}

Mat4 m4_rotate_x(float rad) {
    Mat4 m = m4_identity();
    float c = cosf(rad), s = sinf(rad);
    m.m[1][1] = c;  m.m[1][2] = -s;
    m.m[2][1] = s;  m.m[2][2] = c;
    return m;
}

Mat4 m4_perspective(float fov_deg, float aspect, float near, float far) {
    Mat4 m; memset(&m, 0, sizeof(m));
    float f = 1.0f / tanf(fov_deg * 3.14159265f / 360.0f);
    m.m[0][0] = f / aspect;
    m.m[1][1] = f;
    m.m[2][2] = (far + near) / (near - far);
    m.m[2][3] = (2 * far * near) / (near - far);
    m.m[3][2] = -1.0f;
    return m;
}

Mat4 m4_look_at(Vec3 eye, Vec3 target, Vec3 up) {
    Vec3 f = v3_normalize(v3_sub(target, eye));
    Vec3 r = v3_normalize(v3_cross(f, up));
    Vec3 u = v3_cross(r, f);
    Mat4 m = m4_identity();
    m.m[0][0] = r.x;  m.m[0][1] = r.y;  m.m[0][2] = r.z;  m.m[0][3] = -v3_dot(r, eye);
    m.m[1][0] = u.x;  m.m[1][1] = u.y;  m.m[1][2] = u.z;  m.m[1][3] = -v3_dot(u, eye);
    m.m[2][0] = -f.x; m.m[2][1] = -f.y; m.m[2][2] = -f.z; m.m[2][3] = v3_dot(f, eye);
    return m;
}

Vec4 m4_transform(Mat4 m, Vec4 v) {
    Vec4 r;
    r.x = m.m[0][0]*v.x + m.m[0][1]*v.y + m.m[0][2]*v.z + m.m[0][3]*v.w;
    r.y = m.m[1][0]*v.x + m.m[1][1]*v.y + m.m[1][2]*v.z + m.m[1][3]*v.w;
    r.z = m.m[2][0]*v.x + m.m[2][1]*v.y + m.m[2][2]*v.z + m.m[2][3]*v.w;
    r.w = m.m[3][0]*v.x + m.m[3][1]*v.y + m.m[3][2]*v.z + m.m[3][3]*v.w;
    return r;
}

Vec2i v3_project(Vec3 world, Mat4 vp, int sw, int sh) {
    Vec4 clip = m4_transform(vp, (Vec4){world.x, world.y, world.z, 1.0f});
    Vec2i screen = {0, 0};
    if (clip.w > 0.001f) {
        float ndc_x = clip.x / clip.w;
        float ndc_y = clip.y / clip.w;
        screen.x = (int)((ndc_x * 0.5f + 0.5f) * sw);
        screen.y = (int)((1.0f - (ndc_y * 0.5f + 0.5f)) * sh);
    }
    return screen;
}

