# pass306 DM1 wall pixel-region graphics bridge

Bridge from pass300 comparator-ready wall pixelRegion descriptors to pass305 decoded/checksummed GRAPHICS.DAT wall records. This is metadata-only; no bitmap dumps and no pixel parity claim.

- status: `passed`
- bridged events: `51` / `51`
- required indices covered: `15` / `15`
- actual render-event indices: `[93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107]`

| snapshot | row | wall index | viewport rect | source clip | decoded dimensions | unpacked sha256 |
|---|---|---:|---|---|---|---|
| start_south | D3R2 | 103 | 180,25 44x49 | 0,0 44x49 | 44x49 | `9583ee58a22f26efb026b5ad65bdd804603201843973c5f696680bd836b9a0bd` |
| start_south | D3L | 106 | 7,25 83x49 | 0,0 83x49 | 83x49 | `1b9e4fb3811d692cba8602bad4efb0e9c7c4d5dc58ddd30e65e2e83b515181c7` |
| start_south | D3C | 107 | 77,25 70x49 | 0,0 70x49 | 70x49 | `2ec0205368dcc4d046ed829c6e435730b1f1e8b21773fe12953a2c9e3ca19578` |
| start_south | D2R2 | 98 | 216,24 8x52 | 0,0 8x52 | 8x52 | `4915e30ac285629899b44d33004db77480dc0a2992d87d38689a4022739b7e6a` |
| start_south | D2L | 101 | 0,19 78x74 | 0,0 78x74 | 78x74 | `0616c704688d22a286a31d5e1c8056c7e3f82bb571399ae2103988e36e728520` |
| start_south | D2R | 100 | 146,19 78x74 | 0,0 78x74 | 78x74 | `a8a05e342a5a651b291c4b5a60e819dc3c202bffebea9830aa85be2df5c784bb` |
| start_south | D1C | 97 | 32,9 160x111 | 0,0 160x111 | 160x111 | `21090a186ba9545307b356b0693c17ea2e5165197e6e0d56f32528f754a8bfc2` |
| start_south | D0L | 94 | 0,0 33x136 | 0,0 33x136 | 33x136 | `9b73c633c61d387f4453a3449c3cb8df6d84115707a20f52e5267a6ef064ff31` |
| turn_right_west | D3L2 | 104 | 0,25 44x49 | 0,0 44x49 | 44x49 | `0f7cf7feed751a0916f4dba0306f64ef80dbdfea3c7b336c84d88d6a47f05b4d` |
| turn_right_west | D3R2 | 103 | 180,25 44x49 | 0,0 44x49 | 44x49 | `9583ee58a22f26efb026b5ad65bdd804603201843973c5f696680bd836b9a0bd` |
| turn_right_west | D3L | 106 | 7,25 83x49 | 0,0 83x49 | 83x49 | `1b9e4fb3811d692cba8602bad4efb0e9c7c4d5dc58ddd30e65e2e83b515181c7` |
| turn_right_west | D3R | 105 | 134,25 83x49 | 0,0 83x49 | 83x49 | `292692e6f2bd4881f0be1b4c9bc2213b24280930fe92dba56ec10ed28df95ca3` |
| turn_right_west | D3C | 107 | 77,25 70x49 | 0,0 70x49 | 70x49 | `2ec0205368dcc4d046ed829c6e435730b1f1e8b21773fe12953a2c9e3ca19578` |
| turn_right_west | D2L2 | 99 | 0,24 8x52 | 0,0 8x52 | 8x52 | `984794cba13ce7e7d89fb3a22614862a5eb0627400cc0ed5901b5909406b2fa6` |
| turn_right_west | D2R2 | 98 | 216,24 8x52 | 0,0 8x52 | 8x52 | `4915e30ac285629899b44d33004db77480dc0a2992d87d38689a4022739b7e6a` |
| turn_right_west | D2L | 101 | 0,19 78x74 | 0,0 78x74 | 78x74 | `0616c704688d22a286a31d5e1c8056c7e3f82bb571399ae2103988e36e728520` |
| turn_right_west | D2R | 100 | 146,19 78x74 | 0,0 78x74 | 78x74 | `a8a05e342a5a651b291c4b5a60e819dc3c202bffebea9830aa85be2df5c784bb` |
| turn_right_west | D2C | 102 | 59,19 106x74 | 0,0 106x74 | 106x74 | `a0e5832c7b1dba6ca4c2f2aa38bb73725cde3be5789dab6220edc6f7abaee267` |
| turn_right_west | D1R | 95 | 164,9 60x111 | 0,0 60x111 | 60x111 | `3312d9dcb1749f37805c9d3328864eeddedc1377b22169b0f65355d913d4787c` |
| turn_right_west | D0L | 94 | 0,0 33x136 | 0,0 33x136 | 33x136 | `9b73c633c61d387f4453a3449c3cb8df6d84115707a20f52e5267a6ef064ff31` |
| turn_right_west | D0R | 93 | 191,0 33x136 | 0,0 33x136 | 33x136 | `310d84187894af6ad39550d547c28e171f809b731d59d133ca47061d99b48635` |
| move_forward_west | D3L2 | 104 | 0,25 44x49 | 0,0 44x49 | 44x49 | `0f7cf7feed751a0916f4dba0306f64ef80dbdfea3c7b336c84d88d6a47f05b4d` |
| move_forward_west | D3R2 | 103 | 180,25 44x49 | 0,0 44x49 | 44x49 | `9583ee58a22f26efb026b5ad65bdd804603201843973c5f696680bd836b9a0bd` |
| move_forward_west | D3L | 106 | 7,25 83x49 | 0,0 83x49 | 83x49 | `1b9e4fb3811d692cba8602bad4efb0e9c7c4d5dc58ddd30e65e2e83b515181c7` |
| move_forward_west | D3R | 105 | 134,25 83x49 | 0,0 83x49 | 83x49 | `292692e6f2bd4881f0be1b4c9bc2213b24280930fe92dba56ec10ed28df95ca3` |
| move_forward_west | D3C | 107 | 77,25 70x49 | 0,0 70x49 | 70x49 | `2ec0205368dcc4d046ed829c6e435730b1f1e8b21773fe12953a2c9e3ca19578` |
| move_forward_west | D2L2 | 99 | 0,24 8x52 | 0,0 8x52 | 8x52 | `984794cba13ce7e7d89fb3a22614862a5eb0627400cc0ed5901b5909406b2fa6` |
| move_forward_west | D2R2 | 98 | 216,24 8x52 | 0,0 8x52 | 8x52 | `4915e30ac285629899b44d33004db77480dc0a2992d87d38689a4022739b7e6a` |
| move_forward_west | D2L | 101 | 0,19 78x74 | 0,0 78x74 | 78x74 | `0616c704688d22a286a31d5e1c8056c7e3f82bb571399ae2103988e36e728520` |
| move_forward_west | D2R | 100 | 146,19 78x74 | 0,0 78x74 | 78x74 | `a8a05e342a5a651b291c4b5a60e819dc3c202bffebea9830aa85be2df5c784bb` |
| move_forward_west | D2C | 102 | 59,19 106x74 | 0,0 106x74 | 106x74 | `a0e5832c7b1dba6ca4c2f2aa38bb73725cde3be5789dab6220edc6f7abaee267` |
| move_forward_west | D1L | 96 | 0,9 60x111 | 0,0 60x111 | 60x111 | `688cf9032dad24570c2ce7a8480b14ee1e232e9b98c3a1f8e193549bae419d49` |
| move_forward_west | D1R | 95 | 164,9 60x111 | 0,0 60x111 | 60x111 | `3312d9dcb1749f37805c9d3328864eeddedc1377b22169b0f65355d913d4787c` |
| move_forward_west | D1C | 97 | 32,9 160x111 | 0,0 160x111 | 160x111 | `21090a186ba9545307b356b0693c17ea2e5165197e6e0d56f32528f754a8bfc2` |
| move_forward_west | D0R | 93 | 191,0 33x136 | 0,0 33x136 | 33x136 | `310d84187894af6ad39550d547c28e171f809b731d59d133ca47061d99b48635` |
| turn_left_east | D3L2 | 104 | 0,25 44x49 | 0,0 44x49 | 44x49 | `0f7cf7feed751a0916f4dba0306f64ef80dbdfea3c7b336c84d88d6a47f05b4d` |
| turn_left_east | D3R2 | 103 | 180,25 44x49 | 0,0 44x49 | 44x49 | `9583ee58a22f26efb026b5ad65bdd804603201843973c5f696680bd836b9a0bd` |
| turn_left_east | D2L | 101 | 0,19 78x74 | 0,0 78x74 | 78x74 | `0616c704688d22a286a31d5e1c8056c7e3f82bb571399ae2103988e36e728520` |
| turn_left_east | D2R | 100 | 146,19 78x74 | 0,0 78x74 | 78x74 | `a8a05e342a5a651b291c4b5a60e819dc3c202bffebea9830aa85be2df5c784bb` |
| turn_left_east | D2C | 102 | 59,19 106x74 | 0,0 106x74 | 106x74 | `a0e5832c7b1dba6ca4c2f2aa38bb73725cde3be5789dab6220edc6f7abaee267` |

Remaining blockers:
- original PC34 viewport capture remains route-unproven for pass304 required states
- metadata bridge does not yet perform per-pixel blit/crop comparison against original captures
- D4 rows remain object-stack/content events only and are intentionally excluded from wall bitmap comparison
