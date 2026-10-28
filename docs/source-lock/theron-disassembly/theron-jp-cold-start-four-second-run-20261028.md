# Japanese Theron cold-start Run hold capture

Date: 2026-10-28

## Inputs and method

- Authentic Japan Rev. 1 Track 02: MD5
  `b7afb338ad31be1025b53f9aff12d73a`.
- Authentic System Card 3.0: MD5
  `ff1a674273fe3540ccef576376407d1d`.
- Instrumented Mednafen binary: SHA-256
  `39493ef2a501124fad2dfba142bdfbf3caaa5d412cc34b853bb18c35bb809e4d`.
- Module: `pce_fast`; all captures cold-started from the authenticated CUE
  with `autoload_state=none`.
- The host keyboard sent the mapped PCE Run button for four real seconds at
  10, 15, and 25 seconds after launch. A separate retry sent it at 40 seconds
  with a longer input-grab delay. Each run used a fresh temporary Mednafen
  profile and separate trace outputs.
- The raw input and transition traces remain in the task-specific TRV2
  `/dev/shm/firestaff-theron-run-hold-menu-20261028` directory. Their SHA-256
  digests are retained below; no media, firmware, save state, or raw trace was
  copied into the repository.

## Observations

| Run time | Host key events | `$E4C5` active-low Run samples (`$37`) | Input transactions | CD IRQs | Non-System-Card reads | `$E009` dispatches | Transition |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| 10 s | 98 | 148 | 49,738 | 0 | 0 | 0 | missing |
| 15 s | 109 | 149 | 52,748 | 0 | 0 | 0 | missing |
| 25 s | 103 | 145 | 52,618 | 0 | 0 | 0 | missing |
| 40 s retry | 98 | 149 | 52,698 | 0 | 0 | 0 | missing |

All four runs prove that Run reached the System Card controller poll at
`$E4C5`. The capture summaries report 63,488 authenticated CD data-port
reads, but zero non-System-Card reads, raw-sector spans, CD IRQ callbacks, or
game-owned `$E009` dispatches. They therefore do not show the game receiving
the button or beginning its Track 02 load.

The traces do not establish whether the title menu was visibly ready when Run
was sent. They cannot be used to reject the documented Run action or to infer
that four seconds is insufficient. A future capture must witness the title
screen and a game-owned input poll or menu handoff before mapping this input
to startup behavior.

## Trace identities

| Run time | Transition SHA-256 | Input SHA-256 |
| --- | --- | --- |
| 10 s | `ec9f175ad65980d5db6dc840bf13a59cc3d59c527b34b3567421044d54a82706` | `ea8df22eed0cbdbdca646d593ad48147e2fe3a35e9e8b0aac6483588f004c37b` |
| 15 s | `9fc413c3348b6863733b0897c8a951f436e5094327bf1fb48f709e7e30b8f0c5` | `b1acfd355eca123d1ab71f7eba8d425e46277962ae17d10db0ac938129c9db61` |
| 25 s | `5ace7d7a3894983ca8d0ce8a05c834446c4f42c15512f1f6c9ed32d61c474a2a` | `e70ca8123e65fe6164d38397df02cb30c428391c17655d709f2149c3687efcea` |
| 40 s retry | `ad8530503e21774a8f866690c98570636bdf15e28c11ee9fd42970e4de4ce6ea` | `992b0ce7800f040cb188008c3a3f15cca27a68cf0093e15e0ba56b17c4c636dd` |
