# N2 consolidated viewport/world visuals pass

- host: firestaff-worker
- repo: <repo>
- data: $FIRESTAFF_DATA
- original: <N2_ORIGINAL_GAMES>/DM
- greatstone: <N2_GREATSTONE_ATLAS>
- redmcsb: <N2_REDMCSB_SOURCE>
- started: 2026-04-28T12:32:37+00:00

### git status before
```sh
git status --short --branch
```
```text
## sync/n2-dm1-v1-20260428...origin/main [ahead 46]
?? ..-firestaff-worker-viewport-parity-20260427-040243/
?? verification-m11/lane1-original-faithful-parity-20260428-072831/
?? verification-m11/lane2-hud-20260428-070228/
?? verification-m11/n2_viewport_world_visuals_20260428_123237/
?? verification-m11/verification_summary.md
```
exit=0

### cmake build
```sh
cmake --build build -j8
```
```text
[ 73%] Built target firestaff_m10
[ 80%] Built target firestaff_m11
[ 81%] Built target firestaff_m11_phase_a_probe
[ 83%] Built target firestaff_m11_fs_portable_probe
[ 84%] Built target firestaff_m11_audio_probe
[ 91%] Built target firestaff_m12
[ 92%] Built target capture_ingame_series
[ 93%] Built target firestaff_m11_viewport_state_probe
[ 94%] Built target firestaff
[ 95%] Built target firestaff_m11_game_view_probe
[ 97%] Built target firestaff_m11_v2_initial_4k_capture
[ 98%] Built target firestaff_m11_capture_route_state_probe
[100%] Built target firestaff_m12_startup_menu_probe
```
exit=0

