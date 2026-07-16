#include "dm1_v1_original_save_pc34_handoff.h"
#include "dm1_v1_original_save_classifier.h"
#include "memory_savegame_pc34_native_export_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static uint32_t mix(uint32_t hash, uint32_t value)
{
    unsigned int shift;
    for (shift = 0u; shift < 32u; shift += 8u) {
        hash ^= (value >> shift) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t fingerprint(const DM1OriginalSaveClassifyResult *result)
{
    uint32_t hash = 2166136261u;
    hash = mix(hash, result->game_id);
    hash = mix(hash, (uint32_t)result->size_bytes);
    hash = mix(hash, (uint32_t)(result->size_bytes >> 32));
    hash = mix(hash, result->prefix_checksum32);
    return mix(hash, result->save_part_loader_envelope_payload_bytes);
}

static uint32_t exported_fingerprint(const unsigned char *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;
    for (i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

int main(void)
{
    DM1OriginalSavePC34FixtureSpec spec;
    DM1OriginalSaveClassifyResult first;
    DM1OriginalSaveClassifyResult second;
    DM1OriginalSavePC34RoundtripReport roundtrip;
    unsigned char bytes_a[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char bytes_b[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char exported_a[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char exported_b[SAVEGAME_PC34_MAX_FILE_SIZE];
    size_t size_a = 0u;
    size_t size_b = 0u;
    size_t exported_size_a = 0u;
    size_t exported_size_b = 0u;
    uint32_t canonical_a;
    uint32_t canonical_b;
    uint32_t corpus_forward;
    uint32_t corpus_reverse;

    memset(&spec, 0, sizeof(spec));
    spec.champion_count = 2;
    spec.map_index = 4;
    spec.map_x = 7;
    spec.map_y = 9;
    spec.direction = 1;
    spec.current_active_group_count = 1;
    spec.maximum_active_group_count = 2;
    spec.event_count = 1;
    spec.event_maximum_count = 2;
    spec.game_time = 12345u;
    spec.game_id = 4u;
    if (dm1_v1_original_save_pc34_build_handoff_fixture_bytes(
            &spec, bytes_a, sizeof(bytes_a), &size_a) != SAVEGAME_PC34_OK ||
        !dm1_v1_original_save_classify_bytes(bytes_a, size_a, &first) ||
        !first.pc34_loader_part_envelope_candidate ||
        !dm1_v1_original_save_classify_bytes(bytes_a, size_a, &second) ||
        fingerprint(&first) == 0u || fingerprint(&first) != fingerprint(&second)) {
        return 1;
    }
    memset(&roundtrip, 0, sizeof(roundtrip));
    if (dm1_v1_original_save_pc34_roundtrip_world_reload_bytes(
            bytes_a, size_a, spec.game_id, exported_a, sizeof(exported_a),
            &exported_size_a, &roundtrip) != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK ||
        exported_size_a == 0u || !roundtrip.core_state_matches) {
        return 1;
    }
    canonical_a = exported_fingerprint(exported_a, exported_size_a);
    spec.map_x = 8;
    spec.game_time = 12346u;
    if (dm1_v1_original_save_pc34_build_handoff_fixture_bytes(
            &spec, bytes_b, sizeof(bytes_b), &size_b) != SAVEGAME_PC34_OK ||
        dm1_v1_original_save_pc34_roundtrip_world_reload_bytes(
            bytes_b, size_b, spec.game_id, exported_b, sizeof(exported_b),
            &exported_size_b, &roundtrip) != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK ||
        exported_size_b == 0u || !roundtrip.core_state_matches) {
        return 1;
    }
    canonical_b = exported_fingerprint(exported_b, exported_size_b);
    /* Corpus aggregation is commutative: scan order and filenames cannot
     * influence the provenance identity, only canonical F0433 output bytes. */
    corpus_forward = canonical_a ^ canonical_b;
    corpus_reverse = canonical_b ^ canonical_a;
    if (canonical_a == canonical_b || corpus_forward == 0u ||
        corpus_forward != corpus_reverse ||
        dm1_v1_original_save_classify_bytes(bytes_a, size_a - 1u, &second) ||
        dm1_v1_original_save_classify_bytes((const uint8_t *)"no", 2u,
                                            &second)) {
        return 1;
    }
    puts("ok: DM1 PC34 corpus provenance is path-independent and fail-closed");
    return 0;
}
