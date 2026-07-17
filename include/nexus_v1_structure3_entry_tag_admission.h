#ifndef NEXUS_V1_STRUCTURE3_ENTRY_TAG_ADMISSION_H
#define NEXUS_V1_STRUCTURE3_ENTRY_TAG_ADMISSION_H

#include "nexus_v1_structure3_entry_admission.h"

/* The first four bytes of an admitted Structure3 entry header remain an
 * opaque tag span. No entry kind, model, geometry, or draw meaning follows. */
typedef struct {
    int valid;
    int entry_bound;
    int header_bound;
    int tag_bound;
    int no_draw_only;
    int entry_kind_semantics_permitted;
    int geometry_semantics_permitted;
    int draw_permitted;
    uint32_t level_index;
    uint64_t package_fnv1a64;
    uint32_t entry_offset;
    uint64_t entry_fnv1a64;
    uint32_t tag_offset;
    uint64_t tag_fnv1a64;
    uint8_t raw_bytes[4];
} Nexus_V1_Structure3EntryTagAdmissionReceipt;

int nexus_v1_structure3_entry_tag_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure3EntryAdmissionReceipt *entry,
    Nexus_V1_Structure3EntryTagAdmissionReceipt *out_receipt);

#endif
