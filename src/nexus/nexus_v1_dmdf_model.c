
#include "nexus_v1_dmdf_model.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Big-endian readers (Saturn SH2 is big-endian) */
static uint32_t rb32(const uint8_t *p) {
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
}
static uint16_t rb16(const uint8_t *p) { return ((uint16_t)p[0]<<8)|p[1]; }
static int16_t rbs16(const uint8_t *p) { return (int16_t)rb16(p); }

int nexus_v1_dmdf_is_valid(const uint8_t *data, int size) {
    if (!data || size < 32) return 0;
    return rb32(data) == NEXUS_DMDF_MAGIC;
}

int nexus_v1_dmdf_load(Nexus_V1_Model *model, const uint8_t *data, int size, const char *name) {
    if (!model || !data || size < 32) return -1;
    memset(model, 0, sizeof(*model));

    if (!nexus_v1_dmdf_is_valid(data, size)) return -1;

    model->header.magic = rb32(data);
    model->header.file_size = rb32(data + 4);
    model->header.section_count = rb32(data + 8);
    model->header.flags = rb32(data + 12);
    model->header.data_offset = rb32(data + 28);
    model->name = name;

    printf("DMDF %s: size=%u sections=%u data_offset=%u\n",
        name ? name : "?",
        model->header.file_size,
        model->header.section_count,
        model->header.data_offset);

    /* Parse vertex/face data from sections */
    if (model->header.data_offset > 0 && (int)model->header.data_offset < size) {
        int off = (int)model->header.data_offset;
        /* Read vertex count and face count from data section */
        if (off + 8 <= size) {
            int vc = rb32(data + off);
            int fc = rb32(data + off + 4);
            if (vc > 0 && vc < 10000 && fc > 0 && fc < 30000) {
                model->vertex_count = vc;
                model->header.vertex_count = vc;
                model->header.face_count = fc;
                /* Allocate and read vertices */
                int vert_size = vc * 10; /* 5 int16 per vertex */
                if (off + 8 + vert_size <= size) {
                    model->vertices = (Nexus_DMDFVertex *)calloc(vc, sizeof(Nexus_DMDFVertex));
                    if (model->vertices) {
                        for (int i = 0; i < vc; i++) {
                            int vo = off + 8 + i * 10;
                            model->vertices[i].x = rbs16(data + vo);
                            model->vertices[i].y = rbs16(data + vo + 2);
                            model->vertices[i].z = rbs16(data + vo + 4);
                            model->vertices[i].u = rb16(data + vo + 6);
                            model->vertices[i].v = rb16(data + vo + 8);
                        }
                    }
                }
                /* Read face indices */
                int face_off = off + 8 + vert_size;
                int face_bytes = fc * 6; /* 3 uint16 per triangle */
                if (face_off + face_bytes <= size) {
                    model->faces = (uint16_t *)calloc(fc * 3, sizeof(uint16_t));
                    if (model->faces) {
                        model->face_count = fc;
                        for (int i = 0; i < fc * 3; i++)
                            model->faces[i] = rb16(data + face_off + i * 2);
                    }
                }
                printf("  vertices=%d faces=%d\n", vc, fc);
            }
        }
    }

    return 0;
}

void nexus_v1_dmdf_free(Nexus_V1_Model *model) {
    if (!model) return;
    free(model->vertices); model->vertices = NULL;
    free(model->faces); model->faces = NULL;
    free(model->texture_data); model->texture_data = NULL;
}

