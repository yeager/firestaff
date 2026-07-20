/*
 * src/dm2/dm2_v1_update_weather_pc34_compat.c
 *
 * Bounded slice of the 0x54 timer dispatch into DM2_UPDATE_WEATHER(1).
 * See include/dm2_v1_update_weather_pc34_compat.h for the full source
 * mapping (c_tim_proc.cpp:4179-4183, c_weather.cpp:33-65 arg==1 branch).
 */

#include "dm2_v1_update_weather_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/* Source-faithful DM2_RAND16 (c_random.cpp:24-28):
 * CUTX16(DM2_RAND()) % n — the low 16 bits of the 24-bit draw, then the
 * modulo.  dm2_v1_drops_rand16 applies the modulo to the full 24-bit
 * draw; identical for moduli dividing 2^16 (256, 4) but not for the
 * transition's 8000/500/3. */
static uint16_t weather_rand16(DM2_V1_DropRng *rng, uint16_t n)
{
  if (n == 0)
    return 0;
  return (uint16_t)((dm2_v1_drops_rand24(rng) & 0xffffu) % (uint32_t)n);
}

/* table1d6b76[132] — src/dm2/dm2data.cpp:889-896 verbatim. The weather
 * flags read by DM2_UPDATE_WEATHER live at 4*zone + 0x70 (0x70..0x7f):
 * zone0=0x00, zone1=0x01, zone2=0x00, zone3=0x00. */
const uint8_t
    dm2_v1_update_weather_table1d6b76[DM2_V1_UPDATE_WEATHER_TABLE1D6B76_LEN] =
{
  0x60, 0x57, 0x4e, 0x47, 0x40, 0x3a, 0x34, 0x2f,
  0x2b, 0x27, 0x23, 0x1f, 0x1c, 0x1a, 0x17, 0x15,
  0x13, 0x11, 0x0f, 0x02, 0x01, 0x00, 0x01, 0x02,
  0x02, 0x00, 0x00, 0x40, 0x34, 0x2b, 0x23, 0x1c,
  0x17, 0x13, 0x02, 0x05, 0x00, 0x06, 0x05, 0x07,
  0x03, 0x00, 0x07, 0x01, 0x01, 0x02, 0x06, 0x03,
  0x03, 0x03, 0x05, 0x05, 0x02, 0x06, 0x07, 0x07,
  0x01, 0x00, 0x03, 0x01, 0x06, 0x02, 0x01, 0x03,
  0x05, 0x03, 0x02, 0x0e, 0x16, 0x16, 0x16, 0x0a,
  0x0c, 0x34, 0x40, 0x4e, 0x4e, 0x4e, 0x40, 0x40,
  0x00, 0x01, 0x02, 0x03, 0x00, 0xfd, 0xfe, 0xff,
  0x18, 0x18, 0x18, 0x20, 0x20, 0x20, 0x30, 0x1e,
  0x1e, 0x1e, 0x28, 0x28, 0x28, 0x40, 0x00, 0x00,
  0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
  0x01, 0x01, 0x01, 0x00
};

/* v1d7108[128] — binary pattern data the source loads at runtime via
 * DM2_READ_BINARY("v1d7108.dat") (c_weather.cpp:14-19). Bound verbatim
 * from the extracted v1d7108.dat; indexed as signed i8 at
 * (pattern_row << 5) + retry. */
