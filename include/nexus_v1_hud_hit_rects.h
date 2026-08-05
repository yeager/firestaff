
#ifndef NEXUS_V1_HUD_HIT_RECTS_H
#define NEXUS_V1_HUD_HIT_RECTS_H
#include <stddef.h>
#include <stdint.h>

/* Nexus HUD hit-test rectangles from DM.BIN yam\menuctrl.c at 0x038000.
 * Each entry is a screen rectangle (x1,y1,x2,y2) as big-endian uint16
 * quads.  The ring menu controller uses these for Saturn pad/mouse
 * click region detection.
 * Source: DM.BIN 0x038000, Saturn binary (555,144 bytes). */

typedef struct {
    int16_t x1, y1, x2, y2;
} Nexus_HitRect;

/* Named hit-region indices. */
#define NEXUS_HIT_SIDEBAR_BUTTON_0     0   /* (294,104)-(310,120) 16×16 */
#define NEXUS_HIT_SIDEBAR_BUTTON_1     1   /* (277, 72)-(293, 88) 16×16 */
#define NEXUS_HIT_SIDEBAR_BUTTON_2     2   /* (277, 91)-(293,107) 16×16 */
#define NEXUS_HIT_SIDEBAR_BUTTON_3     3   /* (277,110)-(293,126) 16×16 */
#define NEXUS_HIT_SIDEBAR_BUTTON_4     4   /* (277,136)-(293,152) 16×16 */
#define NEXUS_HIT_SIDEBAR_BUTTON_5     5   /* (253, 80)-(269, 96) 16×16 */
#define NEXUS_HIT_SIDEBAR_BUTTON_6     6   /* (253,136)-(269,152) 16×16 */
#define NEXUS_HIT_VIEWPORT             7   /* (144, 72)-(240,200) 96×128 */
#define NEXUS_HIT_SIDEBAR_PANEL        8   /* (251, 70)-(295,137) 44×67 */
#define NEXUS_HIT_COMPASS             10   /* ( 14,  6)-( 74, 45) 60×39 */
#define NEXUS_HIT_LOWER_VIEWPORT      12   /* (144,136)-(240,200) 96×64 */
#define NEXUS_HIT_DUNGEON_TEXT        27   /* (  8, 48)-(136,136) 128×88 */
#define NEXUS_HIT_FULL_GAME_SCREEN    29   /* (  0, 47)-(320,213) 320×166 */
#define NEXUS_HIT_MOVEMENT_PAD        25   /* ( 27,142)-(129,207) 102×65 */

#define NEXUS_HIT_RECT_COUNT          40
#define NEXUS_HIT_RECT_DM_BIN_OFFSET  0x38000U
#define NEXUS_HIT_RECT_ENTRY_BYTES    8U

/* Retrieve the hit rectangle table.  Returns NEXUS_HIT_RECT_COUNT. */
int nexus_v1_hud_hit_rects(const Nexus_HitRect **out);

/* Parse the retail ring-menu rectangles directly from DM.BIN.  The static
 * accessor remains compatibility-only; runtime handoffs must use this
 * source-bound form when a mounted Saturn package is available. */
int nexus_v1_hud_hit_rects_parse_dm_bin(
    const uint8_t *data,
    size_t data_size,
    Nexus_HitRect *out,
    size_t out_capacity,
    size_t *out_count);

/* Hit-test a screen coordinate against all rectangles.
 * Returns the index of the first matching rectangle, or -1 if none. */
int nexus_v1_hud_hit_test(int screen_x, int screen_y);

#endif
