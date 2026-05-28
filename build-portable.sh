#!/bin/bash
# Build release project first to /build

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
TARGET_DIR="$SCRIPT_DIR/MagicPods-portable"
BUILD_DIR="$SCRIPT_DIR/build/Desktop_Qt_6_10_2-Release"
QMAKE="$HOME/Qt/6.10.2/gcc_64/bin/qmake"
QT_PLUGINS_DIR="$HOME/Qt/6.10.2/gcc_64/plugins"
DOCKER_OUT="$(mktemp -d "${TMPDIR:-/tmp}/magicpodscore-runtime.XXXXXX")"
OWNER_USER="$(id -un)"
OWNER_GROUP="$(id -gn)"

cleanup() {
  rm -rf "$DOCKER_OUT" 2>/dev/null || sudo rm -rf "$DOCKER_OUT"
}
trap cleanup EXIT

if [[ "$(id -u)" -eq 0 ]]; then
  echo "Run ./build-portable.sh without sudo. The script will request sudo only when needed." >&2
  exit 1
fi

echo "==> Removing old $TARGET_DIR"
rm -rf "$TARGET_DIR"

echo "==> Building portable frontend bundle"
cqtdeployer \
  -bin "$BUILD_DIR/magicpods" \
  -targetDir "$TARGET_DIR" \
  -qmake "$QMAKE" \
  -qmlDir "$SCRIPT_DIR/src/app/qml" \
  -extraLibs libxcb-cursor

cp "$QT_PLUGINS_DIR/platforms/libqwayland.so" "$TARGET_DIR/plugins/platforms/"
sed -i '/^export CQT_RUN_FILE=/a export MAGICPODSCORE_LIBDIR="$BASE_DIR"/bin/modules/lib' "$TARGET_DIR/magicpods.sh"

echo "==> Requesting sudo for backend runtime steps"
sudo -v

echo "==> Building MagicPodsCore runtime with Docker"
sudo docker buildx build \
  -f "$SCRIPT_DIR/Dockerfile.magicpodscore" \
  --target runtime_export_stage \
  -o type=local,dest="$DOCKER_OUT" \
  "$SCRIPT_DIR"

echo "==> Copying MagicPodsCore runtime into $TARGET_DIR"
mkdir -p "$TARGET_DIR/bin/modules" "$TARGET_DIR/bin/modules/lib"

if [[ ! -f "$DOCKER_OUT/bin/modules/magicpodscore" ]]; then
  echo "ERROR: $DOCKER_OUT/bin/modules/magicpodscore not found" >&2
  exit 1
fi

sudo cp -f "$DOCKER_OUT/bin/modules/magicpodscore" "$TARGET_DIR/bin/modules/magicpodscore"

if [[ -d "$DOCKER_OUT/lib" ]]; then
  sudo cp -a "$DOCKER_OUT/lib/." "$TARGET_DIR/bin/modules/lib/"
fi

echo "==> Fixing ownership for $TARGET_DIR ($OWNER_USER:$OWNER_GROUP)"
sudo chown -R "$OWNER_USER:$OWNER_GROUP" "$TARGET_DIR"

echo "==> Done: $TARGET_DIR"
