# ReDMCSB auxiliary label and parameter full audit

## Scope

This audit accounts for every numbered ReDMCSB auxiliary symbol in the
2021-02-06 WIP source corpus, comparing its source ownership against the
Firestaff source tree. The machine-readable ledger is
[`REDMCSB_LABEL_PARAMETER_FULL_AUDIT.tsv`](REDMCSB_LABEL_PARAMETER_FULL_AUDIT.tsv).
It contains one row for each original identifier, with concrete source
provenance and a conservative Firestaff disposition.

| Family | Meaning | Symbols | Required decision |
| --- | --- | ---: | --- |
| `A` | Auxiliary alias | 4 | Function-local alias; no standalone port. |
| `L` | Local storage | 4,616 | Automatic local state; no standalone port. |
| `M` | Macro/module label | 397 | Preserve the expression/layout semantic at its use site; no standalone port. |
| `P` | Function parameter | 2,996 | Preserve as part of the enclosing routine contract; not an independently portable ABI entry. |
| **Total** |  | **8,013** |  |

## Interpretation

`A` and `L` rows are explicit `non_port_local_state`: their meaningful scope is
the enclosing ReDMCSB routine. `M` rows are explicit `non_port_module_state`:
the named macro can encode an important expression, accessor, or layout
convention, but the numbered label is source organization, not a callable or
storage ABI. `P` rows are `enclosing_contract_required_not_independent_port`.
For a routine selected for porting, its parameters require review of type and
width, signedness, pointer ownership, mutation, and observable input/output
behavior together with the routine; the `P` number itself does not define a
separate Firestaff interface.

The `firestaff_mapping` field never treats a citation as an implementation
claim. `trace_only:path:line` means the enclosing `Fxxxx` number occurs under
Firestaff `src/`, `include/`, or `tests`; it may be a comment, declaration, or
test reference. `none` means no such exact owner-number citation was found.
Neither result establishes semantic parity. All rows still state the explicit
non-port local/module decision or the enclosing-contract requirement.

## Ledger columns

| Column | Meaning |
| --- | --- |
| `family`, `id`, `symbol` | Original numbered identifier, retaining aliases that share an ID. |
| `redmcsb_source` | First concrete corpus occurrence; for `M`, the first `#define` when present. |
| `semantic_owner` | Nearest enclosing numbered function for `A`/`L`/`P`, or `macro_or_module_definition` for `M`. |
| `classification`, `port_role` | Family classification and its ABI/local/module role. |
| `firestaff_mapping` | Conservative Firestaff traceability, explicitly marked non-equivalence. |
| `disposition` | The required Firestaff decision for the individual row. |

## Method and verification

The source corpus is mounted at
`/Volumes/Extern-disk/firestaff-reference-cache/redmcsb/source/Toolchains/Common/Source`.
Rows were seeded from the complete prior numbered-symbol inventory, then each
symbol was resolved to a concrete corpus location. Macro rows prefer a macro
definition location. Enclosing ownership is derived from ReDMCSB routine
enclosure; Firestaff tracing scans `src/`, `include/`, and `tests/` for the
owner's exact `Fxxxx` number. The method intentionally does not infer a
one-to-one mapping from spelling or comments.

Validation of the TSV gives `A=4`, `L=4,616`, `M=397`, and `P=2,996`, for
`8,013` audited symbols and `8,014` physical lines including the header. No
row has unresolved ReDMCSB source provenance.
