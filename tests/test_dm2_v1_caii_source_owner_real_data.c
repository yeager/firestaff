/* Real-media admission proof for the private CAII prerequisite owner. */
#include "dm2_v1_boot.h"
#include "dm2_v1_caii_source_owner.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    DM2_V1_BootProfile profile;
    DM2_V1_CaiiSourceOwner owner;
    const DM2_V1_DungeonData *dungeon;
    const DM2_V1_AssetLoader *loader;
    const char *archive = getenv("FIRESTAFF_DM2_DOS_ARCHIVE");
    int comparable = 0;
    int distinct = 0;
    uint8_t previous = 0xffu;

    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_DM2_DOS_ARCHIVE is not configured");
        return 0;
    }
    dm2_v1_boot_profile_init(&profile);
    /* CAII retains only data admitted through the selected original ZIP.
     * Its private DB4/GDAT source state must never require a loose install. */
    if (dm2_v1_boot_scan_assets(&profile, archive) != 0 ||
        dm2_v1_boot_enter_game(&profile) != 0) {
        fputs("FAIL: canonical DM2 profile unavailable\n", stderr); return 1;
    }
    dungeon = (const DM2_V1_DungeonData *)profile.dungeon_data;
    loader = dm2_v1_boot_asset_loader(&profile);
    if (!dungeon || !loader || !dm2_v1_caii_source_owner_init(&owner, loader, dungeon) ||
        !owner.valid || !owner.slots_unowned || owner.source_hash == 0u ||
        owner.db4_record_count != (uint16_t)dungeon->thing_type_counts[4] ||
        owner.creature_binding_count == 0u || owner.ai_row_count != DM2_V1_SOURCE_AI_TABLE_SIZE) {
        fputs("FAIL: CAII source owner did not retain admitted source state\n", stderr);
        dm2_v1_caii_source_owner_free(&owner); dm2_v1_boot_cleanup(&profile); return 1;
    }
    for (int i = 0; i < DM2_CREATURE_TYPE_COUNT; ++i) {
        const DM2_AIDefinition *a = NULL, *b = NULL;
        if (dm2_v1_caii_source_owner_ai_spec_def(&owner, i, &a)) {
            if (!dm2_v1_creature_ai_spec_def(i, &b) || !a || !b ||
                a->w0AIFlags != b->w0AIFlags || a->BaseHP != b->BaseHP ||
                a->Defense != b->Defense) {
                fputs("FAIL: private AI row diverges from admitted GDAT route\n", stderr);
                dm2_v1_caii_source_owner_free(&owner); dm2_v1_boot_cleanup(&profile); return 1;
            }
            ++comparable;
        }
    }
    for (uint16_t i = 0u; i < owner.db4_record_count; ++i) {
        uint8_t type = 0u;
        if (!dm2_v1_caii_source_owner_db4_type(&owner, i, &type)) {
            fputs("FAIL: private DB4 source copy is incomplete\n", stderr);
            dm2_v1_caii_source_owner_free(&owner); dm2_v1_boot_cleanup(&profile); return 1;
        }
        if (type != previous) { ++distinct; previous = type; }
    }
    if (comparable == 0 || distinct == 0 ||
        dm2_v1_caii_source_owner_db4_type(&owner, owner.db4_record_count, &previous)) {
        fputs("FAIL: CAII source owner query bounds are not fail-closed\n", stderr);
        dm2_v1_caii_source_owner_free(&owner); dm2_v1_boot_cleanup(&profile); return 1;
    }
    printf("PASS: private CAII owner rows=%u bindings=%u DB4=%u hash=%08x\n",
           owner.ai_row_count, owner.creature_binding_count, owner.db4_record_count,
           owner.source_hash);
    dm2_v1_caii_source_owner_free(&owner);
    dm2_v1_boot_cleanup(&profile);
    return 0;
}
