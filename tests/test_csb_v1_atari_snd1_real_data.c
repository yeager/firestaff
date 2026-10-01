/* Compare authentic Atari ST SND1 rows in both preserved CSB carriers. */

#include "csb_v1_audio_runtime_pc34_compat.h"
#include "audio_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Snd1Fingerprint {
    size_t byte_count;
    unsigned int hash;
    int decode_status;
    unsigned int sample_count;
} Snd1Fingerprint;

typedef struct ExpectedRejectedRow {
    int row;
    size_t byte_count;
    unsigned int hash;
    unsigned int sample_count;
} ExpectedRejectedRow;

static unsigned int hash_bytes(const unsigned char *bytes, size_t count)
{
    unsigned int hash = 2166136261u;
    for (size_t i = 0; i < count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

static int inspect_carrier(const char *archive, const char *member,
                           Snd1Fingerprint fingerprints[
                               CSB_V1_ATARI_ST_SOUND_COUNT])
{
    char graphics_path[4096];
    unsigned char *levels = NULL;
    CsbV1AtariStSoundPayload payload = {0};
    CsbV1StSoundDecodeResult decoded = {0};
    M11_AudioState audio = {0};
    int result;

    if (snprintf(graphics_path, sizeof(graphics_path), "%s::%s", archive,
                 member) >= (int)sizeof(graphics_path)) {
        fputs("FAIL: CSB source path too long\n", stderr);
        return 0;
    }
    if (!M11_Audio_Init(&audio)) {
        fprintf(stderr, "FAIL: audio transport did not initialize for %s\n",
                member);
        return 0;
    }
    for (int index = 0; index < CSB_V1_ATARI_ST_SOUND_COUNT; ++index) {
        const CsbV1AtariStSoundSpec *spec =
            csb_v1_audio_runtime_atari_st_sound_spec((int16_t)index);
        if (!spec || !csb_v1_audio_runtime_load_atari_st_sound_payload(
                         graphics_path, (int16_t)index, &payload)) {
            fprintf(stderr, "FAIL: authentic SND1 row %d unavailable in %s\n",
                    index, member);
            return 0;
        }
        if (payload.byteCount < 2u) {
            fprintf(stderr, "FAIL: authentic SND1 row %d is truncated\n", index);
            csb_v1_audio_runtime_atari_st_sound_payload_free(&payload);
            return 0;
        }
        levels = (unsigned char *)malloc(65536u);
        if (!levels) {
            csb_v1_audio_runtime_atari_st_sound_payload_free(&payload);
            return 0;
        }
        result = csb_v1_audio_runtime_decode_st_sound(
            payload.bytes, payload.byteCount, 0u, levels, 65536u, &decoded);
        fingerprints[index].byte_count = payload.byteCount;
        fingerprints[index].hash = hash_bytes(payload.bytes, payload.byteCount);
        fingerprints[index].decode_status = result;
        fingerprints[index].sample_count =
            (unsigned)(((unsigned)payload.bytes[0] << 8u) | payload.bytes[1]);
        printf("%s row=%d graphic=%u bytes=%zu hash=%08x samples=%u decode=%d\n",
               member, index, spec->graphicIndex,
               fingerprints[index].byte_count, fingerprints[index].hash,
               fingerprints[index].sample_count, result);
        if (result != 0 && result != -2) {
            fprintf(stderr,
                    "FAIL: authentic SND1 row %d returned unexpected status %d\n",
                    index, result);
            free(levels);
            csb_v1_audio_runtime_atari_st_sound_payload_free(&payload);
            return 0;
        }
        if (result == -2) {
            static const int held_rows[] = {1, 12, 16};
            int expected_hold = 0;
            for (size_t held = 0u;
                 held < sizeof(held_rows) / sizeof(held_rows[0]); ++held) {
                if (index == held_rows[held]) expected_hold = 1;
            }
            if (!expected_hold ||
                csb_v1_audio_runtime_decode_st_sound_with_final_hold(
                    payload.bytes, payload.byteCount, 0u, levels, 65536u,
                    &decoded) != 0 ||
                decoded.sampleCount != fingerprints[index].sample_count) {
                fprintf(stderr,
                        "FAIL: authentic SND1 row %d does not match its bounded Timer-A final hold\n",
                        index);
                free(levels);
                csb_v1_audio_runtime_atari_st_sound_payload_free(&payload);
                return 0;
            }
        }
        if (!M11_Audio_PlayCsbAtariStPsgAtSourceVolume(
                &audio, payload.bytes, (int)payload.byteCount,
                spec->period, fingerprints[index].hash, 1) ||
            !audio.csbAtariStSoundAccepted ||
            audio.csbAtariStSoundHash != fingerprints[index].hash) {
            fprintf(stderr,
                    "FAIL: authentic Atari SND1 row %d was not accepted by the audio transport\n",
                    index);
            M11_Audio_Shutdown(&audio);
            free(levels);
            csb_v1_audio_runtime_atari_st_sound_payload_free(&payload);
            return 0;
        }
        free(levels);
        levels = NULL;
        csb_v1_audio_runtime_atari_st_sound_payload_free(&payload);
    }
    M11_Audio_Shutdown(&audio);
    return 1;
}

int main(void)
{
    static const ExpectedRejectedRow rejected_rows[] = {
        { 1, 39u, 0x934d67d8u, 100u },
        { 12, 408u, 0x46b80977u, 1019u },
        { 16, 452u, 0xec9430c2u, 962u }
    };
    static const int expected_decode_status[CSB_V1_ATARI_ST_SOUND_COUNT] = {
        0, -2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        -2, 0, 0, 0, -2, 0, 0, 0, 0, 0
    };
    Snd1Fingerprint hard_disk[CSB_V1_ATARI_ST_SOUND_COUNT] = {{0}};
    Snd1Fingerprint floppy[CSB_V1_ATARI_ST_SOUND_COUNT] = {{0}};
    const char *archive = getenv("FIRESTAFF_CSB_ATARI_MEDIA");
    if (!archive || !archive[0]) {
        puts("SKIP: original CSB Atari ST preservation archive is not staged");
        return 77;
    }
    if (!inspect_carrier(
            archive,
            "HardDisk/2009-02-22 PP/GRAPHICS.DAT", hard_disk) ||
        !inspect_carrier(
            archive,
            "Floppy Disks STX/Chaos Strikes Back for Atari ST Game Disk v2.1 (English).stx::GRAPHICS.DAT",
            floppy)) {
        return 1;
    }
    for (int index = 0; index < CSB_V1_ATARI_ST_SOUND_COUNT; ++index) {
        if (hard_disk[index].byte_count != floppy[index].byte_count ||
            hard_disk[index].hash != floppy[index].hash ||
            hard_disk[index].sample_count != floppy[index].sample_count ||
            hard_disk[index].decode_status != floppy[index].decode_status) {
            fprintf(stderr,
                    "FAIL: hard-disk and v2.1 floppy SND1 row %d differs\n",
                    index);
            return 1;
        }
        if (hard_disk[index].decode_status != expected_decode_status[index]) {
            fprintf(stderr,
                    "FAIL: SND1 row %d decode status %d, expected %d\n",
                    index, hard_disk[index].decode_status,
                    expected_decode_status[index]);
            return 1;
        }
    }
    for (size_t i = 0u; i < sizeof(rejected_rows) / sizeof(rejected_rows[0]);
         ++i) {
        const ExpectedRejectedRow *expected = &rejected_rows[i];
        const Snd1Fingerprint *actual = &hard_disk[expected->row];
        if (actual->byte_count != expected->byte_count ||
            actual->hash != expected->hash ||
            actual->sample_count != expected->sample_count ||
            actual->decode_status != -2) {
            fprintf(stderr,
                    "FAIL: authentic rejected SND1 row %d fingerprint changed\n",
                    expected->row);
            return 1;
        }
    }
    puts("PASS: all CSB Atari SND1 records were read from both authentic carriers");
    return 0;
}
