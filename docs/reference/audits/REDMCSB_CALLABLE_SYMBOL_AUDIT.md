# ReDMCSB Callable Symbol Audit: Partition 1

## Scope

This is partition 1 of the ReDMCSB callable-symbol audit. It covers the full
`E`/`F`/`R`/`S` population from the repository's authoritative numbered-symbol
inventory: 2,137 callable symbols in total.

| Family | Meaning in ReDMCSB | Symbols |
| --- | --- | ---: |
| `E` | Exception handlers | 6 |
| `F` | Functions | 2,104 |
| `R` | TOS/system routines | 8 |
| `S` | Assembly/special routines | 19 |

The source inventory is the ReDMCSB WIP 2021-02-06 inventory committed as
`19aaed93` (`docs/reference/numbered_source_symbol_inventory.tsv`). Its
`redmcsb_files` column records the source files in which each identifier was
found. This audit is limited to DM1/CSB Firestaff code, headers, and tests; it
does not claim DM2, Nexus, or Theron coverage.

## Status Rule

| Status | Count | Rule |
| --- | ---: | --- |
| `verified implementation` | 0 | Requires a reviewed Firestaff implementation plus focused behavioural evidence. No partition-1 symbol met both requirements during this pass. |
| `referenced-not-verified` | 938 | The numbered identifier occurs under Firestaff `src/`, `include/`, or `tests`, but that occurrence has not been promoted to a reviewed implementation claim. It may be a comment, citation, declaration, or incomplete route. |
| `semantic replacement` | 0 | Requires an explicit reviewed mapping from the ReDMCSB callable contract to a differently named Firestaff implementation. No such per-symbol mapping exists in this partition. |
| `missing` | 1,199 | No exact Firestaff source/header/test reference was found by the inventory generator. This is an audit gap, not permission to synthesize behaviour. |

Comments are traceability only. In particular, an `Fxxxx` citation in a source
comment does **not** make that function verified, and neither a platform stub
nor a generic host API is treated as a semantic replacement without a
contract-level review.

The machine-readable companion,
`REDMCSB_CALLABLE_SYMBOL_AUDIT.tsv`, contains one row per callable symbol with
its ReDMCSB source-file provenance and status.

## Concrete Missing Work

The largest unreferenced groups are platform-specific routines, so they should
be resolved as explicit compatibility decisions rather than copied blindly:

1. **Interrupt and timing boundaries:** `E0013`, `E0014`, `E0015`, `E0017`,
   `E0061`, and special routines such as `S0080`/`S0081` need an audited
   SDL/host scheduling decision. Their Atari ST timer, VBlank, DMA, and floppy
   assumptions are not portable implementation evidence.
2. **TOS calls:** `R0055`, `R0056`, and `R0057` need per-call contracts for
   BIOS, XBIOS, and GEMDOS effects; `R0138` remains a copy-protection hidden
   code launcher and must remain unavailable unless authentic media evidence
   supports a bounded route.
3. **Mouse and input ownership:** unreferenced `F0069`, `F0070`, `F0073`, and
   the `S0072`/`S0074`/`S0075`/`S0076` family require an input-sampling and
   screen-update ordering audit before any equivalence claim.
4. **Low-level presentation primitives:** `F0134` and `F0135` are not
   automatically covered by existing high-level viewport work. Their bitmap
   fill and box semantics need pixel-level contracts before being mapped to a
   host renderer.
5. **Dungeon/group state helpers:** `F0145`, `F0146`, `F0147`, and `F0196`
   lack exact Firestaff references in this inventory. Any future movement or
   save/load work that needs group cells, directions, or active-group
   initialization should establish their source contract and focused tests.

The remaining missing `F` rows include non-DM1 media shims and utility-disk
surfaces. They remain intentionally unclaimed until a DM1/CSB call site and
an evidence-backed compatibility decision exist.

## Reproduction

The inventory was originally generated from the external ReDMCSB source tree
with:

```bash
scripts/generate_numbered_source_symbol_inventory.sh \
  /path/to/ReDMCSB_WIP20210206/Toolchains/Common/Source
```

This audit filters that inventory to `E`, `F`, `R`, and `S`, then maps
`referenced_not_verified` to `referenced-not-verified` and
`unmapped_or_unverified` to `missing`. A later promotion must add a
symbol-specific implementation and behavioural-evidence review; regenerating
or grepping the inventory alone is insufficient.
