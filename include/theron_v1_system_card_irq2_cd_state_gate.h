#ifndef THERON_V1_SYSTEM_CARD_IRQ2_CD_STATE_GATE_H
#define THERON_V1_SYSTEM_CARD_IRQ2_CD_STATE_GATE_H

#include "theron_v1_track02.h"

#include <stddef.h>
#include <stdint.h>

/*
 * First verified CD-state effect on the clear path of the authentic System
 * Card 3.0 IRQ2 handler. Hardware status determines the branch, so this
 * receipt deliberately exposes no selected branch or later CD request.
 */

typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage3_track02_record;
    uint16_t handler_address;
    uint16_t clear_path_address;
    uint16_t cd_status_low_address;
    uint16_t cd_status_high_address;
    uint8_t irq_state_zero_page_address;
    uint8_t branch_bit;
    uint16_t branch_when_clear_address;
    int hardware_state_merged_and_stored;
    int selected_branch_unobserved;
} Theron_V1SystemCardIrq2CdStateGate;

/* The caller must authenticate the supplied System Card 3.0 ROM. */
int theron_v1_system_card_irq2_cd_state_gate_from_original_media(
    const Theron_Track02Stage2DynamicPayloadReceipt *payload,
    const uint8_t *system_card_rom,
    size_t system_card_rom_size,
    const char *system_card_rom_md5_hex,
    Theron_V1SystemCardIrq2CdStateGate *out_gate);

/* Optional host/emulator continuation of the full Track 02 handoff. The
 * caller supplies an authenticated System Card image; missing system-card
 * media must not weaken ordinary Track 02 boot. */
int theron_v1_system_card_irq2_cd_state_gate_from_full_track02_media(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5_hex,
    const uint8_t *system_card_rom,
    size_t system_card_rom_size,
    const char *system_card_rom_md5_hex,
    Theron_V1SystemCardIrq2CdStateGate *out_gate);

#endif /* THERON_V1_SYSTEM_CARD_IRQ2_CD_STATE_GATE_H */
