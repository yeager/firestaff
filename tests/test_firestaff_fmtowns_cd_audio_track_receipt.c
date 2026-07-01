/*
 * test_firestaff_fmtowns_cd_audio_track_receipt.c
 *
 * Test driver for src/shared/firestaff_fmtowns_cd_audio_track_receipt.c.
 *
 * The library exposes FirestaffFmtownsCd_AudioReceiptSelfTest()
 * which runs the full internal coverage suite. This driver is a
 * thin main() that calls SelfTest and reports pass/fail to stdout.
 * Build it via CMakeLists.txt as part of the standard CTest matrix.
 *
 * The suite covers (data-free, no game data vendored):
 *   - DM1 v2.0 redump layout: 19 audio receipts with the documented
 *     silent tracks 04/07 (UNUSED_PER_DOC) and 20 (SILENT_PER_DOC,
 *     20s silence), 16 REAL_AUDIO.
 *   - DM2 v1.0 redump layout: 7 audio receipts with track 08
 *     SILENT_PER_DOC and 6 REAL_AUDIO.
 *   - CSB v3.1 redump layout (minimal shape, 30 audio): exactly 3
 *     SILENT_DETECTED tracks get promoted to WATER_DROP per DMWeb.
 *   - Sub-sector byte slice -> SHORT_TRACK.
 *   - No byte stream + CUE-only -> MISSING_BYTES.
 *   - Documented-unused track whose PCM is not silent -> INCONSISTENT.
 *   - Real-audio synthetic stream reports max_abs >= 64 and
 *     looks_like_audio = 1.
 *   - max_sectors_per_track caps the per-track scan width.
 *   - Companion classifier SelfTest() still passes (the receipt
 *     module re-uses its CUE parser + game classifier).
 *
 * Build:
 *   cc -std=c99 -I include \
 *      tests/test_firestaff_fmtowns_cd_audio_track_receipt.c \
 *      src/shared/firestaff_fmtowns_cd_audio_track_receipt.c \
 *      src/shared/firestaff_fmtowns_cd_classify.c \
 *      -o test_firestaff_fmtowns_cd_audio_track_receipt
 */

#include "firestaff_fmtowns_cd_audio_track_receipt.h"

#include <stdio.h>

int main(void) {
    int rc = FirestaffFmtownsCd_AudioReceiptSelfTest();
    if (rc == 0) {
        printf("test_firestaff_fmtowns_cd_audio_track_receipt: PASS\n");
        return 0;
    }
    printf("test_firestaff_fmtowns_cd_audio_track_receipt: FAIL\n");
    return 1;
}