### m11 phase a probe
```sh
./run_firestaff_m11_phase_a_probe.sh
```
```text
# M11 Phase A probe: SDL=sdl3
<repo>/firestaff_m11_phase_a_probe.c: In function ‘main’:
<repo>/firestaff_m11_phase_a_probe.c:49:27: warning: implicit declaration of function ‘setenv’; did you mean ‘getenv’? [-Wimplicit-function-declaration]
   49 | # define m11_setenv(k, v) setenv((k), (v), 0)
      |                           ^~~~~~
<repo>/firestaff_m11_phase_a_probe.c:72:5: note: in expansion of macro ‘m11_setenv’
   72 |     m11_setenv("SDL_VIDEODRIVER", "dummy");
      |     ^~~~~~~~~~
In file included from <repo>/main_loop_m11.c:14:
<repo>/m11_game_view.h:135:43: warning: declaration does not declare anything
  135 |     enum { M11_TORCH_FUEL_CAPACITY = 256 };
      |                                           ^
In file included from <repo>/m11_game_view.c:1:
<repo>/m11_game_view.h:135:43: warning: declaration does not declare anything
  135 |     enum { M11_TORCH_FUEL_CAPACITY = 256 };
      |                                           ^
<repo>/m11_game_view.c: In function ‘m11_draw_v1_message_area’:
<repo>/m11_game_view.c:18559:43: warning: ‘%s’ directive output may be truncated writing up to 79 bytes into a region of size 54 [-Wformat-truncation=]
18559 |         snprintf(clipped, maxChars + 1U, "%s", text);
      |                                           ^~
In file included from /usr/include/stdio.h:980,
                 from <repo>/memory_tick_orchestrator_pc34_compat.h:65,
                 from <repo>/m11_game_view.h:5:
In function ‘snprintf’,
    inlined from ‘m11_draw_v1_message_area’ at <repo>/m11_game_view.c:18559:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 80 bytes into a destination of size 54
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘m11_draw_wall_contents.isra’:
<repo>/m11_game_view.c:7840:55: warning: ‘%d’ directive output may be truncated writing between 1 and 10 bytes into a region of size 4 [-Wformat-truncation=]
 7840 |                 snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                       ^~
<repo>/m11_game_view.c:7840:54: note: directive argument in the range [2, 2147483647]
 7840 |                 snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                      ^~~~
In function ‘snprintf’,
    inlined from ‘m11_draw_wall_contents.isra’ at <repo>/m11_game_view.c:7840:17:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 2 and 11 bytes into a destination of size 4
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘m11_draw_side_feature.isra’:
<repo>/m11_game_view.c:11139:59: warning: ‘%d’ directive output may be truncated writing between 1 and 10 bytes into a region of size 4 [-Wformat-truncation=]
11139 |                     snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                           ^~
<repo>/m11_game_view.c:11139:58: note: directive argument in the range [2, 2147483647]
11139 |                     snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                          ^~~~
In function ‘snprintf’,
    inlined from ‘m11_draw_side_feature.isra’ at <repo>/m11_game_view.c:11139:21:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 2 and 11 bytes into a destination of size 4
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_QuickSave’:
<repo>/m11_game_view.c:4735:40: warning: ‘%s’ directive output may be truncated writing up to 511 bytes into a region of size between 95 and 104 [-Wformat-truncation=]
 4735 |              "F9 RESTORES TICK %u FROM %s",
      |                                        ^~
 4736 |              (unsigned int)state->world.gameTick,
 4737 |              path);
      |              ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_QuickSave’ at <repo>/m11_game_view.c:4734:5:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 25 and 545 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_QuickLoad’:
<repo>/m11_game_view.c:4824:47: warning: ‘%s’ directive output may be truncated writing up to 511 bytes into a region of size between 84 and 93 [-Wformat-truncation=]
 4824 |              "TICK %u HASH %08X RELOADED FROM %s",
      |                                               ^~
......
 4827 |              path);
      |              ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_QuickLoad’ at <repo>/m11_game_view.c:4823:5:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 36 and 556 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘m11_draw_utility_panel’:
<repo>/m11_game_view.c:16594:39: warning: ‘%s’ directive output may be truncated writing up to 63 bytes into a region of size 32 [-Wformat-truncation=]
16594 |         snprintf(line, sizeof(line), "%s",
      |                                       ^~
In function ‘snprintf’,
    inlined from ‘m11_draw_utility_panel’ at <repo>/m11_game_view.c:16594:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 64 bytes into a destination of size 32
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:18804:47: warning: ‘%s’ directive output may be truncated writing up to 95 bytes into a region of size 90 [-Wformat-truncation=]
18804 |         snprintf(line2, sizeof(line2), "HERE  %s", line);
      |                                               ^~   ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_Draw’ at <repo>/m11_game_view.c:18804:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 7 and 102 bytes into a destination of size 96
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:18811:47: warning: ‘%s’ directive output may be truncated writing up to 95 bytes into a region of size 90 [-Wformat-truncation=]
18811 |         snprintf(line2, sizeof(line2), "AHEAD %s", line);
      |                                               ^~   ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_Draw’ at <repo>/m11_game_view.c:18811:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 7 and 102 bytes into a destination of size 96
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:744:37: warning: ‘%s’ directive output may be truncated writing up to 127 bytes into a region of size 80 [-Wformat-truncation=]
  744 |         snprintf(line1, line1Size, "%s", text);
      |                                     ^~
In function ‘snprintf’,
    inlined from ‘m11_dialog_source_split_two_lines’ at <repo>/m11_game_view.c:744:9,
    inlined from ‘M11_GameView_Draw’ at <repo>/m11_game_view.c:19239:29:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 128 bytes into a destination of size 80
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:19124:33: warning: ‘skillY’ may be used uninitialized [-Wmaybe-uninitialized]
19124 |                                 m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
      |                                 ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
19125 |                                               skillX, skillY, skillLine, &skillStyle);
      |                                               ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c:19116:45: note: ‘skillY’ was declared here
19116 |                                 int skillX, skillY;
      |                                             ^~~~~~
<repo>/asset_status_m12.c: In function ‘M12_AssetStatus_Scan’:
<repo>/asset_status_m12.c:256:29: warning: ‘%s’ directive output may be truncated writing up to 575 bytes into a region of size 512 [-Wformat-truncation=]
  256 |     snprintf(out, outSize, "%s", value);
      |                             ^~
......
  312 |             m12_copy_string(matchedPath, M12_ASSET_DATA_DIR_CAPACITY, path);
      |                                                                       ~~~~
In file included from /usr/include/stdio.h:980,
                 from <repo>/asset_status_m12.c:5:
In function ‘snprintf’,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:256:5,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:249:13,
    inlined from ‘m12_try_match_version’ at <repo>/asset_status_m12.c:312:13,
    inlined from ‘m12_fill_game_versions’ at <repo>/asset_status_m12.c:341:17,
    inlined from ‘M12_AssetStatus_Scan’ at <repo>/asset_status_m12.c:379:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 576 bytes into a destination of size 512
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/asset_status_m12.c: In function ‘M12_AssetStatus_Scan’:
<repo>/asset_status_m12.c:256:29: warning: ‘%s’ directive output may be truncated writing up to 1535 bytes into a region of size 512 [-Wformat-truncation=]
  256 |     snprintf(out, outSize, "%s", value);
      |                             ^~
In function ‘snprintf’,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:256:5,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:249:13,
    inlined from ‘m12_fill_game_versions’ at <repo>/asset_status_m12.c:348:21,
    inlined from ‘M12_AssetStatus_Scan’ at <repo>/asset_status_m12.c:379:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 1536 bytes into a destination of size 512
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/menu_startup_m12.c: In function ‘m12_load_runtime_catalog’:
<repo>/menu_startup_m12.c:535:41: warning: ‘%s’ directive output may be truncated writing up to 255 bytes into a region of size 128 [-Wformat-truncation=]
  535 |     snprintf(out + len, outSize - len, "%s", value);
      |                                         ^~
......
  683 |                 m12_append_string(msgid, sizeof(msgid), parsed);
      |                                                         ~~~~~~
In file included from /usr/include/stdio.h:980,
                 from <repo>/menu_startup_m12.c:9:
In function ‘snprintf’,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:535:5,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:526:13,
    inlined from ‘m12_load_runtime_catalog’ at <repo>/menu_startup_m12.c:683:17:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 1 and 256 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/menu_startup_m12.c: In function ‘m12_load_runtime_catalog’:
<repo>/menu_startup_m12.c:535:41: warning: ‘%s’ directive output may be truncated writing up to 255 bytes into a region of size 128 [-Wformat-truncation=]
  535 |     snprintf(out + len, outSize - len, "%s", value);
      |                                         ^~
......
  695 |                     m12_append_string(msgid, sizeof(msgid), parsed);
      |                                                             ~~~~~~
In function ‘snprintf’,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:535:5,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:526:13,
    inlined from ‘m12_load_runtime_catalog’ at <repo>/menu_startup_m12.c:695:21:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 1 and 256 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/menu_startup_render_modern_m12.c: In function ‘draw_card’:
<repo>/menu_startup_render_modern_m12.c:879:9: warning: this ‘if’ clause does not guard... [-Wmisleading-indentation]
  879 |         if (li < 0) li = 0; if (li > 3) li = 3;
      |         ^~
<repo>/menu_startup_render_modern_m12.c:879:29: note: ...this statement, but the latter is misleadingly indented as if it were guarded by the ‘if’
  879 |         if (li < 0) li = 0; if (li > 3) li = 3;
      |                             ^~
<repo>/menu_startup_render_modern_m12.c:880:9: warning: this ‘if’ clause does not guard... [-Wmisleading-indentation]
  880 |         if (gi < 0) gi = 0; if (gi > 2) gi = 2;
      |         ^~
<repo>/menu_startup_render_modern_m12.c:880:29: note: ...this statement, but the latter is misleadingly indented as if it were guarded by the ‘if’
  880 |         if (gi < 0) gi = 0; if (gi > 2) gi = 2;
      |                             ^~
<repo>/menu_startup_render_modern_m12.c:881:9: warning: this ‘if’ clause does not guard... [-Wmisleading-indentation]
  881 |         if (wi < 0) wi = 0; if (wi > 1) wi = 1;
      |         ^~
<repo>/menu_startup_render_modern_m12.c:881:29: note: ...this statement, but the latter is misleadingly indented as if it were guarded by the ‘if’
  881 |         if (wi < 0) wi = 0; if (wi > 1) wi = 1;
      |                             ^~
<repo>/menu_startup_render_modern_m12.c: In function ‘draw_settings_view’:
<repo>/menu_startup_render_modern_m12.c:1134:5: warning: this ‘if’ clause does not guard... [-Wmisleading-indentation]
 1134 |     if (li < 0) li = 0; if (li > 3) li = 3;
      |     ^~
<repo>/menu_startup_render_modern_m12.c:1134:25: note: ...this statement, but the latter is misleadingly indented as if it were guarded by the ‘if’
 1134 |     if (li < 0) li = 0; if (li > 3) li = 3;
      |                         ^~
<repo>/menu_startup_render_modern_m12.c:1135:5: warning: this ‘if’ clause does not guard... [-Wmisleading-indentation]
 1135 |     if (gi < 0) gi = 0; if (gi > 2) gi = 2;
      |     ^~
<repo>/menu_startup_render_modern_m12.c:1135:25: note: ...this statement, but the latter is misleadingly indented as if it were guarded by the ‘if’
 1135 |     if (gi < 0) gi = 0; if (gi > 2) gi = 2;
      |                         ^~
<repo>/menu_startup_render_modern_m12.c:1136:5: warning: this ‘if’ clause does not guard... [-Wmisleading-indentation]
 1136 |     if (wi < 0) wi = 0; if (wi > 1) wi = 1;
      |     ^~
<repo>/menu_startup_render_modern_m12.c:1136:25: note: ...this statement, but the latter is misleadingly indented as if it were guarded by the ‘if’
 1136 |     if (wi < 0) wi = 0; if (wi > 1) wi = 1;
      |                         ^~
# firestaff_m11_phase_a_probe
# SDL major version linked: 3
PASS INV_A12a shutdown without init is no-op
PASS INV_A01 M11_Render_Init returned OK and state is initialised
PASS INV_A02 renderer + texture chain created (no error path taken)
PASS INV_A03 framebuffer is 320*200 = 64000 bytes
PASS INV_A04 palette level 0 index 0 is black
PASS INV_A05 palette level 0 index 15 is white
PASS INV_A06 all 6 palette levels resolvable via F9010_VGA_GetColorRgb
PASS INV_A07 ClearFramebuffer wrote 64000 bytes of colour index 7
PASS INV_A08 Present with all-zero framebuffer succeeded
PASS INV_A09 Present with fully-populated framebuffer succeeded
PASS INV_A10 resize callback updated internal window dimensions
PASS INV_A11 double-init refused without leaking handles
PASS INV_A13 scale mode setter and cycle path cover 1x..stretch
PASS INV_A14 fit mode keeps a centered 16:10 present rect
PASS INV_A14B integer fit mode keeps exact 320x200 pixel multiples
PASS INV_A15 stretch mode uses the full window area
PASS INV_A16 letterbox margins do not map to framebuffer coordinates
PASS INV_A17 window coordinates map back inside framebuffer bounds
PASS INV_A12b shutdown + shutdown is idempotent
# summary: 19/19 invariants passed
M11 Phase A probe: # summary: 19/19 invariants passed
M11 Phase A probe: PASS (19/19)
In file included from <repo>/probes/m11/firestaff_m11_game_view_probe.c:1:
<repo>/m11_game_view.h:135:43: warning: declaration does not declare anything
  135 |     enum { M11_TORCH_FUEL_CAPACITY = 256 };
      |                                           ^
<repo>/probes/m11/firestaff_m11_game_view_probe.c: In function ‘main’:
<repo>/probes/m11/firestaff_m11_game_view_probe.c:2508:49: warning: ‘%s’ directive output may be truncated writing up to 1023 bytes into a region of size 512 [-Wformat-truncation=]
 2508 |             snprintf(gfxPath, sizeof(gfxPath), "%s/GRAPHICS.DAT", dataDir);
      |                                                 ^~
In file included from /usr/include/stdio.h:980,
                 from <repo>/memory_tick_orchestrator_pc34_compat.h:65,
                 from <repo>/m11_game_view.h:5:
In function ‘snprintf’,
    inlined from ‘main’ at <repo>/probes/m11/firestaff_m11_game_view_probe.c:2508:13:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 14 and 1037 bytes into a destination of size 512
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
In file included from <repo>/m11_game_view.c:1:
<repo>/m11_game_view.h:135:43: warning: declaration does not declare anything
  135 |     enum { M11_TORCH_FUEL_CAPACITY = 256 };
      |                                           ^
<repo>/m11_game_view.c: In function ‘m11_draw_v1_message_area’:
<repo>/m11_game_view.c:18559:43: warning: ‘%s’ directive output may be truncated writing up to 79 bytes into a region of size 54 [-Wformat-truncation=]
18559 |         snprintf(clipped, maxChars + 1U, "%s", text);
      |                                           ^~
In file included from /usr/include/stdio.h:980,
                 from <repo>/memory_tick_orchestrator_pc34_compat.h:65,
                 from <repo>/m11_game_view.h:5:
In function ‘snprintf’,
    inlined from ‘m11_draw_v1_message_area’ at <repo>/m11_game_view.c:18559:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 80 bytes into a destination of size 54
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘m11_draw_wall_contents.isra’:
<repo>/m11_game_view.c:7840:55: warning: ‘%d’ directive output may be truncated writing between 1 and 10 bytes into a region of size 4 [-Wformat-truncation=]
 7840 |                 snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                       ^~
<repo>/m11_game_view.c:7840:54: note: directive argument in the range [2, 2147483647]
 7840 |                 snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                      ^~~~
In function ‘snprintf’,
    inlined from ‘m11_draw_wall_contents.isra’ at <repo>/m11_game_view.c:7840:17:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 2 and 11 bytes into a destination of size 4
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘m11_draw_side_feature.isra’:
<repo>/m11_game_view.c:11139:59: warning: ‘%d’ directive output may be truncated writing between 1 and 10 bytes into a region of size 4 [-Wformat-truncation=]
11139 |                     snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                           ^~
<repo>/m11_game_view.c:11139:58: note: directive argument in the range [2, 2147483647]
11139 |                     snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                          ^~~~
In function ‘snprintf’,
    inlined from ‘m11_draw_side_feature.isra’ at <repo>/m11_game_view.c:11139:21:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 2 and 11 bytes into a destination of size 4
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_QuickSave’:
<repo>/m11_game_view.c:4735:40: warning: ‘%s’ directive output may be truncated writing up to 511 bytes into a region of size between 95 and 104 [-Wformat-truncation=]
 4735 |              "F9 RESTORES TICK %u FROM %s",
      |                                        ^~
 4736 |              (unsigned int)state->world.gameTick,
 4737 |              path);
      |              ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_QuickSave’ at <repo>/m11_game_view.c:4734:5:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 25 and 545 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_QuickLoad’:
<repo>/m11_game_view.c:4824:47: warning: ‘%s’ directive output may be truncated writing up to 511 bytes into a region of size between 84 and 93 [-Wformat-truncation=]
 4824 |              "TICK %u HASH %08X RELOADED FROM %s",
      |                                               ^~
......
 4827 |              path);
      |              ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_QuickLoad’ at <repo>/m11_game_view.c:4823:5:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 36 and 556 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘m11_draw_utility_panel’:
<repo>/m11_game_view.c:16594:39: warning: ‘%s’ directive output may be truncated writing up to 63 bytes into a region of size 32 [-Wformat-truncation=]
16594 |         snprintf(line, sizeof(line), "%s",
      |                                       ^~
In function ‘snprintf’,
    inlined from ‘m11_draw_utility_panel’ at <repo>/m11_game_view.c:16594:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 64 bytes into a destination of size 32
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:18804:47: warning: ‘%s’ directive output may be truncated writing up to 95 bytes into a region of size 90 [-Wformat-truncation=]
18804 |         snprintf(line2, sizeof(line2), "HERE  %s", line);
      |                                               ^~   ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_Draw’ at <repo>/m11_game_view.c:18804:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 7 and 102 bytes into a destination of size 96
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:18811:47: warning: ‘%s’ directive output may be truncated writing up to 95 bytes into a region of size 90 [-Wformat-truncation=]
18811 |         snprintf(line2, sizeof(line2), "AHEAD %s", line);
      |                                               ^~   ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_Draw’ at <repo>/m11_game_view.c:18811:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 7 and 102 bytes into a destination of size 96
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:744:37: warning: ‘%s’ directive output may be truncated writing up to 127 bytes into a region of size 80 [-Wformat-truncation=]
  744 |         snprintf(line1, line1Size, "%s", text);
      |                                     ^~
In function ‘snprintf’,
    inlined from ‘m11_dialog_source_split_two_lines’ at <repo>/m11_game_view.c:744:9,
    inlined from ‘M11_GameView_Draw’ at <repo>/m11_game_view.c:19239:29:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 128 bytes into a destination of size 80
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:19124:33: warning: ‘skillY’ may be used uninitialized [-Wmaybe-uninitialized]
19124 |                                 m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
      |                                 ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
19125 |                                               skillX, skillY, skillLine, &skillStyle);
      |                                               ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c:19116:45: note: ‘skillY’ was declared here
19116 |                                 int skillX, skillY;
      |                                             ^~~~~~
<repo>/asset_status_m12.c: In function ‘M12_AssetStatus_Scan’:
<repo>/asset_status_m12.c:256:29: warning: ‘%s’ directive output may be truncated writing up to 575 bytes into a region of size 512 [-Wformat-truncation=]
  256 |     snprintf(out, outSize, "%s", value);
      |                             ^~
......
  312 |             m12_copy_string(matchedPath, M12_ASSET_DATA_DIR_CAPACITY, path);
      |                                                                       ~~~~
In file included from /usr/include/stdio.h:980,
                 from <repo>/asset_status_m12.c:5:
In function ‘snprintf’,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:256:5,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:249:13,
    inlined from ‘m12_try_match_version’ at <repo>/asset_status_m12.c:312:13,
    inlined from ‘m12_fill_game_versions’ at <repo>/asset_status_m12.c:341:17,
    inlined from ‘M12_AssetStatus_Scan’ at <repo>/asset_status_m12.c:379:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 576 bytes into a destination of size 512
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/asset_status_m12.c: In function ‘M12_AssetStatus_Scan’:
<repo>/asset_status_m12.c:256:29: warning: ‘%s’ directive output may be truncated writing up to 1535 bytes into a region of size 512 [-Wformat-truncation=]
  256 |     snprintf(out, outSize, "%s", value);
      |                             ^~
In function ‘snprintf’,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:256:5,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:249:13,
    inlined from ‘m12_fill_game_versions’ at <repo>/asset_status_m12.c:348:21,
    inlined from ‘M12_AssetStatus_Scan’ at <repo>/asset_status_m12.c:379:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 1536 bytes into a destination of size 512
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/menu_startup_m12.c: In function ‘m12_load_runtime_catalog’:
<repo>/menu_startup_m12.c:535:41: warning: ‘%s’ directive output may be truncated writing up to 255 bytes into a region of size 128 [-Wformat-truncation=]
  535 |     snprintf(out + len, outSize - len, "%s", value);
      |                                         ^~
......
  683 |                 m12_append_string(msgid, sizeof(msgid), parsed);
      |                                                         ~~~~~~
In file included from /usr/include/stdio.h:980,
                 from <repo>/menu_startup_m12.c:9:
In function ‘snprintf’,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:535:5,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:526:13,
    inlined from ‘m12_load_runtime_catalog’ at <repo>/menu_startup_m12.c:683:17:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 1 and 256 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/menu_startup_m12.c: In function ‘m12_load_runtime_catalog’:
<repo>/menu_startup_m12.c:535:41: warning: ‘%s’ directive output may be truncated writing up to 255 bytes into a region of size 128 [-Wformat-truncation=]
  535 |     snprintf(out + len, outSize - len, "%s", value);
      |                                         ^~
......
  695 |                     m12_append_string(msgid, sizeof(msgid), parsed);
      |                                                             ~~~~~~
In function ‘snprintf’,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:535:5,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:526:13,
    inlined from ‘m12_load_runtime_catalog’ at <repo>/menu_startup_m12.c:695:21:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 1 and 256 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
PASS INV_GV_01 launcher exposes DM1 as the default builtin launch source with verified assets
PASS INV_GV_02 launcher selection transitions into a real game view through the source hook
PASS INV_GV_302A V1 message log strips Firestaff tick-prefix chrome from boot event text
PASS INV_GV_302B normal V1 keeps source screen-clear gaps black while debug HUD retains slate chrome
PASS INV_GV_430 DM1 primary mouse table maps right-click C151 champion-0 status box to command C007 toggle inventory
PASS INV_GV_431 DM1 primary mouse table maps left-click C151 champion-0 name/hands to command C012 status-box click
PASS INV_GV_432 DM1 primary mouse table scans C187 bar-graph left-click before C151 and returns command C007
PASS INV_GV_433 DM1 mouse zone matching keeps source inclusive right/bottom edges for status boxes
PASS INV_GV_433A DM1 primary mouse table maps left-click C013 spell area to command C100 before falling through
PASS INV_GV_433B DM1 primary mouse table maps left-click C011 action area to command C111 before falling through
PASS INV_GV_434 DM1 secondary movement table maps left-click C007 viewport to command C080 click-in-dungeon-view
PASS INV_GV_435 DM1 secondary movement table maps right-click screen zone, including viewport, to command C083 toggle leader inventory
PASS INV_GV_435A DM1 secondary movement table maps left-click C068 turn-left arrow to command C001
PASS INV_GV_435B DM1 secondary movement table maps left-click C071 move-right arrow to command C004
PASS INV_GV_436 DM1 inventory slot zones C507..C536 route screen clicks through viewport-relative coordinates to commands C028..C057
PASS INV_GV_437 DM1 inventory table gives right-click screen-zone close inventory precedence over viewport-relative slot hits
PASS INV_GV_438 DM1 inventory source slot boxes C507..C536 use source C033 normal slot-box graphic
PASS INV_GV_400 M11 game view builds champion mirror catalog from DUNGEON.DAT at start
PASS INV_GV_401 M11 mirror catalog exposes display name by ordinal
PASS INV_GV_402 M11 mirror catalog exposes display title by ordinal
PASS INV_GV_403 M11 can recruit champion identity by mirror catalog display name
PASS INV_GV_404 M11 mirror ordinal recruit is idempotent for already-present champion
PASS INV_GV_405 M11 resolves the source mirror TextString in the front viewport cell
PASS INV_GV_406 M11 mirror click opens a source-backed resurrect/reincarnate candidate panel
PASS INV_GV_407 M11 mirror panel resurrect command recruits the selected champion
PASS INV_GV_408 viewport source zone C007 is locked to the DM1 224x136 rectangle at screen origin 0,33
PASS INV_GV_409 inventory source panel seam is C020 graphic in layout-696 C101 at 80,52,144x73
PASS INV_GV_410 inventory/action slot-box graphics are source C033/C034/C035 and overhang 16x16 icon cells as 18x18 boxes
PASS INV_GV_411 inventory object-icon atlas seam is 32 icons per GRAPHICS.DAT page, 16x16 source cells starting at graphic 42
PASS INV_GV_412 action-hand icons apply G0498 colour-12 cyan remap while inventory slot icons preserve source colour 12
PASS INV_GV_413 viewport content placement seams expose source C2500 object, C3200 creature, and C2900 projectile points inside C007
PASS INV_GV_414 viewport base uses source ceiling C079 224x39 then floor C078 224x97 inside the 224x136 aperture
PASS INV_GV_415 viewport source draw-order seam is pinned from base/pits/ornaments/walls through doors/buttons
PASS INV_GV_03 game view renders a non-empty dungeon-backed frame
PASS INV_GV_04 game view input turns the party through the real tick orchestrator
PASS INV_GV_05 turning changes the rendered pseudo-viewport frame, not just the inspector text
PASS INV_GV_06 movement changes the rendered pseudo-viewport with real world movement
PASS INV_GV_08 small minimap inset still renders in the corner
PASS INV_GV_09 synthetic viewport harness initialises a focused 3x3 sample state
PASS INV_GV_07 enter now inspects the front-cell target without spending a real tick
PASS INV_GV_07B tab cycles the active front champion and updates the in-view readout
PASS INV_GV_07C space turns front-cell creature contact into a real strike tick for the selected champion
PASS INV_GV_07D clicking the viewport inspects the live front-cell target without spending a tick
PASS INV_GV_07E clicking a champion slot directly arms that champion for the next action
PASS INV_GV_07F clicking the left viewport lane turns the party through the real tick path
PASS INV_GV_07G clicking the lower center viewport advances into a clear front cell without using the HUD arrows
PASS INV_GV_07H clicking the on-screen action button drives the real front-cell attack flow
PASS INV_GV_07I space toggles a closed front door into an animating step and updates the real dungeon square one state closer to OPEN
PASS INV_GV_07M door interaction maps tick emissions to the M11 audio marker pipeline
PASS INV_GV_07J idle cadence advances the real world clock without requiring a manual wait input
PASS INV_GV_07K A strafes relative to facing and moves into the left lane through the real tick path
PASS INV_GV_07L clicking the lower-right viewport lane performs a relative strafe instead of another turn
PASS INV_GV_10 synthetic feature cells add door, stair, and occupancy cues inside the viewport
PASS INV_GV_11 a side door accent stays visible without collapsing the forward corridor window
PASS INV_GV_12 viewport slice and minimap inset coexist in the same frame
PASS INV_GV_12B viewport item and effect cues appear when real thing chains include loot and projectiles
PASS INV_GV_12C runtime viewport rect API returns source DM1 viewport geometry
PASS INV_GV_12D runtime viewport crop is the deterministic 224x136 DM1 aperture inside 320x200
PASS INV_GV_12E two same-state draws produce identical bytes for the 224x136 viewport crop
PASS INV_GV_13 escape from the game view returns control to the launcher
PASS INV_GV_13B quicksave serialises the live dungeon-backed world to a recoverable slot
PASS INV_GV_13C quickload restores the exact live world snapshot after local state drift
PASS INV_GV_13D sidebar save button writes a live quicksave without leaving the viewport
PASS INV_GV_13E sidebar load button restores the last live quicksave in-place
PASS INV_GV_13F sidebar menu header returns control to the launcher
PASS INV_GV_14 sidebar HUD renders separate status and map framing beside the viewport
PASS INV_GV_15 top HUD renders a dedicated party/status strip instead of a single inspector blob
PASS INV_GV_15B party strip reflects source-colored champion bars when champion data exists
PASS INV_GV_15C V1 champion HUD does not draw an invented active-slot yellow rectangle
PASS INV_GV_15D V1 champion HUD leaves unrecruited party slots undrawn
PASS INV_GV_15E V1 champion HUD draws source ready/action hand slot zones inside the status box
PASS INV_GV_15E9 V1 champion HUD status box zones expose layout-696 C151..C154 ids and geometry
PASS INV_GV_15E6 V1 status hand slot zones expose layout-696 C207..C210 parents and C211..C218 child ids/geometry
PASS INV_GV_15V V1 status hand icon zones inset 16x16 object icons within hand slots
PASS INV_GV_15W V1 status hand slot-box zones expose 18x18 C033/C034/C035 overdraw at hand origins
PASS INV_GV_15X V1 status hand slot fallback renders the 18x18 C033 box extent, not the 16x16 parent zone
PASS INV_GV_15E7 V1 status bar graph zones expose layout-696 C187..C190 and C195..C206 ids plus geometry
PASS INV_GV_15E8 V1 status name text zones expose layout-696 C163..C166 ids and geometry
PASS INV_GV_15Y V1 dead status-box fallback preserves source 67x29 C008 extent
PASS INV_GV_15E2 V1 champion HUD name clear zones expose layout-696 C159..C162 ids and geometry
PASS INV_GV_15Z V1 champion HUD name clear uses source C01 gray before centered name text
PASS INV_GV_15AA V1 live status-box fill uses source C12 darkest-gray before overlays
PASS INV_GV_15E3 V1 champion HUD name colors follow F0292 leader yellow / non-leader gold
PASS INV_GV_15E4 V1 champion HUD renders source-colored names inside the compact status name zones
PASS INV_GV_15E5 V1 dead champion HUD prints source centered name in C13 lightest gray
PASS INV_GV_15E10 V1 champion HUD renders all four recruited champion status boxes with source zones, names, and bars
PASS INV_GV_15F V1 champion HUD ready/action hands use normal slot-box graphic when idle
PASS INV_GV_15G V1 champion HUD action hand switches to graphic 35 for the acting champion
PASS INV_GV_15H V1 champion HUD ready-hand wound selects graphic 34 only for ready hand
PASS INV_GV_15I V1 champion HUD action-hand wound selects graphic 34 when idle
PASS INV_GV_15J V1 champion HUD acting action hand overrides wound graphic with graphic 35
PASS INV_GV_15K V1 champion HUD empty normal hands use source icons 212/214
PASS INV_GV_15L V1 champion HUD ready-hand wound advances empty icon to 213 only
PASS INV_GV_15M V1 champion HUD action-hand wound advances empty icon to 215 only
PASS INV_GV_15N V1 champion HUD acting hand changes the box graphic but keeps the source wounded empty icon
PASS INV_GV_15O V1 champion HUD occupied wounded hand uses F0033 object icon instead of empty-hand icon
PASS INV_GV_15OA V1 leader-hand runtime does not synthesize G4055 from the active champion ready-hand slot
PASS INV_GV_15OB V1 leader-hand C017 resolver stays blank when no dedicated source mouse-hand object exists
PASS INV_GV_15OC V1 leader-hand runtime carries a dedicated G4055-equivalent object with source icon/name resolution
PASS INV_GV_15OD normal V1 draws the transient leader-hand object name into source C017
PASS INV_GV_15OE V1 leader-hand remove flow clears the G4055-equivalent object instead of reading champion equipment
PASS INV_GV_15Q V1 status box base graphic uses source dead box only for dead champions
PASS INV_GV_15P V1 status shield border graphic priority follows spell/fire/party source order
PASS INV_GV_15T V1 status shield border zone reuses C007 status box footprint
PASS INV_GV_15R V1 poisoned label zone centers C032 under C007 status box geometry
PASS INV_GV_15S V1 champion damage indicator zones expose C167-C170 ids and center C015 inside C007 geometry
PASS INV_GV_15U V1 champion damage number origin is centered over the C015 damage banner
PASS INV_GV_16 viewport framing uses layered face bands and bright dungeon edges
PASS INV_GV_17 front-cell focus adds a threat-colored viewport reticle plus contextual inspect readout
PASS INV_GV_18 near-lane scanner chips surface left, front, and right contact state inside the viewport
PASS INV_GV_19 post-tick feedback strip renders attack-colored event telemetry inside the HUD
PASS INV_GV_20 forward depth chips summarize the next three center-lane cells with live threat and traversal cues
PASS INV_GV_21 message log contains at least one event after boot and gameplay
Screenshot: <repo>/verification-m11/phase-a/game-view/15_side_ornament_item_creature_count_fidelity.pgm
PASS INV_GV_22 R toggles rest mode on and reports it through last action
PASS INV_GV_22A resting mode uses the source PartyResting input list and suppresses movement ticks until wake-up
PASS INV_GV_22B R again wakes the party from rest mode
PASS INV_GV_23 X triggers stair descent or reports no stairs on the current cell
PASS INV_GV_24 message log count increases or stays stable after gameplay ticks
PASS INV_GV_25 survival drain reduces food over repeated idle ticks
PASS INV_GV_26 party death is detected when all champions reach 0 HP after a tick
PASS INV_GV_27 G picks up the first floor item into the active champion inventory
PASS INV_GV_28 P drops the last held item back to the current cell
PASS INV_GV_29 pickup on empty floor reports nothing to pick up without crashing
PASS INV_GV_30 drop with empty inventory reports nothing to drop without crashing
PASS INV_GV_31 HandleInput routes M12_MENU_INPUT_PICKUP_ITEM through the item pickup path
PASS INV_GV_32 HandleInput routes M12_MENU_INPUT_DROP_ITEM through the item drop path
PASS INV_GV_33 creature AI moves a skeleton toward the party within movement cadence
PASS INV_GV_34 creature on party square deals autonomous damage over time
PASS INV_GV_35 creature attack events appear in the message log
PASS INV_GV_36 creature within sight range moves toward party at its cadence
PASS INV_GV_37 dead creature group does not deal damage
PASS INV_GV_38 creature AI handles constrained movement without crashing
PASS INV_GV_38A focused viewport: D1C normal pit source blit changes the corridor frame
PASS INV_GV_38X focused viewport: D1C normal pit clips inside the DM1 viewport rectangle
PASS INV_GV_38B focused viewport: D1C invisible pit variant differs from normal pit
PASS INV_GV_38Y focused viewport: D1C invisible pit clips inside the DM1 viewport rectangle
PASS INV_GV_38C focused viewport: D1C stairs zone blit changes the corridor frame
PASS INV_GV_38Z focused viewport: D1C stairs clips inside the DM1 viewport rectangle
PASS INV_GV_38D focused viewport: D1C teleporter field zone blit changes the corridor frame
PASS INV_GV_38AA focused viewport: D1C teleporter field clips inside the DM1 viewport rectangle
PASS INV_GV_38L focused viewport: D1C Trolin creature sprite changes the corridor frame
PASS INV_GV_38AB focused viewport: D1C Trolin creature clips inside the DM1 viewport rectangle
PASS INV_GV_38R focused viewport: D1L side-cell Trolin creature differs from empty and center creature frames
PASS INV_GV_38S focused viewport: extreme C3200 side creature clips inside the DM1 viewport rectangle
PASS INV_GV_38M focused viewport: D1C fireball projectile sprite changes the corridor frame
PASS INV_GV_38T focused viewport: D1C fireball projectile clips inside the DM1 viewport rectangle
PASS INV_GV_38Q focused viewport: D1C lightning projectile differs from empty and fireball frames
PASS INV_GV_38U focused viewport: D1C lightning projectile clips inside the DM1 viewport rectangle
PASS INV_GV_38N focused viewport: D1C dagger object sprite changes the corridor frame
PASS INV_GV_38V focused viewport: D1C dagger object clips inside the DM1 viewport rectangle
PASS INV_GV_38O focused viewport: D1C object sprite with G0209 native-index gap changes the corridor frame
PASS INV_GV_38W focused viewport: D1C multi-object pile clips inside the DM1 viewport rectangle
PASS INV_GV_38P focused viewport: D1C multi-object pile differs from single-object frame
PASS INV_GV_38E focused viewport: all normal pit zone specs visibly change their corridor frames
PASS INV_GV_38F focused viewport: all invisible pit zone specs visibly change their corridor frames
PASS INV_GV_38G focused viewport: all stairs front/side zone specs visibly change their corridor frames
PASS INV_GV_38H focused viewport: all teleporter field zone specs visibly change their corridor frames
PASS INV_GV_38I focused viewport: all visibly drawable floor ornament positions change their corridor frames
PASS INV_GV_38J focused viewport: special footprints floor ornament family renders from pre-base graphics
PASS INV_GV_38K focused viewport: all source-bound wall ornament specs change their wall frames
PASS INV_GV_39 corridor square does not trigger pit or teleporter transition
PASS INV_GV_40 corridor at (1,1) does not trigger transition
PASS INV_GV_41 stepping on pit drops party to the level below
PASS INV_GV_42 pit fall deals damage to champion HP
PASS INV_GV_43 pit fall preserves X/Y coordinates on the lower level
PASS INV_GV_44 pit fall writes a log entry mentioning PIT or FELL
PASS INV_GV_45 teleporter transports party to target map and coordinates
PASS INV_GV_46 teleporter applies relative rotation to party direction
PASS INV_GV_47 audible teleporter writes a visible log entry
PASS INV_GV_48 corridor on map 1 does not chain further transitions
PASS INV_GV_49 opening spell panel sets spellPanelOpen flag
PASS INV_GV_50 first rune enters buffer with correct encoded value
PASS INV_GV_51 second rune advances row to 2
PASS INV_GV_52 clear resets rune count and row to zero
PASS INV_GV_53 close panel clears panel flag and buffer
PASS INV_GV_54 casting with fewer than 2 runes returns 0
PASS INV_GV_55 out-of-range symbol indices rejected
PASS INV_GV_56 four consecutive rune entries fill the buffer
PASS INV_GV_57 fifth rune entry rejected when buffer is full
PASS INV_GV_58 SPELL_RUNE_1 input opens panel and enters rune
PASS INV_GV_59 SPELL_CLEAR input closes panel
PASS INV_GV_60 rune encoding matches DM1 formula 0x60+6*row+col
PASS INV_GV_61 CMD_CAST_SPELL for Light spell emits EMIT_SPELL_EFFECT
PASS INV_GV_62 Light spell application increases magicalLightAmount
PASS INV_GV_63 Fireball spell emits EMIT_SPELL_EFFECT with PROJECTILE kind
PASS INV_GV_64 Party Shield spell increases partyShieldDefense
PASS INV_GV_65 invalid spell table index produces no SPELL_EFFECT emission
PASS INV_GV_66 stairs-down (bit 0 clear) descends from map 0 to map 1
PASS INV_GV_67 stairs-up (bit 0 set) ascends from map 1 back to map 0
PASS INV_GV_68 stairs-up on top level (map 0) leads nowhere, party stays
PASS INV_GV_69 stairs-up transition logs ASCENDED message
PASS INV_GV_70 GetSkillLevel returns non-negative for a present champion
PASS INV_GV_71 GetSkillLevel returns -1 for out-of-range champion
PASS INV_GV_72 EMIT_DAMAGE_DEALT awards combat XP to active champion via lifecycle
PASS INV_GV_73 EMIT_SPELL_EFFECT awards magic XP to casting champion via lifecycle
PASS INV_GV_74 potion spell awards priest XP, not wizard XP
PASS INV_GV_75 EMIT_KILL_NOTIFY awards kill XP to active champion
PASS INV_GV_76 EMIT_KILL_NOTIFY with unknown creature type still awards fallback XP
PASS INV_GV_77 UseItem with KU potion heals champion HP
PASS INV_GV_78 doNotDiscard potion converts to empty flask after use
PASS INV_GV_79 UseItem on empty flask returns 0
PASS INV_GV_80 UseItem with empty hands returns 0
PASS INV_GV_81 UseItem on a weapon returns 0 (not consumable)
PASS INV_GV_82 UseItem with DES potion (poison) reduces HP
PASS INV_GV_83 Non-doNotDiscard potion clears slot after use
PASS INV_GV_84 asset loader initializes from GRAPHICS.DAT in the game data directory
PASS INV_GV_85 asset loader enumerates more than 40 graphics from GRAPHICS.DAT
PASS INV_GV_86 wall set graphic 42 loads as 256x32 with valid pixel data
PASS INV_GV_87 wall texture contains at least 3 distinct palette colors
PASS INV_GV_88 floor tile graphic 76 loads as 32x32 with valid pixel data
PASS INV_GV_89 full-screen graphic 4 loads as 320x200
PASS INV_GV_90 QuerySize returns 256x32 for wall set graphic 42
PASS INV_GV_90B odd-width action/PASS area graphic 10 loads as 87x45 with visible palette data
PASS INV_GV_90C movement arrows graphic 13 loads as 87x45 with visible palette data
PASS INV_GV_91 blitting wall texture produces substantial non-zero pixel coverage
PASS INV_GV_92 asset-backed viewport uses at least 6 distinct palette colors
PASS INV_GV_93 repeated load of same graphic returns cached slot
PASS INV_GV_94 zero-sized placeholder graphic returns NULL from loader
PASS INV_GV_95 BlitScaled renders floor tile into 100x60 target rect
PASS INV_GV_96 BlitRegion extracts 64x16 sub-rect from wall texture
PASS INV_GV_97 viewport background graphic 0 loads as 224x136
PASS INV_GV_98 door frame graphic 73 loads as 78x74 (mid depth)
PASS INV_GV_99 door frame graphic 70 loads as 36x49 (far depth)
PASS INV_GV_100 door side graphic 86 loads as 32x123
PASS INV_GV_101 stair graphic 95 loads as 60x111
PASS INV_GV_102 creature sprite base 584/M618 loads as 112x84
PASS INV_GV_103 creature type 1 sprite 588 loads as 64x66
PASS INV_GV_104 floor panel graphic 78 loads as 224x97
PASS INV_GV_105 ceiling panel graphic 79 loads as 224x39
PASS INV_GV_106 near corridor band has more content than dimmed far band
PASS INV_GV_107 asset-backed game view uses at least 8 distinct palette entries
PASS INV_GV_108 asset-backed full frame uses 10+ distinct palette entries
PASS INV_GV_109 object sprite graphic 566 (M612 potion aspect) loads from GRAPHICS.DAT
PASS INV_GV_110 per-map wall set index is in valid range (0-15)
PASS INV_GV_110B source wall/stairs blits offset full wallset graphic range by current map wallSet
PASS INV_GV_111 per-map floor set index is in valid range
PASS INV_GV_112 object sprite graphic 583 (end of M612 family) loads from GRAPHICS.DAT
PASS INV_GV_113 object sprite graphic 500 (scroll aspect) loads from GRAPHICS.DAT
PASS INV_GV_114 wall ornament graphic 259/M615 is loadable from GRAPHICS.DAT
PASS INV_GV_114B object sprite uses G0209 firstNative gap: weapon subtype 43 -> aspect 65 -> graphic 565
PASS INV_GV_114C object source scale units match G2030 table
PASS INV_GV_114C2 object source scale-index selection follows F0115 front/back cells
PASS INV_GV_114C3 object placement binds C2500 layout-696 source zone samples
PASS INV_GV_114D object pile shift index pairs match G0217 samples
PASS INV_GV_114E object shift values match G0223 samples
PASS INV_GV_114E2 object aspect GraphicInfo and CoordinateSet samples match G0209
PASS INV_GV_114E3 object flip-on-right follows G0209 GraphicInfo and relative cell
PASS INV_GV_114F creature D3/D2 palette-change samples match G0221/G0222
PASS INV_GV_114F2 creature slot-9/slot-10 replacement colors match source samples
PASS INV_GV_115 viewport rendering differs when item is on floor vs absent
PASS INV_GV_116 light level is 0 when no light sources present
PASS INV_GV_117 magicalLightAmount raises computed light level
PASS INV_GV_118 light level clamps to 255
PASS INV_GV_119 viewport rendering differs between dark and bright light
PASS INV_GV_120 light indicator area has non-black content in utility panel
PASS INV_GV_121 dark scene has more black pixels in viewport than bright
PASS INV_GV_122 negative magicalLightAmount clamps to 0
PASS INV_GV_123 torch fuel is 0 before first update
PASS INV_GV_124 torch fuel initialized to INITIAL-1 after first update
PASS INV_GV_125 flamitt fuel initialized to FLAMITT_INITIAL-1 after first update
PASS INV_GV_126 full-fuel torch gives more light than half-fuel torch
PASS INV_GV_127 torch extinguished when fuel reaches 0
PASS INV_GV_128 GetTorchFuel returns 0 for out-of-range index
PASS INV_GV_129 unlit torch does not consume fuel
PASS INV_GV_130 light is 0 with extinguished torches and no magic
PASS INV_GV_131 initial animTick is 0
PASS INV_GV_132 initial damageFlashTimer is 0
PASS INV_GV_133 initial attackCueTimer is 0
PASS INV_GV_134 TickAnimation increments animTick
PASS INV_GV_135 NotifyDamageFlash sets damageFlashTimer
PASS INV_GV_136 NotifyDamageFlash sets attackCueTimer
PASS INV_GV_137 TickAnimation decrements damageFlashTimer
PASS INV_GV_138 TickAnimation decrements attackCueTimer
PASS INV_GV_139 damageFlashTimer stays at 0 when already 0
PASS INV_GV_140 CreatureAnimFrame returns 0 or 1
PASS INV_GV_141 CreatureAnimFrame cycles through both frames
PASS INV_GV_142 different creature types have different anim phase
PASS INV_GV_143 flash timers fully decay to 0
PASS INV_GV_144 damage flash renders red pixels on viewport border
PASS INV_GV_145 attack cue renders yellow slash marks
PASS INV_GV_146 NULL-state animation queries return 0 safely
PASS INV_GV_147 NULL-state attack cue creature type returns -1
PASS INV_GV_148 active attack cue reports correct creature type
PASS INV_GV_149 NotifyDamageFlash sets attack cue creature type
PASS INV_GV_150 side-cell creature drawing code path exercised safely
PASS INV_GV_151 BlitScaledMirror produces horizontally flipped output
PASS INV_GV_152 BlitScaledMirror skips transparent pixels
PASS INV_GV_153 V1 depth dimming encodes palette level in upper bits
PASS INV_GV_154 V1 depth dimming preserves original colour index
PASS INV_GV_155 M11_FB_ENCODE/DECODE round-trip for all index/level combos
PASS INV_GV_156 front-facing D1 GiantScorpion view selects native front bitmap (M618+0)
PASS INV_GV_157 side-facing D1 GiantScorpion falls back to front (no SIDE bit, no FLIP_NON_ATTACK)
PASS INV_GV_158 back-facing D2 GiantScorpion falls back to derived front D2 (no BACK bit)
PASS INV_GV_159 front-facing attack GiantScorpion falls back to front (no ATTACK bit, no FLIP_ATTACK)
PASS INV_GV_260 GiantScorpion GraphicInfo matches ReDMCSB CREATURE_INFO (0x0482)
PASS INV_GV_261 Trolin GraphicInfo matches ReDMCSB CREATURE_INFO (0x05B8)
PASS INV_GV_262 GiantScorpion has no side/back/attack bitmaps per MASK0x0008/0x0010/0x0020
PASS INV_GV_263 Trolin has side/back/attack bitmaps (no FLIP_NON_ATTACK) per 0x05B8
PASS INV_GV_264 side-facing D1 Trolin selects native side bitmap (584+43+1) with mirror for relFacing=1
PASS INV_GV_265 back-facing D2 Trolin selects derived back D2 bitmap (663+5=668) without mirror
PASS INV_GV_266 front-facing attack D1 Trolin selects native attack bitmap (584+43+3=630)
PASS INV_GV_267 PainRat GraphicInfo 0x04B4: no SIDE, has BACK, has ATTACK, has FLIP_NON_ATTACK
PASS INV_GV_268 side-facing D1 PainRat falls back to front (584+10) mirrored (FLIP_NON_ATTACK set)
PASS INV_GV_269 back-facing D1 PainRat selects native back bitmap (584+10+2=596)
PASS INV_GV_270 creature 0 (GiantScorpion) GI=0x0482: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=4 derivedCount=8
PASS INV_GV_271 creature 1 (SwampSlime) GI=0x0480: ADD=0 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=2 derivedCount=2
PASS INV_GV_272 creature 2 (Giggler) GI=0x4510: ADD=0 SPECIAL_D2=0 D2_FLIPPED=1 FLIP_DURING_ATTACK=1 maxH=0 maxV=1 nativeCount=2 derivedCount=4
PASS INV_GV_273 creature 3 (PainRat) GI=0x04B4: ADD=0 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=4 derivedCount=6
PASS INV_GV_274 creature 4 (Ruster) GI=0x0701: ADD=1 SPECIAL_D2=0 D2_FLIPPED=1 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=2 derivedCount=5
PASS INV_GV_275 creature 5 (Screamer) GI=0x0581: ADD=1 SPECIAL_D2=1 D2_FLIPPED=1 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=2 derivedCount=5
PASS INV_GV_276 creature 6 (Rockpile) GI=0x070C: ADD=0 SPECIAL_D2=0 D2_FLIPPED=1 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=2 derivedCount=4
PASS INV_GV_277 creature 7 (GhostRive) GI=0x0300: ADD=0 SPECIAL_D2=0 D2_FLIPPED=1 FLIP_DURING_ATTACK=0 maxH=0 maxV=0 nativeCount=1 derivedCount=2
PASS INV_GV_278 creature 8 (WaterElemental) GI=0x5864: ADD=0 SPECIAL_D2=0 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=1 maxV=1 nativeCount=2 derivedCount=4
PASS INV_GV_279 creature 9 (Couatl) GI=0x0282: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=0 maxV=0 nativeCount=4 derivedCount=8
PASS INV_GV_280 creature 10 (StoneGolem) GI=0x1480: ADD=0 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=1 maxH=1 maxV=0 nativeCount=2 derivedCount=2
PASS INV_GV_281 creature 11 (Mummy) GI=0x18C6: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=1 maxV=0 nativeCount=2 derivedCount=8
PASS INV_GV_282 creature 12 (Skeleton) GI=0x1280: ADD=0 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=1 maxV=0 nativeCount=2 derivedCount=2
PASS INV_GV_283 creature 13 (MagentaWorm) GI=0x14A2: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=1 maxH=1 maxV=0 nativeCount=5 derivedCount=10
PASS INV_GV_284 creature 14 (Trolin) GI=0x05B8: ADD=0 SPECIAL_D2=1 D2_FLIPPED=1 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=4 derivedCount=8
PASS INV_GV_285 creature 15 (GiantWasp) GI=0x0381: ADD=1 SPECIAL_D2=1 D2_FLIPPED=1 FLIP_DURING_ATTACK=0 maxH=0 maxV=0 nativeCount=2 derivedCount=5
PASS INV_GV_286 creature 16 (Antman) GI=0x0680: ADD=0 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=2 derivedCount=2
PASS INV_GV_287 creature 17 (Vexirk) GI=0x04A0: ADD=0 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=3 derivedCount=4
PASS INV_GV_288 creature 18 (AnimatedArmour) GI=0x0280: ADD=0 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=0 maxV=0 nativeCount=2 derivedCount=2
PASS INV_GV_289 creature 19 (Materializer) GI=0x4060: ADD=0 SPECIAL_D2=0 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=0 maxV=1 nativeCount=2 derivedCount=4
PASS INV_GV_290 creature 20 (RedDragon) GI=0x10DE: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=1 maxV=0 nativeCount=4 derivedCount=12
PASS INV_GV_291 creature 21 (Oitu) GI=0x0082: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=0 maxV=0 nativeCount=4 derivedCount=8
PASS INV_GV_292 creature 22 (Demon) GI=0x1480: ADD=0 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=1 maxH=1 maxV=0 nativeCount=2 derivedCount=2
PASS INV_GV_293 creature 23 (LordChaos) GI=0x78AA: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=3 maxV=1 nativeCount=6 derivedCount=12
PASS INV_GV_294 creature 24 (LordOrder) GI=0x068A: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=5 derivedCount=10
PASS INV_GV_295 creature 25 (GreyLord) GI=0x78AA: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=3 maxV=1 nativeCount=6 derivedCount=12
PASS INV_GV_296 creature 26 (LordChaosRedDragon) GI=0x78AA: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=3 maxV=1 nativeCount=6 derivedCount=12
PASS INV_GV_297 out-of-range creature type returns 0 for all source-backed GraphicInfo queries
PASS INV_GV_298 every creature has ≥ 1 native + ≥ 2 derived slots (source-backed cumulative totals)
PASS INV_GV_299 native-bitmap count matches independent F097_xxxx_DUNGEONVIEW_LoadGraphics recomputation for all 27 creatures
PASS INV_GV_156 Font init produces unloaded state
PASS INV_GV_157 Original DM1 font loads from GRAPHICS.DAT
PASS INV_GV_158 Font DrawChar 'A' produces visible pixels
PASS INV_GV_159 Font DrawString 'HELLO' renders visible text
PASS INV_GV_160 Font MeasureString('ABC') == 18 pixels
PASS INV_GV_161 Game view Draw uses original DM1 font when available
PASS INV_GV_162 IsGameWon returns 0 for fresh game view
PASS INV_GV_163 GetGameWonTick returns 0 for fresh game view
PASS INV_GV_164 gameWon flag and tick are queryable
PASS INV_GV_165 Endgame victory overlay renders differently from normal
PASS INV_GV_165B V1 endgame overlay keeps invented tick/help text out of default source path
PASS INV_GV_165C V1 endgame uses source C006 The End graphic
PASS INV_GV_165D V1 endgame uses source champion mirror graphic zone
PASS INV_GV_165H V1 endgame blits source champion portrait into C416
PASS INV_GV_165E V1 endgame prints champion name at source coordinate
PASS INV_GV_165I V1 endgame prints raw source champion title after name
PASS INV_GV_165L V1 endgame does not invent champion title from name alone
PASS INV_GV_165K V1 endgame prefers raw Champion.Title bytes when present
PASS INV_GV_165M V1 endgame title x spacing honors source punctuation rule
PASS INV_GV_165F V1 endgame prints source fighter skill-title line
PASS INV_GV_165G V1 endgame prints source ninja/priest/wizard skill-title lines
PASS INV_GV_165J V1 endgame skill levels ignore temporary XP
PASS INV_GV_166 HandleInput ignores movement when game is won
PASS INV_GV_167 HandleInput accepts ESC to return to menu when game won
PASS INV_GV_168 AdvanceIdleTick blocked when game is won
PASS INV_GV_169 IsDialogOverlayActive returns 0 for fresh game view
PASS INV_GV_170 ShowDialogOverlay activates overlay
PASS INV_GV_171 DismissDialogOverlay clears overlay
PASS INV_GV_172 Dialog overlay renders differently from normal
PASS INV_GV_172B V1 dialog overlay keeps placeholder title/footer debug-only
PASS INV_GV_173 HandleInput dismisses dialog overlay on keypress
PASS INV_GV_172C V1 dialog overlay blits source C000 dialog-box backdrop
PASS INV_GV_172D V1 dialog overlay prints source C450 version-zone text
PASS INV_GV_172E V1 dialog message text is centered in source viewport region
PASS INV_GV_172F V1 single-choice dialog message uses reconstructed C469 vertical zone
PASS INV_GV_172G V1 long dialog message uses source-width two-line split
PASS INV_GV_172H V1 source dialog renders bottom C462 choice text zone
PASS INV_GV_172I V1 two-choice dialog uses source C471/C463/C462 zones
PASS INV_GV_172J V1 three-choice dialog uses source C463/C466/C467 zones
PASS INV_GV_172K V1 four-choice dialog uses source C464-C467 zones
PASS INV_GV_172L V1 dialog accept selects first source choice
PASS INV_GV_172M V1 dialog mouse hit selects source choice zone
PASS INV_GV_172N V1 single-choice dialog applies source M621/C451 patch
PASS INV_GV_172O V1 two/four-choice dialogs apply source M622/M623 patches
PASS INV_GV_174 AdvanceIdleTick blocked during dialog overlay
PASS INV_GV_175 EMIT_GAME_WON emission sets gameWon flag
PASS INV_GV_176 EMIT_PARTY_DEAD emission sets partyDead flag
PASS INV_GV_177 Dialog/endgame query APIs are NULL-safe
PASS INV_GV_178 Map overlay is initially inactive
PASS INV_GV_179 ToggleMapOverlay activates map
PASS INV_GV_180 ToggleMapOverlay twice deactivates map
PASS INV_GV_181 MAP_TOGGLE input activates debug map overlay
PASS INV_GV_181B default V1 parity input ignores invented map overlay
PASS INV_GV_182 Movement blocked while map overlay active
PASS INV_GV_183 ESC closes map overlay
PASS INV_GV_184 Map overlay renders differently from normal view
PASS INV_GV_185 AdvanceIdleTick blocked during map overlay
PASS INV_GV_186 MAP_TOGGLE while map active deactivates it
PASS INV_GV_187 Inventory panel is initially inactive
PASS INV_GV_188 ToggleInventoryPanel activates inventory
PASS INV_GV_189 ToggleInventoryPanel twice deactivates inventory
PASS INV_GV_190 INVENTORY_TOGGLE input activates inventory panel
PASS INV_GV_191 Inventory panel renders differently from normal view
PASS INV_GV_359 normal V1 inventory only changes source viewport replacement rectangle
PASS INV_GV_360 normal V1 inventory ready-hand icon is confined to source C507 slot box
PASS INV_GV_361 normal V1 inventory head icon is confined to source C509 slot box
PASS INV_GV_362 V1 inventory champion slots map to source slot-box indices C513..C527 without alias leakage
PASS INV_GV_362A normal V1 inventory slot click picks object into dedicated leader-hand runtime state
PASS INV_GV_362B V1 source backpack slots C528..C536 route but do not alias compact Firestaff backpack slots
PASS INV_GV_363 normal V1 inventory pouch/quiver/backpack icons are confined to source C513/C514/C527 boxes
PASS INV_GV_192 Movement blocked while inventory panel active
PASS INV_GV_193 ESC closes inventory panel
PASS INV_GV_194 AdvanceIdleTick blocked during inventory panel
PASS INV_GV_195 Selected slot starts at 0 when inventory opens
PASS INV_GV_196 DOWN input in inventory advances selected slot
PASS INV_GV_197 MAP_TOGGLE closes inventory and opens debug map
PASS INV_GV_198 INVENTORY_TOGGLE closes map and opens inventory
PASS INV_GV_199 SlotName returns non-NULL for all named slots
PASS INV_GV_200 Map/inventory query APIs are NULL-safe
PASS INV_GV_353 V1 inventory equipment slot zones expose layout-696 C507..C519 ids and geometry
PASS INV_GV_354 V1 inventory backpack grid exposes layout-696 C520..C536 ids and geometry
PASS INV_GV_355 V1 inventory source-zone helpers reject invalid ordinals
PASS INV_GV_356 V1 inventory source slot-box helper preserves DEFS.H indices 8..37
PASS INV_GV_357 V1 inventory backdrop exposes source C017 in viewport replacement zone
PASS INV_GV_358 inventory backdrop C017 loads as 224x136 from GRAPHICS.DAT
PASS INV_GV_201 slot box normal (graphic 33) loads as 18x18 from GRAPHICS.DAT
PASS INV_GV_202 slot box wounded (graphic 34) loads as 18x18 from GRAPHICS.DAT
PASS INV_GV_203 slot box acting-hand (graphic 35) loads as 18x18 from GRAPHICS.DAT
PASS INV_GV_204 panel empty (graphic 20) loads as 144x73 from GRAPHICS.DAT
PASS INV_GV_205 status box frame (graphic 7) loads as 67x29 from GRAPHICS.DAT
PASS INV_GV_206 status box dead (graphic 8) loads as 67x29 from GRAPHICS.DAT
PASS INV_GV_207 champion portraits (graphic 26) loads as 256x87 from GRAPHICS.DAT
PASS INV_GV_208 food label (graphic 30) loads as 34x9 from GRAPHICS.DAT
PASS INV_GV_209 water label (graphic 31) loads as 46x9 from GRAPHICS.DAT
PASS INV_GV_210 poisoned label (graphic 32) loads as 96x15 from GRAPHICS.DAT
PASS INV_GV_211 party shield border (graphic 37) loads as 67x29 from GRAPHICS.DAT
PASS INV_GV_212 fire shield border (graphic 38) loads as 67x29 from GRAPHICS.DAT
PASS INV_GV_213 spell shield border (graphic 39) loads as 67x29 from GRAPHICS.DAT
PASS INV_GV_214 shield border drawn when partyShieldDefense > 0
PASS INV_GV_215 POISONED label drawn when champion poisonDose > 0
PASS INV_GV_216 damage to champion small (graphic 15) loads as 45x7 from GRAPHICS.DAT
PASS INV_GV_217 damage to champion big (graphic 16) loads as 32x29 from GRAPHICS.DAT
PASS INV_GV_218 damage to creature (graphic 14) loads as 88x45 from GRAPHICS.DAT
PASS INV_GV_219 per-champion damage indicator drawn when timer > 0
Screenshot: <repo>/verification-m11/phase-a/game-view/inventory_slotbox_gfx.pgm
Screenshot: <repo>/verification-m11/phase-a/game-view/party_hud_statusbox_gfx.pgm
Screenshot: <repo>/verification-m11/phase-a/game-view/party_hud_statusbox_gfx_vga.ppm
Screenshot: <repo>/verification-m11/phase-a/game-view/party_hud_four_champions_vga.ppm
Screenshot: <repo>/verification-m11/phase-a/game-view/map_overlay_p5.pgm
Screenshot: <repo>/verification-m11/phase-a/game-view/shield_poison_hud.pgm
PASS INV_GV_220 NotifyCreatureHit sets overlay timer and damage amount
PASS INV_GV_221 creature-hit overlay timer reaches 0 after sufficient ticks
PASS INV_GV_222 graphic-14 creature-hit overlay changes viewport pixels
PASS INV_GV_223 graphic-16 inventory damage overlay draws when active champion hit
PASS INV_GV_224 creature anim frames valid for types 0 and 1
PASS INV_GV_225 projectile sprite (graphic 454/M613) loads as 14x11 from GRAPHICS.DAT
PASS INV_GV_226 all 32 projectile sprites (454-485/M613 family) load from GRAPHICS.DAT
PASS INV_GV_227 projectile sprite (graphic 479) loads as 84x18 from GRAPHICS.DAT
PASS INV_GV_228 projectile sprite (graphic 480) loads as 8x14 from GRAPHICS.DAT
PASS INV_GV_229 side-cell creature perspective scaling: Giggler large sprite loadable
PASS INV_GV_230 floor item scatter: different types/subtypes produce different positions
PASS INV_GV_231 explosion type classification: fire/poison/lightning ranges are non-overlapping
PASS INV_GV_232 multi-creature stacking: single creature group produces visible viewport pixels
PASS INV_GV_233 multi-item floor scatter: floor area has visible content when items present
PASS INV_GV_234 wall ornament depth scaling: scale factors decrease monotonically with depth
Screenshot: <repo>/verification-m11/phase-a/game-view/combat_damage_overlays.pgm
PASS INV_GV_235 door ornament depth scaling: side-pane scale factors decrease monotonically
PASS INV_GV_236 viewport area has visible content after draw (items/creatures/ornaments)
PASS INV_GV_237 creature group count+1 is at least 1 for first group
PASS INV_GV_238 side-pane wall ornament graphic base index is valid
PASS INV_GV_239 side-pane projectile gfx index range covers arrow-type projectile
PASS INV_GV_240 creature duplication count matches group size for 3-creature group
PASS INV_GV_241 creature duplication clamps correctly for 7-creature group
PASS INV_GV_242 projectile relative direction computation is correct for all quadrants
PASS INV_GV_243 projectile sprite mirroring triggers only for relDir=1 (right-bound)
PASS INV_GV_244 side-pane creature attack pose activates at depth 0 for both sides
PASS INV_GV_245 projectile sprite transparency key is palette index 10 (C10_COLOR_FLESH)
PASS INV_GV_245B projectile source scale units match G0215 for D1/D2/D3 front/back cells
PASS INV_GV_245C projectile aspect firstNative/GraphicInfo samples match G0210
PASS INV_GV_245D projectile G0210 aspect bitmap delta handles lightning rotation and fireball no-rotation
PASS INV_GV_245E projectile C0 back/rotation aspect applies horizontal+vertical flip flags while C3 fireball stays unflipped
PASS INV_GV_245F projectile placement binds C2900 layout-696 source zone samples
PASS INV_GV_246 projectile sub-cell rotation produces correct relative cells
PASS INV_GV_247 Z-order: floor items drawn before creatures (structural)
PASS INV_GV_248 floor ornament index cache stores values (not skipped)
Screenshot: <repo>/verification-m11/phase-a/game-view/16_projectile_facing_creature_attack_ornament.pgm
PASS INV_GV_250 creature type 0 (GiantScorpion) coordSet=1 transparent=13
PASS INV_GV_251 creature type 6 (Rockpile) coordSet=0 transparent=13
PASS INV_GV_252 creature type 20 (RedDragon) coordSet=1 transparent=4
PASS INV_GV_253 out-of-range creature type returns coordSet=0 transparent=0
PASS INV_GV_254 coord set 1 single creature uses original D1 c10 center/bottom
PASS INV_GV_255 coord set 1 D2 pair uses original c6/c7 positions
PASS INV_GV_256 coord set 0 single creature uses original D3 c5 center/bottom
PASS INV_GV_256B creature placement binds C3200 layout-696 source zone samples
PASS INV_GV_256C creature draw path prefers C3200 over older G0224 midpoint for single front slot
PASS INV_GV_256D side-cell creature placement binds C3200 left/right source zone samples
PASS INV_GV_257 floor ornament ordinal query returns >= 0 for front cell
Screenshot: <repo>/verification-m11/phase-a/game-view/17_floor_ornament_creature_aspect.pgm
PASS INV_GV_15AF V1 champion icons select C028 strip cells via M026 direction-relative source index
PASS INV_GV_15AG V1 champion HUD renders source-colored C113/C114 icon cells and leaves absent icon slots black
PASS INV_GV_350 normal V1 top chrome outside source status boxes/icons contains no title/debug text pixels
PASS INV_GV_351 normal V1 viewport floor/ceiling obeys DM1 parity flip
PASS INV_GV_300D action-hand icon cell zones expose layout-696 C088..C096 ids and geometry
PASS INV_GV_300K V1 object icon source zones resolve 16x16 cells across F0042+ graphics
PASS INV_GV_300L V1 action object icon palette remaps color-12 nybbles to cyan only in action cells
PASS INV_GV_300N V1 action icon cell backdrop color is cyan for living, black for dead, absent ignored
PASS INV_GV_300M V1 action icon hatch gate follows resting/candidate global source disable states
PASS INV_GV_300AL movement arrow panel exposes DATA.C outer box, C009/C013, and layout-696 C068-C073 geometry
PASS INV_GV_300AN screen and centered-dialog zones expose layout-696 C002/C005 geometry
PASS INV_GV_300AO explosion pattern and viewport-centered text zones expose layout-696 C004/C006 geometry
PASS INV_GV_300AM message area zone exposes layout-696 C014/C015 bottom-anchored geometry
PASS INV_GV_300AM2 V1 message area renders player-facing rows in source C015 and suppresses telemetry
PASS INV_GV_300AJ viewport zone exposes layout-696 C007 id and DM1 PC geometry
PASS INV_GV_300AH leader hand object-name zone exposes layout-696 C017 id and geometry
PASS INV_GV_300H action area zone exposes source C011/COMMAND.C right-column geometry
PASS INV_GV_300I spell area graphic anchors at ReDMCSB C013 right-column source position
PASS INV_GV_300AC spell caster panel zones expose layout-696 C221/C224 ids at ReDMCSB C013 position
PASS INV_GV_300AE action result zone exposes layout-696 C075 id and action-area geometry
PASS INV_GV_300AD action PASS zone exposes ReDMCSB COMMAND.C C112 right-aligned geometry
PASS INV_GV_300AF action menu graphic zones route C079/C077/C011 to source-sized rectangles at COMMAND.C position
PASS INV_GV_300AI inventory panel and food/water zones expose layout-696 C101/C103/C104 geometry
PASS INV_GV_300P right-column V1 action graphic uses source C010 with C079/C077/C011 menu zones
PASS INV_GV_300R V1 action+spell strip union covers source C013/C011 right-column stack
PASS INV_GV_300S V1 champion identity graphics use source C026 portraits and C028 icons
PASS INV_GV_300AR V1 champion icon invisibility palette mirrors source G2362 remap bytes
PASS INV_GV_300AK champion icon zones expose layout-696 C113-C116 ids and clipped geometry
PASS INV_GV_300T V1 HUD condition/damage graphics use source C032/C015/C016/C014 ids
PASS INV_GV_300U V1 inventory panel status uses source C020 panel and C030/C031 food-water labels
PASS INV_GV_300V V1 endgame graphics use source C006 The End and C346 champion mirror ids
PASS INV_GV_300AP V1 endgame zones expose source C412-C419, title, text, skill and restart/quit geometry
PASS INV_GV_300W V1 status slot and shield frame graphics use source C007/C008/C033-C035/C037-C039 ids
PASS INV_GV_300Y V1 champion bar colors use source G0046 order with C12 blank bars
PASS INV_GV_300X V1 dialog backdrop/version/choice patches use source C000/C450/M621-M623 geometry
PASS INV_GV_300Z V1 dialog message zones and vertical origins use source C469/C471 geometry
PASS INV_GV_300AA V1 dialog choice text zone ids expose source C462-C467 layout cases
PASS INV_GV_300AB V1 dialog pointer hit zones expose source C456-C461 button zones
PASS INV_GV_300AG spell symbol zones expose layout-696 C245-C260, C261-C264, C252 and C254 ids
PASS INV_GV_300Q V1 spell label cells use source C011 lines graphic rows for available/selected states
PASS INV_GV_300AQ normal V1 spell panel stays in C013 right-column area and uses selected C011 cells, not the old modal viewport panel
PASS INV_GV_300G action menu header zone exposes F0387 source zone 80 geometry
PASS INV_GV_300F action menu row zones expose COMMAND.C C113-C115 / F0387 zones 85-87 geometry
PASS INV_GV_300J action menu text origins match ACTIDRAW.C F0387 PC coordinates
PASS INV_GV_300O action menu colors match F0387 cyan header/black name and black rows/cyan actions
PASS INV_GV_300E action-hand icon pointer hit uses source C092 rightmost cell geometry
# DM-action-icon-cells probe: assetsAvailable=1 cyanCellsDrawn=2
PASS INV_GV_300 action-hand icon cells: both living champions get cyan backdrop (or no assets)
PASS INV_GV_300A action icon mode fills the source action area top band black before drawing cells
PASS INV_GV_300B action-hand icon cells: empty living hand blits source empty-hand icon
PASS INV_GV_300C action-hand icon cells hatch living cells during rest/candidate lockout
PASS INV_GV_301 action-hand icon cells: dead champion cell is solid black
PASS INV_GV_302 action-hand icon cells: absent champion slot does not receive a full cyan overlay
PASS INV_GV_303 action-hand icon cells: rightmost cell ends at x=318 (in-bounds)
PASS INV_GV_304 action-hand icon cells: ActionSetIndex==0 item (Compass) leaves inner cell fully cyan
PASS INV_GV_305 action-hand icon cells: ActionSetIndex>0 item blits source object icon
PASS INV_GV_306 action-hand icon cells: source object icon applies G0498 color-12-to-cyan palette change
PASS INV_GV_307 action-hand icon cells: lit torch uses source charge-count icon variant
PASS INV_GV_308 action-hand icon cells: charged weapon uses source +1 icon variant
PASS INV_GV_309 object icon resolver follows source scroll, compass, and charged-junk variants
PASS INV_GV_309B inventory slot icons use source object icons without action palette remap
Screenshot: <repo>/verification-m11/phase-a/game-view/18_dm_action_hand_icon_cells.pgm
PASS INV_GV_310 action-menu: empty-hand champion activates with ActionSet 2
PASS INV_GV_311 action-menu: acting ordinal stored as DM1 1-based value
PASS INV_GV_312 action-menu: empty-hand ActionSet yields PUNCH/KICK/WAR CRY indices
PASS INV_GV_313 action-menu: header band is cyan when acting champion set
PASS INV_GV_314 action-menu: inner icon-cell cyan fill is suppressed vs icon-mode
PASS INV_GV_315 action-menu: ClearActingChampion returns to idle mode
PASS INV_GV_316 action-menu: ready-hand-only object does not override empty action hand
PASS INV_GV_320 action-menu: invalid row index leaves acting champion set
PASS INV_GV_321 action-menu: PUNCH row click performs action, clears menu, logs message
PASS INV_GV_322 action-menu: PUNCH row click advances (or holds) game tick via CMD_ATTACK
PASS INV_GV_323 action-menu: WAR CRY row click clears menu and logs without committing a strike tick
PASS INV_GV_324 action-menu: TriggerActionRow in idle mode is a no-op
PASS INV_GV_325 action-menu: HandlePointer row-hit closes menu and redraws
Screenshot: <repo>/verification-m11/phase-a/game-view/19_dm_action_menu_mode.pgm
Screenshot: <repo>/verification-m11/phase-a/game-view/20_dm_action_menu_post_click.pgm
PASS INV_GV_326 non-melee action: WAR CRY emits an audio marker
PASS INV_GV_327 non-melee action: WAR CRY advances a time-passes tick
PASS INV_GV_328 non-melee action: acting champion becomes party leader
PASS INV_GV_329 non-melee action: action name 36 is HEAL per G0490 table
PASS INV_GV_330 non-melee action: action name 38 is LIGHT per G0490 table
PASS INV_GV_331 non-melee action: action name 11 is FREEZE LIFE per G0490 table
PASS INV_GV_332 projectile action: rows 20/23/21 are FIREBALL/LIGHTNING/DISPELL per G0490 table
PASS INV_GV_333 projectile action: rows 27/32/42 are INVOKE/SHOOT/THROW per G0490 table
PASS INV_GV_334 projectile action: FIREBALL spawns subtype 0x80
PASS INV_GV_335 projectile action: LIGHTNING spawns subtype 0x82
PASS INV_GV_336 projectile action: DISPELL spawns subtype 0x83
PASS INV_GV_337 projectile action: INVOKE spawns one of the 4 F0407 C027 subtypes
PASS INV_GV_338 projectile action: SHOOT with empty ready hand emits NO AMMUNITION and spawns nothing
PASS INV_GV_339 projectile action: SHOOT with ready-hand ammo spawns kinetic projectile
PASS INV_GV_340 projectile action: THROW with item in action hand spawns kinetic projectile
PASS INV_GV_341 projectile travel: FIREBALL spawns at party cell (startY)
PASS INV_GV_342 projectile travel: first advance steps fireball one cell north (mapY = startY - 1)
PASS INV_GV_343 projectile travel: second cross-cell step reaches mapY = startY - 2
PASS INV_GV_344 projectile travel: runtime-only projectile is reflected in viewport cell summary
PASS INV_GV_345 projectile detonation: fireball eventually impacts and is removed from world.projectiles
PASS INV_GV_346 projectile detonation: magical impact spawns an explosion into world.explosions
PASS INV_GV_347 projectile detonation: explosion is fireball type and appears in viewport cell summary
PASS INV_GV_348 explosion aftermath: F0822 advance ran (frame incremented or one-shot despawned)
PASS INV_GV_349 persistent smoke: F0822 decays attack and increments currentFrame across ticks
# summary: 580/580 invariants passed
M11 game-view probe: # summary: 580/580 invariants passed
Artifact: <repo>/verification-m11/phase-a/phase_a_probe.log
Binary:   <repo>/firestaff
```
exit=0

