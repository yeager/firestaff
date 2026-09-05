#ifndef DM1_V1_FMTOWNS_DUNGEON_DAT_H
#define DM1_V1_FMTOWNS_DUNGEON_DAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* FM Towns DM1 DUNGEON.DAT classifier.
 *
 * The FM Towns DUNGEON.DAT is structurally identical to the PC 3.4
 * format (same header layout and thing types). Header word 0 is the ornament
 * random seed (99/0x0063); the actual map count is byte 4 and is 14.
 * but differs in:
 *   - Total file size: EN 33423 vs PC 33357 (66 bytes larger)
 *   - Minor thing count differences at header offsets 2, 10, 18
 *   - Japanese variant (JP) is 33931 bytes
 *
 * Header counts remain dynamic. English uses the ordinary checksum-bearing
 * tail reader; the exact-hash Japanese CD body has a dedicated reader because
 * it ends after RawMapData without the two-byte F0434 checksum trailer. */

#define DM1_FMTOWNS_DUNGEON_EN_SIZE     33423U
#define DM1_FMTOWNS_DUNGEON_JP_SIZE     33931U
#define DM1_FMTOWNS_DUNGEON_ORNAMENT_SEED 99U
#define DM1_FMTOWNS_DUNGEON_MAP_COUNT     14U

typedef struct {
    int      is_fmtowns;
    uint32_t file_size;
    uint16_t map_count;
    uint8_t  md5[16];
    int      lang;
} DM1_V1_FmtownsDungeonReceipt;

/* Probe whether a DUNGEON.DAT buffer could be the FM Towns DM1 version.
 * Checks ornament seed 99, map count 14, and a known FM Towns size.
 * Returns 1 if recognized. */
int dm1_v1_fmtowns_dungeon_probe(const uint8_t *data, size_t size);

/* Build a receipt for the FM Towns DM1 DUNGEON.DAT.
 * Returns 0 on success, -1 on error. */
int dm1_v1_fmtowns_dungeon_receipt(const uint8_t *data, size_t size,
                                    DM1_V1_FmtownsDungeonReceipt *out);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_DUNGEON_DAT_H */
