/* Rebuild and verify a source-only Structure3 capture campaign. This accepts
 * no Saturn pixels or decoder facts: it proves only that the request ledger
 * exactly matches the hash-verified retail LEV corpus. */
#include "nexus_v1_engine.h"
#include "nexus_v1_structure3_capture_manifest.h"
#include "asset_find_by_hash.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *read_file(const char *path, int *out_size)
{
    FILE *file;
    long size;
    uint8_t *data;

    if (!path || !out_size || !(file = fopen(path, "rb"))) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        size > INT_MAX || fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    data = (uint8_t *)malloc((size_t)size + 1U);
    if (!data || fread(data, 1U, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    data[size] = 0U;
    *out_size = (int)size;
    return data;
}

static int canonical_file_matches_md5(const char *path, const char *md5)
{
    uint8_t *bytes;
    int size;
    int matches;

    bytes = read_file(path, &size);
    if (!bytes) return 0;
    matches = nexus_v1_dgn_bytes_match_canonical_md5(bytes, size, md5);
    free(bytes);
    return matches;
}

static int read_hex(const char **cursor, const char *label, uint64_t *out)
{
    unsigned long long value;
    int consumed = 0;

    if (!cursor || !*cursor || !label || !out ||
        strncmp(*cursor, label, strlen(label)) != 0 ||
        sscanf(*cursor + strlen(label), "%llx%n", &value, &consumed) != 1 ||
        (*cursor)[strlen(label) + (size_t)consumed] != '\n') return 0;
    *out = (uint64_t)value;
    *cursor += strlen(label) + (size_t)consumed + 1U;
    return 1;
}

static int read_flag(const char **cursor, const char *text)
{
    size_t size = strlen(text);
    if (!cursor || !*cursor || strncmp(*cursor, text, size) != 0) return 0;
    *cursor += size;
    return 1;
}

static int parse_target(const char *text,
                        Nexus_V1_DgnStructure3CaptureTargetReceipt *target)
{
    const char *p;
    uint64_t value;

    if (!text || !target ||
        strncmp(text, NEXUS_V1_STRUCTURE3_CAPTURE_TARGET_MAGIC "\n",
                sizeof(NEXUS_V1_STRUCTURE3_CAPTURE_TARGET_MAGIC)) != 0) return 0;
    memset(target, 0, sizeof(*target));
    p = text + sizeof(NEXUS_V1_STRUCTURE3_CAPTURE_TARGET_MAGIC);
#define TARGET_FIELD(label, member) \
    if (!read_hex(&p, label, &value) || value > UINT32_MAX) return 0; \
    target->member = (uint32_t)value
    TARGET_FIELD("level_index=", level_index);
    if (target->level_index < 0 || target->level_index > 15) return 0;
    if (!read_hex(&p, "dgn_fnv1a64=", &target->candidate.dgn_fnv1a64)) return 0;
    TARGET_FIELD("structure3_payload_fnv1a32=", candidate.structure3_payload_fnv1a32);
    TARGET_FIELD("typed_mesh_corpus_fnv1a32=", candidate.typed_mesh_corpus_fnv1a32);
    TARGET_FIELD("entry_index=", candidate.entry_index);
    TARGET_FIELD("face_ordinal=", candidate.face_ordinal);
    TARGET_FIELD("face_row_fnv1a32=", candidate.face_row_fnv1a32);
    TARGET_FIELD("referenced_vertex_rows_fnv1a32=", candidate.referenced_vertex_rows_fnv1a32);
    TARGET_FIELD("normal_row_fnv1a32=", candidate.normal_row_fnv1a32);
    TARGET_FIELD("fill_selector=", candidate.fill_selector);
    TARGET_FIELD("entry_byte_offset=", entry_byte_offset);
    TARGET_FIELD("vertex_byte_offset=", vertex_byte_offset);
    TARGET_FIELD("face_byte_offset=", face_byte_offset);
    TARGET_FIELD("normal_byte_offset=", normal_byte_offset);
    TARGET_FIELD("vertex_count=", vertex_count);
    TARGET_FIELD("face_count=", face_count);
#undef TARGET_FIELD
    if (!read_flag(&p, "capture_manifest_magic=" NEXUS_V1_STRUCTURE3_CAPTURE_MANIFEST_MAGIC "\n") ||
        !read_flag(&p, "required_lanes=texture_span,palette_state,vdp1_state,transform_state,normal_culling_state,vdp1_command\n") ||
        !read_flag(&p, "original_saturn_capture_required=1\n") ||
        !read_flag(&p, "no_draw_only=1\n") || *p != '\0') return 0;
    target->valid = 1;
    target->capture_producer_required = 1;
    target->original_saturn_capture_required = 1;
    target->no_draw_only = 1;
    return 1;
}

static int target_matches(const Nexus_V1_DgnStructure3CaptureTargetReceipt *a,
                          const Nexus_V1_DgnStructure3CaptureTargetReceipt *b)
{
    return a && b && a->valid && b->valid &&
        a->level_index == b->level_index &&
        a->candidate.dgn_fnv1a64 == b->candidate.dgn_fnv1a64 &&
        a->candidate.structure3_payload_fnv1a32 == b->candidate.structure3_payload_fnv1a32 &&
        a->candidate.typed_mesh_corpus_fnv1a32 == b->candidate.typed_mesh_corpus_fnv1a32 &&
        a->candidate.entry_index == b->candidate.entry_index &&
        a->candidate.face_ordinal == b->candidate.face_ordinal &&
        a->candidate.face_row_fnv1a32 == b->candidate.face_row_fnv1a32 &&
        a->candidate.referenced_vertex_rows_fnv1a32 == b->candidate.referenced_vertex_rows_fnv1a32 &&
        a->candidate.normal_row_fnv1a32 == b->candidate.normal_row_fnv1a32 &&
        a->candidate.fill_selector == b->candidate.fill_selector &&
        a->entry_byte_offset == b->entry_byte_offset &&
        a->vertex_byte_offset == b->vertex_byte_offset &&
        a->face_byte_offset == b->face_byte_offset &&
        a->normal_byte_offset == b->normal_byte_offset &&
        a->vertex_count == b->vertex_count && a->face_count == b->face_count &&
        !a->fallback_visuals_permitted && !b->fallback_visuals_permitted;
}

static int parse_ledger(const char *text,
                        Nexus_V1_DgnStructure3CaptureCampaignReceipt *ledger)
{
    const char *p;
    uint64_t value;

    if (!text || !ledger ||
        strncmp(text, NEXUS_V1_STRUCTURE3_CAPTURE_CAMPAIGN_MAGIC "\n",
                sizeof(NEXUS_V1_STRUCTURE3_CAPTURE_CAMPAIGN_MAGIC)) != 0) return 0;
    memset(ledger, 0, sizeof(*ledger));
    p = text + sizeof(NEXUS_V1_STRUCTURE3_CAPTURE_CAMPAIGN_MAGIC);
    if (!read_hex(&p, "target_count=", &value) || value > UINT32_MAX) return 0;
    ledger->target_count = (uint32_t)value;
    if (!read_hex(&p, "level_mask=", &value) || value > UINT32_MAX) return 0;
    ledger->level_mask = (uint32_t)value;
    if (!read_hex(&p, "ordered_target_fnv1a64=", &ledger->ordered_target_fnv1a64) ||
        !read_hex(&p, "source_identity_fnv1a64=", &ledger->source_identity_fnv1a64) ||
        !read_hex(&p, "typed_mesh_corpus_fnv1a32=", &value) ||
        value != NEXUS_DGN_RETAIL_TYPED_MESH_CORPUS_FNV1A32 ||
        !read_flag(&p, "structure1a_model_entry_mapping_proven=0\n") ||
        !read_flag(&p, "original_saturn_capture_required=1\n") ||
        !read_flag(&p, "no_draw_only=1\n") ||
        !read_flag(&p, "decoder_or_renderer_authorized=0\n") || *p != '\0') return 0;
    ledger->original_saturn_capture_required = 1;
    ledger->no_draw_only = 1;
    return ledger->target_count != 0U && ledger->level_mask == UINT32_C(0xffff);
}

int main(int argc, char **argv)
{
    Nexus_V1_DgnStructure3CaptureCampaignReceipt expected;
    Nexus_V1_DgnStructure3CaptureCampaignReceipt ledger;
    int level;
    int verified = 0;

    if (argc != 3) {
        fprintf(stderr, "usage: %s DATA_DIR CAMPAIGN_DIRECTORY\n", argv[0]);
        return 2;
    }
    nexus_v1_dgn_structure3_capture_campaign_init(&expected);
    for (level = 0; level <= 15; ++level) {
        char lev_name[16], lev_path[1024], target_path[1024];
        const char *md5;
        uint8_t *dgn;
        int dgn_size;
        Nexus_V1_Level data;
        uint32_t entry;

        snprintf(lev_name, sizeof(lev_name), "LEV%02d.DGN", level);
        md5 = nexus_v1_known_file_md5(lev_name);
        if (snprintf(lev_path, sizeof(lev_path), "%s/%s", argv[1], lev_name) >=
                (int)sizeof(lev_path) || !md5 || !canonical_file_matches_md5(lev_path, md5) ||
            !(dgn = read_file(lev_path, &dgn_size))) goto failed;
        memset(&data, 0, sizeof(data));
        if (nexus_v1_level_load(&data, dgn, dgn_size, level) != 0) {
            free(dgn);
            goto failed;
        }
        for (entry = 0U; entry < (uint32_t)data.structure3_directory.entry_count; ++entry) {
            uint32_t face;
            for (face = 0U; face < data.structure3_entry_face_counts[entry]; ++face) {
                Nexus_V1_DgnStructure3CaptureTargetReceipt expected_target, actual_target;
                uint8_t *target_bytes = NULL;
                int target_size;
                if (!nexus_v1_dgn_structure3_capture_target_build(
                        &data, dgn, dgn_size, level, 1, entry, face, &expected_target) ||
                    snprintf(target_path, sizeof(target_path), "%s/LEV%02d-E%04u-F%04u.target",
                             argv[2], level, entry, face) >= (int)sizeof(target_path) ||
                    !(target_bytes = read_file(target_path, &target_size)) ||
                    !parse_target((const char *)target_bytes, &actual_target) ||
                    !target_matches(&expected_target, &actual_target) ||
                    !nexus_v1_dgn_structure3_capture_campaign_add_target(
                        &expected, &expected_target)) {
                    free(target_bytes);
                    free(dgn);
                    goto failed;
                }
                (void)target_size;
                free(target_bytes);
                ++verified;
            }
        }
        free(dgn);
    }
    {
        char ledger_path[1024];
        uint8_t *ledger_bytes = NULL;
        int ledger_size;
        if (snprintf(ledger_path, sizeof(ledger_path),
                     "%s/STRUCTURE3-FACE-CAPTURE-CAMPAIGN.target", argv[2]) >=
                (int)sizeof(ledger_path) ||
            !(ledger_bytes = read_file(ledger_path, &ledger_size)) ||
            !parse_ledger((const char *)ledger_bytes, &ledger) ||
            ledger.target_count != expected.target_count ||
            ledger.level_mask != expected.level_mask ||
            ledger.ordered_target_fnv1a64 != expected.ordered_target_fnv1a64 ||
            ledger.source_identity_fnv1a64 != expected.source_identity_fnv1a64) {
            free(ledger_bytes);
            goto failed;
        }
        (void)ledger_size;
        free(ledger_bytes);
    }
    printf("verified %d source-only Structure3 face targets\n", verified);
    printf("structure1a_model_entry_mapping_proven=0\n");
    printf("decoder_or_renderer_authorized=0\n");
    return 0;
failed:
    fprintf(stderr, "Structure3 capture campaign does not match canonical source\n");
    return 1;
}
