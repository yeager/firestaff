#ifndef FIRESTAFF_REDMCSB_F1042_UNINSTALL_INTERRUPT_HANDLER_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1042_UNINSTALL_INTERRUPT_HANDLER_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * IO.C F1042_ removes the X68000 INT1_05 interrupt handler only in the
 * MEDIA692_X31J route. PC 3.4 supplies neither that handler nor a portable
 * host adapter, so this boundary does not simulate its removal.
 */
bool redmcsb_f1042_uninstall_interrupt_handler_pc34_compat(void);

const char *redmcsb_f1042_uninstall_interrupt_handler_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1042_UNINSTALL_INTERRUPT_HANDLER_PC34_COMPAT_H */
