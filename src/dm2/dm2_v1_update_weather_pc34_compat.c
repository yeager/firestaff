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
