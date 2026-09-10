# Dungeon Master PC 3.4: Hall of Champions interaction capture

These frames are direct 320×200 host captures from the original English DOS
PC 3.4 executable, running under DOSBox-X.  They are reference material, not
Firestaff screenshots and not reconstructed artwork.

The route enters the Hall of Champions, reaches and resurrects Chani Sayyadina
Sihaya, then uses the original secondary-click champion-panel route.  The
first frame shows the original inventory panel.  The second holds the primary
button over the original Eye control, showing the statistics panel.  The third
shows the panel after releasing that control.

| File | Original state | SHA-256 |
| --- | --- | --- |
| `01_hoc_chani_inventory_original.png` | Chani inventory panel open | `505d66e5b6dcb9629a8d0ec178b69415a601478acbcc77965d78e7c2d637aba7` |
| `02_hoc_chani_eye_statistics_held_original.png` | Eye control held; statistics shown | `2a2b9db8d5081e250df1659ff1f9dde653c7f926fc307ca06184b164009fae1a` |
| `03_hoc_chani_eye_statistics_released_original.png` | Eye control released; inventory restored | `6c4a8dc225a5a37eaed8ddf126e9070028aaf409bb257754dfa68431c4d53128` |

The capture input uses `scripts/dosbox_dm1_original_viewport_reference_capture.sh`.
Its route metadata is intentionally not promoted here: it contains machine
execution details and is not needed to identify the visual reference.

This closes only the authentic HoC inventory and Eye-statistics reference
portion of the DM1 capture requirement.  An original scroll-in-hand/Eye
description capture and an original floor-to-hand pickup capture remain open.
