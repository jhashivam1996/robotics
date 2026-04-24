#!/usr/bin/env bash
set -euo pipefail

INPUT_FILE="${1:-}"
OUTPUT_FILE="${2:-frame.pgm}"

usage() {
  cat <<'EOF'
Usage:
  ./esp32-ov7670-camera-phase3/extract-pgm.sh <monitor-log> [output-file]
EOF
}

if [[ "$INPUT_FILE" == "-h" || "$INPUT_FILE" == "--help" ]]; then
  usage
  exit 0
fi

if [[ -z "$INPUT_FILE" ]]; then
  usage
  exit 1
fi

if [[ ! -f "$INPUT_FILE" ]]; then
  echo "Input file not found: $INPUT_FILE" >&2
  exit 1
fi

python3 - "$INPUT_FILE" "$OUTPUT_FILE" <<'PY'
from pathlib import Path
import sys

input_path = Path(sys.argv[1])
output_path = Path(sys.argv[2])

data = input_path.read_bytes()
start_marker = b"BEGIN_PGM"
end_marker = b"END_PGM"

start = data.find(start_marker)
end = data.find(end_marker)

if start == -1 or end == -1 or end <= start:
    print(f"No PGM block found in {input_path}", file=sys.stderr)
    sys.exit(1)

payload = data[start + len(start_marker):end]
payload = payload.replace(b"\r", b"")
payload = payload.lstrip(b"\n")

output_path.write_bytes(payload)
print(f"Wrote {output_path}")
PY
