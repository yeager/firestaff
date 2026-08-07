#include "fmtowns_bios_host.h"
#include <stddef.h>

static const fmtowns_bios_host_t *g_host = NULL;

const fmtowns_bios_host_t *fmtowns_bios_host_bind_pc34(
        const fmtowns_bios_host_t *host) {
    const fmtowns_bios_host_t *prev = g_host;
    g_host = host;
    return prev;
}

const fmtowns_bios_host_t *fmtowns_bios_host_current_pc34(void) {
    return g_host;
}

void fmtowns_bios_host_unbind_pc34(void) {
    g_host = NULL;
}

fmtowns_bios_status_t fmtowns_bios_host_call_pc34(
        fmtowns_bios_slot_t slot, fmtowns_bios_regs_t *regs) {
    if (!g_host) return FMTOWNS_BIOS_HOST_UNBOUND;
    if (!regs)   return FMTOWNS_BIOS_HOST_BAD_ARGS;
    switch (slot) {
    case FMTOWNS_BIOS_SLOT_TBIOS:
        if (!g_host->call_tbios) return FMTOWNS_BIOS_HOST_UNSUPPORTED;
        return g_host->call_tbios(g_host->ctx, regs);
    case FMTOWNS_BIOS_SLOT_SECONDARY:
        if (!g_host->call_secondary) return FMTOWNS_BIOS_HOST_UNSUPPORTED;
        return g_host->call_secondary(g_host->ctx, regs);
    case FMTOWNS_BIOS_SLOT_TIMING:
        if (!g_host->call_timing) return FMTOWNS_BIOS_HOST_UNSUPPORTED;
        return g_host->call_timing(g_host->ctx, regs);
    case FMTOWNS_BIOS_SLOT_HARDWARE_INIT:
        if (!g_host->call_hardware_init) return FMTOWNS_BIOS_HOST_UNSUPPORTED;
        return g_host->call_hardware_init(g_host->ctx, regs);
    default:
        return FMTOWNS_BIOS_HOST_BAD_SLOT;
    }
}

fmtowns_bios_status_t fmtowns_bios_host_io_read_u8_pc34(
        uint16_t port, uint8_t *out) {
    if (!g_host) return FMTOWNS_BIOS_HOST_UNBOUND;
    if (!out)    return FMTOWNS_BIOS_HOST_BAD_ARGS;
    if (!g_host->io_port_read_u8) return FMTOWNS_BIOS_HOST_UNSUPPORTED;
    return g_host->io_port_read_u8(g_host->ctx, port, out);
}

fmtowns_bios_status_t fmtowns_bios_host_io_write_u8_pc34(
        uint16_t port, uint8_t val) {
    if (!g_host) return FMTOWNS_BIOS_HOST_UNBOUND;
    if (!g_host->io_port_write_u8) return FMTOWNS_BIOS_HOST_UNSUPPORTED;
    return g_host->io_port_write_u8(g_host->ctx, port, val);
}

fmtowns_bios_status_t fmtowns_bios_host_fetch_sjis_glyph_pc34(
        uint8_t lead, uint8_t trail,
        uint8_t *bitmap_out, size_t bitmap_out_cap,
        size_t *bytes_written, uint8_t *glyph_w, uint8_t *glyph_h) {
    if (!g_host) return FMTOWNS_BIOS_HOST_UNBOUND;
    if (!bitmap_out || bitmap_out_cap == 0) return FMTOWNS_BIOS_HOST_BAD_ARGS;
    if (!g_host->tbios_fetch_sjis_glyph) return FMTOWNS_BIOS_HOST_UNSUPPORTED;
    return g_host->tbios_fetch_sjis_glyph(
        g_host->ctx, lead, trail,
        bitmap_out, bitmap_out_cap,
        bytes_written, glyph_w, glyph_h);
}
