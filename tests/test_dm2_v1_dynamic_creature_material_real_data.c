/* Canonical GRAPHICS.DAT proof for the live SKProject QUERY_CREATURE_PICST
 * route: FB/FC/FD selects a concrete CREATURES/type/dtImage, never F9. */
#include "dm2_v1_boot.h"
#include "dm2_v1_creature.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *data_root(void)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");

    if (root && root[0]) return root;
    return NULL;
}

int main(void)
{
    DM2_V1_BootProfile profile;
    const char *root = data_root();
    int found = 0;

    if (!root) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }
    dm2_v1_boot_profile_init(&profile);
    if (dm2_v1_boot_scan_assets(&profile, root) != 0 ||
        dm2_v1_boot_enter_game(&profile) != 0) {
        fputs("FAIL: selected canonical DM2 profile was not accepted\n", stderr);
        dm2_v1_boot_cleanup(&profile);
        return 1;
    }
    for (int creature = 0; creature < DM2_AI_TABLE_SIZE && !found; ++creature) {
        const DM2_AIDefinition *ai = dm2_v1_creature_ai_spec(creature);
        if (!ai || (ai->w0AIFlags & DM2_AIFLAG_STATIC) != 0u) continue;
        for (uint16_t command = 0u; command <= 0x40u; ++command) {
            DM2_V1_BootDynamicCreatureMaterialReceipt receipt;
            memset(&receipt, 0, sizeof(receipt));
            if (!dm2_v1_boot_dynamic_creature_material_receipt(
                    &profile, creature, command, 0xffffu, 2, &receipt)) {
                continue;
            }
            if (!receipt.valid || receipt.creature_type != creature ||
                receipt.command != command || receipt.previous_frame != 0xffffu ||
                receipt.direction != 2u ||
                receipt.image.category != DM2_GDAT_CATEGORY_CREATURES ||
                receipt.image.entry_index != creature ||
                receipt.image.field != receipt.image_field ||
                receipt.image_field == DM2_GDAT_IMG_MAP_CHIP ||
                receipt.animation_table_hash == 0u || receipt.material_hash == 0u ||
                receipt.palette_hash == 0u || receipt.image.raw_hash == 0u ||
                receipt.image.decoded_hash == 0u ||
                !receipt.raw_material_bytes ||
                receipt.raw_material_byte_count != receipt.image.raw_byte_count ||
                receipt.raw_material_hash == 0u ||
                receipt.raw_material_receipt_hash == 0u) {
                fputs("FAIL: dynamic creature receipt lost original GDAT ownership\n", stderr);
                dm2_v1_boot_cleanup(&profile);
                return 1;
            }
            printf("PASS: dynamic CREATURES/%02x dtImage/%02x material=%08x\n",
                   creature, receipt.image_field, receipt.material_hash);
            found = 1;
            break;
        }
    }
    dm2_v1_boot_cleanup(&profile);
    if (!found) {
        fputs("FAIL: selected corpus exposes no admitted dynamic V5 creature material\n",
              stderr);
        return 1;
    }
    return 0;
}
