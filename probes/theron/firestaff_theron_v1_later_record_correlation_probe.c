#include "asset_status_m12.h"
#include "theron_v1_later_record_correlation.h"
#include "theron_v1_stage3_manifest_evidence.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_fail;
static int g_skip;

static void check(int condition, const char *name) {
    if (!condition) {
        ++g_fail;
        printf("[FAIL] %s\n", name);
    } else {
        printf("[PASS] %s\n", name);
    }
}

static void test_bounded_selector_catalog(void) {
    Theron_V1Stage3ManifestEvidence manifest;
    Theron_V1LaterRecordCorrelation correlation;
    Theron_V1Stage3DescriptorRecordBoundary boundary;
    uint8_t raw_track[40u * 2352u];
    size_t index;

#define PUT_BE16(destination, value) \
    do { \
        (destination)[0] = (uint8_t)((value) >> 8u); \
        (destination)[1] = (uint8_t)(value); \
    } while (0)

    memset(&manifest, 0, sizeof(manifest));
    manifest.valid = 1;
    manifest.variant = THERON_TRACK02_VARIANT_US_BIN;
    manifest.track02_record = 20u;
    manifest.raw_sector = 20u;
    manifest.raw_offset = 20u * 2352u;
    manifest.user_data_offset = manifest.raw_offset + 16u;
    manifest.descriptor_count =
        THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT;
    manifest.first_descriptor.word2 = 10u;
    manifest.descriptors[0].word2 = 10u;
    manifest.descriptors[1].word2 = 11u;
    manifest.descriptors[3].word2 = 100u;
    manifest.descriptors[4].word2 = 11u;

    memset(raw_track, 0, sizeof(raw_track));
    for (index = 0u; index < 40u; ++index) {
        uint8_t *sector = raw_track + index * 2352u;
        size_t byte_index;

        sector[0] = 0x00u;
        for (byte_index = 1u; byte_index < 11u; ++byte_index) {
            sector[byte_index] = 0xffu;
        }
        sector[11] = 0x00u;
        sector[15] = 0x01u;
    }
    for (index = 0u; index < 2048u; ++index) {
        raw_track[21u * 2352u + 16u + index] =
            (uint8_t)(index ^ 0x5au);
    }
    for (index = 0u; index < manifest.descriptor_count; ++index) {
        uint8_t *encoded = raw_track + manifest.user_data_offset + 4u +
            index * 6u;
        PUT_BE16(encoded, manifest.descriptors[index].word0);
        PUT_BE16(encoded + 2u, manifest.descriptors[index].word1);
        PUT_BE16(encoded + 4u, manifest.descriptors[index].word2);
    }

    check(theron_v1_later_record_correlation_from_manifest(
              &manifest, 40u * 2352u, &correlation) &&
              correlation.nonzero_selector_count == 4u &&
              correlation.resolved_selector_count == 3u &&
              correlation.out_of_bounds_selector_count == 1u &&
              correlation.self_reference_proven &&
              correlation.self_resolved_record_in_bounds &&
              correlation.resolved_selector_hash != 0u,
          "bounded descriptor selectors retain only in-range Track02 records");
    check(theron_v1_stage3_descriptor_record_boundary_from_manifest(
              raw_track, sizeof(raw_track), &manifest, 1u, &boundary) &&
              boundary.valid &&
              boundary.descriptor_ordinal == 1u &&
              boundary.descriptor.word2 == 11u &&
              boundary.derived_record_base == 10u &&
              boundary.resolved_track02_record == 21u &&
              boundary.raw_sector == 21u &&
              boundary.raw_offset == 21u * 2352u &&
              boundary.user_data_offset == 21u * 2352u + 16u &&
              boundary.user_data_bytes == 2048u &&
              boundary.user_data_hash != 0u &&
              boundary.descriptor_source_raw_offset ==
                  20u * 2352u + 16u + 4u + 6u &&
              boundary.descriptor_source_bytes == 6u &&
              boundary.descriptor_source_hash != 0u &&
              boundary.descriptor_source_bytes_proven &&
              boundary.selector_occurrence_count == 2u &&
              boundary.selector_first_ordinal == 1u &&
              boundary.selector_last_ordinal == 4u &&
              boundary.selector_row_hash != 0u &&
              boundary.selector_aliases_proven &&
              boundary.record_coordinate_proven &&
              boundary.mode1_user_data_proven &&
              !boundary.descriptor_semantics_proven,
          "descriptor selector binds one exact opaque MODE1 user-data boundary");
    raw_track[21u * 2352u + 15u] = 0x02u;
    check(!theron_v1_stage3_descriptor_record_boundary_from_manifest(
              raw_track, sizeof(raw_track), &manifest, 1u, &boundary),
          "descriptor boundary rejects a non-MODE1 sector");
    raw_track[21u * 2352u + 15u] = 0x01u;
    check(!theron_v1_stage3_descriptor_record_boundary_from_manifest(
              raw_track, sizeof(raw_track), &manifest, 2u, &boundary),
          "descriptor boundary rejects a zero opaque selector");
    {
        Theron_V1Stage3DescriptorCorpusMediaCorrelation corpus;

        check(!theron_v1_stage3_descriptor_corpus_media_correlation_from_manifest(
                  raw_track, sizeof(raw_track), &manifest, &corpus),
              "corpus correlation rejects an out-of-bounds selector");
    }

#undef PUT_BE16
}

