# ReDMCSB source-gap and patch report

**Review date:** 2026-11-27
**Reference:** ReDMCSB source archive and Apple IIgs disk images found in the
locally installed ZIP under `.firestaff` (program revision not authenticated)
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
`COPYPRO6.C` `7fb68f2a3c10c6e4fceb0ba60c1eca4631254186c54120245b02166372d6cfee`,
`CHAMDRAW.C` `8192e23c011575b69da1e5a4c62616665f590cc37df1de9a548a266b24883a7c`,
`COMMAND.C` `68c7388a693c08c207c8dc7907acf1322b99a9467a035a044600c343c903a20f`,
and `STARTUP1.C`
`86b2498d027e4815505869028beae0f0023baf15ead8348d8a1955213e79120d`, and
`SWITCH.C` `3acc4d7dae9949d3bfd34ed6fad5b6e77940e7b652774e01340fcc8603dab494`.
The primary disk image's unadmitted `DUNGEON.MASTER` has MD5
`3265c912344d847a628e8e27e481f17a` and SHA-256
`46abaf65e3265cf305969b147abf322624762a823255ebf341c3f8d284d18107`.

## Findings

### Apple IIGS rest keyboard bindings are present in ReDMCSB

`COMMAND.C`'s G0460 table binds Return (`0x000D`) to C146 wake-up and
Escape (`0x001B`) to C147 freeze; G0461 binds Escape to C148 unfreeze.
Firestaff's IIGS input path had suppressed Return while resting and rejected
Escape whenever already frozen, leaving those source commands unavailable.
`M11_GameView_HandleInput` now routes Return to the shared rest wake-up and
toggles the IIGS frozen state in either direction. Other ordinary commands
remain suppressed during rest. The focused
`test_m11_iigs_rest_keyboard_bindings` regression checks the two Escape
transitions, rejects movement and rest-toggle while resting, and checks that
Return clears all three rest-state mirrors. It also checks that a frozen
party does not advance through an idle tick. This is a Firestaff integration
fix; no ReDMCSB source patch is indicated. Native IIGS runtime verification
remains outstanding.

This report separates proposed fixes to ReDMCSB itself from Firestaff
implementation gaps. A source-complete behavior does not need a ReDMCSB
patch simply because Firestaff's corresponding path is incomplete.

### IIGS reincarnation RNG is present in ReDMCSB

The source-reading concern that triggered this audit was resolved by reading
the complete preprocessor condition. `BASE.C F0027`'s `MEDIA202` branch at
lines 1694-1695 includes `G20E/G21E`; F0027 therefore has an IIGS
implementation. `DEFS.H:1` maps `M002_RANDOM(n)` to F0027, while the
HORIZCNT-based F0028/F0029 helpers are separate one-bit/two-bit APIs.

I inspected the installed ZIP's primary 2IMG image using Firestaff's bounded
ProDOS reader. Its `DUNGEON.MASTER` is type `$B5` and 232,158 bytes, but its
MD5 does not match either admitted program owner (G20: `51b9d22c7418651ee49ad77f37c80a3c`; G21:
`eccc49e7b29ee233cfb10615e8ec86f3`). A second valid `[b][non boot]` image
contains a same-length program with MD5 `49bc1f77956e03978521adfd23550ea5`;
the two local program copies differ at 121 bytes in 25 ranges. None of those
differences occur in the six spans used by the current IIGS metadata adapter
for source-owned tables. These facts show matching table data but do not
authenticate either executable revision as G20E or G21E.

I also used the locally installed CiderPress II CLI to disassemble the primary
image's 65816 code. The output around code bank `02`, offsets `$05A0-$05E6`,
shows an F0027-shaped LCG operation: the `$BB40E62D` multiplier, `+11` state
update, and the shifted 16-bit result. The adjacent HORIZCNT routines begin
at `$05F2` and `$0627`, consistent with the separate F0028/F0029 source
branches. The OMF also resolves this file's seed path: the routine reads and
writes a 32-bit state as two direct-page words at `$0084/$0086` (`02/$05B2`,
`02/$05D0-$05D3`). IIGS startup
`STARTUP1.C:229-238` reserves four words, calls `ReadTimeHex`, then stores the
first two returned words to `G0349` and `G0349+2`; the OMF shows those stores
to `$0084/$0086` at `03/$C939-$C945`. CiderPress II documents the first four
IIgs date/time bytes as seconds, minutes, hours, and year minus 1900. This
matches the seed initialization described for the source F0027 path. Because
the local program owner is not version-authenticated, the disassembly is
supporting evidence for the source comparison, not proof that this installed
build is an admitted G20E/G21E executable.

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

The Firestaff IIGS implementation now matches the startup clock seed's byte
layout and preserves zero through world initialization. `F0280`'s two
`M003_RANDOM(256)` food/water draws also match the source order. Exact parity
still depends on matching every earlier runtime F0027 draw, not merely the
seed and C161's twelve final draws.

### Installed Apple IIgs images: metadata-comparison scope

The installed `Dungeon-Master_Apple-IIgs_EN.zip` contains four nested ZIPs,
including the primary 2IMG and a `[b][non boot]` 2IMG. Both images contain an
819,200-byte ProDOS volume. Their `DUNGEON.MASTER` files are 232,158 bytes;
the primary and `[b]` MD5s are `3265c912344d847a628e8e27e481f17a` and
`49bc1f77956e03978521adfd23550ea5`, respectively. Neither equals the admitted
G20/G21 executable hashes.

