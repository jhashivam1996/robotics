#!/usr/bin/env bash
set -euo pipefail

PORT="/dev/ttyUSB0"
BAUDRATE="115200"

usage() {
  cat <<'EOF'
Usage:
  ./arduino-uno-car/monitor.sh [--port <port>] [--baudrate <baudrate>]
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

exec arduino-cli monitor -p "$PORT" -c "baudrate=$BAUDRATE"
