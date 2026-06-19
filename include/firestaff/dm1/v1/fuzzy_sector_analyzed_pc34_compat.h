#ifndef FIRESTAFF_DM1_V1_FUZZY_SECTOR_ANALYZED_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_FUZZY_SECTOR_ANALYZED_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0031_i_Graphic562_FuzzySectorAnalyzed_CPSE.
 *
 * G0031 is a single int16_t copy-protection state flag tracking
 * whether the floppy-disk fuzzy sector has been successfully
 * analyzed. Init value (DATA.C:39 + DATA.C:579): C00255_FALSE (0).
 * Loaded from graphic #562 at startup.
 *
 * Read sites:
 * - CLIKMENU.C:366/491/559 — check G0031 to gate copy-protection
 *   fuzzy-sector re-reads in the click-menu flow.
 * - COPYPRO6.C:69/99 — set G0031 = C00136_TRUE after successful
 *   fuzzy-sector analysis.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798+
 * (Graphics.dat init-table gates). This gate is a non-mirror-
 * candidate contract for the G0031 fuzzy-sector-analyzed flag.
 */

typedef struct DM1_V1_FuzzySectorAnalyzedResultPc34 {
    int accepted;
    int assertionCount;
    int tableSize;
    int tableMatchesDeclaration;
    int initializedFalse;
    int valueIsC00255;
    int valueInRange;
    int lookupFunctionCorrect;
} DM1_V1_FuzzySectorAnalyzedResultPc34;

int
dm1_v1_fuzzy_sector_analyzed_get_pc34(void);

int
dm1_v1_fuzzy_sector_analyzed_size_pc34(void);

int
dm1_v1_fuzzy_sector_analyzed_run_pc34(
    DM1_V1_FuzzySectorAnalyzedResultPc34 *out);

#endif