#ifndef FIRESTAFF_REDMCSB_F1035_UNREFERENCED_H
#define FIRESTAFF_REDMCSB_F1035_UNREFERENCED_H

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB IO2.C F1035 constructs a one-character string for F1034_. */
typedef void (*redmcsb_f1035_f1034_fn)(void *context, const char *string);

void redmcsb_f1035_unreferenced(redmcsb_f1035_f1034_fn f1034,
                                 void *context,
                                 int character);

const char *redmcsb_f1035_unreferenced_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1035_UNREFERENCED_H */
