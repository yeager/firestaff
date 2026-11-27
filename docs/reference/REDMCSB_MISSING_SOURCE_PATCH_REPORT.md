# ReDMCSB source-gap and patch report

**Review date:** 2026-11-27
**Reference:** ReDMCSB source archive and the locally installed Apple IIgs
G20E retail image under `.firestaff`
**Scope:** source defects and omissions relevant to Firestaff DM1/CSB work.
**Status:** report and Firestaff-side patch only; the external ReDMCSB archive
was not modified.

The source files used for the findings have these local SHA-256 identities:
`BASE.C` `d0dd42b1443d171cb56e59e9e1c63fee4c0308b8dc2cbafcca403ff2bfa26b31`,
`IBMIO.C` `2e56d93e2e6a61168279b6723ade3686d65c325fdad73f80622bdeea88cda3bb`,
`MUSIC.C` `b3216caa7c350a0de66ff8b669290a85e54ca4b6c5c9478ebd9acf9995a2cd88`,
`REVIVE.C` `d0168118386c297cadab04525ab24efcf7138e0156074207d727d1edc3d4a18b`,
`PANEL.C` `88b706cb539ae848c065a8ad02162a43c1d6344a6285160adaf0c59b2b1d074a`,
`DEFS.H` `33b3160a5dd7d9c62b9ad0f3d26ac09ddacd6328c9717656fe2bbe8786625728`,
`CHAMDRAW.C` `8192e23c011575b69da1e5a4c62616665f590cc37df1de9a548a266b24883a7c`,
`COMMAND.C` `68c7388a693c08c207c8dc7907acf1322b99a9467a035a044600c343c903a20f`,
and `STARTUP1.C`
`86b2498d027e4815505869028beae0f0023baf15ead8348d8a1955213e79120d`. The
disk image's `DUNGEON.MASTER` has SHA-256
`46abaf65e3265cf305969b147abf322624762a823255ebf341c3f8d284d18107`.

## Findings

This report separates proposed fixes to ReDMCSB itself from Firestaff
implementation gaps. A source-complete behavior does not need a ReDMCSB
patch simply because Firestaff's corresponding path is incomplete.

### IIGS reincarnation RNG is present in ReDMCSB

The source-reading concern that triggered this audit was resolved by reading
the complete preprocessor condition. `BASE.C F0027`'s `MEDIA202` branch at
lines 1694-1695 includes `G20E/G21E`; F0027 therefore has an IIGS
implementation. `DEFS.H:1` maps `M002_RANDOM(n)` to F0027, while the
HORIZCNT-based F0028/F0029 helpers are separate one-bit/two-bit APIs.

I also extracted `DUNGEON.MASTER` (ProDOS type `$B5`, 232,158 bytes) from the
first authentic G20E 2IMG disk with Firestaff's existing bounded ProDOS
reader, then used the locally installed CiderPress II CLI to disassemble the
65816 code. The output around code bank `02`, offsets `$05A0-$05E6`, shows the
F0027 LCG operation: the `$BB40E62D` multiplier, `+11` state update, and the
shifted 16-bit result. The adjacent HORIZCNT routines begin at `$05F2` and
`$0627`, matching the distinct F0028/F0029 source branches. The OMF also
resolves the seed path: F0027 reads and writes its 32-bit state as two direct-
page words at `$0084/$0086` (`02/$05B2`, `02/$05D0-$05D3`). IIGS startup
`STARTUP1.C:229-238` reserves four words, calls `ReadTimeHex`, then stores the
first two returned words to `G0349` and `G0349+2`; the OMF shows those stores
to `$0084/$0086` at `03/$C939-$C945`. CiderPress II documents the first four
IIgs date/time bytes as seconds, minutes, hours, and year minus 1900. This is
the F0027 startup seed; it overwrites the source global's zero-initialized
BSS value before the game uses it.

**Disposition:** no ReDMCSB source patch is needed for IIGS F0027. Firestaff's
previous SDL-counter substitution and fixed `0xF1A5` initializer were gaps.
It now seeds the IIGS world from the local `ReadTimeHex` fields and uses the
source-compatible session RNG for each of the twelve `M002_RANDOM(7)` calls.
The native world initializer preserves a zero seed, while the generic PC34
initializer keeps its existing zero-to-one rule:

```diff
-static int m11_iigs_random_statistic(void)
+static int m11_iigs_random_statistic(M11_GameViewState* state)
 {
-    const uint16_t hostSample = (uint16_t)SDL_GetPerformanceCounter();
-    return (int)(hostSample % LIFECYCLE_STAT_COUNT);
+    if (!state) return 0;
+    return F0732_COMBAT_RngRandom_Compat(
+        &state->world.masterRng, LIFECYCLE_STAT_COUNT);
 }
```