const int8_t
    dm2_v1_update_weather_pattern[DM2_V1_UPDATE_WEATHER_PATTERN_LEN] =
{
  (int8_t)0x01, (int8_t)0x05, (int8_t)0x05, (int8_t)0x05,
  (int8_t)0x05, (int8_t)0x05, (int8_t)0x05, (int8_t)0x05,
  (int8_t)0x05, (int8_t)0x05, (int8_t)0x05, (int8_t)0x05,
  (int8_t)0x05, (int8_t)0x05, (int8_t)0x05, (int8_t)0x05,
  (int8_t)0xfb, (int8_t)0xfb, (int8_t)0xfb, (int8_t)0xfb,
  (int8_t)0xfb, (int8_t)0xfb, (int8_t)0xfb, (int8_t)0xfb,
  (int8_t)0xfb, (int8_t)0xfb, (int8_t)0xfb, (int8_t)0xfb,
  (int8_t)0xfb, (int8_t)0xfb, (int8_t)0xfb, (int8_t)0xff,
  (int8_t)0x01, (int8_t)0x02, (int8_t)0x02, (int8_t)0x02,
  (int8_t)0x04, (int8_t)0x04, (int8_t)0x06, (int8_t)0x06,
  (int8_t)0x06, (int8_t)0x07, (int8_t)0x08, (int8_t)0x09,
  (int8_t)0x07, (int8_t)0x05, (int8_t)0x04, (int8_t)0x03,
  (int8_t)0xfd, (int8_t)0xfc, (int8_t)0xfb, (int8_t)0xf9,
  (int8_t)0xf7, (int8_t)0xf8, (int8_t)0xf9, (int8_t)0xfa,
  (int8_t)0xfa, (int8_t)0xfa, (int8_t)0xfc, (int8_t)0xfc,
  (int8_t)0xfe, (int8_t)0xfe, (int8_t)0xfe, (int8_t)0xff,
  (int8_t)0x0a, (int8_t)0x12, (int8_t)0x16, (int8_t)0x1a,
  (int8_t)0xfc, (int8_t)0xfc, (int8_t)0xfc, (int8_t)0xfc,
  (int8_t)0xfc, (int8_t)0xfc, (int8_t)0xfc, (int8_t)0xfc,
  (int8_t)0xfd, (int8_t)0xfd, (int8_t)0xfd, (int8_t)0xfd,
  (int8_t)0xfe, (int8_t)0xfe, (int8_t)0xfe, (int8_t)0xfe,
  (int8_t)0xfe, (int8_t)0xfe, (int8_t)0xfe, (int8_t)0xfe,
  (int8_t)0xfe, (int8_t)0xfe, (int8_t)0xfe, (int8_t)0xfe,
  (int8_t)0xfe, (int8_t)0xfe, (int8_t)0xfe, (int8_t)0xfe,
  (int8_t)0x02, (int8_t)0x02, (int8_t)0x02, (int8_t)0x02,
  (int8_t)0x02, (int8_t)0x02, (int8_t)0x02, (int8_t)0x02,
  (int8_t)0x02, (int8_t)0x02, (int8_t)0x02, (int8_t)0x02,
  (int8_t)0x02, (int8_t)0x02, (int8_t)0x02, (int8_t)0x02,
  (int8_t)0x03, (int8_t)0x03, (int8_t)0x03, (int8_t)0x03,
  (int8_t)0x04, (int8_t)0x04, (int8_t)0x04, (int8_t)0x04,
  (int8_t)0x04, (int8_t)0x04, (int8_t)0x04, (int8_t)0x04,
  (int8_t)0xe6, (int8_t)0xea, (int8_t)0xee, (int8_t)0xf6
};

/* table1d70f0[24] — src/dm2/dm2data.cpp:182-191 verbatim. Time-of-day
 * words indexed by (gametick + v1e1438) / 0x555 % 0x18
 * (c_weather.cpp:99-103, 563-565). */
const int8_t dm2_v1_weather_table1d70f0[DM2_V1_WEATHER_TABLE1D70F0_LEN] =
{
  0x05, 0x05, 0x04, 0x03,
  0x02, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x02,
  0x03, 0x04, 0x05, 0x05
};

