#ifndef FIRESTAFF_REDMCSB_F1023_PRINT_STRING_H
#define FIRESTAFF_REDMCSB_F1023_PRINT_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*redmcsb_f1023_console_print_fn)(void *context,
                                                const char *string);

/*
 * ReDMCSB F1023 passes its supplied string directly to the X68000 console
 * PRINT service once.
 */
void redmcsb_f1023_print_string(redmcsb_f1023_console_print_fn console_print,
                                void *context,
                                const char *string);

const char *redmcsb_f1023_print_string_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1023_PRINT_STRING_H */