static void test_corpus_media_correlation(void) {
    Theron_V1Stage3ManifestEvidence manifest;
    Theron_V1Stage3DescriptorCorpusMediaCorrelation corpus;
    uint8_t raw_track[40u * 2352u];
    size_t index;

#define PUT_BE16(destination, value) \
    do { \
        (destination)[0] = (uint8_t)((value) >> 8u); \
        (destination)[1] = (uint8_t)(value); \
    } while (0)

    memset(&manifest, 0, sizeof(manifest));
    manifest.valid = 1;
    manifest.variant = THERON_TRACK02_VARIANT_US_BIN;
    manifest.track02_record = 20u;
    manifest.raw_sector = 20u;
    manifest.raw_offset = 20u * 2352u;
    manifest.user_data_offset = manifest.raw_offset + 16u;
    manifest.descriptor_count =
        THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT;
    manifest.first_descriptor.word2 = 10u;
    manifest.descriptors[0].word2 = 10u;
    manifest.descriptors[1].word2 = 11u;
    manifest.descriptors[3].word2 = 12u;
    manifest.descriptors[4].word2 = 11u;

    memset(raw_track, 0, sizeof(raw_track));
    for (index = 0u; index < 40u; ++index) {
        uint8_t *sector = raw_track + index * 2352u;
        size_t byte_index;

        sector[0] = 0x00u;
        for (byte_index = 1u; byte_index < 11u; ++byte_index) {
            sector[byte_index] = 0xffu;
        }
        sector[11] = 0x00u;
        sector[15] = 0x01u;
    }
    for (index = 0u; index < manifest.descriptor_count; ++index) {
        uint8_t *encoded = raw_track + manifest.user_data_offset + 4u +
            index * 6u;
        PUT_BE16(encoded, manifest.descriptors[index].word0);
        PUT_BE16(encoded + 2u, manifest.descriptors[index].word1);
        PUT_BE16(encoded + 4u, manifest.descriptors[index].word2);
    }

    check(theron_v1_stage3_descriptor_corpus_media_correlation_from_manifest(
              raw_track, sizeof(raw_track), &manifest, &corpus) &&
              corpus.valid &&
              corpus.variant == THERON_TRACK02_VARIANT_US_BIN &&
              corpus.stage3_track02_record == 20u &&
              corpus.derived_record_base == 10u &&
              corpus.descriptor_count ==
                  THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT &&
              corpus.nonzero_selector_count == 4u &&
              corpus.zero_selector_count ==
                  THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT - 4u &&
              corpus.resolved_record_count == 4u &&
              corpus.distinct_record_count == 3u &&
              corpus.min_resolved_record == 20u &&
              corpus.max_resolved_record == 22u &&
              corpus.resolved_record_hash != 0u &&
              corpus.resolved_user_data_hash != 0u &&
              corpus.corpus_media_proven &&
              !corpus.descriptor_semantics_proven,
          "corpus correlation proves the full synthetic descriptor table");
    raw_track[22u * 2352u + 15u] = 0x02u;
    check(!theron_v1_stage3_descriptor_corpus_media_correlation_from_manifest(
              raw_track, sizeof(raw_track), &manifest, &corpus),
          "corpus correlation rejects a non-MODE1 resolved sector");
    raw_track[22u * 2352u + 15u] = 0x01u;
    raw_track[22u * 2352u + 16u] ^= 0xffu;
    {
        Theron_V1Stage3DescriptorCorpusMediaCorrelation mutated;

        check(theron_v1_stage3_descriptor_corpus_media_correlation_from_manifest(
                  raw_track, sizeof(raw_track), &manifest, &mutated) &&
                  mutated.resolved_user_data_hash !=
                      corpus.resolved_user_data_hash,
              "corpus correlation tracks a changed resolved user byte");
    }
    raw_track[22u * 2352u + 16u] ^= 0xffu;
    manifest.first_descriptor.word2 = 0u;
    check(!theron_v1_stage3_descriptor_corpus_media_correlation_from_manifest(
              raw_track, sizeof(raw_track), &manifest, &corpus),
          "corpus correlation rejects a zero first selector");
    manifest.first_descriptor.word2 = 10u;
    check(!theron_v1_stage3_descriptor_corpus_media_correlation_from_manifest(
              raw_track, sizeof(raw_track) - 2352u + 1u, &manifest, &corpus),
          "corpus correlation rejects non-sector-aligned media");
    {
        Theron_V1Stage3DescriptorRecordSpan span;
        Theron_V1Stage3DescriptorRecordSpan mutated_span;
        Theron_V1Stage3DescriptorCorpusMediaCorrelation unproven_corpus;

        check(theron_v1_stage3_descriptor_corpus_media_correlation_from_manifest(
                  raw_track, sizeof(raw_track), &manifest, &corpus) &&
                  corpus.corpus_media_proven,
              "corpus re-proves after rejection battery");
        check(theron_v1_stage3_descriptor_record_span_from_corpus(
                  &manifest, &corpus, &span) &&
                  span.valid &&
                  span.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  span.stage3_track02_record == 20u &&
                  span.derived_record_base == 10u &&
                  span.referenced_record_count == 3u &&
                  span.min_referenced_record == 20u &&
                  span.max_referenced_record == 22u &&
                  span.span_record_slots == 3u &&
                  span.unreferenced_slot_count == 0u &&
                  span.slot_flag_hash != 0u &&
                  span.span_topology_proven &&
                  !span.descriptor_semantics_proven,
              "span topology derives the synthetic referenced-record set");
        check(theron_v1_stage3_descriptor_record_span_contains(&span, 20u) &&
                  theron_v1_stage3_descriptor_record_span_contains(&span, 21u) &&
                  theron_v1_stage3_descriptor_record_span_contains(&span, 22u) &&
                  !theron_v1_stage3_descriptor_record_span_contains(&span, 19u) &&
                  !theron_v1_stage3_descriptor_record_span_contains(&span, 23u),
              "span membership answers only referenced records");
        memset(&unproven_corpus, 0, sizeof(unproven_corpus));
        check(!theron_v1_stage3_descriptor_record_span_from_corpus(
                  &manifest, &unproven_corpus, &mutated_span),
              "span topology rejects an unproven corpus");
        manifest.track02_record = 21u;
        check(!theron_v1_stage3_descriptor_record_span_from_corpus(
                  &manifest, &corpus, &mutated_span),
              "span topology rejects a mismatched manifest record");
        manifest.track02_record = 20u;
        manifest.descriptors[3].word2 = 11u;
        check(!theron_v1_stage3_descriptor_record_span_from_corpus(
                  &manifest, &corpus, &mutated_span),
              "span topology rejects a distinct-count drift");
        manifest.descriptors[3].word2 = 12u;
        memset(&mutated_span, 0, sizeof(mutated_span));
        check(!theron_v1_stage3_descriptor_record_span_contains(
                  &mutated_span, 20u),
              "span membership rejects an invalid span");
    }

#undef PUT_BE16
}

