
#ifndef NEXUS_V1_DMDF_MODEL_H
#define NEXUS_V1_DMDF_MODEL_H
#include <stdint.h>

/* DMDF — Dungeon Master Data Format
 * Magic: "DMDF" at offset 0
 * Used for creature models (.MNS files) in DM Nexus Saturn.
 * Big-endian (SH2 processor). */

#define NEXUS_DMDF_MAGIC 0x444D4446  /* "DMDF" */

typedef struct {
    uint32_t magic;
    uint32_t file_size;
    uint32_t section_count;
    uint32_t flags;
    uint32_t reserved[4];
    uint32_t data_offset;
    uint32_t vertex_offset;
    uint32_t vertex_count;
    uint32_t face_count;
} Nexus_DMDFHeader;

typedef struct {
    int16_t x, y, z;
    int16_t nx, ny, nz;  /* normal */
    uint16_t u, v;        /* texture coords */
} Nexus_DMDFVertex;

typedef struct {
    Nexus_DMDFHeader header;
    Nexus_DMDFVertex *vertices;
    int vertex_count;
    uint16_t *faces;      /* triangle/quad indices */
    int face_count;
    uint8_t *texture_data;
    int texture_size;
    const char *name;
} Nexus_V1_Model;

int nexus_v1_dmdf_load(Nexus_V1_Model *model, const uint8_t *data, int size, const char *name);
void nexus_v1_dmdf_free(Nexus_V1_Model *model);
int nexus_v1_dmdf_is_valid(const uint8_t *data, int size);

#endif

