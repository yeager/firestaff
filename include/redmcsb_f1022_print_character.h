#ifndef FIRESTAFF_REDMCSB_F1022_PRINT_CHARACTER_H
#define FIRESTAFF_REDMCSB_F1022_PRINT_CHARACTER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*redmcsb_f1022_console_print_fn)(void *context,
                                                const char *string);

/*
 * ReDMCSB F1022 writes character into byte zero of its persistent two-byte
 * string, then invokes the X68000 console PRINT service once.
 */
void redmcsb_f1022_print_character(redmcsb_f1022_console_print_fn console_print,
                                   void *context,
                                   char character);

const char *redmcsb_f1022_print_character_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1022_PRINT_CHARACTER_H */
