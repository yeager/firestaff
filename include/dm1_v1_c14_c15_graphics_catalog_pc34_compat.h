#ifndef FIRESTAFF_DM1_V1_C14_C15_GRAPHICS_CATALOG_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_C14_C15_GRAPHICS_CATALOG_PC34_COMPAT_H

#include "dm1_v1_f0115_source_material_handoff_pc34_compat.h"
#include "dm1_v1_object_world_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_C14_C15_GRAPHICS_CATALOG_MAX_SURFACES 64

/* A production-only view of PC34 GRAPHICS.DAT decoder output.  Entries keep
 * decoder-owned pointers and fingerprints, so a later buffer substitution is
 * rejected before F0115 receives C14/C15 material. */
typedef struct DM1_V1_C14C15GraphicsCatalogEntryPc34 {
    const DM1_V1_ObjectWorldGraphicsSurfacePc34 *surface;
    unsigned int graphicIndex;
    uint32_t pixelsFNV1a;
} DM1_V1_C14C15GraphicsCatalogEntryPc34;

typedef struct DM1_V1_C14C15GraphicsCatalogPc34 {
    int valid;
    int surfaceCount;
    DM1_V1_C14C15GraphicsCatalogEntryPc34 entries[
        DM1_V1_C14_C15_GRAPHICS_CATALOG_MAX_SURFACES];
    const char *sourceAnchor;
} DM1_V1_C14C15GraphicsCatalogPc34;

int dm1_v1_c14_c15_graphics_catalog_build_pc34(
    const DM1_V1_ObjectWorldGraphicsSurfacePc34 *surfaces,
    int surfaceCount,
    DM1_V1_C14C15GraphicsCatalogPc34 *outCatalog);

/* Requires the catalog fingerprint and the existing F0248 raw C14/C15
 * receipt to agree. C14 associated objects additionally retain their exact
 * F0142/G0209 Slot identity. Any mismatch is a no-draw admission failure. */
int dm1_v1_c14_c15_graphics_catalog_admit_receipt_pc34(
    const DM1_V1_C14C15GraphicsCatalogPc34 *catalog,
    const DM1_V1_F0248LiveEffectMaterialReceiptPc34 *receipt,
    DM1_V1_F0248LiveEffectKindPc34 kind,
    unsigned short rawThing,
    unsigned short associatedThing,
    int expectedGraphicIndex);

const char *dm1_v1_c14_c15_graphics_catalog_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
