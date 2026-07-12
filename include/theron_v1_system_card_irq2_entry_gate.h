#ifndef THERON_V1_SYSTEM_CARD_IRQ2_ENTRY_GATE_H
#define THERON_V1_SYSTEM_CARD_IRQ2_ENTRY_GATE_H

#include "theron_v1_track02.h"

#include <stddef.h>
#include <stdint.h>

/*
 * Read-only entry gate for System Card 3.0's genuine IRQ2 handler. The live
 * F5 bit 0 value after CD_READ is not inferred, so this receipt never selects
 * either branch and never attributes a later Track 02 request or game role.
 */

typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage3_track02_record;
    uint16_t irq2_vector_address;
    uint16_t irq2_handler_address;
    uint8_t gate_zero_page_address;
    uint8_t gate_bit;
    uint16_t set_branch_indirect_vector_address;
    uint16_t clear_path_subroutine_address;
    int clear_path_saves_a_x_y;
    int clear_path_returns_via_rti;
    int selected_branch_unobserved;
} Theron_V1SystemCardIrq2EntryGate;

/* The caller must authenticate the supplied System Card 3.0 ROM. */
int theron_v1_system_card_irq2_entry_gate_from_original_media(
    const Theron_Track02Stage2DynamicPayloadReceipt *payload,
    const uint8_t *system_card_rom,
    size_t system_card_rom_size,
    const char *system_card_rom_md5_hex,
    Theron_V1SystemCardIrq2EntryGate *out_gate);

#endif /* THERON_V1_SYSTEM_CARD_IRQ2_ENTRY_GATE_H */
