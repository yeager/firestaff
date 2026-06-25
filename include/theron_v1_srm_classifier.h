#ifndef THERON_V1_SRM_CLASSIFIER_H
#define THERON_V1_SRM_CLASSIFIER_H

#include <stddef.h>
#include <stdint.h>

/* ══════════════════════════════════════════════════════════════════════
 * Theron V1 SRM (Save RAM) classifier — bounded real-artifact boundary.
 *
 * Source/evidence:
 *   - DMWeb reference, section "Theron's Quest savegame format":
 *       "greatstone has a section on Theron's Quest save games. TQ's
 *        save format is *completely different* from DM's (it uses
 *        gzipped custom format with a header). We should look at this
 *        when implementing src/theron/ V1 save/load."
 *   - dmweb community docs `community/documentation/` page credits
 *     Sphenx with several custom Theron's Quest save games documented
 *     at greatstone; Sphenx is also credited alongside kentaro.k-21
 *     on the SKWIN DM2 skproject.
 *   - The original PC Engine CD "Save Disk" cartridge stores 5 named
 *     disk slots (TQR uses a 5-slot disk save model per Sphenx's
 *     TQ-RTC tool chain). Firestaff keeps a 5-slot disk manifest
 *     here, not a slot-N.tqsv manifest (which is the synthetic native
 *     in-game save format already covered by theron_v1_save_load.c).
 *
 * What this module does:
 *   - Enumerates up to THERON_V1_SRM_DISK_SLOT_COUNT disk slots under
 *     a save-disk root (default `~/.firestaff/data/theron/save/`,
 *     override `FIRESTAFF_THERON_SRM_DIR`).
 *   - For each slot path `slotN.srm`, reads a small prefix and reports:
 *       * present and gzipped → PRESENT_AND_RECOGNIZED
 *       * present, not gzipped → UNRECOGNIZED (could be Sphenx's raw
 *         format, must stay non-launchable until decoded)
 *       * present, gzipped prefix but truncated → MALFORMED
 *       * missing → ABSENT
 *   - Computes a 32-bit rolling checksum over the first 1 KiB of each
 *     file (or whole file if smaller) for receipt/manifest purposes.
 *
 * What this module does NOT do (kept honest):
 *   - It does not decode the gzipped payload, so it cannot synthesize
 *     a launchable Theron_DungeonProgression/Champion state. That
 *     conversion is a separate milestone (greatstone TQ-RTC work).
 *   - It does not promote any public screenshot or claim playability.
 *   - It does not claim full Sphenx-format coverage. Unknown `.srm`
 *     files that lack the gzip magic stay UNRECOGNIZED.
 *
 * Track 02 + Track 02 SRM interplay:
 *   - Theron's Quest has a strict "no in-dungeon saves" design rule
 *     (THQUEST.ASM T080). The real save model is the Save Disk
 *     cartridge, written only at dungeon entrance. This module is the
 *     bounded real-artifact counterpart to the synthetic in-game
 *     theron_v1_save_load.c slot model.
 * ══════════════════════════════════════════════════════════════════════ */

#define THERON_V1_SRM_DISK_SLOT_COUNT    5
#define THERON_V1_SRM_SLOT_NAME_MAX      64
#define THERON_V1_SRM_PATH_MAX           512
#define THERON_V1_SRM_PREFIX_BYTES       1024
#define THERON_V1_SRM_GZIP_MAGIC_0       0x1F
#define THERON_V1_SRM_GZIP_MAGIC_1       0x8B
#define THERON_V1_SRM_GZIP_MAGIC_2_DEFLATE 0x08

/* Per-slot classification.  Order is stable; integer values are part of
 * the public manifest contract for downstream receipt tooling. */
typedef enum {
    THERON_V1_SRM_SLOT_ABSENT = 0,
    THERON_V1_SRM_SLOT_UNRECOGNIZED = 1,
    THERON_V1_SRM_SLOT_MALFORMED = 2,
    THERON_V1_SRM_SLOT_PRESENT_AND_RECOGNIZED = 3
} Theron_V1SrmSlotStatus;

typedef struct {
    int slot_index;                                /* 0..4 */
    Theron_V1SrmSlotStatus status;
    char path[THERON_V1_SRM_PATH_MAX];             /* resolved path */
    uint64_t size_bytes;                           /* 0 if ABSENT */
    uint32_t prefix_checksum32;                    /* rolling sum over the first
                                                      THERON_V1_SRM_PREFIX_BYTES
                                                      bytes (or whole file if
                                                      smaller); 0 if ABSENT */
    int gzip_magic_seen;                           /* 1 if first 2 bytes were 0x1F 0x8B */
    int gzip_deflate_method_seen;                  /* 1 if byte 2 is DEFLATE (0x08) */
} Theron_V1SrmSlotInfo;

typedef struct {
    char root[THERON_V1_SRM_PATH_MAX];
    int slot_count;                                /* always THERON_V1_SRM_DISK_SLOT_COUNT */
    int present_count;                             /* 0..5 */
    int recognized_count;                          /* 0..5 */
    Theron_V1SrmSlotInfo slots[THERON_V1_SRM_DISK_SLOT_COUNT];
    int root_resolved;                             /* 1 if root was non-NULL/non-empty */
} Theron_V1SrmManifest;

/* Resolve the default save-disk root: env override
 * `FIRESTAFF_THERON_SRM_DIR` first, then
 * `$HOME/.firestaff/data/theron/save`, then `./theron-save/`.
 * Returns a stable, NUL-terminated absolute path in `out_root` (always
 * non-empty on success).  Returns 1 on success, 0 on input failure. */
int theron_v1_srm_default_root(char out_root[THERON_V1_SRM_PATH_MAX]);

/* Build the canonical slot path: `<root>/slot<index>.srm`.
 * Returns 1 on success, 0 on bad input.  The output is a clean,
 * deterministic, platform-correct path (no double separators). */
int theron_v1_srm_slot_path(const char *root,
                             int slot_index,
                             char out_path[THERON_V1_SRM_PATH_MAX]);

/* Run the classifier over a save-disk root.  Populates `out_manifest`
 * with one Theron_V1SrmSlotInfo per slot, the resolved root, and the
 * present/recognized rollup.  The function tolerates a missing root
 * (slot status = ABSENT for all slots) and never reports an error for
 * a clean host with no real .srm data.  Returns 1 on success. */
int theron_v1_srm_classify_root(const char *root,
                                Theron_V1SrmManifest *out_manifest);

/* Status-name helpers (stable string contract for receipts/manifests). */
const char *theron_v1_srm_slot_status_name(Theron_V1SrmSlotStatus status);

/* Source/evidence citation.  Same shape as the other Theron probes. */
const char *theron_v1_srm_source_evidence(void);

#endif /* THERON_V1_SRM_CLASSIFIER_H */
