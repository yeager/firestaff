#include "nexus_v1_engine.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static Nexus_V1_MenuBpkRendererHandoffReceipt handoff_for(
    const Nexus_V1_Engine *engine)
{
    Nexus_V1_MenuBpkRendererHandoffReceipt receipt;
    memset(&receipt, 0xff, sizeof(receipt));
    expect(nexus_v1_menu_bpk_renderer_handoff_receipt(engine, &receipt) == 0,
           "handoff receipt call succeeds");
    return receipt;
}

static void check_missing_engine_blocks(void)
{
    Nexus_V1_Engine engine;
    Nexus_V1_MenuBpkRendererHandoffReceipt receipt;

    memset(&engine, 0, sizeof(engine));
    receipt = handoff_for(&engine);

    expect(receipt.status == NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_MISSING,
           "missing MENU.BPK receipt stays missing");
    expect(receipt.decode_route == NEXUS_V1_BPK_DECODE_ROUTE_INVALID,
           "missing MENU.BPK receipt has invalid decode route");
    expect(receipt.blocks_real_menu_surface_render,
           "missing MENU.BPK receipt blocks real menu render");
    expect(!receipt.fallback_visuals_permitted,
           "missing MENU.BPK receipt does not permit fallback visuals");
}

static void check_decode_route(Nexus_V1_BpkRuntimeDecodeRoute route,
                               Nexus_V1_MenuBpkRendererHandoffStatus status,
                               int can_render,
                               int blocks_render)
{
    Nexus_V1_Engine engine;
    Nexus_V1_MenuBpkRendererHandoffReceipt receipt;

    memset(&engine, 0, sizeof(engine));
    engine.menu_bpk_decode_receipt_attempted = 1;
    engine.menu_bpk_source.canonical_hash_verified = 1;
    engine.menu_bpk_decode_receipt_valid = 1;
    engine.menu_bpk_decode_receipt.route = route;
    engine.menu_bpk_decode_receipt.archive_entries = 163;
    engine.menu_bpk_decode_receipt.surface_entries = 162;
    engine.menu_bpk_decode_receipt.ready_stored_surfaces =
        can_render ? 162U : 0U;
    engine.menu_bpk_decode_receipt.blocked_prs3_surfaces =
        route == NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3 ? 162U : 0U;

    receipt = handoff_for(&engine);

    expect(receipt.attempted == 1, "attempt flag is preserved");
    expect(receipt.receipt_valid == 1, "valid receipt flag is preserved");
    expect(receipt.canonical_source_hash_verified == 1,
           "MENU.BPK routes require the canonical source hash");
    expect(receipt.status == status, "decode route maps to handoff status");
    expect(receipt.archive_entries == 163,
           "archive entry count is preserved");
    expect(receipt.surface_entries == 162,
           "surface entry count is preserved");
    expect(receipt.can_render_stored_surfaces == can_render,
           "renderable stored-surface flag follows the route");
    expect(receipt.blocks_real_menu_surface_render == blocks_render,
           "MENU.BPK routes expose the strict render blocker");
    expect(!receipt.fallback_visuals_permitted,
           "MENU.BPK renderer handoff never admits synthetic fallback");
}

static void check_unverified_source_blocks_route(void)
{
    Nexus_V1_Engine engine;
    Nexus_V1_MenuBpkRendererHandoffReceipt receipt;

    memset(&engine, 0, sizeof(engine));
    engine.menu_bpk_decode_receipt_attempted = 1;
    engine.menu_bpk_source.exact_source_entry_observed = 1;
    engine.menu_bpk_decode_receipt_valid = 1;
    engine.menu_bpk_decode_receipt.route =
        NEXUS_V1_BPK_DECODE_ROUTE_READY_STORED;
    engine.menu_bpk_decode_receipt.ready_stored_surfaces = 1;

    receipt = handoff_for(&engine);

    expect(receipt.status == NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_SOURCE,
           "parseable MENU.BPK route blocks until source hash is verified");
    expect(!receipt.receipt_valid && !receipt.canonical_source_hash_verified,
           "unverified MENU.BPK route cannot publish a valid receipt");
    expect(receipt.blocks_real_menu_surface_render,
           "unverified MENU.BPK route blocks real menu render");
    expect(!receipt.fallback_visuals_permitted,
           "unverified MENU.BPK route still forbids fallback visuals");
}

int main(void)
{
    check_missing_engine_blocks();
    check_unverified_source_blocks_route();
    check_decode_route(NEXUS_V1_BPK_DECODE_ROUTE_INVALID,
                       NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_INVALID, 0, 0);
    check_decode_route(NEXUS_V1_BPK_DECODE_ROUTE_NO_SURFACES,
                       NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_NO_SURFACES, 0, 0);
    check_decode_route(NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3,
                       NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_PRS3, 0, 1);
    check_decode_route(NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_TRUNCATED,
                       NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_TRUNCATED,
                       0, 1);
    check_decode_route(NEXUS_V1_BPK_DECODE_ROUTE_READY_STORED,
                       NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_STORED, 1, 0);
    check_decode_route(NEXUS_V1_BPK_DECODE_ROUTE_READY_DECODED,
                       NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_DECODED, 1, 0);

    if (failures) {
        fprintf(stderr,
                "Nexus MENU.BPK renderer handoff gate: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus MENU.BPK renderer handoff gate passed");
    return 0;
}
