# CSB/DM2 source-lock evidence

This is a source-lock/provenance gate for local CSB and DM2 original inputs. It records archive and selected member identities only; it does **not** claim CSB/DM2 runtime or visual parity.

Result: PASS — 6 archives, 17 selected members checked.

## Locked inputs

| game/source | archive | status | selected members |
| --- | --- | --- | ---: |
| CSB Amiga hard-disk candidate | `Game,Chaos_Strikes_Back,Amiga,Software.7z` | `archive_and_candidate_members_locked_not_yet_curated` | 2 |
| CSB Atari ST hard-disk candidate | `Game,Chaos_Strikes_Back,Atari_ST,Software.7z` | `archive_and_candidate_members_locked_not_yet_curated` | 2 |
| DM2 DOS EN release | `Dungeon-Master-II-Skullkeep_DOS_EN.zip` | `archive_and_members_locked` | 4 |
| DM2 DOS repack | `Dungeon Master 2.zip` | `archive_and_core_data_locked_runtime_differs_from_en_release` | 4 |
| DM2 DOS CD/layout candidate | `Dungeon_Master_II_-_The_Legend_of_Skullkeep_1994.zip` | `archive_and_core_data_locked` | 4 |
| DM2 DOS source disassembly | `Game,Dungeon_Master_II,DOS,Source,Disassembly,Software.7z` | `archive_and_source_member_locked_curated_extract_present` | 1 |

## Missing gates

- No curated CSB extracted source-manifest is present in repo; CSB remains archive/member-candidate locked only.
- No CSB equivalent of tools/greatstone_dm1_source_lock_check.py exists yet.
- No DM2 runtime/source-manifest gate maps SKULL.ASM symbols to Firestaff code paths yet.
- No CSB/DM2 DOSBox/original capture or rendering parity probe is enabled.

## Notes

- DM2 DOS EN and CD/layout candidate share core `DUNGEON.DAT`, `GRAPHICS.DAT`, `SONGLIST.DAT`, and `SKULL.EXE` hashes.
- The `Dungeon Master 2.zip` repack shares core data with DM2 DOS EN but has a different `SKULL.EXE`; keep it as a locked alternate, not the canonical runtime source until curated.
- CSB Amiga and Atari candidates share the same selected dungeon hash but have different graphics hashes; a curator still needs to choose the CSB target/version before runtime work.
