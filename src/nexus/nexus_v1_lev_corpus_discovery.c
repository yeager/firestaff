#include "nexus_v1_lev_corpus_discovery.h"

#include "asset_find_by_hash.h"

#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

static uint64_t fnv1a64_update(uint64_t hash, const uint8_t *bytes, size_t size)
{
    size_t i;
    for (i = 0U; i < size; ++i) {
        hash ^= bytes[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static int direct_file_identity(const char *path, uint64_t *out_size,
                                uint64_t *out_fnv1a64)
{
    uint8_t buffer[4096];
    uint64_t total = 0U;
    uint64_t hash = UINT64_C(1469598103934665603);
    FILE *file;
    size_t count;
    int io_failed;

    if (!path || !out_size || !out_fnv1a64 || strstr(path, "::") ||
        !(file = fopen(path, "rb"))) return 0;
    while ((count = fread(buffer, 1U, sizeof(buffer), file)) != 0U) {
        if (UINT64_MAX - total < count) {
            fclose(file);
            return 0;
        }
        total += count;
        hash = fnv1a64_update(hash, buffer, count);
    }
    /* fclose() must not sit behind ferror() in a || chain: a read error on a
     * corpus file would skip the close, leaking one descriptor per scanned
     * file rather than one overall. */
    io_failed = ferror(file) != 0;
    if (fclose(file) != 0) io_failed = 1;
    if (io_failed || total == 0U || hash == 0U)
        return 0;
    *out_size = total;
    *out_fnv1a64 = hash;
    return 1;
}

int nexus_v1_lev_corpus_discover_direct_expected(
    const char *data_dir, const Nexus_V1_LevCorpusExpectedLevel *expected,
    uint32_t expected_count, Nexus_V1_LevCorpusDiscoveryReceipt *out_receipt)
{
    Nexus_V1_LevCorpusDiscoveryReceipt receipt;
    const char *md5s[NEXUS_V1_LEV_CORPUS_LEVEL_COUNT + 1U];
    char paths[NEXUS_V1_LEV_CORPUS_LEVEL_COUNT][ASSET_PATH_MAX];
    int matched[NEXUS_V1_LEV_CORPUS_LEVEL_COUNT];
    uint32_t i;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.direct_files_only = 1;
    if (!data_dir || !data_dir[0]) {
        receipt.skipped_missing_corpus = 1;
        *out_receipt = receipt;
        return 0;
    }
    if (!expected || expected_count != NEXUS_V1_LEV_CORPUS_LEVEL_COUNT) {
        *out_receipt = receipt;
        return 0;
    }
    for (i = 0U; i < expected_count; ++i) {
        if (expected[i].level_index != i || !expected[i].expected_md5 ||
            strlen(expected[i].expected_md5) != 32U) {
            *out_receipt = receipt;
            return 0;
        }
        md5s[i] = expected[i].expected_md5;
    }
    md5s[expected_count] = NULL;
    memset(paths, 0, sizeof(paths));
    memset(matched, 0, sizeof(matched));
    if (asset_find_all_files_by_md5_list(data_dir, md5s, paths, matched,
                                         (int)expected_count, 2) !=
        (int)expected_count) {
        receipt.skipped_missing_corpus = 1;
        *out_receipt = receipt;
        return 0;
    }
    for (i = 0U; i < expected_count; ++i) {
        Nexus_V1_LevCorpusDirectLevelIdentity *level = &receipt.levels[i];
        uint32_t j;

        if (!matched[i] || !paths[i][0] || strstr(paths[i], "::") ||
            !asset_file_matches_md5(paths[i], expected[i].expected_md5) ||
            !direct_file_identity(paths[i], &level->byte_count,
                                  &level->fnv1a64)) {
            *out_receipt = receipt;
            return 0;
        }
        for (j = 0U; j < i; ++j) {
            if (strcmp(paths[i], receipt.levels[j].direct_path) == 0 ||
                level->fnv1a64 == receipt.levels[j].fnv1a64) {
                *out_receipt = receipt;
                return 0;
            }
        }
        level->valid = 1;
        level->level_index = i;
        snprintf(level->direct_path, sizeof(level->direct_path), "%s", paths[i]);
        snprintf(level->md5, sizeof(level->md5), "%s", expected[i].expected_md5);
        receipt.corpus_byte_count += level->byte_count;
        receipt.corpus_fnv1a64 = fnv1a64_update(
            receipt.corpus_fnv1a64 ? receipt.corpus_fnv1a64 :
                UINT64_C(1469598103934665603),
            (const uint8_t *)&level->fnv1a64, sizeof(level->fnv1a64));
    }
    if (!receipt.corpus_byte_count || !receipt.corpus_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_lev_corpus_discover_direct(
    const char *data_dir, Nexus_V1_LevCorpusDiscoveryReceipt *out_receipt)
{
    Nexus_V1_LevCorpusExpectedLevel expected[NEXUS_V1_LEV_CORPUS_LEVEL_COUNT];
    char names[NEXUS_V1_LEV_CORPUS_LEVEL_COUNT][16];
    uint32_t i;

    for (i = 0U; i < NEXUS_V1_LEV_CORPUS_LEVEL_COUNT; ++i) {
        snprintf(names[i], sizeof(names[i]), "LEV%02u.DGN", i);
        expected[i].level_index = i;
        expected[i].expected_md5 = nexus_v1_known_file_md5(names[i]);
        if (!expected[i].expected_md5) {
            if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
            return 0;
        }
    }
    return nexus_v1_lev_corpus_discover_direct_expected(
        data_dir, expected, NEXUS_V1_LEV_CORPUS_LEVEL_COUNT, out_receipt);
}

int nexus_v1_lev_corpus_direct_identity_still_matches(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity)
{
    struct stat status;
    uint64_t byte_count;
    uint64_t fnv1a64;

    if (!identity || !identity->valid || !identity->direct_path[0] ||
        !identity->md5[0] || !identity->byte_count || !identity->fnv1a64 ||
        strstr(identity->direct_path, "::") ||
        stat(identity->direct_path, &status) != 0 ||
        !S_ISREG(status.st_mode) ||
        !asset_file_matches_md5(identity->direct_path, identity->md5) ||
        !direct_file_identity(identity->direct_path, &byte_count, &fnv1a64)) {
        return 0;
    }
    return byte_count == identity->byte_count && fnv1a64 == identity->fnv1a64;
}

int nexus_v1_lev_corpus_build_header_descriptor_provenance(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    Nexus_V1_DgnDirectLevHeaderDescriptorProvenance *out_provenance)
{
    Nexus_V1_DgnDirectLevHeaderDescriptorProvenance provenance;
    Nexus_V1_DgnStructure1Layout layout;
    uint64_t dgn_fnv1a64;
    uint64_t descriptor_end;

    if (!out_provenance) return 0;
    memset(&provenance, 0, sizeof(provenance));
    if (!identity || !identity->valid || !identity->byte_count ||
        !identity->fnv1a64 || !dgn_data || dgn_size < NEXUS_DGN_BLOCK_SIZE ||
        identity->byte_count != (uint64_t)dgn_size) {
        *out_provenance = provenance;
        return 0;
    }
    dgn_fnv1a64 = fnv1a64_update(UINT64_C(1469598103934665603), dgn_data,
                                  (size_t)dgn_size);
    if (dgn_fnv1a64 != identity->fnv1a64 ||
        nexus_v1_dgn_structure1_layout(&layout, dgn_data, dgn_size) != 0 ||
        !layout.valid || !layout.structure1f.valid ||
        layout.structure1f.relative_offset < 0 || layout.structure1f.size <= 0 ||
        layout.structure1f.total_entry_count < 0) {
        *out_provenance = provenance;
        return 0;
    }
    descriptor_end = (uint64_t)layout.structure1_offset +
        (uint64_t)layout.structure1f.relative_offset +
        (uint64_t)layout.structure1f.size;
    if (layout.structure1_offset < 0 ||
        descriptor_end > (uint64_t)dgn_size ||
        (uint64_t)layout.structure1f.total_entry_count > UINT32_MAX) {
        *out_provenance = provenance;
        return 0;
    }
    provenance.valid = 1;
    provenance.level_index = identity->level_index;
    provenance.header_offset = 0U;
    provenance.header_length = 0x14U;
    provenance.header_fnv1a64 = fnv1a64_update(UINT64_C(1469598103934665603),
                                                dgn_data,
                                                provenance.header_length);
    provenance.descriptor_offset = (uint32_t)((uint64_t)layout.structure1_offset +
        (uint64_t)layout.structure1f.relative_offset);
    provenance.descriptor_length = (uint32_t)layout.structure1f.size;
    provenance.descriptor_count = (uint32_t)layout.structure1f.total_entry_count;
    provenance.descriptor_fnv1a64 = fnv1a64_update(
        UINT64_C(1469598103934665603),
        dgn_data + provenance.descriptor_offset, provenance.descriptor_length);
    provenance.package_fnv1a64 = dgn_fnv1a64;
    provenance.no_draw_only = 1;
    provenance.blocks_real_dgn_mesh_render = 1;
    if (!provenance.header_fnv1a64 || !provenance.descriptor_fnv1a64) {
        memset(&provenance, 0, sizeof(provenance));
    }
    *out_provenance = provenance;
    return provenance.valid;
}

int nexus_v1_lev_corpus_bind_active_level_no_draw(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *receipt,
    uint32_t level_index, const uint8_t *active_dgn, int active_dgn_size)
{
    const Nexus_V1_LevCorpusDirectLevelIdentity *level;
    uint64_t hash;

    if (!engine || !receipt || !receipt->valid ||
        receipt->payload_materialized || !receipt->direct_files_only ||
        level_index >= NEXUS_V1_LEV_CORPUS_LEVEL_COUNT || !active_dgn ||
        active_dgn_size <= 0 || engine->game.current_level != (int)level_index ||
        engine->current_level_dgn_data != active_dgn ||
        engine->current_level_dgn_size != active_dgn_size) return 0;
    level = &receipt->levels[level_index];
    hash = fnv1a64_update(UINT64_C(1469598103934665603), active_dgn,
                          (size_t)active_dgn_size);
    if (!nexus_v1_lev_corpus_direct_identity_still_matches(level) ||
        level->byte_count != (uint64_t)active_dgn_size ||
        level->fnv1a64 != hash) return 0;
    engine->current_level_structure2_source.level_index = (int)level_index;
    engine->current_level_structure2_source.canonical_hash_verified = 1;
    engine->current_level_structure2_source.materialization_bound = 1;
    engine->current_level_structure2_source.loaded_bytes_bound = 1;
    engine->current_level_structure2_source.loaded_dgn_size = active_dgn_size;
    engine->current_level_structure2_source.loaded_dgn_fnv1a64 = hash;
    engine->current_level_structure2_source.structure2_payload_envelope_valid = 1;
    engine->current_level_structure2_source.payload_decoder_permitted = 0;
    engine->current_level_structure2_source.fallback_visuals_permitted = 0;
    return 1;
}

int nexus_v1_lev_corpus_admit_m11_dungeon_no_draw(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *receipt,
    uint64_t route_epoch, uint32_t level_index,
    const Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt *geometry)
{
    const Nexus_V1_LevCorpusDirectLevelIdentity *level;
    Nexus_V1_DgnDirectLevHeaderDescriptorProvenance header_descriptor;

    if (!engine || !receipt || !receipt->valid ||
        level_index >= NEXUS_V1_LEV_CORPUS_LEVEL_COUNT || !route_epoch) return 0;
    level = &receipt->levels[level_index];
    if (!level->valid || !level->md5[0] || !level->byte_count || !level->fnv1a64 ||
        !nexus_v1_lev_corpus_bind_active_level_no_draw(
            engine, receipt, level_index, engine->current_level_dgn_data,
            engine->current_level_dgn_size) ||
        !nexus_v1_lev_corpus_build_header_descriptor_provenance(
            level, engine->current_level_dgn_data,
            engine->current_level_dgn_size, &header_descriptor)) return 0;
    return nexus_v1_engine_set_m11_direct_lev_dungeon_no_draw_receipt(
        engine, route_epoch, (int)level_index, level->md5, level->byte_count,
        level->fnv1a64, &header_descriptor, geometry);
}
