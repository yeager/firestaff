#include "theron_v1_system_card_irq2_entry_gate.h"

#include <string.h>

#define THERON_V1_SYSCARD3_ROM_MD5 "ff1a674273fe3540ccef576376407d1d"
#define THERON_V1_SYSCARD3_CONTAINER_HEADER_BYTES 0x0200u
#define THERON_V1_SYSCARD3_ROM_BYTES 0x40200u
#define THERON_V1_SYSCARD3_IRQ2_VECTOR_OFFSET 0x1ff6u
#define THERON_V1_SYSCARD3_IRQ2_HANDLER_OFFSET 0x0736u

int theron_v1_system_card_irq2_entry_gate_from_original_media(
    const Theron_Track02Stage2DynamicPayloadReceipt *payload,
    const uint8_t *system_card_rom,
    size_t system_card_rom_size,
    const char *system_card_rom_md5_hex,
    Theron_V1SystemCardIrq2EntryGate *out_gate) {
    static const uint8_t entry_bytes[] = {
        0x8fu, 0xf5u, 0xfau, /* BBS0 $F5,$E733 */
        0x48u, 0xdau, 0x5au, /* PHA / PHX / PHY */
        0x44u, 0x04u,       /* BSR $E742 */
        0x7au, 0xfau, 0x68u, 0x40u /* PLY / PLX / PLA / RTI */
    };
    const size_t vector_offset = THERON_V1_SYSCARD3_CONTAINER_HEADER_BYTES +
        THERON_V1_SYSCARD3_IRQ2_VECTOR_OFFSET;
    const size_t handler_offset = THERON_V1_SYSCARD3_CONTAINER_HEADER_BYTES +
        THERON_V1_SYSCARD3_IRQ2_HANDLER_OFFSET;

    if (!out_gate) return 0;
    memset(out_gate, 0, sizeof(*out_gate));
    if (!payload || !payload->valid ||
        (payload->variant != THERON_TRACK02_VARIANT_JP_BIN &&
         payload->variant != THERON_TRACK02_VARIANT_US_BIN) ||
        payload->header_word0 != 0x00ffu || !system_card_rom ||
        !system_card_rom_md5_hex ||
        strcmp(system_card_rom_md5_hex, THERON_V1_SYSCARD3_ROM_MD5) != 0 ||
        system_card_rom_size != THERON_V1_SYSCARD3_ROM_BYTES ||
        vector_offset + 2u > system_card_rom_size ||
        handler_offset + sizeof(entry_bytes) > system_card_rom_size ||
        system_card_rom[vector_offset] != 0x36u ||
        system_card_rom[vector_offset + 1u] != 0xe7u ||
        memcmp(system_card_rom + handler_offset, entry_bytes,
               sizeof(entry_bytes)) != 0) {
        return 0;
    }

    out_gate->valid = 1;
    out_gate->variant = payload->variant;
    out_gate->stage3_track02_record = payload->track02_record;
    out_gate->irq2_vector_address = 0xfff6u;
    out_gate->irq2_handler_address = 0xe736u;
    out_gate->gate_zero_page_address = 0xf5u;
    out_gate->gate_bit = 0u;
    out_gate->set_branch_indirect_vector_address = 0x2200u;
    out_gate->clear_path_subroutine_address = 0xe742u;
    out_gate->clear_path_saves_a_x_y = 1;
    out_gate->clear_path_returns_via_rti = 1;
    out_gate->selected_branch_unobserved = 1;
    return 1;
}