static uint8_t *read_file_bytes(const char *path, size_t *out_size) {
    FILE *file = NULL;
    long size;
    uint8_t *bytes = NULL;

    if (!path || !out_size || !(file = fopen(path, "rb")) ||
        fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (uint8_t *)malloc((size_t)size)) ||
        fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        if (file) fclose(file);
        free(bytes);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return bytes;
}

static int inspect(const char *path,
                   const char *md5_hex,
                   Theron_V1LaterRecordCorrelation *out_correlation) {
    Theron_Track02Stage2DynamicPayloadReceipt payload;
    Theron_V1Stage3ManifestEvidence manifest;
    uint8_t *bytes;
    size_t size;
    char actual_md5[33];
    int ok;

    bytes = read_file_bytes(path, &size);
    if (!bytes) return 0;
    ok = m12_file_md5_hex(path, actual_md5) &&
        strcmp(actual_md5, md5_hex) == 0 &&
        theron_v1_track02_inspect_stage2_dynamic_payload(
            bytes, size, md5_hex, &payload) == THERON_TRACK02_SIGNAL_OK &&
        theron_v1_stage3_manifest_evidence_from_payload(
            bytes, size, &payload, &manifest) &&
        theron_v1_later_record_correlation_from_manifest(
            &manifest, size, out_correlation);
    free(bytes);
    return ok;
}

