#!/usr/bin/env bash
set -euo pipefail

: "${VERSION:?VERSION required}"
: "${BUILD_DIR:?BUILD_DIR required}"
: "${ANDROID_SDK_ROOT:?ANDROID_SDK_ROOT required}"
: "${MANIFEST_SRC:?MANIFEST_SRC required}"

RELEASE_DIR="${RELEASE_DIR:-release}"
APP_NAME="Firestaff"
APK_STAGING="${BUILD_DIR}/apk-staging"
AAPT2="${ANDROID_SDK_ROOT}/build-tools/34.0.0/aapt2"
ZIPALIGN="${ANDROID_SDK_ROOT}/build-tools/34.0.0/zipalign"
APKSIGNER="${ANDROID_SDK_ROOT}/build-tools/34.0.0/apksigner"
ANDROID_JAR="${ANDROID_SDK_ROOT}/platforms/android-34/android.jar"

rm -rf "${APK_STAGING}"
mkdir -p "${APK_STAGING}/lib/arm64-v8a"
mkdir -p "${APK_STAGING}/assets"
mkdir -p "${RELEASE_DIR}"

cp "${BUILD_DIR}/libmain.so" "${APK_STAGING}/lib/arm64-v8a/libmain.so"

SDL3_SO=""
for candidate in "${BUILD_DIR}/libSDL3.so" "/tmp/SDL3-android/lib/libSDL3.so"; do
    if [[ -f "$candidate" ]]; then
        SDL3_SO="$candidate"
        break
    fi
done
if [[ -n "$SDL3_SO" ]]; then
    cp "$SDL3_SO" "${APK_STAGING}/lib/arm64-v8a/"
else
    echo "Warning: libSDL3.so not found, APK may not work" >&2
fi

sed "s/\${FIRESTAFF_VERSION}/${VERSION}/g" "${MANIFEST_SRC}" > "${APK_STAGING}/AndroidManifest.xml"

UNALIGNED="${BUILD_DIR}/${APP_NAME}-${VERSION}-unaligned.apk"
ALIGNED="${BUILD_DIR}/${APP_NAME}-${VERSION}-aligned.apk"
FINAL="${RELEASE_DIR}/${APP_NAME}-${VERSION}-android-arm64.apk"

cd "${APK_STAGING}"
zip -r -0 "${UNALIGNED}" . -x '*.DS_Store'
cd -

"${ZIPALIGN}" -f 4 "${UNALIGNED}" "${ALIGNED}"

if [[ -f "${BUILD_DIR}/debug.keystore" ]]; then
    KEYSTORE="${BUILD_DIR}/debug.keystore"
else
    keytool -genkeypair -v \
        -keystore "${BUILD_DIR}/debug.keystore" \
        -alias androiddebugkey \
        -keyalg RSA -keysize 2048 -validity 10000 \
        -storepass android -keypass android \
        -dname "CN=Firestaff Debug"
    KEYSTORE="${BUILD_DIR}/debug.keystore"
fi

"${APKSIGNER}" sign \
    --ks "${KEYSTORE}" \
    --ks-key-alias androiddebugkey \
    --ks-pass pass:android \
    --key-pass pass:android \
    --min-sdk-version 24 \
    --out "${FINAL}" \
    "${ALIGNED}"

echo "Built: ${FINAL}"
