# Windows Build Verification - 2026-05-20

Scope: close the Platform Windows build TODO item with GitHub Actions release evidence. This records a hosted Windows/MSYS2 build, smoke probe, ZIP package, NSIS installer, checksum, and publish path. It does not claim a local N2 Windows cross-build.

## Evidence

- Release tag: `v2.4.1`
- Release commit: `2207e528ef6a7582d36a74642f3cafefefb94c30`
- Release workflow run: `26177856511`
- Workflow URL: https://github.com/yeager/firestaff/actions/runs/26177856511
- Release URL: https://github.com/yeager/firestaff/releases/tag/v2.4.1

GitHub Actions reported these release jobs successful:

- `resolve-version`
- `macOS Universal (arm64+x86_64)`
- `Windows x86_64`
- `Linux x86_64`
- `Linux ARM64 (Steam Deck)`
- `Publish GitHub Release`

The published `v2.4.1` release contains the expected Windows artifacts:

- `Firestaff-2.4.1-windows.zip`
- `Firestaff-2.4.1-windows-installer.exe`
- `Firestaff-2.4.1-windows.sha256`

## Source path locked by workflow

The release workflow Windows lane runs on `windows-2022` with MSYS2/UCRT64, installs GCC/CMake/Ninja/SDL3/NSIS, configures CMake with Ninja, builds the project, runs the headless M11 smoke probes, packages the portable ZIP, packages the NSIS installer, writes Windows checksums, uploads artifacts, and publishes them through the release job.

## Verification commands

```sh
gh run view 26177856511 --repo yeager/firestaff --json status,conclusion,headSha,url,jobs

gh release view v2.4.1 --repo yeager/firestaff --json tagName,name,isDraft,isPrerelease,publishedAt,url,assets
```

Both checks were run from main review before this TODO update. The workflow conclusion was `success`, the Windows job conclusion was `success`, and the release assets listed the Windows ZIP, NSIS installer, and SHA256 file.

## Remaining scope

The TODO item is closed for Windows build verification. Future Windows work should be tracked as separate runtime/installer QA if needed, not as the baseline Platform Windows build item.