int dm2_v1_update_weather_1(DM2_V1_UpdateWeatherState *state,
                            DM2_V1_DropRng *rng,
                            DM2_V1_UpdateWeatherReceipt *out_receipt)
{
  DM2_V1_UpdateWeatherReceipt rc;
  uint8_t retry;
  int32_t product;

  memset(&rc, 0, sizeof(rc));
  rc.reschedule_delay = -1;
  rc.rand_draw = -1;

  if (out_receipt != 0) {
    out_receipt->valid = 0;
    out_receipt->weather_allowed = 0;
    out_receipt->retry = 0;
    out_receipt->pattern_delta = 0;
    out_receipt->intensity_before = 0;
    out_receipt->intensity_after = 0;
    out_receipt->transition_forced = 0;
    out_receipt->reschedule_delay = -1;
    out_receipt->rand_draw = -1;
  }

  if (state == 0)
    return 0;

  /* Bounds: 4*zone + 0x70 must stay inside table1d6b76[132]
   * (zone 0..31), and (pattern_row << 5) + retry must stay inside
   * v1d7108[128] (pattern_row 0..3). Anything else: no mutation. */
  if (state->zone_index < 0 ||
      state->zone_index > DM2_V1_UPDATE_WEATHER_MAX_ZONE)
    return 0;
  if (state->pattern_row < 0 ||
      state->pattern_row >= DM2_V1_UPDATE_WEATHER_PATTERN_ROWS)
    return 0;

  rc.valid = 1;

  /* c_weather.cpp:63-64 — v1e147f = table1d6b76[4*v1e1472 + 0x70]. */
  rc.weather_allowed =
      (int)dm2_v1_update_weather_table1d6b76[
          4 * (int)state->zone_index + DM2_V1_UPDATE_WEATHER_FLAG_BASE];
  state->weather_allowed = (int8_t)rc.weather_allowed;

  /* c_weather.cpp:68-75 — ++v1e147b (byte arithmetic, compared as
   * unsigned); retry > 0x1f forces a transition with no requeue. */
  retry = (uint8_t)((uint8_t)state->retry + 1u);
  state->retry = (int8_t)retry;
  rc.retry = (int)retry;
  rc.intensity_before = state->intensity;
  rc.intensity_after = state->intensity;

  if (retry > DM2_V1_UPDATE_WEATHER_MAX_RETRY) {
    /* DM2_weather_3df7_0037(0) — the weather transition itself is not
     * part of this bounded slice; the host owns it. No requeue. */
    rc.transition_forced = 1;
    rc.reschedule_delay = -1;
    if (out_receipt != 0)
      *out_receipt = rc;
    return 1;
  }

  /* c_weather.cpp:78-86 — snapshot, intensity step, clamp. The source
   * zero-extends v1e1484 (step) and sign-extends the pattern byte, then
   * keeps the low 16 bits of the product. */
  state->previous_intensity = state->intensity;
  rc.pattern_delta =
      (int)dm2_v1_update_weather_pattern[
          ((int)(uint8_t)state->pattern_row << 5) + (int)retry];
  product = (int32_t)(uint8_t)state->step * (int32_t)rc.pattern_delta;
  state->intensity = (int16_t)(state->intensity + (int16_t)product);
  if (state->intensity < 0)
    state->intensity = 0;
  else if (state->intensity > 0xff)
    state->intensity = 0xff;
  rc.intensity_after = state->intensity;

  /* c_weather.cpp:87-89 — DM2_SET_TIMER_WEATHER(RAND16(256) + 50). */
  if (rng != 0) {
    rc.rand_draw = (int)dm2_v1_drops_rand16(rng, 256);
    rc.reschedule_delay = rc.rand_draw + DM2_V1_UPDATE_WEATHER_REQUEUE_MIN;
  }

  if (out_receipt != 0)
    *out_receipt = rc;
  return 1;
}