### m11 game view probe
```sh
./run_firestaff_m11_game_view_probe.sh
```
```text
In file included from <repo>/probes/m11/firestaff_m11_game_view_probe.c:1:
<repo>/m11_game_view.h:135:43: warning: declaration does not declare anything
  135 |     enum { M11_TORCH_FUEL_CAPACITY = 256 };
      |                                           ^
<repo>/probes/m11/firestaff_m11_game_view_probe.c: In function ‘main’:
<repo>/probes/m11/firestaff_m11_game_view_probe.c:2508:49: warning: ‘%s’ directive output may be truncated writing up to 1023 bytes into a region of size 512 [-Wformat-truncation=]
 2508 |             snprintf(gfxPath, sizeof(gfxPath), "%s/GRAPHICS.DAT", dataDir);
      |                                                 ^~
In file included from /usr/include/stdio.h:980,
                 from <repo>/memory_tick_orchestrator_pc34_compat.h:65,
                 from <repo>/m11_game_view.h:5:
In function ‘snprintf’,
    inlined from ‘main’ at <repo>/probes/m11/firestaff_m11_game_view_probe.c:2508:13:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 14 and 1037 bytes into a destination of size 512
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
In file included from <repo>/m11_game_view.c:1:
<repo>/m11_game_view.h:135:43: warning: declaration does not declare anything
  135 |     enum { M11_TORCH_FUEL_CAPACITY = 256 };
      |                                           ^
<repo>/m11_game_view.c: In function ‘m11_draw_v1_message_area’:
<repo>/m11_game_view.c:18559:43: warning: ‘%s’ directive output may be truncated writing up to 79 bytes into a region of size 54 [-Wformat-truncation=]
18559 |         snprintf(clipped, maxChars + 1U, "%s", text);
      |                                           ^~
In file included from /usr/include/stdio.h:980,
                 from <repo>/memory_tick_orchestrator_pc34_compat.h:65,
                 from <repo>/m11_game_view.h:5:
In function ‘snprintf’,
    inlined from ‘m11_draw_v1_message_area’ at <repo>/m11_game_view.c:18559:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 80 bytes into a destination of size 54
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘m11_draw_wall_contents.isra’:
<repo>/m11_game_view.c:7840:55: warning: ‘%d’ directive output may be truncated writing between 1 and 10 bytes into a region of size 4 [-Wformat-truncation=]
 7840 |                 snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                       ^~
<repo>/m11_game_view.c:7840:54: note: directive argument in the range [2, 2147483647]
 7840 |                 snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                      ^~~~
In function ‘snprintf’,
    inlined from ‘m11_draw_wall_contents.isra’ at <repo>/m11_game_view.c:7840:17:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 2 and 11 bytes into a destination of size 4
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘m11_draw_side_feature.isra’:
<repo>/m11_game_view.c:11139:59: warning: ‘%d’ directive output may be truncated writing between 1 and 10 bytes into a region of size 4 [-Wformat-truncation=]
11139 |                     snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                           ^~
<repo>/m11_game_view.c:11139:58: note: directive argument in the range [2, 2147483647]
11139 |                     snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                          ^~~~
In function ‘snprintf’,
    inlined from ‘m11_draw_side_feature.isra’ at <repo>/m11_game_view.c:11139:21:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 2 and 11 bytes into a destination of size 4
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_QuickSave’:
<repo>/m11_game_view.c:4735:40: warning: ‘%s’ directive output may be truncated writing up to 511 bytes into a region of size between 95 and 104 [-Wformat-truncation=]
 4735 |              "F9 RESTORES TICK %u FROM %s",
      |                                        ^~
 4736 |              (unsigned int)state->world.gameTick,
 4737 |              path);
      |              ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_QuickSave’ at <repo>/m11_game_view.c:4734:5:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 25 and 545 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_QuickLoad’:
<repo>/m11_game_view.c:4824:47: warning: ‘%s’ directive output may be truncated writing up to 511 bytes into a region of size between 84 and 93 [-Wformat-truncation=]
 4824 |              "TICK %u HASH %08X RELOADED FROM %s",
      |                                               ^~
......
 4827 |              path);
      |              ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_QuickLoad’ at <repo>/m11_game_view.c:4823:5:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 36 and 556 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘m11_draw_utility_panel’:
<repo>/m11_game_view.c:16594:39: warning: ‘%s’ directive output may be truncated writing up to 63 bytes into a region of size 32 [-Wformat-truncation=]
16594 |         snprintf(line, sizeof(line), "%s",
      |                                       ^~
In function ‘snprintf’,
    inlined from ‘m11_draw_utility_panel’ at <repo>/m11_game_view.c:16594:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 64 bytes into a destination of size 32
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:18804:47: warning: ‘%s’ directive output may be truncated writing up to 95 bytes into a region of size 90 [-Wformat-truncation=]
18804 |         snprintf(line2, sizeof(line2), "HERE  %s", line);
      |                                               ^~   ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_Draw’ at <repo>/m11_game_view.c:18804:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 7 and 102 bytes into a destination of size 96
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:18811:47: warning: ‘%s’ directive output may be truncated writing up to 95 bytes into a region of size 90 [-Wformat-truncation=]
18811 |         snprintf(line2, sizeof(line2), "AHEAD %s", line);
      |                                               ^~   ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_Draw’ at <repo>/m11_game_view.c:18811:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 7 and 102 bytes into a destination of size 96
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:744:37: warning: ‘%s’ directive output may be truncated writing up to 127 bytes into a region of size 80 [-Wformat-truncation=]
  744 |         snprintf(line1, line1Size, "%s", text);
      |                                     ^~
In function ‘snprintf’,
    inlined from ‘m11_dialog_source_split_two_lines’ at <repo>/m11_game_view.c:744:9,
    inlined from ‘M11_GameView_Draw’ at <repo>/m11_game_view.c:19239:29:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 128 bytes into a destination of size 80
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:19124:33: warning: ‘skillY’ may be used uninitialized [-Wmaybe-uninitialized]
19124 |                                 m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
      |                                 ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
19125 |                                               skillX, skillY, skillLine, &skillStyle);
      |                                               ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c:19116:45: note: ‘skillY’ was declared here
19116 |                                 int skillX, skillY;
      |                                             ^~~~~~
<repo>/asset_status_m12.c: In function ‘M12_AssetStatus_Scan’:
<repo>/asset_status_m12.c:256:29: warning: ‘%s’ directive output may be truncated writing up to 575 bytes into a region of size 512 [-Wformat-truncation=]
  256 |     snprintf(out, outSize, "%s", value);
      |                             ^~
......
  312 |             m12_copy_string(matchedPath, M12_ASSET_DATA_DIR_CAPACITY, path);
      |                                                                       ~~~~
In file included from /usr/include/stdio.h:980,
                 from <repo>/asset_status_m12.c:5:
In function ‘snprintf’,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:256:5,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:249:13,
    inlined from ‘m12_try_match_version’ at <repo>/asset_status_m12.c:312:13,
    inlined from ‘m12_fill_game_versions’ at <repo>/asset_status_m12.c:341:17,
    inlined from ‘M12_AssetStatus_Scan’ at <repo>/asset_status_m12.c:379:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 576 bytes into a destination of size 512
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/asset_status_m12.c: In function ‘M12_AssetStatus_Scan’:
<repo>/asset_status_m12.c:256:29: warning: ‘%s’ directive output may be truncated writing up to 1535 bytes into a region of size 512 [-Wformat-truncation=]
  256 |     snprintf(out, outSize, "%s", value);
      |                             ^~
In function ‘snprintf’,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:256:5,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:249:13,
    inlined from ‘m12_fill_game_versions’ at <repo>/asset_status_m12.c:348:21,
    inlined from ‘M12_AssetStatus_Scan’ at <repo>/asset_status_m12.c:379:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 1536 bytes into a destination of size 512
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/menu_startup_m12.c: In function ‘m12_load_runtime_catalog’:
<repo>/menu_startup_m12.c:535:41: warning: ‘%s’ directive output may be truncated writing up to 255 bytes into a region of size 128 [-Wformat-truncation=]
  535 |     snprintf(out + len, outSize - len, "%s", value);
      |                                         ^~
......
  683 |                 m12_append_string(msgid, sizeof(msgid), parsed);
      |                                                         ~~~~~~
In file included from /usr/include/stdio.h:980,
                 from <repo>/menu_startup_m12.c:9:
In function ‘snprintf’,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:535:5,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:526:13,
    inlined from ‘m12_load_runtime_catalog’ at <repo>/menu_startup_m12.c:683:17:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 1 and 256 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/menu_startup_m12.c: In function ‘m12_load_runtime_catalog’:
<repo>/menu_startup_m12.c:535:41: warning: ‘%s’ directive output may be truncated writing up to 255 bytes into a region of size 128 [-Wformat-truncation=]
  535 |     snprintf(out + len, outSize - len, "%s", value);
      |                                         ^~
......
  695 |                     m12_append_string(msgid, sizeof(msgid), parsed);
      |                                                             ~~~~~~
In function ‘snprintf’,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:535:5,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:526:13,
    inlined from ‘m12_load_runtime_catalog’ at <repo>/menu_startup_m12.c:695:21:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 1 and 256 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
PASS INV_GV_01 launcher exposes DM1 as the default builtin launch source with verified assets
PASS INV_GV_02 launcher selection transitions into a real game view through the source hook
PASS INV_GV_302A V1 message log strips Firestaff tick-prefix chrome from boot event text
PASS INV_GV_302B normal V1 keeps source screen-clear gaps black while debug HUD retains slate chrome
PASS INV_GV_430 DM1 primary mouse table maps right-click C151 champion-0 status box to command C007 toggle inventory
PASS INV_GV_431 DM1 primary mouse table maps left-click C151 champion-0 name/hands to command C012 status-box click
PASS INV_GV_432 DM1 primary mouse table scans C187 bar-graph left-click before C151 and returns command C007
PASS INV_GV_433 DM1 mouse zone matching keeps source inclusive right/bottom edges for status boxes
PASS INV_GV_433A DM1 primary mouse table maps left-click C013 spell area to command C100 before falling through
PASS INV_GV_433B DM1 primary mouse table maps left-click C011 action area to command C111 before falling through
PASS INV_GV_434 DM1 secondary movement table maps left-click C007 viewport to command C080 click-in-dungeon-view
PASS INV_GV_435 DM1 secondary movement table maps right-click screen zone, including viewport, to command C083 toggle leader inventory
PASS INV_GV_435A DM1 secondary movement table maps left-click C068 turn-left arrow to command C001
PASS INV_GV_435B DM1 secondary movement table maps left-click C071 move-right arrow to command C004
PASS INV_GV_436 DM1 inventory slot zones C507..C536 route screen clicks through viewport-relative coordinates to commands C028..C057
PASS INV_GV_437 DM1 inventory table gives right-click screen-zone close inventory precedence over viewport-relative slot hits
PASS INV_GV_438 DM1 inventory source slot boxes C507..C536 use source C033 normal slot-box graphic
PASS INV_GV_400 M11 game view builds champion mirror catalog from DUNGEON.DAT at start
PASS INV_GV_401 M11 mirror catalog exposes display name by ordinal
PASS INV_GV_402 M11 mirror catalog exposes display title by ordinal
PASS INV_GV_403 M11 can recruit champion identity by mirror catalog display name
PASS INV_GV_404 M11 mirror ordinal recruit is idempotent for already-present champion
PASS INV_GV_405 M11 resolves the source mirror TextString in the front viewport cell
PASS INV_GV_406 M11 mirror click opens a source-backed resurrect/reincarnate candidate panel
PASS INV_GV_407 M11 mirror panel resurrect command recruits the selected champion
PASS INV_GV_408 viewport source zone C007 is locked to the DM1 224x136 rectangle at screen origin 0,33
PASS INV_GV_409 inventory source panel seam is C020 graphic in layout-696 C101 at 80,52,144x73
PASS INV_GV_410 inventory/action slot-box graphics are source C033/C034/C035 and overhang 16x16 icon cells as 18x18 boxes
PASS INV_GV_411 inventory object-icon atlas seam is 32 icons per GRAPHICS.DAT page, 16x16 source cells starting at graphic 42
PASS INV_GV_412 action-hand icons apply G0498 colour-12 cyan remap while inventory slot icons preserve source colour 12
PASS INV_GV_413 viewport content placement seams expose source C2500 object, C3200 creature, and C2900 projectile points inside C007
PASS INV_GV_414 viewport base uses source ceiling C079 224x39 then floor C078 224x97 inside the 224x136 aperture
PASS INV_GV_415 viewport source draw-order seam is pinned from base/pits/ornaments/walls through doors/buttons
PASS INV_GV_03 game view renders a non-empty dungeon-backed frame
PASS INV_GV_04 game view input turns the party through the real tick orchestrator
PASS INV_GV_05 turning changes the rendered pseudo-viewport frame, not just the inspector text
PASS INV_GV_06 movement changes the rendered pseudo-viewport with real world movement
PASS INV_GV_08 small minimap inset still renders in the corner
PASS INV_GV_09 synthetic viewport harness initialises a focused 3x3 sample state
PASS INV_GV_07 enter now inspects the front-cell target without spending a real tick
PASS INV_GV_07B tab cycles the active front champion and updates the in-view readout
PASS INV_GV_07C space turns front-cell creature contact into a real strike tick for the selected champion
PASS INV_GV_07D clicking the viewport inspects the live front-cell target without spending a tick
PASS INV_GV_07E clicking a champion slot directly arms that champion for the next action
PASS INV_GV_07F clicking the left viewport lane turns the party through the real tick path
PASS INV_GV_07G clicking the lower center viewport advances into a clear front cell without using the HUD arrows
PASS INV_GV_07H clicking the on-screen action button drives the real front-cell attack flow
PASS INV_GV_07I space toggles a closed front door into an animating step and updates the real dungeon square one state closer to OPEN
PASS INV_GV_07M door interaction maps tick emissions to the M11 audio marker pipeline
PASS INV_GV_07J idle cadence advances the real world clock without requiring a manual wait input
PASS INV_GV_07K A strafes relative to facing and moves into the left lane through the real tick path
PASS INV_GV_07L clicking the lower-right viewport lane performs a relative strafe instead of another turn
PASS INV_GV_10 synthetic feature cells add door, stair, and occupancy cues inside the viewport
PASS INV_GV_11 a side door accent stays visible without collapsing the forward corridor window
PASS INV_GV_12 viewport slice and minimap inset coexist in the same frame
PASS INV_GV_12B viewport item and effect cues appear when real thing chains include loot and projectiles
PASS INV_GV_12C runtime viewport rect API returns source DM1 viewport geometry
PASS INV_GV_12D runtime viewport crop is the deterministic 224x136 DM1 aperture inside 320x200
PASS INV_GV_12E two same-state draws produce identical bytes for the 224x136 viewport crop
PASS INV_GV_13 escape from the game view returns control to the launcher
PASS INV_GV_13B quicksave serialises the live dungeon-backed world to a recoverable slot
PASS INV_GV_13C quickload restores the exact live world snapshot after local state drift
PASS INV_GV_13D sidebar save button writes a live quicksave without leaving the viewport
PASS INV_GV_13E sidebar load button restores the last live quicksave in-place
PASS INV_GV_13F sidebar menu header returns control to the launcher
PASS INV_GV_14 sidebar HUD renders separate status and map framing beside the viewport
PASS INV_GV_15 top HUD renders a dedicated party/status strip instead of a single inspector blob
PASS INV_GV_15B party strip reflects source-colored champion bars when champion data exists
PASS INV_GV_15C V1 champion HUD does not draw an invented active-slot yellow rectangle
PASS INV_GV_15D V1 champion HUD leaves unrecruited party slots undrawn
PASS INV_GV_15E V1 champion HUD draws source ready/action hand slot zones inside the status box
PASS INV_GV_15E9 V1 champion HUD status box zones expose layout-696 C151..C154 ids and geometry
PASS INV_GV_15E6 V1 status hand slot zones expose layout-696 C207..C210 parents and C211..C218 child ids/geometry
PASS INV_GV_15V V1 status hand icon zones inset 16x16 object icons within hand slots
PASS INV_GV_15W V1 status hand slot-box zones expose 18x18 C033/C034/C035 overdraw at hand origins
PASS INV_GV_15X V1 status hand slot fallback renders the 18x18 C033 box extent, not the 16x16 parent zone
PASS INV_GV_15E7 V1 status bar graph zones expose layout-696 C187..C190 and C195..C206 ids plus geometry
PASS INV_GV_15E8 V1 status name text zones expose layout-696 C163..C166 ids and geometry
PASS INV_GV_15Y V1 dead status-box fallback preserves source 67x29 C008 extent
PASS INV_GV_15E2 V1 champion HUD name clear zones expose layout-696 C159..C162 ids and geometry
PASS INV_GV_15Z V1 champion HUD name clear uses source C01 gray before centered name text
PASS INV_GV_15AA V1 live status-box fill uses source C12 darkest-gray before overlays
PASS INV_GV_15E3 V1 champion HUD name colors follow F0292 leader yellow / non-leader gold
PASS INV_GV_15E4 V1 champion HUD renders source-colored names inside the compact status name zones
PASS INV_GV_15E5 V1 dead champion HUD prints source centered name in C13 lightest gray
PASS INV_GV_15E10 V1 champion HUD renders all four recruited champion status boxes with source zones, names, and bars
PASS INV_GV_15F V1 champion HUD ready/action hands use normal slot-box graphic when idle
PASS INV_GV_15G V1 champion HUD action hand switches to graphic 35 for the acting champion
PASS INV_GV_15H V1 champion HUD ready-hand wound selects graphic 34 only for ready hand
PASS INV_GV_15I V1 champion HUD action-hand wound selects graphic 34 when idle
PASS INV_GV_15J V1 champion HUD acting action hand overrides wound graphic with graphic 35
PASS INV_GV_15K V1 champion HUD empty normal hands use source icons 212/214
PASS INV_GV_15L V1 champion HUD ready-hand wound advances empty icon to 213 only
PASS INV_GV_15M V1 champion HUD action-hand wound advances empty icon to 215 only
PASS INV_GV_15N V1 champion HUD acting hand changes the box graphic but keeps the source wounded empty icon
PASS INV_GV_15O V1 champion HUD occupied wounded hand uses F0033 object icon instead of empty-hand icon
PASS INV_GV_15OA V1 leader-hand runtime does not synthesize G4055 from the active champion ready-hand slot
PASS INV_GV_15OB V1 leader-hand C017 resolver stays blank when no dedicated source mouse-hand object exists
PASS INV_GV_15OC V1 leader-hand runtime carries a dedicated G4055-equivalent object with source icon/name resolution
PASS INV_GV_15OD normal V1 draws the transient leader-hand object name into source C017
PASS INV_GV_15OE V1 leader-hand remove flow clears the G4055-equivalent object instead of reading champion equipment
PASS INV_GV_15Q V1 status box base graphic uses source dead box only for dead champions
PASS INV_GV_15P V1 status shield border graphic priority follows spell/fire/party source order
PASS INV_GV_15T V1 status shield border zone reuses C007 status box footprint
PASS INV_GV_15R V1 poisoned label zone centers C032 under C007 status box geometry
PASS INV_GV_15S V1 champion damage indicator zones expose C167-C170 ids and center C015 inside C007 geometry
PASS INV_GV_15U V1 champion damage number origin is centered over the C015 damage banner
PASS INV_GV_16 viewport framing uses layered face bands and bright dungeon edges
PASS INV_GV_17 front-cell focus adds a threat-colored viewport reticle plus contextual inspect readout
PASS INV_GV_18 near-lane scanner chips surface left, front, and right contact state inside the viewport
PASS INV_GV_19 post-tick feedback strip renders attack-colored event telemetry inside the HUD
PASS INV_GV_20 forward depth chips summarize the next three center-lane cells with live threat and traversal cues
PASS INV_GV_21 message log contains at least one event after boot and gameplay
Screenshot: <repo>/verification-m11/game-view/15_side_ornament_item_creature_count_fidelity.pgm
PASS INV_GV_22 R toggles rest mode on and reports it through last action
PASS INV_GV_22A resting mode uses the source PartyResting input list and suppresses movement ticks until wake-up
PASS INV_GV_22B R again wakes the party from rest mode
PASS INV_GV_23 X triggers stair descent or reports no stairs on the current cell
PASS INV_GV_24 message log count increases or stays stable after gameplay ticks
PASS INV_GV_25 survival drain reduces food over repeated idle ticks
PASS INV_GV_26 party death is detected when all champions reach 0 HP after a tick
PASS INV_GV_27 G picks up the first floor item into the active champion inventory
PASS INV_GV_28 P drops the last held item back to the current cell
PASS INV_GV_29 pickup on empty floor reports nothing to pick up without crashing
PASS INV_GV_30 drop with empty inventory reports nothing to drop without crashing
PASS INV_GV_31 HandleInput routes M12_MENU_INPUT_PICKUP_ITEM through the item pickup path
PASS INV_GV_32 HandleInput routes M12_MENU_INPUT_DROP_ITEM through the item drop path
PASS INV_GV_33 creature AI moves a skeleton toward the party within movement cadence
PASS INV_GV_34 creature on party square deals autonomous damage over time
PASS INV_GV_35 creature attack events appear in the message log
PASS INV_GV_36 creature within sight range moves toward party at its cadence
PASS INV_GV_37 dead creature group does not deal damage
PASS INV_GV_38 creature AI handles constrained movement without crashing
PASS INV_GV_38A focused viewport: D1C normal pit source blit changes the corridor frame
PASS INV_GV_38X focused viewport: D1C normal pit clips inside the DM1 viewport rectangle
PASS INV_GV_38B focused viewport: D1C invisible pit variant differs from normal pit
PASS INV_GV_38Y focused viewport: D1C invisible pit clips inside the DM1 viewport rectangle
PASS INV_GV_38C focused viewport: D1C stairs zone blit changes the corridor frame
PASS INV_GV_38Z focused viewport: D1C stairs clips inside the DM1 viewport rectangle
PASS INV_GV_38D focused viewport: D1C teleporter field zone blit changes the corridor frame
PASS INV_GV_38AA focused viewport: D1C teleporter field clips inside the DM1 viewport rectangle
PASS INV_GV_38L focused viewport: D1C Trolin creature sprite changes the corridor frame
PASS INV_GV_38AB focused viewport: D1C Trolin creature clips inside the DM1 viewport rectangle
PASS INV_GV_38R focused viewport: D1L side-cell Trolin creature differs from empty and center creature frames
PASS INV_GV_38S focused viewport: extreme C3200 side creature clips inside the DM1 viewport rectangle
PASS INV_GV_38M focused viewport: D1C fireball projectile sprite changes the corridor frame
PASS INV_GV_38T focused viewport: D1C fireball projectile clips inside the DM1 viewport rectangle
PASS INV_GV_38Q focused viewport: D1C lightning projectile differs from empty and fireball frames
PASS INV_GV_38U focused viewport: D1C lightning projectile clips inside the DM1 viewport rectangle
PASS INV_GV_38N focused viewport: D1C dagger object sprite changes the corridor frame
PASS INV_GV_38V focused viewport: D1C dagger object clips inside the DM1 viewport rectangle
PASS INV_GV_38O focused viewport: D1C object sprite with G0209 native-index gap changes the corridor frame
PASS INV_GV_38W focused viewport: D1C multi-object pile clips inside the DM1 viewport rectangle
PASS INV_GV_38P focused viewport: D1C multi-object pile differs from single-object frame
PASS INV_GV_38E focused viewport: all normal pit zone specs visibly change their corridor frames
PASS INV_GV_38F focused viewport: all invisible pit zone specs visibly change their corridor frames
PASS INV_GV_38G focused viewport: all stairs front/side zone specs visibly change their corridor frames
PASS INV_GV_38H focused viewport: all teleporter field zone specs visibly change their corridor frames
PASS INV_GV_38I focused viewport: all visibly drawable floor ornament positions change their corridor frames
PASS INV_GV_38J focused viewport: special footprints floor ornament family renders from pre-base graphics
PASS INV_GV_38K focused viewport: all source-bound wall ornament specs change their wall frames
PASS INV_GV_39 corridor square does not trigger pit or teleporter transition
PASS INV_GV_40 corridor at (1,1) does not trigger transition
PASS INV_GV_41 stepping on pit drops party to the level below
PASS INV_GV_42 pit fall deals damage to champion HP
PASS INV_GV_43 pit fall preserves X/Y coordinates on the lower level
PASS INV_GV_44 pit fall writes a log entry mentioning PIT or FELL
PASS INV_GV_45 teleporter transports party to target map and coordinates
PASS INV_GV_46 teleporter applies relative rotation to party direction
PASS INV_GV_47 audible teleporter writes a visible log entry
PASS INV_GV_48 corridor on map 1 does not chain further transitions
PASS INV_GV_49 opening spell panel sets spellPanelOpen flag
PASS INV_GV_50 first rune enters buffer with correct encoded value
PASS INV_GV_51 second rune advances row to 2
PASS INV_GV_52 clear resets rune count and row to zero
PASS INV_GV_53 close panel clears panel flag and buffer
PASS INV_GV_54 casting with fewer than 2 runes returns 0
PASS INV_GV_55 out-of-range symbol indices rejected
PASS INV_GV_56 four consecutive rune entries fill the buffer
PASS INV_GV_57 fifth rune entry rejected when buffer is full
PASS INV_GV_58 SPELL_RUNE_1 input opens panel and enters rune
PASS INV_GV_59 SPELL_CLEAR input closes panel
PASS INV_GV_60 rune encoding matches DM1 formula 0x60+6*row+col
PASS INV_GV_61 CMD_CAST_SPELL for Light spell emits EMIT_SPELL_EFFECT
PASS INV_GV_62 Light spell application increases magicalLightAmount
PASS INV_GV_63 Fireball spell emits EMIT_SPELL_EFFECT with PROJECTILE kind
PASS INV_GV_64 Party Shield spell increases partyShieldDefense
PASS INV_GV_65 invalid spell table index produces no SPELL_EFFECT emission
PASS INV_GV_66 stairs-down (bit 0 clear) descends from map 0 to map 1
PASS INV_GV_67 stairs-up (bit 0 set) ascends from map 1 back to map 0
PASS INV_GV_68 stairs-up on top level (map 0) leads nowhere, party stays
PASS INV_GV_69 stairs-up transition logs ASCENDED message
PASS INV_GV_70 GetSkillLevel returns non-negative for a present champion
PASS INV_GV_71 GetSkillLevel returns -1 for out-of-range champion
PASS INV_GV_72 EMIT_DAMAGE_DEALT awards combat XP to active champion via lifecycle
PASS INV_GV_73 EMIT_SPELL_EFFECT awards magic XP to casting champion via lifecycle
PASS INV_GV_74 potion spell awards priest XP, not wizard XP
PASS INV_GV_75 EMIT_KILL_NOTIFY awards kill XP to active champion
PASS INV_GV_76 EMIT_KILL_NOTIFY with unknown creature type still awards fallback XP
PASS INV_GV_77 UseItem with KU potion heals champion HP
PASS INV_GV_78 doNotDiscard potion converts to empty flask after use
PASS INV_GV_79 UseItem on empty flask returns 0
PASS INV_GV_80 UseItem with empty hands returns 0
PASS INV_GV_81 UseItem on a weapon returns 0 (not consumable)
PASS INV_GV_82 UseItem with DES potion (poison) reduces HP
PASS INV_GV_83 Non-doNotDiscard potion clears slot after use
PASS INV_GV_84 asset loader initializes from GRAPHICS.DAT in the game data directory
PASS INV_GV_85 asset loader enumerates more than 40 graphics from GRAPHICS.DAT
PASS INV_GV_86 wall set graphic 42 loads as 256x32 with valid pixel data
PASS INV_GV_87 wall texture contains at least 3 distinct palette colors
PASS INV_GV_88 floor tile graphic 76 loads as 32x32 with valid pixel data
PASS INV_GV_89 full-screen graphic 4 loads as 320x200
PASS INV_GV_90 QuerySize returns 256x32 for wall set graphic 42
PASS INV_GV_90B odd-width action/PASS area graphic 10 loads as 87x45 with visible palette data
PASS INV_GV_90C movement arrows graphic 13 loads as 87x45 with visible palette data
PASS INV_GV_91 blitting wall texture produces substantial non-zero pixel coverage
PASS INV_GV_92 asset-backed viewport uses at least 6 distinct palette colors
PASS INV_GV_93 repeated load of same graphic returns cached slot
PASS INV_GV_94 zero-sized placeholder graphic returns NULL from loader
PASS INV_GV_95 BlitScaled renders floor tile into 100x60 target rect
PASS INV_GV_96 BlitRegion extracts 64x16 sub-rect from wall texture
PASS INV_GV_97 viewport background graphic 0 loads as 224x136
PASS INV_GV_98 door frame graphic 73 loads as 78x74 (mid depth)
PASS INV_GV_99 door frame graphic 70 loads as 36x49 (far depth)
PASS INV_GV_100 door side graphic 86 loads as 32x123
PASS INV_GV_101 stair graphic 95 loads as 60x111
PASS INV_GV_102 creature sprite base 584/M618 loads as 112x84
PASS INV_GV_103 creature type 1 sprite 588 loads as 64x66
PASS INV_GV_104 floor panel graphic 78 loads as 224x97
PASS INV_GV_105 ceiling panel graphic 79 loads as 224x39
PASS INV_GV_106 near corridor band has more content than dimmed far band
PASS INV_GV_107 asset-backed game view uses at least 8 distinct palette entries
PASS INV_GV_108 asset-backed full frame uses 10+ distinct palette entries
PASS INV_GV_109 object sprite graphic 566 (M612 potion aspect) loads from GRAPHICS.DAT
PASS INV_GV_110 per-map wall set index is in valid range (0-15)
PASS INV_GV_110B source wall/stairs blits offset full wallset graphic range by current map wallSet
PASS INV_GV_111 per-map floor set index is in valid range
PASS INV_GV_112 object sprite graphic 583 (end of M612 family) loads from GRAPHICS.DAT
PASS INV_GV_113 object sprite graphic 500 (scroll aspect) loads from GRAPHICS.DAT
PASS INV_GV_114 wall ornament graphic 259/M615 is loadable from GRAPHICS.DAT
PASS INV_GV_114B object sprite uses G0209 firstNative gap: weapon subtype 43 -> aspect 65 -> graphic 565
PASS INV_GV_114C object source scale units match G2030 table
PASS INV_GV_114C2 object source scale-index selection follows F0115 front/back cells
PASS INV_GV_114C3 object placement binds C2500 layout-696 source zone samples
PASS INV_GV_114D object pile shift index pairs match G0217 samples
PASS INV_GV_114E object shift values match G0223 samples
PASS INV_GV_114E2 object aspect GraphicInfo and CoordinateSet samples match G0209
PASS INV_GV_114E3 object flip-on-right follows G0209 GraphicInfo and relative cell
PASS INV_GV_114F creature D3/D2 palette-change samples match G0221/G0222
PASS INV_GV_114F2 creature slot-9/slot-10 replacement colors match source samples
PASS INV_GV_115 viewport rendering differs when item is on floor vs absent
PASS INV_GV_116 light level is 0 when no light sources present
PASS INV_GV_117 magicalLightAmount raises computed light level
PASS INV_GV_118 light level clamps to 255
PASS INV_GV_119 viewport rendering differs between dark and bright light
PASS INV_GV_120 light indicator area has non-black content in utility panel
PASS INV_GV_121 dark scene has more black pixels in viewport than bright
PASS INV_GV_122 negative magicalLightAmount clamps to 0
PASS INV_GV_123 torch fuel is 0 before first update
PASS INV_GV_124 torch fuel initialized to INITIAL-1 after first update
PASS INV_GV_125 flamitt fuel initialized to FLAMITT_INITIAL-1 after first update
PASS INV_GV_126 full-fuel torch gives more light than half-fuel torch
PASS INV_GV_127 torch extinguished when fuel reaches 0
PASS INV_GV_128 GetTorchFuel returns 0 for out-of-range index
PASS INV_GV_129 unlit torch does not consume fuel
PASS INV_GV_130 light is 0 with extinguished torches and no magic
PASS INV_GV_131 initial animTick is 0
PASS INV_GV_132 initial damageFlashTimer is 0
PASS INV_GV_133 initial attackCueTimer is 0
PASS INV_GV_134 TickAnimation increments animTick
PASS INV_GV_135 NotifyDamageFlash sets damageFlashTimer
PASS INV_GV_136 NotifyDamageFlash sets attackCueTimer
PASS INV_GV_137 TickAnimation decrements damageFlashTimer
PASS INV_GV_138 TickAnimation decrements attackCueTimer
PASS INV_GV_139 damageFlashTimer stays at 0 when already 0
PASS INV_GV_140 CreatureAnimFrame returns 0 or 1
PASS INV_GV_141 CreatureAnimFrame cycles through both frames
PASS INV_GV_142 different creature types have different anim phase
PASS INV_GV_143 flash timers fully decay to 0
PASS INV_GV_144 damage flash renders red pixels on viewport border
PASS INV_GV_145 attack cue renders yellow slash marks
PASS INV_GV_146 NULL-state animation queries return 0 safely
PASS INV_GV_147 NULL-state attack cue creature type returns -1
PASS INV_GV_148 active attack cue reports correct creature type
PASS INV_GV_149 NotifyDamageFlash sets attack cue creature type
PASS INV_GV_150 side-cell creature drawing code path exercised safely
PASS INV_GV_151 BlitScaledMirror produces horizontally flipped output
PASS INV_GV_152 BlitScaledMirror skips transparent pixels
PASS INV_GV_153 V1 depth dimming encodes palette level in upper bits
PASS INV_GV_154 V1 depth dimming preserves original colour index
PASS INV_GV_155 M11_FB_ENCODE/DECODE round-trip for all index/level combos
PASS INV_GV_156 front-facing D1 GiantScorpion view selects native front bitmap (M618+0)
PASS INV_GV_157 side-facing D1 GiantScorpion falls back to front (no SIDE bit, no FLIP_NON_ATTACK)
PASS INV_GV_158 back-facing D2 GiantScorpion falls back to derived front D2 (no BACK bit)
PASS INV_GV_159 front-facing attack GiantScorpion falls back to front (no ATTACK bit, no FLIP_ATTACK)
PASS INV_GV_260 GiantScorpion GraphicInfo matches ReDMCSB CREATURE_INFO (0x0482)
PASS INV_GV_261 Trolin GraphicInfo matches ReDMCSB CREATURE_INFO (0x05B8)
PASS INV_GV_262 GiantScorpion has no side/back/attack bitmaps per MASK0x0008/0x0010/0x0020
PASS INV_GV_263 Trolin has side/back/attack bitmaps (no FLIP_NON_ATTACK) per 0x05B8
PASS INV_GV_264 side-facing D1 Trolin selects native side bitmap (584+43+1) with mirror for relFacing=1
PASS INV_GV_265 back-facing D2 Trolin selects derived back D2 bitmap (663+5=668) without mirror
PASS INV_GV_266 front-facing attack D1 Trolin selects native attack bitmap (584+43+3=630)
PASS INV_GV_267 PainRat GraphicInfo 0x04B4: no SIDE, has BACK, has ATTACK, has FLIP_NON_ATTACK
PASS INV_GV_268 side-facing D1 PainRat falls back to front (584+10) mirrored (FLIP_NON_ATTACK set)
PASS INV_GV_269 back-facing D1 PainRat selects native back bitmap (584+10+2=596)
PASS INV_GV_270 creature 0 (GiantScorpion) GI=0x0482: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=4 derivedCount=8
PASS INV_GV_271 creature 1 (SwampSlime) GI=0x0480: ADD=0 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=2 derivedCount=2
PASS INV_GV_272 creature 2 (Giggler) GI=0x4510: ADD=0 SPECIAL_D2=0 D2_FLIPPED=1 FLIP_DURING_ATTACK=1 maxH=0 maxV=1 nativeCount=2 derivedCount=4
PASS INV_GV_273 creature 3 (PainRat) GI=0x04B4: ADD=0 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=4 derivedCount=6
PASS INV_GV_274 creature 4 (Ruster) GI=0x0701: ADD=1 SPECIAL_D2=0 D2_FLIPPED=1 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=2 derivedCount=5
PASS INV_GV_275 creature 5 (Screamer) GI=0x0581: ADD=1 SPECIAL_D2=1 D2_FLIPPED=1 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=2 derivedCount=5
PASS INV_GV_276 creature 6 (Rockpile) GI=0x070C: ADD=0 SPECIAL_D2=0 D2_FLIPPED=1 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=2 derivedCount=4
PASS INV_GV_277 creature 7 (GhostRive) GI=0x0300: ADD=0 SPECIAL_D2=0 D2_FLIPPED=1 FLIP_DURING_ATTACK=0 maxH=0 maxV=0 nativeCount=1 derivedCount=2
PASS INV_GV_278 creature 8 (WaterElemental) GI=0x5864: ADD=0 SPECIAL_D2=0 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=1 maxV=1 nativeCount=2 derivedCount=4
PASS INV_GV_279 creature 9 (Couatl) GI=0x0282: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=0 maxV=0 nativeCount=4 derivedCount=8
PASS INV_GV_280 creature 10 (StoneGolem) GI=0x1480: ADD=0 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=1 maxH=1 maxV=0 nativeCount=2 derivedCount=2
PASS INV_GV_281 creature 11 (Mummy) GI=0x18C6: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=1 maxV=0 nativeCount=2 derivedCount=8
PASS INV_GV_282 creature 12 (Skeleton) GI=0x1280: ADD=0 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=1 maxV=0 nativeCount=2 derivedCount=2
PASS INV_GV_283 creature 13 (MagentaWorm) GI=0x14A2: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=1 maxH=1 maxV=0 nativeCount=5 derivedCount=10
PASS INV_GV_284 creature 14 (Trolin) GI=0x05B8: ADD=0 SPECIAL_D2=1 D2_FLIPPED=1 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=4 derivedCount=8
PASS INV_GV_285 creature 15 (GiantWasp) GI=0x0381: ADD=1 SPECIAL_D2=1 D2_FLIPPED=1 FLIP_DURING_ATTACK=0 maxH=0 maxV=0 nativeCount=2 derivedCount=5
PASS INV_GV_286 creature 16 (Antman) GI=0x0680: ADD=0 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=2 derivedCount=2
PASS INV_GV_287 creature 17 (Vexirk) GI=0x04A0: ADD=0 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=3 derivedCount=4
PASS INV_GV_288 creature 18 (AnimatedArmour) GI=0x0280: ADD=0 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=0 maxV=0 nativeCount=2 derivedCount=2
PASS INV_GV_289 creature 19 (Materializer) GI=0x4060: ADD=0 SPECIAL_D2=0 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=0 maxV=1 nativeCount=2 derivedCount=4
PASS INV_GV_290 creature 20 (RedDragon) GI=0x10DE: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=1 maxV=0 nativeCount=4 derivedCount=12
PASS INV_GV_291 creature 21 (Oitu) GI=0x0082: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=0 maxV=0 nativeCount=4 derivedCount=8
PASS INV_GV_292 creature 22 (Demon) GI=0x1480: ADD=0 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=1 maxH=1 maxV=0 nativeCount=2 derivedCount=2
PASS INV_GV_293 creature 23 (LordChaos) GI=0x78AA: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=3 maxV=1 nativeCount=6 derivedCount=12
PASS INV_GV_294 creature 24 (LordOrder) GI=0x068A: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=1 maxH=0 maxV=0 nativeCount=5 derivedCount=10
PASS INV_GV_295 creature 25 (GreyLord) GI=0x78AA: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=3 maxV=1 nativeCount=6 derivedCount=12
PASS INV_GV_296 creature 26 (LordChaosRedDragon) GI=0x78AA: ADD=2 SPECIAL_D2=1 D2_FLIPPED=0 FLIP_DURING_ATTACK=0 maxH=3 maxV=1 nativeCount=6 derivedCount=12
PASS INV_GV_297 out-of-range creature type returns 0 for all source-backed GraphicInfo queries
PASS INV_GV_298 every creature has ≥ 1 native + ≥ 2 derived slots (source-backed cumulative totals)
PASS INV_GV_299 native-bitmap count matches independent F097_xxxx_DUNGEONVIEW_LoadGraphics recomputation for all 27 creatures
PASS INV_GV_156 Font init produces unloaded state
PASS INV_GV_157 Original DM1 font loads from GRAPHICS.DAT
PASS INV_GV_158 Font DrawChar 'A' produces visible pixels
PASS INV_GV_159 Font DrawString 'HELLO' renders visible text
PASS INV_GV_160 Font MeasureString('ABC') == 18 pixels
PASS INV_GV_161 Game view Draw uses original DM1 font when available
PASS INV_GV_162 IsGameWon returns 0 for fresh game view
PASS INV_GV_163 GetGameWonTick returns 0 for fresh game view
PASS INV_GV_164 gameWon flag and tick are queryable
PASS INV_GV_165 Endgame victory overlay renders differently from normal
PASS INV_GV_165B V1 endgame overlay keeps invented tick/help text out of default source path
PASS INV_GV_165C V1 endgame uses source C006 The End graphic
PASS INV_GV_165D V1 endgame uses source champion mirror graphic zone
PASS INV_GV_165H V1 endgame blits source champion portrait into C416
PASS INV_GV_165E V1 endgame prints champion name at source coordinate
PASS INV_GV_165I V1 endgame prints raw source champion title after name
PASS INV_GV_165L V1 endgame does not invent champion title from name alone
PASS INV_GV_165K V1 endgame prefers raw Champion.Title bytes when present
PASS INV_GV_165M V1 endgame title x spacing honors source punctuation rule
PASS INV_GV_165F V1 endgame prints source fighter skill-title line
PASS INV_GV_165G V1 endgame prints source ninja/priest/wizard skill-title lines
PASS INV_GV_165J V1 endgame skill levels ignore temporary XP
PASS INV_GV_166 HandleInput ignores movement when game is won
PASS INV_GV_167 HandleInput accepts ESC to return to menu when game won
PASS INV_GV_168 AdvanceIdleTick blocked when game is won
PASS INV_GV_169 IsDialogOverlayActive returns 0 for fresh game view
PASS INV_GV_170 ShowDialogOverlay activates overlay
PASS INV_GV_171 DismissDialogOverlay clears overlay
PASS INV_GV_172 Dialog overlay renders differently from normal
PASS INV_GV_172B V1 dialog overlay keeps placeholder title/footer debug-only
PASS INV_GV_173 HandleInput dismisses dialog overlay on keypress
PASS INV_GV_172C V1 dialog overlay blits source C000 dialog-box backdrop
PASS INV_GV_172D V1 dialog overlay prints source C450 version-zone text
PASS INV_GV_172E V1 dialog message text is centered in source viewport region
PASS INV_GV_172F V1 single-choice dialog message uses reconstructed C469 vertical zone
PASS INV_GV_172G V1 long dialog message uses source-width two-line split
PASS INV_GV_172H V1 source dialog renders bottom C462 choice text zone
PASS INV_GV_172I V1 two-choice dialog uses source C471/C463/C462 zones
PASS INV_GV_172J V1 three-choice dialog uses source C463/C466/C467 zones
PASS INV_GV_172K V1 four-choice dialog uses source C464-C467 zones
PASS INV_GV_172L V1 dialog accept selects first source choice
PASS INV_GV_172M V1 dialog mouse hit selects source choice zone
PASS INV_GV_172N V1 single-choice dialog applies source M621/C451 patch
PASS INV_GV_172O V1 two/four-choice dialogs apply source M622/M623 patches
PASS INV_GV_174 AdvanceIdleTick blocked during dialog overlay
PASS INV_GV_175 EMIT_GAME_WON emission sets gameWon flag
PASS INV_GV_176 EMIT_PARTY_DEAD emission sets partyDead flag
PASS INV_GV_177 Dialog/endgame query APIs are NULL-safe
PASS INV_GV_178 Map overlay is initially inactive
PASS INV_GV_179 ToggleMapOverlay activates map
PASS INV_GV_180 ToggleMapOverlay twice deactivates map
PASS INV_GV_181 MAP_TOGGLE input activates debug map overlay
PASS INV_GV_181B default V1 parity input ignores invented map overlay
PASS INV_GV_182 Movement blocked while map overlay active
PASS INV_GV_183 ESC closes map overlay
PASS INV_GV_184 Map overlay renders differently from normal view
PASS INV_GV_185 AdvanceIdleTick blocked during map overlay
PASS INV_GV_186 MAP_TOGGLE while map active deactivates it
PASS INV_GV_187 Inventory panel is initially inactive
PASS INV_GV_188 ToggleInventoryPanel activates inventory
PASS INV_GV_189 ToggleInventoryPanel twice deactivates inventory
PASS INV_GV_190 INVENTORY_TOGGLE input activates inventory panel
PASS INV_GV_191 Inventory panel renders differently from normal view
PASS INV_GV_359 normal V1 inventory only changes source viewport replacement rectangle
PASS INV_GV_360 normal V1 inventory ready-hand icon is confined to source C507 slot box
PASS INV_GV_361 normal V1 inventory head icon is confined to source C509 slot box
PASS INV_GV_362 V1 inventory champion slots map to source slot-box indices C513..C527 without alias leakage
PASS INV_GV_362A normal V1 inventory slot click picks object into dedicated leader-hand runtime state
PASS INV_GV_362B V1 source backpack slots C528..C536 route but do not alias compact Firestaff backpack slots
PASS INV_GV_363 normal V1 inventory pouch/quiver/backpack icons are confined to source C513/C514/C527 boxes
PASS INV_GV_192 Movement blocked while inventory panel active
PASS INV_GV_193 ESC closes inventory panel
PASS INV_GV_194 AdvanceIdleTick blocked during inventory panel
PASS INV_GV_195 Selected slot starts at 0 when inventory opens
PASS INV_GV_196 DOWN input in inventory advances selected slot
PASS INV_GV_197 MAP_TOGGLE closes inventory and opens debug map
PASS INV_GV_198 INVENTORY_TOGGLE closes map and opens inventory
PASS INV_GV_199 SlotName returns non-NULL for all named slots
PASS INV_GV_200 Map/inventory query APIs are NULL-safe
PASS INV_GV_353 V1 inventory equipment slot zones expose layout-696 C507..C519 ids and geometry
PASS INV_GV_354 V1 inventory backpack grid exposes layout-696 C520..C536 ids and geometry
PASS INV_GV_355 V1 inventory source-zone helpers reject invalid ordinals
PASS INV_GV_356 V1 inventory source slot-box helper preserves DEFS.H indices 8..37
PASS INV_GV_357 V1 inventory backdrop exposes source C017 in viewport replacement zone
PASS INV_GV_358 inventory backdrop C017 loads as 224x136 from GRAPHICS.DAT
PASS INV_GV_201 slot box normal (graphic 33) loads as 18x18 from GRAPHICS.DAT
PASS INV_GV_202 slot box wounded (graphic 34) loads as 18x18 from GRAPHICS.DAT
PASS INV_GV_203 slot box acting-hand (graphic 35) loads as 18x18 from GRAPHICS.DAT
PASS INV_GV_204 panel empty (graphic 20) loads as 144x73 from GRAPHICS.DAT
PASS INV_GV_205 status box frame (graphic 7) loads as 67x29 from GRAPHICS.DAT
PASS INV_GV_206 status box dead (graphic 8) loads as 67x29 from GRAPHICS.DAT
PASS INV_GV_207 champion portraits (graphic 26) loads as 256x87 from GRAPHICS.DAT
PASS INV_GV_208 food label (graphic 30) loads as 34x9 from GRAPHICS.DAT
PASS INV_GV_209 water label (graphic 31) loads as 46x9 from GRAPHICS.DAT
PASS INV_GV_210 poisoned label (graphic 32) loads as 96x15 from GRAPHICS.DAT
PASS INV_GV_211 party shield border (graphic 37) loads as 67x29 from GRAPHICS.DAT
PASS INV_GV_212 fire shield border (graphic 38) loads as 67x29 from GRAPHICS.DAT
PASS INV_GV_213 spell shield border (graphic 39) loads as 67x29 from GRAPHICS.DAT
PASS INV_GV_214 shield border drawn when partyShieldDefense > 0
PASS INV_GV_215 POISONED label drawn when champion poisonDose > 0
PASS INV_GV_216 damage to champion small (graphic 15) loads as 45x7 from GRAPHICS.DAT
PASS INV_GV_217 damage to champion big (graphic 16) loads as 32x29 from GRAPHICS.DAT
PASS INV_GV_218 damage to creature (graphic 14) loads as 88x45 from GRAPHICS.DAT
PASS INV_GV_219 per-champion damage indicator drawn when timer > 0
Screenshot: <repo>/verification-m11/game-view/inventory_slotbox_gfx.pgm
Screenshot: <repo>/verification-m11/game-view/party_hud_statusbox_gfx.pgm
Screenshot: <repo>/verification-m11/game-view/party_hud_statusbox_gfx_vga.ppm
Screenshot: <repo>/verification-m11/game-view/party_hud_four_champions_vga.ppm
Screenshot: <repo>/verification-m11/game-view/map_overlay_p5.pgm
Screenshot: <repo>/verification-m11/game-view/shield_poison_hud.pgm
PASS INV_GV_220 NotifyCreatureHit sets overlay timer and damage amount
PASS INV_GV_221 creature-hit overlay timer reaches 0 after sufficient ticks
PASS INV_GV_222 graphic-14 creature-hit overlay changes viewport pixels
PASS INV_GV_223 graphic-16 inventory damage overlay draws when active champion hit
PASS INV_GV_224 creature anim frames valid for types 0 and 1
PASS INV_GV_225 projectile sprite (graphic 454/M613) loads as 14x11 from GRAPHICS.DAT
PASS INV_GV_226 all 32 projectile sprites (454-485/M613 family) load from GRAPHICS.DAT
PASS INV_GV_227 projectile sprite (graphic 479) loads as 84x18 from GRAPHICS.DAT
PASS INV_GV_228 projectile sprite (graphic 480) loads as 8x14 from GRAPHICS.DAT
PASS INV_GV_229 side-cell creature perspective scaling: Giggler large sprite loadable
PASS INV_GV_230 floor item scatter: different types/subtypes produce different positions
PASS INV_GV_231 explosion type classification: fire/poison/lightning ranges are non-overlapping
PASS INV_GV_232 multi-creature stacking: single creature group produces visible viewport pixels
PASS INV_GV_233 multi-item floor scatter: floor area has visible content when items present
PASS INV_GV_234 wall ornament depth scaling: scale factors decrease monotonically with depth
Screenshot: <repo>/verification-m11/game-view/combat_damage_overlays.pgm
PASS INV_GV_235 door ornament depth scaling: side-pane scale factors decrease monotonically
PASS INV_GV_236 viewport area has visible content after draw (items/creatures/ornaments)
PASS INV_GV_237 creature group count+1 is at least 1 for first group
PASS INV_GV_238 side-pane wall ornament graphic base index is valid
PASS INV_GV_239 side-pane projectile gfx index range covers arrow-type projectile
PASS INV_GV_240 creature duplication count matches group size for 3-creature group
PASS INV_GV_241 creature duplication clamps correctly for 7-creature group
PASS INV_GV_242 projectile relative direction computation is correct for all quadrants
PASS INV_GV_243 projectile sprite mirroring triggers only for relDir=1 (right-bound)
PASS INV_GV_244 side-pane creature attack pose activates at depth 0 for both sides
PASS INV_GV_245 projectile sprite transparency key is palette index 10 (C10_COLOR_FLESH)
PASS INV_GV_245B projectile source scale units match G0215 for D1/D2/D3 front/back cells
PASS INV_GV_245C projectile aspect firstNative/GraphicInfo samples match G0210
PASS INV_GV_245D projectile G0210 aspect bitmap delta handles lightning rotation and fireball no-rotation
PASS INV_GV_245E projectile C0 back/rotation aspect applies horizontal+vertical flip flags while C3 fireball stays unflipped
PASS INV_GV_245F projectile placement binds C2900 layout-696 source zone samples
PASS INV_GV_246 projectile sub-cell rotation produces correct relative cells
PASS INV_GV_247 Z-order: floor items drawn before creatures (structural)
PASS INV_GV_248 floor ornament index cache stores values (not skipped)
Screenshot: <repo>/verification-m11/game-view/16_projectile_facing_creature_attack_ornament.pgm
PASS INV_GV_250 creature type 0 (GiantScorpion) coordSet=1 transparent=13
PASS INV_GV_251 creature type 6 (Rockpile) coordSet=0 transparent=13
PASS INV_GV_252 creature type 20 (RedDragon) coordSet=1 transparent=4
PASS INV_GV_253 out-of-range creature type returns coordSet=0 transparent=0
PASS INV_GV_254 coord set 1 single creature uses original D1 c10 center/bottom
PASS INV_GV_255 coord set 1 D2 pair uses original c6/c7 positions
PASS INV_GV_256 coord set 0 single creature uses original D3 c5 center/bottom
PASS INV_GV_256B creature placement binds C3200 layout-696 source zone samples
PASS INV_GV_256C creature draw path prefers C3200 over older G0224 midpoint for single front slot
PASS INV_GV_256D side-cell creature placement binds C3200 left/right source zone samples
PASS INV_GV_257 floor ornament ordinal query returns >= 0 for front cell
Screenshot: <repo>/verification-m11/game-view/17_floor_ornament_creature_aspect.pgm
PASS INV_GV_15AF V1 champion icons select C028 strip cells via M026 direction-relative source index
PASS INV_GV_15AG V1 champion HUD renders source-colored C113/C114 icon cells and leaves absent icon slots black
PASS INV_GV_350 normal V1 top chrome outside source status boxes/icons contains no title/debug text pixels
PASS INV_GV_351 normal V1 viewport floor/ceiling obeys DM1 parity flip
PASS INV_GV_300D action-hand icon cell zones expose layout-696 C088..C096 ids and geometry
PASS INV_GV_300K V1 object icon source zones resolve 16x16 cells across F0042+ graphics
PASS INV_GV_300L V1 action object icon palette remaps color-12 nybbles to cyan only in action cells
PASS INV_GV_300N V1 action icon cell backdrop color is cyan for living, black for dead, absent ignored
PASS INV_GV_300M V1 action icon hatch gate follows resting/candidate global source disable states
PASS INV_GV_300AL movement arrow panel exposes DATA.C outer box, C009/C013, and layout-696 C068-C073 geometry
PASS INV_GV_300AN screen and centered-dialog zones expose layout-696 C002/C005 geometry
PASS INV_GV_300AO explosion pattern and viewport-centered text zones expose layout-696 C004/C006 geometry
PASS INV_GV_300AM message area zone exposes layout-696 C014/C015 bottom-anchored geometry
PASS INV_GV_300AM2 V1 message area renders player-facing rows in source C015 and suppresses telemetry
PASS INV_GV_300AJ viewport zone exposes layout-696 C007 id and DM1 PC geometry
PASS INV_GV_300AH leader hand object-name zone exposes layout-696 C017 id and geometry
PASS INV_GV_300H action area zone exposes source C011/COMMAND.C right-column geometry
PASS INV_GV_300I spell area graphic anchors at ReDMCSB C013 right-column source position
PASS INV_GV_300AC spell caster panel zones expose layout-696 C221/C224 ids at ReDMCSB C013 position
PASS INV_GV_300AE action result zone exposes layout-696 C075 id and action-area geometry
PASS INV_GV_300AD action PASS zone exposes ReDMCSB COMMAND.C C112 right-aligned geometry
PASS INV_GV_300AF action menu graphic zones route C079/C077/C011 to source-sized rectangles at COMMAND.C position
PASS INV_GV_300AI inventory panel and food/water zones expose layout-696 C101/C103/C104 geometry
PASS INV_GV_300P right-column V1 action graphic uses source C010 with C079/C077/C011 menu zones
PASS INV_GV_300R V1 action+spell strip union covers source C013/C011 right-column stack
PASS INV_GV_300S V1 champion identity graphics use source C026 portraits and C028 icons
PASS INV_GV_300AR V1 champion icon invisibility palette mirrors source G2362 remap bytes
PASS INV_GV_300AK champion icon zones expose layout-696 C113-C116 ids and clipped geometry
PASS INV_GV_300T V1 HUD condition/damage graphics use source C032/C015/C016/C014 ids
PASS INV_GV_300U V1 inventory panel status uses source C020 panel and C030/C031 food-water labels
PASS INV_GV_300V V1 endgame graphics use source C006 The End and C346 champion mirror ids
PASS INV_GV_300AP V1 endgame zones expose source C412-C419, title, text, skill and restart/quit geometry
PASS INV_GV_300W V1 status slot and shield frame graphics use source C007/C008/C033-C035/C037-C039 ids
PASS INV_GV_300Y V1 champion bar colors use source G0046 order with C12 blank bars
PASS INV_GV_300X V1 dialog backdrop/version/choice patches use source C000/C450/M621-M623 geometry
PASS INV_GV_300Z V1 dialog message zones and vertical origins use source C469/C471 geometry
PASS INV_GV_300AA V1 dialog choice text zone ids expose source C462-C467 layout cases
PASS INV_GV_300AB V1 dialog pointer hit zones expose source C456-C461 button zones
PASS INV_GV_300AG spell symbol zones expose layout-696 C245-C260, C261-C264, C252 and C254 ids
PASS INV_GV_300Q V1 spell label cells use source C011 lines graphic rows for available/selected states
PASS INV_GV_300AQ normal V1 spell panel stays in C013 right-column area and uses selected C011 cells, not the old modal viewport panel
PASS INV_GV_300G action menu header zone exposes F0387 source zone 80 geometry
PASS INV_GV_300F action menu row zones expose COMMAND.C C113-C115 / F0387 zones 85-87 geometry
PASS INV_GV_300J action menu text origins match ACTIDRAW.C F0387 PC coordinates
PASS INV_GV_300O action menu colors match F0387 cyan header/black name and black rows/cyan actions
PASS INV_GV_300E action-hand icon pointer hit uses source C092 rightmost cell geometry
# DM-action-icon-cells probe: assetsAvailable=1 cyanCellsDrawn=2
PASS INV_GV_300 action-hand icon cells: both living champions get cyan backdrop (or no assets)
PASS INV_GV_300A action icon mode fills the source action area top band black before drawing cells
PASS INV_GV_300B action-hand icon cells: empty living hand blits source empty-hand icon
PASS INV_GV_300C action-hand icon cells hatch living cells during rest/candidate lockout
PASS INV_GV_301 action-hand icon cells: dead champion cell is solid black
PASS INV_GV_302 action-hand icon cells: absent champion slot does not receive a full cyan overlay
PASS INV_GV_303 action-hand icon cells: rightmost cell ends at x=318 (in-bounds)
PASS INV_GV_304 action-hand icon cells: ActionSetIndex==0 item (Compass) leaves inner cell fully cyan
PASS INV_GV_305 action-hand icon cells: ActionSetIndex>0 item blits source object icon
PASS INV_GV_306 action-hand icon cells: source object icon applies G0498 color-12-to-cyan palette change
PASS INV_GV_307 action-hand icon cells: lit torch uses source charge-count icon variant
PASS INV_GV_308 action-hand icon cells: charged weapon uses source +1 icon variant
PASS INV_GV_309 object icon resolver follows source scroll, compass, and charged-junk variants
PASS INV_GV_309B inventory slot icons use source object icons without action palette remap
Screenshot: <repo>/verification-m11/game-view/18_dm_action_hand_icon_cells.pgm
PASS INV_GV_310 action-menu: empty-hand champion activates with ActionSet 2
PASS INV_GV_311 action-menu: acting ordinal stored as DM1 1-based value
PASS INV_GV_312 action-menu: empty-hand ActionSet yields PUNCH/KICK/WAR CRY indices
PASS INV_GV_313 action-menu: header band is cyan when acting champion set
PASS INV_GV_314 action-menu: inner icon-cell cyan fill is suppressed vs icon-mode
PASS INV_GV_315 action-menu: ClearActingChampion returns to idle mode
PASS INV_GV_316 action-menu: ready-hand-only object does not override empty action hand
PASS INV_GV_320 action-menu: invalid row index leaves acting champion set
PASS INV_GV_321 action-menu: PUNCH row click performs action, clears menu, logs message
PASS INV_GV_322 action-menu: PUNCH row click advances (or holds) game tick via CMD_ATTACK
PASS INV_GV_323 action-menu: WAR CRY row click clears menu and logs without committing a strike tick
PASS INV_GV_324 action-menu: TriggerActionRow in idle mode is a no-op
PASS INV_GV_325 action-menu: HandlePointer row-hit closes menu and redraws
Screenshot: <repo>/verification-m11/game-view/19_dm_action_menu_mode.pgm
Screenshot: <repo>/verification-m11/game-view/20_dm_action_menu_post_click.pgm
PASS INV_GV_326 non-melee action: WAR CRY emits an audio marker
PASS INV_GV_327 non-melee action: WAR CRY advances a time-passes tick
PASS INV_GV_328 non-melee action: acting champion becomes party leader
PASS INV_GV_329 non-melee action: action name 36 is HEAL per G0490 table
PASS INV_GV_330 non-melee action: action name 38 is LIGHT per G0490 table
PASS INV_GV_331 non-melee action: action name 11 is FREEZE LIFE per G0490 table
PASS INV_GV_332 projectile action: rows 20/23/21 are FIREBALL/LIGHTNING/DISPELL per G0490 table
PASS INV_GV_333 projectile action: rows 27/32/42 are INVOKE/SHOOT/THROW per G0490 table
PASS INV_GV_334 projectile action: FIREBALL spawns subtype 0x80
PASS INV_GV_335 projectile action: LIGHTNING spawns subtype 0x82
PASS INV_GV_336 projectile action: DISPELL spawns subtype 0x83
PASS INV_GV_337 projectile action: INVOKE spawns one of the 4 F0407 C027 subtypes
PASS INV_GV_338 projectile action: SHOOT with empty ready hand emits NO AMMUNITION and spawns nothing
PASS INV_GV_339 projectile action: SHOOT with ready-hand ammo spawns kinetic projectile
PASS INV_GV_340 projectile action: THROW with item in action hand spawns kinetic projectile
PASS INV_GV_341 projectile travel: FIREBALL spawns at party cell (startY)
PASS INV_GV_342 projectile travel: first advance steps fireball one cell north (mapY = startY - 1)
PASS INV_GV_343 projectile travel: second cross-cell step reaches mapY = startY - 2
PASS INV_GV_344 projectile travel: runtime-only projectile is reflected in viewport cell summary
PASS INV_GV_345 projectile detonation: fireball eventually impacts and is removed from world.projectiles
PASS INV_GV_346 projectile detonation: magical impact spawns an explosion into world.explosions
PASS INV_GV_347 projectile detonation: explosion is fireball type and appears in viewport cell summary
PASS INV_GV_348 explosion aftermath: F0822 advance ran (frame incremented or one-shot despawned)
PASS INV_GV_349 persistent smoke: F0822 decays attack and increments currentFrame across ticks
# summary: 580/580 invariants passed
M11 game-view probe: # summary: 580/580 invariants passed
```
exit=0

