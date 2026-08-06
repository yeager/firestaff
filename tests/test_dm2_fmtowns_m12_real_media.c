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
#include "dm2_v1_fmtowns_cdda_music.h"
#include "dm2_v1_fmtowns_disc.h"
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
        expect(launch.profile &&
                   launch.profile->fmtowns_startup_media_verified &&
                   launch.profile->fmtowns_animation_media_verified &&
                   launch.profile->fmtowns_animation_streams_verified &&
                   strcmp(launch.profile->fmtowns_twanim_md5,
                          "07a5629466e0c941bdc27c78cf8b9941") == 0 &&
                   strcmp(launch.profile->fmtowns_skull_md5,
                          "0f4b44d286cbee35924a95e7d75ad7e5") == 0 &&
                   strcmp(launch.profile->fmtowns_swoosh_md5,
                          "ecec4d7ac081b099056531043191b55a") == 0 &&
                   strcmp(launch.profile->fmtowns_title_md5,
                          "d795bab0b392b61534f64163fbbedc38") == 0 &&
                   strcmp(launch.profile->fmtowns_end_md5,
                          "b4a6a38657ac3c1857872952a25964d4") == 0 &&
                   launch.profile->fmtowns_startup_plan.valid &&
                   launch.profile->fmtowns_startup_plan.stage_count == 4 &&
                   launch.profile->fmtowns_startup_plan.stages[0] ==
                       DM2_FMTOWNS_STARTUP_STAGE_SWOOSH &&
                   launch.profile->fmtowns_startup_plan.stages[1] ==
                       DM2_FMTOWNS_STARTUP_STAGE_TITLE &&
                   launch.profile->fmtowns_startup_plan.stages[2] ==
                       DM2_FMTOWNS_STARTUP_STAGE_SKULL &&
                   launch.profile->fmtowns_startup_plan.stages[3] ==
                       DM2_FMTOWNS_STARTUP_STAGE_END,
               "FM Towns follows its original AUTOEXEC animation and startup order");
        expect(launch.profile &&
                   dm2_v1_fmtowns_anim_stream_is_hme242_swoosh(
                       &launch.profile->fmtowns_swoosh_stream) &&
                   dm2_v1_fmtowns_anim_stream_is_hme242_title(
                       &launch.profile->fmtowns_title_stream) &&
                   dm2_v1_fmtowns_anim_stream_is_hme242_end(
                       &launch.profile->fmtowns_end_stream),
                   "FM Towns startup media has complete retail executable and stream identities");
        {
            DM2_V1_FmtownsDiscReceipt disc;
            DM2_V1_FmtownsAnimFrameReceipt first_frame;
            DM2_V1_FmtownsAnimFrameReceipt last_frame;
            DM2_V1_FmtownsAnimFrameReceipt swoosh_first_frame;
            DM2_V1_FmtownsAnimFrameReceipt swoosh_last_frame;
            DM2_V1_FmtownsAnimFrameReceipt end_last_frame;
            DM2_V1_FmtownsAnimPaletteReceipt title_palette;
            DM2_V1_FmtownsAnimPaletteReceipt end_first_palette;
            DM2_V1_FmtownsAnimPaletteReceipt end_middle_palette;
            DM2_V1_FmtownsAnimPaletteReceipt end_last_palette;
            DM2_V1_FmtownsAnimSoundReceipt title_sound;
            uint8_t *title = NULL;
            size_t title_size = 0u;
            uint8_t *swoosh = NULL;
            size_t swoosh_size = 0u;
            uint8_t *end = NULL;
            size_t end_size = 0u;
            uint8_t *twanim = NULL;
            size_t twanim_size = 0u;
            uint8_t *skull = NULL;
            size_t skull_size = 0u;
            uint8_t pixels[320u * 200u / 2u];
            int first_ok = 0;
            int last_ok = 0;
            int swoosh_first_ok = 0;
            int swoosh_last_ok = 0;
            unsigned int end_frame_count = 0u;

            memset(&disc, 0, sizeof(disc));
            memset(&first_frame, 0, sizeof(first_frame));
            memset(&last_frame, 0, sizeof(last_frame));
            memset(&swoosh_first_frame, 0, sizeof(swoosh_first_frame));
            memset(&swoosh_last_frame, 0, sizeof(swoosh_last_frame));
            memset(&end_last_frame, 0, sizeof(end_last_frame));
            memset(&title_palette, 0, sizeof(title_palette));
            memset(&end_first_palette, 0, sizeof(end_first_palette));
            memset(&end_middle_palette, 0, sizeof(end_middle_palette));
            memset(&end_last_palette, 0, sizeof(end_last_palette));
            memset(&title_sound, 0, sizeof(title_sound));
            if (launch.profile && launch.profile->fmtowns_disc_image &&
                       dm2_v1_fmtowns_disc_probe(
                           launch.profile->fmtowns_disc_image,
                           launch.profile->fmtowns_disc_image_size,
                           &disc) == 0 &&
                       dm2_v1_fmtowns_disc_extract_alloc(
                           launch.profile->fmtowns_disc_image,
                           launch.profile->fmtowns_disc_image_size,
                           &disc.title, &title, &title_size) == 0) {
                first_ok = dm2_v1_fmtowns_anim_stream_decode_frame(
                           title, title_size, 0u, pixels, sizeof(pixels),
                           &first_frame);
                last_ok = dm2_v1_fmtowns_anim_stream_decode_frame(
                           title, title_size, 224u, pixels, sizeof(pixels),
                           &last_frame);
                (void)dm2_v1_fmtowns_anim_stream_decode_palette(
                    title, title_size, &title_palette);
                (void)dm2_v1_fmtowns_anim_stream_decode_title_sound(
                    title, title_size, &title_sound);
            }
            if (launch.profile && launch.profile->fmtowns_disc_image &&
                dm2_v1_fmtowns_disc_extract_alloc(
                    launch.profile->fmtowns_disc_image,
                    launch.profile->fmtowns_disc_image_size,
                    &disc.twanim_exp, &twanim, &twanim_size) != 0) {
                free(twanim);
                twanim = NULL;
                twanim_size = 0u;
            }
            if (launch.profile && launch.profile->fmtowns_disc_image &&
                dm2_v1_fmtowns_disc_probe(
                    launch.profile->fmtowns_disc_image,
                    launch.profile->fmtowns_disc_image_size, &disc) == 0 &&
                dm2_v1_fmtowns_disc_extract_alloc(
                    launch.profile->fmtowns_disc_image,
                    launch.profile->fmtowns_disc_image_size,
                    &disc.swoosh, &swoosh, &swoosh_size) == 0) {
                swoosh_first_ok = dm2_v1_fmtowns_anim_stream_decode_frame(
                    swoosh, swoosh_size, 0u, pixels, sizeof(pixels),
                    &swoosh_first_frame);
                swoosh_last_ok = dm2_v1_fmtowns_anim_stream_decode_frame(
                    swoosh, swoosh_size, 18u, pixels, sizeof(pixels),
                    &swoosh_last_frame);
            }
            if (launch.profile && launch.profile->fmtowns_disc_image &&
                dm2_v1_fmtowns_disc_probe(
                    launch.profile->fmtowns_disc_image,
                    launch.profile->fmtowns_disc_image_size, &disc) == 0 &&
                dm2_v1_fmtowns_disc_extract_alloc(
                    launch.profile->fmtowns_disc_image,
                    launch.profile->fmtowns_disc_image_size,
                    &disc.end, &end, &end_size) == 0) {
                while (end_frame_count < 4096u) {
                    DM2_V1_FmtownsAnimFrameReceipt candidate;
                    memset(&candidate, 0, sizeof(candidate));
                    if (!dm2_v1_fmtowns_anim_stream_decode_frame(
                            end, end_size, end_frame_count, pixels,
                            sizeof(pixels), &candidate)) {
                        break;
                    }
                    end_last_frame = candidate;
                    ++end_frame_count;
                }
                (void)dm2_v1_fmtowns_anim_stream_decode_palette_for_frame(
                    end, end_size, 0u, &end_first_palette);
                (void)dm2_v1_fmtowns_anim_stream_decode_palette_for_frame(
                    end, end_size, 100u, &end_middle_palette);
                (void)dm2_v1_fmtowns_anim_stream_decode_palette_for_frame(
                    end, end_size, 419u, &end_last_palette);
            }
            if (launch.profile && launch.profile->fmtowns_disc_image &&
                dm2_v1_fmtowns_disc_probe(
                    launch.profile->fmtowns_disc_image,
                    launch.profile->fmtowns_disc_image_size, &disc) == 0) {
                (void)dm2_v1_fmtowns_disc_extract_alloc(
                    launch.profile->fmtowns_disc_image,
                    launch.profile->fmtowns_disc_image_size,
                    &disc.skull_exp, &skull, &skull_size);
            }
            /* FNV-1a receipts are of the two 320x200/4bpp buffers decoded
             * from the selected HME-242 TITLE stream, not stored artwork. */
            expect(first_ok && last_ok && first_frame.valid &&
                       first_frame.width == 320u && first_frame.height == 200u &&
                       first_frame.decoded_frame_count == 1u &&
                       last_frame.valid && last_frame.decoded_frame_count == 225u &&
                       last_frame.display_duration == 6u &&
                       first_frame.compressed_command_count == 14458u &&
                       last_frame.compressed_command_count == 9u &&
                       first_frame.output_fnv1a == 0xc7ad2279u &&
                       last_frame.output_fnv1a == 0x5ef57a09u,
                   "FM Towns TITLE decodes its first and final retail frames in RAM");
            expect(title_palette.valid && title_palette.color_count == 16u &&
                       title_palette.source_record_offset != 0u &&
                       title_palette.output_fnv1a != 0u,
                   "FM Towns TITLE decodes its original PL palette in RAM");
            expect(title_sound.valid && title_sound.source_record_offset == 14u &&
                       title_sound.sample_count == 12862u &&
                       title_sound.samples != NULL &&
                       title_sound.sample_fnv1a == 0x0b829ae7u &&
                       title_sound.event_count == 5u &&
                       title_sound.events[0].source_record_offset == 101790u &&
                       title_sound.events[0].preceding_frame_count == 14u &&
                       title_sound.events[0].sound_index == 1u &&
                       title_sound.events[0].left_volume == 70u &&
                       title_sound.events[0].right_volume == 10u &&
                       title_sound.events[4].source_record_offset == 492266u &&
                       title_sound.events[4].preceding_frame_count == 131u &&
                       title_sound.events[4].left_volume == 255u &&
                       title_sound.events[4].right_volume == 255u &&
                       title_sound.events[0].source_frequency_hz == 1000u &&
                       title_sound.events[4].source_frequency_hz == 1000u &&
                       title_sound.events[0].player_frequency_hz == 5500u &&
                       title_sound.events[4].player_frequency_hz == 5500u,
                   "FM Towns TITLE retains its retail SND2 and five SO events in RAM");
            expect(swoosh_first_ok && swoosh_last_ok &&
                       swoosh_first_frame.width == 320u &&
                       swoosh_first_frame.height == 200u &&
                       swoosh_first_frame.requested_frame == 0u &&
                       swoosh_first_frame.decoded_frame_count == 1u &&
                       swoosh_first_frame.display_duration == 0u &&
                       swoosh_last_frame.width == 320u &&
                       swoosh_last_frame.height == 200u &&
                       swoosh_last_frame.requested_frame == 18u &&
                       swoosh_last_frame.decoded_frame_count == 19u &&
                       swoosh_last_frame.display_duration == 126u &&
                       swoosh_first_frame.output_fnv1a != 0u &&
                       swoosh_last_frame.output_fnv1a != 0u,
                   "FM Towns SWOOSH infers its retail IMG1 canvas from EN/DL records in RAM");
            expect(end_frame_count == 420u && end_last_frame.valid &&
                       end_last_frame.requested_frame == 419u &&
                       end_last_frame.output_fnv1a == 0x553d172fu &&
                       end_last_frame.display_duration == 2000u &&
                       end_last_frame.compressed_command_count == 1u,
                   "FM Towns END replays its original FO/NE loops in RAM");
            expect(end_first_palette.valid &&
                       end_first_palette.source_record_offset == 34u &&
                       end_first_palette.output_fnv1a == 0xfd41be13u &&
                       end_middle_palette.valid &&
                       end_middle_palette.source_record_offset == 111174u &&
                       end_middle_palette.output_fnv1a == 0xce718356u &&
                       end_last_palette.valid &&
                       end_last_palette.source_record_offset == 466082u &&
                       end_last_palette.output_fnv1a == 0xc4440608u,
                   "FM Towns END binds each displayed frame to its original PL palette");
            expect(skull && skull_size == 374416u &&
                       launch.profile->fmtowns_skull_p3.valid &&
                       launch.profile->fmtowns_skull_p3.level == 1u &&
                       launch.profile->fmtowns_skull_p3.header_size == 0x180u &&
                       launch.profile->fmtowns_skull_p3.file_size == skull_size &&
                       launch.profile->fmtowns_skull_p3.runtime_offset == 0x180u &&
                       launch.profile->fmtowns_skull_p3.runtime_size == 0x80u &&
                       launch.profile->fmtowns_skull_p3.relocation_offset == 0x200u &&
                       launch.profile->fmtowns_skull_p3.relocation_size == 0u &&
                       launch.profile->fmtowns_skull_p3.load_image_offset == 0x200u &&
                       launch.profile->fmtowns_skull_p3.load_image_size == 0x5b490u &&
                       launch.profile->fmtowns_skull_p3.symbol_table_offset == 0u &&
                       launch.profile->fmtowns_skull_p3.symbol_table_size == 0u &&
                       launch.profile->fmtowns_skull_p3.initial_eip == 0x5741cu &&
                       launch.profile->fmtowns_skull_p3.memory_requirements ==
                           0x5b490u &&
                       launch.profile->fmtowns_cdda_music.valid &&
                       launch.profile->fmtowns_cdda_music.source_offset == 0x3dacu &&
                       launch.profile->fmtowns_cdda_music.source_size == 29u &&
                       skull[0x3dacu] == 0u && skull[0x3db0u] == 1u &&
                       dm2_v1_fmtowns_hmp_to_cdda(
                           &launch.profile->fmtowns_cdda_music, 4) == 1 &&
                       dm2_v1_fmtowns_cdda_map_table(
                           &launch.profile->fmtowns_cdda_music) != NULL,
                   "FM Towns SKULL is a bounded native P3 program and supplies its HMP-to-CDDA map from RAM");
            expect(twanim && twanim_size == 72184u &&
                       launch.profile->fmtowns_twanim_p3.valid &&
                       launch.profile->fmtowns_twanim_p3.level == 1u &&
                       launch.profile->fmtowns_twanim_p3.header_size == 0x180u &&
                       launch.profile->fmtowns_twanim_p3.file_size == twanim_size &&
                       launch.profile->fmtowns_twanim_p3.runtime_offset == 0x180u &&
                       launch.profile->fmtowns_twanim_p3.runtime_size == 0x80u &&
                       launch.profile->fmtowns_twanim_p3.relocation_offset == 0x200u &&
                       launch.profile->fmtowns_twanim_p3.relocation_size == 0u &&
                       launch.profile->fmtowns_twanim_p3.load_image_offset == 0x200u &&
                       launch.profile->fmtowns_twanim_p3.load_image_size == 0x117f8u &&
                       launch.profile->fmtowns_twanim_p3.symbol_table_offset == 0u &&
                       launch.profile->fmtowns_twanim_p3.symbol_table_size == 0u &&
                       launch.profile->fmtowns_twanim_p3.initial_eip == 0x10470u &&
                       launch.profile->fmtowns_twanim_p3.memory_requirements ==
                           0x117f8u,
                   "FM Towns TWANIM is the authenticated native P3 player for the retained title streams");
            free(title);
            free(swoosh);
            free(end);
            free(twanim);
            free(skull);
        }
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
            M12_AssetStatus english_archive_status;
            const M12_AssetVersionStatus* english_archive_version;
            char virtual_companion[1024];

            /* This is the M12 handoff the user actually gets after choosing
             * English: scan the selected DOS archive and preserve the
             * scanner-provided virtual member provenance. */
            memset(&english_archive_status, 0, sizeof(english_archive_status));
            M12_AssetStatus_ScanGame(&english_archive_status,
                                     english_companion_archive, "dm2");
            versionIndex = M12_AssetStatus_FindVersionIndex("dm2", "pc-en");
            english_archive_version = versionIndex >= 0
                ? M12_AssetStatus_GetVersion(&english_archive_status, "dm2",
                                              (size_t)versionIndex)
                : NULL;
            expect(english_archive_version && english_archive_version->matched &&
                       strstr(english_archive_version->matchedPath,
                              "::data/graphics.dat") != NULL,
                   "M12 retains the DOS archive companion member provenance");
            memset(&launch, 0, sizeof(launch));
            expect(english_archive_version &&
                       dm2_v1_boot_startup_launch_alloc_with_language(
                           selectedRuntime,
                           english_archive_version->matchedPath, 0,
                           &launch) == 1,
                   "FM Towns English accepts M12's original ZIP provenance");
            expect_complete_english_text_overlay(launch.profile);
            dm2_v1_boot_startup_launch_cleanup(&launch);

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
