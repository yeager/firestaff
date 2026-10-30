# Nexus BKR Continue tuple consumer read trace (2026-10-31)

## Capture

This package contains selected output from an authentic Ymir `--debug` Continue replay on TRV2. The replay used the retail Nexus disc, matching Saturn IPL, and the authentic Japanese BKR save. No synthetic game or save data was used. The run log and bundled traces are retained on TRV2 at `/Volumes/Extern-disk/firestaff-evidence/nexus/nexus_bkr_continue_consumer_read_20261031`; copy that directory to an accessible location before running the verifier. The original capture profile was temporary and has been removed, so this repository does not contain enough information to recreate the exact capture.

The read trace watches SH-2 reads of `0x06064544..0x0606454b`. It contains 477 CPU rows and the capture contains 477 register-snapshot rows, with zero dropped master/slave rows. The trace and snapshot files do not include a shared per-read identity, so equal row counts do not establish alignment. The write trace contains 42 rows, also with zero dropped rows. See the bundle files for the complete machine-generated receipts.

## Findings

The prior authentic loader capture established that the BKR loader writes `00 01 40 00 00 14 00 1a` to `0x06064544..0x0606454b`. In this replay, trace rows at `0x0602103c`, `0x06021068`, `0x06021078`, `0x0602107e`, and `0x06021092` carry caller metadata naming `0x06020fee` (`FUN_06020fee`, caller return `0x06011196`) and report addresses within the tuple range. Separately, static decompilation shows `FUN_06020fee` copies fields from its `param_1` structure, including words at offsets `+0`, `+2`, `+4`, and `+6`, into locals passed to an indirect initializer.

The capture is consistent with the tuple being read during the `FUN_06020fee` path, but it does not prove per-instruction register/address pairing or a dynamic loader-to-consumer dataflow. The trace reports addresses and caller metadata, not effective base pointers or loaded values; the snapshot file has no shared per-read identity with the trace. Some other accesses in the same watched range are attributed to rendering/iteration code at `0x060261xx..0x060263xx`, and the capture can include unrelated reads of the same addresses. Treat the call-target metadata as context, not as a proven direct call edge.

The existing source-read verifier remains scoped to the loader source reads and destination writes. This consumer trace is supporting evidence only and does not establish that the gameplay engine applies the loaded position, or that Continue is fully playable.

## Reproduction and integrity

After copying the retained package from TRV2, set its location and run the verifier from the Firestaff repository root. For example:

```sh
export FIRESTAFF_NEXUS_BKR_CONSUMER_EVIDENCE_DIR="/path/to/nexus_bkr_continue_consumer_read_20261031"
python3 scripts/verify_nexus_bkr_consumer_read_trace.py "$FIRESTAFF_NEXUS_BKR_CONSUMER_EVIDENCE_DIR"
```

The verifier checks the trace and receipt counts, required read/write fields, and selected call-context rows. It does not pair register snapshots with reads by row position.

`SHA256SUMS.txt` records hashes for all files in this evidence package except itself.
