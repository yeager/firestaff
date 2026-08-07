#include "fmtowns_tsugaru_bridge.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int   init_calls = 0, shutdown_calls = 0, tbios_calls = 0;
static int   timing_calls = 0, io_r_calls = 0, io_w_calls = 0;
static char  captured_path[256];
static int   init_should_fail = 0;

static int stub_init(const char *path) {
    ++init_calls;
    if (path) { strncpy(captured_path, path, sizeof(captured_path)-1); }
    else      { captured_path[0] = '\0'; }
    return init_should_fail ? -1 : 0;
}
static void stub_shutdown(void) { ++shutdown_calls; }
static fmtowns_bios_status_t stub_tbios(void *c, fmtowns_bios_regs_t *r) {
    (void)c; (void)r; ++tbios_calls; return FMTOWNS_BIOS_HOST_OK;
}
static fmtowns_bios_status_t stub_timing(void *c, fmtowns_bios_regs_t *r) {
    (void)c; (void)r; ++timing_calls; return FMTOWNS_BIOS_HOST_OK;
}
static fmtowns_bios_status_t stub_io_r(void *c, uint16_t p, uint8_t *o) {
    (void)c; (void)p; ++io_r_calls; if (o) *o = 0x5a; return FMTOWNS_BIOS_HOST_OK;
}
static fmtowns_bios_status_t stub_io_w(void *c, uint16_t p, uint8_t v) {
    (void)c; (void)p; (void)v; ++io_w_calls; return FMTOWNS_BIOS_HOST_OK;
}

int main(void) {
    /* Not registered by default. */
    assert(fmtowns_tsugaru_bridge_available_pc34() == 0);
    assert(fmtowns_tsugaru_bridge_host_pc34() == NULL);

    /* NULL vtable rejected. */
    assert(fmtowns_tsugaru_bridge_register_pc34(NULL, "x") == 0);

    /* Missing required entry rejected. */
    fmtowns_tsugaru_bridge_vtable_v1_t bad = {0};
    bad.init = stub_init; bad.shutdown = stub_shutdown; /* no call_tbios */
    assert(fmtowns_tsugaru_bridge_register_pc34(&bad, "x") == 0);

    /* Init failure rejected and rolled back. */
    fmtowns_tsugaru_bridge_vtable_v1_t vt = {0};
    vt.init = stub_init; vt.shutdown = stub_shutdown; vt.call_tbios = stub_tbios;
    init_should_fail = 1;
    assert(fmtowns_tsugaru_bridge_register_pc34(&vt, "path") == 0);
    assert(fmtowns_tsugaru_bridge_available_pc34() == 0);
    /* Init was called (and rolled back), shutdown was NOT — the bridge
     * did not successfully initialise. */
    assert(init_calls == 1 && shutdown_calls == 0);
    init_should_fail = 0;

    /* Successful registration with partial vtable. */
    vt.call_timing       = stub_timing;
    vt.io_port_read_u8   = stub_io_r;
    vt.io_port_write_u8  = stub_io_w;
    /* Leave call_secondary and call_hardware_init NULL. */
    assert(fmtowns_tsugaru_bridge_register_pc34(&vt, "/rom/FMT_F20.ROM") == 1);
    assert(fmtowns_tsugaru_bridge_available_pc34() == 1);
    assert(strcmp(captured_path, "/rom/FMT_F20.ROM") == 0);

    const fmtowns_bios_host_t *h = fmtowns_tsugaru_bridge_host_pc34();
    assert(h != NULL);
    assert(h->call_tbios != NULL);
    assert(h->call_timing != NULL);
    assert(h->io_port_read_u8 != NULL);
    assert(h->io_port_write_u8 != NULL);
    /* Optional entries stayed NULL. */
    assert(h->call_secondary == NULL);
    assert(h->call_hardware_init == NULL);
    /* SJIS glyph is served by the shim, not the bridge. */
    assert(h->tbios_fetch_sjis_glyph == NULL);

    /* Bind and dispatch. */
    fmtowns_bios_host_bind_pc34(h);
    fmtowns_bios_regs_t regs = {0};
    assert(fmtowns_bios_host_call_pc34(FMTOWNS_BIOS_SLOT_TBIOS, &regs)
           == FMTOWNS_BIOS_HOST_OK);
    assert(tbios_calls == 1);
    assert(fmtowns_bios_host_call_pc34(FMTOWNS_BIOS_SLOT_TIMING, &regs)
           == FMTOWNS_BIOS_HOST_OK);
    assert(timing_calls == 1);
    /* NULL-entry slot yields UNSUPPORTED. */
    assert(fmtowns_bios_host_call_pc34(FMTOWNS_BIOS_SLOT_SECONDARY, &regs)
           == FMTOWNS_BIOS_HOST_UNSUPPORTED);
    assert(fmtowns_bios_host_call_pc34(FMTOWNS_BIOS_SLOT_HARDWARE_INIT, &regs)
           == FMTOWNS_BIOS_HOST_UNSUPPORTED);
    uint8_t v;
    assert(fmtowns_bios_host_io_read_u8_pc34(0x04E9, &v) == FMTOWNS_BIOS_HOST_OK);
    assert(v == 0x5a);
    assert(io_r_calls == 1);
    assert(fmtowns_bios_host_io_write_u8_pc34(0x04E9, 0x00) == FMTOWNS_BIOS_HOST_OK);
    assert(io_w_calls == 1);

    /* Re-register replaces cleanly. */
    fmtowns_tsugaru_bridge_vtable_v1_t vt2 = vt;
    assert(fmtowns_tsugaru_bridge_register_pc34(&vt2, "/rom/other.ROM") == 1);
    /* Prior shutdown was invoked before re-init. */
    assert(shutdown_calls == 1);
    assert(strcmp(captured_path, "/rom/other.ROM") == 0);

    /* Unregister calls shutdown once and restores unbound state. */
    fmtowns_tsugaru_bridge_unregister_pc34();
    assert(shutdown_calls == 2);
    assert(fmtowns_tsugaru_bridge_available_pc34() == 0);
    assert(fmtowns_tsugaru_bridge_host_pc34() == NULL);
    /* Double-unregister is safe (no additional shutdown call). */
    fmtowns_tsugaru_bridge_unregister_pc34();
    assert(shutdown_calls == 2);

    fmtowns_bios_host_unbind_pc34();
    puts("All fmtowns_tsugaru_bridge tests passed.");
    return 0;
}
