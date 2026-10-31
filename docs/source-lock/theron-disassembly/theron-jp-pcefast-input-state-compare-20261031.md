# Theron JP PCE Fast input-conditioned `$2912` state update

## Result

Three bounded neutral-control runs and three bounded UP runs started from the
same authentic JP Akutuba F5 save state and used the same authentic JP Rev. 1
disc and System Card. The UP script was `up@61:120` on slot 0. In all three UP
runs, frame 61 read the direction bank at `$44C1` with raw mask `0010` and
active-low value `3e`. Each UP run then recorded this ordered main-RAM sequence
under the shared `TheronTraceCDSharedEventSequence`:

```text
event 2810: $D334 reads logical $2912 / physical $1F0912 = 00
event 2811: $D349 writes logical $2912 / physical $1F0912 = 10
event 2831: $D334 reads logical $2912 / physical $1F0912 = 10
```

The `$D334` instruction bytes are `CD 12 29 D0 0D EE 20 29`; the `$D349`
store bytes are `8D 12 29 73 0D 29 0F 29`. In all three neutral runs, the
input trace records a frame-61 `$44C1` poll with `raw=0000`, `value=3f`, and
no scripted event or apply. Their bounded RAM traces observed no `$D334` read
or `$D349` write to `$2912`; they recorded 574–576 zero writes from `$D2F2`.
The reusable verifier passed all six manifest-bound trace bundles three
consecutive times.

This is evidence that UP input is associated with a repeatable game-RAM state
update in this authentic saved state. The input trace records the `$44C1` poll
frame, while the RAM read/write traces have their own shared event counter; the
current tooling does not expose a direct shared sequence number on the poll or
its caller. `$2912` semantics, party movement, visible response, game-owned
media provenance, and parity remain unproven. The timeout intentionally ended
each reference run after trace collection.

## Provenance

| Input | Identity |
|---|---|
| JP Rev. 1 CUE | MD5 `85706e7c2f658bc2792511d618dfc7a5` |
| JP Rev. 1 Track 02 | MD5 `b7afb338ad31be1025b53f9aff12d73a` |
| System Card 3.0 | MD5 `ff1a674273fe3540ccef576376407d1d` |
| Akutuba F5 save state | SHA-256 `2cc9938b96640a74db1a5b706113564b5d578d5011daf5f85c588ef1c98d70ee` |
| Instrumented PCE Fast binary | MD5 `09e5cae2a37d7c0734d970d98dc07f40` |

The traces, six run logs, capture manifest, and source checksum output are
retained outside the repository on the external disk in
`firestaff-theron-evidence/theron-state-input-join-20261031-r1/`. The capture
manifest SHA-256 is
`00862f47b67ea4ec2130e2b07b1718f5ad04fbbdbb6534bd91112097a72181a0`; the
separate TRV2 source-checksum output SHA-256 is
`0c0995bff50a8f2b029c394101ebde28f006a6c9ee0ab1783ed0514df0493356`. The
verifier checks every trace/log hash against the manifest and compares the
declared source identities with expected values. It does not re-hash source
media, BIOS, state, or emulator binaries; their remote checksum output is
preserved separately. Trace SHA-256 values:

| Run | Input trace | Main-RAM consumer trace | Main-RAM write trace |
|---|---|---|---|
| UP 1 | `b2d064b640f86ca040ebee73b6cdda808d16620bb4c5f4bad47dfc7ebefc932e` | `fd22159841cd6e4ba1c0e08e0a5674557ca2d1eb8800ffd14a9cbbd1c2e4db0e` | `b2c08504ee8bbba295885301a70517d3038b0c29818add7b5aeaa4a225e13cc6` |
| UP 2 | `d138f1ccda8b51da68eddc10283529919faacef98470730ca8a06622c97620ad` | `b7977e59668f46f71233ffab62271dc8a8ef0b63e9e6b0d881011f33559253f0` | `b2c08504ee8bbba295885301a70517d3038b0c29818add7b5aeaa4a225e13cc6` |
| UP 3 | `47c0f72c3830676f7a36929dabb65637c182dede9b36f64048ec198ceac97b5e` | `c5bbd89d9d7804c04a4204ab05b49c13f03bd0efeef99fbcf60abc37f68dbeb0` | `b2c08504ee8bbba295885301a70517d3038b0c29818add7b5aeaa4a225e13cc6` |
| Neutral 1 | `af5fb44439cfbab397b7e22968d6cf6b9c9e6b4f9c37badf666dfd26d3f8a560` | `c2792cd30c82493bf181e551f183a1d57e40379ee1b69b4c2e4b2915a7a065cb` | `d91cc7f1a4400be0e27a9d2b29f1b73c597fa053a9bec901af6c636bb437e32c` |
| Neutral 2 | `b1afe52cc030012776c543a598d67f860f1a496fb8d4ee70b06b99387f92ba6e` | `c2792cd30c82493bf181e551f183a1d57e40379ee1b69b4c2e4b2915a7a065cb` | `90243af5e0cf456692a8af7b00ff41986072fd96c6a492bba6f996b29cf016f2` |
| Neutral 3 | `91e72bfc9539a5a36cc27b14690080c15fbd79ee6897436f7f949f32d1177efd` | `c2792cd30c82493bf181e551f183a1d57e40379ee1b69b4c2e4b2915a7a065cb` | `2b2e957edb3930210896d5a7bd867e7dceb34c4430e5ac1c92ae11bfb5afb1ad` |

## Reproduction check

Run `python3 scripts/verify_theron_pcefast_input_state_comparison.py
<external-capture-directory>/capture-manifest.json`. It checks artifact hashes,
the CUE/state paths in the logs, zero scripted/physical inputs across all
neutral polls, frame-61 UP or neutral polls, the ordered
`$00` read → `$10` write → `$10` read, and the absence of that consumer path in
neutral controls. The manifest supplies run IDs and artifact grouping; it is
unsigned. The input trace does not include the RAM event counter, so the
verifier cannot establish a direct causal poll-to-write join or gameplay
semantics.
