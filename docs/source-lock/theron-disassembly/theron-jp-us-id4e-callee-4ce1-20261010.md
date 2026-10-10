# JP/US ID `$4E` callee at `$4CE1`

The existing ID `$4E` caller listing includes a `JSR $4CE1`; this rooted
HuC6280 listing extends that evidence through `$4D0C` and includes the shared
conditional-error destination `$4D0D`. The 45-byte span is identical in the
authentic JP Rev. 1 and US Track 02 images at offsets `$2BF271` and `$2BFBA1`,
with SHA-256
`e1479fdfa979233393e3454645708d6f344192fce6f69aa7ea2c3c21e4cf458c`.
The real-media test checks both complete image identities, unique span
occurrence, the exact branch targets, the RTS/BRK boundary, and byte-for-byte
listing correspondence.

Statically, the callee copies `$4D7B` to `$4EC1`, calls `$4EC9`, and branches to
`$4D0D` on carry. On the continuing path it calls `$4F5E` with `A=$06`, branches
to the same BRK on carry, decrements `$5B`, copies three values into
`$37CC/$37D0/$37D1`, calls `$3876`, clears `$5B`, and returns. This does not
establish the meaning of those fields or the behavior of the three external
callees, nor runtime source-bank selection or ID `$4E` execution.
