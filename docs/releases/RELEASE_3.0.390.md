# Firestaff v3.0.390

## Developer changes

- `DM1 PC34 selector diagnostics`: report whether Quit came from the keyboard or the original selector hitbox when startup detail logging is enabled.
- `DM1 PC34 selector verification`: cover Q, Escape and the original Quit hitbox against authentic PC34 media, ensuring each route exits before dungeon runtime.
- `DM1 PC34 motion diagnostics`: update the timing audit with a real-media profile. SDL dummy output reached about 60 presented frames/s with 15–17% CPU per core; this does not establish GPU usage or physical display latency.
- `Startup menu catalogs`: refresh extracted source references after the selector diagnostics changed source line numbers.
- `Release metadata`: update the project version, embedded changelog and reproducible SPDX source SBOM for v3.0.390.

## Known limitations

- The timing profile used SDL dummy video and audio. Native display scanout, GPU load and the reported MacBook motion lag remain unverified.
- Full DM1, CSB and DM2 visual and gameplay parity across every platform remains open; original-media startup tests do not establish complete parity.
