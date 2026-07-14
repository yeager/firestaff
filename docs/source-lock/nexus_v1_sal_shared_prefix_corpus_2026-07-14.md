# Nexus V1 SAL Shared-Prefix Corpus Receipt, 2026-07-14

The local retail corpus at `/Users/bosse/.firestaff/data/nexus` contains all
sixteen canonical `SNDLEV00.SAL` through `SNDLEV15.SAL` banks. Their expected
per-level sizes come from the existing verified audio receipt table.

`nexus_v1_audio_sal_shared_prefix_receipt()` compares every byte at each
offset across all sixteen supplied banks. The corpus probe observes an exact
shared prefix of `0x45bb5` bytes (`0x000000..0x045bb4`) and records the first
divergence at `0x045bb5`.

This is an equality receipt only. It does not label the common bytes as a
header, sample table, DSP program, codec state, or playback data. It cannot
enable SAL decoding, event dispatch, SCSP/VDP behavior, or host playback.
