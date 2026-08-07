#ifndef FMTOWNS_BIOS_HOST_H
#define FMTOWNS_BIOS_HOST_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Host contract for TownsOS BIOS runtime execution.
 *
 * Firestaff is pure C. Tsugaru (github.com/captainys/TOWNSEMU) is
 * C++ with a substantial build surface. Rather than link against
 * it directly (which would violate the LANGUAGES C rule in
 * CMakeLists.txt), Firestaff defines a small C ABI here that a
 * separate adapter — either a Tsugaru bridge built as an optional
 * shared library, or an in-process minimal TBIOS shim (Option B
 * in docs/fmtowns/TOWNSOS_BIOS_INTEGRATION.md) — can implement.
 *
 * The default runtime binding is fail-closed: every call returns
 * FMTOWNS_BIOS_HOST_UNBOUND until a caller registers a real host.
 *
 * The four canonical Phar Lap slots (docs/fmtowns/pharlap_call_sites.json):
 *   fs:[0x20]  -> TBIOS
 *   fs:[0x40]  -> secondary
 *   fs:[0x48]  -> timing
 *   fs:[0x80]  -> hardware init
 *
 * Callers dispatch by (slot, subfunction, args). Register a host
 * once at startup via fmtowns_bios_host_bind_pc34(); unregister
 * with fmtowns_bios_host_unbind_pc34() (mainly for tests).
 */

typedef enum fmtowns_bios_slot {
    FMTOWNS_BIOS_SLOT_TBIOS         = 0x20,
    FMTOWNS_BIOS_SLOT_SECONDARY     = 0x40,
    FMTOWNS_BIOS_SLOT_TIMING        = 0x48,
    FMTOWNS_BIOS_SLOT_HARDWARE_INIT = 0x80
} fmtowns_bios_slot_t;

typedef enum fmtowns_bios_status {
    FMTOWNS_BIOS_HOST_OK        = 0,
    FMTOWNS_BIOS_HOST_UNBOUND   = -1,
    FMTOWNS_BIOS_HOST_BAD_SLOT  = -2,
    FMTOWNS_BIOS_HOST_BAD_ARGS  = -3,
    FMTOWNS_BIOS_HOST_UNSUPPORTED = -4,
    FMTOWNS_BIOS_HOST_FAILED    = -5
} fmtowns_bios_status_t;

/* Real-mode register bundle passed across the far-call boundary.
 * Layout matches the DM1/CSB/DM2 Phar Lap thunk expectations. */
typedef struct fmtowns_bios_regs {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi;
    uint32_t ebp;
    uint16_t ds, es, fs;
    uint16_t flags;
} fmtowns_bios_regs_t;

/* Host implementation: one function per canonical slot. Any may be
 * NULL — a NULL slot returns FMTOWNS_BIOS_HOST_UNSUPPORTED. */
typedef struct fmtowns_bios_host {
    void *ctx;
    fmtowns_bios_status_t (*call_tbios)        (void *ctx, fmtowns_bios_regs_t *r);
    fmtowns_bios_status_t (*call_secondary)    (void *ctx, fmtowns_bios_regs_t *r);
    fmtowns_bios_status_t (*call_timing)       (void *ctx, fmtowns_bios_regs_t *r);
    fmtowns_bios_status_t (*call_hardware_init)(void *ctx, fmtowns_bios_regs_t *r);

    /* Direct I/O port access (documented ports:
     * 0x04E9 SOUND_INT_REASON + 4 CMOS/RS232C). */
    fmtowns_bios_status_t (*io_port_read_u8)   (void *ctx, uint16_t port, uint8_t *out);
    fmtowns_bios_status_t (*io_port_write_u8)  (void *ctx, uint16_t port, uint8_t val);

    /* Shift-JIS glyph fetch — the specific TBIOS subfunction the
     * JDM font module needs. Returns 16x16 or 12x12 mono bitmap in
     * `bitmap_out` (caller-owned buffer, at least 32 bytes for
     * 16x16). `bytes_written` reports what was actually filled. */
    fmtowns_bios_status_t (*tbios_fetch_sjis_glyph)(
        void *ctx, uint8_t lead, uint8_t trail,
        uint8_t *bitmap_out, size_t bitmap_out_cap,
        size_t *bytes_written, uint8_t *glyph_w, uint8_t *glyph_h);
} fmtowns_bios_host_t;

/* Bind/unbind. Returns previous host (or NULL). Thread-safety: not
 * synchronised — callers must bind once at startup. */
const fmtowns_bios_host_t *fmtowns_bios_host_bind_pc34(
    const fmtowns_bios_host_t *host);
const fmtowns_bios_host_t *fmtowns_bios_host_current_pc34(void);
void fmtowns_bios_host_unbind_pc34(void);

/* Dispatchers. Return FMTOWNS_BIOS_HOST_UNBOUND if no host is
 * bound (fail-closed default). */
fmtowns_bios_status_t fmtowns_bios_host_call_pc34(
    fmtowns_bios_slot_t slot, fmtowns_bios_regs_t *regs);

fmtowns_bios_status_t fmtowns_bios_host_io_read_u8_pc34(
    uint16_t port, uint8_t *out);

fmtowns_bios_status_t fmtowns_bios_host_io_write_u8_pc34(
    uint16_t port, uint8_t val);

fmtowns_bios_status_t fmtowns_bios_host_fetch_sjis_glyph_pc34(
    uint8_t lead, uint8_t trail,
    uint8_t *bitmap_out, size_t bitmap_out_cap,
    size_t *bytes_written, uint8_t *glyph_w, uint8_t *glyph_h);

#ifdef __cplusplus
}
#endif

#endif /* FMTOWNS_BIOS_HOST_H */
