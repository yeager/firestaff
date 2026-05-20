#!/usr/bin/env bash
set -u

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
BUILD_DIR=${BUILD_DIR:-"$ROOT_DIR/build/web-wasm-probe"}
GENERATOR=${GENERATOR:-Ninja}

missing=0
for tool in emcmake emcc cmake; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "WEB_WASM_TOOLCHAIN_PROBE: missing $tool"
        missing=1
    else
        printf 'WEB_WASM_TOOLCHAIN_PROBE: %s=%s\n' "$tool" "$(command -v "$tool")"
    fi
done

if [ "$missing" -ne 0 ]; then
    echo "WEB_WASM_TOOLCHAIN_PROBE: SKIP - Emscripten toolchain is not available; no Web/WASM build claim is made."
    exit 0
fi

mkdir -p "$BUILD_DIR"
cd "$ROOT_DIR"

echo "WEB_WASM_TOOLCHAIN_PROBE: configure BUILD_DIR=$BUILD_DIR"
emcmake cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -G "$GENERATOR" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF

echo "WEB_WASM_TOOLCHAIN_PROBE: build smoke targets"
cmake --build "$BUILD_DIR" --parallel 2 --target firestaff_m11_audio_probe firestaff_m11_phase_a_probe firestaff

echo "WEB_WASM_TOOLCHAIN_PROBE: PASS - Web/WASM configure and smoke-target build completed. Browser/runtime validation is still required before closing the Platform TODO."
