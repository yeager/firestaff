#include "nexus_v1_dgn_face_material_provenance.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    static const uint8_t retail_dgn[] = {0x44, 0x47, 0x4e, 0x00};
    Nexus_V1_DgnFaceMaterialBinding bindings[] = {
        {0, 3, NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_STATIC},
        {1, 1, NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_ANIMATED},
        {2, 0, NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_STATIC}
    };
    Nexus_V1_DgnFaceMaterialInput input;
    Nexus_V1_DgnFaceMaterialReceipt receipt;

    memset(&input, 0, sizeof(input));
    input.source = NEXUS_V1_DGN_FACE_MATERIAL_SOURCE_RETAIL_DGN;
    input.dgn_bytes = retail_dgn;
    input.dgn_size = (int)sizeof(retail_dgn);
    input.canonical_dgn_bytes = retail_dgn;
    input.canonical_dgn_size = (int)sizeof(retail_dgn);
    input.canonical_source_verified = 1;
    input.bindings = bindings;
    input.face_count = 3;
    input.material_selector_count = 4;

    expect(nexus_v1_dgn_face_material_validate(&input, &receipt) == 1 &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_READY &&
               receipt.canonical_source_verified &&
               receipt.reopened_bytes_match_canonical &&
               receipt.canonical_dgn_size == (int)sizeof(retail_dgn) &&
               receipt.face_count == 3 && receipt.static_selector_count == 2 &&
               receipt.animated_selector_count == 1 &&
               receipt.selector_bindings_complete &&
               receipt.can_submit_raster_input && !receipt.permits_fallback_visuals,
           "canonical retail DGN bindings alone reach the raster-input boundary");

    {
        uint8_t reopened_dgn[sizeof(retail_dgn)];

        memcpy(reopened_dgn, retail_dgn, sizeof(reopened_dgn));
        reopened_dgn[2] ^= 0x01U;
        input.dgn_bytes = reopened_dgn;
        expect(!nexus_v1_dgn_face_material_validate(&input, &receipt) &&
                   receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE &&
                   !receipt.reopened_bytes_match_canonical &&
                   !receipt.can_submit_raster_input &&
                   !receipt.permits_fallback_visuals,
               "a changed launcher-reopened DGN buffer cannot inherit canonical admission");
        input.dgn_bytes = retail_dgn;
    }

    input.canonical_dgn_size = (int)sizeof(retail_dgn) - 1;
    expect(!nexus_v1_dgn_face_material_validate(&input, &receipt) &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE &&
               !receipt.can_submit_raster_input,
           "a shortened canonical catalog buffer cannot admit a reopened DGN buffer");
    input.canonical_dgn_size = (int)sizeof(retail_dgn);

    input.source = NEXUS_V1_DGN_FACE_MATERIAL_SOURCE_SYNTHETIC;
    expect(!nexus_v1_dgn_face_material_validate(&input, &receipt) &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE &&
               !receipt.can_submit_raster_input && !receipt.permits_fallback_visuals,
           "synthetic DGN fixtures cannot enter the raster-input boundary");

    input.source = NEXUS_V1_DGN_FACE_MATERIAL_SOURCE_RETAIL_DGN;
    input.canonical_source_verified = 0;
    expect(!nexus_v1_dgn_face_material_validate(&input, &receipt) &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE,
           "unverified retail bytes cannot be promoted by their source label");

    input.canonical_source_verified = 1;
    bindings[2].face_ordinal = 1;
    expect(!nexus_v1_dgn_face_material_validate(&input, &receipt) &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_BINDING,
           "duplicate face ordinals fail closed");
    bindings[2].face_ordinal = 2;
    bindings[1].material_selector = 4;
    expect(!nexus_v1_dgn_face_material_validate(&input, &receipt) &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_BINDING &&
               strcmp(nexus_v1_dgn_face_material_status_name(receipt.status),
                      "blocked-binding") == 0,
           "out-of-range material selectors fail closed with stable status");

    if (failures) {
        fprintf(stderr, "Nexus DGN face/material provenance: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus DGN face/material provenance passed");
    return 0;
}