### m11 ingame capture smoke
```sh
./run_firestaff_m11_ingame_capture_smoke.sh
```
```text
In-game capture smoke PASS: 6 screenshots
```
exit=0

### m11 launcher smoke
```sh
./run_firestaff_m11_launcher_smoke.sh
```
```text
<repo>/firestaff_m11_phase_a_probe.c: In function ‘main’:
<repo>/firestaff_m11_phase_a_probe.c:49:27: warning: implicit declaration of function ‘setenv’; did you mean ‘getenv’? [-Wimplicit-function-declaration]
   49 | # define m11_setenv(k, v) setenv((k), (v), 0)
      |                           ^~~~~~
<repo>/firestaff_m11_phase_a_probe.c:72:5: note: in expansion of macro ‘m11_setenv’
   72 |     m11_setenv("SDL_VIDEODRIVER", "dummy");
      |     ^~~~~~~~~~
In file included from <repo>/main_loop_m11.c:14:
<repo>/m11_game_view.h:135:43: warning: declaration does not declare anything
  135 |     enum { M11_TORCH_FUEL_CAPACITY = 256 };
      |                                           ^
In file included from <repo>/m11_game_view.c:1:
<repo>/m11_game_view.h:135:43: warning: declaration does not declare anything
  135 |     enum { M11_TORCH_FUEL_CAPACITY = 256 };
      |                                           ^
<repo>/m11_game_view.c: In function ‘m11_draw_v1_message_area’:
<repo>/m11_game_view.c:18559:43: warning: ‘%s’ directive output may be truncated writing up to 79 bytes into a region of size 54 [-Wformat-truncation=]
18559 |         snprintf(clipped, maxChars + 1U, "%s", text);
      |                                           ^~
In file included from /usr/include/stdio.h:980,
                 from <repo>/memory_tick_orchestrator_pc34_compat.h:65,
                 from <repo>/m11_game_view.h:5:
In function ‘snprintf’,
    inlined from ‘m11_draw_v1_message_area’ at <repo>/m11_game_view.c:18559:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 80 bytes into a destination of size 54
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘m11_draw_wall_contents.isra’:
<repo>/m11_game_view.c:7840:55: warning: ‘%d’ directive output may be truncated writing between 1 and 10 bytes into a region of size 4 [-Wformat-truncation=]
 7840 |                 snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                       ^~
<repo>/m11_game_view.c:7840:54: note: directive argument in the range [2, 2147483647]
 7840 |                 snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                      ^~~~
In function ‘snprintf’,
    inlined from ‘m11_draw_wall_contents.isra’ at <repo>/m11_game_view.c:7840:17:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 2 and 11 bytes into a destination of size 4
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘m11_draw_side_feature.isra’:
<repo>/m11_game_view.c:11139:59: warning: ‘%d’ directive output may be truncated writing between 1 and 10 bytes into a region of size 4 [-Wformat-truncation=]
11139 |                     snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                           ^~
<repo>/m11_game_view.c:11139:58: note: directive argument in the range [2, 2147483647]
11139 |                     snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                          ^~~~
In function ‘snprintf’,
    inlined from ‘m11_draw_side_feature.isra’ at <repo>/m11_game_view.c:11139:21:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 2 and 11 bytes into a destination of size 4
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_QuickSave’:
<repo>/m11_game_view.c:4735:40: warning: ‘%s’ directive output may be truncated writing up to 511 bytes into a region of size between 95 and 104 [-Wformat-truncation=]
 4735 |              "F9 RESTORES TICK %u FROM %s",
      |                                        ^~
 4736 |              (unsigned int)state->world.gameTick,
 4737 |              path);
      |              ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_QuickSave’ at <repo>/m11_game_view.c:4734:5:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 25 and 545 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_QuickLoad’:
<repo>/m11_game_view.c:4824:47: warning: ‘%s’ directive output may be truncated writing up to 511 bytes into a region of size between 84 and 93 [-Wformat-truncation=]
 4824 |              "TICK %u HASH %08X RELOADED FROM %s",
      |                                               ^~
......
 4827 |              path);
      |              ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_QuickLoad’ at <repo>/m11_game_view.c:4823:5:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 36 and 556 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘m11_draw_utility_panel’:
<repo>/m11_game_view.c:16594:39: warning: ‘%s’ directive output may be truncated writing up to 63 bytes into a region of size 32 [-Wformat-truncation=]
16594 |         snprintf(line, sizeof(line), "%s",
      |                                       ^~
In function ‘snprintf’,
    inlined from ‘m11_draw_utility_panel’ at <repo>/m11_game_view.c:16594:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 64 bytes into a destination of size 32
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:18804:47: warning: ‘%s’ directive output may be truncated writing up to 95 bytes into a region of size 90 [-Wformat-truncation=]
18804 |         snprintf(line2, sizeof(line2), "HERE  %s", line);
      |                                               ^~   ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_Draw’ at <repo>/m11_game_view.c:18804:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 7 and 102 bytes into a destination of size 96
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:18811:47: warning: ‘%s’ directive output may be truncated writing up to 95 bytes into a region of size 90 [-Wformat-truncation=]
18811 |         snprintf(line2, sizeof(line2), "AHEAD %s", line);
      |                                               ^~   ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_Draw’ at <repo>/m11_game_view.c:18811:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 7 and 102 bytes into a destination of size 96
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:744:37: warning: ‘%s’ directive output may be truncated writing up to 127 bytes into a region of size 80 [-Wformat-truncation=]
  744 |         snprintf(line1, line1Size, "%s", text);
      |                                     ^~
In function ‘snprintf’,
    inlined from ‘m11_dialog_source_split_two_lines’ at <repo>/m11_game_view.c:744:9,
    inlined from ‘M11_GameView_Draw’ at <repo>/m11_game_view.c:19239:29:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 128 bytes into a destination of size 80
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:19124:33: warning: ‘skillY’ may be used uninitialized [-Wmaybe-uninitialized]
19124 |                                 m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
      |                                 ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
19125 |                                               skillX, skillY, skillLine, &skillStyle);
      |                                               ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c:19116:45: note: ‘skillY’ was declared here
19116 |                                 int skillX, skillY;
      |                                             ^~~~~~
<repo>/asset_status_m12.c: In function ‘M12_AssetStatus_Scan’:
<repo>/asset_status_m12.c:256:29: warning: ‘%s’ directive output may be truncated writing up to 575 bytes into a region of size 512 [-Wformat-truncation=]
  256 |     snprintf(out, outSize, "%s", value);
      |                             ^~
......
  312 |             m12_copy_string(matchedPath, M12_ASSET_DATA_DIR_CAPACITY, path);
      |                                                                       ~~~~
In file included from /usr/include/stdio.h:980,
                 from <repo>/asset_status_m12.c:5:
In function ‘snprintf’,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:256:5,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:249:13,
    inlined from ‘m12_try_match_version’ at <repo>/asset_status_m12.c:312:13,
    inlined from ‘m12_fill_game_versions’ at <repo>/asset_status_m12.c:341:17,
    inlined from ‘M12_AssetStatus_Scan’ at <repo>/asset_status_m12.c:379:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 576 bytes into a destination of size 512
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/asset_status_m12.c: In function ‘M12_AssetStatus_Scan’:
<repo>/asset_status_m12.c:256:29: warning: ‘%s’ directive output may be truncated writing up to 1535 bytes into a region of size 512 [-Wformat-truncation=]
  256 |     snprintf(out, outSize, "%s", value);
      |                             ^~
In function ‘snprintf’,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:256:5,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:249:13,
    inlined from ‘m12_fill_game_versions’ at <repo>/asset_status_m12.c:348:21,
    inlined from ‘M12_AssetStatus_Scan’ at <repo>/asset_status_m12.c:379:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 1536 bytes into a destination of size 512
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/menu_startup_m12.c: In function ‘m12_load_runtime_catalog’:
<repo>/menu_startup_m12.c:535:41: warning: ‘%s’ directive output may be truncated writing up to 255 bytes into a region of size 128 [-Wformat-truncation=]
  535 |     snprintf(out + len, outSize - len, "%s", value);
      |                                         ^~
......
  683 |                 m12_append_string(msgid, sizeof(msgid), parsed);
      |                                                         ~~~~~~
In file included from /usr/include/stdio.h:980,
                 from <repo>/menu_startup_m12.c:9:
In function ‘snprintf’,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:535:5,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:526:13,
    inlined from ‘m12_load_runtime_catalog’ at <repo>/menu_startup_m12.c:683:17:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 1 and 256 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/menu_startup_m12.c: In function ‘m12_load_runtime_catalog’:
<repo>/menu_startup_m12.c:535:41: warning: ‘%s’ directive output may be truncated writing up to 255 bytes into a region of size 128 [-Wformat-truncation=]
  535 |     snprintf(out + len, outSize - len, "%s", value);
      |                                         ^~
......
  695 |                     m12_append_string(msgid, sizeof(msgid), parsed);
      |                                                             ~~~~~~
In function ‘snprintf’,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:535:5,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:526:13,
    inlined from ‘m12_load_runtime_catalog’ at <repo>/menu_startup_m12.c:695:21:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 1 and 256 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/menu_startup_render_modern_m12.c: In function ‘draw_card’:
<repo>/menu_startup_render_modern_m12.c:879:9: warning: this ‘if’ clause does not guard... [-Wmisleading-indentation]
  879 |         if (li < 0) li = 0; if (li > 3) li = 3;
      |         ^~
<repo>/menu_startup_render_modern_m12.c:879:29: note: ...this statement, but the latter is misleadingly indented as if it were guarded by the ‘if’
  879 |         if (li < 0) li = 0; if (li > 3) li = 3;
      |                             ^~
<repo>/menu_startup_render_modern_m12.c:880:9: warning: this ‘if’ clause does not guard... [-Wmisleading-indentation]
  880 |         if (gi < 0) gi = 0; if (gi > 2) gi = 2;
      |         ^~
<repo>/menu_startup_render_modern_m12.c:880:29: note: ...this statement, but the latter is misleadingly indented as if it were guarded by the ‘if’
  880 |         if (gi < 0) gi = 0; if (gi > 2) gi = 2;
      |                             ^~
<repo>/menu_startup_render_modern_m12.c:881:9: warning: this ‘if’ clause does not guard... [-Wmisleading-indentation]
  881 |         if (wi < 0) wi = 0; if (wi > 1) wi = 1;
      |         ^~
<repo>/menu_startup_render_modern_m12.c:881:29: note: ...this statement, but the latter is misleadingly indented as if it were guarded by the ‘if’
  881 |         if (wi < 0) wi = 0; if (wi > 1) wi = 1;
      |                             ^~
<repo>/menu_startup_render_modern_m12.c: In function ‘draw_settings_view’:
<repo>/menu_startup_render_modern_m12.c:1134:5: warning: this ‘if’ clause does not guard... [-Wmisleading-indentation]
 1134 |     if (li < 0) li = 0; if (li > 3) li = 3;
      |     ^~
<repo>/menu_startup_render_modern_m12.c:1134:25: note: ...this statement, but the latter is misleadingly indented as if it were guarded by the ‘if’
 1134 |     if (li < 0) li = 0; if (li > 3) li = 3;
      |                         ^~
<repo>/menu_startup_render_modern_m12.c:1135:5: warning: this ‘if’ clause does not guard... [-Wmisleading-indentation]
 1135 |     if (gi < 0) gi = 0; if (gi > 2) gi = 2;
      |     ^~
<repo>/menu_startup_render_modern_m12.c:1135:25: note: ...this statement, but the latter is misleadingly indented as if it were guarded by the ‘if’
 1135 |     if (gi < 0) gi = 0; if (gi > 2) gi = 2;
      |                         ^~
<repo>/menu_startup_render_modern_m12.c:1136:5: warning: this ‘if’ clause does not guard... [-Wmisleading-indentation]
 1136 |     if (wi < 0) wi = 0; if (wi > 1) wi = 1;
      |     ^~
<repo>/menu_startup_render_modern_m12.c:1136:25: note: ...this statement, but the latter is misleadingly indented as if it were guarded by the ‘if’
 1136 |     if (wi < 0) wi = 0; if (wi > 1) wi = 1;
      |                         ^~
In file included from <repo>/probes/m11/firestaff_m11_game_view_probe.c:1:
<repo>/m11_game_view.h:135:43: warning: declaration does not declare anything
  135 |     enum { M11_TORCH_FUEL_CAPACITY = 256 };
      |                                           ^
<repo>/probes/m11/firestaff_m11_game_view_probe.c: In function ‘main’:
<repo>/probes/m11/firestaff_m11_game_view_probe.c:2508:49: warning: ‘%s’ directive output may be truncated writing up to 1023 bytes into a region of size 512 [-Wformat-truncation=]
 2508 |             snprintf(gfxPath, sizeof(gfxPath), "%s/GRAPHICS.DAT", dataDir);
      |                                                 ^~
In file included from /usr/include/stdio.h:980,
                 from <repo>/memory_tick_orchestrator_pc34_compat.h:65,
                 from <repo>/m11_game_view.h:5:
In function ‘snprintf’,
    inlined from ‘main’ at <repo>/probes/m11/firestaff_m11_game_view_probe.c:2508:13:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 14 and 1037 bytes into a destination of size 512
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
In file included from <repo>/m11_game_view.c:1:
<repo>/m11_game_view.h:135:43: warning: declaration does not declare anything
  135 |     enum { M11_TORCH_FUEL_CAPACITY = 256 };
      |                                           ^
<repo>/m11_game_view.c: In function ‘m11_draw_v1_message_area’:
<repo>/m11_game_view.c:18559:43: warning: ‘%s’ directive output may be truncated writing up to 79 bytes into a region of size 54 [-Wformat-truncation=]
18559 |         snprintf(clipped, maxChars + 1U, "%s", text);
      |                                           ^~
In file included from /usr/include/stdio.h:980,
                 from <repo>/memory_tick_orchestrator_pc34_compat.h:65,
                 from <repo>/m11_game_view.h:5:
In function ‘snprintf’,
    inlined from ‘m11_draw_v1_message_area’ at <repo>/m11_game_view.c:18559:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 80 bytes into a destination of size 54
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘m11_draw_wall_contents.isra’:
<repo>/m11_game_view.c:7840:55: warning: ‘%d’ directive output may be truncated writing between 1 and 10 bytes into a region of size 4 [-Wformat-truncation=]
 7840 |                 snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                       ^~
<repo>/m11_game_view.c:7840:54: note: directive argument in the range [2, 2147483647]
 7840 |                 snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                      ^~~~
In function ‘snprintf’,
    inlined from ‘m11_draw_wall_contents.isra’ at <repo>/m11_game_view.c:7840:17:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 2 and 11 bytes into a destination of size 4
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘m11_draw_side_feature.isra’:
<repo>/m11_game_view.c:11139:59: warning: ‘%d’ directive output may be truncated writing between 1 and 10 bytes into a region of size 4 [-Wformat-truncation=]
11139 |                     snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                           ^~
<repo>/m11_game_view.c:11139:58: note: directive argument in the range [2, 2147483647]
11139 |                     snprintf(countStr, sizeof(countStr), "%d", countInGroup);
      |                                                          ^~~~
In function ‘snprintf’,
    inlined from ‘m11_draw_side_feature.isra’ at <repo>/m11_game_view.c:11139:21:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 2 and 11 bytes into a destination of size 4
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_QuickSave’:
<repo>/m11_game_view.c:4735:40: warning: ‘%s’ directive output may be truncated writing up to 511 bytes into a region of size between 95 and 104 [-Wformat-truncation=]
 4735 |              "F9 RESTORES TICK %u FROM %s",
      |                                        ^~
 4736 |              (unsigned int)state->world.gameTick,
 4737 |              path);
      |              ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_QuickSave’ at <repo>/m11_game_view.c:4734:5:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 25 and 545 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_QuickLoad’:
<repo>/m11_game_view.c:4824:47: warning: ‘%s’ directive output may be truncated writing up to 511 bytes into a region of size between 84 and 93 [-Wformat-truncation=]
 4824 |              "TICK %u HASH %08X RELOADED FROM %s",
      |                                               ^~
......
 4827 |              path);
      |              ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_QuickLoad’ at <repo>/m11_game_view.c:4823:5:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 36 and 556 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘m11_draw_utility_panel’:
<repo>/m11_game_view.c:16594:39: warning: ‘%s’ directive output may be truncated writing up to 63 bytes into a region of size 32 [-Wformat-truncation=]
16594 |         snprintf(line, sizeof(line), "%s",
      |                                       ^~
In function ‘snprintf’,
    inlined from ‘m11_draw_utility_panel’ at <repo>/m11_game_view.c:16594:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 64 bytes into a destination of size 32
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:18804:47: warning: ‘%s’ directive output may be truncated writing up to 95 bytes into a region of size 90 [-Wformat-truncation=]
18804 |         snprintf(line2, sizeof(line2), "HERE  %s", line);
      |                                               ^~   ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_Draw’ at <repo>/m11_game_view.c:18804:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 7 and 102 bytes into a destination of size 96
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:18811:47: warning: ‘%s’ directive output may be truncated writing up to 95 bytes into a region of size 90 [-Wformat-truncation=]
18811 |         snprintf(line2, sizeof(line2), "AHEAD %s", line);
      |                                               ^~   ~~~~
In function ‘snprintf’,
    inlined from ‘M11_GameView_Draw’ at <repo>/m11_game_view.c:18811:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 7 and 102 bytes into a destination of size 96
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:744:37: warning: ‘%s’ directive output may be truncated writing up to 127 bytes into a region of size 80 [-Wformat-truncation=]
  744 |         snprintf(line1, line1Size, "%s", text);
      |                                     ^~
In function ‘snprintf’,
    inlined from ‘m11_dialog_source_split_two_lines’ at <repo>/m11_game_view.c:744:9,
    inlined from ‘M11_GameView_Draw’ at <repo>/m11_game_view.c:19239:29:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 128 bytes into a destination of size 80
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c: In function ‘M11_GameView_Draw’:
<repo>/m11_game_view.c:19124:33: warning: ‘skillY’ may be used uninitialized [-Wmaybe-uninitialized]
19124 |                                 m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
      |                                 ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
19125 |                                               skillX, skillY, skillLine, &skillStyle);
      |                                               ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
<repo>/m11_game_view.c:19116:45: note: ‘skillY’ was declared here
19116 |                                 int skillX, skillY;
      |                                             ^~~~~~
<repo>/asset_status_m12.c: In function ‘M12_AssetStatus_Scan’:
<repo>/asset_status_m12.c:256:29: warning: ‘%s’ directive output may be truncated writing up to 575 bytes into a region of size 512 [-Wformat-truncation=]
  256 |     snprintf(out, outSize, "%s", value);
      |                             ^~
......
  312 |             m12_copy_string(matchedPath, M12_ASSET_DATA_DIR_CAPACITY, path);
      |                                                                       ~~~~
In file included from /usr/include/stdio.h:980,
                 from <repo>/asset_status_m12.c:5:
In function ‘snprintf’,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:256:5,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:249:13,
    inlined from ‘m12_try_match_version’ at <repo>/asset_status_m12.c:312:13,
    inlined from ‘m12_fill_game_versions’ at <repo>/asset_status_m12.c:341:17,
    inlined from ‘M12_AssetStatus_Scan’ at <repo>/asset_status_m12.c:379:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 576 bytes into a destination of size 512
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/asset_status_m12.c: In function ‘M12_AssetStatus_Scan’:
<repo>/asset_status_m12.c:256:29: warning: ‘%s’ directive output may be truncated writing up to 1535 bytes into a region of size 512 [-Wformat-truncation=]
  256 |     snprintf(out, outSize, "%s", value);
      |                             ^~
In function ‘snprintf’,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:256:5,
    inlined from ‘m12_copy_string’ at <repo>/asset_status_m12.c:249:13,
    inlined from ‘m12_fill_game_versions’ at <repo>/asset_status_m12.c:348:21,
    inlined from ‘M12_AssetStatus_Scan’ at <repo>/asset_status_m12.c:379:9:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin_snprintf’ output between 1 and 1536 bytes into a destination of size 512
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/menu_startup_m12.c: In function ‘m12_load_runtime_catalog’:
<repo>/menu_startup_m12.c:535:41: warning: ‘%s’ directive output may be truncated writing up to 255 bytes into a region of size 128 [-Wformat-truncation=]
  535 |     snprintf(out + len, outSize - len, "%s", value);
      |                                         ^~
......
  683 |                 m12_append_string(msgid, sizeof(msgid), parsed);
      |                                                         ~~~~~~
In file included from /usr/include/stdio.h:980,
                 from <repo>/menu_startup_m12.c:9:
In function ‘snprintf’,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:535:5,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:526:13,
    inlined from ‘m12_load_runtime_catalog’ at <repo>/menu_startup_m12.c:683:17:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 1 and 256 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
<repo>/menu_startup_m12.c: In function ‘m12_load_runtime_catalog’:
<repo>/menu_startup_m12.c:535:41: warning: ‘%s’ directive output may be truncated writing up to 255 bytes into a region of size 128 [-Wformat-truncation=]
  535 |     snprintf(out + len, outSize - len, "%s", value);
      |                                         ^~
......
  695 |                     m12_append_string(msgid, sizeof(msgid), parsed);
      |                                                             ~~~~~~
In function ‘snprintf’,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:535:5,
    inlined from ‘m12_append_string’ at <repo>/menu_startup_m12.c:526:13,
    inlined from ‘m12_load_runtime_catalog’ at <repo>/menu_startup_m12.c:695:21:
/usr/include/x86_64-linux-gnu/bits/stdio2.h:54:10: note: ‘__builtin___snprintf_chk’ output between 1 and 256 bytes into a destination of size 128
   54 |   return __builtin___snprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1,
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   55 |                                    __glibc_objsize (__s), __fmt,
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   56 |                                    __va_arg_pack ());
      |                                    ~~~~~~~~~~~~~~~~~
M11 launcher smoke: PASS
Log: <repo>/verification-m11/launcher-smoke/launcher_smoke.log
```
exit=0

