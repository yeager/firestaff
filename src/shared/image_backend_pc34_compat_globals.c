/*
 * image_backend_pc34_compat_globals.c
 *
 * Storage for the legacy image-backend globals declared as
 * `extern` in include/image_backend_pc34_compat.h.
 *
 * The pc34 backend functions (F0687 IMG3_GetNibble,
 * F0688 IMG3_GetPixelCount, F0685 IMG3_LineColorFilling,
 * F0686 IMG_CopyFromPreviousLine, and the IMG3_Compat_Expand*
 * family) reference these globals. They were originally
 * defined in src/shared/image_frontend_pc34.c, which is
 * legacy ReDMCSB-style code (uses M704_ZONE_LEFT, M705_ZONE_RIGHT,
 * and a preprocessor environment not present in firestaff_m10).
 * To unblock the modern backend without dragging in the legacy
 * frontend, the globals are moved here as a single translation
 * unit that the firestaff_m10 library can build.
 *
 * Source-lock: include/image_backend_pc34_compat.h:4
 *   `extern unsigned short G2157_;`
 *   (paired with the legacy definition in
 *    src/shared/image_frontend_pc34.c:19 which is NOT in
 *    firestaff_m10 lib).
 *
 * This is a pre-existing module-decomposition fix; before this
 * file existed, the firestaff_m10 lib had unresolved symbol
 * _G2157_ whenever any new M11+M10 unit test was built (see
 * docs/audits/REDMSB_FIRESTAFF_AUDIT_2026-06-16.md, Bug B).
 */

#include "image_backend_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;
