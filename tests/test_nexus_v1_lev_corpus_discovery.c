#define _POSIX_C_SOURCE 200809L

#include "nexus_v1_lev_corpus_discovery.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        ++failures; \
    } \
} while (0)

static uint64_t fixture_fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t i;

    for (i = 0U; i < size; ++i) {
        hash ^= bytes[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static void fixture_be16(uint8_t *bytes, size_t offset, uint16_t value)
{
    bytes[offset] = (uint8_t)(value >> 8);
    bytes[offset + 1U] = (uint8_t)value;
}

static void fixture_be32(uint8_t *bytes, size_t offset, uint32_t value)
{
    bytes[offset] = (uint8_t)(value >> 24);
    bytes[offset + 1U] = (uint8_t)(value >> 16);
    bytes[offset + 2U] = (uint8_t)(value >> 8);
    bytes[offset + 3U] = (uint8_t)value;
}

static uint8_t *fixture_minimal_dgn(size_t *out_size)
{
    const size_t size = 0x9000U;
    const size_t structure1 = 0x800U;
    uint8_t *bytes = (uint8_t *)calloc(1U, size);

    if (!bytes) return NULL;
    fixture_be16(bytes, 0x0cU, 1U);
    fixture_be16(bytes, 0x0eU, 17U);
    fixture_be32(bytes, 0x10U, 0x8048U);
    bytes[structure1 + 2U] = 0x40U;
    bytes[structure1 + 3U] = 0x40U;
    fixture_be32(bytes, structure1 + 0x10U, 0x38U);
    fixture_be32(bytes, structure1 + 0x14U, 0x38U);
    fixture_be32(bytes, structure1 + 0x34U, 0x8038U);
    *out_size = size;
    return bytes;
}

int main(void)
{
    static const char letters[] = "abcdefghijklmnop";
    static const char *const md5s[16] = {
        "23ca472302f49b3ea5592b146a312da0",
        "458cea6e6d953307cac11533c2f4d03d",
        "0f5be288a5f008ab2ecd84af048edb0a",
        "f8d9024e1efc5c82b039694d66704264",
        "4e42d6a2302a9c53ae84b080d6c714db",
        "527848b310b793e41e0616f38ee85059",
        "c005d21e6f74e5515ab798ff251bcb88",
        "a22f54f8f1fb7a1f0cae2ce9b677d5f1",
        "608796394f9a5e7a23e4dc0e432845de",
        "62ef56bd00f791447b5a55b5373db2e1",
        "e5e0766271462dc6afd0ec2534382bf0",
        "339f18db4f791a1fab73ff9a1c167669",
        "6ecd1f1b606a74eecd1f9451923f46ff",
        "f4d0803271994194972edb3928efd4dd",
        "3f349bd7f413433357d94acbf19688d1",
        "5be3971b6ea34a76dd15daf44fbbcf1f"
    };
    Nexus_V1_LevCorpusExpectedLevel expected[16];
    Nexus_V1_LevCorpusDiscoveryReceipt receipt;
    Nexus_V1_Engine engine;
    char directory[128];
    char path[ASSET_PATH_MAX];
    uint8_t active[16];
    uint32_t i;

    {
        Nexus_V1_LevCorpusDirectLevelIdentity identity;
        Nexus_V1_DgnDirectLevHeaderDescriptorProvenance provenance;
        uint8_t *dgn;
        size_t dgn_size = 0U;

        memset(&identity, 0, sizeof(identity));
        dgn = fixture_minimal_dgn(&dgn_size);
        CHECK(dgn != NULL, "minimal DGN descriptor provenance fixture allocates");
        if (dgn) {
            identity.valid = 1;
            identity.level_index = 3U;
            identity.byte_count = dgn_size;
            identity.fnv1a64 = fixture_fnv1a64(dgn, dgn_size);
            memset(&provenance, 0, sizeof(provenance));
            CHECK(nexus_v1_lev_corpus_build_header_descriptor_provenance(
                      &identity, dgn, (int)dgn_size, &provenance) == 1 &&
                  provenance.valid && provenance.level_index == 3U &&
                  provenance.header_offset == 0U &&
                  provenance.header_length == 0x14U &&
                  provenance.descriptor_offset == 0x8838U &&
                  provenance.descriptor_length == 16U &&
                  provenance.descriptor_count == 0U &&
                  provenance.package_fnv1a64 == identity.fnv1a64 &&
                  provenance.no_draw_only && !provenance.fallback_visuals_permitted &&
                  provenance.blocks_real_dgn_mesh_render,
                  "parser-backed direct DGN header and Structure1F descriptor provenance is bounded no-draw evidence");
            dgn[0x800U + 0x34U] = 0U;
            dgn[0x800U + 0x35U] = 0U;
            dgn[0x800U + 0x36U] = 0U;
            dgn[0x800U + 0x37U] = 0U;
            identity.fnv1a64 = fixture_fnv1a64(dgn, dgn_size);
            CHECK(nexus_v1_lev_corpus_build_header_descriptor_provenance(
                      &identity, dgn, (int)dgn_size, &provenance) == 0 &&
                  !provenance.valid,
                  "malformed Structure1F descriptor pointer rejects before M11 admission");
            dgn[0x800U + 0x34U] = 0U;
            dgn[0x800U + 0x35U] = 0U;
            dgn[0x800U + 0x36U] = 0x80U;
            dgn[0x800U + 0x37U] = 0x38U;
            identity.fnv1a64 = fixture_fnv1a64(dgn, dgn_size);
            identity.byte_count = dgn_size - 1U;
            CHECK(nexus_v1_lev_corpus_build_header_descriptor_provenance(
                      &identity, dgn, (int)dgn_size, &provenance) == 0 &&
                  !provenance.valid,
                  "descriptor provenance rejects an out-of-bounds source span");
            identity.byte_count = dgn_size;
            identity.fnv1a64 ^= UINT64_C(1);
            CHECK(nexus_v1_lev_corpus_build_header_descriptor_provenance(
                      &identity, dgn, (int)dgn_size, &provenance) == 0 &&
                  !provenance.valid,
                  "descriptor provenance rejects package FNV drift");
            free(dgn);
        }
    }

    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_v1_lev_corpus_discover_direct(NULL, &receipt) == 0 &&
          !receipt.valid && receipt.skipped_missing_corpus &&
          receipt.direct_files_only && !receipt.payload_materialized,
          "production LEV discovery skips safely when local retail media is absent");
    CHECK(snprintf(directory, sizeof(directory),
                   "/tmp/firestaff-nexus-lev-corpus-%ld", (long)getpid()) > 0 &&
#ifdef _WIN32
          mkdir(directory) == 0,
#else
          mkdir(directory, 0700) == 0,
#endif
          "temporary direct LEV corpus directory exists");
    for (i = 0U; i < 16U && failures == 0; ++i) {
        FILE *file = NULL;
        expected[i].level_index = i;
        expected[i].expected_md5 = md5s[i];
        CHECK(snprintf(path, sizeof(path), "%s/renamed-level-%02u.DGN", directory,
                       i) > 0 && (file = fopen(path, "wb")) != NULL,
              "direct LEV fixture opens");
        if (file) {
            char bytes[16];
            memset(bytes, letters[i], sizeof(bytes));
            CHECK(fwrite(bytes, 1U, sizeof(bytes), file) == sizeof(bytes) &&
                  fclose(file) == 0,
                  "direct LEV fixture writes exact identity byte");
        }
    }
    memset(&receipt, 0, sizeof(receipt));
    CHECK(failures == 0 && nexus_v1_lev_corpus_discover_direct_expected(
              directory, expected, 16U, &receipt) == 1 && receipt.valid &&
          receipt.direct_files_only && !receipt.payload_materialized &&
          !receipt.skipped_missing_corpus && receipt.corpus_byte_count == 256U &&
          receipt.levels[0].valid && receipt.levels[0].level_index == 0U &&
          strstr(receipt.levels[0].direct_path, "::") == NULL &&
          strcmp(receipt.levels[0].md5, md5s[0]) == 0,
          "hash-first direct discovery retains only verified LEV identities");

    memset(&engine, 0, sizeof(engine));
    memset(active, 'a', sizeof(active));
    engine.game.current_level = 0;
    engine.current_level_dgn_data = active;
    engine.current_level_dgn_size = (int)sizeof(active);
    CHECK(nexus_v1_lev_corpus_bind_active_level_no_draw(
              &engine, &receipt, 0U, active, (int)sizeof(active)) == 1 &&
          engine.current_level_structure2_source.loaded_bytes_bound &&
          engine.current_level_structure2_source.loaded_dgn_fnv1a64 ==
              receipt.levels[0].fnv1a64 &&
          !engine.current_level_structure2_source.payload_decoder_permitted &&
          !engine.current_level_structure2_source.fallback_visuals_permitted,
          "active no-draw route binds only the discovered direct DGN identity");
    active[0] = 'z';
    CHECK(nexus_v1_lev_corpus_bind_active_level_no_draw(
              &engine, &receipt, 0U, active, (int)sizeof(active)) == 0,
          "active no-draw route rejects changed DGN bytes");
    active[0] = 'a';

    {
        FILE *file = fopen(receipt.levels[0].direct_path, "wb");
        CHECK(file != NULL && fwrite("zzzzzzzzzzzzzzzz", 1U, 16U, file) == 16U &&
                  fclose(file) == 0,
              "direct LEV fixture can be mutated after discovery");
    }
    CHECK(!nexus_v1_lev_corpus_direct_identity_still_matches(
              &receipt.levels[0]) &&
              nexus_v1_lev_corpus_bind_active_level_no_draw(
                  &engine, &receipt, 0U, active, (int)sizeof(active)) == 0,
          "direct LEV source drift rejects active no-draw admission");
    {
        FILE *file = fopen(receipt.levels[0].direct_path, "wb");
        CHECK(file != NULL && fwrite("aaaaaaaaaaaaaaaa", 1U, 16U, file) == 16U &&
                  fclose(file) == 0,
              "direct LEV fixture restores its discovered source bytes");
    }
    CHECK(nexus_v1_lev_corpus_direct_identity_still_matches(
              &receipt.levels[0]),
          "restored direct LEV source identity rehashes exactly");

    CHECK(snprintf(path, sizeof(path), "%s/renamed-level-15.DGN", directory) > 0 &&
          unlink(path) == 0,
          "one direct LEV fixture is removed for skip-safe missing-corpus path");
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_v1_lev_corpus_discover_direct_expected(directory, expected, 16U,
                                                        &receipt) == 0 &&
          !receipt.valid && receipt.skipped_missing_corpus &&
          receipt.direct_files_only && !receipt.payload_materialized,
          "missing direct corpus skips without accepting virtual or retained payload data");

    for (i = 0U; i < 15U; ++i) {
        if (snprintf(path, sizeof(path), "%s/renamed-level-%02u.DGN", directory,
                     i) > 0) unlink(path);
    }
    rmdir(directory);
    if (failures) {
        fprintf(stderr, "test_nexus_v1_lev_corpus_discovery: FAIL %d\n", failures);
        return 1;
    }
    puts("test_nexus_v1_lev_corpus_discovery: PASS");
    return 0;
}
