#ifndef FMTOWNS_TBIOS_SHIM_H
#define FMTOWNS_TBIOS_SHIM_H

#include <stddef.h>
#include <stdint.h>
#include "fmtowns_bios_host.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Option-B minimal TBIOS shim (docs/fmtowns/TOWNSOS_BIOS_INTEGRATION.md).
 *
 * Implements the `fmtowns_bios_host_t` contract without pulling in a
 * full FM Towns emulator. This is intentionally scoped: it services
 * ONLY the `tbios_fetch_sjis_glyph` entry — the specific TownsOS
 * TBIOS surface the JDM Shift-JIS font module needs. Every other
 * host entry returns FMTOWNS_BIOS_HOST_UNSUPPORTED (a Tsugaru or
 * full-emulator adapter is the correct home for those).
 *
 * The shim reads a real Fujitsu FMT_F20.ROM (or compatible TBIOS
 * ROM dump) and performs a direct table lookup into the ANK 8x16
 * region for narrow characters and the JIS X 0208 16x16 region for
 * Shift-JIS pairs. Font offsets inside FMT_F20 are well documented
 * (see references section of TOWNSOS_BIOS_INTEGRATION.md).
 *
 * Fail-closed contract:
 *   * `fmtowns_tbios_shim_load_rom_pc34()` returns non-zero iff the
 *     supplied buffer passes ROM identification.
 *   * Until a ROM is loaded, `fetch_sjis_glyph` returns
 *     FMTOWNS_BIOS_HOST_UNBOUND.
 *   * If the ROM is loaded but the requested pair falls outside a
 *     recognised region, the entry returns FMTOWNS_BIOS_HOST_FAILED.
 */

/* Load a TownsOS BIOS ROM image into the shim. `rom_bytes` must
 * remain valid for the shim's lifetime (the shim does not copy).
 * Returns 1 if `rom_size` and the leading signature are consistent
 * with FMT_F20.ROM (128 KiB, starts with "V31L" or similar TBIOS
 * fingerprint); 0 otherwise. Calling with rom=NULL clears the
 * currently loaded ROM and restores the fail-closed default. */
int fmtowns_tbios_shim_load_rom_pc34(const uint8_t *rom_bytes,
                                     size_t rom_size);

/* Return 1 if a ROM is currently loaded and validated. */
int fmtowns_tbios_shim_has_rom_pc34(void);

/* Return a `fmtowns_bios_host_t` pointer that the caller can pass
 * to `fmtowns_bios_host_bind_pc34()`. The pointer targets a
 * process-static struct; do not free it. Only `tbios_fetch_sjis_glyph`
 * is populated; every other host entry is NULL (dispatchers will
 * return FMTOWNS_BIOS_HOST_UNSUPPORTED). */
const fmtowns_bios_host_t *fmtowns_tbios_shim_host_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FMTOWNS_TBIOS_SHIM_H */
