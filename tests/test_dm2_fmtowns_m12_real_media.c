/* Opt-in real-media regression for the DM2 FM Towns launcher receipt.
 *
 * The archive remains user-owned.  This test only scans it and asserts the
 * M12 receipt keeps the virtual source path; it never extracts a game file to
 * disk.  Set FIRESTAFF_DM2_FMTOWNS_ROOT to either the global data root (with
 * dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip below it) or the dm2
 * directory itself. */

#include "asset_status_m12.h"
#include "dm2_v1_asset_loader.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void expect(int condition, const char* message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

/* The FM Towns disc remains the native Japanese GDAT owner, while the
 * explicitly selected PC-English corpus supplies the overlay.  Do not call
 * the overlay complete merely because one known label (SAVE) resolves: every
 * native non-empty GDAT text key must have a non-empty companion value before
 * an English Towns session is admitted.  Both loaders operate on the selected
 * RAM buffers; no archive member is materialised on disk. */
static void expect_complete_english_text_overlay(
    const DM2_V1_BootProfile* profile)
{
    DM2_V1_AssetLoader native_loader;
    DM2_V1_GdatEntryIterator iterator;
    DM2_V1_GdatEntryQueryReceipt entry;
    unsigned int native_text_count = 0u;
    unsigned int missing_text_count = 0u;

    if (!profile || !profile->graphics_mem || profile->graphics_mem_size == 0u ||
        dm2_v1_asset_loader_init(&native_loader, profile->graphics_mem,
                                  profile->graphics_mem_size) != 0) {
        expect(0, "FM Towns English has its selected native GDAT in RAM");
        return;
    }

    memset(&iterator, 0, sizeof(iterator));
    iterator.category_first = 0;
    iterator.category_last = DM2_GDAT_CATEGORY_LIMIT;
    iterator.index_filter = -1;
    iterator.type_filter = DM2_GDAT_ENTRY_TYPE_TEXT;
    iterator.field_filter = -1;
    while (dm2_v1_query_next_gdat_entry(&native_loader, &iterator, &entry)) {
        const uint8_t* text;
        size_t text_size = 0u;

        if (!entry.present || entry.raw_length == 0u) {
            continue;
        }
        ++native_text_count;
        text = dm2_v1_runtime_i18n_text(entry.category, entry.index,
                                        entry.field, &text_size);
        if (!text || text_size == 0u) {
            ++missing_text_count;
        }
    }
    dm2_v1_asset_loader_free(&native_loader);
    expect(native_text_count > 0u,
           "FM Towns CD exposes original GDAT text for the English coverage check");
    expect(missing_text_count == 0u,
           "every non-empty FM Towns GDAT text key has a real English companion value");
}

