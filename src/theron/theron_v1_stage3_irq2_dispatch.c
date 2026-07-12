#include "theron_v1_stage3_irq2_dispatch.h"

#include <string.h>

#define THERON_V1_STAGE3_ENTRY_ADDRESS 0x3800u
#define THERON_V1_HUC6280_BRK_OPCODE 0x00u
#define THERON_V1_STAGE3_IRQ2_SELECTOR 0xffu
#define THERON_V1_HUC6280_IRQ2_VECTOR 0xfff6u

int theron_v1_stage3_irq2_dispatch_from_dynamic_payload(
    const Theron_Track02Stage2DynamicPayloadReceipt *payload,
    Theron_V1Stage3Irq2DispatchReceipt *out_receipt) {

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!payload || !payload->valid ||
        (payload->variant != THERON_TRACK02_VARIANT_JP_BIN &&
         payload->variant != THERON_TRACK02_VARIANT_US_BIN) ||
        payload->user_data_bytes !=
            THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES ||
        payload->header_word0 != 0x00ffu || payload->header_word1 != 0x0308u) {
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->variant = payload->variant;
    out_receipt->track02_record = payload->track02_record;
    out_receipt->entry_address = THERON_V1_STAGE3_ENTRY_ADDRESS;
    out_receipt->opcode = THERON_V1_HUC6280_BRK_OPCODE;
    out_receipt->irq2_selector = THERON_V1_STAGE3_IRQ2_SELECTOR;
    out_receipt->continuation_address = THERON_V1_STAGE3_ENTRY_ADDRESS + 2u;
    out_receipt->irq2_vector_address = THERON_V1_HUC6280_IRQ2_VECTOR;
    out_receipt->irq2_dispatch_proven = 1;
    out_receipt->manifest_not_linear_cpu_code = 1;
    return 1;
}
