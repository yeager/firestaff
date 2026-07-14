# ReDMCSB numbered-symbol audit

This is an implementation-audit index for the supplied ReDMCSB WIP 2021-02-06
source tree.  It is deliberately conservative: a ReDMCSB-style name in a
Firestaff comment is useful traceability, but it is not proof that the same
original behaviour is implemented or tested.

The generated companion file, `numbered_source_symbol_inventory.tsv`, lists every
unique numbered source symbol.  Its status column has this strict meaning:

| Status | Meaning |
| --- | --- |
| `referenced_not_verified` | The exact symbol occurs in Firestaff source, headers, or tests.  The implementation still needs a function-by-function behavioural review and evidence. |
| `unmapped_or_unverified` | No exact Firestaff reference was found.  It must not be represented as implemented. |
| `semantic_mapping_not_audited` | A data/label family where a one-to-one C identifier is not required, but Firestaff has no audited semantic mapping. |
| `non_executable_symbol_not_audited` | A module or auxiliary label, not an independently callable implementation target. |

## Source inventory

| Family | Unique symbols | ReDMCSB meaning | Firestaff implementation status |
| --- | ---: | --- | --- |
| `Axxx` | 4 | Auxiliary numbered labels. | Not independently callable.  No audited mapping yet. |
| `Cxxx` | 866 | Numbered constants, not functions. | Some semantics are represented by Firestaff enums, tables, and compatibility constants, but no complete `Cxxx`-to-semantic manifest exists.  Correctness is not proven globally. |
| `Exxx` | 6 | Exception handlers. | Callable family.  Exact references are indexed, but all six need explicit behavioural and platform-boundary review. |
| `Fxxxx` | 2,104 | ReDMCSB functions. | Callable family.  Firestaff has source-locked implementations and tests for selected routines, for example DM1 `DUNVIEW.C` `F0119`/`F0120`, but there is no evidence-backed one-to-one completion claim for all 2,104 functions. |
| `Gxxxx` | 2,074 | Globals. | Storage labels, not functions.  Firestaff may intentionally use different ownership and names; a complete semantic ownership map is still missing. |
| `Lxxxx` | 4,616 | Locals. | Compiler/decompiler local labels.  They have no standalone cross-language implementation target.  Audit is only meaningful through their enclosing function. |
| `Mxxx` | 397 | Module or macro labels. | Non-callable source organisation labels.  No direct one-to-one implementation claim is appropriate. |
| `Pxxxx` | 2,996 | Parameters. | Function-local ABI/decompiler parameter labels.  Audit belongs to the enclosing function and its tested contract. |
| `Rxxx` | 8 | TOS/system routines. | Callable system boundary.  Firestaff substitutes portable SDL/host services where appropriate, but each mapping needs an explicit equivalence or non-applicability decision. |
| `Sxxx` | 19 | Assembly or special routines. | Callable special boundary.  Each needs an explicit compatibility decision; a source-name match is insufficient. |

The four callable families contain **2,137** routines: `F` 2,104, `E` 6,
`R` 8, and `S` 19.  `G`, `L`, and `P` are globals, locals, and parameters;
`M` is module/macro labelling.  They must not be counted as functions.

At the 2026-07-14 inventory snapshot, Firestaff source/header/test files
contain exact numeric references for 938 of the 2,104 `Fxxx` identifiers and
none of the `Exxx`, `Rxxx`, or `Sxxx` identifiers.  These are traceability
counts only, not correctness counts; all 2,137 callable rows remain outside a
complete verified one-to-one implementation claim.

## How to use the inventory

Regenerate it from a local ReDMCSB source checkout:

```bash
scripts/generate_numbered_source_symbol_inventory.sh \
  /path/to/ReDMCSB_WIP20210206/Toolchains/Common/Source
```

The generator records a symbol as referenced only when the full numbered name
appears under Firestaff `src/`, `include/`, or `tests/`.  This prevents prose
in TODO/DONE from promoting a symbol.  A reviewer must then inspect the
function contract, Firestaff call site, original data boundary, and focused
test before changing its status to a verified implementation.

## Audit rule

Do not create synthetic behaviour to fill an `unmapped_or_unverified` row.
Use original PC34/CSB data and ReDMCSB control flow where applicable, and keep
platform-specific TOS or assembly replacements explicitly documented.
