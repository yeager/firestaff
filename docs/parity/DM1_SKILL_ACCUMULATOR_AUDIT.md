# DM1 skill accumulator audit

Status: local correction under verification. Savegame work is not required
to reproduce it.

ReDMCSB `CHAMPION.C` F0303:730-738 selects a signed long accumulator for
MEDIA010/MEDIA506 and unsigned long for MEDIA720. `DEFS.H`:608-621 likewise
varies temporary experience signedness. F0303:752-770 adds temporary experience,
adds the base skill for hidden skills, halves the combined value, then computes
the level. F0304:887/893 adds permanent experience without saturation.

Two native level-query owners require coordinated correction:

- `F0848_LIFECYCLE_ComputeSkillLevel_Compat` uses signed additions and clamps
  negative results. `F0849_LIFECYCLE_AddSkillExperience_Compat` additionally
  saturates permanent experience at INT32_MAX.
- `dm1_skill_get_level_ex` independently performs signed additions. M11 uses
  this path for inventory-aware queries and F0848 as a fallback. Fixing only
  one owner would leave contradictory level results.

## Reproduction

On 2026-09-06, a local C probe set base experience to INT32_MAX and temporary
experience to 1, then called `dm1_skill_get_level`. Compiling the original
implementation with `-fsanitize=undefined -fno-sanitize-recover=undefined`
reported signed integer overflow at `dm1_v1_skill_experience_pc34_compat.c:216`
and exited 1. This is a boundary RAM probe, not evidence that normal play has
reached that value, and not an emulator capture.

## Implemented local correction

Both level-query owners now use `dm1_skill_accumulator_level`, with explicit
early/late policy and defined wrapping arithmetic. F0849 exposes policy-aware
awards and no longer saturates permanent XP. The live M10 award/query path and
M11 inventory/fallback/endgame queries consume the world policy. M11 selects
the late policy for the catalogue's PC34 EN/multilanguage and Amiga 3.6 hashes;
direct starts without a launcher hash identify the actual graphics member in
memory. DM1 FM Towns remains F20 signed, not CSB F31 unsigned.

The registered `dm1_skill_accumulator_policy` test covers both level owners
and the live world award/query boundary. Five original DOS 3.4 archive cases
verify that direct startup binds the unsigned policy. Five focused skill,
award, failure-XP and level-up gates pass. The original signed-overflow probe
now passes UBSan. These checks do not prove all-edition gameplay parity.

## Remaining verification

Six original-media cases now verify early Atari EN/DE/FR, Amiga 2.0 and
DM1 F20 EN/JP startup binding. The HD-named Amiga case selects 2.0 too.
Exercise late Amiga 3.6 with its authentic media. Unknown edition identities retain the legacy
signed default and are not claimed as identified. Audit standalone legacy
callers and getters, including `dm1_skill_add_experience`, separately; no live
production caller of that alternate award helper was found in the source scan.

## Original implementation requirements

Introduce an explicit edition policy shared by both query owners and the
before/after queries used for level gains. Preserve 32-bit XP bit patterns and
perform additions with defined wrapping. Apply source-edition signedness to
the accumulator and temporary words, including the hidden/base average.
Do not globally switch all editions to unsigned or retain saturation as a
substitute for original behavior.

Verify 0x7fffffff+1, 0xffffffff+1, hidden/base wrap before averaging, temporary
words 0x8000/0xffff, and no false level gain on wrap. Retain existing 16-bit
award-width regressions. Edition selection must reach live M11 queries and
M10 award consumers, not just a new unused helper.
