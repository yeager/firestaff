#include "theron_v1_system_card_irq2_cd_state_gate.h"

#include <string.h>

#define THERON_V1_SYSCARD3_ROM_MD5 "ff1a674273fe3540ccef576376407d1d"
#define THERON_V1_SYSCARD3_CONTAINER_HEADER_BYTES 0x0200u
#define THERON_V1_SYSCARD3_ROM_BYTES 0x40200u
#define THERON_V1_SYSCARD3_IRQ2_HANDLER_OFFSET 0x0736u
#define THERON_V1_SYSCARD3_IRQ2_CLEAR_PATH_OFFSET 0x0742u

int theron_v1_system_card_irq2_cd_state_gate_from_original_media(
    const Theron_Track02Stage2DynamicPayloadReceipt *payload,
    const uint8_t *system_card_rom,
    size_t system_card_rom_size,
    const char *system_card_rom_md5_hex,
    Theron_V1SystemCardIrq2CdStateGate *out_gate) {
    static const uint8_t entry_clear_path[] = {
        0x44u, 0x04u /* BSR $E742 */
    };
    static const uint8_t cd_state_gate[] = {
        0xadu, 0x02u, 0x18u, /* LDA $1802 */
        0x2du, 0x03u, 0x18u, /* AND $1803 */
        0x05u, 0xf2u,       /* ORA $F2 */
        0x85u, 0xf2u,       /* STA $F2 */
        0x2fu, 0xf2u, 0x64u /* BBR2 $F2,$E7B3 */
    };
    const size_t handler_offset = THERON_V1_SYSCARD3_CONTAINER_HEADER_BYTES +
        THERON_V1_SYSCARD3_IRQ2_HANDLER_OFFSET;
    const size_t clear_path_offset =
        THERON_V1_SYSCARD3_CONTAINER_HEADER_BYTES +
        THERON_V1_SYSCARD3_IRQ2_CLEAR_PATH_OFFSET;

    if (!out_gate) return 0;
    memset(out_gate, 0, sizeof(*out_gate));
    if (!payload || !payload->valid ||
        (payload->variant != THERON_TRACK02_VARIANT_JP_BIN &&
         payload->variant != THERON_TRACK02_VARIANT_US_BIN) ||
        payload->header_word0 != 0x00ffu || !system_card_rom ||
        !system_card_rom_md5_hex ||
        strcmp(system_card_rom_md5_hex, THERON_V1_SYSCARD3_ROM_MD5) != 0 ||
        system_card_rom_size != THERON_V1_SYSCARD3_ROM_BYTES ||
        handler_offset + 8u > system_card_rom_size ||
        clear_path_offset + sizeof(cd_state_gate) > system_card_rom_size ||
        memcmp(system_card_rom + handler_offset + 6u, entry_clear_path,
               sizeof(entry_clear_path)) != 0 ||
        memcmp(system_card_rom + clear_path_offset, cd_state_gate,
               sizeof(cd_state_gate)) != 0) {
        return 0;
    }

    out_gate->valid = 1;
    out_gate->variant = payload->variant;
    out_gate->stage3_track02_record = payload->track02_record;
    out_gate->handler_address = 0xe736u;
    out_gate->clear_path_address = 0xe742u;
    out_gate->cd_status_low_address = 0x1802u;
    out_gate->cd_status_high_address = 0x1803u;
    out_gate->irq_state_zero_page_address = 0xf2u;
    out_gate->branch_bit = 2u;
    out_gate->branch_when_clear_address = 0xe7b3u;
    out_gate->hardware_state_merged_and_stored = 1;
    out_gate->selected_branch_unobserved = 1;
    return 1;
}
