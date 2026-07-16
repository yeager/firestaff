#include "dm2_v1_record_name_helper.h"

#include <string.h>

static const char *const dm2_v1_record_names[16] = {
    "DOOR",
    "TELEPORTER",
    "TEXT",
    "ACTUATOR",
    "CREATURE",
    "WEAPON",
    "CLOTHING",
    "SCROLL",
    "POTION",
    "CONTAINER",
    "MISCITEM",
    "MISSILE",
    "CLOUD",
    "UNUSED_D",
    "UNUSED_E",
    "UNUSED_F"
};

void dm2_v1_record_name_receipt_clear(DM2_V1_RecordNameReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

int dm2_v1_getRecordNameOf(uint16_t object_id,
                           DM2_V1_RecordNameReceipt *out_receipt)
{
    uint8_t record_type;

    dm2_v1_record_name_receipt_clear(out_receipt);
    if (!out_receipt) {
        return 0;
    }

    record_type = (uint8_t)((object_id >> 10) & 0x0fu);
    out_receipt->handled = 1;
    out_receipt->source_locked = 1;
    out_receipt->valid = 1;
    out_receipt->object_id = object_id;
    out_receipt->record_type = record_type;
    out_receipt->record_name = dm2_v1_record_names[record_type];
    out_receipt->symbol = "getRecordNameOf";
    out_receipt->source_path = "SKWIN/SkWinCore.cpp:824";
    return 1;
}

const char *dm2_v1_getRecordNameOf_source_evidence(void)
{
    return "skproject SKWIN/SkWinCore.cpp getRecordNameOf:824; "
           "bounded ObjectID record-family naming over the high nibble "
           "without record traversal, GDAT lookup, or fallback content.";
}
