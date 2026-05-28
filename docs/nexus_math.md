# Nexus V1 Math Subsystem Audit — Source-Locked

## Summary
Nexus has a well-separated 3D math subsystem (nexus_v1_math3d.c) using
floating-point vec3/mat4 operations. RNG uses standard C rand(). The
3D geometry blob in DGN files is not parsed.

## 1. Vector3 Operations (nexus_v1_math3d.c)

Vec3 structure: typedef struct { float x, y, z; } Vec3;

Available operations:
- v3_add(a, b): component-wise addition
- v3_sub(a, b): component-wise subtraction
- v3_scale(v, s): scalar multiplication
- v3_dot(a, b): dot product
- v3_cross(a, b): cross product
- v3_length(v): Euclidean length (sqrtf)
- v3_normalize(v): unit vector (safe against zero-divide)

All use stdlib math.h (sqrtf, cosf, sinf). No SIMD, no fixed-point.

## 2. Matrix4x4 Operations

Mat4 structure: 4x4 row-major float array.

Available operations:
- m4_identity(): identity matrix
- m4_multiply(a, b): matrix multiplication
- m4_translate(x,y,z): translation matrix
- m4_rotate_y(rad): Y-axis rotation (cosf/sinf)
- m4_rotate_x(rad): X-axis rotation (cosf/sinf)
- m4_perspective(fov, aspect, near, far): perspective projection
- m4_look_at(eye, target, up): view matrix (look-at transform)

Perspective: standard matrix with f = 1/tan(fov * PI / 360).
Look-at: f=normalize(target-eye), r=normalize(cross(f,up)), u=cross(r,f).

## 3. Random Number Generation

In nexus_v1_combat.c:
  static int rng(int max) { return max > 0 ? (rand() % max) : 0; }

In nexus_v2_particles.c:
  static float randf(void) { return (float)rand() / (float)RAND_MAX; }

Uses standard C rand() with modulo (int) or normalization (float).
No seed control exposed; uses system srand() default.
Not cryptographically strong; not required for game simulation.

## 4. Geometry Subsystem

Dungeon grid: DMWeb DGN Structure1B, 64x64 cells, 8 bytes per cell.
Grid parsing: nexus_v1_level_get_square() reads the decoded Structure1B
passability grid produced by nexus_v1_level_load().

3D Geometry: DGN files contain additional Structure1C through Structure1F data
and later render payloads after Structure1B:
- Collision descriptors
- Doors
- Floor objects, sensors and decorations
- Wall sensors and decorations
Status: PARTIAL. Structure1B is parsed; the rest is still being split out.

Wall projection: 4 directions (N/E/S/W z-face or x-face).
Each wall = 2 triangles (one quad). Screen-space via perspective matrix.

## 5. Fixed-Point vs Floating-Point

Decision: floating-point only throughout.
- All vec3/mat4 in float (IEEE 754 single-precision)
- Trigonometry via sinf/cosf/sqrtf
- No fixed-point, no SIMD, no hardware acceleration

Original Saturn likely used fixed-point (SH-2 integer unit, no FPU).
Firestaff uses float for simplicity on modern PCs.

## 6. Math Subsystem Status

| Component         | Status | Notes                        |
|-------------------|--------|------------------------------|
| Vec3 operations   | DONE   | All basic ops implemented    |
| Mat4 operations   | DONE   | Transform/prj/look-at done   |
| RNG               | DONE   | rand()-based, simple          |
| Grid parsing      | DONE   | DMWeb Structure1B, 64x64     |
| 3D geometry parse | TODO   | DGN blob unparsed            |
| Wall projection   | PARTIAL| Basic projection exists       |
| Floor/ceiling proj| PARTIAL| May be incomplete             |
| Z-buffer          | EXISTS | In rasterizer for depth sort  |

## 7. Notable Observations

Clean separation: nexus_v1_math3d.c is pure math, no game logic,
no dependencies on dungeon data or asset formats. Trivial to test
or replace with SIMD/fixed-point version.

No determinism: no fixed seed, no deterministic replay mode.
Typical for production games; makes bug replication harder.

Floating-point only: no quaternions. Sufficient for 90-degree
dungeon viewport turns. Would need extension for free-look.

## 8. Math Comparison: Nexus vs DM1

| Aspect         | DM1                  | Nexus                     |
|----------------|----------------------|---------------------------|
| Math model     | Fixed-point 16.16    | Floating-point float      |
| 3D geometry    | None (2D sprites)    | Full vec3/mat4 pipeline   |
| Rotation       | 90-deg turns (table) | m4_rotate_y               |
| Projection     | 2D sprite blit       | m4_perspective + m4_look_at|
| RNG            | Likely rand()        | rand()                    |
| Collision      | Grid-based           | Grid-based + z-buffer    |
