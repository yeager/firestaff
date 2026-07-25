#include "dm1_v1_resurrection_pc34_compat.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

int dm1_v1_resurrection_materialize_candidate_text_stats_pc34(
    const char *encodedName,
    size_t encodedNameLength,
    const char *encodedTitle,
    size_t encodedTitleLength,
    const char *encodedHealth,
    const char *encodedStamina,
    const char *encodedMana,
    const char *encodedStatistics,
    size_t encodedStatisticsLength,
    int sourceBytesProven,
    char *outName,
    size_t outNameCapacity,
    char *outTitle,
    size_t outTitleCapacity,
    uint16_t outVitals[3],
    uint16_t outStatistics[7]);

static void test_materializes_candidate_text_and_decoded_values(void)
{
    char name[16];
    char title[16];
    uint16_t vitals[3] = { 0, 0, 0 };
    uint16_t statistics[7] = { 0, 0, 0, 0, 0, 0, 0 };
    int rc;
    (void)rc;

    memset(name, 0, sizeof(name));
    memset(title, 0, sizeof(title));

    rc = dm1_v1_resurrection_materialize_candidate_text_stats_pc34(
        "TIGGY", 5,
        "ARCHMASTER", 10,
        "APJA", "ABCD", "AABP",
        "ABACADAEAFAGAH", 14,
        1,
        name, sizeof(name),
        title, sizeof(title),
        vitals,
        statistics);

    assert(rc == 1);
    assert(strcmp(name, "TIGGY") == 0);
    assert(strcmp(title, "ARCHMASTER") == 0);
    assert(vitals[0] == 0x0f90u);
    assert(vitals[1] == 0x0123u);
    assert(vitals[2] == 0x001fu);
    assert(statistics[0] == 0x0001u);
    assert(statistics[1] == 0x0002u);
    assert(statistics[2] == 0x0003u);
    assert(statistics[3] == 0x0004u);
    assert(statistics[4] == 0x0005u);
    assert(statistics[5] == 0x0006u);
    assert(statistics[6] == 0x0007u);
}

static void test_missing_proof_fails_without_mutation(void)
{
    char name[8] = "OLD";
    char title[8] = "TITLE";
    uint16_t vitals[3] = { 11, 22, 33 };
    uint16_t statistics[7] = { 1, 2, 3, 4, 5, 6, 7 };
    int rc;
    (void)rc;

    rc = dm1_v1_resurrection_materialize_candidate_text_stats_pc34(
        "WUUF", 4, "BIKA", 4, "AAAA", "AAAA", "AAAA",
        "AAAAAAAAAAAAAA", 14, 0,
        name, sizeof(name), title, sizeof(title), vitals, statistics);

    assert(rc == 0);
    assert(strcmp(name, "OLD") == 0);
    assert(strcmp(title, "TITLE") == 0);
    assert(vitals[0] == 11 && vitals[1] == 22 && vitals[2] == 33);
    assert(statistics[0] == 1 && statistics[6] == 7);
}

static void test_malformed_encoded_values_fail_without_mutation(void)
{
    char name[8] = "OLD";
    char title[8] = "TITLE";
    uint16_t vitals[3] = { 11, 22, 33 };
    uint16_t statistics[7] = { 1, 2, 3, 4, 5, 6, 7 };
    int rc;
    (void)rc;

    rc = dm1_v1_resurrection_materialize_candidate_text_stats_pc34(
        "WUUF", 4, "BIKA", 4, "AAAZ", "AAAA", "AAAA",
        "AAAAAAAAAAAAAA", 14, 1,
        name, sizeof(name), title, sizeof(title), vitals, statistics);

    assert(rc == 0);
    assert(strcmp(name, "OLD") == 0);
    assert(vitals[0] == 11);

    rc = dm1_v1_resurrection_materialize_candidate_text_stats_pc34(
        "WUUF", 4, "BIKA", 4, "AAAA", "AAAA", "AAAA",
        "AAAAAAAAAAAAAZ", 14, 1,
        name, sizeof(name), title, sizeof(title), vitals, statistics);

    assert(rc == 0);
    assert(strcmp(title, "TITLE") == 0);
    assert(statistics[6] == 7);
}

static void test_text_capacity_fails_before_publication(void)
{
    char name[4] = "OLD";
    char title[8] = "TITLE";
    uint16_t vitals[3] = { 11, 22, 33 };
    uint16_t statistics[7] = { 1, 2, 3, 4, 5, 6, 7 };
    int rc;
    (void)rc;

    rc = dm1_v1_resurrection_materialize_candidate_text_stats_pc34(
        "TIGGY", 5, "BIKA", 4, "AAAA", "AAAA", "AAAA",
        "AAAAAAAAAAAAAA", 14, 1,
        name, sizeof(name), title, sizeof(title), vitals, statistics);

    assert(rc == 0);
    assert(strcmp(name, "OLD") == 0);
    assert(strcmp(title, "TITLE") == 0);
    assert(vitals[0] == 11);
}

int main(void)
{
    test_materializes_candidate_text_and_decoded_values();
    test_missing_proof_fails_without_mutation();
    test_malformed_encoded_values_fail_without_mutation();
    test_text_capacity_fails_before_publication();
    return 0;
}
