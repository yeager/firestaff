/*
 * CSB V1 audio runtime boundary.
 *
 * This module mirrors the ReDMCSB PC/Atari pending-sound runtime contract only.
 * It deliberately does not decode or mix real audio payloads.
 */

#include "csb_v1_audio_runtime_pc34_compat.h"

#include "memory_graphics_dat_header_pc34_compat.h"
#include "memory_graphics_dat_select_pc34_compat.h"

#include <stdlib.h>
#include <string.h>

/* ReDMCSB DATA.C:1260-1302, MEDIA719_I34E_I34M. The PC table is compiled
 * into the original executable and points at original GRAPHICS.DAT entries.
 * Keep this separate from CSBWin's 22-entry sound1772 table. */
static const CsbV1Pc34SoundSpec csb_v1_pc34_sound_specs[CSB_V1_SOUND_COUNT] = {
    {671u, 112u,  11u, 3u, 6u}, {672u, 112u,  15u, 0u, 3u},
    {673u, 112u,  72u, 3u, 6u}, {673u, 145u,  72u, 3u, 6u},
    {674u, 112u,  10u, 3u, 6u}, {675u, 112u,  99u, 3u, 7u},
    {675u, 112u,  98u, 0u, 4u}, {677u, 112u, 110u, 3u, 6u},
    {678u, 112u,   2u, 3u, 6u}, {679u, 112u,  80u, 3u, 6u},
    {680u, 112u,  82u, 3u, 6u}, {681u, 112u,  84u, 3u, 6u},
    {682u, 112u,  86u, 3u, 6u}, {684u, 112u,  40u, 2u, 4u},
    {685u, 112u,  70u, 1u, 4u}, {687u, 138u,  75u, 3u, 6u},
    {683u, 112u,  95u, 3u, 6u}, {707u, 138u, 106u, 0u, 4u},
    {704u, 138u, 105u, 0u, 4u}, {690u, 112u,  57u, 3u, 5u},
    {691u, 112u,  52u, 3u, 5u}, {692u, 112u,  50u, 3u, 5u},
    {693u, 112u,  96u, 2u, 4u}, {688u, 112u,  60u, 3u, 5u},
    {708u, 138u,  56u, 0u, 4u}, {689u, 112u,  55u, 3u, 5u},
    {709u, 112u,  58u, 0u, 4u}, {710u, 112u,  53u, 0u, 4u},
    {701u, 138u,  24u, 0u, 4u}, {702u, 138u,  21u, 0u, 4u},
    {703u, 138u,  23u, 0u, 4u}, {705u, 138u,  27u, 0u, 4u},
    {706u, 138u,  28u, 0u, 4u}, {711u, 138u,  29u, 0u, 4u},
    {712u, 150u,  22u, 0u, 4u}
};

