#include "dm1_v1_c14_c15_graphics_catalog_pc34_compat.h"

#include <string.h>

static const char *const kSourceAnchor =
    "ReDMCSB GRF1.C/EXPAND.C decoded GRAPHICS.DAT catalog; "
    "DUNVIEW.C F0115:5668-6220 consumes C14/C15; "
    "DUNGEON.C F0142/G0209 resolves C14.Slot object material";

const char *dm1_v1_c14_c15_graphics_catalog_source_evidence_pc34(void)
{
    return kSourceAnchor;
}

static int catalog_surface_valid(const DM1_V1_ObjectWorldGraphicsSurfacePc34 *surface,
                                 uint32_t *outHash)
{
    size_t required;
    uint32_t hash;
    if (outHash) *outHash = 0u;
    if (!surface || !surface->verifiedPc34GraphicsDat || !surface->pixels ||
        surface->graphicIndex <= 0 || surface->width <= 0 || surface->height <= 0) {
        return 0;
    }
    required = (size_t)surface->width * (size_t)surface->height;
    if (surface->pixelCount < required) return 0;
    hash = dm1_v1_f0115_source_material_fnv1a_pc34(surface->pixels, required);
    if (hash == 0u) return 0;
    if (outHash) *outHash = hash;
    return 1;
}

int dm1_v1_c14_c15_graphics_catalog_build_pc34(
    const DM1_V1_ObjectWorldGraphicsSurfacePc34 *surfaces,
    int surfaceCount,
    DM1_V1_C14C15GraphicsCatalogPc34 *outCatalog)
{
    DM1_V1_C14C15GraphicsCatalogPc34 catalog;
    int i;
    if (!outCatalog) return 0;
    memset(&catalog, 0, sizeof(catalog));
    catalog.sourceAnchor = kSourceAnchor;
    *outCatalog = catalog;
    if (!surfaces || surfaceCount <= 0 ||
        surfaceCount > DM1_V1_C14_C15_GRAPHICS_CATALOG_MAX_SURFACES) return 1;
    for (i = 0; i < surfaceCount; ++i) {
        uint32_t hash;
        int j;
        if (!catalog_surface_valid(&surfaces[i], &hash)) return 1;
        for (j = 0; j < i; ++j) {
            if (catalog.entries[j].graphicIndex == (unsigned int)surfaces[i].graphicIndex) {
                return 1;
            }
        }
        catalog.entries[i].surface = &surfaces[i];
        catalog.entries[i].graphicIndex = (unsigned int)surfaces[i].graphicIndex;
        catalog.entries[i].pixelsFNV1a = hash;
    }
    catalog.valid = 1;
    catalog.surfaceCount = surfaceCount;
    *outCatalog = catalog;
    return 1;
}

static int catalog_has_graphic(const DM1_V1_C14C15GraphicsCatalogPc34 *catalog,
                               int graphicIndex)
{
    int i;
    if (!catalog || !catalog->valid || graphicIndex <= 0 ||
        catalog->surfaceCount <= 0 ||
        catalog->surfaceCount > DM1_V1_C14_C15_GRAPHICS_CATALOG_MAX_SURFACES) {
        return 0;
    }
    for (i = 0; i < catalog->surfaceCount; ++i) {
        const DM1_V1_C14C15GraphicsCatalogEntryPc34 *entry = &catalog->entries[i];
        uint32_t hash;
        if (entry->graphicIndex != (unsigned int)graphicIndex ||
            !catalog_surface_valid(entry->surface, &hash) ||
            hash != entry->pixelsFNV1a) {
            continue;
        }
        return 1;
    }
    return 0;
}

int dm1_v1_c14_c15_graphics_catalog_admit_receipt_pc34(
    const DM1_V1_C14C15GraphicsCatalogPc34 *catalog,
    const DM1_V1_F0248LiveEffectMaterialReceiptPc34 *receipt,
    DM1_V1_F0248LiveEffectKindPc34 kind,
    unsigned short rawThing,
    unsigned short associatedThing,
    int expectedGraphicIndex)
{
    if (!receipt || !receipt->valid || receipt->noDraw ||
        !receipt->saveReceiptBound || receipt->rawThing != rawThing ||
        receipt->graphicIndex != expectedGraphicIndex ||
        receipt->rawRecordFNV1a == 0u || receipt->graphicsPixelsFNV1a == 0u ||
        receipt->paletteFNV1a == 0u || !catalog_has_graphic(catalog, expectedGraphicIndex)) {
        return 0;
    }
    if (kind == DM1_V1_F0248_LIVE_EFFECT_PROJECTILE_C14_PC34) {
        return THING_GET_TYPE(rawThing) == THING_TYPE_PROJECTILE &&
               receipt->associatedThing == associatedThing;
    }
    if (kind == DM1_V1_F0248_LIVE_EFFECT_EXPLOSION_C15_PC34) {
        return THING_GET_TYPE(rawThing) == THING_TYPE_EXPLOSION &&
               associatedThing == THING_NONE && receipt->associatedThing == THING_NONE;
    }
    return 0;
}
