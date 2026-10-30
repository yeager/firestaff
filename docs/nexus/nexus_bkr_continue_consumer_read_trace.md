# Nexus BKR Continue tuple consumer read trace (2026-10-31)

## Capture

This package contains selected output from an authentic Ymir `--debug` Continue replay on TRV2. The replay used the retail Nexus disc, matching Saturn IPL, and the authentic Japanese BKR save. No synthetic game or save data was used. The run log and bundled traces are retained here; the original capture profile was temporary and has been removed after copying these files.

The read trace watches SH-2 reads of `0x06064544..0x0606454b`. It contains 477 CPU rows and 477 aligned register snapshots, with zero dropped master/slave rows. The write trace contains 42 rows, also with zero dropped rows. See the bundle files for the complete machine-generated receipts.

## Findings

The prior authentic loader capture established that the BKR loader writes `00 01 40 00 00 14 00 1a` to `0x06064544..0x0606454b`. In this replay, read instructions at `0x0602103c`, `0x06021068`, `0x06021078`, `0x0602107e`, and `0x06021092` are attributed by the trace to a call whose target metadata is `0x06020fee` (`FUN_06020fee`, caller return `0x06011196`). The corresponding static decompilation copies fields from its `param_1` structure, including the words at offsets `+0`, `+4`, and `+6`, into locals passed to an indirect initializer.

This is consistent with the tuple being consumed during the `FUN_06020fee` initialization path. It does not yet prove a dynamic loader-to-consumer dataflow: Ymir's trace records the memory address accessed and call-context metadata, but not the effective base pointer for each individual instruction or the value read. Some other accesses in the same watched range are attributed to rendering/iteration code at `0x060261xx..0x060263xx`, and the capture can include unrelated reads of the same addresses. Treat the call-target metadata as context, not as a proven direct call edge.

The existing source-read verifier remains scoped to the loader source reads and destination writes. This consumer trace is supporting evidence only and does not establish that the gameplay engine applies the loaded position, or that Continue is fully playable.

## Reproduction and integrity

From the Firestaff repository root, validate the trace structure and the subset of rows associated with the `FUN_06020fee` call context:

```sh
python3 scripts/verify_nexus_bkr_consumer_read_trace.py /Volumes/Extern-disk/firestaff-evidence/nexus/nexus_bkr_continue_consumer_read_20261031
```

`SHA256SUMS.txt` records hashes for all files in this evidence package except itself.
