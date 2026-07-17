#ifndef NEXUS_V1_STRUCTURE1F_DIRECTORY_ADMISSION_H
#define NEXUS_V1_STRUCTURE1F_DIRECTORY_ADMISSION_H

#include "nexus_v1_lev_corpus_discovery.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t source_tag;
    uint32_t record_offset;
    uint32_t record_length;
    uint32_t record_count;
    uint32_t record_size;
    uint64_t record_fnv1a64;
} Nexus_V1_Structure1FDirectoryFamilyReceipt;

/* Parser-observed Structure1F directory facts only. No family record is
 * assigned placement, face, mesh, material, texture, palette, or draw meaning. */
typedef struct {
    int valid;
    int direct_identity_bound;
    int parser_layout_bound;
    int family_directory_bound;
    int no_draw_only;
    int geometry_semantics_permitted;
    int texture_semantics_permitted;
    int draw_permitted;
    uint32_t level_index;
    uint64_t package_fnv1a64;
    uint32_t directory_offset;
    uint32_t directory_length;
    uint64_t directory_fnv1a64;
    uint32_t total_record_count;
    Nexus_V1_Structure1FDirectoryFamilyReceipt
        families[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT];
} Nexus_V1_Structure1FDirectoryAdmissionReceipt;

int nexus_v1_structure1f_directory_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data,
    int dgn_size,
    Nexus_V1_Structure1FDirectoryAdmissionReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