The Firestaff IIGS path now matches the startup clock seed's byte layout and
preserves zero through world initialization. `F0280`'s two
`M003_RANDOM(256)` food/water draws also match the source order. Exact parity
still depends on matching every earlier runtime F0027 draw, not merely the
seed and C161's twelve final draws.

**Remaining Firestaff gap:** verify the startup and candidate RNG state at
runtime on original media, including source draws between `F0280` and C161.
The IIGS F0280 candidate path already consumes two matching F0027 draws for
`M003_RANDOM(256)` food and water at `REVIVE.C:184-185`; Firestaff's native
candidate constructor does the same. What remains unverified is the global
seed at first use and any intervening game-time draws before the C161 action.
The full IIGS C161 route also needs original-media runtime verification.

### ReDMCSB call-site patch A — MEDIA349 error argument order

`GSTHINGS.C:203-205` declares `F2165_PrintErrorMessage(errorNumber,
message)`. `BASE.C:1132` uses that order; the MEDIA349 call at line 1129
reverses it and already labels the defect.

```diff
diff --git a/Toolchains/Common/Source/BASE.C b/Toolchains/Common/Source/BASE.C
--- a/Toolchains/Common/Source/BASE.C
+++ b/Toolchains/Common/Source/BASE.C
@@
 #ifdef MEDIA349_G14ED
-        F2165_PrintErrorMessage("Dungeon Master system error ->", P0018_i_ErrorNumber); /* BUGX_XX: Wrong parameters order */
+        F2165_PrintErrorMessage(P0018_i_ErrorNumber, "Dungeon Master system error ->");
 #endif
```

**Apply after:** compile the G14ED target and exercise a nonzero error code to
check formatting and stop behavior. This repairs the source call contract;
the original executable may still contain the same defect.

### ReDMCSB call-site patch B — FM Towns CD state query

`MUSIC.C:459-474` defines `F2257_IsCDPlaying(void)` and returns live CD
playback state. In the MEDIA670 `F0740_MUSIC_Pause` branch, line 531 tests the
function designator instead of calling it. The designator is always
non-null, so the playing branch is always taken.

```diff
diff --git a/Toolchains/Common/Source/MUSIC.C b/Toolchains/Common/Source/MUSIC.C
--- a/Toolchains/Common/Source/MUSIC.C
+++ b/Toolchains/Common/Source/MUSIC.C
@@
-        if (F2257_IsCDPlaying) { /* BUGX_XX: function is not called because of missing () */
+        if (F2257_IsCDPlaying()) {
```

**Apply after:** compile F31E/F31J and exercise pause while CD audio is active
and after it has stopped.

### ReDMCSB defensive patch C — DOS device type bounds

`IBMIO.C:2167-2175` defines an eight-entry conversion table for device types
0 through 7. The DOS IOCTL result at `F8129_GetConvertedDeviceType:2193` is an
untrusted byte; the source comment records type 9 for a 2.88 MB floppy. The
`F0715_GetConvertedDeviceType` callers use zero as the unsupported/no-drive
result (`FLOPPY.C:389,418,427`; `CEDT029.C:67,78,87`), so use that value for
types outside the table. Initialize the result byte too, so a failed IOCTL
cannot leave the index uninitialized.

```diff
         L2687_auc_[0] = 0;
+        L2687_auc_[1] = 8; /* Unsupported device until DOS returns a type. */
         asm     mov     ax, 440Dh      /* INT 21,44,D / IOCTL,D - Generic I/O for Block Devices */
         asm     mov     bx, P3323_i_DriveNumber
         asm     mov     cx, 860h       /* 08 for disk drive (block device) / 60 get device parameters */
         asm     push    ds
         asm     lds     dx, L2688_puc_      /* DS:DX pointer to parameter block */
         asm     int     21h            /* DOS - 2+ - IOCTL - */
         asm     pop     ds
-        L2689_i_ = G8099_ai_ConvertedDeviceType[L2687_auc_[1]]; /* BUGX_XX: G8099_ai_ConvertedDeviceType only contains 8 bytes, no boundary check on the value of L2687_auc_[1]. For example, with a 2.88MB floppy disk drive on DOS 5.0, this would return 9 and read a byte outside of the array */
+        if (L2687_auc_[1] < 8)
+                L2689_i_ = G8099_ai_ConvertedDeviceType[L2687_auc_[1]];
+        else
+                L2689_i_ = 0;
```