### viewport state probe
```sh
./build/firestaff_m11_viewport_state_probe $FIRESTAFF_DATA verification-m11/n2_viewport_world_visuals_20260428_123237/viewport-state
```
```text
PASS source-driven viewport state probe
verification-m11/n2_viewport_world_visuals_20260428_123237/viewport-state/dm1_viewport_state_probe.md
verification-m11/n2_viewport_world_visuals_20260428_123237/viewport-state/dm1_viewport_state_probe.json
```
exit=0

### capture route state probe
```sh
./build/firestaff_m11_capture_route_state_probe $FIRESTAFF_DATA verification-m11/n2_viewport_world_visuals_20260428_123237/capture-route-state
```
```text
wrote verification-m11/n2_viewport_world_visuals_20260428_123237/capture-route-state/pass76_capture_route_state_probe.md and verification-m11/n2_viewport_world_visuals_20260428_123237/capture-route-state/pass76_capture_route_state_probe.json
PASS capture route state probe
```
exit=0

### pass83 viewport content points
```sh
python3 tools/pass83_firestaff_viewport_content_points_probe.py
```
```text
{
  "points": 271,
  "scenes": 6,
  "problems": [],
  "stats": "parity-evidence/overlays/pass83/pass83_firestaff_viewport_content_points_stats.json",
  "summary": "parity-evidence/overlays/pass83/pass83_firestaff_viewport_content_points_summary.md"
}
```
exit=0

