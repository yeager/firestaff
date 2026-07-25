#ifndef FIRESTAFF_CSB_V1_F0426_DIALOG_IS_MESSAGE_ON_TWO_LINES_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0426_DIALOG_IS_MESSAGE_ON_TWO_LINES_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ReDMCSB DIALOG.C F0426 (CSB 2.0/2.1, CHANGE7_35): a dialog message
 * longer than 30 monospaced 6-pixel glyphs is split at the first ASCII space
 * at or to the right of its character midpoint. The caller owns writable
 * output buffers large enough for copies of message and its suffix.
 */
int csb_v1_f0426_dialog_is_message_on_two_lines_pc34_compat(
    char *message,
    char *part1,
    char *part2);

#define F0426_DIALOG_IsMessageOnTwoLines \
    csb_v1_f0426_dialog_is_message_on_two_lines_pc34_compat

const char *csb_v1_f0426_dialog_is_message_on_two_lines_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
