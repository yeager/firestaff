
#ifndef NEXUS_V1_MATH3D_H
#define NEXUS_V1_MATH3D_H

/* Fixed-point 3D math for Nexus software renderer.
 * Saturn SH2 had no FPU — all math was fixed-point.
 * We use float internally but match Saturn precision where it matters. */

typedef struct { float x, y, z; } Vec3;
typedef struct { float x, y, z, w; } Vec4;
typedef struct { float m[4][4]; } Mat4;
typedef struct { float x, y; } Vec2;
typedef struct { int x, y; } Vec2i;

/* Vector ops */
Vec3 v3_add(Vec3 a, Vec3 b);
Vec3 v3_sub(Vec3 a, Vec3 b);
Vec3 v3_scale(Vec3 v, float s);
float v3_dot(Vec3 a, Vec3 b);
Vec3 v3_cross(Vec3 a, Vec3 b);
Vec3 v3_normalize(Vec3 v);
float v3_length(Vec3 v);

/* Matrix ops */
Mat4 m4_identity(void);
Mat4 m4_multiply(Mat4 a, Mat4 b);
Mat4 m4_translate(float x, float y, float z);
Mat4 m4_rotate_y(float radians);
Mat4 m4_rotate_x(float radians);
Mat4 m4_perspective(float fov_deg, float aspect, float near, float far);
Mat4 m4_look_at(Vec3 eye, Vec3 target, Vec3 up);
Vec4 m4_transform(Mat4 m, Vec4 v);

/* Project 3D → 2D screen coords */
Vec2i v3_project(Vec3 world, Mat4 view_proj, int screen_w, int screen_h);

#endif

