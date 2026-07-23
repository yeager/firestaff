#include "csb_v1_f1606_f1645_vdi_source_audit_pc34_compat.h"

#define VDI(number) { number##u, "ReDMCSB VDI platform vector F" #number, "fail_closed: no authenticated CSB PC34 VDI platform owner", 1, 1, 1, 1 }
#define NONE(number) { number##u, "no numbered F" #number " body in ReDMCSB callable inventory", "fail_closed: no source owner", 0, 1, 1, 1 }

static const CSB_V1_F1606F1645SourceAuditPc34 k_audit[] = {
    NONE(1606),
    VDI(1607), VDI(1608), VDI(1609), VDI(1610), VDI(1611), VDI(1612), VDI(1613), VDI(1614), VDI(1615), VDI(1616),
    VDI(1617), VDI(1618), VDI(1619), VDI(1620), VDI(1621), VDI(1622), VDI(1623), VDI(1624), VDI(1625), VDI(1626),
    VDI(1627), VDI(1628), VDI(1629), VDI(1630), VDI(1631), VDI(1632), VDI(1633), VDI(1634), VDI(1635),
    NONE(1636), NONE(1637), NONE(1638), NONE(1639), NONE(1640), NONE(1641), NONE(1642), NONE(1643), NONE(1644), NONE(1645)
};

#undef NONE
#undef VDI

const CSB_V1_F1606F1645SourceAuditPc34 *
csb_v1_f1606_f1645_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const CSB_V1_F1606F1645SourceAuditPc34 *
csb_v1_f1606_f1645_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;

    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *csb_v1_f1606_f1645_source_audit_evidence_pc34(void)
{
    return "ReDMCSB UTSTVDI1.C, UTSTVDI2.C, UTSTVDI3.C, HINTTEXT.C, UTSTWKS.C, "
           "and IO.C own VDI vectors F1607-F1635. No CSB PC34 owner is present, "
           "so all workstation, clipping, locator, mouse, keyboard, and callback "
           "routes fail closed without authenticated PC34 material. This audit does "
           "not render or synthesize UI, graphics, timing, input, or callbacks.";
}
