#include "dm2_v1_runtime.h"
#include "dm2_v1_session_fixture.h"
#include <stdio.h>
#include <string.h>
int main(void) {
    DM2_V1_G1DirectMissileReceipt m = {1,0x3800,0x1234,8,9,7,11};
    DM2_V1_G1FlyingItemSourceReceipt s;
    DM2_V1_G1MissileTimerReceipt timer;
    DM2_V1_RuntimeFlyingItemReceipt r;
    DM2_V1_RuntimeFlyingItemReceipt stored;
    DM2_V1_G1FlyingItemMaterialReceipt material;
    DM2_V1_SessionState session;
    uint8_t raw[4] = {1,2,3,4};
    uint8_t timer_raw[80] = {0};
    timer_raw[7 * 10 + 4] = 0x1d;
    timer_raw[7 * 10 + 5] = 0x22;
    timer_raw[7 * 10 + 8] = 0x00;
    timer_raw[7 * 10 + 9] = 0x08;
    int ok = dm2_v1_g1_flying_item_source_receipt(0x3800,0x0d,2,8,0,3,6,0x40,&s) &&
        dm2_v1_g1_direct_missile_timer_receipt(timer_raw,sizeof(timer_raw),7,&timer) &&
        timer.direction == 2 &&
        dm2_v1_runtime_flying_item_timer_receipt(&m,&s,&timer,&r) && r.valid && r.no_draw && r.timer_receipt_hash;
    timer.timer_index = 8;
    ok &= !dm2_v1_runtime_flying_item_timer_receipt(&m,&s,&timer,&r);
    timer.timer_index = 7;
    timer.direction = 4;
    ok &= !dm2_v1_runtime_flying_item_timer_receipt(&m,&s,&timer,&r);
    timer.direction = 2;
    s.missile_object_id ^= 1; ok &= !dm2_v1_runtime_flying_item_timer_receipt(&m,&s,&timer,&r);
    s.missile_object_id ^= 1;
    memset(&material, 0, sizeof(material));
    material.valid = 1;
    material.source = s;
    material.raw_gfx256_bytes = raw;
    material.raw_gfx256_byte_count = sizeof(raw);
    material.raw_gfx256_hash = 21;
    material.raw_gfx256_receipt_hash = 22;
    material.local_palette_hash = 23;
    material.clip_rect_id = s.clip_rect_id;
    material.raw4_hash = 24;
    material.raw4_receipt_hash = 25;
    material.identity_hash = 26;
    ok &= dm2_v1_runtime_flying_item_timer_receipt(&m,&s,&timer,&r) &&
        dm2_v1_runtime_flying_item_material_receipt(&r,&material,&r) &&
        r.no_draw && r.raw_gfx256_hash == 21 &&
        r.raw_gfx256_receipt_hash == 22 && r.palette_hash == 23 &&
        r.raw4_hash == 24 && r.raw4_receipt_hash == 25;
    material.raw4_receipt_hash = 0;
    ok &= !dm2_v1_runtime_flying_item_material_receipt(&r,&material,&r);
    material.raw4_receipt_hash = 25;
    material.source.identity_hash ^= 1;
    ok &= !dm2_v1_runtime_flying_item_material_receipt(&r,&material,&r);
    dm2_v1_test_session_fixture_new(&session);
    session.original_timer_count = 8;
    memcpy(&session.original_timers[7], timer_raw + 70, DM2_TIMER_ENTRY_SIZE);
    ok &= dm2_v1_runtime_flying_item_timer_from_session(&session,&m,&s,&r) &&
        r.valid && r.no_draw;
    session.original_timer_count = 7;
    ok &= !dm2_v1_runtime_flying_item_timer_from_session(&session,&m,&s,&r);
    session.original_timer_count = 8;
    material.source = s;
    material.identity_hash = 26;
    ok &= dm2_v1_runtime_admit_flying_item_material(
        &session,&m,&s,&material) &&
        dm2_v1_runtime_last_flying_item_receipt(&stored) && stored.valid &&
        stored.no_draw && stored.raw_gfx256_hash == material.raw_gfx256_hash;
    material.raw_gfx256_hash = 0;
    ok &= !dm2_v1_runtime_admit_flying_item_material(&session,&m,&s,&material) &&
        !dm2_v1_runtime_last_flying_item_receipt(&stored);
    if (!ok) fputs("flying item timer receipt failed\n",stderr);
    return ok ? 0 : 1;
}
