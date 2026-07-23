#include "csb_v1_f1846_f1885_hint_io_source_audit_pc34_compat.h"

#define NONE(number) { number##u, "no numbered F" #number " body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 }
#define BLOCK(number, anchor, reason) { number##u, anchor, reason, 1, 1, 1, 1 }

static const CSB_V1_F1846F1885SourceAuditPc34 k_audit[] = {
    NONE(1846), NONE(1847),
    BLOCK(1848, "HINTIORQ.C F1848_OpenOrCreateFile", "fail_closed: no authenticated CSB PC34 hint file owner"),
    BLOCK(1849, "HINTIORQ.C F1849_DeleteFile", "fail_closed: no CSB PC34 destructive hint file route"),
    BLOCK(1850, "HINTIORQ.C F1850_RenameFile", "fail_closed: no CSB PC34 hint file rename route"),
    BLOCK(1851, "HINTIORQ.C F1851_GetDirectoryFileList", "fail_closed: no authenticated CSB PC34 directory-list owner"),
    NONE(1852), NONE(1853), NONE(1854),
    BLOCK(1855, "HINTLOAD.C; HINTGRAP.C; HINTHTC.C; HINTFLOP.C; HINTIORQ.C F1855_GetAvailableIORequestIndex", "fail_closed: no authenticated CSB PC34 I/O-request owner"),
    BLOCK(1856, "HINTLOAD.C; HINTGRAP.C; HINTHTC.C; HINTFLOP.C; HINTIORQ.C F1856_CloseFile", "fail_closed: no authenticated CSB PC34 hint file owner"),
    BLOCK(1857, "HINTLOAD.C; HINTGRAP.C; HINTHTC.C; HINTLZW.C; HINTFLOP.C; HINTIORQ.C F1857_PerformIORequestOperation", "fail_closed: no authenticated CSB PC34 I/O-request owner"),
    BLOCK(1858, "HINTIORQ.C F1858_GetIORequestValue", "fail_closed: no authenticated CSB PC34 I/O-request owner"),
    BLOCK(1859, "HINTLOAD.C; HINTGRAP.C; HINTHTC.C; HINTLZW.C; HINTFLOP.C; HINTIORQ.C F1859_SetIORequestValue", "fail_closed: no authenticated CSB PC34 I/O-request owner"),
    NONE(1860),
    BLOCK(1861, "HINTLOAD.C; HINTHTC.C; HINTFLOP.C; HINTIORQ.C F1861_GetIORequestOutputValue", "fail_closed: no authenticated CSB PC34 I/O-request owner"),
    NONE(1862), NONE(1863), NONE(1864), NONE(1865), NONE(1866), NONE(1867), NONE(1868), NONE(1869),
    BLOCK(1870, "UTSTWKS.C F1870_AddVerticalBlankClearPalette", "fail_closed: no authenticated CSB PC34 palette owner"),
    BLOCK(1871, "UTSTWKS.C F1871_RemoveVerticalBlankClearPalette", "fail_closed: no authenticated CSB PC34 palette owner"),
    BLOCK(1872, "HINTGRAP.C; HINT001.C; HINTSCR.C F1872_LoadGraphics", "fail_closed: no authenticated CSB PC34 hint graphics owner"),
    BLOCK(1873, "HINT008.C; HINTGRAP.C F1873_Pre_F1875_FreeHintGraphics_CPSX", "fail_closed: no authenticated CSB PC34 hint graphics owner"),
    NONE(1874),
    BLOCK(1875, "HINTGRAP.C; HINT001.C; HINTSCR.C F1875_FreeHintGraphics_CPSX", "fail_closed: no authenticated CSB PC34 hint graphics owner"),
    BLOCK(1876, "HINT008.C; HINTGRAP.C F1876_Post_F1875_FreeHintGraphics_CPSX", "fail_closed: no authenticated CSB PC34 hint graphics owner"),
    NONE(1877),
    BLOCK(1878, "HINTGRAP.C; HINTGTXT.C; HINT001.C; HINTSCR.C; HINTPAL.C F1878_LoadGraphic", "fail_closed: no authenticated CSB PC34 graphic owner"),
    BLOCK(1879, "HINTGRAP.C F1879_BlitBitmapFromGraphic", "fail_closed: no authenticated CSB PC34 bitmap owner"),
    BLOCK(1880, "HINTGRAP.C; HINT001.C; HINTSCR.C F1880_LoadBitmapFromGraphic", "fail_closed: no authenticated CSB PC34 bitmap owner"),
    NONE(1881),
    BLOCK(1882, "HINTTEXT.C F1882_PrintTextString", "fail_closed: no authenticated CSB PC34 hint text owner"),
    NONE(1883),
    BLOCK(1884, "HINT006.C; HINTTEXT.C; HINT001.C; HINTHINT.C; HINTSCR.C; HINTINPT.C F1884_Text", "fail_closed: no authenticated CSB PC34 hint text owner"),
    NONE(1885)
};

#undef BLOCK
#undef NONE

const CSB_V1_F1846F1885SourceAuditPc34 *
csb_v1_f1846_f1885_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const CSB_V1_F1846F1885SourceAuditPc34 *
csb_v1_f1846_f1885_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *csb_v1_f1846_f1885_source_audit_evidence_pc34(void)
{
    return "ReDMCSB HINTIORQ.C, HINTLOAD.C, HINTGRAP.C, HINTTEXT.C, HINTSCR.C, "
           "HINTPAL.C, HINT001.C, HINT008.C, HINT006.C, HINTHINT.C, HINTINPT.C, "
           "HINTHTC.C, HINTFLOP.C, HINTLZW.C, and UTSTWKS.C own the identified "
           "F1846-F1885 routes. No CSB PC34 owner is present, so all routes fail "
           "closed without authenticated PC34 material. This audit does not render "
           "or synthesize files, graphics, text, UI, timing, or input.";
}
