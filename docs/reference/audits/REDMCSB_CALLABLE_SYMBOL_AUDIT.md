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
| `implemented-narrow` | 164 | Scoped PC34 compatibility adapters with focused behavioural evidence. |
| `implemented-source-audited` | 7 | Source-owned platform gates with explicit fail-closed host contracts. |
| `implemented` | 5 | Reviewed direct compatibility implementations. |
| `implemented_narrow` | 6 | Legacy ledger spelling for scoped, focused adapters. |
| `verified-source-mapping` | 9 | Reviewed source mappings with focused evidence. |
| `referenced-not-verified` | 820 | A numbered identifier occurs under Firestaff code or tests, but lacks a reviewed contract-level claim. |
| `source-nonapplicable` | 2 | Reviewed as non-applicable to a PC34 runtime route. |
| `platform-only/non-applicable` | 2 | Explicit non-runtime platform/toolchain surface. |
| `missing` | 1,122 | No exact Firestaff source/header/test reference was found by the inventory generator. This is an audit gap, not permission to synthesize behaviour. |

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

1. **Interrupt and timing boundaries:** the PC34/PAL scheduler and source
   host-service gates now cover `E0013`, `E0014`, `E0015`, `E0017`, `E0061`,
   `S0080`, and `S0081`. Remaining platform-only timer, DMA and floppy aliases
   require explicit source contracts before they can be promoted.
2. **TOS calls:** `R0055`, `R0056`, and `R0057` need per-call contracts for
   BIOS, XBIOS, and GEMDOS effects; `R0138` remains a copy-protection hidden
   code launcher and must remain unavailable unless authentic media evidence
   supports a bounded route.
3. **Mouse and input ownership:** `F0069`, `F0070`, `F0073`, and
   `S0072`/`S0074`/`S0075`/`S0076` now have source-owned pointer, packet and
   command-order contracts. Remaining input aliases need the same treatment.
4. **Low-level presentation primitives:** `F0134` and `F0135` have bounded
   pixel-level contracts; their remaining work is live renderer consumption.
5. **Dungeon/group state helpers:** `F0145`, `F0146`, `F0147`, and `F0196`
   now have raw C04/active-group handoff coverage. Broader group simulation
   remains tracked in the DM1 runtime queue.

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