**Apply after:** compile the DOS target and verify device types 7, 8, and 9
return the table value or unsupported result without reading beyond the
array. This is a source-level bounds fix; no DOS runtime trace was captured.

### ReDMCSB memory-safety patch D — preserve rename target after duplicate name

`REVIVE.C:639-642` temporarily points the edit cursor at the title when a
duplicate-name check fails during title editing, but the restore line always
points it at `Name`. Later backspaces can then edit before the champion name
buffer. Restore the pointer according to the active field after restoring the
trimmed name:

```diff
-                                M547_STRCPY(L0812_pc_RenamedChampionString = P0597_ps_Champion->Name, L0821_ac_ChampionNameBackupString); /* BUGX_XX L0812_pc_RenamedChampionString should be set to the champion name ONLY if L0809_i_RenamedChampionString != C0_RENAME_CHAMPION_TITLE. If the entered champion name is identical to the name of another champion already in the party and the title is being edited then an inconsistency appears: L0812_pc_RenamedChampionString points to the champion name whereas AL0808_ui_CharacterIndex contains the current character index in the champion title. If the title contains at least one character then this leads to the ability to use backspace past the beginning of the champion name and then overwrite data of the preceding champion in memory */
+                                M547_STRCPY(P0597_ps_Champion->Name, L0821_ac_ChampionNameBackupString);
+                                L0812_pc_RenamedChampionString =
+                                        (L0809_i_RenamedChampionString == C0_RENAME_CHAMPION_TITLE)
+                                                ? P0597_ps_Champion->Title
+                                                : P0597_ps_Champion->Name;
```

**Apply after:** compile affected editions and exercise duplicate-name rejection
while editing a title, then backspace and type at the start of the title. The
source comments document the unsafe pointer state; the proposed repair has
not been run against the original executable.

### ReDMCSB zone-consistency patch E — clear the P20JB music toggle zone

`PANEL.C:2391-2394` clears `C568_ZONE_TOGGLE_MUSIC_ICON` when the P20JB
inventory hides the music state. Under `MEDIA469_P20JB`, however,
`DEFS.H:3933-3934` defines `M701_ZONE_TOGGLE_MUSIC_ICON` as zone 570.
Independent draw and input owners both use that `M701` constant:
`CHAMDRAW.C:1099-1101` paints or clears the icon in zone 570, and
`COMMAND.C:417` routes `C141_COMMAND_TOGGLE_MUSIC` through zone 570. The
PANEL clear therefore targets the stale zone 568 and can leave the old music
icon pixels behind when this P20JB state is hidden.

```diff
diff --git a/Toolchains/Common/Source/PANEL.C b/Toolchains/Common/Source/PANEL.C
--- a/Toolchains/Common/Source/PANEL.C
+++ b/Toolchains/Common/Source/PANEL.C
@@
         if (!G2106_B_ShowMusicState) {
-                F0734_ClearZoneInInventory(C568_ZONE_TOGGLE_MUSIC_ICON);
+                F0734_ClearZoneInInventory(M701_ZONE_TOGGLE_MUSIC_ICON);
         }
```

**Apply after:** compile the P20JB target and verify that hiding the music
state clears the same zone that its draw and command tables own. The source
owners agree on 570; an authentic P20JB layout/hit-table check and runtime
redraw check are still needed before treating the patch as confirmed behavior.

## Other ReDMCSB annotations to investigate

| Source | Candidate | Next evidence needed |
| --- | --- | --- |
| `COPYPRO6.C:197` | The comment says an address is written where fuzzy-sector bytes are expected. | Resolve `G0293`'s effective type, allocation size, and consumers before selecting a copy width. |
| `GRF1.C`, `VDEO1.C`, `VDEO2.C` | Graphics/display paths contain platform assembly boundaries. | Retain the target ABI and authentic framebuffer/media evidence; no portable patch follows from the callable names alone. |

## Tooling and limits

`da65` and .NET SDK 10 were installed locally in `.firestaff/tools` because
system package installation is disabled. CiderPress II was cloned and built
there and used to disassemble the extracted OMF; Firestaff's ProDOS reader
performed the extraction. These tools and extracted game files are outside
Git; no user media was added to the repository.

The five ReDMCSB hunks are proposed source patches, not changes to the external
archive. Patch E remains a source-consistency candidate until P20JB media and
runtime redraw behavior are checked. The IIGS patch is applied in Firestaff.
No upstream message, release, or tag was created. Source/disassembly evidence
is not a substitute for a matched original-media interactive runtime trace.
