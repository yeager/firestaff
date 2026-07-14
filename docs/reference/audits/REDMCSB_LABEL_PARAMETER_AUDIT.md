# ReDMCSB label and parameter audit: partition 3

This partition audits every numbered ReDMCSB `Axxx`, `Lxxxx`, `Mxxx`, and
`Pxxxx` symbol from the 2021-02-06 WIP source corpus: **8,013 symbols** in
total.  These identifiers are not an additional callable-function backlog.
Their implementation meaning is enclosed by a function contract, a macro, or
a platform/module boundary.

The machine-readable inventory is
[`REDMCSB_LABEL_PARAMETER_AUDIT.tsv`](REDMCSB_LABEL_PARAMETER_AUDIT.tsv).
It contains one row for every source symbol and is the normative record for
this partition.

## Disposition

| Family | Rows | Ownership | Status | Porting decision |
| --- | ---: | --- | --- | --- |
| `Axxx` | 4 | Auxiliary aliases inside the named routine. | `no_standalone_port` | Preserve only through the enclosing routine's algorithm. |
| `Lxxxx` | 4,616 | Function-local storage. | `no_standalone_port` | Do not create a global, API, or standalone port for a local temporary. |
| `Mxxx` | 397 | Macro/module name. | `no_standalone_port` | Port the required expression, layout accessor, or module semantic where used; never treat the macro label as a callable target. |
| `Pxxxx` | 2,996 | Function parameter. | `runtime_abi_surface` | Preserve the value, type/width, signedness, pointer/ownership, and observable input/output contract at the enclosing routine boundary. |

`Pxxxx` is the only family in this partition that is a runtime-relevant ABI
surface.  That does not make each parameter a standalone implementation unit:
the port target is the enclosing function's complete contract.  `Axxx` and
`Lxxxx` are runtime values during execution, but neither is an externally
addressable storage or ABI surface.  `Mxxx` can be semantically important,
including layout and bitfield access, but its numbered label is source-level
macro organisation rather than an ABI.

## Ownership rule

For `A`, `L`, and `P` rows, `semantic_owner` is the nearest enclosing numbered
`Fxxxx` routine in the source file.  This provides the review boundary: inspect
the function's inputs, state reads/writes, return/result effects, and tests as
one unit.  It is deliberately not a claim that a matching Firestaff C symbol
already exists or that the routine is parity-verified.

112 rows use `platform_or_unnumbered_file_scope` instead: 39 locals and 73
parameters in platform/utility code whose enclosing routine has no numbered
`Fxxxx` identifier.  They remain correctly classified (`L` has no standalone
port; `P` remains a runtime ABI surface) without inventing an owner.  All
`Mxxx` rows use `macro_or_module_definition` because their semantic ownership
is the macro/module definition and its use sites, not one enclosing function.

## Inventory fields

| Field | Meaning |
| --- | --- |
| `family`, `id`, `symbol` | The original numbered source identifier. IDs may repeat where an alias family intentionally shares a slot. |
| `redmcsb_files` | Semicolon-separated original source files containing the symbol. |
| `semantic_owner` | Enclosing numbered routine, macro/module ownership, or the explicit platform/unnumbered fallback described above. |
| `classification` | `auxiliary_alias`, `function_local_storage`, `macro_or_module_name`, or `function_parameter`. |
| `status` | The standalone-port/ABI decision from the table above. |

## Provenance and method

The corpus is `Toolchains/Common/Source` from the local
`ReDMCSB_WIP20210206.7z` archive (SHA-256
`58de16a34476a1ac2349ca01a4768cdecbadc9929fed0b69e8b9698f996159d1`).
The inventory was formed from unique identifiers matching
`[ALMP][0-9]{3,4}_...`, preserving their source-file occurrences, then audited
by source enclosure.  The count reconciles exactly to the historical complete
numbered-symbol census: `4 + 4,616 + 397 + 2,996 = 8,013`.

This audit is an ownership and port-shape decision, not a completion claim.
Changing a `runtime_abi_surface` row to a verified implementation requires an
evidence-backed review of its enclosing routine and a focused behavioral test.
