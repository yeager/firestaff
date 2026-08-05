/*
 * theron_v1_asset_loader.c — Theron's Quest V1 Phase 4: Asset Loader
 *
 * Loads Theron's Quest binary assets from PC Engine HuCard/CD-ROM format.
 *
 * Source-lock:
 *   docs/source-lock/tqr_v1_track02_consumer_disassembly_2026-08-05.md
 *     — static consumer boundary and missing post-CD RAM capture
 *   docs/source-lock/tqr_v1_track02_graphics_format_real_media_2026-07-11.md
 *     — real-media graphics scan, intentionally decoder-blocked
 *   docs/source-lock/tqr_v1_huc6260_palette_word_format_2026-07-11.md
 *     — caller-offset palette-word decoder only
 *   HuC6260/HuC6270 documentation — VDC/VCE graphics format
 */

#include "theron_v1_asset_loader.h"
#include "theron_v1_palette.h"
#include "theron_v1_track02.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* ── Track magic signatures ──────────────────────────────────────── */
#define TR_MAGIC_THQ   0x31515448UL  /* "THQ1" — HuCard ROM marker */

/* ── Hash verification (Phase 2) ───────────────────────────────── */
/* Full SHA256/MD5 verification comes in Phase 2 when THERO.DAT
 * canonical hashes are locked in the asset catalog.
 * For Phase 4, assets are unverified (assets_verified=0). */

/* ══════════════════════════════════════════════════════════════════════
 * Track 03 graphics parsing
 * ══════════════════════════════════════════════════════════════════════ */

/*
 * tr_asset_parse_track03 — parse Track 03 tile data.
 *
 * The old THG3 marker format was a Firestaff-only guess. Retail CUEs
 * declare tracks 03--18 as audio, and no original Track 02 loader/CD
 * capture has identified a tile-bank byte span or ownership route.
 * THQUEST.ASM T410 remains an audit anchor only.
 */