### pass84 original overlay readiness
```sh
python3 tools/pass84_original_overlay_readiness_probe.py
```
```text
{
  "blockers": [
    "default pass74 compare inputs are missing Firestaff PPM full-frame captures",
    "default pass74 compare inputs are missing original raw 320x200 screenshots",
    "recorded pass74 stats reference artifacts absent from the fresh worktree",
    "pass78 route attempts remained text-mode/prompt captures, not gameplay",
    "original shot-label manifest is absent; semantic route checkpoints are not auditable"
  ],
  "champion_inventory_spell_route_readiness": {
    "explicit_champion_keys_supported_by_route_script": true,
    "required_scenes": [
      "01_ingame_start",
      "02_ingame_turn_right",
      "03_ingame_move_forward",
      "04_ingame_spell_panel",
      "05_ingame_after_cast",
      "06_ingame_inventory_panel"
    ],
    "route_shape_dry_run_supported": true,
    "semantically_locked_original_runtime_route": false,
    "serialized_original_frame_clicks_supported_by_route_script": true
  },
  "firestaff_png_reviews": [
    {
      "bytes": 9345,
      "exists": true,
      "path": "verification-screens/01_ingame_start_latest.png"
    },
    {
      "bytes": 8973,
      "exists": true,
      "path": "verification-screens/02_ingame_turn_right_latest.png"
    },
    {
      "bytes": 8126,
      "exists": true,
      "path": "verification-screens/03_ingame_move_forward_latest.png"
    },
    {
      "bytes": 8430,
      "exists": true,
      "path": "verification-screens/04_ingame_spell_panel_latest.png"
    },
    {
      "bytes": 8125,
      "exists": true,
      "path": "verification-screens/05_ingame_after_cast_latest.png"
    },
    {
      "bytes": 2483,
      "exists": true,
      "path": "verification-screens/06_ingame_inventory_panel_latest.png"
    }
  ],
  "firestaff_ppm_inputs": [
    {
      "bytes": 0,
      "exists": false,
      "path": "verification-screens/01_ingame_start_latest.ppm"
    },
    {
      "bytes": 0,
      "exists": false,
      "path": "verification-screens/02_ingame_turn_right_latest.ppm"
    },
    {
      "bytes": 0,
      "exists": false,
      "path": "verification-screens/03_ingame_move_forward_latest.ppm"
    },
    {
      "bytes": 0,
      "exists": false,
      "path": "verification-screens/04_ingame_spell_panel_latest.ppm"
    },
    {
      "bytes": 0,
      "exists": false,
      "path": "verification-screens/05_ingame_after_cast_latest.ppm"
    },
    {
      "bytes": 0,
      "exists": false,
      "path": "verification-screens/06_ingame_inventory_panel_latest.ppm"
    }
  ],
  "honesty": "Evidence/readiness only; no pixel-parity or semantic-route parity is claimed.",
  "original_raw_inputs": [
    {
      "bytes": 0,
      "exists": false,
      "path": "verification-screens/pass70-original-dm1-viewports/image0001-raw.png"
    },
    {
      "bytes": 0,
      "exists": false,
      "path": "verification-screens/pass70-original-dm1-viewports/image0002-raw.png"
    },
    {
      "bytes": 0,
      "exists": false,
      "path": "verification-screens/pass70-original-dm1-viewports/image0003-raw.png"
    },
    {
      "bytes": 0,
      "exists": false,
      "path": "verification-screens/pass70-original-dm1-viewports/image0004-raw.png"
    },
    {
      "bytes": 0,
      "exists": false,
      "path": "verification-screens/pass70-original-dm1-viewports/image0005-raw.png"
    },
    {
      "bytes": 0,
      "exists": false,
      "path": "verification-screens/pass70-original-dm1-viewports/image0006-raw.png"
    }
  ],
  "pass74_stats": {
    "exists": true,
    "missing_referenced_artifacts": [
      "parity-evidence/overlays/pass74/01_ingame_start_full_frame_mask.png",
      "parity-evidence/overlays/pass74/02_ingame_turn_right_full_frame_mask.png",
      "parity-evidence/overlays/pass74/03_ingame_move_forward_full_frame_mask.png",
      "parity-evidence/overlays/pass74/04_ingame_spell_panel_full_frame_mask.png",
      "parity-evidence/overlays/pass74/05_ingame_after_cast_full_frame_mask.png",
      "parity-evidence/overlays/pass74/06_ingame_inventory_panel_full_frame_mask.png",
      "verification-screens/01_ingame_start_latest.ppm",
      "verification-screens/02_ingame_turn_right_latest.ppm",
      "verification-screens/03_ingame_move_forward_latest.ppm",
      "verification-screens/04_ingame_spell_panel_latest.ppm",
      "verification-screens/05_ingame_after_cast_latest.ppm",
      "verification-screens/06_ingame_inventory_panel_latest.ppm",
      "verification-screens/pass70-original-dm1-viewports/image0001-raw.png",
      "verification-screens/pass70-original-dm1-viewports/image0002-raw.png",
      "verification-screens/pass70-original-dm1-viewports/image0003-raw.png",
      "verification-screens/pass70-original-dm1-viewports/image0004-raw.png",
      "verification-screens/pass70-original-dm1-viewports/image0005-raw.png",
      "verification-screens/pass70-original-dm1-viewports/image0006-raw.png"
    ],
    "recorded_pairs": 6,
    "recorded_pass": true,
    "recorded_problems": [],
    "schema": "pass74_fullscreen_panel_pair_compare.v1"
  },
  "pass78_route_attempts": [
    {
      "all_gameplay_320x200": false,
      "captures": [
        {
          "bytes": 1317,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-vga-exe/image0001-raw.png",
          "height": 400,
          "sha256": "d2038406f95df5e4557afb5724cab7f8d3911ad12b182c7135ed5e008b80bae3",
          "width": 720
        },
        {
          "bytes": 1330,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-vga-exe/image0002-raw.png",
          "height": 400,
          "sha256": "a976197474fada07c962c8db510bb354e0b468f3235ef9af2d54232f27de2601",
          "width": 720
        },
        {
          "bytes": 1317,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-vga-exe/image0003-raw.png",
          "height": 400,
          "sha256": "d2038406f95df5e4557afb5724cab7f8d3911ad12b182c7135ed5e008b80bae3",
          "width": 720
        },
        {
          "bytes": 1345,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-vga-exe/image0004-raw.png",
          "height": 400,
          "sha256": "7b28c9c1e6870e26184c139de3791d154f8a3d75bc61d73676edf381ea4a92e3",
          "width": 720
        },
        {
          "bytes": 1748,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-vga-exe/image0005-raw.png",
          "height": 400,
          "sha256": "0f5acc833d03b74e0d4ac7bdaf7619dc4f4d59205dff2c105f9d52448654a3aa",
          "width": 720
        },
        {
          "bytes": 1763,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-vga-exe/image0006-raw.png",
          "height": 400,
          "sha256": "e24942569d12f251e41c405d47aaef519992955e630363db098346a48ae46e60",
          "width": 720
        }
      ],
      "dimensions_seen": {
        "720x400": 6
      },
      "path": "parity-evidence/overlays/pass78/pass78_program_vga_exe_prompt_blocker.json"
    },
    {
      "all_gameplay_320x200": false,
      "captures": [
        {
          "bytes": 9942,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-vga/image0001-raw.png",
          "height": 400,
          "sha256": "7a85a07aeb1a87484c1cbdbc05eeed2230f0c97cf97e222d9d8ec5acd36df3cc",
          "width": 720
        },
        {
          "bytes": 9956,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-vga/image0002-raw.png",
          "height": 400,
          "sha256": "f0cf0c33e8a03aa2d7318a871501435a2367c95c57f5c8b674daab21ce4786ef",
          "width": 720
        },
        {
          "bytes": 9956,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-vga/image0003-raw.png",
          "height": 400,
          "sha256": "f0cf0c33e8a03aa2d7318a871501435a2367c95c57f5c8b674daab21ce4786ef",
          "width": 720
        },
        {
          "bytes": 9959,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-vga/image0004-raw.png",
          "height": 400,
          "sha256": "baf621d44e1d730b77f986bd9208598e73c476496bdd1a95d64b0d9b02c1ef00",
          "width": 720
        },
        {
          "bytes": 10371,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-vga/image0005-raw.png",
          "height": 400,
          "sha256": "182e5873052b365ce89806ada21ca74cb385d10174d820ebdf71608d1f4d71e8",
          "width": 720
        },
        {
          "bytes": 10386,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-vga/image0006-raw.png",
          "height": 400,
          "sha256": "85566be37f242e57a0c8c90919c10a40b797b26647efa8368850ba82e5977eaf",
          "width": 720
        }
      ],
      "dimensions_seen": {
        "720x400": 6
      },
      "path": "parity-evidence/overlays/pass78/pass78_program_vga_prompt_blocker.json"
    },
    {
      "all_gameplay_320x200": false,
      "captures": [
        {
          "bytes": 6457,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-target3/image0001-raw.png",
          "height": 400,
          "sha256": "de99ce0d86e2039756377ff09ac751858868f233f5881b852b078646e6fbdbcf",
          "width": 720
        },
        {
          "bytes": 6469,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-target3/image0002-raw.png",
          "height": 400,
          "sha256": "bb096ac28b9ad712ce8c6de59131f8f44a459fb94b888e70252ea43336268c3d",
          "width": 720
        },
        {
          "bytes": 6469,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-target3/image0003-raw.png",
          "height": 400,
          "sha256": "bb096ac28b9ad712ce8c6de59131f8f44a459fb94b888e70252ea43336268c3d",
          "width": 720
        },
        {
          "bytes": 6469,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-target3/image0004-raw.png",
          "height": 400,
          "sha256": "bb096ac28b9ad712ce8c6de59131f8f44a459fb94b888e70252ea43336268c3d",
          "width": 720
        },
        {
          "bytes": 6457,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-target3/image0005-raw.png",
          "height": 400,
          "sha256": "de99ce0d86e2039756377ff09ac751858868f233f5881b852b078646e6fbdbcf",
          "width": 720
        },
        {
          "bytes": 6486,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-target3/image0006-raw.png",
          "height": 400,
          "sha256": "cbbaac85ef502d90fb07030eca178d41b2234f046d741b6e0defa514d99ffde3",
          "width": 720
        }
      ],
      "dimensions_seen": {
        "720x400": 6
      },
      "path": "parity-evidence/overlays/pass78/pass78_selector_blocker_target3.json"
    },
    {
      "all_gameplay_320x200": false,
      "captures": [
        {
          "bytes": 6544,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-v-key/image0001-raw.png",
          "height": 400,
          "sha256": "78bdaa04e30f4bc5fdf6fa7bf597b5c6d1842e29981cd1edc4d970b62054bba5",
          "width": 720
        },
        {
          "bytes": 6535,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-v-key/image0002-raw.png",
          "height": 400,
          "sha256": "2d0fcbdfa0eab0b462dcbdb17988a2549a362b152ad8589e00a9436e9bc80cb7",
          "width": 720
        },
        {
          "bytes": 6536,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-v-key/image0003-raw.png",
          "height": 400,
          "sha256": "eee59fc433c0c6efc69eeec475ba92ba8dc246938cf6496297f13577c1de56f3",
          "width": 720
        },
        {
          "bytes": 6536,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-v-key/image0004-raw.png",
          "height": 400,
          "sha256": "eee59fc433c0c6efc69eeec475ba92ba8dc246938cf6496297f13577c1de56f3",
          "width": 720
        },
        {
          "bytes": 6523,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-v-key/image0005-raw.png",
          "height": 400,
          "sha256": "8fde0655a753254afa83229f56ff03c311d37b4d84d8d884fc063d5ba26225f1",
          "width": 720
        },
        {
          "bytes": 6536,
          "classification": "text_or_non_gameplay_blocker",
          "file": "verification-screens/pass78-original-route-v-key/image0006-raw.png",
          "height": 400,
          "sha256": "eee59fc433c0c6efc69eeec475ba92ba8dc246938cf6496297f13577c1de56f3",
          "width": 720
        }
      ],
      "dimensions_seen": {
        "720x400": 6
      },
      "path": "parity-evidence/overlays/pass78/pass78_selector_blocker_v_key.json"
    }
  ],
  "ready_for_overlay_comparison": false,
  "schema": "pass84_original_overlay_readiness_probe.v1",
  "shot_label_manifest": {
    "exists": false,
    "path": "verification-screens/pass70-original-dm1-viewports/original_viewport_shot_labels.tsv"
  }
}
```
exit=0

