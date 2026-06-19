#ifndef FIRESTAFF_DM1_V1_INDIRECT_STOP_EXPIRING_EVENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INDIRECT_STOP_EXPIRING_EVENT_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0022_i_Graphic562_IndirectStopExpiringEvent_CPSE.
 *
 * G0022 is a single int16_t flag that tracks whether the
 * "indirect-stop-expiring-event" copy-protection state is
 * active. The variable is loaded from graphic #562 in DATA.C
 * (DATA.C:232 PC 3.4 init, DATA.C:874 Atari init) and
 * always initialized to C00555_FALSE.
 *
 * Read sites:
 * - MOVESENS.C:744 — if G0022 != C00555_FALSE, dispatch the
 *   copy-protection stop event.
 * - MOVESENS.C:746 — BUG0_00 useless comparison (kept for
 *   compatibility).
 * - TIMELINE.C:1922 — set G0022 = C00136_TRUE when the
 *   stop-event ticks.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820/
 * 821/830/831/832/833 (Graphics.dat init-table gates batches 1+2+3+
 * 4+5+6+7+8+9+10+11+12). This gate is a non-mirror-candidate
 * contract for the G0022 copy-protection state flag.
 */

typedef struct DM1_V1_IndirectStopExpiringEventResultPc34 {
    int accepted;
    int assertionCount;
    int tableSize;
    int tableMatchesDeclaration;
    int initializedFalse;
    int valueIsC00555;
    int valueInRange;
    int lookupFunctionCorrect;
} DM1_V1_IndirectStopExpiringEventResultPc34;

int
dm1_v1_indirect_stop_expiring_event_get_pc34(void);

int
dm1_v1_indirect_stop_expiring_event_size_pc34(void);

int
dm1_v1_indirect_stop_expiring_event_run_pc34(
    DM1_V1_IndirectStopExpiringEventResultPc34 *out);

#endif