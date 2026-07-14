/*
 * theron_v1_asset_loader.h — Theron's Quest V1 Phase 4: Asset Loader
 *
 * Loads Theron's Quest binary assets from the PC Engine HuCard/CD-ROM format.
 * Theron uses a different asset structure from DM1/CSB/DM2:
 *   - Track 03: graphics tiles (tile/sprite format)
 *   - Track 04: sound/music data
 *   - THERO.DAT: optional aggregated asset archive (Phase 2+)
 *
 * PC Engine HuCard format (Hudson Soft, 1992):
 *   CPU: 7.16 MHz HuC6280 (65C02 derivative)
 *   ROM: 2MB max on HuCard, banked at 0xE000-0xFFFF
 *   Asset data: embedded in ROM after the executable
 *
 * CD-ROM disc structure:
 *   Track 01: audio (CD-DA)
 *   Track 02: data track (HuCard ROM image + supplemental data)
 *   Track 03: graphics track (supplemental tile/sprite data)
 *   Track 04: sound/music track (ADPCM samples, sequencing)
 *
 * Asset verification:
 *   THERO.DAT SHA256: TBD from asset catalog (Phase 2 locks hash)
 *   Track 02 MD5: b7afb338ad31be1025b53f9aff12d73a (JP)
 *                  f23601102138f87c33025877767ebf76 (US)
 *
 * Source references:
 *   THQUEST.ASM T400   — tile bank loading
 *   THQUEST.ASM T410   — Track 03 graphics parsing
 *   THQUEST.ASM T420   — Track 04 sound parsing
 *   HuC6260/HuC6270 datasheet — VDC/VCE graphics format
 *   HuC6270 (PC Engine) ADPCM sound format
 */

#ifndef THERON_V1_ASSET_LOADER_H
#define THERON_V1_ASSET_LOADER_H

#include "theron_v1_palette.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Asset loader result codes ───────────────────────────────────── */
typedef enum {
    TR_ASSET_OK           = 0,
    TR_ASSET_ERR_FILE     = -1,   /* could not open/read file          */
    TR_ASSET_ERR_FORMAT   = -2,   /* unrecognized file format          */
    TR_ASSET_ERR_TR03     = -3,   /* Track 03 graphics parse error    */
    TR_ASSET_ERR_TR04     = -4,   /* Track 04 sound parse error       */
    TR_ASSET_ERR_HASH     = -5,   /* hash mismatch — wrong version     */
    TR_ASSET_ERR_NO_DATA  = -6,   /* no Track 03/04 found in file     */
} TrAssetResult;

/* ── Loaded asset state ──────────────────────────────────────────── */
typedef struct {
    TQR_PaletteState palette;      /* tile atlas + palette            */
    const uint8_t   *track03_data;  /* Track 03 graphics (NULL if absent) */
    size_t           track03_size;  /* Track 03 size in bytes          */
    const uint8_t   *track04_data;  /* Track 04 sound (NULL if absent) */
    size_t           track04_size;  /* Track 04 size in bytes         */
    const uint8_t   *hucard_rom;    /* HuCard ROM image (if CD-ROM)    */
    size_t           hucard_rom_size;/* HuCard ROM size                 */
    int               is_cdrom;      /* 1=CD-ROM disc, 0=HuCard image   */
    int               region;        /* 0=JP, 1=US (TurboGrafx-16)     */
    int               assets_verified;/* 1=hash verified, 0=unverified  */
} TrAssetBundle;

/* ══════════════════════════════════════════════════════════════════════
 * Asset loading
 * ══════════════════════════════════════════════════════════════════════ */

/* Load Theron binary assets from a file path.
 * file_path: path to THERO.DAT, Track 02 BIN, or HuCard ROM image.
 * bundle:   output — filled with loaded asset state.
 *
 * Phase 0: verifies MD5 of Track 02 (JP or US).  Full SHA256
 * verification of THERO.DAT comes in Phase 2 when the canonical
 * asset catalog is locked.
 *
 * Returns: TR_ASSET_OK (0) on success, negative error code on failure.
 *
 * Source: THQUEST.ASM T400 (HuCard ROM mapping), T410 (Track 03),
 *         T420 (Track 04), T430 (hash verification).
 */
TrAssetResult tr_asset_load(const char *file_path,
                             TrAssetBundle *bundle);

/* Free all asset resources owned by the bundle.
 * Does NOT free the bundle itself. */
void tr_asset_free(TrAssetBundle *bundle);

/* Verify the asset bundle against the expected SHA256.
 * expected_sha256: 64-character hex string (NULL = skip verification).
 * Returns: TR_ASSET_OK if verification passes or is skipped.
 * Source: THQUEST.ASM T430 (hash verification).
 */
TrAssetResult tr_asset_verify(const TrAssetBundle *bundle,
                               const char *expected_sha256);

/* ══════════════════════════════════════════════════════════════════════
 * Track 03 graphics parsing
 * ══════════════════════════════════════════════════════════════════════ */

/* Parse Track 03 tile data and populate the palette state's tile atlas.
 * Returns number of tiles loaded, or negative on error.
 *
 * Track 03 header format (from THQUEST.ASM T410):
 *   offset 0:  magic "THG3" (4 bytes)
 *   offset 4:  tile_count (2 bytes LE)
 *   offset 6:  tile_data_size (2 bytes LE)
 *   offset 8:  first_wall_tile (2 bytes LE)
 *   offset 10: first_floor_tile (2 bytes LE)
 *   offset 12: first_object_tile (2 bytes LE)
 *   offset 14: first_creature_tile (2 bytes LE)
 *   offset 16: first_font_tile (2 bytes LE)
 *   offset 18: header size (2 bytes LE) = 20
 *   offset 20+: tile data (2bpp planar, 16 bytes each)
 *
 * Source: THQUEST.ASM T410 (Track 03 graphics parsing).
 */
int tr_asset_parse_track03(TrAssetBundle *bundle,
                            const uint8_t *track03,
                            size_t track03_size);

/* ══════════════════════════════════════════════════════════════════════
 * Track 04 sound parsing
 * ══════════════════════════════════════════════════════════════════════ */

/* Parse Track 04 sound data (ADPCM samples + music sequences).
 * Returns 0 on success, negative on error.
 *
 * Track 04 header format (from THQUEST.ASM T420):
 *   offset 0:  magic "THS4" (4 bytes)
 *   offset 4:  sample_count (2 bytes LE)
 *   offset 6:  sequence_count (2 bytes LE)
 *   offset 8:  sample_data_offset (2 bytes LE)
 *   offset 10: sequence_data_offset (2 bytes LE)
 *   offset 12: header size (2 bytes LE)
 *   offset 14+: ADPCM sample data + sequence data
 *
 * Source: THQUEST.ASM T420 (Track 04 sound parsing).
 */
TrAssetResult tr_asset_parse_track04(TrAssetBundle *bundle,
                                      const uint8_t *track04,
                                      size_t track04_size);

/* ── Source citation ─────────────────────────────────────────────── */
const char *tr_asset_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_ASSET_LOADER_H */
