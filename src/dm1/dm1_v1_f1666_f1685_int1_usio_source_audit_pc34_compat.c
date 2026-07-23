#include "dm1_v1_f1666_f1685_int1_usio_source_audit_pc34_compat.h"

static const DM1_V1_F1666F1685SourceAuditPc34 k_audit[] = {
    { 1666u, "INT1.C:88 F1666_UninstallVerticalBlankHandlerManager", "fail_closed: X68000 vertical-blank interrupt manager", 1, 1, 1, 1 },
    { 1667u, "INT1.C:104 F1667_ExecuteVerticalBlankHandlers", "fail_closed: X68000 vertical-blank handler execution", 1, 1, 1, 1 },
    { 1668u, "INT1.C:125 F1668_InstallTimerDHandlerManager", "fail_closed: X68000 Timer-D interrupt manager", 1, 1, 1, 1 },
    { 1669u, "INT1.C:162 F1669_UninstallTimerDHandlerManager", "fail_closed: X68000 Timer-D interrupt manager", 1, 1, 1, 1 },
    { 1670u, "INT1.C:177 F1670_ExecuteTimerDHandlers", "fail_closed: X68000 Timer-D handler execution", 1, 1, 1, 1 },
    { 1671u, "INT1.C:195 F1671_InstallSampleHandlerManager", "fail_closed: X68000 sample-handler manager", 1, 1, 1, 1 },
    { 1672u, "INT1.C:205 F1672_UninstallSampleHandlerManager", "fail_closed: X68000 sample-handler manager", 1, 1, 1, 1 },
    { 1673u, "INT1.C:215 F1673_ExecuteSampleHandlers_Unreferenced", "fail_closed: source-unreferenced X68000 sample handler", 1, 1, 1, 1 },
    { 1674u, "INT1.C:226 F1674_AddInterruptHandler", "fail_closed: X68000 supervisor interrupt registry", 1, 1, 1, 1 },
    { 1675u, "INT1.C:272 F1675_InstallInterruptHandler", "fail_closed: X68000 interrupt dispatcher", 1, 1, 1, 1 },
    { 1676u, "INT1.C:294 F1676_UninstallInterruptHandler", "fail_closed: X68000 supervisor interrupt registry", 1, 1, 1, 1 },
    { 1677u, "INT1.C:334 F1677_UninstallInterruptHandler", "fail_closed: X68000 interrupt dispatcher", 1, 1, 1, 1 },
    { 1678u, "INT1.C:353 F1678_UninstallAllHandlerManagers", "fail_closed: X68000 interrupt manager teardown", 1, 1, 1, 1 },
    { 1679u, "INT1.C:361 F1679_INT1_04_InstallInterruptHandler", "fail_closed: X68000 INT1 library ABI", 1, 1, 1, 1 },
    { 1680u, "INT1.C:373 F1680_INT1_05_UninstallInterruptHandler", "fail_closed: X68000 INT1 library ABI", 1, 1, 1, 1 },
    { 1681u, "INT1.C:381 F1681_INT1_06_", "fail_closed: source-empty X68000 INT1 library vector", 1, 1, 1, 1 },
    { 1682u, "INT1.C:388 F1682_", "fail_closed: X68000 INT1 library open ABI", 1, 1, 1, 1 },
    { 1683u, "INT1.C:402 F1683_INT1_03_Expunge", "fail_closed: X68000 INT1 library expunge ABI", 1, 1, 1, 1 },
    { 1684u, "USIO1.C:80 F1684_GetMouseStatus", "dm1_v1_input_command_queue_pc34_compat", 1, 1, 1, 1 },
    { 1685u, "USIO1.C:180 F1685_InstallMouseInterruptHandler", "docs/reference/audits/REDMCSB_MISSING_PLATFORM_BOUNDARIES.tsv platform boundary", 1, 1, 1, 1 }
};

const DM1_V1_F1666F1685SourceAuditPc34 *
dm1_v1_f1666_f1685_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F1666F1685SourceAuditPc34 *
dm1_v1_f1666_f1685_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f1666_f1685_source_audit_evidence_pc34(void)
{
    return "ReDMCSB INT1.C and USIO1.C are the authority for F1666-F1685. "
           "INT1 and mouse-interrupt routes require X68000 IOCS and interrupt state "
           "and remain fail closed without authentic raw PC34 material. F1684 retains "
           "its existing caller-owned input owner. The audit does not render or "
           "synthesize UI or timing paths.";
}
