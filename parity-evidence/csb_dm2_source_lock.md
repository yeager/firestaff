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
- CSB Amiga and Atari hard-disk candidates share the same selected dungeon hash but have different graphics hashes. The CSB graphics/render parity lane is now source-guarded to Atari ST English v2.x, not Amiga graphics.
- Canonical Atari ST English v2.x member anchor: `_extracted/csb-atari/Floppy Disks MSA/Chaos Strikes Back for Atari ST Game Disk v2.0 (English).msa` root `GRAPHICS.DAT` bytes=272069 sha256=`cff31dbdc071af2c6de8a0b9e1110b189e067706868d42fc8b2267e18422f687`; root `DUNGEON.DAT` bytes=2098 sha256=`59a72978879f3a3e9de3a6767ee069266d369244b1091314ddc16c03d8d41530`.
- Amiga `Graphics.DAT` remains a rejected separate lineage for that lane: `_extracted/csb-amiga/HardDisk/Chaos Strikes Back for Amiga v3.3 (French) Hacked by Meynaf/DungeonMaster/Graphics.DAT` bytes=435076 sha256=`3af5396fa32af08af5e0581a6cdf5b30c8397834efa5b9e0c8c991219d256942`.
