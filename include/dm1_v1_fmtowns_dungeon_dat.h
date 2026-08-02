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
 * format (same header layout, same thing types, same map count = 99/0x63)
 * but differs in:
 *   - Total file size: EN 33423 vs PC 33357 (66 bytes larger)
 *   - Minor thing count differences at header offsets 2, 10, 18
 *   - Japanese variant (JP) is 33931 bytes
 *
 * The existing F0479/memory_dungeon_dat loader handles this format
 * without modification since it reads the header counts dynamically. */

#define DM1_FMTOWNS_DUNGEON_EN_SIZE     33423U
#define DM1_FMTOWNS_DUNGEON_JP_SIZE     33931U
#define DM1_FMTOWNS_DUNGEON_MAP_COUNT   99U

typedef struct {
    int      is_fmtowns;
    uint32_t file_size;
    uint16_t map_count;
    uint8_t  md5[16];
    int      lang;
} DM1_V1_FmtownsDungeonReceipt;

/* Probe whether a DUNGEON.DAT buffer could be the FM Towns DM1 version.
 * Checks map count == 99 and size matches known FM Towns sizes.
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
