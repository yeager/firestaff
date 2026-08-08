#ifndef DM2_V1_DOS_STARTUP_MEDIA_H
#define DM2_V1_DOS_STARTUP_MEDIA_H

#include <stdint.h>
#include <stddef.h>

/* Read-only receipt for the PC English distribution's outer startup route.
 * DM2.BAT delegates to IBMIOP before SKULL.EXE.  That wrapper owns the
 * Splash/FTL/INTRO movie route; SKULL.EXE then owns the static GDAT menu.
 * All members remain at their user-selected install path: this module never
 * extracts archive members or writes decoded movie data. */
typedef struct {
    int valid;
    int complete;
    int batch_dispatches_ibmiop;
    int ibmiop_verified;
    int splash_verified;
    int ftl_verified;
    int intro_verified;
    int end_verified;
    int intrplay_pcx_verified;
    int intro_has_interplay_mve;
    int end_has_interplay_mve;
    uint32_t intro_mve_header_offset;
    uint32_t end_mve_header_offset;
    uint32_t receipt_hash;
} DM2_V1_DosStartupMediaReceipt;

/* Probes the supplied DOS installation directory against the retail PC
 * English manifest.  A missing, renamed or byte-different member yields an
 * invalid receipt.  The caller retains ownership of the directory and no
 * game bytes survive this call. */
int dm2_v1_dos_startup_media_probe(
    const char *install_root, DM2_V1_DosStartupMediaReceipt *out);

/* Copies the authenticated retail INTRO executable directly into caller-owned
 * RAM.  This is intentionally a one-member source read, not archive
 * extraction or an on-disk cache: the full byte range is SHA-256 checked
 * against the DOS-English manifest again at the point it becomes resident.
 * The supplied startup receipt must be the complete IBMIOP route obtained
 * from the same selected install root.  On failure, no bytes are retained. */
int dm2_v1_dos_startup_media_load_intro_verified(
    const char *install_root, const DM2_V1_DosStartupMediaReceipt *receipt,
    uint8_t **out_bytes, size_t *out_byte_count);

#endif /* DM2_V1_DOS_STARTUP_MEDIA_H */
