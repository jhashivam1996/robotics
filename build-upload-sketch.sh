#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage:
  ./build-upload-sketch.sh --mode <compile|upload> --fqbn <fqbn> --sketch <path> [--port <port>] [--board-options <key=value[,key=value...]>] [--upload-property <key=value>]

Examples:
  ./build-upload-sketch.sh --mode compile --fqbn arduino:avr:uno --sketch ./arduino-uno-car
  ./build-upload-sketch.sh --mode upload --fqbn arduino:avr:uno --sketch ./arduino-uno-car --port /dev/ttyACM0

Notes:
  - Requires arduino-cli to be installed and on PATH.
  - `--mode upload` compiles first, then uploads.
  - `--port` is required only for upload mode.
  - `--board-options` can be repeated to pass Arduino board menu selections such as `UploadSpeed=115200`.
  - `--upload-property` can be repeated to override upload-only properties such as `upload.speed=115200`.
EOF
}

require_arduino_cli() {
  if ! command -v arduino-cli >/dev/null 2>&1; then
    echo "arduino-cli is not installed or not on PATH." >&2
    echo >&2
    echo "Install it with:" >&2
    echo "  curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh" >&2
    echo >&2
    echo "That usually downloads an 'arduino-cli' binary into your current directory." >&2
    echo "Then choose one of these options to put it on your PATH." >&2
    echo >&2
    echo "Option 1: system-wide install" >&2
    echo "  sudo mv arduino-cli /usr/local/bin/" >&2
    echo >&2
    echo "Option 2: user-only install without sudo" >&2
    echo "  mkdir -p ~/.local/bin" >&2
    echo "  mv arduino-cli ~/.local/bin/" >&2
    echo "  export PATH=\"\$HOME/.local/bin:\$PATH\"" >&2
    echo >&2
    echo "If you choose option 2 and want the PATH change to persist, add this line to ~/.bashrc:" >&2
    echo "  export PATH=\"\$HOME/.local/bin:\$PATH\"" >&2
    echo >&2
    echo "After that, verify with:" >&2
    echo "  arduino-cli version" >&2
    exit 1
  fi
}

MODE=""
FQBN=""
SKETCH_DIR=""
PORT=""
BOARD_OPTIONS=()
UPLOAD_PROPERTIES=()

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
    --sketch)
      SKETCH_DIR="${2:-}"
      shift 2
      ;;
    --port)
      PORT="${2:-}"
      shift 2
      ;;
    --board-options)
      BOARD_OPTIONS+=("${2:-}")
      shift 2
      ;;
    --upload-property)
      UPLOAD_PROPERTIES+=("${2:-}")
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

if [[ -z "$FQBN" ]]; then
  echo "--fqbn is required." >&2
  usage
  exit 1
fi

if [[ -z "$SKETCH_DIR" ]]; then
  echo "--sketch is required." >&2
  usage
  exit 1
fi

if [[ ! -d "$SKETCH_DIR" ]]; then
  echo "Sketch directory not found: $SKETCH_DIR" >&2
  exit 1
fi

if [[ "$MODE" == "upload" && -z "$PORT" ]]; then
  echo "--port is required for upload mode." >&2
  usage
  exit 1
fi

require_arduino_cli

COMPILE_ARGS=(
  --fqbn "$FQBN"
)

for option in "${BOARD_OPTIONS[@]}"; do
  COMPILE_ARGS+=(--board-options "$option")
done

arduino-cli compile "${COMPILE_ARGS[@]}" "$SKETCH_DIR"

if [[ "$MODE" == "upload" ]]; then
  UPLOAD_ARGS=(
    --fqbn "$FQBN"
    --port "$PORT"
  )

  for option in "${BOARD_OPTIONS[@]}"; do
    UPLOAD_ARGS+=(--board-options "$option")
  done

  for property in "${UPLOAD_PROPERTIES[@]}"; do
    UPLOAD_ARGS+=(--upload-property "$property")
  done

  arduino-cli upload "${UPLOAD_ARGS[@]}" "$SKETCH_DIR"
fi
