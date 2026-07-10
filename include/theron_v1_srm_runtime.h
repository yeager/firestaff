#ifndef THERON_V1_SRM_RUNTIME_H
#define THERON_V1_SRM_RUNTIME_H

#include "theron_v1_srm_classifier.h"
#include "theron_v1_world.h"

#include <stddef.h>
#include <stdint.h>

/* Runtime interchange for Firestaff's bounded FSTQPTY1 Save Disk body.
 * The gzip .srm container is real file I/O; only the body mapping is the
 * documented Firestaff envelope, not a claim about unknown Sphenx bodies.
 * ReDMCSB: THQUEST.ASM T080/T800, between-dungeon persistence. */
typedef enum {
    THERON_V1_SRM_RUNTIME_OK = 1,
    THERON_V1_SRM_RUNTIME_ZLIB_UNAVAILABLE = 0,
    THERON_V1_SRM_RUNTIME_BAD_INPUT = -1,
    THERON_V1_SRM_RUNTIME_IO_FAILED = -2,
    THERON_V1_SRM_RUNTIME_UNSUPPORTED_BODY = -3,
    THERON_V1_SRM_RUNTIME_MEDIA_UNVERIFIED = -4
} Theron_V1SrmRuntimeStatus;

typedef struct {
    Theron_V1SrmRuntimeStatus status;
    Theron_V1SrmEnvelopeKind envelope_kind;
    Theron_V1SrmProgressImportStatus import_status;
    size_t srm_size;
    Theron_DungeonID dungeon;
    uint8_t level;
    uint8_t quest_mask;
    uint8_t champion_count;
    uint32_t party_gold;
    Theron_RuntimeMediaIdentity track02_identity;
} Theron_V1SrmRuntimeReceipt;

/* Writes one gzip-wrapped FSTQPTY1 .srm body.  It is intended for the
 * same bounded import route below and never overwrites a destination until
 * compression and the complete write have succeeded. */
Theron_V1SrmRuntimeStatus theron_v1_srm_runtime_export_path(
    const Theron_V1_World *world,
    const char *srm_path,
    Theron_V1SrmRuntimeReceipt *out_receipt);

/* The single Continue runtime route: read/decode a real .srm file, restore
 * party/progression/quest/level bytes, then derive and bind Track 02 media
 * identity from the supplied hash-profiled Track 02 bytes.  No world state
 * is changed until both restore and media identity verification succeed. */
Theron_V1SrmRuntimeStatus theron_v1_srm_runtime_continue_path(
    Theron_V1_World *world,
    const char *srm_path,
    const uint8_t *track02_bytes,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1SrmRuntimeReceipt *out_receipt);

const char *theron_v1_srm_runtime_status_name(Theron_V1SrmRuntimeStatus status);

#endif