int32_t dm2_v1_weather_transition(DM2_V1_UpdateWeatherState *state,
                                  int32_t gametick, int arg,
                                  DM2_V1_DropRng *rng,
                                  DM2_V1_WeatherTransitionReceipt *out_receipt)
{
  DM2_V1_WeatherTransitionReceipt rc;
  int32_t t;

  memset(&rc, 0, sizeof(rc));
  rc.queue_delay = -1;

  if (out_receipt != 0)
    *out_receipt = rc;

  if (state == 0 || rng == 0)
    return 0;

  rc.valid = 1;
  rc.arg = arg;

  if (arg == 0) {
    /* c_weather.cpp:518-555 — full transition. */
    rc.light_update_requested = 1; /* DM2_UPDATE_GLOB_VAR(0x40,0,6): host */
    state->day_tick = gametick + DM2_V1_WEATHER_DAY_TICKS;
    state->storm_active = 0;
    state->weather_allowed = 0;

    if (state->storm_request == 0) {
      /* c_weather.cpp:529-535 — normal reseed. */
      rc.queue_delay =
          (int)weather_rand16(rng, 8000) + 500;
      ++rc.draws;
      state->pattern_row = (int8_t)dm2_v1_drops_randdir(rng);
      ++rc.draws;
      state->step = (int8_t)(weather_rand16(rng, 3) + 1);
      ++rc.draws;
    } else {
      /* c_weather.cpp:537-543 — storm-forced branch. */
      rc.storm_path = 1;
      state->rain_counter = 0;
      rc.queue_delay = (int)weather_rand16(rng, 500);
      ++rc.draws;
      state->pattern_row = 3;
      state->step = 1;
    }

    /* c_weather.cpp:545-554 — common reset + wind + requeue. */
    state->cloud_state = 1;
    state->lightning_flag = 0;
    state->intensity = 0;
    state->previous_intensity = 0;
    state->retry = 0;
    state->wind_dir = (int8_t)dm2_v1_drops_randdir(rng);
    ++rc.draws;
    rc.reseeded = 1;
  } else {
    /* c_weather.cpp:557-560 — keep-current branch, no requeue. */
    state->previous_intensity = 0;
    if (state->step == 0)
      state->step = 1;
  }

  /* c_weather.cpp:562-567 — common tail. */
  state->cloud_timer = (int16_t)(weather_rand16(rng, 4) + 4);
  ++rc.draws;
  t = (gametick + state->day_offset) / DM2_V1_WEATHER_DAY_TICKS;
  rc.hour = (int)(t % DM2_V1_WEATHER_HOURS_PER_DAY);
  rc.days = t / DM2_V1_WEATHER_HOURS_PER_DAY;
  state->day_word = (int16_t)dm2_v1_weather_table1d70f0[rc.hour];
  state->storm_request = 0;

  rc.pattern_row = state->pattern_row;
  rc.step = state->step;
  rc.cloud_timer = state->cloud_timer;
  rc.day_word = state->day_word;

  if (out_receipt != 0)
    *out_receipt = rc;
  return rc.days;
}

/* Source-faithful DM2_RANDBIT (c_random.cpp:30-37): advance, (>>8) & 1. */
static int weather_randbit(DM2_V1_DropRng *rng)
{
  return (int)(dm2_v1_drops_rand24(rng) & 1u);
}

