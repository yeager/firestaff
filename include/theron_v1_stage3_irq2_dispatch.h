#ifndef THERON_V1_STAGE3_IRQ2_DISPATCH_H
#define THERON_V1_STAGE3_IRQ2_DISPATCH_H

#include "theron_v1_track02.h"

#include <stddef.h>
#include <stdint.h>

/*
 * Loader-bound entry semantics for the original payload loaded at $3800.
 * Its leading bytes are HuC6280 `BRK $ff`: the immediate is an IRQ2 dispatch
 * selector and the CPU continuation is $3802.  This does not classify the
 * following manifest descriptors or predict later CD reads.
 *
 * Evidence: docs/source-lock/tqr_v1_track02_ipl_loader_2026-07-11.md.
 */

typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t track02_record;
    uint16_t entry_address;
    uint8_t opcode;
    uint8_t irq2_selector;
    uint16_t continuation_address;
    uint16_t irq2_vector_address;
    int irq2_dispatch_proven;
    int manifest_not_linear_cpu_code;
} Theron_V1Stage3Irq2DispatchReceipt;

int theron_v1_stage3_irq2_dispatch_from_dynamic_payload(
    const Theron_Track02Stage2DynamicPayloadReceipt *payload,
    Theron_V1Stage3Irq2DispatchReceipt *out_receipt);

/* Runtime-consumable form of the dispatch receipt.  Unlike the metadata-only
 * helper above, this reads the authenticated MODE1 user-data window and
 * verifies the bytes that stage two transfers to $3800 before exposing BRK
 * $ff dispatch.  It deliberately stops at the IRQ2 boundary: no manifest
 * descriptor, palette, object, level, or subsequent CD request is inferred.
 * ReDMCSB is not applicable here; original evidence is the Theron's Quest
 * stage-two loader at docs/source-lock/theron-disassembly/
 * theron-us-stage2-huc6280.asm:163-181. */
int theron_v1_stage3_irq2_dispatch_from_original_media(
    const uint8_t *track02_data,
    size_t track02_size,
    const Theron_Track02Stage2DynamicPayloadReceipt *payload,
    Theron_V1Stage3Irq2DispatchReceipt *out_receipt);

#endif /* THERON_V1_STAGE3_IRQ2_DISPATCH_H */
