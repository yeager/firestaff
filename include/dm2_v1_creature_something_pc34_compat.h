#ifndef DM2_V1_CREATURE_SOMETHING_PC34_COMPAT_H
#define DM2_V1_CREATURE_SOMETHING_PC34_COMPAT_H

/*
 * DM2 v1 PC 3.4 creature animation-frame reader + CCM mticks delta —
 * bounded slice.
 *
 * Binds two source functions:
 *
 *   DM2_GET_CREATURE_ANIMATION_FRAME (skproject/SKULLWIN/c_creature.cpp:
 *   3217-3278) with its DM2_4FCC tail (c_creature.cpp:3285-3378) — the
 *   GDAT-backed animation-frame stepper.  Data path:
 *     - dtRaw8/0xfb (GDAT_CREATURE_ANIM_ATTRIBUTION): 4-byte rows; scan
 *       for word@0 == command or the 0xffff terminator; the matched
 *       row's word@2 is the sequence base written to *io_adj_base
 *       (c_creature.cpp:3241-3252);
 *     - dtRaw7/0xfc (GDAT_CREATURE_ANIM_INFO_SEQUENCE): 4-byte rows
 *       indexed by (sequence base + frame offset);
 *     - the AIDefinition bit0 gate (c_creature.cpp:3256 jnz_test8 on
 *       aidef byte@0 & 1) is resolved DATA-BACKED through the proven
 *       session AI table (dm2_v1_creature_ai_spec_def — the GDAT
 *       extended-mode loader's provenance chain, c_record.cpp:1351-1354);
 *     - bit0 set (static): count the 0xfc rows from the sequence base up
 *       to and including the first row whose byte@1 high nibble is 0;
 *       the frame word is count | 0x9000 when argl1 == 0, else count |
 *       (((argl1 & 0x3f) << 6) | 0x8000) (c_creature.cpp:3259-3276).  The
 *       anim-row out pointer is NOT written on this path (the caller's
 *       NULL state persists);
 *     - bit0 clear (dynamic): *io_frame_word = -1 and DM2_4FCC advances
 *       the frame: frame 0xffff restarts at 0; otherwise the current
 *       row's byte@1 high nibble pre-advances the frame (0 nibble ends
 *       the sequence with return 0).  The walk then skips rows
 *       probabilistically — byte@1 low nibble 0xf never skips, otherwise
 *       skip when nibble < (DM2_RAND() & 0xf) — and stops at a row whose
 *       byte@3 high nibble + ((byte@3 & 0xc) >> 2) decides the return
 *       (0 = sequence end).  *out_anim_row points at the stopped 4-byte
 *       GDAT row (loader-owned); *io_frame_word = the walked offset
 *       (c_creature.cpp:3285-3378).  DM2_RAND draws consume the session
 *       LCG stream (c_random.cpp:13-19).
 *   Fail-closed where the source would dereference an absent table:
 *   attribution missing returns 0 exactly like the source's SPX -1
 *   guard; a missing 0xfc info table, an unterminated attribution scan,
 *   or any row walk past the loaded table span receipts gdat_missing /
 *   table_oob and returns 0 instead of reading out of bounds.
 *
 *   DM2_CREATURE_SOMETHING_1c9a_0a48 (skproject/SKULLWIN/c_1c9a.cpp:
 *   5434-5672) — the per-CCM-tick animation reader whose result the
 *   DM2_PROCEED_CCM end re-queue consumes as its mticks delta
 *   (c_ai.cpp:5614).  s350 context enters as parameters (the s350
 *   struct itself stays host-owned):
 *     - record_handle is s350.v1e054e (the SPX_Creature DB4 record);
 *       byte@4 = creature type (vb_0c), word@0xa = flag word, word@0xc =
 *       packed coordinates;
 *     - the CAII slot (record byte@5) is s350.creatures: byte@7 =
 *       frame/direction byte (mutated), byte@0x1a = CCM command;
 *     - the AIDefinition row (s350.v1e0552, c_ai.cpp:5867) resolves
 *       data-backed through dm2_v1_creature_ai_spec_def: byte@9 = jitter
 *       bounds (vb_10), byte@0 bit0 = the GAF static gate / parl01
 *       selector, byte@1 bit 0x10 = the dying-mode triple guard;
 *     - adj is s350.v1e055e (the DM2_query_1c9a_02c3 pair, c_ai.cpp:
 *       5868-5869) — two i16s updated by GAF and written back verbatim
 *       (c_1c9a.cpp:5549-5550);
 *     - anim_row_io is s350.v1e055a: when NULL the source fetches the
 *       row through GAF with parl01 = 0 (aidef bit0 clear) or the
 *       record's packed word@0xc (bit0 set); when GAF leaves it NULL the
 *       source falls back to a zeroed 4-byte local (c_1c9a.cpp:5478-5481)
 *       and resets the pointer to NULL before returning
 *       (c_1c9a.cpp:5668-5670) — receipted anim_fallback, never
 *       fabricated content: the zero row is the source's own fallback;
 *     - frame byte dance (c_1c9a.cpp:5484-5537): when row byte@3 & 1 and
 *       the CCM command is not 0x23/0x24/0x25, slot byte@7 is masked to
 *       0xc0 and two jitter draws OR in (vb_10 & 3) and ((vb_10 >> 2) &
 *       3) << 3, each RAND16(n) with a RANDBIT sign flip masked to 0x7;
 *       row byte@3 & 2 then sets/clears bit 0x40 on a RANDBIT draw;
 *     - the noise request (c_1c9a.cpp:5539-5545): when (row byte@0 &
 *       0x7f) != 0x7f the source runs DM2_QUEUE_NOISE_GEN1(0xf, type,
 *       index, 0x46, 0x80, xA, yA, 1) — the noise system is unproven, so
 *       the request is receipted (noise_would_queue + index), never
 *       simulated;
 *     - the delta arithmetic (c_1c9a.cpp:5547-5667), all branches bound
 *       verbatim including the dead 75x%100 modulo whose quotient the
 *       source keeps (c_1c9a.cpp:5629-5637), the map/home comparisons
 *       (ddat.v1d3248/v1e08d6 as parameters), the table1d607e uc[2] & 1
 *       probe (per-module source-locked copy, mdata.c:1564-1613) and the
 *       final signed 16-bit truncation (c_1c9a.cpp:5666).  The source
 *       indexes table1d607e with s350.v1e0584 unchecked — a v1e0584
 *       outside the proven 0x2f span fails closed
 *       (gdat_w1_out_of_span) only when that probe is actually reached.
 *     Returns gametick + delta exactly like the source
 *     (c_1c9a.cpp:5667).
 *
 * `rng` is the session's proven c_random LCG stream (DM2_V1_DropRng,
 * c_random.cpp:13-47) and is consumed in exact source draw order.  NULL
 * receipts rng_unbound and fails closed before the first draw.
 */

