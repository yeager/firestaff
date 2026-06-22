#include "campaign_m12.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

static void check(int ok, const char* name) {
    if (!ok) {
        ++failures;
        printf("FAIL %s\n", name);
    } else {
        printf("PASS %s\n", name);
    }
}

int main(void) {
    M12_CampaignSessionTimer timer;
    M12_CampaignSlot slot;
    char buf[16];
    int flushed;

    memset(&slot, 0, sizeof(slot));
    slot.status = CAMPAIGN_SLOT_ACTIVE;
    slot.playTimeSeconds = 120;

    M12_CampaignSessionTimer_Init(&timer);
    check(timer.running == 0 && timer.paused == 0 && timer.elapsedSeconds == 0,
          "init resets timer");
    M12_CampaignSessionTimer_Tick(&timer, 30);
    check(timer.elapsedSeconds == 0, "stopped timer ignores ticks");

    M12_CampaignSessionTimer_Start(&timer);
    M12_CampaignSessionTimer_Tick(&timer, 30);
    M12_CampaignSessionTimer_Tick(&timer, 15);
    check(timer.elapsedSeconds == 45, "running timer accumulates positive seconds");
    M12_CampaignSessionTimer_Tick(&timer, -10);
    check(timer.elapsedSeconds == 45, "negative ticks ignored");

    M12_CampaignSessionTimer_Pause(&timer);
    M12_CampaignSessionTimer_Tick(&timer, 10);
    check(timer.elapsedSeconds == 45 && timer.paused == 1,
          "paused timer does not accumulate");
    M12_CampaignSessionTimer_Resume(&timer);
    M12_CampaignSessionTimer_Tick(&timer, 15);
    check(timer.elapsedSeconds == 60 && timer.paused == 0,
          "resumed timer continues accumulation");

    M12_Campaign_FormatPlayTime(timer.elapsedSeconds, buf, (int)sizeof(buf));
    check(strcmp(buf, "00:01:00") == 0, "session time formats as HH:MM:SS");

    flushed = M12_CampaignSessionTimer_FlushToSlot(&timer, &slot);
    check(flushed == 60, "flush reports accumulated seconds");
    check(slot.playTimeSeconds == 180, "flush adds elapsed seconds to slot");
    check(slot.modifiedAt != 0, "flush updates slot modification time");
    check(timer.running == 0 && timer.paused == 0 && timer.elapsedSeconds == 0,
          "flush resets timer");
    check(M12_CampaignSessionTimer_FlushToSlot(&timer, &slot) == 0,
          "empty flush is a no-op");

    M12_CampaignSessionTimer_Start(&timer);
    M12_CampaignSessionTimer_Tick(&timer, 5);
    M12_CampaignSessionTimer_Start(&timer);
    check(timer.running == 1 && timer.elapsedSeconds == 0,
          "start resets unflushed stale session time");

    if (failures) {
        printf("test_campaign_m12_session_timer: FAIL %d\n", failures);
        return 1;
    }
    puts("test_campaign_m12_session_timer: PASS");
    return 0;
}
