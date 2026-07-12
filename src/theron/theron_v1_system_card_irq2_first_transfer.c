#include "theron_v1_system_card_irq2_first_transfer.h"

#include <string.h>

#define THERON_V1_SYSCARD3_ROM_MD5 "ff1a674273fe3540ccef576376407d1d"
#define THERON_V1_SYSCARD3_ROM_BYTES 0x40200u
#define THERON_V1_SYSCARD3_IRQ2_HANDLER_OFFSET 0x04beu

int theron_v1_system_card_irq2_first_transfer_from_original_media(
    const Theron_Track02Stage2DynamicPayloadReceipt *payload,
    const uint8_t *system_card_rom,
    size_t system_card_rom_size,
    const char *system_card_rom_md5_hex,
    Theron_V1SystemCardIrq2FirstTransfer *out_transfer) {
    static const uint8_t handler_first_transfer[] = {
        0x22u,             /* SAX */
        0xc9u, 0x0au,      /* CMP #$0a */
        0x90u, 0xcfu,      /* BCC $4492 */
        0xc9u, 0x23u,      /* CMP #$23 */
        0xb0u, 0xcbu,      /* BCS $4492 */
        0x44u, 0x78u       /* BSR $4541 */
    };

    if (!out_transfer) return 0;
    memset(out_transfer, 0, sizeof(*out_transfer));
    if (!payload || !payload->valid ||
        (payload->variant != THERON_TRACK02_VARIANT_JP_BIN &&
         payload->variant != THERON_TRACK02_VARIANT_US_BIN) ||
        payload->header_word0 != 0x00ffu || !system_card_rom ||
        !system_card_rom_md5_hex ||
        strcmp(system_card_rom_md5_hex, THERON_V1_SYSCARD3_ROM_MD5) != 0 ||
        system_card_rom_size != THERON_V1_SYSCARD3_ROM_BYTES ||
        THERON_V1_SYSCARD3_IRQ2_HANDLER_OFFSET +
                sizeof(handler_first_transfer) > system_card_rom_size ||
        memcmp(system_card_rom + THERON_V1_SYSCARD3_IRQ2_HANDLER_OFFSET,
               handler_first_transfer, sizeof(handler_first_transfer)) != 0) {
        return 0;
    }

    out_transfer->valid = 1;
    out_transfer->variant = payload->variant;
    out_transfer->stage3_track02_record = payload->track02_record;
    out_transfer->handler_address = 0x44beu;
    out_transfer->accepted_a_minimum = 0x0au;
    out_transfer->accepted_a_maximum = 0x22u;
    out_transfer->out_of_range_transfer_address = 0x4492u;
    out_transfer->in_range_transfer_address = 0x4541u;
    out_transfer->selected_transfer_unobserved = 1;
    out_transfer->record_request_unproven = 1;
    return 1;
}
