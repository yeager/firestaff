#include "fmtowns_tsugaru_bridge.h"
#include <stddef.h>
#include <string.h>

static fmtowns_tsugaru_bridge_vtable_v1_t g_vt;
static int                                  g_registered = 0;
static fmtowns_bios_host_t                  g_host;

static fmtowns_bios_status_t fwd_tbios(void *c, fmtowns_bios_regs_t *r) {
    return g_vt.call_tbios ? g_vt.call_tbios(c, r) : FMTOWNS_BIOS_HOST_UNSUPPORTED;
}
static fmtowns_bios_status_t fwd_secondary(void *c, fmtowns_bios_regs_t *r) {
    return g_vt.call_secondary ? g_vt.call_secondary(c, r) : FMTOWNS_BIOS_HOST_UNSUPPORTED;
}
static fmtowns_bios_status_t fwd_timing(void *c, fmtowns_bios_regs_t *r) {
    return g_vt.call_timing ? g_vt.call_timing(c, r) : FMTOWNS_BIOS_HOST_UNSUPPORTED;
}
static fmtowns_bios_status_t fwd_hardware_init(void *c, fmtowns_bios_regs_t *r) {
    return g_vt.call_hardware_init ? g_vt.call_hardware_init(c, r) : FMTOWNS_BIOS_HOST_UNSUPPORTED;
}
static fmtowns_bios_status_t fwd_io_read(void *c, uint16_t p, uint8_t *o) {
    return g_vt.io_port_read_u8 ? g_vt.io_port_read_u8(c, p, o) : FMTOWNS_BIOS_HOST_UNSUPPORTED;
}
static fmtowns_bios_status_t fwd_io_write(void *c, uint16_t p, uint8_t v) {
    return g_vt.io_port_write_u8 ? g_vt.io_port_write_u8(c, p, v) : FMTOWNS_BIOS_HOST_UNSUPPORTED;
}

int fmtowns_tsugaru_bridge_register_pc34(
        const fmtowns_tsugaru_bridge_vtable_v1_t *vt,
        const char *bios_rom_path) {
    if (!vt) return 0;
    /* Required entries must not be NULL. */
    if (!vt->init || !vt->shutdown || !vt->call_tbios) return 0;
    /* Reset any prior binding first. */
    fmtowns_tsugaru_bridge_unregister_pc34();
    memcpy(&g_vt, vt, sizeof(g_vt));
    int rc = g_vt.init(bios_rom_path);
    if (rc != 0) {
        memset(&g_vt, 0, sizeof(g_vt));
        return 0;
    }
    g_host.ctx                    = NULL;
    g_host.call_tbios             = fwd_tbios;
    g_host.call_secondary         = vt->call_secondary     ? fwd_secondary     : NULL;
    g_host.call_timing            = vt->call_timing        ? fwd_timing        : NULL;
    g_host.call_hardware_init     = vt->call_hardware_init ? fwd_hardware_init : NULL;
    g_host.io_port_read_u8        = vt->io_port_read_u8    ? fwd_io_read       : NULL;
    g_host.io_port_write_u8       = vt->io_port_write_u8   ? fwd_io_write      : NULL;
    g_host.tbios_fetch_sjis_glyph = NULL; /* JDM shim serves this. */
    g_registered = 1;
    return 1;
}

void fmtowns_tsugaru_bridge_unregister_pc34(void) {
    if (g_registered && g_vt.shutdown) g_vt.shutdown();
    memset(&g_vt, 0, sizeof(g_vt));
    memset(&g_host, 0, sizeof(g_host));
    g_registered = 0;
}

int fmtowns_tsugaru_bridge_available_pc34(void) {
    return g_registered;
}

const fmtowns_bios_host_t *fmtowns_tsugaru_bridge_host_pc34(void) {
    return g_registered ? &g_host : NULL;
}
