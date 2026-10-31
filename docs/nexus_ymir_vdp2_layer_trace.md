# Nexus Ymir VDP2 layer trace analysis

Use `scripts/analyze_nexus_ymir_vdp2_layer_outputs.py` to compare one Ymir
software-renderer layer with the final RGBX image from the same VDP2 pass. The
analyzer requires a complete layer-trace receipt and the matching
`rendered-frames/index.csv` row, so the selected frame and VDP2 serial are
checked together before any pixel comparison.

Capture real Nexus media with a Ymir build that includes
`--trace-vdp2-layer-outputs`:

```sh
ymir-sdl3 --load-state 3 --capture-frames 900 --capture-no-present \
  --dump-memory --debug \
  --trace-vdp2-render-state /capture/render-state.csv \
  --trace-vdp2-rendered-frames /capture/rendered-frames \
  --trace-vdp2-layer-outputs /capture/layer-outputs.bin \
  --trace-vdp2-render-state-frame-range 891:892 \
  --trace-vdp2-render-state-row-range 0:224
```

Compare NBG1/EXBG (layer ID 3) from frame 891, VDP2 serial 1280:

```sh
python3 scripts/analyze_nexus_ymir_vdp2_layer_outputs.py \
  /capture/layer-outputs.bin \
  /capture/layer-outputs.bin.receipt.txt \
  /capture/rendered-frames/vdp1-891-vdp2-1280-320x224.rgbx8888le \
  /capture/rendered-frames/index.csv \
  --frame 891 --serial 1280 --layer 3 --width 320 --height 224 \
  --output-rgba /capture/nexus-frame891-nbg1.rgba \
  --output-json /capture/nexus-frame891-nbg1-comparison.json
```

The report includes input SHA-256 values, exact RGB match counts, unique layer
colors, nonzero priority pixels, and per-character color-calculation flags. The
optional RGBA output is derived from the selected Ymir layer. A single layer is
not represented as the final composed frame; VDP1, other enabled layers, and
composition rules still need to be evaluated.

Keep captures outside Git. The trace is useful only when it comes from the same
authentic game run as its rendered-frame reference.
