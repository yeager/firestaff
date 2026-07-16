#include "dm2_v1_record_name_helper.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_true(int condition, const char *label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    }
}

static void expect_record_name(uint16_t object_id,
                               uint8_t record_type,
                               const char *record_name)
{
    DM2_V1_RecordNameReceipt receipt;

    expect_true(dm2_v1_getRecordNameOf(object_id, &receipt) == 1,
                "getRecordNameOf accepts source ObjectID");
    expect_true(receipt.handled && receipt.source_locked && receipt.valid &&
                    !receipt.blocked &&
                    receipt.object_id == object_id &&
                    receipt.record_type == record_type &&
                    strcmp(receipt.record_name, record_name) == 0 &&
                    strcmp(receipt.symbol, "getRecordNameOf") == 0 &&
                    strcmp(receipt.source_path,
                           "SKWIN/SkWinCore.cpp:824") == 0,
                "getRecordNameOf records family name and provenance");
}

int main(void)
{
    DM2_V1_RecordNameReceipt receipt;

    expect_record_name(0x000cu, 0u, "DOOR");
    expect_record_name(0x0401u, 1u, "TELEPORTER");
    expect_record_name(0x1098u, 4u, "CREATURE");
    expect_record_name(0xd407u, 5u, "WEAPON");
    expect_record_name(0xe408u, 9u, "CONTAINER");
    expect_record_name(0xffffu, 15u, "UNUSED_F");

    expect_true(dm2_v1_getRecordNameOf(0x0000u, NULL) == 0,
                "getRecordNameOf rejects missing receipt");
    dm2_v1_record_name_receipt_clear(&receipt);
    expect_true(!receipt.valid && receipt.record_name == NULL,
                "record-name receipt clear resets output");
    expect_true(strstr(dm2_v1_getRecordNameOf_source_evidence(),
                       "getRecordNameOf:824") != NULL,
                "source evidence names skproject symbol line");

    if (failures) {
        return 1;
    }
    puts("DM2 record-name helper: ok");
    return 0;
}
