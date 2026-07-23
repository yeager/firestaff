# CSB F0466-F0485 Graphics/Memory Source Audit

Authority: ReDMCSB PC34 `EXPAND.C` and `MEMORY.C`. This batch binds only an
already hash-admitted original graphics cache to an identically proven startup
package. It never replaces the source memory/cache or bitmap algorithms.

| Range | ReDMCSB owner | Evidence | Firestaff disposition |
| --- | --- | --- | --- |
| F0466 | `EXPAND.C` | compressed graphic to planar bitmap expansion | Exact `CSBgraphics.dat` source/cache identity is required; expansion remains owner-only. |
| F0467 | `MEMORY.C` | indexed compressed-data offset | Parsed cache index and bounded entry span are required; no offset is recalculated. |
| F0468-F0470 | `MEMORY.C` | heap/GEM allocation and release | Original memory ownership is unproven in the host; no allocation or release occurs. |
| F0471-F0473, F0480-F0483, F0485 | `MEMORY.C` | cache list, sort, release, defragment, allocation and usage reset | Original cache ownership is unproven; no cache state changes occur. |
| F0474, F0477-F0479, F0484 | `MEMORY.C` | graphics file read/open/close/header/load | Hash-admitted cache, exact path/MD5, parsed header and bounded selected entry are required. No file-handle or graphic-load action occurs. |
| F0475-F0476 | `MEMORY.C` | memory and graphics-memory initialization | Host allocation initialization remains owner-only. |

Any absent cache, unparsed header, untrusted path/MD5, package-source drift, or
out-of-range entry rejects before an owner can consume the receipt.
