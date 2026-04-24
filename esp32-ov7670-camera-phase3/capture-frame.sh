#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
UPLOAD_SCRIPT="$PROJECT_DIR/upload-sketch.sh"
EXTRACT_SCRIPT="$PROJECT_DIR/extract-pgm.sh"

PORT="/dev/ttyUSB0"
BAUDRATE="115200"
RAW_LOG="$PROJECT_DIR/frame.log"
PGM_FILE="$PROJECT_DIR/frame.pgm"
PNG_FILE="$PROJECT_DIR/frame.png"
TIMEOUT_SECONDS="90"
SKIP_UPLOAD="0"
NO_OPEN="0"

usage() {
  cat <<'EOF'
Usage:
  ./esp32-ov7670-camera-phase3/capture-frame.sh [options]

Options:
  --port <port>            Serial port, default /dev/ttyUSB0
  --baudrate <baudrate>    Monitor baudrate, default 115200
  --timeout <seconds>      Capture timeout, default 90
  --raw-log <path>         Raw monitor log output path
  --pgm <path>             Extracted PGM output path
  --png <path>             Converted PNG output path
  --skip-upload            Do not upload before capturing
  --no-open                Do not open the final image
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
    --timeout)
      TIMEOUT_SECONDS="${2:-}"
      shift 2
      ;;
    --raw-log)
      RAW_LOG="${2:-}"
      shift 2
      ;;
    --pgm)
      PGM_FILE="${2:-}"
      shift 2
      ;;
    --png)
      PNG_FILE="${2:-}"
      shift 2
      ;;
    --skip-upload)
      SKIP_UPLOAD="1"
      shift
      ;;
    --no-open)
      NO_OPEN="1"
      shift
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

if [[ "$SKIP_UPLOAD" != "1" ]]; then
  "$UPLOAD_SCRIPT" --mode upload --port "$PORT"
  sleep 2
fi

mkdir -p "$(dirname "$RAW_LOG")" "$(dirname "$PGM_FILE")" "$(dirname "$PNG_FILE")"
rm -f "$RAW_LOG" "$PGM_FILE" "$PNG_FILE"

python3 - "$PORT" "$BAUDRATE" "$RAW_LOG" "$TIMEOUT_SECONDS" <<'PY'
from pathlib import Path
import sys
import time
import serial

port = sys.argv[1]
baudrate = int(sys.argv[2])
raw_log_path = Path(sys.argv[3])
timeout_seconds = float(sys.argv[4])
start = time.monotonic()
buffer = b""
capture_requested = False
fallback_sent = False

with raw_log_path.open("wb") as raw_log:
    with serial.Serial(port=port, baudrate=baudrate, timeout=0.2, write_timeout=1) as ser:
        ser.dtr = False
        ser.rts = False
        ser.reset_input_buffer()
        time.sleep(0.5)

        while True:
            if time.monotonic() - start > timeout_seconds:
                raise TimeoutError(f"Timed out waiting for END_PGM after {timeout_seconds} seconds")

            chunk = ser.read(1024)
            if not chunk:
                time.sleep(0.05)
                continue

            raw_log.write(chunk)
            raw_log.flush()
            buffer += chunk

            if not capture_requested and b"Ready. Send 'c' over serial to capture one frame." in buffer:
                ser.write(b"c")
                ser.flush()
                capture_requested = True

            if not capture_requested and not fallback_sent and time.monotonic() - start > 3.0:
                ser.write(b"c")
                ser.flush()
                capture_requested = True
                fallback_sent = True

            if b"END_PGM" in buffer:
                break

            if len(buffer) > 65536:
                buffer = buffer[-32768:]
PY

"$EXTRACT_SCRIPT" "$RAW_LOG" "$PGM_FILE"

FINAL_IMAGE="$PGM_FILE"
if command -v convert >/dev/null 2>&1; then
  convert "$PGM_FILE" "$PNG_FILE"
  FINAL_IMAGE="$PNG_FILE"
fi

echo "Raw log: $RAW_LOG"
echo "PGM: $PGM_FILE"
echo "Image: $FINAL_IMAGE"

if [[ "$NO_OPEN" != "1" ]] && command -v xdg-open >/dev/null 2>&1; then
  xdg-open "$FINAL_IMAGE" >/dev/null 2>&1 &
fi
