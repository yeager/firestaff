#ifndef FIRESTAFF_DM1_V1_F0115_SOURCE_MATERIAL_HANDOFF_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0115_SOURCE_MATERIAL_HANDOFF_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "dm1_v1_f0115_square_material_scheduler_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

/* Source-bound F0115 object and C14 material handoff. ReDMCSB DUNVIEW.C
 * :4820-5078 resolves C2500/G0209 piles; :5668-5900 resolves C2900/M613 or
 * F0142/G0209 projectiles. Missing original pixels always remain no-draw. */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DM1_V1_F0115SourceMaterialKindPc34 {
    DM1_V1_F0115_SOURCE_MATERIAL_FLOOR_OBJECT_PC34 = 0,
    DM1_V1_F0115_SOURCE_MATERIAL_THROWN_OBJECT_PC34 = 1,
    DM1_V1_F0115_SOURCE_MATERIAL_NATIVE_PROJECTILE_PC34 = 2
} DM1_V1_F0115SourceMaterialKindPc34;

typedef struct DM1_V1_F0115SourcePixelsPc34 {
    const uint8_t *pixels;
    size_t pixelCount;
    unsigned int graphicIndex;
    int width;
    int height;
    int sourceOwned;
    /* Set only by the PC34 GRAPHICS.DAT decoder after its raw entry and
     * indexed surface have been bound. Generic F0115 callers may omit it;
     * F0248/F0810 receipts require it. */
    int verifiedPc34GraphicsDat;
} DM1_V1_F0115SourcePixelsPc34;

/* F0248/F0810 live-effect admission is intentionally stricter than the
 * generic F0115 surface handoff.  It needs the exact raw C14/C15 row and the
 * decoder-owned indexed palette, so a host-generated sprite cannot acquire a
 * render or original-save receipt. */
typedef enum DM1_V1_F0248LiveEffectKindPc34 {
    DM1_V1_F0248_LIVE_EFFECT_PROJECTILE_C14_PC34 = 14,
    DM1_V1_F0248_LIVE_EFFECT_EXPLOSION_C15_PC34 = 15
} DM1_V1_F0248LiveEffectKindPc34;

typedef struct DM1_V1_F0248LiveEffectMaterialInputPc34 {
    DM1_V1_F0248LiveEffectKindPc34 kind;
    const struct DungeonThings_Compat *things;
    unsigned short rawThing;
    /* C14's stored Slot; unused for C15. */
    unsigned short associatedThing;
    /* C15's decoded Type/Attack/Centered; unused for C14. */
    int explosionType;
    int explosionAttack;
    int explosionCentered;
    int expectedGraphicIndex;
    const DM1_V1_F0115SourcePixelsPc34 *surface;
    const uint8_t *palette;
    size_t paletteByteCount;
    int paletteOwnedByPc34GraphicsDat;
} DM1_V1_F0248LiveEffectMaterialInputPc34;

typedef struct DM1_V1_F0248LiveEffectMaterialReceiptPc34 {
    int valid;
    int noDraw;
    int saveReceiptBound;
    unsigned short rawThing;
    /* C14 keeps the exact F0212.Slot consumed by F0142/G0209.  C15 has no
     * associated object and stores THING_NONE. */
    unsigned short associatedThing;
    int graphicIndex;
    uint32_t rawRecordFNV1a;
    uint32_t graphicsPixelsFNV1a;
    uint32_t paletteFNV1a;
    const char *sourceAnchor;
} DM1_V1_F0248LiveEffectMaterialReceiptPc34;

typedef struct DM1_V1_F0115SourceMaterialInputPc34 {
    DM1_V1_F0115SourceMaterialKindPc34 kind;
    int thingType;
    int subtype;
    int projectileSubtype;
    int weaponProjectileAspectOrdinal;
    int relativeForward;
    int relativeSide;
    int relativeCell;
    int sourceZoneRow;
    int pileIndex;
    int viewportX;
    int viewportY;
    int viewportW;
    int viewportH;
    const DM1_V1_F0115SourcePixelsPc34 *surface;
} DM1_V1_F0115SourceMaterialInputPc34;

typedef struct DM1_V1_F0115SourceMaterialHandoffPc34 {
    int valid;
    int noDraw;
    int usesF0791Blit;
    int transparentColor;
    int graphicIndex;
    int sourceZone;
    int sourceZoneRow;
    int pileIndex;
    int drawX;
    int drawY;
    int drawW;
    int drawH;
    int mirror;
    uint32_t materialFNV1a;
    const uint8_t *pixels;
    size_t pixelCount;
    const char *sourceAnchor;
} DM1_V1_F0115SourceMaterialHandoffPc34;

uint32_t dm1_v1_f0115_source_material_fnv1a_pc34(const uint8_t *bytes,
                                                  size_t byteCount);
int dm1_v1_f0115_source_material_handoff_pc34(
    const DM1_V1_F0115SourceMaterialInputPc34 *input,
    DM1_V1_F0115SourceMaterialHandoffPc34 *outHandoff);

/* Converts an admitted handoff into the existing per-square F0128 scheduler
 * input. The scheduler never receives a raw, unverified surface. */
int dm1_v1_f0115_source_material_to_square_pc34(
    int viewSquare,
    int materialKind,
    const DM1_V1_F0115SourceMaterialHandoffPc34 *handoff,
    DM1_V1_F0115SquareMaterialPc34 *outMaterial);

/* Builds the C14/C15 receipt used by F0248/F0810 live projectile and
 * explosion materialization.  Any missing raw row, decoded/raw disagreement,
 * unowned GRAPHICS.DAT pixels, or absent original palette returns a valid
 * no-draw receipt. */
int dm1_v1_f0248_live_effect_material_receipt_pc34(
    const DM1_V1_F0248LiveEffectMaterialInputPc34 *input,
    DM1_V1_F0248LiveEffectMaterialReceiptPc34 *outReceipt);

#ifdef __cplusplus
}
#endif

#endif
