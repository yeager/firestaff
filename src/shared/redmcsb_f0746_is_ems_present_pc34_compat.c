#include "redmcsb_f0746_is_ems_present_pc34_compat.h"

int16_t redmcsb_f0746_is_ems_present_pc34_compat(
    const redmcsb_f0746_dos_pc34_compat *dos,
    int16_t do_not_use_ems,
    int16_t initial_si)
{
    static const char emm_device_name[] = "EMMXXXX0";
    int16_t file_handle;
    uint16_t device_information;
    uint8_t output_status;
    int16_t si = initial_si;

    /* STARTUP2.C:106-145, PC 3.4 MEDIA707_I34E_I34M. */
    if (do_not_use_ems != INT16_C(0)) {
        return INT16_C(0);
    }
    if (!dos->open_read_only(dos->context, emm_device_name, &file_handle)) {
        return INT16_C(0);
    }

    if (dos->ioctl_get_device_info(dos->context, file_handle,
                                   &device_information) &&
        (device_information & UINT16_C(0x0080)) != UINT16_C(0) &&
        dos->ioctl_get_output_status(dos->context, file_handle,
                                     &output_status) &&
        output_status != UINT8_C(0)) {
        si = INT16_C(1);
    }

    if (!dos->close(dos->context, file_handle)) {
        return INT16_C(0);
    }
    return si;
}

const char *redmcsb_f0746_is_ems_present_source_evidence_pc34(void)
{
    return "ReDMCSB STARTUP2.C:87-145 (PC 3.4 MEDIA707_I34E_I34M) first "
           "bypasses EMS when G2132_B_DotNotUseEMS is nonzero, then opens "
           "EMMXXXX0 read-only (INT 21h/3D00), requires IOCTL 4400h DX bit "
           "7 and IOCTL 4407h AL nonzero, and closes every opened handle "
           "through INT 21h/3E. The assembly only writes SI=1 on success "
           "and returns SI after a successful close; a close carry returns "
           "AX=0. DEFS.H:6348 declares G2132_B_DotNotUseEMS.";
}
