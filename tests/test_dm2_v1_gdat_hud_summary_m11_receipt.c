#include "dm2_v1_gdat_hud_m11_command.h"

#include <stdio.h>
#include <string.h>

static uint32_t hash_bytes(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;
    for (i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

int main(void)
{
    DM2_V1_GdatHudM11CommandPlan plan;
    DM2_V1_GdatHudSummaryM11Receipt receipt;
    DM2_V1_GdatHudPicstTransformReceipt transform;
    DM2_V1_GdatHudPicstDrawReceipt draw;
    uint8_t pixels[64u * 64u];
    uint8_t target[40u * 60u];
    uint8_t palette[16];
    uint8_t source_bytes[1] = { 0xa5u };
    int x;
    int y;
    int ok;

    for (y = 0; y < 64; ++y)
        for (x = 0; x < 64; ++x)
            pixels[y * 64 + x] = (uint8_t)((x + y * 3) & 0x0fu);
    for (x = 0; x < 16; ++x) palette[x] = (uint8_t)(0x30 + x);

    memset(&plan, 0, sizeof(plan));
    plan.valid = 1; plan.command_count = 1; plan.command_hash = 0x1122u;
    plan.commands[0].gdat_category = 1;
    plan.commands[0].gdat_index = 5;
    plan.commands[0].gdat_field = 13;
    plan.commands[0].pixels = pixels;
    plan.commands[0].width = 64; plan.commands[0].height = 64;
    plan.commands[0].format = DM2_IMG_FMT_U4;
    memcpy(plan.commands[0].palette16, palette, sizeof(palette));
    plan.commands[0].decoded_hash = hash_bytes(pixels, sizeof(pixels));
    plan.commands[0].palette_hash = hash_bytes(palette, sizeof(palette));
    plan.commands[0].raw_hash = 0x3344u;
    plan.commands[0].material_source_bytes = source_bytes;
    plan.commands[0].material_source_byte_count = sizeof(source_bytes);
    plan.commands[0].material_receipt_hash = 0x7788u;
    plan.commands[0].destination_rect_id = 57u;
    plan.commands[0].destination_table_hash = 0x99aau;
    plan.commands[0].destination.x = 1; plan.commands[0].destination.y = 2;
    plan.commands[0].destination.w = 31; plan.commands[0].destination.h = 53;

    ok = dm2_v1_gdat_hud_summary_m11_receipt(&plan, 5, 13, &receipt) &&
        receipt.valid && receipt.no_draw && receipt.category == 1u &&
        receipt.index == 5u && receipt.field == 13u &&
        receipt.decoded_hash == plan.commands[0].decoded_hash &&
        receipt.palette_hash == plan.commands[0].palette_hash &&
        receipt.destination_rect_id == 57u && receipt.destination_hash == 0x99aau;
    ok &= !dm2_v1_gdat_hud_summary_m11_receipt(&plan, 6, 13, &receipt);
    ok &= dm2_v1_gdat_hud_summary_m11_receipt(&plan, 5, 13, &receipt) &&
        dm2_v1_gdat_hud_picst_transform_receipt(&receipt, 15, &transform) &&
        transform.valid && transform.no_draw && transform.scale_x == 0x1fu &&
        transform.scale_y == 0x35u && transform.destination_rect_id == 57u;

    memset(target, 0xee, sizeof(target));
    memset(palette, 0, sizeof(palette));
    ok &= dm2_v1_gdat_hud_picst_draw_indexed(&plan, &receipt, &transform, 5, 13,
        target, 40, 60, palette, &draw) && draw.valid && draw.drawn &&
        draw.width == 31u && draw.height == 53u &&
        draw.destination_rect_id == 57u &&
        memcmp(palette, plan.commands[0].palette16, sizeof(palette)) == 0 &&
        target[2 * 40 + 1] == pixels[1] &&
        target[(2 + 52) * 40 + 1 + 30] == pixels[63 * 64 + 62] &&
        target[0] == 0xeeu && target[2 * 40] == 0xeeu;

    /* Changing any receipt material must fail before writing a substitute. */
    pixels[0] ^= 1u;
    ok &= !dm2_v1_gdat_hud_picst_draw_indexed(&plan, &receipt, &transform, 5, 13,
        target, 40, 60, palette, &draw);
    pixels[0] ^= 1u;
    plan.commands[0].palette16[0] ^= 1u;
    ok &= !dm2_v1_gdat_hud_picst_draw_indexed(&plan, &receipt, &transform, 5, 13,
        target, 40, 60, palette, &draw);
    plan.commands[0].palette16[0] ^= 1u;
    ++plan.commands[0].destination.w;
    ok &= !dm2_v1_gdat_hud_picst_draw_indexed(&plan, &receipt, &transform, 5, 13,
        target, 40, 60, palette, &draw);
    --plan.commands[0].destination.w;
    ++transform.summary_identity_hash;
    ok &= !dm2_v1_gdat_hud_picst_draw_indexed(&plan, &receipt, &transform, 5, 13,
        target, 40, 60, palette, &draw);
    --transform.summary_identity_hash;
    ok &= dm2_v1_gdat_hud_picst_transform_receipt(&receipt, 16, &transform) &&
        transform.scale_x == 0x2fu &&
        !dm2_v1_gdat_hud_picst_draw_indexed(&plan, &receipt, &transform, 5, 13,
            target, 40, 60, palette, &draw) &&
        !dm2_v1_gdat_hud_picst_transform_receipt(&receipt, 41, &transform);
    plan.commands[0].palette_hash = 0u;
    ok &= !dm2_v1_gdat_hud_summary_m11_receipt(&plan, 5, 13, &receipt);
    plan.commands[0].palette_hash = hash_bytes(plan.commands[0].palette16, 16u);
    plan.commands[0].destination_table_hash = 0u;
    ok &= !dm2_v1_gdat_hud_summary_m11_receipt(&plan, 5, 13, &receipt);
    if (!ok) fputs("DM2 HUD summary M11 receipt failed\n", stderr);
    return ok ? 0 : 1;
}