static int inspect_corpus(
    const char *path,
    const char *md5_hex,
    Theron_V1Stage3ManifestEvidence *out_manifest,
    Theron_V1Stage3DescriptorCorpusMediaCorrelation *out_corpus) {
    Theron_Track02Stage2DynamicPayloadReceipt payload;
    uint8_t *bytes;
    size_t size;
    char actual_md5[33];
    int ok;

    bytes = read_file_bytes(path, &size);
    if (!bytes) return 0;
    ok = m12_file_md5_hex(path, actual_md5) &&
        strcmp(actual_md5, md5_hex) == 0 &&
        theron_v1_track02_inspect_stage2_dynamic_payload(
            bytes, size, md5_hex, &payload) == THERON_TRACK02_SIGNAL_OK &&
        theron_v1_stage3_manifest_evidence_from_payload(
            bytes, size, &payload, out_manifest) &&
        theron_v1_stage3_descriptor_corpus_media_correlation_from_manifest(
            bytes, size, out_manifest, out_corpus);
    free(bytes);
    return ok;
}

int main(void) {
    const char *jp_path = getenv("FIRESTAFF_THERON_TRACK02_JP_BIN");
    const char *us_path = getenv("FIRESTAFF_THERON_TRACK02_US_BIN");
    Theron_V1LaterRecordCorrelation jp;
    Theron_V1LaterRecordCorrelation us;
    Theron_V1Stage3ManifestEvidence us_manifest;
    Theron_V1Stage3DescriptorCorpusMediaCorrelation us_corpus;
    Theron_V1Stage3DescriptorRecordSpan us_span;
    Theron_V1LaterRecordCorrelationComparison comparison;
    int have_jp = 0;
    int have_us = 0;

    test_bounded_selector_catalog();
    test_corpus_media_correlation();

    if (jp_path) {
        have_jp = 1;
        check(inspect(jp_path, THERON_TRACK02_MD5_JP_BIN, &jp),
              "JP raw Track02 establishes later-record self correlation");
        check(jp.valid && jp.stage3_track02_record == 0x0004dfu &&
                  jp.first_descriptor_selector == 0x000au &&
                  jp.derived_record_base == 0x0004d5u &&
                  jp.self_reference_proven &&
                  jp.self_resolved_record_in_bounds &&
                  jp.nonzero_selector_count == 214u &&
                  jp.resolved_selector_count +
                      jp.out_of_bounds_selector_count ==
                      jp.nonzero_selector_count,
              "JP first opaque selector resolves to its proven stage-three sector");
    }
    if (us_path) {
        have_us = 1;
        check(inspect(us_path, THERON_TRACK02_MD5_US_BIN, &us),
              "US raw Track02 establishes later-record self correlation");
        check(us.valid && us.stage3_track02_record == 0x0004e0u &&
                  us.first_descriptor_selector == 0x000au &&
                  us.derived_record_base == 0x0004d6u &&
                  us.self_reference_proven &&
                  us.self_resolved_record_in_bounds &&
                  us.nonzero_selector_count == 216u &&
                  us.resolved_selector_count +
                      us.out_of_bounds_selector_count ==
                      us.nonzero_selector_count,
              "US first opaque selector resolves to its proven stage-three sector");
        check(inspect_corpus(us_path, THERON_TRACK02_MD5_US_BIN, &us_manifest,
                             &us_corpus),
              "US raw Track02 proves the full stage-three descriptor corpus");
        check(us_corpus.valid &&
                  us_corpus.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  us_corpus.stage3_track02_record == 0x0004e0u &&
                  us_corpus.derived_record_base == 0x0004d6u &&
                  us_corpus.descriptor_count == 218u &&
                  us_corpus.nonzero_selector_count == 216u &&
                  us_corpus.zero_selector_count == 2u &&
                  us_corpus.resolved_record_count == 216u &&
                  us_corpus.distinct_record_count == 162u &&
                  us_corpus.min_resolved_record == 0x0004d7u &&
                  us_corpus.max_resolved_record == 0x0005d3u &&
                  us_corpus.resolved_record_hash == 0xbd3eeb40u &&
                  us_corpus.resolved_user_data_hash == 0x90c5b97fu &&
                  us_corpus.corpus_media_proven &&
                  !us_corpus.descriptor_semantics_proven,
              "US descriptor corpus resolves 216 selectors to 162 distinct MODE1 records");
        check(theron_v1_stage3_descriptor_record_span_from_corpus(
                  &us_manifest, &us_corpus, &us_span) &&
                  us_span.valid &&
                  us_span.variant == THERON_TRACK02_VARIANT_US_BIN &&
                  us_span.stage3_track02_record == 0x0004e0u &&
                  us_span.derived_record_base == 0x0004d6u &&
                  us_span.referenced_record_count == 162u &&
                  us_span.min_referenced_record == 0x0004d7u &&
                  us_span.max_referenced_record == 0x0005d3u &&
                  us_span.span_record_slots == 253u &&
                  us_span.unreferenced_slot_count == 91u &&
                  us_span.slot_flag_hash == 0x5634053bu &&
                  us_span.span_topology_proven &&
                  !us_span.descriptor_semantics_proven,
              "US span topology proves 162 referenced records across 253 slots");
        check(theron_v1_stage3_descriptor_record_span_contains(
                  &us_span, 0x0004e0u) &&
                  theron_v1_stage3_descriptor_record_span_contains(
                      &us_span, 0x0004d7u) &&
                  theron_v1_stage3_descriptor_record_span_contains(
                      &us_span, 0x0005d3u) &&
                  !theron_v1_stage3_descriptor_record_span_contains(
                      &us_span, 0x0004deu) &&
                  !theron_v1_stage3_descriptor_record_span_contains(
                      &us_span, 0x0004e1u) &&
                  !theron_v1_stage3_descriptor_record_span_contains(
                      &us_span, 0x0004d6u) &&
                  !theron_v1_stage3_descriptor_record_span_contains(
                      &us_span, 0x0005d4u) &&
                  !theron_v1_stage3_descriptor_record_span_contains(
                      &us_span, 0x000b52u),
              "US span membership includes the self record and rejects gaps/outside");
    }
    if (!have_jp && !have_us) {
        ++g_skip;
        printf("[SKIP] set FIRESTAFF_THERON_TRACK02_JP_BIN and FIRESTAFF_THERON_TRACK02_US_BIN\n");
        printf("--- %d failed, %d skipped ---\n", g_fail, g_skip);
        return 0;
    }
    if (have_jp && have_us) {
        check(theron_v1_later_record_correlation_compare(&jp, &us,
                                                         &comparison) &&
                  comparison.valid &&
                  comparison.shared_first_selector == 0x000au &&
                  comparison.first_base == 0x0004d5u &&
                  comparison.second_base == 0x0004d6u &&
                  comparison.base_delta == 1u &&
                  comparison.both_self_references_proven,
              "JP/US later-record coordinate bases differ only with stage-three shift");
    } else {
        ++g_skip;
        printf("[SKIP] JP/US comparison needs both Track02 variants staged\n");
    }
    printf("--- %d failed, %d skipped ---\n", g_fail, g_skip);
    return g_fail ? 1 : 0;
}
