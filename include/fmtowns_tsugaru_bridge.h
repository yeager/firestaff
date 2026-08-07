#ifndef FMTOWNS_TSUGARU_BRIDGE_H
#define FMTOWNS_TSUGARU_BRIDGE_H

#include <stddef.h>
#include <stdint.h>
#include "fmtowns_bios_host.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Tsugaru extern "C" bridge contract.
 *
 * The Option-B TBIOS shim covers text-render service without any
 * emulator. The remaining `fmtowns_bios_host_t` slots — call_tbios
 * subfunctions beyond glyph fetch, call_secondary/timing/hardware_init,
 * and direct I/O port access — require real CPU execution of the
 * Phar Lap real-mode thunks. Tsugaru (github.com/captainys/TOWNSEMU)
 * is the canonical FM Towns emulator and the intended host for these.
 *
 * Firestaff is pure C; Tsugaru is C++. This header defines a
 * process-static, C-ABI vtable that a Tsugaru-side `extern "C"`
 * translation unit can populate at load time. Firestaff never links
 * Tsugaru directly:
 *
 *   Tsugaru side (C++):
 *     extern "C" fmtowns_tsugaru_bridge_vtable_t
 *         fmtowns_tsugaru_bridge_vtable_v1;
 *     // Fill in function pointers, wrap TownsThread instances etc.
 *
 *   Firestaff side (C):
 *     if (fmtowns_tsugaru_bridge_available_pc34()) {
 *         fmtowns_bios_host_bind_pc34(
 *             fmtowns_tsugaru_bridge_host_pc34());
 *     }
 *
 * A shared library (libfirestaff_tsugaru_bridge.dylib / .so / .dll)
 * built from the Tsugaru side is expected to expose the vtable and
 * register itself via `fmtowns_tsugaru_bridge_register_pc34()`. Users
 * who do not build the bridge get an unbound bridge — the JDM shim
 * still works, and everything else that would call a non-glyph BIOS
 * entry returns FMTOWNS_BIOS_HOST_UNBOUND rather than fabricating.
 *
 * Version discipline: the vtable is version-suffixed
 * (`fmtowns_tsugaru_bridge_vtable_v1`). Bumping the vtable layout
 * means bumping the symbol name so old bridges fail to bind rather
 * than silently mismatching the ABI.
 */

typedef struct fmtowns_tsugaru_bridge_vtable_v1 {
    /* Called once when Firestaff registers the bridge. Bridge boots
     * a TownsThread, loads the BIOS ROM path, and returns 0 on
     * success or a non-zero error code. */
    int (*init)(const char *bios_rom_path);

    /* Called once at shutdown. */
    void (*shutdown)(void);

    /* Slot dispatchers — same signatures as fmtowns_bios_host_t. */
    fmtowns_bios_status_t (*call_tbios)        (void *ctx, fmtowns_bios_regs_t *r);
    fmtowns_bios_status_t (*call_secondary)    (void *ctx, fmtowns_bios_regs_t *r);
    fmtowns_bios_status_t (*call_timing)       (void *ctx, fmtowns_bios_regs_t *r);
    fmtowns_bios_status_t (*call_hardware_init)(void *ctx, fmtowns_bios_regs_t *r);
    fmtowns_bios_status_t (*io_port_read_u8)   (void *ctx, uint16_t port, uint8_t *out);
    fmtowns_bios_status_t (*io_port_write_u8)  (void *ctx, uint16_t port, uint8_t val);
} fmtowns_tsugaru_bridge_vtable_v1_t;

/* Register a Tsugaru bridge vtable. Copies the vtable into Firestaff-
 * owned storage (the caller's vtable pointer need not outlive this
 * call). Returns 1 on success, 0 if the vtable is NULL or any
 * required entry is NULL. Optional entries: io_port_read_u8,
 * io_port_write_u8, call_secondary — a NULL entry causes the
 * dispatcher to return UNSUPPORTED. Required entries: init,
 * shutdown, call_tbios. */
int fmtowns_tsugaru_bridge_register_pc34(
    const fmtowns_tsugaru_bridge_vtable_v1_t *vt,
    const char *bios_rom_path);

/* Unregister and call the bridge's shutdown hook. Safe to call
 * even when nothing is registered. */
void fmtowns_tsugaru_bridge_unregister_pc34(void);

/* Return 1 if a Tsugaru bridge is currently registered. */
int fmtowns_tsugaru_bridge_available_pc34(void);

/* Return a `fmtowns_bios_host_t*` populated from the registered
 * Tsugaru vtable, suitable for passing to
 * `fmtowns_bios_host_bind_pc34()`. Returns NULL if no bridge is
 * registered. */
const fmtowns_bios_host_t *fmtowns_tsugaru_bridge_host_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FMTOWNS_TSUGARU_BRIDGE_H */