int dm2_v1_update_weather_0(DM2_V1_UpdateWeatherState *state,
                            int32_t gametick,
                            unsigned int retrieve_mask,
                            uint16_t gdat_entry_6c,
                            DM2_V1_DropRng *rng,
                            DM2_V1_UpdateWeatherFrameReceipt *out_receipt)
{
  DM2_V1_UpdateWeatherFrameReceipt rc;
  int flash;
  int vql_2c;
  uint8_t slot_cmd[3];

  memset(&rc, 0, sizeof(rc));
  rc.hour = -1;
  rc.day_word = -1;
  rc.thunder_volume = -1;
  rc.bolt_rect_rand = -1;
  rc.bolt_dir = -1;
  rc.weather_gate = 1;

  if (out_receipt != 0)
    *out_receipt = rc;

  if (state == 0 || rng == 0)
    return 0;

  /* Bounds: the table read needs zone 0..31; the thunder-volume path
   * divides by v1e1484, so intensity != 0 with step == 0 is rejected up
   * front rather than crashing like the source would. */
  if (state->zone_index < 0 ||
      state->zone_index > DM2_V1_UPDATE_WEATHER_MAX_ZONE)
    return 0;
  if (state->intensity != 0 && state->step == 0)
    return 0;

  rc.valid = 1;
  memset(slot_cmd, 0, sizeof(slot_cmd));

  /* c_weather.cpp:60-64 — v1e147f = table1d6b76[4*v1e1472 + 0x70]. */
  rc.weather_allowed =
      (int)dm2_v1_update_weather_table1d6b76[
          4 * (int)state->zone_index + DM2_V1_UPDATE_WEATHER_FLAG_BASE];
  state->weather_allowed = (int8_t)rc.weather_allowed;

  /* c_weather.cpp:92-105 — day rollover. */
  if ((uint32_t)gametick >= (uint32_t)state->day_tick) {
    int32_t t = (gametick + state->day_offset) / DM2_V1_WEATHER_DAY_TICKS;
    rc.day_rolled = 1;
    rc.hour = (int)(t % DM2_V1_WEATHER_HOURS_PER_DAY);
    rc.day_word = (int16_t)dm2_v1_weather_table1d70f0[rc.hour];
    state->day_word = rc.day_word;
    state->day_tick = gametick + DM2_V1_WEATHER_DAY_TICKS;
    if (state->weather_allowed != 0)
      ++rc.light_recalc_requests; /* DM2_RECALC_LIGHT_LEVEL: host */
  }

  /* c_weather.cpp:106-199 — lightning evaluation. */
  flash = 0;
  if (state->intensity == 0) {
    /* 109-126: rain decay every 3rd tick, flash = RAND16(64) == 0,
     * lightning_flag = 0 (RG1Bhi), cloud_state = 1. */
    if ((uint8_t)state->rain_counter > 0 && gametick % 3 == 0)
      state->rain_counter = (int8_t)((uint8_t)state->rain_counter - 1u);
    flash = (weather_rand16(rng, 64) == 0) ? 1 : 0;
    ++rc.draws;
    state->lightning_flag = 0;
    state->cloud_state = 1;
  } else {
    /* 127-199: threshold as the low word of 0x100 - intensity +
     * (RAND & 0xf); RG51w; cloud_state = CUTX8(intensity); flag latch;
     * rain increment gating; gated flash evaluation. */
    uint32_t r0 = dm2_v1_drops_rand24(rng);
    uint16_t threshold;
    uint16_t rg51w;
    ++rc.draws;
    threshold = (uint16_t)(0x100u - (uint16_t)state->intensity +
                           (uint16_t)(r0 & 0xfu));
    rg51w = (state->intensity < 0xcd) ? 7u : 0x28u;
    state->cloud_state = (int8_t)((uint16_t)state->intensity & 0xffu);
    if (state->lightning_flag == 0) {
      state->lightning_flag =
          (int8_t)((weather_rand16(rng, threshold) <= 7) ? 1 : 0);
      ++rc.draws;
    }
    if (state->lightning_flag != 0 &&
        (uint8_t)state->rain_counter < 0xffu) {
      uint8_t fl = (uint8_t)state->lightning_flag;
      if (fl < 0x80u) {
        if (fl >= 0x40u && (gametick & 1) == 0) {
          /* skip01038: rain_counter = flag + 1 */
          state->rain_counter = (int8_t)(fl + 1u);
        } else {
          int inc;
          if (fl < 0x10u || gametick % 3 != 0)
            inc = ((gametick & 3) == 0);
          else
            inc = 1;
          if (inc)
            state->rain_counter =
                (int8_t)((uint8_t)state->rain_counter + 1u);
        }
      } else {
        state->rain_counter = (int8_t)(fl + 1u);
      }
    }
    if (state->lightning_enabled != 0) {
      flash = (weather_rand16(rng, threshold) <= rg51w) ? 1 : 0;
      ++rc.draws;
    }
  }
  rc.flash_eval = flash;

  /* c_weather.cpp:201-203 — weather gate. */
  if (state->weather_allowed == 0) {
    rc.weather_gate = 0;
    if (out_receipt != 0)
      *out_receipt = rc;
    return 1;
  }

  /* c_weather.cpp:205-209 — consume the pending light change. */
  if (state->light_pending != 0) {
    state->light_pending = 0;
    ++rc.light_recalc_requests; /* DM2_RECALC_LIGHT_LEVEL: host */
  }

  /* c_weather.cpp:213-237 — cloud command. Slot advances only when the
   * host RETRIEVE_ENVIRONMENT_CMD_CD_FW accepts the slot. */
  rc.slots = 0;
  if (state->clouds_enabled != 0 &&
      (uint8_t)state->cloud_state >= 0x10u) {
    uint8_t cs = (uint8_t)state->cloud_state;
    if (cs < 0x40u)
      rc.cloud_cmd = 0x67;
    else if (cs < 0x80u)
      rc.cloud_cmd = 0x68;
    else {
      rc.cloud_cmd = 0x69;
      state->storm_active = 1;
      rc.storm_set = 1;
    }
    slot_cmd[rc.slots] = (uint8_t)rc.cloud_cmd;
    if (retrieve_mask & DM2_V1_UPDATE_WEATHER_RETRIEVE_CLOUD)
      ++rc.slots;
  }

  /* c_weather.cpp:239-256 — rain command. */
  if (state->rain_enabled != 0 &&
      (uint8_t)state->rain_counter >= 0x40u) {
    uint8_t rain = (uint8_t)state->rain_counter;
    if (rain < 0x80u)
      rc.rain_cmd = 0x6a;
    else if (rain < 0xc0u)
      rc.rain_cmd = 0x6b;
    else
      rc.rain_cmd = 0x6c;
    slot_cmd[rc.slots] = (uint8_t)rc.rain_cmd;
    if (retrieve_mask & DM2_V1_UPDATE_WEATHER_RETRIEVE_RAIN)
      ++rc.slots;
  }

  /* c_weather.cpp:258-440 — lightning execution. */
  vql_2c = 0;
  if (flash != 0) {
    uint16_t draw;
    flash = 0;
    if (state->intensity < 0xb6)
      rc.light_flash_request = 1; /* UPDATE_GLOB_VAR(0x40,0,6): host */
    draw = weather_rand16(rng, (uint16_t)(state->intensity + 1));
    ++rc.draws;
    if (draw >= 60) {
      vql_2c = (int)(dm2_v1_drops_rand24(rng) & 7u) + 1;
      ++rc.draws;
      rc.thunder_count = vql_2c;
      if (gdat_entry_6c == 0) {
        /* c_weather.cpp:286-421: CREATE_CLOUD placement loop — host. */
        rc.cloud_placement_request = 1;
      } else {
        /* c_weather.cpp:423-440: INVOKE_MESSAGE + NOISE_GEN2 — host. */
        rc.invoke_message_request = 1;
      }
      rc.rng_diverges = 1;
    }
  }

  /* c_weather.cpp:441-474 — lightning bolt command (only when no
   * thunder clouds are attempted). */
  if (vql_2c == 0) {
    if (weather_randbit(rng) != 0) {
      ++rc.draws;
      rc.bolt_cmd = 100 + (int)weather_rand16(rng, 3);
      ++rc.draws;
      slot_cmd[rc.slots] = (uint8_t)rc.bolt_cmd;
      if (retrieve_mask & DM2_V1_UPDATE_WEATHER_RETRIEVE_BOLT) {
        ++rc.slots;
        flash = 1;
        rc.bolt_rect_rand = (int)weather_rand16(rng, 100);
        ++rc.draws;
        rc.bolt_dir = (int)dm2_v1_drops_randdir(rng);
        ++rc.draws;
      }
    } else {
      ++rc.draws;
    }
  }

  /* c_weather.cpp:476-504 — thunder sound latch + final light change. */
  if (flash != 0) {
    if (state->thunder_latch != 0) {
      state->thunder_latch = 0;
    } else {
      int volume;
      if (state->intensity != 0)
        volume = 0x4c - (int)state->intensity / (int)(uint8_t)state->step;
      else {
        volume = (int)weather_rand16(rng, 10) + 5;
        ++rc.draws;
      }
      if (volume < 1)
        volume = 1;
      else if (volume > 15)
        volume = 15;
      rc.thunder_volume = volume;
      rc.thunder_sound = 1; /* QUEUE_NOISE_GEN1: host */
      state->thunder_latch = 1;
    }
    /* m_4A899: v1e024c = 1; RECALC_LIGHT_LEVEL (host). */
    state->light_pending = 1;
    ++rc.light_recalc_requests;
  }

  /* m_4A8A8 — 0xff terminator after the compacted live chain. */
  memcpy(rc.live_cmds, slot_cmd, sizeof(slot_cmd));

  if (out_receipt != 0)
    *out_receipt = rc;
  return 1;
}
