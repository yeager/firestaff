#include "dm1_v1_f0926_f0945_platform_loader_source_audit_pc34_compat.h"

static const DM1_V1_F0926F0945SourceAuditPc34 k_audit[] = {
    { 926u, "PRIM1.C:297 F0926_FLOPPY_ForceMediaChangeDetection", "redmcsb_f0926_floppy_force_media_change_detection_pc34_compat", 1, 1, 1, 1 },
    { 927u, "PRIM1.C:398 F0927_PrintLoaderError", "redmcsb_f0927_print_loader_error_pc34_compat", 1, 1, 1, 1 },
    { 928u, "PRIM1.C:423 F0928_PrintOSError", "redmcsb_f0928_print_os_error_pc34_compat", 1, 1, 1, 1 },
    { 929u, "PRIM1.C:448 F0929_PRIM_05_FTL_Load", "redmcsb_f0929_prim_ftl_load_pc34_compat", 1, 1, 1, 1 },
    { 930u, "PRIM1.C:649 F0930_GetHeaderChecksum", "redmcsb_f0930_get_header_checksum", 1, 1, 1, 1 },
    { 931u, "PRIM1.C:672 F0931_ChecksumWords", "f0931_f0932_prim_checksums_pc34_compat", 1, 1, 1, 1 },
    { 932u, "PRIM1.C:688 F0932_ChecksumBytes", "f0931_f0932_prim_checksums_pc34_compat", 1, 1, 1, 1 },
    { 933u, "PRIM1.C:706 F0933_GetHexStringFromValue", "redmcsb_f0933_get_hex_string_from_value", 1, 1, 1, 1 },
    { 934u, "PRIM1.C:718 F0934_ConvertValueToHexDigits", "redmcsb_f0934_convert_value_to_hex_digits", 1, 1, 1, 1 },
    { 935u, "no numbered F0935 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 936u, "EXEC.C:66 F0936_LaunchProcess", "redmcsb_f0936_launch_process_pc34_compat", 1, 1, 1, 1 },
    { 937u, "EXEC.C:175 F0937_InitAmigaData", "redmcsb_f0937_init_amiga_data_pc34_compat", 1, 1, 1, 1 },
    { 938u, "EXEC.C:275 F0938_CallCloseWorkbench", "redmcsb_f0938_call_close_workbench_pc34_compat", 1, 1, 1, 1 },
    { 939u, "EXEC.C:294 F0939_ReleaseAmigaData", "redmcsb_f0939_release_amiga_data_pc34_compat", 1, 1, 1, 1 },
    { 940u, "EXEC.C:333 F0940_CopyCopperInstructions", "redmcsb_f0940_copy_copper_instructions_pc34_compat", 1, 1, 1, 1 },
    { 941u, "no numbered F0941 body in ReDMCSB corpus", "fail_closed: no source owner", 0, 1, 1, 1 },
    { 942u, "EXEC.C:379 F0942_InitDiskResource", "redmcsb_f0942_init_disk_resource_pc34_compat", 1, 1, 1, 1 },
    { 943u, "EXEC.C:397 F0943_ReleaseDiskResource", "redmcsb_f0943_release_disk_resource_pc34_compat", 1, 1, 1, 1 },
    { 944u, "EXEC.C:408 F0944_ Exec vector patch check", "redmcsb_f0944_exec_vector_patch_check_pc34_compat", 1, 1, 1, 1 },
    { 945u, "SOUND.C:425 F0945_InitAudioData", "redmcsb_f0945_init_audio_data_pc34_compat", 1, 1, 1, 1 }
};

const DM1_V1_F0926F0945SourceAuditPc34 *
dm1_v1_f0926_f0945_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F0926F0945SourceAuditPc34 *
dm1_v1_f0926_f0945_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f0926_f0945_source_audit_evidence_pc34(void)
{
    return "ReDMCSB PRIM1.C, EXEC.C, and SOUND.C are the authority for "
           "F0926-F0945. F0935 and F0941 have no numbered source body in the "
           "audited corpus; loader, floppy, Amiga Exec, copper, disk.resource, "
           "and audio.device routes remain fail closed without verified PC34 "
           "material. The audit does not render or synthesize UI or timing paths.";
}
