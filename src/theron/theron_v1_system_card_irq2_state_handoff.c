#include "theron_v1_system_card_irq2_state_handoff.h"

#include <string.h>

#define THERON_V1_SYSCARD3_ROM_MD5 "ff1a674273fe3540ccef576376407d1d"
#define THERON_V1_SYSCARD3_ROM_BYTES 0x40200u
#define THERON_V1_SYSCARD3_IRQ2_VECTOR_OFFSET 0x1ff6u
#define THERON_V1_SYSCARD3_IRQ2_HANDLER_OFFSET 0x04beu

int theron_v1_system_card_irq2_state_handoff_from_original_media(
    const Theron_Track02Stage2DynamicPayloadReceipt *payload,
    const uint8_t *system_card_rom,
    size_t system_card_rom_size,
    const char *system_card_rom_md5_hex,
    Theron_V1SystemCardIrq2StateHandoff *out_handoff) {
    static const uint8_t handler_entry[] = {
        0x22u,             /* SAX: the stage-two X value becomes A. */
        0xc9u, 0x0au,      /* CMP #$0a */
        0x90u, 0xcfu,      /* BCC $4492 */
        0xc9u, 0x23u,      /* CMP #$23 */
        0xb0u, 0xcbu       /* BCS $4492 */
    };

    if (!out_handoff) return 0;
    memset(out_handoff, 0, sizeof(*out_handoff));
    if (!payload || !payload->valid ||
        (payload->variant != THERON_TRACK02_VARIANT_JP_BIN &&
         payload->variant != THERON_TRACK02_VARIANT_US_BIN) ||
        payload->header_word0 != 0x00ffu ||
        !system_card_rom || !system_card_rom_md5_hex ||
        strcmp(system_card_rom_md5_hex, THERON_V1_SYSCARD3_ROM_MD5) != 0 ||
        system_card_rom_size != THERON_V1_SYSCARD3_ROM_BYTES ||
        THERON_V1_SYSCARD3_IRQ2_HANDLER_OFFSET + sizeof(handler_entry) >
            system_card_rom_size ||
        system_card_rom[THERON_V1_SYSCARD3_IRQ2_VECTOR_OFFSET] != 0xbeu ||
        system_card_rom[THERON_V1_SYSCARD3_IRQ2_VECTOR_OFFSET + 1u] != 0x44u ||
        memcmp(system_card_rom + THERON_V1_SYSCARD3_IRQ2_HANDLER_OFFSET,
               handler_entry, sizeof(handler_entry)) != 0) {
        return 0;
    }

    out_handoff->valid = 1;
    out_handoff->variant = payload->variant;
    out_handoff->stage3_track02_record = payload->track02_record;
    /* Theron stage two: LDA $278A / LDX $278B / LDY $278C / INY at $4090. */
    out_handoff->stage2_a_source_address = 0x278au;
    out_handoff->stage2_x_source_address = 0x278bu;
    out_handoff->stage2_y_source_address = 0x278cu;
    out_handoff->stage2_increments_y = 1;
    out_handoff->irq2_vector_address = 0xfff6u;
    out_handoff->irq2_handler_address = 0x44beu;
    out_handoff->handler_swaps_a_x = 1;
    out_handoff->stage2_x_becomes_handler_a = 1;
    out_handoff->handler_a_minimum = 0x0au;
    out_handoff->handler_a_maximum = 0x22u;
    out_handoff->handler_out_of_range_branch = 0x4492u;
    return 1;
}