### greatstone source lock
```sh
python3 tools/greatstone_dm1_source_lock_check.py
```
```text
missing original source file: <repo>/verification-screens/dm1-dosbox-capture/DungeonMasterPC34/DATA/GRAPHICS.DAT
```
exit=1

### resolve viewport zone
```sh
python3 tools/resolve_dm1_zone.py viewport
```
```text
Traceback (most recent call last):
  File "<repo>/tools/resolve_dm1_zone.py", line 241, in <module>
    manifest = _json.load(open(ROOT / "extracted-graphics-v1/manifest.json"))["entries"]
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
FileNotFoundError: [Errno 2] No such file or directory: '<repo>/extracted-graphics-v1/manifest.json'
```
exit=1

## Viewport/world TODO scan
V1_BLOCKERS.md:114:    vertical/horizontal door on the party square, creature damage /
V1_BLOCKERS.md:115:    kills from a closing door on a creature square.  The Pass 38
V1_BLOCKERS.md:133:  stairs regression is prevented by an explicit creature-context
V1_BLOCKERS.md:143:    — DM1 PC 3.4 creatures never step onto stairs).
V1_BLOCKERS.md:144:  - `m11_game_view.c:m11_square_walkable_for_creature` is now a thin
V1_BLOCKERS.md:147:    (state 5) are now walkable for creatures, matching the shared
V1_BLOCKERS.md:154:    `run_firestaff_m11_pass39_creature_walkability_probe.sh` drives
V1_BLOCKERS.md:157:    context equals F0706 for every square, creature context equals
V1_BLOCKERS.md:158:    F0706 except stairs are blocked, creature context rejects both
V1_BLOCKERS.md:159:    stairs-up and stairs-down, creature context accepts corridor /
V1_BLOCKERS.md:161:    creature context rejects walls / closed door / animating door
V1_BLOCKERS.md:169:  - Thing-list occupancy checks for creature legality (another group
V1_BLOCKERS.md:171:    `m11_creature_try_move` — Pass 39 is strictly about square-element
V1_BLOCKERS.md:177:    in DM1 PC 3.4 — creatures are blocked outright, which is the
V1_BLOCKERS.md:190:  pass 42.  The DM1 viewport rectangle is now encoded as a named enum
V1_BLOCKERS.md:200:    `firestaff_m11_pass40_viewport_lock_probe.c` +
V1_BLOCKERS.md:201:    `run_firestaff_m11_pass40_viewport_lock_probe.sh` verify 22/22
V1_BLOCKERS.md:204:    overlap rectangles between the DM1 viewport and every
V1_BLOCKERS.md:209:  - Evidence: `parity-evidence/pass40_viewport_lock.md` (source
V1_BLOCKERS.md:223:  - At pass 40, the runtime viewport rectangle was still
V1_BLOCKERS.md:226:    to the source DM1 viewport `(0, 33, 224, 136)`, and phase 77/79
V1_BLOCKERS.md:239:  is in `parity-evidence/pass40_viewport_lock.md` §4.
V1_BLOCKERS.md:283:    remains `BLOCKED_ON_REFERENCE` (blocker §11 / pass 47b).
V1_BLOCKERS.md:284:  - No viewport, chrome, or bar-graph changes (blockers §4, §6, §7).
V1_BLOCKERS.md:305:    viewport rectangle recorded in pass 40.
V1_BLOCKERS.md:348:  - The runtime viewport rectangle (still blocked on pass 47b
V1_BLOCKERS.md:408:  - Font-bank wiring, palette, viewport, and party-panel origin are
V1_BLOCKERS.md:443:  - No M10 behavior, save-format, palette, viewport, or party-panel
V1_BLOCKERS.md:472:  - No palette work (blocker §10 / pass 46), viewport change, or
V1_BLOCKERS.md:512:  - No viewport/layout/pixel-overlay parity claim beyond the palette
V1_BLOCKERS.md:515:## 11. ReDMCSB pixel overlays missing for viewport and side panel
V1_BLOCKERS.md:518:  - Pass 33 §4 and Pass 34 §3 both have `BLOCKED_ON_REFERENCE` rows
V1_BLOCKERS.md:533:    (ZONES.H is not in the local ReDMCSB dump), (b) viewport dungeon
V1_BLOCKERS.md:536:  the DEFS.H-anchored viewport bounding rect can now be diffed
V1_BLOCKERS.md:537:  honestly.  Phase 70 also locks the source viewport base graphics
V1_BLOCKERS.md:539:  draw-order inputs (`INV_GV_414/415`) so the next visual patch can
V1_BLOCKERS.md:541:  asset seam.  Pass 70 follow-up captured six original DOS viewport
V1_BLOCKERS.md:551:  source-state anchor via `firestaff_m11_viewport_state_probe`: source
V1_BLOCKERS.md:552:  party state `(map=0,x=1,y=3,dir=SOUTH)`, a 3x3 relative viewport
V1_BLOCKERS.md:553:  neighborhood with thing-chain counts, and critical viewport asset
V1_BLOCKERS.md:558:  measurements for viewport/action/spell/inventory surfaces; high deltas
V1_BLOCKERS.md:567:  frames, not `320x200` gameplay frames, so no original viewport reference
V1_BLOCKERS.md:577:  DUNGEON.DAT/GRAPHICS.DAT viewport state plus `zones_h_reconstruction.json`;
V1_BLOCKERS.md:706:    converted event-index paths and locks the remaining direct-marker TODO
V1_BLOCKERS.md:948:  3. Resolve the four documented remaining direct-marker TODO buckets, either
V1_BLOCKERS.md:983:    through the `F0033_OBJECT_GetIconIndex` resolver instead of scaled viewport
V1_BLOCKERS.md:1023:  - Viewport content/draw-order parity remains separate from these right-panel
PARITY_MATRIX_DM1_V1.md:41:| `BLOCKED_ON_REFERENCE` | Cannot verify — missing original reference data or capture. |
PARITY_MATRIX_DM1_V1.md:51:| Viewport region bounds | DEFS.H: `C112_BYTE_WIDTH_VIEWPORT`=112 (224px at 4bpp), `C136_HEIGHT_VIEWPORT`=136. Graphic #0 (`C000_DERIVED_BITMAP_VIEWPORT`) is 224×136, confirming exact match. Viewport screen origin `G2067/G2068=(0, 33)` from COORD.C:81-82. Viewport uses color indices 16–31 via `G8177_c_ViewportColorIndexOffset=0x10`. Pass 71 source-lock check verifies local PC 3.4 `GRAPHICS.DAT`/`DUNGEON.DAT`/`SONG.DAT` hashes plus Greatstone/SCK-style geometry for 22 critical graphics, including viewport base assets `0078` floor `224×97`, `0079` ceiling `224×39`, front walls, doors, object icons, and projectile/effect sources. | `m11_game_view.c` runtime viewport enum now binds `M11_VIEWPORT_*` directly to `M11_DM1_VIEWPORT_*`: `(0, 33, 224, 136)`. Phase 76–79 migrated probe gates to this source rectangle and removed the old prototype `PROBE_VIEWPORT_*` constants. Phase 70 adds probe-visible base/draw-order seams: C079 ceiling `224×39`, C078 floor `224×97`, and the current source-backed viewport render pass order through pits/ornaments/walls/stairs/fields/doors/buttons (`INV_GV_414/415`). Pass 70 follow-up produced six original DOS viewport crops plus first diff stats, proving the comparison path works while showing semantic state mismatch remains. Pass 78 then hardens the original-route capture path and audits failed attempts, accepting none because they remain `720x400` selector/prompt captures rather than `320x200` gameplay frames. Pass 79 resolves that launcher blocker with source-backed `DM -vv -sn -pk` flags: title/menu and Enter-to-dungeon capture sets now audit as real `320x200` graphics frames. Pass 80 adds a semantic raw-frame classifier: the title set is `title_or_menu`, but the Enter/pass77 set repeats one `dungeon_gameplay` frame and fails expected spell-panel/inventory checkpoints, so semantic route parity is still not claimed. Pass 112 adds a stricter route-label + classifier join gate and an N2 xvfb/DOSBox route probe; it produced six raw `320x200` frames, but the measured classes were `graphics_320x200_unclassified`, three `title_or_menu`, and two `entrance_menu`, with duplicate raw hashes, so no overlay-ready route is promoted. Pass 113 adds a party-state follow-up probe: Greatstone DUNGEON XML anchors the source-plausible first-champion target at start `(3,2)` east -> Elija portrait wall `(10,2)` west side, but N2 direct-start mouse/keypad movement probes repeat dungeon frames with blank right-column controls; `tools/pass113_original_party_state_probe.py` marks this as `direct_start_no_party_signature=true`, not party-control-ready. Pass 72 adds `firestaff_m11_viewport_state_probe`, a deterministic DUNGEON.DAT/GRAPHICS.DAT source-state anchor for party `(map=0,x=1,y=3,dir=SOUTH)`, 3x3 viewport neighborhood thing-chain counts, and critical viewport asset dimensions. Pass 83 adds source-zone geometry SVG overlays for the `C2500` object, `C2900` projectile, and `C3200` creature anchor-point families across the six deterministic V1 screenshots. | `MATCHED` for rectangle/source-asset bounds — pass 40's old `KNOWN_DIFF` was superseded by the all-graphics viewport migration. Pixel/content parity remains `KNOWN_DIFF`: original media exists and a deterministic source start state is now locked, but the M11 renderer has not yet been overlaid from that state to a matching original capture. See `parity-evidence/dm1_all_graphics_phase76_probe_dm1_viewport_rect.md`, phase 77, phase 79, phase 80, phase 70 viewport source-base/draw-order evidence, `parity-evidence/pass70_original_dm1_viewport_crops_and_diff_results.md`, `parity-evidence/greatstone_dm1_source_lock_check.md`, and `parity-evidence/pass72_source_driven_viewport_state_probe.md`. | Continue viewport content parity source-first: render from the locked deterministic DUNGEON.DAT/GRAPHICS.DAT state and `zones_h_reconstruction.json`; original runtime capture should use the pass79 direct-flag handoff, then solve/prove the exact input route before treating pixel deltas as parity evidence. |
PARITY_MATRIX_DM1_V1.md:52:| Party/champion region | `DEFS.H` constants: portrait 32×29 (`G2078`/`G2079`), atlas addressing `M027_PORTRAIT_X`/`M028_PORTRAIT_Y`, champion status-box spacing 69 px (`C69_CHAMPION_STATUS_BOX_SPACING`), status-box frame graphic `C007_GRAPHIC_STATUS_BOX` 67×29, layout-696 status/name/bar zones (`C151..C154`, `C159..C166`, `C187..C206`). | Firestaff V1: `M11_PORTRAIT_W=32, M11_PORTRAIT_H=29` (matches); `M11_V1_PARTY_SLOT_STEP=69` + `M11_V1_PARTY_SLOT_W=67` (pass 41). Atlas indexing `(i & 7)*32, (i >> 3)*29` matches. Pass 81 locks the four recruited champion HUD fixture with source status-box/name/bar zones and slot-colored bars. Pass 96 refreshes the pass83 champion-HUD overlays to the active V1 source-origin top-row geometry (`M11_V1_PARTY_PANEL_X=0`; legacy `M11_PARTY_PANEL_X=12` is V2-only chrome). Pass 107 audits the `C150..C218` source chain against the pass83 overlay, and pass 110 independently resolves the `type=7` `C195..C206` HP/stamina/mana value records to 4×25 bottom-flush bar rectangles plus sample fill math. V2 vertical-slice mode still uses the legacy `M11_PARTY_SLOT_STEP=77` / `M11_PARTY_SLOT_W=71` via `m11_party_slot_step()` / `m11_party_slot_w()` to keep the pre-baked 302×28 four-slot HUD sprite aligned. | `MATCHED` (V1 stride + slot width + source-origin four-champion HUD fixture) — portrait identity, slot horizontal stride, status-box footprint, compact name zones, bar-zone colors, and the x=0 source-origin top row are source-anchored for the visible four-slot HUD. See `parity-evidence/pass41_status_box_stride.md`, `parity-evidence/overlays/pass41/`, `parity-evidence/pass81_champion_hud_four_slot_lock.md`, `parity-evidence/pass83_champion_hud_zone_overlay.md`, `parity-evidence/pass96_champion_hud_overlay_source_origin_refresh.md`, `parity-evidence/pass107_v1_hud_status_source_chain_audit.md`, and `parity-evidence/pass110_v1_hud_type7_bar_value_resolver.md`. Original runtime pixel overlay remains `BLOCKED_ON_REFERENCE`; pass 112 verifies that labeled N2 captures still fail the required semantic classes before the HUD route reaches overlay readiness. Do not treat these source/overlay audits as original-runtime parity claims. | Retire stride/HUD source-geometry line. Remaining sub-row: party-panel original-runtime overlay comparison remains `BLOCKED_ON_REFERENCE`, tracked through the original DOS reference route. |
PARITY_MATRIX_DM1_V1.md:56:| Dialog/endgame overlays | ReDMCSB `DIALOG.C:F0427_DIALOG_Draw` expands the original dialog-box graphic into the viewport, prints `V3.4` in `C450_ZONE_DIALOG_VERSION`, patches 1/2/4-choice layouts with dialog patch zones, centers up to two message strings, and uses source choice zones/colours. `ENDGAME.C:F0444_STARTEND_Endgame` has a separate source flow using `THE END`, champion mirror/portrait zones, champion text, and restart/quit controls. | Firestaff has compat stubs (`dialog_frontend_pc34_compat.*`, `endgame_frontend_pc34_compat.*`) and runtime flags. Passes 107–121 replaced the visible dialog placeholder path with source-backed V1 drawing: source C000 backdrop, C450 `V3.4`, C469/C471 message zones, source-width line splitting, C462–C467 choice text zones, choice hit flow, and M621/M622/M623 1/2/4-choice patch graphics. Passes 122–123, 127, and 2957–2976 replaced the default V1 game-won panel with source endgame graphics and helper-backed geometry: C006 `THE END`, C346 champion mirrors in C412–C415, C416–C419 portrait blits, restart/quit source boxes, champion names at x=87/y=14+48n, raw title text, and source skill-title rows. Remaining endgame composition gaps are timing/music/restart loop and original overlay comparison captures. | `KNOWN_DIFF` (narrowed) — dialog visual path is substantially source-backed; endgame visual composition is source-backed but not yet original-capture compared. | Capture original dialog/endgame frames for overlay comparison; source-bind endgame timing/music/restart loop if required for lock. |
PARITY_MATRIX_DM1_V1.md:70:| Panel backgrounds/ornaments | ReDMCSB/DUNVIEW.C ornament paths use per-map ornament index tables; local renderer resolves wall/floor/door ornament ordinals through the DUNGEON.DAT metadata cache. Known graphics ranges: wall ornaments start at `259` (`M615`), floor ornaments use regular sets from `247` plus special footprints `379..384`, and door ornaments resolve through per-map door ornament tables. | Firestaff parses per-map wall/floor/door ornament indices, renders floor ornaments below items/creatures/projectiles, renders wall/door ornaments on center and side panels, and has focused gates (`INV_GV_38I/J/K`, `INV_GV_114`, `INV_GV_234/235/238/248`) proving visible draw-paths, footprint special-case, depth scaling, side-pane path, and cache storage. | `KNOWN_DIFF` (narrowed) — ornament data paths and draw-paths are source-backed/probe-gated, but exact original panel placement/clipping/z-order still lacks screenshot overlay. | Capture original ornament-heavy views and overlay against focused Firestaff fixtures; then retire remaining placement/z-order diffs. |
PARITY_MATRIX_DM1_V1.md:84:| Creature palettes | 14 creature palettes (G8175_CREAT_PAL) extracted from `VIDEODRV.C` — 14 types × 6 replacement colors (indices 1–6) | In `recovered_palette.json` with full VGA6 data; the M11 viewport renderer now queries G0243 creature replacement-color set indices and remaps creature sprite palette slots 9/10 during compositing, with probe samples covering mixed slot-9-only, slot-10-only, both-slot, and no-replacement cases. | `MATCHED` for data and renderer slot-9/slot-10 remap seam; exact creature screenshot overlays remain covered by the viewport content pixel-parity blocker | Continue original screenshot overlays for creature-heavy views; renderer replacement-color seam is probe-backed by pass 102. |
PARITY_MATRIX_DM1_V1.md:87:| Falsecolor vs. true-color | Current `ppm-falsecolor/` exports are inspection artifacts, not claimed final RGB | M10 VGA palette export uses the pass-46 source-backed palette lookup and still produces valid 320×200 PPM output. | `MATCHED` (palette export seam) / `UNPROVEN` (full pixel-overlay color parity) | Re-export comparison artifacts after viewport/layout capture paths are ready. |
PARITY_MATRIX_DM1_V1.md:113:| Creature AI/movement | `DUNGEON.C` source available | `verification-m10/creature-ai/` suite exists | `UNPROVEN` | Add original-backed cases |
PARITY_MATRIX_DM1_V1.md:130:| Idle animation cadence | No original timing data captured | Unknown | `BLOCKED_ON_REFERENCE` | Capture original via emulator with frame timing |
PARITY_MATRIX_DM1_V1.md:131:| Attack cue duration | No original timing data captured | Unknown | `BLOCKED_ON_REFERENCE` | Capture via emulator |
PARITY_MATRIX_DM1_V1.md:132:| Damage flash duration | No original timing data captured | Unknown | `BLOCKED_ON_REFERENCE` | Capture via emulator |
PARITY_MATRIX_DM1_V1.md:133:| Message timing | No original timing data captured | Unknown | `BLOCKED_ON_REFERENCE` | Capture via emulator |
PARITY_MATRIX_DM1_V1.md:134:| Input responsiveness | No original timing data captured | Unknown | `BLOCKED_ON_REFERENCE` | Capture via emulator |
PARITY_MATRIX_DM1_V1.md:135:| Spell sequence pacing | No original timing data captured | Unknown | `BLOCKED_ON_REFERENCE` | Capture via emulator |
PARITY_MATRIX_DM1_V1.md:136:| Door open/close sequencing | No original timing data captured | Unknown | `BLOCKED_ON_REFERENCE` | Capture via emulator |
PARITY_MATRIX_DM1_V1.md:145:| Sound trigger points | ReDMCSB I34E sound-index namespace mapped to SND3 item indices in Pass 52 (`DEFS.H`, `DATA.C`, `PASS52_AUDIO_FINDINGS.md`); Pass 55 audits remaining direct M11 marker calls | Sound-event → SND3 mapping table landed (`sound_event_snd3_map_v1.[ch]`); pass 53 routes `EMIT_SOUND_REQUEST` payloads through mapped event-index playback when original SND3 assets are present; pass 55 converts source-backed action cues (`WAR CRY`, `BLOW HORN`, `SHOOT`, `THROW`) to mapped event-index playback and documents four remaining direct-marker TODO buckets; pass 125 preserves the source sound-event index across fallback/no-audio marker paths so headless ordering probes can distinguish source events from direct markers; runtime trigger cadence still not captured against original audio | `KNOWN_DIFF` (narrowed) | Capture cadence/overlap against original runtime and resolve the four documented direct-marker TODO buckets |
PARITY_MATRIX_DM1_V1.md:147:| Sound samples (content) — GRAPHICS.DAT SND3 SFX bank | 33 SND3 items (indices 671-675, 677-685, 687-693, 701-712) at 6000 Hz, per dmweb; Greatstone Sound 00..32 labels; real-file decode verified Pass 51 (`PASS51_AUDIO_FINDINGS.md`, `parity-evidence/pass51_v1_graphics_dat_snd3_probe.txt`); event mapping verified Pass 52 (`parity-evidence/pass52_v1_snd3_event_map_probe.txt`); runtime SND3 branch verified Pass 53 (`PASS53_AUDIO_FINDINGS.md`, `parity-evidence/pass53_v1_snd3_runtime_probe.txt`); direct-marker audit verified Pass 55 (`PASS55_AUDIO_FINDINGS.md`, `parity-evidence/pass55_m11_direct_audio_marker_audit.txt`) | Format + decoder landed (`graphics_dat_snd3_loader_v1.[ch]`); event mapping landed (`sound_event_snd3_map_v1.[ch]`); pass 53 loads all 35 mapped event-index buffers from original `GRAPHICS.DAT`, linearly resamples 6000 Hz unsigned PCM to the fixed 22050 Hz SDL float stream, queues mapped `EMIT_SOUND_REQUEST` buffers with procedural fallback preserved, and pass 55 converts four source-backed action cue sites to event-index playback; pass 125 locks fallback event-index observability for queue/order probes; probes PASS (6/6 + 5/5 + 5/5 + pass55 audit + pass125 4/4) | `KNOWN_DIFF` (narrowed) | Capture original cadence/overlap and resolve remaining direct-marker TODO buckets before claiming full SFX parity |
PARITY_MATRIX_DM1_V1.md:148:| Sound cadence/overlap | No original capture | Unknown | `BLOCKED_ON_REFERENCE` | Capture via emulator with audio |
PARITY_MATRIX_DM1_V1.md:180:| CSB original data acquired | `BLOCKED_ON_REFERENCE` | Must acquire before any CSB parity work |
PARITY_MATRIX_DM1_V1.md:182:| DM2 original data acquired | `BLOCKED_ON_REFERENCE` | Must acquire before any DM2 parity work |
PARITY_MATRIX_DM1_V1.md:192:| `MATCHED` | 13 — GRAPHICS.DAT decode coverage, placeholder cataloging, title-side UI asset mapping, Champion portraits (pass 34), Spell-panel asset identity (pass 34), M9 gate infra, M10 suite infra, M11 suite infra, Submenu matrix infra, Champion status-box stride + slot width V1 source-anchored (pass 41), viewport rectangle bounds (all-graphics phases 76–81), equipment/item icon resolver + palette split (passes 84–95), rune/C011 spell label cells (pass 96). |
PARITY_MATRIX_DM1_V1.md:193:| `KNOWN_DIFF` | 14 — prior palette + typography + Firestaff-invented UI chrome, the invented/convenience map overlay, placeholder dialog/endgame overlays, plus the now-narrowed inventory-screen and ornament rows. Viewport −28×−18 drift from pass 33/40 was superseded by all-graphics phases 76–81; champion status-box +8 px stride retired in pass 41. |
PARITY_MATRIX_DM1_V1.md:195:| `BLOCKED_ON_REFERENCE` | ~11 — timing, audio, CSB data, DM2 data, plus pixel-overlay rows across §1–§2 that remain unblocked until a ReDMCSB headless rasteriser or emulator capture lands. |
PARITY_MATRIX_DM1_V1.md:197:**Bottom line (updated 2026-04-27, pass 96 champion HUD source-origin refresh):** Firestaff has measured ownership progress (passes 29–32), measured visual drift (passes 33–34), a complete text-vs-graphics enumeration (pass 35), source-faithful champion status-box spacing (pass 41), a four-recruited-champion V1 HUD fixture locked by probe/screenshot evidence (pass 81), source-origin champion-HUD overlay evidence refreshed at x=0 rather than the legacy V2-only 12px inset (pass 96), source-matched DM1 viewport rectangle bounds via the all-graphics phases, source-bound action/inventory object icons plus C011 spell label cells, a narrowed inventory-screen row backed by deterministic capture evidence, narrowed ornament metadata/draw-path coverage, and the invented map overlay is explicitly `KNOWN_DIFF` but now debug-only and unreachable in normal V1 parity play. Dialog visuals are now source-backed across backdrop/version/message/choice/patch/input flow; endgame visuals are source-backed for `THE END`, champion mirrors, restart/quit boxes, and champion names, but still incomplete. **This still does not complete DM1/V1 parity**; remaining work is viewport content/draw-order parity, original screenshot overlays, audio/timing, remaining source endgame details, and non-current surfaces such as pointer/held-object icons and exact inventory/spell-panel/ornament placement overlays.
parity-evidence/dm1_all_graphics_phase72_capture_stale_palette_guard.md:5:The deterministic in-game capture pipeline previously produced a false rainbow/static visual regression when a stale binary interpreted packed framebuffer bytes through the old EGA-like path. The smoke test from phase 71 proved files existed, but did not yet prove the viewport was free of the obsolete saturated colours that exposed the stale path.
parity-evidence/dm1_all_graphics_phase72_capture_stale_palette_guard.md:9:Extended `run_firestaff_m11_ingame_capture_smoke.sh` with a viewport colour sanity check over `01_ingame_start_latest.ppm`:
parity-evidence/dm1_all_graphics_phase72_capture_stale_palette_guard.md:11:- samples the DM1 viewport rectangle `(x=0..223, y=33..168)`
parity-evidence/dm1_all_graphics_phase72_capture_stale_palette_guard.md:18:- threshold: stale-colour ratio must stay below 1% of sampled viewport pixels
parity-evidence/dm1_all_graphics_phase2137_2156_v1_viewport_occlusion_gate.md:1:# DM1 all-graphics phase 2137-2156 — V1 viewport source occlusion gate
parity-evidence/dm1_all_graphics_phase2137_2156_v1_viewport_occlusion_gate.md:4:Scope: Firestaff V1 dungeon viewport walls, floor ornaments/pits, stairs, teleporter fields, side doors, side door ornaments/masks.
parity-evidence/dm1_all_graphics_phase2137_2156_v1_viewport_occlusion_gate.md:8:Added a source-order visibility gate for normal V1 viewport passes.  The renderer now computes the nearest non-open center-lane square from the sampled 3×3 viewport cells and prevents farther source-backed side/floor/field/door overlays from being drawn beyond that blocker.
parity-evidence/dm1_all_graphics_phase2137_2156_v1_viewport_occlusion_gate.md:10:This keeps layout-696 wall/object work bounded by the same center-line occlusion rule already used by center contents, avoiding the bad class of viewport regressions where distant D2/D3 side features leak around a nearer closed center wall or door.  The source-backed content anchor seams remain intact, including C2500 object placement and C3200 creature `/side/back` and `/mid/large` bitmap-selection paths.
parity-evidence/dm1_all_graphics_phase2137_2156_v1_viewport_occlusion_gate.md:42:- This evidence note: `parity-evidence/dm1_all_graphics_phase2137_2156_v1_viewport_occlusion_gate.md`
parity-evidence/dm1_all_graphics_phase2137_2156_v1_viewport_occlusion_gate.md:43:- Existing generated V1 screenshot crops remain under `verification-screens/*_latest_viewport_224x136.png` for visual inspection of the normal DM1 viewport aperture.
parity-evidence/dm1_all_graphics_phase2137_2156_v1_viewport_occlusion_gate.md:47:Added `tools/verify_v1_viewport_occlusion_gate.py` as a lightweight source-shape gate for the same occlusion rule.  It verifies that:
parity-evidence/dm1_all_graphics_phase2137_2156_v1_viewport_occlusion_gate.md:50:- The pit, floor-ornament, stair, teleporter-field, side-wall, side-door, side-door-ornament, and side destroyed-door-mask source-backed passes all test `relForward > maxVisibleForward` before calling `m11_sample_viewport_cell`.
parity-evidence/dm1_all_graphics_phase2137_2156_v1_viewport_occlusion_gate.md:51:- `m11_draw_viewport` derives `maxVisibleForward` once from the sampled 3×3 cells and passes it into those families.
parity-evidence/dm1_all_graphics_phase2137_2156_v1_viewport_occlusion_gate.md:53:This is deliberately narrower than a pixel-parity proof: it catches the regression class where pits, floor ornaments, stairs, fields, side walls, or side-door overlays behind a blocking front wall/door become eligible for sampling/drawing again.  It does not validate HUD/original-capture/V2 paths and does not replace the asset-backed `firestaff_m11_game_view_probe` visual matrix.
parity-evidence/dm1_all_graphics_phase2137_2156_v1_viewport_occlusion_gate.md:58:python3 tools/verify_v1_viewport_occlusion_gate.py
parity-evidence/pass87_original_overlay_click_readiness_probe.json:91:      "path": "verification-screens/pass70-original-dm1-viewports/image0001-raw.png"
parity-evidence/pass87_original_overlay_click_readiness_probe.json:96:      "path": "verification-screens/pass70-original-dm1-viewports/image0002-raw.png"
parity-evidence/pass87_original_overlay_click_readiness_probe.json:101:      "path": "verification-screens/pass70-original-dm1-viewports/image0003-raw.png"
parity-evidence/pass87_original_overlay_click_readiness_probe.json:106:      "path": "verification-screens/pass70-original-dm1-viewports/image0004-raw.png"
parity-evidence/pass87_original_overlay_click_readiness_probe.json:111:      "path": "verification-screens/pass70-original-dm1-viewports/image0005-raw.png"
parity-evidence/pass87_original_overlay_click_readiness_probe.json:116:      "path": "verification-screens/pass70-original-dm1-viewports/image0006-raw.png"
parity-evidence/pass87_original_overlay_click_readiness_probe.json:134:      "verification-screens/pass70-original-dm1-viewports/image0001-raw.png",
parity-evidence/pass87_original_overlay_click_readiness_probe.json:135:      "verification-screens/pass70-original-dm1-viewports/image0002-raw.png",
parity-evidence/pass87_original_overlay_click_readiness_probe.json:136:      "verification-screens/pass70-original-dm1-viewports/image0003-raw.png",
parity-evidence/pass87_original_overlay_click_readiness_probe.json:137:      "verification-screens/pass70-original-dm1-viewports/image0004-raw.png",
parity-evidence/pass87_original_overlay_click_readiness_probe.json:138:      "verification-screens/pass70-original-dm1-viewports/image0005-raw.png",
parity-evidence/pass87_original_overlay_click_readiness_probe.json:139:      "verification-screens/pass70-original-dm1-viewports/image0006-raw.png"
parity-evidence/pass112_original_semantic_route_audit.json:3:  "attempt_dir": "verification-screens/pass70-original-dm1-viewports",
parity-evidence/pass112_original_semantic_route_audit.json:4:  "label_manifest": "verification-screens/pass70-original-dm1-viewports/original_viewport_shot_labels.tsv",
parity-evidence/pass112_original_semantic_route_audit.json:5:  "classifier_json": "verification-screens/pass70-original-dm1-viewports/pass80_original_frame_classifier.json",
parity-evidence/pass112_original_semantic_route_audit.json:26:    "missing shot-label manifest: verification-screens/pass70-original-dm1-viewports/original_viewport_shot_labels.tsv",
parity-evidence/pass112_original_semantic_route_audit.json:27:    "missing pass80 classifier JSON: verification-screens/pass70-original-dm1-viewports/pass80_original_frame_classifier.json"
parity-evidence/dm1_all_graphics_phase56_viewport_floor_ceiling_parity.md:1:# DM1 all-graphics phase 56 — viewport floor/ceiling parity flip
parity-evidence/dm1_all_graphics_phase56_viewport_floor_ceiling_parity.md:5:Implemented ReDMCSB `F0128_DUNGEONVIEW_Draw_CPSF` parity behavior for the DM1 viewport floor/ceiling base:
parity-evidence/dm1_all_graphics_phase56_viewport_floor_ceiling_parity.md:20:- `INV_GV_351` — normal V1 viewport floor/ceiling obeys DM1 parity flip
parity-evidence/dm1_all_graphics_phase56_viewport_floor_ceiling_parity.md:22:The gate renders two non-debug V1 frames with adjacent parity and verifies that the viewport floor/ceiling samples differ when GRAPHICS.DAT assets are available.
parity-evidence/dm1_all_graphics_phase56_viewport_floor_ceiling_parity.md:28:- `verification-m11/viewport-parity-clean-20260425-143212/01_ingame_start_latest.png`
parity-evidence/dm1_all_graphics_phase56_viewport_floor_ceiling_parity.md:29:- `verification-m11/viewport-parity-clean-20260425-143212/05_ingame_after_cast_latest.png`
parity-evidence/dm1_all_graphics_phase56_viewport_floor_ceiling_parity.md:36:- Remaining roughness: heavy source dither/noisy viewport texture and incomplete pixel-perfect wall/object composition.
parity-evidence/dm1_all_graphics_phase56_viewport_floor_ceiling_parity.md:42:PASS INV_GV_351 normal V1 viewport floor/ceiling obeys DM1 parity flip
parity-evidence/dm1_all_graphics_phase61_creature_c3200_multi_slot.md:1:# DM1 all-graphics phase 61 — creature C3200 multi-slot placement
parity-evidence/dm1_all_graphics_phase61_creature_c3200_multi_slot.md:5:Extended creature placement so multi-creature duplicate slots also resolve through the layout-696 `C3200_ZONE_` helper instead of the older Graphic558 midpoint approximation.
parity-evidence/dm1_all_graphics_phase61_creature_c3200_multi_slot.md:7:Phase 60 anchored single center-lane creatures to C3200; this pass makes the multi-creature branch use the same source coordinate family before local face-rect conversion.
parity-evidence/dm1_all_graphics_phase61_creature_c3200_multi_slot.md:13:- `INV_GV_256C` — creature draw path prefers C3200 over older G0224 midpoint for single front slot
parity-evidence/dm1_all_graphics_phase61_creature_c3200_multi_slot.md:25:PASS INV_GV_256B creature placement binds C3200 layout-696 source zone samples
parity-evidence/dm1_all_graphics_phase61_creature_c3200_multi_slot.md:26:PASS INV_GV_256C creature draw path prefers C3200 over older G0224 midpoint for single front slot
parity-evidence/dm1_all_graphics_phase61_creature_c3200_multi_slot.md:33:- Side-cell creature placement still needs the non-center C3200 groups, not just center groups.
parity-evidence/dm1_all_graphics_phase61_creature_c3200_multi_slot.md:34:- Full multi-creature screenshot coverage can be added once a stable focused multi-creature fixture exists.
parity-evidence/pass45_font_bank_wiring.md:16:- no viewport / side-panel relocation
parity-evidence/pass82_firestaff_source_zone_overlay_probe.md:13:  - `reference-artifacts/anchors/0000_viewport_full_frame.png` — C000 viewport aperture, 224×136.
parity-evidence/pass82_firestaff_source_zone_overlay_probe.md:18:  - viewport `0,33,224,136`
parity-evidence/pass82_firestaff_source_zone_overlay_probe.md:46:These numbers are measurements only. Dynamic viewport contents and route/state differences are expected to differ from a static anchor; do not read them as parity failure or parity success without source interpretation.
parity-evidence/pass82_firestaff_source_zone_overlay_probe.md:48:| scene | viewport C000 | action C010 | spell C009 | inventory C020 |
parity-evidence/pass82_firestaff_source_zone_overlay_probe.md:60:- The overlays make HUD, viewport, action/spell, inventory, and message regions inspectable without relying on local absolute paths.
parity-evidence/dm1_all_graphics_phase26_stairs_zones.md:10:The renderer now samples visible viewport cells with `DUNGEON_ELEMENT_STAIRS`, checks stairs orientation against party facing, and blits original wall-set 0 stairs graphics into the resolved ReDMCSB layout-696 zones.
parity-evidence/dm1_all_graphics_phase26_stairs_zones.md:79:- Next viewport domains: teleporters/fields and invisible pit variants.
parity-evidence/pass105_n2_original_entrance_route_unblock.md:23:`scripts/dosbox_dm1_original_viewport_reference_capture.sh` now maps original 320x200 coordinates to DOSBox client-relative X/Y for the generated `original_viewport_route_keys_xdotool.sh` helper. The same probe now logs:
parity-evidence/pass105_n2_original_entrance_route_unblock.md:31:`tools/pass80_original_frame_classifier.py` now distinguishes a low-color playable corridor from a flat close-wall frame by requiring low luma variance (`viewport.luma_stddev < 45.0`) for the `wall_closeup` guard. N2 evidence showed playable corridor frames with low color and six unique gray levels, but high luma variance (`viewport.luma_stddev=57.48`, `inventory_extent.luma_stddev=68.96`). Those should classify as `dungeon_gameplay`; the actual flat wall remains `wall_closeup` (`viewport.luma_stddev=27.99`).
parity-evidence/pass105_n2_original_entrance_route_unblock.md:37:| `bash -n scripts/dosbox_dm1_original_viewport_reference_capture.sh` | 0 | shell syntax OK |
parity-evidence/dm1_all_graphics_phase2897_2902_inventory_viewport_probe_batch.md:3:Branch: `parallel/inventory-viewport-20260426085439`
parity-evidence/dm1_all_graphics_phase2897_2902_inventory_viewport_probe_batch.md:24:- viewport zone `C007` is still pinned to DM1 source rect `(0,33,224,136)`.
parity-evidence/dm1_all_graphics_phase2897_2902_inventory_viewport_probe_batch.md:25:- viewport content placement seams expose source-backed C2500 object, C3200 creature, and C2900 projectile points inside C007.
parity-evidence/dm1_all_graphics_phase2897_2902_inventory_viewport_probe_batch.md:27:This is structural placement evidence, not a pixel overlay claim.  Remaining viewport work is still draw-order/content overlay against original runtime captures.
parity-evidence/pass77_actual_after_cast_fixture.md:43:ctest --test-dir build -R 'm11_(viewport_state|capture_route_state)' --output-on-failure
parity-evidence/dm1_all_graphics_phase77_legacy_viewport_probe_migration.md:1:# DM1 all-graphics phase 77 — migrate legacy viewport probe checks to DM1 rect
parity-evidence/dm1_all_graphics_phase77_legacy_viewport_probe_migration.md:5:After phase 76 introduced explicit `PROBE_DM1_VIEWPORT_*` constants, several older visual sanity gates still sampled the historical prototype viewport rectangle `(12,24,196,118)`. Those gates predate the source-bound DM1 viewport move and could pass while ignoring pixels in the real DM1 viewport margins.
parity-evidence/dm1_all_graphics_phase77_legacy_viewport_probe_migration.md:9:Migrated the remaining viewport-content sanity checks to use the source DM1 viewport constants `(0,33,224,136)`:
parity-evidence/dm1_all_graphics_phase77_legacy_viewport_probe_migration.md:11:- `INV_GV_10` synthetic feature cues inside viewport
parity-evidence/dm1_all_graphics_phase77_legacy_viewport_probe_migration.md:12:- `INV_GV_12` viewport/minimap coexistence
parity-evidence/dm1_all_graphics_phase77_legacy_viewport_probe_migration.md:13:- `INV_GV_12B` item/effect cues inside viewport
parity-evidence/dm1_all_graphics_phase77_legacy_viewport_probe_migration.md:16:- `INV_GV_92` asset-backed viewport palette diversity
parity-evidence/dm1_all_graphics_phase77_legacy_viewport_probe_migration.md:23:PASS INV_GV_10 synthetic feature cells add door, stair, and occupancy cues inside the viewport
parity-evidence/dm1_all_graphics_phase77_legacy_viewport_probe_migration.md:24:PASS INV_GV_12 viewport slice and minimap inset coexist in the same frame
parity-evidence/dm1_all_graphics_phase77_legacy_viewport_probe_migration.md:25:PASS INV_GV_12B viewport item and effect cues appear when real thing chains include loot and projectiles
parity-evidence/dm1_all_graphics_phase77_legacy_viewport_probe_migration.md:26:PASS INV_GV_16 viewport framing uses layered face bands and bright dungeon edges
parity-evidence/dm1_all_graphics_phase77_legacy_viewport_probe_migration.md:27:PASS INV_GV_17 front-cell focus adds a threat-colored viewport reticle plus contextual inspect readout
parity-evidence/dm1_all_graphics_phase77_legacy_viewport_probe_migration.md:28:PASS INV_GV_92 asset-backed viewport uses at least 6 distinct palette colors
parity-evidence/dm1_all_graphics_phase105_dialog_endgame_classification.md:11:- expands the original dialog-box graphic into the viewport
parity-evidence/dm1_all_graphics_phase105_dialog_endgame_classification.md:16:- uses `F0600_DIALOG_subroutine()` to blit either the viewport dialog or redraw the viewport depending on dialog set
parity-evidence/pass78_original_route_lock_attempt.md:13:This pass is deliberately blocker-safe: it refuses to normalize text-mode or menu-prompt captures as gameplay viewport references.
parity-evidence/pass78_original_route_lock_attempt.md:17:Updated `scripts/dosbox_dm1_original_viewport_reference_capture.sh`:
parity-evidence/pass78_original_route_lock_attempt.md:37:scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
parity-evidence/pass78_original_route_lock_attempt.md:56:scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
parity-evidence/pass78_original_route_lock_attempt.md:76:scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
parity-evidence/pass78_original_route_lock_attempt.md:99:scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
parity-evidence/dm1_all_graphics_phase_v1_leader_hand_transient_state.md:35:- Inventory-panel interactions and source draw-order work remain out of scope for this slice.
parity-evidence/dm1_all_graphics_phase16_center_door_opening_states.md:14:- states `4/5`: keep base closed/destroyed panel behavior for now until destroyed masks/ornaments are ported
parity-evidence/dm1_all_graphics_phase16_center_door_opening_states.md:71:Extend opening-state clipping to side doors and then add destroyed-mask/ornament/button handling from `F0111_DUNGEONVIEW_DrawDoor` / `F0109` / `F0110`.
parity-evidence/pass102_v1_viewport_creature_replacement_palette_gate.md:1:# Pass 102 — V1 viewport creature replacement palette gate
parity-evidence/pass102_v1_viewport_creature_replacement_palette_gate.md:4:Lane: Viewport/world visuals (DM1 V1 creatures/evidence)
parity-evidence/pass102_v1_viewport_creature_replacement_palette_gate.md:8:Added a focused probe for the renderer-facing DM1 creature replacement-color seam. This is not a new art path; it pins that the existing M11 creature compositor resolves the `G0243_as_Graphic559_CreatureInfo` replacement-color set nibbles into the `VIDRV_12_SetCreatureReplacementColors` slot-9/slot-10 palette targets before drawing creature sprites.
parity-evidence/pass102_v1_viewport_creature_replacement_palette_gate.md:12:- Firestaff `m11_game_view.c` creature aspect table carries `replacementColorSetIndices` from source-backed `G0243_as_Graphic559_CreatureInfo`.
parity-evidence/pass102_v1_viewport_creature_replacement_palette_gate.md:13:- Firestaff `m11_creature_replacement_colors()` resolves those nibbles through the VIDRV-style replacement-color table used by `m11_draw_creature_sprite_ex()`.
parity-evidence/pass102_v1_viewport_creature_replacement_palette_gate.md:24:- Updated `PARITY_MATRIX_DM1_V1.md` so the stale creature-palette row no longer claims rendering integration is absent.
parity-evidence/pass102_v1_viewport_creature_replacement_palette_gate.md:36:PASS INV_GV_114F2 creature slot-9/slot-10 replacement colors match source samples
parity-evidence/pass102_v1_viewport_creature_replacement_palette_gate.md:40:Full local run log retained at `parity-evidence/runs/pass102_viewport_creature_palette_20260428T1120Z/output.log` (ignored run artifact).
parity-evidence/pass102_v1_viewport_creature_replacement_palette_gate.md:42:This still does not claim pixel-perfect creature parity. The remaining blocker is the same as the wider viewport lane: run semantically matched original DM1 creature-heavy captures and overlay them against deterministic Firestaff fixtures.
parity-evidence/pass104_lane5_original_faithful_gate_status.md:11:Evidence-only consolidation after pass103. This pass reruns the current source/data and V1 gate stack and records the current honesty boundary: which gates are original-faithful/source-backed now, and which remain blocked by the missing semantic original DM1 gameplay route. No renderer, runtime, capture-route implementation, or lane-owned HUD/inventory/viewport files were changed.
parity-evidence/pass104_lane5_original_faithful_gate_status.md:20:| `python3 tools/verify_v1_viewport_draw_order_gate.py` | 0 | PASS: source-shape draw order verified |
parity-evidence/pass104_lane5_original_faithful_gate_status.md:21:| `python3 tools/verify_v1_viewport_occlusion_gate.py` | 0 | PASS: source-shape occlusion/sample gates verified |
parity-evidence/pass104_lane5_original_faithful_gate_status.md:32:- V1 viewport draw-order source shape is green: wall/door ornaments before open-cell contents; open-cell order remains floor ornaments → floor items → creatures → projectiles/effects.
parity-evidence/pass104_lane5_original_faithful_gate_status.md:33:- V1 viewport occlusion/source sampling shape is green across max-visible-forward, pits, floor ornaments, stairs, teleporter fields, side walls, side doors, side-door ornaments, destroyed-door masks, and viewport call-site wiring.
parity-evidence/pass104_lane5_original_faithful_gate_status.md:47:- `verification-screens/pass70-original-dm1-viewports/original_viewport_shot_labels.tsv` is still absent, so semantic checkpoints are not auditable.
parity-evidence/dm1_all_graphics_phase65_ingame_capture_palette_decode.md:23:- Viewport colored corruption did not fully disappear; the live 3D viewport still shows multicolored/noisy wall/floor artifacts.
parity-evidence/dm1_all_graphics_phase65_ingame_capture_palette_decode.md:24:- Therefore the remaining live viewport issue is not only capture palette decode; it likely lives in viewport/world rendering, source bitmap interpretation, clipping, masking, or draw-buffer handling.
parity-evidence/dm1_all_graphics_phase33_floor_ornament_right_flip.md:1:# DM1 all-graphics phase 33 — floor ornament right-side flip
parity-evidence/dm1_all_graphics_phase33_floor_ornament_right_flip.md:4:Scope: Firestaff V1 / DM1 floor ornament horizontal mirroring.
parity-evidence/dm1_all_graphics_phase33_floor_ornament_right_flip.md:8:Added horizontal flip support for source-zone blits and applied it to right-side floor ornament views.
parity-evidence/dm1_all_graphics_phase33_floor_ornament_right_flip.md:10:ReDMCSB `F0108_DUNGEONVIEW_DrawFloorOrnament` flips floor ornaments for right-side views:

## End status
## sync/n2-dm1-v1-20260428...origin/main [ahead 46]
?? ..-firestaff-worker-viewport-parity-20260427-040243/
?? verification-m11/lane1-original-faithful-parity-20260428-072831/
?? verification-m11/lane2-hud-20260428-070228/
?? verification-m11/n2_viewport_world_visuals_20260428_123237/
?? verification-m11/verification_summary.md
- ended: 2026-04-28T12:34:30+00:00
