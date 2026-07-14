#ifndef FIRESTAFF_REDMCSB_F0936_LAUNCH_PROCESS_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0936_LAUNCH_PROCESS_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Host boundaries for ReDMCSB EXEC.C F0936's AmigaDOS/Exec calls. */
typedef void *(*redmcsb_f0936_load_seg_pc34_compat)(void *context,
                                                     const char *name);
typedef void *(*redmcsb_f0936_create_proc_pc34_compat)(
    void *context,
    const char *name,
    long priority,
    void *segment_list,
    long stack_size);
typedef void (*redmcsb_f0936_put_msg_pc34_compat)(void *context,
                                                   void *process,
                                                   void *message);
typedef void (*redmcsb_f0936_wait_port_pc34_compat)(void *context,
                                                     void *reply_port);
typedef void *(*redmcsb_f0936_get_msg_pc34_compat)(void *context,
                                                    void *reply_port);
typedef void (*redmcsb_f0936_unload_seg_pc34_compat)(void *context,
                                                      void *segment_list);

typedef struct {
    redmcsb_f0936_load_seg_pc34_compat load_seg;
    redmcsb_f0936_create_proc_pc34_compat create_proc;
    redmcsb_f0936_put_msg_pc34_compat put_msg;
    redmcsb_f0936_wait_port_pc34_compat wait_port;
    redmcsb_f0936_get_msg_pc34_compat get_msg;
    redmcsb_f0936_unload_seg_pc34_compat unload_seg;
} redmcsb_f0936_launch_process_ops_pc34_compat;

/*
 * ReDMCSB EXEC.C F0936_LaunchProcess. The supplied callbacks and opaque
 * message/reply-port objects represent the original AmigaDOS/Exec boundary.
 * As in the source, this routine assumes every callback succeeds.
 */
void redmcsb_f0936_launch_process_pc34_compat(
    const char *name,
    bool wait_process_completion,
    void *message,
    void *reply_port,
    const redmcsb_f0936_launch_process_ops_pc34_compat *ops,
    void *context);

const char *redmcsb_f0936_launch_process_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0936_LAUNCH_PROCESS_PC34_COMPAT_H */
