# Pass 112 — original semantic-route audit

This joins route-shot labels with pass80 raw-frame classifications before original frames are allowed into overlay comparison.

- attempt dir: `verification-screens/pass70-original-dm1-viewports`
- semantic route ready: `false`
- honesty: Semantic route audit only. Passing this gate makes original-vs-Firestaff overlay inputs eligible for pixel comparison; it does not claim pixel parity.

## Problems

- missing shot-label manifest: verification-screens/pass70-original-dm1-viewports/original_viewport_shot_labels.tsv
- missing pass80 classifier JSON: verification-screens/pass70-original-dm1-viewports/pass80_original_frame_classifier.json

## Six-shot semantic checkpoints

| # | route label | expected label | classification | expected class | sha256 |
|---|-------------|----------------|----------------|----------------|--------|
