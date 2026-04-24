#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$PROJECT_DIR/.." && pwd)"
GENERIC_HELPER="$ROOT_DIR/build-upload-sketch.sh"

CAR_SKETCH_DIR="$PROJECT_DIR"
CAR_FQBN="arduino:avr:uno"
CAR_PORT="/dev/ttyUSB0"

usage() {
  cat <<'EOF'
Usage:
  ./arduino-uno-car/upload-sketch.sh [--mode <compile|upload>]

Behavior:
  - Uses the hardcoded Arduino Uno car sketch path, board type, and USB port.
  - Default mode is `upload`.
  - `--mode compile` only compiles.
  - `--mode upload` compiles first, then uploads.
EOF
}

MODE="upload"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --mode)
      MODE="${2:-}"
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

"$GENERIC_HELPER" \
  --mode "$MODE" \
  --fqbn "$CAR_FQBN" \
  --sketch "$CAR_SKETCH_DIR" \
  --port "$CAR_PORT"
