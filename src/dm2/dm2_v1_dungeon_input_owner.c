#include "dm2_v1_dungeon_input_owner.h"

#include <string.h>

static uint32_t dm2_v1_dungeon_input_table_hash(void)
{
    uint32_t hash = 2166136261u;
    Dm2TouchClickZonePc34Compat zone;
    unsigned int i;

    for (i = 0; i < DM2_TOUCHCLICK_Compat_GetViewZoneCount(
                    DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT); ++i) {
        if (!DM2_TOUCHCLICK_Compat_GetViewZone(
                DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT, i, &zone))
            return 0u;
        const uint32_t values[] = {
            zone.commandId, zone.zoneIndex, zone.buttonMask,
            (uint16_t)zone.x, (uint16_t)zone.y,
            (uint16_t)zone.w, (uint16_t)zone.h
        };
        for (size_t j = 0; j < sizeof(values) / sizeof(values[0]); ++j) {
            const uint8_t *bytes = (const uint8_t *)&values[j];
            for (size_t k = 0; k < sizeof(values[j]); ++k) {
                hash ^= bytes[k];
                hash *= 16777619u;
            }
        }
    }
    return hash;
}

int dm2_v1_dungeon_input_owner_init(DM2_V1_DungeonInputOwner *owner,
                                    const char *graphics_md5)
{
    if (!owner) return 0;
    memset(owner, 0, sizeof(*owner));
    if (!graphics_md5 ||
        strcmp(graphics_md5, DM2_V1_DUNGEON_INPUT_PC_EN_GRAPHICS_MD5) != 0)
        return 0;

    owner->source_table_hash = dm2_v1_dungeon_input_table_hash();
    if (owner->source_table_hash == 0u) return 0;
    memcpy(owner->graphics_md5, graphics_md5, sizeof(owner->graphics_md5));
    owner->active = 1;
    return 1;
}

int dm2_v1_dungeon_input_owner_route(
    const DM2_V1_DungeonInputOwner *owner,
    int16_t screen_x,
    int16_t screen_y,
    unsigned int button_mask,
    DM2_V1_DungeonInputEventSink sink,
    void *sink_ctx,
    DM2_V1_DungeonInputReceipt *out_receipt)
{
    Dm2TouchClickZonePc34Compat zone;
    DM2_V1_DungeonInputReceipt receipt;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!owner || !owner->active ||
        strcmp(owner->graphics_md5,
               DM2_V1_DUNGEON_INPUT_PC_EN_GRAPHICS_MD5) != 0) {
        receipt.blocked_unverified_graphics = 1;
        *out_receipt = receipt;
        return 0;
    }
    receipt.source_table_hash = owner->source_table_hash;
    if (!DM2_TOUCHCLICK_Compat_HitTestInView(
            DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT,
            screen_x, screen_y, button_mask, &zone)) {
        receipt.blocked_no_source_zone = 1;
        *out_receipt = receipt;
        return 0;
    }

    receipt.accepted = 1;
    receipt.event_index = (uint16_t)zone.commandId;
    receipt.source_zone_index = (uint16_t)zone.zoneIndex;
    receipt.source_x = (int16_t)zone.x;
    receipt.source_y = (int16_t)zone.y;
    receipt.source_w = (int16_t)zone.w;
    receipt.source_h = (int16_t)zone.h;
    if (sink) sink(sink_ctx, (int16_t)zone.commandId, screen_x, screen_y);
    *out_receipt = receipt;
    return 1;
}