int main(void)
{
    const char* root = getenv("FIRESTAFF_DM2_FMTOWNS_ROOT");
    const char* english_companion =
        getenv("FIRESTAFF_DM2_ENGLISH_COMPANION");
    const char* english_companion_archive =
        getenv("FIRESTAFF_DM2_ENGLISH_COMPANION_ARCHIVE");
    const M12_AssetVersionStatus* version;
    const M12_AssetRequiredFileStatus* graphics;
    const M12_AssetRequiredFileStatus* dungeon;
    M12_AssetStatus status;
    char selectedRuntime[512];
    int versionIndex;

    if (!root || root[0] == '\0') {
        puts("SKIP: FIRESTAFF_DM2_FMTOWNS_ROOT is not set");
        return 0;
    }

    memset(&status, 0, sizeof(status));
    memset(selectedRuntime, 0, sizeof(selectedRuntime));
    M12_AssetStatus_ScanGame(&status, root, "dm2");
    versionIndex = M12_AssetStatus_FindVersionIndex("dm2", "fmtowns-ja");
    version = versionIndex >= 0
        ? M12_AssetStatus_GetVersion(&status, "dm2", (size_t)versionIndex)
        : NULL;
    graphics = M12_AssetStatus_GetRequiredFile(&status, "dm2", 0U);
    dungeon = M12_AssetStatus_GetRequiredFile(&status, "dm2", 1U);

    expect(M12_AssetStatus_GameAvailable(&status, "dm2") == 1,
           "FM Towns original ZIP is launch-admitted by M12");
    expect(version && version->matched &&
               strcmp(version->matchedMd5,
                      "027ff3b8ddc2c4c4cdda7ada0b0bc46c") == 0 &&
               strstr(version->matchedPath,
                      ".zip::DATA/GRAPHICS.DAT") != NULL,
           "M12 records the verified FM Towns GDAT as virtual provenance");
    /* Required-file rows are the scan's default launch pair.  In a shared
     * root that may correctly be PC-DOS, so edition-specific provenance is
     * asserted through the resolver below.  A direct archive request has no
     * competing edition and must publish its two virtual members directly. */
    if (strstr(root, "Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip") != NULL) {
        expect(graphics && graphics->matched &&
                   strcmp(graphics->matchedHash,
                          "027ff3b8ddc2c4c4cdda7ada0b0bc46c") == 0 &&
                   strstr(graphics->matchedPath,
                          ".zip::DATA/GRAPHICS.DAT") != NULL,
               "direct FM Towns GRAPHICS.DAT remains a virtual archive member");
        expect(dungeon && dungeon->matched &&
                   strcmp(dungeon->matchedHash,
                          "74c7549f174574201988bf936385841a") == 0 &&
                   strstr(dungeon->matchedPath,
                          ".zip::DATA/DUNGEON.DAT") != NULL,
               "direct FM Towns DUNGEON.DAT remains a virtual archive member");
    }
    expect(M12_AssetStatus_ResolveRuntimeDataDirForVersion(
               &status, "dm2", "fmtowns-ja", selectedRuntime,
               sizeof(selectedRuntime)) &&
               strstr(selectedRuntime,
                      "Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip") != NULL,
           "selected FM Towns edition retains its original archive handoff");

    /* The Japanese CD is still the session owner. English is admitted only
     * through this separately selected canonical PC corpus; both sources are
     * read in memory and the ZIP is never unpacked to disk. */
    if (english_companion && english_companion[0] != '\0') {
        DM2_V1_BootStartupLaunch launch;
        DM2_V1_BootStartupLaunch missing_companion;
        DM2_V1_DialogueOpenPanelHostCommand dialogue;
        const uint8_t* text;
        size_t text_size = 0u;
        memset(&missing_companion, 0, sizeof(missing_companion));
        expect(dm2_v1_boot_startup_launch_alloc_with_language(
                   selectedRuntime, NULL, 0, &missing_companion) == 0,
               "FM Towns English refuses to launch without its real companion corpus");
        memset(&launch, 0, sizeof(launch));
        expect(dm2_v1_boot_startup_launch_alloc_with_language(
                   selectedRuntime, english_companion, 0, &launch) == 1,
               "FM Towns English requires and accepts the explicit PC-English companion");
        expect(launch.profile &&
                   launch.profile->platform == DM2_PLATFORM_FMTOWNS_JA &&
                   dm2_v1_runtime_i18n_ready(),
               "FM Towns runtime keeps Japanese CD ownership and binds English text only");
        expect_complete_english_text_overlay(launch.profile);
        text = dm2_v1_runtime_i18n_text(0x07, 0x00, 0x00, &text_size);
        expect(text && text_size >= 7u && memcmp(text, "FIGHTER", 7u) == 0,
               "English text comes from the authenticated PC GDAT companion");
        memset(&dialogue, 0, sizeof(dialogue));
        expect(dm2_v1_boot_dialogue_open_panel_host_command(
                   launch.profile, &dialogue) && dialogue.valid &&
                   strcmp((const char *)dialogue.draw.text[0], "SAVE") == 0 &&
                   strcmp((const char *)dialogue.draw.text[1], "CANCEL") == 0,
               "FM Towns save dialogue consumes English labels from the PC companion");
        dm2_v1_boot_startup_launch_cleanup(&launch);

        /* Archive provenance must be accepted through the same RAM-only
         * companion path.  This is the normal distribution form for the
         * user-owned DOS edition; no member may be materialised on disk. */
        if (english_companion_archive && english_companion_archive[0] != '\0') {
            char virtual_companion[1024];
            memset(&launch, 0, sizeof(launch));
            snprintf(virtual_companion, sizeof(virtual_companion),
                     "%s::data/graphics.dat", english_companion_archive);
            expect(dm2_v1_boot_startup_launch_alloc_with_language(
                       selectedRuntime, virtual_companion, 0, &launch) == 1,
                   "FM Towns English accepts a verified PC GDAT ZIP member in RAM");
            expect_complete_english_text_overlay(launch.profile);
            text = dm2_v1_runtime_i18n_text(0x07, 0x00, 0x00, &text_size);
            expect(text && text_size >= 7u && memcmp(text, "FIGHTER", 7u) == 0,
                   "ZIP companion supplies authenticated English text");
            memset(&dialogue, 0, sizeof(dialogue));
            expect(dm2_v1_boot_dialogue_open_panel_host_command(
                   launch.profile, &dialogue) && dialogue.valid &&
                   strcmp((const char *)dialogue.draw.text[0], "SAVE") == 0 &&
                   strcmp((const char *)dialogue.draw.text[1], "CANCEL") == 0,
                   "FM Towns save dialogue uses English labels from the ZIP companion");
            dm2_v1_boot_startup_launch_cleanup(&launch);

            /* The real DOS archive stores the member as DATA/GRAPHICS.DAT.
             * M12 keeps that source spelling in virtual provenance, so it
             * must pass the same RAM-only, hash-verified companion gate. */
            memset(&launch, 0, sizeof(launch));
            snprintf(virtual_companion, sizeof(virtual_companion),
                     "%s::DATA/GRAPHICS.DAT", english_companion_archive);
            expect(dm2_v1_boot_startup_launch_alloc_with_language(
                       selectedRuntime, virtual_companion, 0, &launch) == 1,
                   "FM Towns English accepts the original uppercase ZIP member");
            expect_complete_english_text_overlay(launch.profile);
            dm2_v1_boot_startup_launch_cleanup(&launch);
        }
    } else {
        puts("SKIP: FIRESTAFF_DM2_ENGLISH_COMPANION is not set");
    }

    if (failures != 0) {
        return 1;
    }
    puts("PASS: DM2 FM Towns M12 real-media receipt stays in the original ZIP");
    return 0;
}
