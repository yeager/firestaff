#ifndef FIRESTAFF_REDMCSB_F1048_SETJMP_H
#define FIRESTAFF_REDMCSB_F1048_SETJMP_H

#include <setjmp.h>

/*
 * ReDMCSB names the native setjmp entry point F1048_setjmp for the media
 * routes where the alias is enabled.  Preserve the standard macro call so
 * its required calling context is not hidden behind a function wrapper.
 */
#define F1048_setjmp(environment) setjmp(environment)

const char *redmcsb_f1048_setjmp_source_evidence(void);

#endif /* FIRESTAFF_REDMCSB_F1048_SETJMP_H */
