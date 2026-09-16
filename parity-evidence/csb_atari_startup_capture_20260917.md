# CSB Atari ST original startup capture receipt

## Scope

This receipt records a private, emulator-owned original startup capture. It
contains no game image, firmware, screenshot, audio bytes, or local path. It
is not a Firestaff pixel-parity or gameplay claim.

## Source and session

- TOS 1.62 SHA-256:
  `220fc9b35fd99908db9f9075fb3d850bf196d25741405ac6fa062facbbbd1583`.
- CSB STX SHA-256:
  `d9aed23f7916d60dfef61c7b79bc3eb1995f8afbb6a6c8b7b4160ee12ada1025`.
- Hatari 2.4.1 ran a write-protected STE session with audio synchronisation
  enabled at 44100 Hz and a 100 ms host buffer.

## Private artifacts

The emulator wrote two PNG screenshots at 18 and 36 seconds after boot:

```text
18s  aa5db159cfa011d07f665b5f6e44d2fa9a8b759c4a950509c317f0f9be6caee5
36s  7dd601bbdfaba1f2278a9cde6ba47a0d4c6f208baa1b1529538212098515e53d
```

The same session recorded a private WAV stream:

```text
sha256 57eea0f0d79eafd4e0a54d19627098fdfd8835c913ce4336995c9c405cca3911
bytes  6374824
```

The capture tool reported a non-silent signal and zero Hatari emulation-sample
warnings. The screenshots and WAV remain private. They establish only a
reproducible original startup/audio capture channel; HUD, dungeon viewport,
door, DSA and gameplay parity still require their own authenticated routes.
