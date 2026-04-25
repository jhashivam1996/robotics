#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$PROJECT_DIR/.." && pwd)"
GENERIC_HELPER="$ROOT_DIR/build-upload-sketch.sh"
ESP32_COMMON_HELPER="$ROOT_DIR/esp32-wrapper-common.sh"

source "$ESP32_COMMON_HELPER"

ESP32_SOUND_DETECTOR_SKETCH_DIR="$PROJECT_DIR"
DEFAULT_ESP32_FQBN="esp32:esp32:esp32doit-devkit-v1"
DEFAULT_ESP32_PORT="/dev/ttyUSB0"
DEFAULT_BOARD_OPTIONS=("UploadSpeed=115200")
MODE="upload"
FQBN="$DEFAULT_ESP32_FQBN"
PORT="$DEFAULT_ESP32_PORT"

usage() {
  cat <<'EOF'
Usage:
  ./esp32-hw484-sound-detector/upload-sketch.sh [--mode <compile|upload>] [--fqbn <fqbn>] [--port <port>]

Behavior:
  - Uses the hardcoded ESP32 HW-484 sound detector sketch path.
  - Default mode is `upload`.
  - `--mode compile` only compiles.
  - `--mode upload` compiles first, then uploads.
  - Default board is `esp32:esp32:esp32doit-devkit-v1`.
  - Default port is `/dev/ttyUSB0`.
  - Default board option is `UploadSpeed=115200` for safer ESP32 uploads.

Preflight checks:
  - Confirms the requested ESP32 board core is installed.
  - Confirms Python `pyserial` is available for the installed ESP32 toolchain.
EOF
}

preflight_check_esp32_core() {
  local core_id
  core_id="$(echo "$FQBN" | cut -d: -f1-2)"

  if ! arduino-cli core list | awk 'NR > 1 { print $1 }' | grep -Fxq "$core_id"; then
    echo "Missing Arduino core for requested FQBN: $FQBN" >&2
    echo "Expected installed core id: $core_id" >&2
    echo >&2
    echo "Installed cores:" >&2
    arduino-cli core list >&2
    echo >&2
    echo "Install the required core, then retry." >&2
    exit 1
  fi
}

preflight_check_pyserial() {
  if ! command -v python3 >/dev/null 2>&1; then
    echo "Missing Python runtime: python3" >&2
    echo "The installed ESP32 board tools require python3 to run esptool." >&2
    exit 1
  fi

  if python3 -c "import serial" >/dev/null 2>&1; then
    return
  fi

  echo "Missing Python module: serial" >&2
  echo "The installed ESP32 board tools require pyserial for compile/upload steps." >&2
  echo >&2
  echo "Common fixes:" >&2
  echo "  Option 1: sudo apt install python3-serial" >&2
  echo "  Option 2: python3 -m pip install --user pyserial" >&2
  exit 1
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --mode)
      MODE="${2:-}"
      shift 2
      ;;
    --fqbn)
      FQBN="${2:-}"
      shift 2
      ;;
    --port)
      PORT="${2:-}"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown argument: $1" >&2
      usage
      exit 1
      ;;
  esac
done

if [[ "$MODE" != "compile" && "$MODE" != "upload" ]]; then
  echo "--mode must be one of: compile, upload" >&2
  usage
  exit 1
fi

preflight_check_esp32_core
preflight_check_pyserial

ARGS=(
  --mode "$MODE"
  --fqbn "$FQBN"
  --sketch "$ESP32_SOUND_DETECTOR_SKETCH_DIR"
)

for option in "${DEFAULT_BOARD_OPTIONS[@]}"; do
  ARGS+=(--board-options "$option")
done

UPLOAD_HELP_PORT=""

if [[ "$MODE" == "upload" ]]; then
  ARGS+=(--port "$PORT")
  UPLOAD_HELP_PORT="$PORT"
fi

esp32_run_upload_helper "$GENERIC_HELPER" "$UPLOAD_HELP_PORT" "${ARGS[@]}"
