/*
 * dm2_v1_invoke_message_pc34_compat.c — DM2_INVOKE_MESSAGE
 * (skproject/SKULLWIN/c_tim_proc.cpp:4332-4367) bounded slice.
 * See the header for the full source-lock documentation.
 */

#include "dm2_v1_invoke_message_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const char dm2_v1_invoke_message_evidence[] =
    "skproject c_tim_proc.cpp:4332-4367 DM2_INVOKE_MESSAGE bounded slice "
    "(setmticks c_timer.h:66 + settype 0x4 + actor RG3UW 0->1/1->3/2->2 "
    "c_tim_proc.cpp:4340-4357 + setxyA/setxyB c_timer.h:82,90 + ticketed "
    "DM2_QUEUE_TIMER c_timer.cpp:235-257 bound; message dispatch "
    "host-owned)";

const char *dm2_v1_invoke_message_source_evidence(void) {
  return dm2_v1_invoke_message_evidence;
}

int dm2_v1_invoke_message(
    DM2_V1_SourceTimerQueue *queue,
    int map_current,
    int xa, int ya,
    int xb, int sel,
    int32_t tick,
    DM2_V1_InvokeMessageReceipt *receipt) {
  DM2_V1_InvokeMessageReceipt local;
  DM2_V1_SourceTimer timer;
  DM2_V1_SourceTimerResult enq = DM2_V1_SOURCE_TIMER_OK;
  uint16_t sel_uw;

  memset(&local, 0, sizeof(local));
  snprintf(local.source_evidence, sizeof(local.source_evidence), "%s",
           dm2_v1_invoke_message_evidence);

  if (receipt == NULL) {
    receipt = &local;
  } else {
    *receipt = local;
  }

  if (queue == NULL || map_current < 0 || map_current > 0xff) {
    return 0;
  }

  memset(&timer, 0, sizeof(timer));

  /* c_timer.h:66 setmticks — (map << 24) | tick.  The source ORs the
   * tick unmasked; the bounded slice masks it exactly like the proven
   * scheduling producer (dm2_v1_creature_schedule_at). */
  timer.ticks_and_map =
      ((uint32_t)map_current << 24) |
      ((uint32_t)tick & DM2_V1_SOURCE_TIMER_TICK_MASK);
  timer.type = 0x4u; /* c_tim_proc.cpp:4339 */

  /* c_tim_proc.cpp:4340-4357 — the RG3UW actor mapping; every other
   * value keeps the c_tim init default 0 (the timer was zeroed above,
   * mirroring c_tim::init). */
  sel_uw = (uint16_t)sel;
  if (sel_uw == 0u) {
    timer.actor = 1u;
  } else if (sel_uw == 1u) {
    timer.actor = 3u;
  } else if (sel_uw == 2u) {
    timer.actor = 2u;
  }

  /* c_timer.h:82 setxyA(eaxl lo, edxl lo); c_timer.h:90 setxyB(ebxl lo,
   * ecxl lo). */
  timer.value_a = (int16_t)((xa & 0xff) | ((ya & 0xff) << 8));
  timer.value_b = (int16_t)((xb & 0xff) | ((sel & 0xff) << 8));

  receipt->type = (int)timer.type;
  receipt->actor = (int)timer.actor;
  receipt->value_a = timer.value_a;
  receipt->value_b = timer.value_b;
  receipt->ticks_and_map = timer.ticks_and_map;

  /* c_tim_proc.cpp:4361 DM2_QUEUE_TIMER — bound ticketed with the
   * producer-module source_index 0u convention. */
  receipt->ticket =
      dm2_v1_source_timer_enqueue_ticketed(queue, &timer, 0u, &enq);
  if (enq != DM2_V1_SOURCE_TIMER_OK || receipt->ticket == 0u) {
    receipt->queue_rejected = 1;
    return 0;
  }

  receipt->valid = 1;
  return 1;
}
