#include "m11_game_view.h"
#include "csb_v1_audio_runtime_pc34_compat.h"
#include "csb_v1_boot.h"
#include "csb_v1_fmtowns_graphics_dat.h"
#include "csb_v1_inscription_presentation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *archive = getenv("FIRESTAFF_CSB_FMTOWNS_ARCHIVE");
    const char *language = getenv("FIRESTAFF_CSB_FMTOWNS_GAME_LANGUAGE");
    const int japanese = language && strcmp(language, "ja") == 0;
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    CsbV1FmtownsSoundPayload sound;
    const CSB_V1_BootProfile *profile;

    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_CSB_FMTOWNS_ARCHIVE not set");
        return 77;
    }
    memset(&spec, 0, sizeof(spec));
    spec.title = "CHAOS STRIKES BACK";
    spec.gameId = "csb";
    spec.sourceId = "csb";
    spec.dataDir = archive;
    /* The launcher passes M12's authenticated edition identity with the
     * selected archive.  English F31 has no separate boolean flag, so omit
     * neither this identity nor the Japanese switch: otherwise M11 must
     * correctly reject an arbitrary direct CSB ZIP instead of guessing its
     * platform. */
    spec.verifiedAssetMd5 = japanese
        ? "761d6fc588b31aeaaa9caf3725e111b9"
        : "405b757038eea3c263e60f240854d6de";
    spec.csbFmtownsJapanese = japanese;
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec.presentationWidth = 320;
    spec.presentationHeight = 200;
    M11_GameView_Init(&view);
    if (!M11_GameView_Start(&view, &spec)) {
        fprintf(stderr, "FAIL: packed CSB FM Towns M11 start\n");
        M11_GameView_Shutdown(&view);
        return 1;
    }
    if (!view.csbBootProfile || !view.active) {
        fprintf(stderr, "FAIL: packed CSB FM Towns M11 runtime ownership\n");
        M11_GameView_Shutdown(&view);
        return 1;
    }
    profile = (const CSB_V1_BootProfile*)view.csbBootProfile;
    memset(&sound, 0, sizeof(sound));
    if (!csb_v1_audio_runtime_load_fmtowns_sound_payload_bytes(
            profile->fmtowns_graphics_bytes, profile->fmtowns_graphics_size,
            0, &sound) ||
        sound.byteCount == 0u || sound.spec.graphicIndex != 671u) {
        fprintf(stderr, "FAIL: packed CSB FM Towns authentic runtime sound: %s\n",
                profile->graphics_path);
        csb_v1_audio_runtime_fmtowns_sound_payload_free(&sound);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    {
        unsigned int hash = 2166136261u;
        size_t index;
        for (index = 0u; index < sound.byteCount; ++index) {
            hash ^= sound.bytes[index];
            hash *= 16777619u;
        }
        if (!M11_Audio_PlayCsbFmtownsRuntimePcm(
                &view.audioState, (const int8_t*)sound.bytes,
                (int)sound.byteCount, 127, hash) ||
            !view.audioState.csbFmtownsRuntimeSoundAccepted ||
            view.audioState.csbFmtownsRuntimeSoundByteCount !=
                (int)sound.byteCount ||
            view.audioState.csbFmtownsRuntimeSoundMixerVolume != view.audioState.sfxVolume) {
            fprintf(stderr, "FAIL: packed CSB FM Towns 5500 Hz PCM transport\n");
            csb_v1_audio_runtime_fmtowns_sound_payload_free(&sound);
            M11_GameView_Shutdown(&view);
            return 1;
        }
        for (int volume = 1; volume <= 127; ++volume) {
            if (!M11_Audio_PlayCsbFmtownsRuntimePcm(&view.audioState,
                    (const int8_t*)sound.bytes, (int)sound.byteCount, volume, hash) ||
                view.audioState.csbFmtownsRuntimeSoundMixerVolume !=
                    (view.audioState.sfxVolume * volume + 63) / 127) {
                fputs("FAIL: Towns native 127-step driver volume\n", stderr);
                csb_v1_audio_runtime_fmtowns_sound_payload_free(&sound);
                M11_GameView_Shutdown(&view);
                return 1;
            }
        }
    }
    csb_v1_audio_runtime_fmtowns_sound_payload_free(&sound);
    for (int event = 0; event < CSB_V1_SOUND_COUNT; ++event) {
        unsigned int hash = 2166136261u;
        if (!csb_v1_audio_runtime_load_fmtowns_sound_payload_bytes(
                profile->fmtowns_graphics_bytes, profile->fmtowns_graphics_size,
                (int16_t)event, &sound)) {
            fprintf(stderr, "FAIL: original F31 sound event %d\n", event);
            M11_GameView_Shutdown(&view);
            return 1;
        }
        {
            const unsigned char* original = profile->fmtowns_graphics_bytes;
            size_t offset = 4u + 728u * 8u;
            size_t count;
            unsigned char* damaged;
            CsbV1FmtownsSoundPayload rejected = {0};
            for (unsigned int record = 0; record < sound.spec.graphicIndex; ++record)
                offset += original[4u + record * 2u] |
                          ((unsigned int)original[5u + record * 2u] << 8);
            if (offset + 2u > profile->fmtowns_graphics_size) {
                csb_v1_audio_runtime_fmtowns_sound_payload_free(&sound);
                M11_GameView_Shutdown(&view);
                return 1;
            }
            count = ((size_t)original[offset] << 8) | original[offset + 1u];
            if (count != sound.byteCount || count > profile->fmtowns_graphics_size - offset - 2u ||
                memcmp(sound.bytes, original + offset + 2u, count)) {
                fputs("FAIL: F31 PCM does not match original container bytes\n", stderr);
                csb_v1_audio_runtime_fmtowns_sound_payload_free(&sound);
                M11_GameView_Shutdown(&view);
                return 1;
            }
            /* Damage only a private RAM copy of authenticated media. The
             * loader must reject a count that runs beyond the sound record. */
            damaged = malloc(profile->fmtowns_graphics_size);
            if (!damaged) {
                csb_v1_audio_runtime_fmtowns_sound_payload_free(&sound);
                M11_GameView_Shutdown(&view);
                return 1;
            }
            memcpy(damaged, original, profile->fmtowns_graphics_size);
            damaged[offset] = damaged[offset + 1u] = 0xffu;
            int accepted = csb_v1_audio_runtime_load_fmtowns_sound_payload_bytes(
                damaged, profile->fmtowns_graphics_size, (int16_t)event, &rejected);
            free(damaged);
            if (accepted || rejected.bytes || rejected.byteCount) {
                fputs("FAIL: out-of-record F31 sample count accepted\n", stderr);
                csb_v1_audio_runtime_fmtowns_sound_payload_free(&rejected);
                csb_v1_audio_runtime_fmtowns_sound_payload_free(&sound);
                M11_GameView_Shutdown(&view);
                return 1;
            }
        }
        for (size_t b = 0; b < sound.byteCount; ++b) {
            hash ^= sound.bytes[b];
            hash *= 16777619u;
        }
        if (!M11_Audio_PlayCsbFmtownsRuntimePcm(&view.audioState,
                (const int8_t*)sound.bytes, (int)sound.byteCount, 127, hash) ||
            view.audioState.csbFmtownsRuntimePcm.sampleCount !=
                (int)((sound.byteCount * M11_AUDIO_SAMPLE_RATE + 5499u) / 5500u)) {
            csb_v1_audio_runtime_fmtowns_sound_payload_free(&sound);
            M11_GameView_Shutdown(&view);
            return 1;
        }
        for (int p = 0; p < view.audioState.csbFmtownsRuntimePcm.sampleCount; ++p) {
            size_t source = (size_t)p * 5500u / M11_AUDIO_SAMPLE_RATE;
            int value;
            if (source >= sound.byteCount) source = sound.byteCount - 1u;
            value = sound.bytes[source];
            if (value >= 128) value -= 256;
            if (view.audioState.csbFmtownsRuntimePcm.samples[p] != value / 128.0f) {
                csb_v1_audio_runtime_fmtowns_sound_payload_free(&sound);
                M11_GameView_Shutdown(&view);
                return 1;
            }
        }
        csb_v1_audio_runtime_fmtowns_sound_payload_free(&sound);
    }
    puts("PASS: all 35 F31 sound events match original signed PCM at 5500 Hz");
    puts("PASS: packed CSB FM Towns runtime sound uses original GRAPHICS.DAT PCM");
    puts("PASS: packed CSB FM Towns M11 starts from original ZIP");
    /* Exercise the original GAME/Enter path before inspecting inventory.
     * MINI.DAT supplies the champion and dungeon; do not fabricate a party. */
    for (int tick = 0; tick < 700; ++tick)
        (void)M11_GameView_AdvanceIdleTick(&view);
    (void)M11_GameView_HandlePointer(&view, 52, 110, 1);
    for (int tick = 0; tick < 10; ++tick)
        (void)M11_GameView_AdvanceIdleTick(&view);
    (void)M11_GameView_HandlePointer(&view, 250, 50, 1);
    for (int tick = 0; tick < 240; ++tick)
        (void)M11_GameView_AdvanceIdleTick(&view);
    if (profile->runtime.champion_count != 1 ||
        !profile->runtime.dungeon_handle) {
        fputs("FAIL: original F31 GAME/Enter inventory world not loaded\n", stderr);
        M11_GameView_Shutdown(&view);
        return 1;
    }
    puts("PASS: original F31 MINI.DAT party and dungeon loaded for inventory");
    {
        int residents = 0;
        int containers = profile->runtime.dungeon_handle->thing_type_counts[9];
        for (int index = 0; index < containers; ++index) {
            uint16_t slots[8];
            int count = csb_v1_runtime_read_container_slots(&profile->runtime,
                (uint16_t)((9 << 10) | index), slots);
            if (count < 0 || count > 8) {
                fputs("FAIL: original F31 container read rejected\n", stderr);
                M11_GameView_Shutdown(&view);
                return 1;
            }
            residents += count;
        }
        printf("original F31 inventory: containers=%d residents=%d\n", containers, residents);
        for (int chestIndex = 0; chestIndex < containers; ++chestIndex) {
            uint16_t chest = (uint16_t)((9 << 10) | chestIndex);
            uint16_t slots[8];
            int x, y, w, h;
            uint8_t layout[10000];
            CSB_V1_FmtownsItemDecodeReceipt layoutReceipt;
            CSB_V1_F31InventorySlotRectangle rectangles[8];
            if (!csb_v1_fmtowns_graphics_copy_raw_item(profile->fmtowns_graphics_bytes,
                    profile->fmtowns_graphics_size, 696u, layout, sizeof(layout), &layoutReceipt) ||
                !csb_v1_media720_f0635_f31_chest_rectangles(layout,
                    layoutReceipt.stream_byte_count, 0, rectangles)) {
                fputs("FAIL: original F31 chest rectangle graph\n", stderr);
                M11_GameView_Shutdown(&view);
                return 1;
            }
            x = rectangles[0].x; y = rectangles[0].y;
            w = rectangles[0].width; h = rectangles[0].height;
            fprintf(stderr, "original F31 C537: %d,%d %dx%d\n", x, y, w, h);
            int count = csb_v1_runtime_read_container_slots(&profile->runtime, chest, slots);
            if (count == 0) continue;
            view.inventoryPanelActive = 1;
            view.world.party.activeChampionIndex = 0;
            if (count <= 0 ||
                !csb_v1_runtime_write_inventory_slot_from_boot_profile_pc34(
                    view.csbBootProfile, 0, 1, chest) ||
                !csb_v1_runtime_write_leader_hand_from_boot_profile_pc34(
                    view.csbBootProfile, THING_NONE) ||
                !DM1_V1_M11Runtime_OpenActionHandChestPc34Compat(&view)) {
                fputs("FAIL: original F31 chest input setup\n", stderr);
                M11_GameView_Shutdown(&view);
                return 1;
            }
            for (int slot = 0; slot < count; ++slot) {
            for (int mode = 0; mode < 2; ++mode) {
            view.presentationMode = mode ? M12_PRESENTATION_V21_UPSCALED : M12_PRESENTATION_V1_ORIGINAL;
            x = rectangles[slot].x; y = rectangles[slot].y;
            w = rectangles[slot].width; h = rectangles[slot].height;
            (void)M11_GameView_HandlePointer(&view, x + w / 2, 33 + y + h / 2, 1);
            (void)M11_GameView_HandlePointerButtonRelease(&view,
                x + w / 2, 33 + y + h / 2, DM1_V1_MOUSE_MASK_LEFT_PC34);
            if (DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&view) != slots[slot]) {
                fprintf(stderr, "FAIL: F31 chest pickup got=%04x expected=%04x\n",
                    DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&view), slots[slot]);
                M11_GameView_Shutdown(&view);
                return 1;
            }
            /* CHEST.C F0333/F0334 retains the hole until close. Returning
             * the object must empty the hand, not swap with its neighbour. */
            (void)M11_GameView_HandlePointer(&view, x + w / 2, 33 + y + h / 2, 1);
            (void)M11_GameView_HandlePointerButtonRelease(&view,
                x + w / 2, 33 + y + h / 2, DM1_V1_MOUSE_MASK_LEFT_PC34);
            int canReturn = (csb_v1_runtime_object_allowed_slots_from_boot_profile_pc34(
                view.csbBootProfile, slots[slot]) & 0x0400u) != 0;
            if (DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&view) !=
                    (canReturn ? THING_NONE : slots[slot])) {
                fprintf(stderr, "FAIL: F31 replacement chest=%d slot=%d hand=%04x\n",
                    chestIndex, slot, DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&view));
                fprintf(stderr, "source allowed slots=%04x original=%04x\n",
                    csb_v1_runtime_object_allowed_slots_from_boot_profile_pc34(view.csbBootProfile, slots[slot]), slots[slot]);
                M11_GameView_Shutdown(&view);
                return 1;
            }
            if (!canReturn) {
                /* Original dungeons can pre-place oversized equipment in
                 * chests. F0302 still rejects inserting it (G0038=0x0400).
                 * Preserve that rejection, then reset this test placement. */
                DM1_V1_M11Runtime_CloseOpenChestPc34Compat(&view);
                DM1_V1_M11Runtime_ClearLeaderHandObjectPc34Compat(&view);
                if (!csb_v1_runtime_write_container_slots_from_boot_profile_pc34(
                        view.csbBootProfile, chest, slots) ||
                    !DM1_V1_M11Runtime_OpenActionHandChestPc34Compat(&view)) return 1;
            }
            }
            }
            DM1_V1_M11Runtime_CloseOpenChestPc34Compat(&view);
            {
                uint16_t after[8];
                if (csb_v1_runtime_read_container_slots(&profile->runtime, chest, after) != count ||
                    memcmp(after, slots, sizeof(after)) != 0) return 1;
            }
        }
    }
    M11_GameView_Shutdown(&view);
    return 0;
}
