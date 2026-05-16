#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
FIRESTAFF_DATA="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"

cc -std=c99 -Wall -Wextra -pedantic -DCOMPILE_H '-DSTATICFUNCTION=static' '-DSEPARATOR=,' '-DFINAL_SEPARATOR=)' \
  -I$HERE \
  $HERE/firestaff_m9_beta_harness.c \
  $HERE/stateful_boot_plan_reachability_pc34_compat.c \
  $HERE/boot_plan_script_pc34_compat.c \
  $HERE/boot_plan_runtime_pc34_compat.c \
  $HERE/boot_program_runtime_pc34_compat.c \
  $HERE/startup_runtime_driver_pc34_compat.c \
  $HERE/graphics_dat_entry_classify_pc34_compat.c \
  $HERE/late_special_dispatch_pc34_compat.c \
  $HERE/host_video_pgm_backend_pc34_compat.c \
  $HERE/screen_bitmap_present_pc34_compat.c \
  $HERE/screen_bitmap_export_pgm_pc34_compat.c \
  $HERE/dialog_frontend_pc34_compat.c \
  $HERE/memory_graphics_dat_header_pc34_compat.c \
  $HERE/memory_graphics_dat_menu_activate_consequence_pc34_compat.c \
  $HERE/memory_graphics_dat_submenu_consequence_pc34_compat.c \
  $HERE/memory_graphics_dat_menu_activate_pc34_compat.c \
  $HERE/memory_graphics_dat_menu_render_effect_pc34_compat.c \
  $HERE/memory_graphics_dat_menu_state_pc34_compat.c \
  $HERE/memory_graphics_dat_event_dispatch_pc34_compat.c \
  $HERE/memory_graphics_dat_input_command_queue_pc34_compat.c \
  $HERE/memory_graphics_dat_main_loop_typed_command_queue_pc34_compat.c \
  $HERE/memory_graphics_dat_main_loop_command_queue_pc34_compat.c \
  $HERE/memory_graphics_dat_main_loop_command_loop_pc34_compat.c \
  $HERE/memory_graphics_dat_main_loop_command_pc34_compat.c \
  $HERE/memory_graphics_dat_startup_tick_pc34_compat.c \
  $HERE/memory_graphics_dat_main_loop_entry_pc34_compat.c \
  $HERE/memory_graphics_dat_startup_dispatch_pc34_compat.c \
  $HERE/memory_graphics_dat_first_frame_pc34_compat.c \
  $HERE/memory_graphics_dat_startup_wiring_pc34_compat.c \
  $HERE/memory_graphics_dat_boot_pc34_compat.c \
  $HERE/memory_graphics_dat_startup_pc34_compat.c \
  $HERE/memory_graphics_dat_viewport_path_pc34_compat.c \
  $HERE/memory_graphics_dat_bitmap_path_pc34_compat.c \
  $HERE/memory_graphics_dat_special_pc34_compat.c \
  $HERE/memory_graphics_dat_runtime_transaction_pc34_compat.c \
  $HERE/memory_graphics_dat_state_pc34_compat.c \
  $HERE/memory_graphics_dat_composed_transaction_pc34_compat.c \
  $HERE/memory_graphics_dat_select_pc34_compat.c \
  $HERE/memory_graphics_dat_metadata_pc34_compat.c \
  $HERE/memory_graphics_dat_transaction_pc34_compat.c \
  $HERE/memory_graphics_dat_pc34_compat.c \
  $HERE/memory_cache_frontend_pc34_compat.c \
  $HERE/memory_frontend_pc34_compat.c \
  $HERE/memory_load_expand_pc34_compat.c \
  $HERE/expand_frontend_pc34_compat.c \
  $HERE/bitmap_call_pc34_compat.c \
  $HERE/image_expand_pc34_compat.c \
  $HERE/image_backend_pc34_compat.c \
  $HERE/bitmap_copy_pc34_compat.c \
  $HERE/title_frontend_v1.c \
  $HERE/title_dat_loader_v1.c \
  -o $HERE/firestaff_m9_beta_harness
if [ $# -eq 0 ]; then
  echo "built: $HERE/firestaff_m9_beta_harness"
  exit 0
fi
$HERE/firestaff_m9_beta_harness "$@"
