#ifndef FIRESTAFF_SCREENSHOT_M11_H
#define FIRESTAFF_SCREENSHOT_M11_H

/*
 * screenshot_m11 — Engine-side screenshot capture.
 *
 * Writes a 24-bit BMP from an indexed VGA framebuffer + 256-entry
 * RGB palette.  Output filename is firestaff-YYYYMMDD-HHMMSS.bmp.
 *
 * Hotkeys (wired by main_loop_m11):
 *   F12        — capture immediately with HUD visible
 *   Ctrl+F12   — enter "screenshot mode" (pause + hide HUD) until F12
 *                or Esc is pressed.
 *
 * Default outputDir is ~/.firestaff/screenshots/, overridable from
 * the launcher config (screenshotPath).
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Capture a 24-bit BMP from an indexed framebuffer + 256-entry palette.
 *
 *   framebuffer : width*height bytes, each is a palette index (0..255).
 *   width/height: positive image dimensions.
 *   palette     : pointer to 256 * 3 bytes (R,G,B,R,G,B,...) in 0..255.
 *                 If NULL, a grayscale fallback is used.
 *   outputDir   : directory to write into (created if missing).  NULL =>
 *                 ~/.firestaff/screenshots/.
 *
 * Returns 1 on success, 0 on any error.  On success, writes the full
 * output path into `outPath` (if non-NULL, with capacity outPathCap).
 */
int M11_Screenshot_Capture(const unsigned char* framebuffer,
                           int width, int height,
                           const unsigned char* palette,
                           const char* outputDir,
                           char* outPath, int outPathCap);

/* Capture using the m11 indexed framebuffer + current VGA palette.
 * Convenience helper that calls M11_Screenshot_Capture for you. */
int M11_Screenshot_CaptureCurrent(const char* outputDir,
                                  char* outPath, int outPathCap);

/* Resolve the default output directory (~/.firestaff/screenshots).  The
 * directory is created if missing.  Returns a static buffer. */
const char* M11_Screenshot_DefaultDir(void);

/* Screenshot-mode runtime toggle (Ctrl+F12).  When active, HUD overlays
 * should hide themselves and the game treats itself as paused.  Modules
 * query this via the getter below. */
void M11_Screenshot_ToggleMode(void);
int  M11_Screenshot_IsModeActive(void);
void M11_Screenshot_ExitMode(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_SCREENSHOT_M11_H */