#include <stdint.h>

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_caii_alloc_pc34_compat.h"
#include "dm2_v1_creature.h"
#include "dm2_v1_drops.h"

#define DM2_V1_ANIM_DELTA_BAND_BASE 0       /* RAND16 + base nibble */
#define DM2_V1_ANIM_DELTA_BAND_DYING_X3 1   /* mode 0x13 + b03: *3 */
#define DM2_V1_ANIM_DELTA_BAND_FLEE_X4 2    /* w_0a 0x8000 band: *4 + bit */
#define DM2_V1_ANIM_DELTA_BAND_75PCT 3      /* w_0a & 8: max(1, 75x/100) */
#define DM2_V1_ANIM_DELTA_BAND_SMALL 4      /* 0x4000 && x < 3: max(1, x) */
#define DM2_V1_ANIM_DELTA_BAND_MAP_X2 5     /* v1e0238 != 0: *2 (or *4) */
#define DM2_V1_ANIM_DELTA_BAND_PLAIN 6      /* no modifier */
#define DM2_V1_ANIM_DELTA_BAND_BIG_MIN 7    /* w_0a & 0x40: min(1, hi) */

typedef struct {
  int valid;
  int creature_type;         /* vb_04 */
  int command;               /* RG51w (slot byte@0x1a or 0x11) */
  int gdat_missing;          /* fail-closed: 0xfc info table absent */
  int table_oob;             /* fail-closed: walk past the table span */
  int aidef_unknown;         /* fail-closed: no session AI row */
  int attribution_found;     /* word@0 == command / 0xffff terminator */
  int attribution_base;      /* row word@2 -> *io_adj_base */
  int static_path;           /* aidef byte@0 bit0 set */
  int frame_word;            /* *io_frame_word result */
  int anim_row_set;          /* *out_anim_row points at a GDAT row */
  int sequence_end;          /* 4FCC ended the sequence (return 0) */
  int rand_draws;            /* session LCG draws consumed */
  int return_value;          /* the source RG1L */
  char source_evidence[512];
} DM2_V1_CreatureAnimFrameReceipt;

/*
 * DM2_GET_CREATURE_ANIMATION_FRAME + DM2_4FCC bounded slice.  `rng` may
 * be NULL only when no draw can occur (static path or a sequence that
 * ends without a skip roll); a needed draw with NULL rng fails closed
 * (receipt.valid == 0, return 0).  `io_adj_base` / `io_frame_word` /
 * `out_anim_row` mirror the source's ebxpw/ecxpw/argpp0.  `argl1` is the
 * source's parl01 (0 or packed coordinates).  Returns the source RG1L.
 */
