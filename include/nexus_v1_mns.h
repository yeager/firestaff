#ifndef NEXUS_V1_MNS_H
#define NEXUS_V1_MNS_H

#include <stdint.h>

#define NEXUS_MNS_MAGIC          0x444D4446
#define NEXUS_MNS_MOTN_MAGIC     0x4D4F544E
#define NEXUS_MNS_TEXT_MAGIC     0x54455854
#define NEXUS_MNS_JOINT_SIZE     52
#define NEXUS_MNS_MESH_DESC_SIZE 24
#define NEXUS_MNS_VERTEX_SIZE    12
#define NEXUS_MNS_FACE_SIZE      12
#define NEXUS_MNS_MAX_JOINTS     32
#define NEXUS_MNS_MAX_VERTICES   512
#define NEXUS_MNS_MAX_FACES      512
#define NEXUS_MNS_MAX_TEXTURES   16
#define NEXUS_MNS_TEXT_DESC_SIZE  20

typedef struct {
    int32_t x, y, z;
} Nexus_V1_MnsVertex;

typedef struct {
    uint16_t v0, v1, v2, v3;
    uint16_t flags;
    uint16_t misc;
    int is_quad;
    int flip_x;
    int flip_y;
    int texture_index;
} Nexus_V1_MnsFace;

typedef struct {
    int vertex_count;
    int face_count;
    uint32_t vertex_offset;
    uint32_t face_offset;
    Nexus_V1_MnsVertex vertices[NEXUS_MNS_MAX_VERTICES];
    Nexus_V1_MnsFace faces[NEXUS_MNS_MAX_FACES];
} Nexus_V1_MnsMesh;

typedef struct {
    int index;
    int32_t origin_x, origin_y, origin_z;
    uint32_t mesh_offset;
    uint32_t sibling_offset;
    uint32_t child_offset;
    int has_mesh;
    Nexus_V1_MnsMesh mesh;
} Nexus_V1_MnsJoint;

typedef struct {
    uint16_t image_id;
    uint16_t encoding;
    uint16_t width;
    uint16_t height;
    uint32_t image_offset;
    uint32_t pixel_hash;
    int pixel_count;
} Nexus_V1_MnsTextureDesc;

/* MOTN keyframe: per-joint rotation for one frame (DMWeb StrucMotnFrame) */
#define NEXUS_MNS_MAX_MOTN_TABLES   8
#define NEXUS_MNS_MAX_MOTN_FRAMES  64

typedef struct {
    uint16_t duration;     /* frame duration (30fps units) */
    uint16_t flags;
    int16_t  rotation[NEXUS_MNS_MAX_JOINTS][3]; /* x,y,z per joint */
} Nexus_V1_MnsMotnFrame;

typedef struct {
    int frame_count;
    Nexus_V1_MnsMotnFrame frames[NEXUS_MNS_MAX_MOTN_FRAMES];
} Nexus_V1_MnsMotnTable;

typedef struct {
    int valid;
    int table_count;
    int joint_count_motn;
    Nexus_V1_MnsMotnTable tables[NEXUS_MNS_MAX_MOTN_TABLES];
} Nexus_V1_MnsMotnResult;

typedef struct {
    int valid;
    uint32_t file_size;
    uint32_t motn_offset;
    uint32_t text_offset;
    int joint_count;
    int texture_count;
    int total_vertices;
    int total_faces;
    Nexus_V1_MnsJoint joints[NEXUS_MNS_MAX_JOINTS];
    Nexus_V1_MnsTextureDesc textures[NEXUS_MNS_MAX_TEXTURES];
    Nexus_V1_MnsMotnResult motn;
} Nexus_V1_MnsDecodeResult;

int nexus_v1_mns_decode(const uint8_t *data, int data_size,
                         Nexus_V1_MnsDecodeResult *out);

int nexus_v1_mns_render_texture(const uint8_t *data, int data_size,
                                 const Nexus_V1_MnsDecodeResult *result,
                                 int texture_index,
                                 uint32_t *rgba_out, int rgba_capacity);

#endif
