#include "redmcsb_f0693_wait_vertical_blank_pc34_compat.h"

#include <stdio.h>

typedef struct {
    ReDMCSBF0693WaitVerticalBlankPc34Compat *gate;
    int deliveries;
    int saw_pending_gate;
} VBlankDelivery;

static void deliver_one_vertical_blank(void *context)
{
    VBlankDelivery *delivery = context;

    ++delivery->deliveries;
    delivery->saw_pending_gate = delivery->gate->waiting_for_vertical_blank;
    F0693_VerticalBlankCallback_PC34(delivery->gate);
}

int main(void)
{
    ReDMCSBF0693WaitVerticalBlankPc34Compat gate = { false, NULL, NULL };
    VBlankDelivery delivery = { &gate, 0, 0 };

    gate.deliver_vertical_blank = deliver_one_vertical_blank;
    gate.context = &delivery;
    if (!F0693_WaitVerticalBlank_PC34(&gate) || delivery.deliveries != 1 ||
        !delivery.saw_pending_gate || gate.waiting_for_vertical_blank) {
        fprintf(stderr, "F0693 VBlank gate did not wait for its callback\n");
        return 1;
    }

    gate.deliver_vertical_blank = NULL;
    if (F0693_WaitVerticalBlank_PC34(&gate) || gate.waiting_for_vertical_blank) {
        fprintf(stderr, "F0693 VBlank gate accepted a missing callback\n");
        return 1;
    }

    puts("PASS redmcsb_f0693_wait_vertical_blank_pc34_compat");
    return 0;
}