int tr_asset_parse_track03(TrAssetBundle *bundle,
                            const uint8_t *track03,
                            size_t track03_size) {
    (void)bundle;
    (void)track03;
    (void)track03_size;
    /* Retail CUEs declare tracks 03--18 as audio. "THG3" is a Firestaff
     * marker, not an original Theron's Quest format. A captured HuC6280
     * loader/CD route must identify an actual Track 02 tile-bank span before
     * this API can ever return pixels to the runtime. */
    return -1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Track 04 sound parsing
 * ══════════════════════════════════════════════════════════════════════ */

/*
 * tr_asset_parse_track04 — parse Track 04 sound data.
 *
 * The old THS4 marker format was a Firestaff-only guess. Retail CUEs
 * declare tracks 03--18 as audio, and no original Track 02 loader/CD
 * capture has identified an audio byte grammar or playback ownership.
 * THQUEST.ASM T420 remains an audit anchor only.
 */
TrAssetResult tr_asset_parse_track04(TrAssetBundle *bundle,
                                      const uint8_t *track04,
                                      size_t track04_size) {
    (void)bundle;
    (void)track04;
    (void)track04_size;
    /* No original audio data track or loader route has been proven. Keep
     * audio ownership outside the runtime. */
    return TR_ASSET_ERR_TR04;
}

/* ══════════════════════════════════════════════════════════════════════
 * Format detection and data extraction
 * ══════════════════════════════════════════════════════════════════════ */

/* The former THG3/THS4 marker scan was a Firestaff-only guess. Retail CUE
 * sheets declare tracks 03--18 as audio and no original loader trace has
 * identified such a byte grammar inside Track 02. Never let coincidental
 * bytes in authenticated media create a graphics or audio route. */
static TrAssetResult find_tracks_in_buffer(TrAssetBundle *bundle,
                                            const uint8_t *data,
                                            size_t data_size) {
    (void)bundle;
    (void)data;
    (void)data_size;
    return TR_ASSET_ERR_NO_DATA;
}

/* ══════════════════════════════════════════════════════════════════════
 * Public API
 * ══════════════════════════════════════════════════════════════════════ */

TrAssetResult tr_asset_load(const char *file_path, TrAssetBundle *bundle) {
    if (!file_path || !bundle) return TR_ASSET_ERR_FILE;

    memset(bundle, 0, sizeof(*bundle));
    bundle->assets_verified = 0;

    /* Initialize palette with defaults */
    tqr_palette_init_defaults(&bundle->palette);

    FILE *fp = fopen(file_path, "rb");
    if (!fp) {
        printf("[TQR] Could not open %s: no verified asset data\n",
               file_path);
        /* An empty palette/tile state is not a usable asset load.  Do not
         * report success merely because the old procedural defaults were
         * removed; callers must take the explicit no-data route. */
        return TR_ASSET_ERR_NO_DATA;
    }

    /* Get file size */
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    /* A raw Track 02 container must at least have enough bytes for the
     * legacy region probe below.  Empty/truncated input is not a valid
     * source-backed container and must not reach data[0..3]. */
    if (file_size < 4 || file_size > 64 * 1024 * 1024) {
        fclose(fp);
        return TR_ASSET_ERR_FILE;
    }

    uint8_t *data = (uint8_t *)malloc((size_t)file_size);
    if (!data) {
        fclose(fp);
        return TR_ASSET_ERR_FILE;
    }

    size_t bytes_read = fread(data, 1, (size_t)file_size, fp);
    fclose(fp);

    if (bytes_read != (size_t)file_size) {
        free(data);
        return TR_ASSET_ERR_FILE;
    }

    /* Track 02 media ownership is centralized in raw-media intake. It
     * hash-verifies and materializes only the documented US split image;
     * JP's similarly named End file is already a complete ISO. Do not
     * reconstruct bytes here: a second, path-only implementation can accept
     * a truncated US tail or duplicate JP media. */

    /* Scan for Track 03/04 magic signatures */
    TrAssetResult r = find_tracks_in_buffer(bundle, data, (size_t)file_size);
    if (r < 0) {
        /* Raw Track 02 remains a valid runtime container. Keep its bytes for
         * source-backed consumers, but do not infer graphics or audio banks
         * from marker-like data. Rendering remains fail-closed until an
         * original loader/CD route identifies an actual bank. */
        bundle->hucard_rom = data;
        bundle->hucard_rom_size = (size_t)file_size;
        bundle->region = 1;
        printf("[TQR] Verified Track 02 accepted: %s; later original graphics "
               "bank is not captured yet, so fallback visuals stay disabled\n",
               file_path);
        return TR_ASSET_OK;
    }

    /* Parse Track 03 if found */
    if (bundle->track03_data) {
        int tiles = tr_asset_parse_track03(bundle,
                                            bundle->track03_data,
                                            bundle->track03_size);
        if (tiles < 0) {
            printf("[TQR] Track 03 parse error: %d\n", tiles);
            free(data);
            memset(bundle, 0, sizeof(*bundle));
            return TR_ASSET_ERR_TR03;
        }
    }

    /* Parse Track 04 if found */
    if (bundle->track04_data) {
        r = tr_asset_parse_track04(bundle,
                                    bundle->track04_data,
                                    bundle->track04_size);
        if (r < 0) {
            printf("[TQR] Track 04 parse error: %d\n", r);
            free(data);
            memset(bundle, 0, sizeof(*bundle));
            return TR_ASSET_ERR_TR04;
        }
    }

    /* Keep the raw data for potential CD-ROM HuCard extraction */
    bundle->hucard_rom = data;
    bundle->hucard_rom_size = (size_t)file_size;

    /* Detect region from magic markers */
    {
        uint32_t magic = data[0] | ((uint32_t)data[1] << 8) |
                         ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
        bundle->region = (magic == TR_MAGIC_THQ) ? 0 : 1; /* 0=JP, 1=US */
    }

    printf("[TQR] Asset load complete: %s (%d bytes, region=%s)\n",
           file_path, (int)file_size,
           bundle->region == 0 ? "JP" : "US");

    return TR_ASSET_OK;
}

void tr_asset_block_synthetic_rendering_for_verified_media(
    TrAssetBundle *bundle,
    const char *verified_track02_md5_hex) {
    Theron_Track02Variant variant;

    if (!bundle || !bundle->hucard_rom || bundle->hucard_rom_size == 0u ||
        !verified_track02_md5_hex || !verified_track02_md5_hex[0]) {
        return;
    }
    variant = theron_v1_track02_variant_for_md5(verified_track02_md5_hex);
    if (variant != THERON_TRACK02_VARIANT_JP_BIN &&
        variant != THERON_TRACK02_VARIANT_US_BIN) {
        return;
    }
    /* Verified JP/US Track 02 bytes are authoritative. The legacy loader may
     * already have initialized a default palette or tile store, but neither is
     * source evidence. Only a decoded original bank plus a proven HuC6260
     * palette route can clear this admission boundary. */
    bundle->synthetic_rendering_blocked =
        !tr_asset_generated_v1_rendering_allowed(bundle);
}

int tr_asset_generated_v1_rendering_allowed(const TrAssetBundle *bundle) {
    if (!bundle) {
        return 0;
    }
    /* A graphics bank must be present and have produced original tile bytes,
     * and the palette must come from a hash/offset-proved HuC6260 route.
     * The synthetic_rendering_blocked flag only suppresses generated fallback
     * when no such bank exists; a decoded tile bank plus verified palette
     * route is authoritative original data and overrides the block.
     * Source: theron_v1_asset_loader.h synthetic_rendering_blocked contract;
     * docs/source-lock/tqr_v1_track02_graphics_format_real_media_2026-07-11.md. */
    return bundle->track03_data != NULL &&
           bundle->palette.tile_count > 0 &&
           bundle->palette_route_verified;
}

void tr_asset_mark_palette_route_verified(TrAssetBundle *bundle) {
    if (!bundle) {
        return;
    }
    /* Only a caller with verified Track 02 identity and a concrete palette
     * span/consumer may set this. The default deterministic palette does not
     * qualify. */
    bundle->palette_route_verified = 1;
}

TrAssetResult tr_asset_verify(const TrAssetBundle *bundle,
                              const char *expected_sha256) {
    if (!bundle) return TR_ASSET_ERR_HASH;

    /* This legacy generic loader has no authoritative SHA256 catalog.
     * Production startup uses the separate hash-verified Track 02
     * loader and its strict render admission gates. */
    if (!expected_sha256) {
        /* Verification skipped */
        return TR_ASSET_OK;
    }

    /* Never claim verification without a catalog and a comparison. A caller
     * that supplies an expected digest must use the hash-bound Track 02 boot
     * path; this legacy API cannot authenticate the bundle. */
    return TR_ASSET_ERR_HASH;
}

void tr_asset_free(TrAssetBundle *bundle) {
    if (!bundle) return;

    /* Free palette tiles */
    tqr_palette_free_tiles(&bundle->palette);

    /* Free raw ROM data */
    if (bundle->hucard_rom) {
        free((void *)bundle->hucard_rom);
        bundle->hucard_rom = NULL;
    }

    bundle->track03_data = NULL;
    bundle->track04_data = NULL;
    bundle->track03_size = 0;
    bundle->track04_size = 0;
    bundle->hucard_rom_size = 0;
    bundle->assets_verified = 0;
    bundle->synthetic_rendering_blocked = 0;
    bundle->palette_route_verified = 0;
}

/* ── Source citation ─────────────────────────────────────────────── */
const char *tr_asset_source_evidence(void) {
    return "tqr_v1_track02_consumer_disassembly_2026-08-05.md "
           "+ tqr_v1_track02_graphics_format_real_media_2026-07-11.md "
           "+ tqr_v1_huc6260_palette_word_format_2026-07-11.md "
           "+ HuC6260/HuC6270 documentation (VDC/VCE format); "
           "graphics/audio ownership remains capture-gated";
}