I then compared all 49 source spans read by the metadata and audio adapters
against the primary and `[b]` program files using Firestaff’s ProDOS reader
and the installed `libfirestaff_m12`/ZIP/zlib libraries. This includes all
startup and dungeon palettes, UI geometry, inventory/chest hit tables, ornament/object
frames and aspects, creature/projectile/explosion tables, attack sounds,
native sound records, and the distance-volume grid. Every span was byte-equal
between these two installed programs. This closes the earlier *between these
two files* comparison gap, but does not authenticate their ownership: neither
program was compared to the admitted G20/G21 executable at every span, and a
third-party modification could preserve these tables while changing code.

The primary disk's `GRAPHICS.GAME`, `DUNGEON.GAME`, `DUNGEON.BONUS`, and
`SYSTEM/START` match the currently recognized authentic resource hashes. The
`[b]` disk matches the first three, but its `SYSTEM/START` differs. Therefore
the primary is a strong candidate for an original-media variant with altered
program bytes, while the `[b]` image remains less fully bound. Firestaff's
current owner validator still requires an exact G20/G21 program hash. Do not
weaken that gate from table equality or matching companions alone; identify
the executable revision and bind the program to an authoritative original
release before admitting another identity.

**Remaining Firestaff gap:** admit and identify the installed program revision,
then verify startup and candidate RNG state at runtime on version-authenticated
original media, including source draws between `F0280` and C161.
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

### ReDMCSB copy-protection patch F — retain fuzzy-sector bytes

`COPYPRO6.C:149-198` implements the `MEDIA607_X30J_X31J` check. It skips the
eight-byte `HPR-0007` header, compares the next four random bytes against two
prior readings in `G0293_ai_FuzzyBits_CPSE`, shifts the newest reading to the
second slot, and then line 197 mistakenly stores `P0593_puc_SectorBuffer`
itself into that history array. `DEFS.H:5667-5669` declares the X30J history
as 32 `int16_t` values (64 bytes); `DEFS.H:3216-3218` maps the check buffer to
`G0295`, which `BASE.C:20-23` allocates as 256 bytes for X30J. Thus the four
bytes at offsets 8-11 are within the read buffer and the pointer write is an
incorrect history update, not an overrun of the history array.

Replace the pointer assignment with a bytewise copy of the same four values
the assembly compared:

```diff
-        *((unsigned char**)G0293_ai_FuzzyBits_CPSE) = P0593_puc_SectorBuffer; /* BUGX_XX: Stores the address ... instead of actual bytes in the buffer */
+        ((unsigned char*)G0293_ai_FuzzyBits_CPSE)[0] = P0593_puc_SectorBuffer[8];
+        ((unsigned char*)G0293_ai_FuzzyBits_CPSE)[1] = P0593_puc_SectorBuffer[9];
+        ((unsigned char*)G0293_ai_FuzzyBits_CPSE)[2] = P0593_puc_SectorBuffer[10];
+        ((unsigned char*)G0293_ai_FuzzyBits_CPSE)[3] = P0593_puc_SectorBuffer[11];
```

Bytewise stores avoid alignment and aliasing assumptions while preserving the
assembly's four-byte read order. **Apply after:** compile the X30J target and
exercise repeated sector-7 reads using authenticated original media to confirm
the two-reading history and copy-protection result. Source and disassembly
evidence establish the defect and intended values; no matching X30J executable
or runtime trace was available to confirm the final behavior.

### ReDMCSB allocation-failure patch G — validate segment-header buffer

`SWITCH.C:103-109` allocates `L5182_puc_SegmentHeadersBuffer` using the
untrusted file header's bounded segment count, then checks
`L5181_puc_HeaderBuffer` instead. The first allocation has already succeeded
and is non-NULL, so failure of the second allocation is not detected; the next
cast and `Fread` receive a NULL destination. Check the pointer that was just
allocated:

```diff
         L5182_puc_SegmentHeadersBuffer = F1650_AllocateMemory(L5180_l_ByteCount, P5081_i_Requirements);
-        if (L5181_puc_HeaderBuffer == NULL) { /* BUGX_XX: Wrong variable tested, should be L5182_puc_SegmentHeadersBuffer */
+        if (L5182_puc_SegmentHeadersBuffer == NULL) {
                 L5179_i_Result = -6;
```

**Apply after:** compile each affected Amiga `exec` target and inject failure
of the second allocation after the header allocation succeeds. The existing
error label already closes the file and releases the first buffer; verify that
the corrected check returns `-6` before a read through a NULL segment-header
pointer. The source comment and control flow establish the faulty check; no affected-target runtime test
was available during this review.

## Other ReDMCSB annotations to investigate

| Source | Candidate | Next evidence needed |
| --- | --- | --- |
| `GRF1.C`, `VDEO1.C`, `VDEO2.C` | Graphics/display paths contain platform assembly boundaries. | Retain the target ABI and authentic framebuffer/media evidence; no portable patch follows from the callable names alone. |

## Tooling and limits

`da65` and .NET SDK 10 were installed locally in `.firestaff/tools` because
system package installation is disabled. CiderPress II was cloned and built
there and used to disassemble the extracted OMF; Firestaff's ProDOS reader
performed the extraction. These tools and extracted game files are outside
Git; no user media was added to the repository.

The ReDMCSB hunks are proposed source patches, not changes to the external
archive. Patches E, F, and G remain source-supported candidates until their
target builds and relevant runtime behavior are checked. The IIGS patch is
applied in Firestaff.
No upstream message, release, or tag was created. Source/disassembly evidence
is not a substitute for a matched original-media interactive runtime trace.
