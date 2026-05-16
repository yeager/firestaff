
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

    /* Parse vertex/face data from sections.
     * Full parsing requires reverse-engineering the section format. */

    return 0;
}

void nexus_v1_dmdf_free(Nexus_V1_Model *model) {
    if (!model) return;
    free(model->vertices); model->vertices = NULL;
    free(model->faces); model->faces = NULL;
    free(model->texture_data); model->texture_data = NULL;
}

