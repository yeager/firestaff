/* Real GRAPHICS.DAT proof for SkWinCore::EXTENDED_LOAD_SPELLS_DEFINITION.
 * The independent scan deliberately uses the original GDAT fields rather
 * than a Firestaff spell table, then verifies the boot-to-M11 receipt path. */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_boot.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value + 0x9e3779b9u + (hash << 6) + (hash >> 2);
    return hash ? hash : 1u;
}

static int scan_extended_spells(const DM2_V1_AssetLoader *loader,
                                uint32_t *out_count,
                                uint32_t *out_hash)
{
    uint32_t hash = 0x4553504cu;
    uint32_t count = 0u;
    uint32_t index;

    *out_count = 0u;
    *out_hash = 0u;
    for (index = 0u; index < 254u; ++index) {
        uint16_t words[7];
        const uint8_t *name;
        size_t name_size = 0u;
        uint32_t field;

        if (!dm2_v1_asset_load_word_value(loader,
                                           DM2_GDAT_CATEGORY_SPELL_DEF,
                                           (int)index, 1, &words[0]) ||
            words[0] == 0u) {
            continue;
        }
        for (field = 2u; field <= 7u; ++field) {
            if (!dm2_v1_asset_load_word_value(
                    loader, DM2_GDAT_CATEGORY_SPELL_DEF, (int)index,
                    (int)field, &words[field - 1u])) {
                return 0;
            }
        }
        name = dm2_v1_asset_load_text_sized(
            loader, DM2_GDAT_CATEGORY_SPELL_DEF, (int)index, 0x18,
            &name_size);
        if (!name || name_size == 0u) return 0;
        hash = hash_step(hash, index);
        for (field = 0u; field < 7u; ++field) {
            hash = hash_step(hash, words[field]);
        }
        for (field = 0u; field < name_size; ++field) {
            hash = hash_step(hash, name[field]);
        }
        hash = hash_step(hash, (uint32_t)name_size);
        ++count;
    }
    *out_count = count;
    *out_hash = count ? hash : 0u;
    return 1;
}

static int same_receipt(const DM2_V1_ExtendedSpellsDefinitionReceipt *a,
                        const DM2_V1_ExtendedSpellsDefinitionReceipt *b)
{
    return a && b && a->loaded == b->loaded &&
           a->spell_count == b->spell_count && a->gdat_hash == b->gdat_hash;
}

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM2_DOS_ARCHIVE");
    char graphics_path[2048];
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_BootProfile boot;
    DM2_V1_BootStartupHostViewReceipt host;
    DM2_V1_BootStartupPackagedFullStartReceipt package;
    DM2_V1_BootStartupPackagedConsumerReceipt consumer;
    DM2_V1_BootStartupHostFrameReceipt host_frame;
    DM2_V1_BootStartupRenderOwnershipReceipt ownership;
    uint32_t expected_count = 0u;
    uint32_t expected_hash = 0u;
    int failures = 0;

    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_DM2_DOS_ARCHIVE is not set");
        return 0;
    }
    snprintf(graphics_path, sizeof(graphics_path),
             "%s::data/graphics.dat", archive);
    if (!asset_read_path_alloc(graphics_path, &graphics, &graphics_size) ||
        !graphics || graphics_size == 0u) {
        fputs("FAIL: selected canonical DM2 GRAPHICS.DAT is unreadable\n", stderr);
        return 1;
    }
    memset(&loader, 0, sizeof(loader));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        !scan_extended_spells(&loader, &expected_count, &expected_hash)) {
        fputs("FAIL: canonical SPELL_DEF GDAT was not readable\n", stderr);
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        return 1;
    }
    if (expected_count == 0u) {
        puts("PASS: selected GRAPHICS.DAT has no source SPELL_DEF rows; extended spells remain unavailable");
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        return 0;
    }

    dm2_v1_boot_profile_init(&boot);
    /* GRAPHICS.DAT and the boot profile use the identical user-selected
     * archive; ZIP material remains an in-memory virtual member. */
    if (dm2_v1_boot_scan_assets(&boot, archive) != 0 ||
        dm2_v1_boot_enter_game(&boot) != 0 ||
        !dm2_v1_boot_startup_host_view_receipt_from_runtime_state(
            &boot, 1, boot.save_root, 0, 0u, 0, 0, &host) ||
        !dm2_v1_boot_startup_packaged_full_start_receipt_from_host_view(
            &host, &package) ||
        !dm2_v1_boot_startup_packaged_consumer_receipt_from_full_start(
            &package, &consumer) ||
        !dm2_v1_boot_startup_host_frame_receipt_from_consumer(
            &consumer, &host_frame) ||
        !dm2_v1_boot_startup_render_ownership_receipt_from_runtime_state(
            &boot, 1, boot.save_root, 0, 0u, 0, 0, &ownership)) {
        fputs("FAIL: boot-to-M11 receipt path was not admitted\n", stderr);
        ++failures;
    } else {
        DM2_V1_ExtendedSpellsDefinitionReceipt expected = {
            1, expected_count, expected_hash
        };
        if (!same_receipt(&host.extended_spells, &expected) ||
            !same_receipt(&package.extended_spells, &expected) ||
            !same_receipt(&consumer.extended_spells, &expected) ||
            !same_receipt(&host_frame.extended_spells, &expected) ||
            !same_receipt(&ownership.extended_spells, &expected) ||
            !ownership.extended_spells_definition_consumed) {
            fputs("FAIL: SPELL_DEF receipt changed before M11 consumption\n", stderr);
            ++failures;
        }
    }
    dm2_v1_boot_cleanup(&boot);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    if (failures) return 1;
    printf("PASS: extended SPELL_DEF receipt count=%u hash=%08x\n",
           expected_count, expected_hash);
    return 0;
}
