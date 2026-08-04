#!/usr/bin/env bash
set -euo pipefail

: "${VERSION:?VERSION required}"
: "${BUILD_DIR:?BUILD_DIR required}"
: "${PLIST_SRC:?PLIST_SRC required}"

RELEASE_DIR="${RELEASE_DIR:-release}"
APP_NAME="Firestaff"
STAGING_DIR="$(mktemp -d)"
BUNDLE_DIR="${STAGING_DIR}/Payload/${APP_NAME}.app"

mkdir -p "${BUNDLE_DIR}"
mkdir -p "${RELEASE_DIR}"

cp "${BUILD_DIR}/firestaff" "${BUNDLE_DIR}/firestaff"

sed "s/\${FIRESTAFF_VERSION}/${VERSION}/g" "${PLIST_SRC}" > "${BUNDLE_DIR}/Info.plist"

if [[ -f "${BUILD_DIR}/LaunchScreen.storyboardc" ]]; then
    cp -R "${BUILD_DIR}/LaunchScreen.storyboardc" "${BUNDLE_DIR}/"
fi

if [[ -d "${BUILD_DIR}/AppIcon.xcassets" ]]; then
    cp -R "${BUILD_DIR}/AppIcon.xcassets" "${BUNDLE_DIR}/"
fi

printf 'APPL????' > "${BUNDLE_DIR}/PkgInfo"

codesign --force --deep --sign - "${BUNDLE_DIR}" 2>/dev/null || true

IPA_OUTPUT="${RELEASE_DIR}/${APP_NAME}-${VERSION}-ios-arm64.ipa"
cd "${STAGING_DIR}"
zip -r -9 "${IPA_OUTPUT}" Payload/

rm -rf "${STAGING_DIR}"

echo "Built: ${RELEASE_DIR}/${APP_NAME}-${VERSION}-ios-arm64.ipa"
