#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$PROJECT_DIR/.." && pwd)"
ESP32_COMMON_HELPER="$ROOT_DIR/esp32-wrapper-common.sh"

source "$ESP32_COMMON_HELPER"

PORT="/dev/ttyUSB0"
BAUDRATE="115200"

usage() {
  cat <<'EOF'
Usage:
  ./esp32-ov7670-camera-phase3/monitor.sh [--port <port>] [--baudrate <baudrate>]
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --port)
      PORT="${2:-}"
      shift 2
      ;;
    --baudrate)
      BAUDRATE="${2:-}"
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

esp32_run_monitor "$PORT" "$BAUDRATE"
