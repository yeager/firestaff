# JP/US ID `$53` continuation at `$4C71`

The authentic JP Rev. 1 and US Track 02 images contain the same 112-byte
window rooted at logical `$4C71`, immediately after the `$4C6D` call to
`$4B00`. The full media identities are checked by
`tests/test_theron_v1_id53_post_4b00_continuation_source_lock.py`; the candidate
window is at raw offset `$2BF201` in JP and `$2BFB31` in US, with SHA-256
`e656afc5e5dcbd8c22d0763699e586f80785fc83ba17c0cfe94923a6fcf38d86` in both.
It occurs once in each authenticated BIN.

The rooted HuC6280 decode ends exactly at the RTS instruction `$4CE0`. It
loads six bytes from `$4EC3..$4EC8` into `$0E..$13` and `$3004/$3005`,
initializes zero/`$60` fields, sets `$00/$01` to `$47/$34`, calls `$33A1`,
`$3583`, `$3C59`, `$3B86`, `$3C6E`, and `$36FC`, then restores the saved MPR
values from the stack and clears `$5B`.
The attached listing is byte-for-byte verified against both regional spans.

This is static source correspondence only. It does not prove runtime selection
of this Track 02 candidate, bind the logical routine to an executed physical
bank, or assign gameplay/display semantics to the fields or callees. The
separate `$3221` MPR1 provenance question in `TODO-theron.md` remains open.
