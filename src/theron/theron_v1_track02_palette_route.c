#include <string.h>
#include <stdio.h>

#include "theron_v1_dungeon_handoff.h"
#include "theron_v1_track02_palette_route.h"

static int is_known_raw_track02_md5(const char *md5) {
    return md5 &&
        (strcmp(md5, THERON_V1_TRACK02_MD5_JP_BIN) == 0 ||
         strcmp(md5, THERON_V1_TRACK02_MD5_US_BIN) == 0);
}

int theron_v1_track02_palette_route_validate(
    const Theron_V1Track02PaletteRouteFacts *facts,
    Theron_V1Track02PaletteRouteReceipt *out_receipt) {
    Theron_V1Track02PaletteRouteReceipt receipt = {0};
    const Theron_V1Track02PaletteStoreReceipt *index_store;
    const Theron_V1Track02PaletteStoreReceipt *low_store;
    const Theron_V1Track02PaletteStoreReceipt *high_store;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!facts || !out_receipt || !facts->raw_track02 ||
        facts->raw_track02_bytes == 0u ||
        facts->raw_track02_bytes % THERON_V1_TRACK02_RAW_SECTOR_BYTES != 0u ||
        !is_known_raw_track02_md5(facts->raw_track02_md5) ||
        !theron_v1_track02_raw_bytes_match_md5(facts->raw_track02,
                                                facts->raw_track02_bytes,
                                                facts->raw_track02_md5) ||
        !facts->trace_receipt ||
        !facts->trace_receipt->trace_authenticated ||
        !facts->trace_receipt->dynamic_cd_read_authenticated ||
        !facts->trace_receipt->controller_transition_authenticated ||
        !facts->stores ||
        facts->store_count != THERON_V1_TRACK02_PALETTE_ROUTE_STORE_COUNT) {
        return 0;
    }

    index_store = &facts->stores[0];
    low_store = &facts->stores[1];
    high_store = &facts->stores[2];
    if (!index_store->observed_direct_sta || !low_store->observed_direct_sta ||
        !high_store->observed_direct_sta ||
        index_store->kind != THERON_V1_TRACK02_PALETTE_STORE_INDEX ||
        low_store->kind != THERON_V1_TRACK02_PALETTE_STORE_LOW ||
        high_store->kind != THERON_V1_TRACK02_PALETTE_STORE_HIGH ||
        index_store->address < 0x0402u || index_store->address > 0x0405u ||
        low_store->address < 0x0402u || low_store->address > 0x0405u ||
        high_store->address < 0x0402u || high_store->address > 0x0405u ||
        index_store->program_counter == 0u || low_store->program_counter == 0u ||
        high_store->program_counter == 0u) {
        return 0;
    }

    receipt.accepted = 1;
    receipt.real_cd_verified = 1;
    receipt.render_allowed = 0;
    receipt.track02_variant =
        theron_v1_track02_variant_for_md5(facts->raw_track02_md5);
    if (receipt.track02_variant == THERON_TRACK02_VARIANT_UNKNOWN) {
        return 0;
    }
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             facts->raw_track02_md5);
    receipt.vce_index_address = index_store->address;
    receipt.vce_low_address = low_store->address;
    receipt.vce_high_address = high_store->address;
    receipt.vce_index = index_store->accumulator;
    receipt.vce_low = low_store->accumulator;
    receipt.vce_high = high_store->accumulator;
    receipt.status = "authenticated_direct_vce_store_route_render_blocked";
    *out_receipt = receipt;
    return 1;
}
