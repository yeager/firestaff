#include "redmcsb_f1052_wait_for_scan_line_pc34_compat.h"

void redmcsb_f1052_wait_for_scan_line_pc34_compat(int16_t scan_line)
{
    (void)scan_line;
}

const char *redmcsb_f1052_wait_for_scan_line_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/FILLBOX.C:5-33 encloses "
           "F1052_WaitForScanLine in "
           "MEDIA746_A36M_A31E_A31M_A33M_A35E_A35M. FILLBOX.C:20-31 "
           "adds 44 to the requested scan line, shifts it by eight, and "
           "polls Amiga VPOSR/VHPOSR at 0xDFF004 with mask 0x1FF00 until "
           "the vertical position reaches that value. FILLBOX.C:565, 709, "
           "731, 746, and 759 call it only from that Amiga route. No PC "
           "3.4 branch or portable timing adapter is supplied.";
}