int dm2_v1_creature_get_animation_frame(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_DropRng *rng,
    int creature_type,
    int command,
    uint16_t *io_adj_base,
    int16_t *io_frame_word,
    const uint8_t **out_anim_row,
    int32_t argl1,
    DM2_V1_CreatureAnimFrameReceipt *receipt);

/*
 * Standalone DM2_4FCC (skproject/SKULLWIN/c_creature.cpp:3285-3378) —
 * the same frame walk GAF runs on its dynamic tail, exposed for the
 * call sites that already own the sequence base (c_ai.cpp:5388 inside
 * DM2_13e4_0982's !flag branch; DM2_50CB c_ai.cpp:5275-5338 is the
 * deterministic twin bound separately by the CCM loop module).
 * `adj_base` is the caller-owned sequence base (word@s350.v1e055e),
 * `io_frame_word` the current frame word (0xffff restarts at 0),
 * `out_anim_row` the stopped 4-byte GDAT row (loader-owned).  Every
 * DM2_RAND draw consumes the session LCG (c_random.cpp:13-19); a
 * needed draw with NULL rng fails closed (receipt.valid == 0).
 * Returns the source RG1L.
 */
int dm2_v1_creature_anim_4fcc(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_DropRng *rng,
    int creature_type,
    uint16_t adj_base,
    int16_t *io_frame_word,
    const uint8_t **out_anim_row,
    DM2_V1_CreatureAnimFrameReceipt *receipt);

typedef struct {
  int valid;
  int32_t result;            /* gametick + delta (c_1c9a.cpp:5667) */
  int not_creature_db;       /* handle is not a DB4 creature record */
  int no_slot;               /* record byte@5 == 0xff (s350.creatures NULL) */
  int aidef_unknown;         /* no session AI row for the record's type */
  int rng_unbound;           /* a source draw had no session stream */
  int gdat_missing;          /* fail-closed inside GAF */
  int table_oob;             /* fail-closed inside GAF */
  int gdat_w1_out_of_span;   /* v1e0584 beyond table1d607e's 0x2f span */
  int creature_type;         /* vb_0c */
  int command;               /* slot byte@0x1a */
  int anim_fetched;          /* GAF ran (anim row was NULL) */
  int anim_fallback;         /* the source's zeroed 4-byte fallback row */
  int gaf_return;            /* GAF RG1L (-1 when not called) */
  int adj_base_after;        /* vw_04 written back to v1e055e */
  int frame_word_after;      /* vw_08 written back to v1e055e */
  int jitter_applied;        /* row byte@3 & 1 and no mode guard */
  int mode_guard;            /* command in {0x23, 0x24, 0x25} */
  int frame_byte_before;     /* slot byte@7 before the dance */
  int frame_byte_after;      /* slot byte@7 after the write */
  int rand_bit6_applied;     /* row byte@3 & 2 drew for bit 0x40 */
  int noise_would_queue;     /* (row byte@0 & 0x7f) != 0x7f */
  int noise_index;           /* row byte@0 & 0x7f */
  int delta;                 /* RG1L before the gametick add */
  int delta_band;            /* DM2_V1_ANIM_DELTA_BAND_* */
  int rand_draws;            /* session LCG draws consumed */
  char source_evidence[512];
} DM2_V1_CreatureSomethingReceipt;

/*
 * DM2_CREATURE_SOMETHING_1c9a_0a48 bounded slice (c_1c9a.cpp:5434-5672).
 * Returns the source result (gametick + delta) when receipt.valid == 1;
 * returns 0 with receipt.valid == 0 for every fail-closed path.  `adj`
 * is the two-i16 s350.v1e055e pair (in/out); `anim_row_io` is the
 * s350.v1e055a row pointer (in/out — the source's fallback reset writes
 * NULL back).  `map_current`/`map_home` stand in for ddat.v1d3248 /
 * ddat.v1e08d6, `v1e0238` and `savegame_b03` for the ddat globals,
 * `v1e0584` for s350.v1e0584 (the GDAT word@1 table index, -1 unset),
 * `timer_x`/`timer_y` for s350.v1e0562's getxA/getyA noise coordinates,
 * and `game_tick` for timdat.gametick.  When `receipt` is non-NULL it
 * always receives the audit record.
 */
int32_t dm2_v1_creature_something_1c9a_0a48(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_CaiiArray *caii,
    const DM2_V1_AssetLoader *loader,
    DM2_V1_DropRng *rng,
    int16_t record_handle,
    int16_t *adj,
    const uint8_t **anim_row_io,
    int map_current,
    int map_home,
    int32_t v1e0238,
    int savegame_b03,
    int v1e0584,
    int timer_x,
    int timer_y,
    unsigned long game_tick,
    DM2_V1_CreatureSomethingReceipt *receipt);

#endif
