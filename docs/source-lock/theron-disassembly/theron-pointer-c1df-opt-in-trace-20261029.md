# Theron PCE Fast C1DF trace opt-in verification

## Change

The capture wrapper exposes `THERON_CAPTURE_POINTER_C1DF_TRACE=0|1`, but the
instrumented PCE Fast core previously logged C1DF instruction/register
activity whenever the general mouse trace was enabled. The runtime now reads
`FIRESTAFF_THERON_POINTER_C1DF_TRACE` once and gates the C1DF instruction hook
and its surrounding code-PC/register window behind the requested flag.
Ordinary mouse-device events and watched pointer-RAM accesses remain
independent of this option.

## Build and capture

- The complete Theron Mednafen 1.32.1 patch set applied to an isolated source
  copy on TRV2. A serial `-j1` build completed with PCE Fast enabled.
- The resulting Mednafen binary SHA-256 is
  `9bfe68683cf5181d8002410883ec56f5146de4890a9a7e84b0b6b6377930dc34`.
- Two five-second captures used the same authentic Japanese Rev. 1 Track 02
  (MD5 `b7afb338ad31be1025b53f9aff12d73a`), System Card 3.0 (MD5
  `ff1a674273fe3540ccef576376407d1d`), and Ak-Tu-Ba save state (MD5
  `14dec90b96ec3e14622ec0fab535a92f`). They selected the PCE Fast module and
  differed only in the trace flag.

| Capture | Trace flag | C1DF CPU rows | Mouse trace SHA-256 |
| --- | ---: | ---: | --- |
| Disabled | `0` | `0` | `cdde360335e09c2b70a85f633275263d5e3d3967a47cb4e6de5b47d196757758` |
| Enabled | `1` | `16,199` | `c6930c2a03229e37e6fa3738c0557382f793fb5402501d67fb357e56930f8e20` |

The capture summary agrees with the row counts: `pointer_c1df_trace_requested`
is `0` and `1`, respectively, and `pointer_c1df_trace_rows` is `0` and
`16199`. Three loops of `tests/test_theron_v1_mednafen_live_capture_script.sh`
and the compiled parser/gate regression passed. Redacted Gitleaks found no
leaks in the changed files.

## Limits

Both captures loaded an emulator save state. Their transition receipts say
`transition=missing` and report zero authenticated CD-to-RAM receipts. The
A/B result verifies only that detailed C1DF tracing is opt-in; it does not
establish pointer coordinates, click targets, source-LBA ownership, or native
Theron gameplay parity.
