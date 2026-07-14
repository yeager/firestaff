/*
 * ReDMCSB FILENAME.C F0744_ReplaceTildeInFileName, PC 3.4 route.
 *
 * FILENAME.C:60-81 replaces every '~' with the language suffix character.
 * When that character is '\0', it removes each '~' by shifting the remaining
 * string left in place.
 */
#ifndef FIRESTAFF_REDMCSB_F0744_REPLACE_TILDE_IN_FILE_NAME_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0744_REPLACE_TILDE_IN_FILE_NAME_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Applies the PC 3.4 F0744 mutation to a valid writable, null-terminated
 * filename. The source does not validate the pointer or its storage.
 */
void redmcsb_f0744_replace_tilde_in_file_name_pc34_compat(
    char *file_name,
    char replacement_character);

const char *redmcsb_f0744_replace_tilde_in_file_name_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
