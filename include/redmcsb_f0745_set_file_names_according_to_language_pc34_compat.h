/*
 * ReDMCSB FILENAME.C F0745_SetFileNamesAccordingToLanguage, PC 3.4 route.
 *
 * FILENAME.C:59-105 maps the selected language to a filename marker and
 * replaces every '~' in the five mutable filename buffers. English removes
 * the marker; French and German replace it with 'F' and 'G', respectively.
 */
#ifndef FIRESTAFF_REDMCSB_F0745_SET_FILE_NAMES_ACCORDING_TO_LANGUAGE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0745_SET_FILE_NAMES_ACCORDING_TO_LANGUAGE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    REDMCSB_F0745_LANGUAGE_ENGLISH_PC34_COMPAT = 0,
    REDMCSB_F0745_LANGUAGE_FRENCH_PC34_COMPAT = 1,
    REDMCSB_F0745_LANGUAGE_GERMAN_PC34_COMPAT = 2
} redmcsb_f0745_language_pc34_compat;

/* Each pointer must reference a mutable, NUL-terminated filename buffer. */
typedef struct {
    char *dungeon_file_name;
    char *expansion_set_dungeon_file_name;
    char *bonus_dungeon_file_name;
    char *saved_game_file_name;
    char *saved_game_backup_file_name;
} redmcsb_f0745_file_names_pc34_compat;

/*
 * Executes F0745 for one of the three source language values. The source
 * consumes '~' markers in place, so subsequent calls cannot replace a marker
 * that an earlier call already consumed.
 */
void redmcsb_f0745_set_file_names_according_to_language_pc34_compat(
    redmcsb_f0745_file_names_pc34_compat *file_names,
    redmcsb_f0745_language_pc34_compat language);

const char *redmcsb_f0745_set_file_names_according_to_language_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
