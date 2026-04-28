# ESP32-CAM OV3660 Setup

This project targets:
- board: `esp32:esp32:esp32cam`
- serial port default: `/dev/ttyUSB0`

## 1. Put In Your Wi‑Fi Credentials

Edit:

[`esp32-cam-ov3660-streamer.ino`](/home/shivam/company-repos/robotics/esp32-cam-ov3660-streamer/esp32-cam-ov3660-streamer.ino)

Replace:

```cpp
const char *ssid = "YOUR_WIFI_SSID";
const char *password = "YOUR_WIFI_PASSWORD";
```

## 2. Compile

From repo root:

```bash
./esp32-cam-ov3660-streamer/upload-sketch.sh --mode compile
```

From inside the project:

```bash
cd esp32-cam-ov3660-streamer
./upload-sketch.sh --mode compile
```

## 3. Upload

From repo root:

```bash
./esp32-cam-ov3660-streamer/upload-sketch.sh --mode upload
```

From inside the project:

```bash
cd esp32-cam-ov3660-streamer
./upload-sketch.sh --mode upload
```

If you are using a bare `ESP32-CAM`, follow the `IO0 -> GND` upload notes in:

[`HARDWARE.md`](/home/shivam/company-repos/robotics/esp32-cam-ov3660-streamer/HARDWARE.md)

## 4. Monitor

From repo root:

```bash
./esp32-cam-ov3660-streamer/monitor.sh
```

From inside the project:

```bash
cd esp32-cam-ov3660-streamer
./monitor.sh
```

After boot, the serial monitor should print:
- camera startup messages
- whether `OV3660` was detected
- the final local IP address

Open:

```text
http://<printed-ip>/
```
