#ifndef FIRESTAFF_REDMCSB_F1053_PRE_F0380_COMMAND_PROCESS_QUEUE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1053_PRE_F0380_COMMAND_PROCESS_QUEUE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * COMMAND.C F1053 is an Amiga-only container for injected 68k fake code
 * explicitly marked never executed. It has no portable C behavior.
 */
void redmcsb_f1053_pre_f0380_command_process_queue_pc34_compat(void);

void F1053_Pre_F0380_COMMAND_ProcessQueue_CPSC(void);

const char *redmcsb_f1053_pre_f0380_command_process_queue_source_evidence_pc34(
    void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1053_PRE_F0380_COMMAND_PROCESS_QUEUE_PC34_COMPAT_H */
