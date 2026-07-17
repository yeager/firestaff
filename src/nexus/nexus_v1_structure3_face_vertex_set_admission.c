#include "nexus_v1_structure3_face_vertex_set_admission.h"

#include <string.h>

int nexus_v1_structure3_face_vertex_set_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure3EntryAdmissionReceipt *entry,
    const Nexus_V1_Structure3FaceAdmissionReceipt *face,
    Nexus_V1_Structure3FaceVertexSetAdmissionReceipt *out_receipt)
{
    Nexus_V1_Structure3FaceVertexSetAdmissionReceipt receipt;
    uint32_t slot;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    for (slot = 0U; slot < 4U; ++slot) {
        if (!nexus_v1_structure3_face_vertex_admit(
                identity, dgn_data, dgn_size, entry, face, slot,
                &receipt.slots[slot])) {
            *out_receipt = receipt;
            return 0;
        }
    }
    receipt.valid = receipt.entry_bound = receipt.face_row_bound = 1;
    receipt.ordered_slots_bound = receipt.no_draw_only = 1;
    receipt.level_index = face->level_index;
    receipt.package_fnv1a64 = face->package_fnv1a64;
    receipt.face_ordinal = face->face_ordinal;
    receipt.face_offset = face->face_offset;
    receipt.face_fnv1a64 = face->face_fnv1a64;
    *out_receipt = receipt;
    return 1;
}
