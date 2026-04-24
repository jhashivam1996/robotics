#!/usr/bin/env bash

esp32_wait_for_port() {
  local port="$1"
  local timeout_seconds="${2:-20}"
  local settle_seconds="${3:-1}"
  local deadline=$((SECONDS + timeout_seconds))

  while (( SECONDS <= deadline )); do
    if [[ -c "$port" ]]; then
      sleep "$settle_seconds"
      return 0
    fi

    sleep 0.2
  done

  echo "Timed out waiting for serial port: $port" >&2
  return 1
}

esp32_print_manual_boot_help() {
  local port="$1"

  cat >&2 <<EOF
ESP32 upload failed.

If your board did not auto-enter the bootloader, retry with manual BOOT:
  1. Hold the BOOT button.
  2. Tap EN/RST once while BOOT stays pressed.
  3. Keep holding BOOT until the upload starts writing.
  4. Release BOOT and retry the upload.

Retry command:
  ./upload-sketch.sh --mode upload --port $port
EOF
}

esp32_run_upload_helper() {
  local helper="$1"
  local port="$2"
  shift 2

  if "$helper" "$@"; then
    if [[ -n "$port" ]]; then
      esp32_wait_for_port "$port" 20 1 >/dev/null 2>&1 || true
    fi
    return 0
  fi

  local status=$?

  if [[ -n "$port" ]]; then
    echo >&2
    esp32_print_manual_boot_help "$port"
  fi

  return "$status"
}

esp32_run_monitor() {
  local port="$1"
  local baudrate="$2"
  local attach_delay_seconds="${3:-2}"
  local port_timeout_seconds="${4:-20}"
  local max_attempts="${5:-3}"
  local attempt=1

  if ! command -v arduino-cli >/dev/null 2>&1; then
    echo "arduino-cli is not installed or not on PATH." >&2
    return 1
  fi

  esp32_wait_for_port "$port" "$port_timeout_seconds" 1
  sleep "$attach_delay_seconds"

  while (( attempt <= max_attempts )); do
    local start_seconds=$SECONDS

    if arduino-cli monitor -p "$port" -c "baudrate=$baudrate"; then
      return 0
    fi

    local status=$?
    local elapsed_seconds=$((SECONDS - start_seconds))

    if [[ "$status" -eq 130 || "$elapsed_seconds" -ge 3 ]]; then
      return "$status"
    fi

    if (( attempt == max_attempts )); then
      echo "Monitor exited before the ESP32 serial port stabilized." >&2
      echo "If the board was just reset, wait a moment and retry ./monitor.sh." >&2
      return "$status"
    fi

    echo "Monitor exited early. Waiting for $port and retrying (${attempt}/${max_attempts})..." >&2
    esp32_wait_for_port "$port" "$port_timeout_seconds" 1
    sleep 1
    attempt=$((attempt + 1))
  done

  return 1
}
