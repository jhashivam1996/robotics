#include "esp_http_server.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include "camera_index.h"

typedef struct {
  httpd_req_t *req;
  size_t len;
} jpg_chunking_t;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static size_t jpgEncodeStream(void *arg, size_t index, const void *data, size_t len) {
  jpg_chunking_t *j = static_cast<jpg_chunking_t *>(arg);
  if (!index) {
    j->len = 0;
  }
  if (httpd_resp_send_chunk(j->req, static_cast<const char *>(data), len) != ESP_OK) {
    return 0;
  }
  j->len += len;
  return len;
}

static esp_err_t indexHandler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, index_ov3660_html, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t statusHandler(httpd_req_t *req) {
  static char jsonResponse[1024];

  sensor_t *sensor = esp_camera_sensor_get();
  if (sensor == nullptr) {
    httpd_resp_set_status(req, "500 Internal Server Error");
    return httpd_resp_send(req, "sensor unavailable", HTTPD_RESP_USE_STRLEN);
  }

  const int length = snprintf(
    jsonResponse,
    sizeof(jsonResponse),
    "{\"framesize\":%u,\"quality\":%u,\"brightness\":%d,\"contrast\":%d,\"saturation\":%d,\"special_effect\":%u,"
    "\"wb_mode\":%u,\"awb\":%u,\"awb_gain\":%u,\"aec\":%u,\"aec2\":%u,\"ae_level\":%d,\"aec_value\":%u,"
    "\"agc\":%u,\"agc_gain\":%u,\"gainceiling\":%u,\"bpc\":%u,\"wpc\":%u,\"raw_gma\":%u,\"lenc\":%u,"
    "\"vflip\":%u,\"hmirror\":%u,\"dcw\":%u,\"colorbar\":%u}",
    sensor->status.framesize, sensor->status.quality, sensor->status.brightness, sensor->status.contrast,
    sensor->status.saturation, sensor->status.special_effect, sensor->status.wb_mode, sensor->status.awb,
    sensor->status.awb_gain, sensor->status.aec, sensor->status.aec2, sensor->status.ae_level,
    sensor->status.aec_value, sensor->status.agc, sensor->status.agc_gain, sensor->status.gainceiling,
    sensor->status.bpc, sensor->status.wpc, sensor->status.raw_gma, sensor->status.lenc, sensor->status.vflip,
    sensor->status.hmirror, sensor->status.dcw, sensor->status.colorbar
  );

  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, jsonResponse, length);
}

static esp_err_t captureHandler(httpd_req_t *req) {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  esp_err_t response = ESP_OK;
  if (fb->format == PIXFORMAT_JPEG) {
    response = httpd_resp_send(req, reinterpret_cast<const char *>(fb->buf), fb->len);
  } else {
    jpg_chunking_t chunking = {req, 0};
    response = frame2jpg_cb(fb, 80, jpgEncodeStream, &chunking) ? httpd_resp_send_chunk(req, nullptr, 0) : ESP_FAIL;
  }

  esp_camera_fb_return(fb);
  return response;
}

static esp_err_t streamHandler(httpd_req_t *req) {
  camera_fb_t *fb = nullptr;
  esp_err_t response = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
  if (response != ESP_OK) {
    return response;
  }
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      response = ESP_FAIL;
    } else {
      size_t jpgLength = 0;
      uint8_t *jpgBuffer = nullptr;

      if (fb->format == PIXFORMAT_JPEG) {
        jpgLength = fb->len;
        jpgBuffer = fb->buf;
      } else if (!frame2jpg(fb, 80, &jpgBuffer, &jpgLength)) {
        response = ESP_FAIL;
      }

      if (response == ESP_OK) {
        char partHeader[64];
        const size_t headerLength = snprintf(partHeader, sizeof(partHeader), STREAM_PART, jpgLength);
        response = httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));
        if (response == ESP_OK) {
          response = httpd_resp_send_chunk(req, partHeader, headerLength);
        }
        if (response == ESP_OK) {
          response = httpd_resp_send_chunk(req, reinterpret_cast<const char *>(jpgBuffer), jpgLength);
        }
      }

      if (fb->format != PIXFORMAT_JPEG && jpgBuffer != nullptr) {
        free(jpgBuffer);
      }
      esp_camera_fb_return(fb);
      fb = nullptr;
    }

    if (response != ESP_OK) {
      break;
    }
  }

  return response;
}

void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  config.max_uri_handlers = 16;

  httpd_uri_t indexUri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = indexHandler,
    .user_ctx = nullptr
  };

  httpd_uri_t statusUri = {
    .uri = "/status",
    .method = HTTP_GET,
    .handler = statusHandler,
    .user_ctx = nullptr
  };

  httpd_uri_t captureUri = {
    .uri = "/capture",
    .method = HTTP_GET,
    .handler = captureHandler,
    .user_ctx = nullptr
  };

  httpd_uri_t streamUri = {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = streamHandler,
    .user_ctx = nullptr
  };

  httpd_handle_t cameraHttpd = nullptr;
  if (httpd_start(&cameraHttpd, &config) == ESP_OK) {
    httpd_register_uri_handler(cameraHttpd, &indexUri);
    httpd_register_uri_handler(cameraHttpd, &statusUri);
    httpd_register_uri_handler(cameraHttpd, &captureUri);
    httpd_register_uri_handler(cameraHttpd, &streamUri);
  }
}
