#include "csb_v1_f1806_f1845_memory_io_source_audit_pc34_compat.h"

#define NONE(number) { number##u, "no numbered F" #number " body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 }
#define BLOCK(number, anchor, reason) { number##u, anchor, reason, 1, 1, 1, 1 }

static const CSB_V1_F1806F1845SourceAuditPc34 k_audit[] = {
    NONE(1806), NONE(1807), NONE(1808), NONE(1809), NONE(1810), NONE(1811), NONE(1812), NONE(1813), NONE(1814), NONE(1815),
    NONE(1816), NONE(1817), NONE(1818), NONE(1819), NONE(1820), NONE(1821), NONE(1822), NONE(1823),
    BLOCK(1824, "CEDT005.C; ANIM.C; UTMEMORY.C; HINTMEM.C F1824_AllocateMemoryHeap", "fail_closed: no authenticated CSB PC34 heap owner"),
    BLOCK(1825, "CEDT005.C; ANIM.C; UTMEMORY.C; HINTMEM.C F1825_FreeMemoryHeap", "fail_closed: no authenticated CSB PC34 heap owner"),
    NONE(1826), NONE(1827), NONE(1828), NONE(1829), NONE(1830), NONE(1831), NONE(1832), NONE(1833), NONE(1834), NONE(1835),
    BLOCK(1836, "HINTIORQ.C F1836_GetIORequest", "fail_closed: no authenticated CSB PC34 I/O-request owner"),
    NONE(1837), NONE(1838), NONE(1839), NONE(1840), NONE(1841), NONE(1842),
    BLOCK(1843, "HINTIORQ.C F1843_GetFixedDrvmap", "fail_closed: no authenticated CSB PC34 drive-map owner"),
    NONE(1844), NONE(1845)
};

#undef BLOCK
#undef NONE

const CSB_V1_F1806F1845SourceAuditPc34 *
csb_v1_f1806_f1845_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const CSB_V1_F1806F1845SourceAuditPc34 *
csb_v1_f1806_f1845_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *csb_v1_f1806_f1845_source_audit_evidence_pc34(void)
{
    return "ReDMCSB CEDT005.C, ANIM.C, UTMEMORY.C, HINTMEM.C, and HINTIORQ.C "
           "own the identified F1806-F1845 memory and I/O vectors. No CSB PC34 "
           "owner is present, so all heap, request, and drive-map routes fail "
           "closed without authenticated PC34 material. This audit does not render "
           "or synthesize UI, graphics, timing, input, memory, or file behavior.";
}