static uint16_t csb_v1_audio_read_be16(const uint8_t* bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static int csb_v1_audio_read_nibble(const uint8_t* encoded,
                                    size_t encodedSize,
                                    size_t* nibbleIndex,
                                    uint8_t* outNibble)
{
    size_t byteIndex;

    byteIndex = *nibbleIndex / 2u;
    if (byteIndex >= encodedSize) {
        return 0;
    }
    *outNibble = (uint8_t)((*nibbleIndex & 1u) ?
                               (encoded[byteIndex] & 0x0fu) :
                               (encoded[byteIndex] >> 4));
    ++*nibbleIndex;
    return 1;
}

static int csb_v1_audio_valid_index(int16_t soundIndex)
{
    return soundIndex >= 0 && soundIndex < CSB_V1_SOUND_COUNT;
}

static int csb_v1_audio_valid_mode(int16_t mode)
{
    return mode >= CSB_V1_MODE_DO_NOT_PLAY &&
           mode <= CSB_V1_MODE_PLAY_ONE_TICK_LATER;
}

const CsbV1Pc34SoundSpec*
csb_v1_audio_runtime_pc34_sound_spec(int16_t soundIndex)
{
    if (!csb_v1_audio_valid_index(soundIndex)) {
        return NULL;
    }
    return &csb_v1_pc34_sound_specs[soundIndex];
}

void csb_v1_audio_runtime_pc34_sound_payload_free(
    CsbV1Pc34SoundPayload *payload)
{
    if (!payload) {
        return;
    }
    free(payload->bytes);
    memset(payload, 0, sizeof(*payload));
}

int csb_v1_audio_runtime_load_pc34_sound_payload(
    const char *graphicsDatPath,
    int16_t soundIndex,
    CsbV1Pc34SoundPayload *outPayload)
{
    const CsbV1Pc34SoundSpec *spec;
    struct MemoryGraphicsDatState_Compat state;
    struct MemoryGraphicsDatHeader_Compat header;
    struct MemoryGraphicsDatSelection_Compat selection;
    uint8_t *record = NULL;
    uint16_t payloadBytes;
    int result = 0;

    if (!graphicsDatPath || !outPayload) {
        return 0;
    }
    memset(outPayload, 0, sizeof(*outPayload));
    memset(&state, 0, sizeof(state));
    memset(&header, 0, sizeof(header));
    spec = csb_v1_audio_runtime_pc34_sound_spec(soundIndex);
    if (!spec || !F0479_MEMORY_LoadGraphicsDatHeader_Compat(
                     graphicsDatPath, &state, &header) ||
        !F0490_MEMORY_SelectGraphicFromHeader_Compat(
            &header, spec->graphicIndex, &selection) ||
        selection.compressedByteCount != selection.decompressedByteCount ||
        selection.compressedByteCount < 4u) {
        goto cleanup;
    }

    record = (uint8_t *)malloc(selection.compressedByteCount);
    if (!record || !F0477_MEMORY_OpenGraphicsDat_CPSDF_Compat(
                       graphicsDatPath, &state) ||
        !F0474_MEMORY_LoadGraphic_CPSDF_Compat(
            selection.offset, selection.compressedByteCount, &state, record)) {
        goto cleanup;
    }
    (void)F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(&state);

    payloadBytes = csb_v1_audio_read_be16(record);
    /* ReDMCSB IO.C F0060 reads the count then supplies record + 2. The PC3.4
     * entries carry exactly two unused tail bytes after that source payload. */
    if ((size_t)payloadBytes + 4u != selection.compressedByteCount) {
        goto cleanup;
    }
    outPayload->bytes = (uint8_t *)malloc(payloadBytes);
    if (!outPayload->bytes) {
        goto cleanup;
    }
    memcpy(outPayload->bytes, record + 2u, payloadBytes);
    outPayload->byteCount = payloadBytes;
    outPayload->spec = *spec;
    result = 1;

cleanup:
    if (state.referenceCount > 0) {
        (void)F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(&state);
    }
    F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
    free(record);
    if (!result) {
        csb_v1_audio_runtime_pc34_sound_payload_free(outPayload);
    }
    return result;
}

CsbV1PsgChannelAmplitudes
csb_v1_audio_runtime_channel_amplitudes(int16_t amplitudeIndex)
{
    static const uint8_t channel_a_loud[16] = {
        0, 8, 10, 11, 12, 13, 13, 13,
        14, 14, 14, 14, 15, 14, 14, 14
    };
    static const uint8_t channel_b_loud[16] = {
        0, 5, 7, 9, 9, 5, 10, 12,
        8, 10, 12, 13, 11, 14, 14, 14
    };
    static const uint8_t channel_c_loud[16] = {
        0, 0, 0, 0, 0, 0, 6, 0,
        0, 10, 10, 10, 0, 11, 13, 14
    };
    CsbV1PsgChannelAmplitudes amplitudes;
    uint16_t index = (uint16_t)amplitudeIndex & 0x000fu;

    /* SOUND.C F0061 uses V0061004's three 16-entry loud tables. */
    amplitudes.channelA = channel_a_loud[index];
    amplitudes.channelB = channel_b_loud[index];
    amplitudes.channelC = channel_c_loud[index];
    return amplitudes;
}

static void csb_v1_audio_clear_pending(CsbV1AudioRuntime* runtime)
{
    runtime->pendingSoundIndex = CSB_V1_SOUND_NONE;
    runtime->pendingVolume = 0;
    runtime->pendingPriority = 0;
}

void csb_v1_audio_runtime_init(CsbV1AudioRuntime* runtime)
{
    if (!runtime) {
        return;
    }
    memset(runtime, 0, sizeof(*runtime));
    csb_v1_audio_clear_pending(runtime);
    runtime->lastPlayedSoundIndex = CSB_V1_SOUND_NONE;
    runtime->lastCreatureAttackTime = -200;
}

int csb_v1_audio_runtime_request(CsbV1AudioRuntime* runtime,
                                 const CsbV1AudioRequest* request)
{
    if (!runtime || !request || !csb_v1_audio_valid_index(request->soundIndex) ||
        request->volume <= 0 || !csb_v1_audio_valid_mode(request->mode)) {
        if (runtime) {
            runtime->totalRejectedRequests++;
        }
        return 0;
    }
    if (request->mode == CSB_V1_MODE_DO_NOT_PLAY) {
        runtime->totalRejectedRequests++;
        return 0;
    }

    runtime->totalRequests++;
    if (request->mode == CSB_V1_MODE_PLAY_IMMEDIATELY) {
        runtime->lastPlayedSoundIndex = request->soundIndex;
        runtime->totalImmediatePlays++;
        csb_v1_audio_clear_pending(runtime);
        return 1;
    }

    /*
     * ReDMCSB SOUND.C F0064 lines 1632-1638:
     * nonzero play modes update G0583_i_PendingSoundIndex only if the new
     * request is louder, or equally loud with a higher SOUND_DATA priority.
     */
    if (runtime->pendingSoundIndex == CSB_V1_SOUND_NONE ||
        request->volume > runtime->pendingVolume ||
        (request->volume == runtime->pendingVolume &&
         request->priority > runtime->pendingPriority)) {
        runtime->pendingSoundIndex = request->soundIndex;
        runtime->pendingVolume = request->volume;
        runtime->pendingPriority = request->priority;
        return 1;
    }
    return 0;
}

int csb_v1_audio_runtime_flush_pending(CsbV1AudioRuntime* runtime)
{
    if (!runtime || runtime->pendingSoundIndex == CSB_V1_SOUND_NONE) {
        return 0;
    }

    /*
     * ReDMCSB SOUND.C F0065 lines 1804-1865 and GAMELOOP.C lines 114-115:
     * one pending sound is played during the game-loop tick and the pending
     * index/volume are reset before damage and time advance.
     */
    runtime->lastPlayedSoundIndex = runtime->pendingSoundIndex;
    runtime->totalPendingFlushes++;
    csb_v1_audio_clear_pending(runtime);
    return 1;
}

void csb_v1_audio_runtime_record_creature_attack(CsbV1AudioRuntime* runtime,
                                                 int32_t gameTime)
{
    if (!runtime) {
        return;
    }
    runtime->lastCreatureAttackTime = gameTime;
}

void csb_v1_audio_runtime_save_snapshot(const CsbV1AudioRuntime* runtime,
                                        CsbV1AudioSaveSnapshot* outSnapshot)
{
    if (!runtime || !outSnapshot) {
        return;
    }
    outSnapshot->lastCreatureAttackTime = runtime->lastCreatureAttackTime;
}

void csb_v1_audio_runtime_load_snapshot(CsbV1AudioRuntime* runtime,
                                        const CsbV1AudioSaveSnapshot* snapshot)
{
    if (!runtime || !snapshot) {
        return;
    }
    runtime->lastCreatureAttackTime = snapshot->lastCreatureAttackTime;
    csb_v1_audio_clear_pending(runtime);
}

int csb_v1_audio_runtime_decode_st_sound(const uint8_t* encoded,
                                         size_t encodedSize,
                                         uint8_t initialLevel,
                                         uint8_t* outLevels,
                                         size_t outLevelCapacity,
                                         CsbV1StSoundDecodeResult* outResult)
{
    size_t declaredSamples;
    size_t emitted = 0;
    size_t nibbleIndex = 0;
    size_t repeatsRemaining = 0;
    uint8_t currentLevel;

    if (!encoded || !outLevels || !outResult || encodedSize < 2u ||
        initialLevel > 15u) {
        return -1;
    }

    declaredSamples = csb_v1_audio_read_be16(encoded);
    if (outLevelCapacity < declaredSamples) {
        return -3;
    }

    currentLevel = initialLevel;
    while (emitted < declaredSamples) {
        uint8_t nibble;

        if (repeatsRemaining != 0u) {
            --repeatsRemaining;
        } else {
            size_t repeatValue = 0;

            if (!csb_v1_audio_read_nibble(encoded + 2u, encodedSize - 2u,
                                          &nibbleIndex, &nibble)) {
                return -2;
            }
            if (nibble != 0u) {
                currentLevel = nibble;
            } else {
                do {
                    if (!csb_v1_audio_read_nibble(encoded + 2u,
                                                  encodedSize - 2u,
                                                  &nibbleIndex, &nibble)) {
                        return -2;
                    }
                    if (repeatValue > (SIZE_MAX >> 3u)) {
                        return -2;
                    }
                    repeatValue = (repeatValue << 3u) | (nibble & 0x07u);
                } while ((nibble & 0x08u) != 0u);

                if (repeatValue > SIZE_MAX - 2u) {
                    return -2;
                }
                repeatsRemaining = repeatValue + 2u;
            }
        }

        outLevels[emitted++] = currentLevel;
    }

    outResult->sampleCount = emitted;
    outResult->encodedBytesConsumed = 2u + ((nibbleIndex + 1u) / 2u);
    return 0;
}

const char* csb_v1_audio_runtime_source_evidence(void)
{
    return "DEFS.H:135-138; SOUND.C F0060:887-931,1164-1246; "
           "SOUND.C F0061:1144-1307; "
           "SOUND.C:1632-1638; SOUND.C:1804-1865; "
           "GAMELOOP.C:114-115; LOADSAVE.C:1530/2739; PROJEXPL.C:5";
}
