#ifndef CAMERA_INDEX_H_
#define CAMERA_INDEX_H_

static const char index_ov3660_html[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>ESP32-CAM OV3660 Stream</title>
  <style>
    :root {
      --bg: #101314;
      --panel: #1a1f21;
      --ink: #e8ecef;
      --muted: #9aa7b0;
      --accent: #58d68d;
      --line: #2b3337;
    }
    body {
      margin: 0;
      font-family: "Segoe UI", sans-serif;
      background: radial-gradient(circle at top, #1d262a, var(--bg) 55%);
      color: var(--ink);
    }
    main {
      max-width: 980px;
      margin: 0 auto;
      padding: 24px;
    }
    .card {
      background: rgba(26, 31, 33, 0.92);
      border: 1px solid var(--line);
      border-radius: 18px;
      padding: 18px;
      box-shadow: 0 18px 40px rgba(0, 0, 0, 0.25);
    }
    h1 {
      margin: 0 0 10px;
      font-size: 1.9rem;
    }
    p {
      color: var(--muted);
      line-height: 1.5;
    }
    img {
      width: 100%;
      margin-top: 16px;
      border-radius: 14px;
      background: #000;
      min-height: 240px;
      object-fit: contain;
    }
    .row {
      display: flex;
      gap: 12px;
      flex-wrap: wrap;
      margin-top: 16px;
    }
    button, a {
      appearance: none;
      border: 1px solid var(--line);
      background: #13181a;
      color: var(--ink);
      border-radius: 999px;
      padding: 10px 14px;
      cursor: pointer;
      text-decoration: none;
      font-size: 0.95rem;
    }
    button.primary {
      background: var(--accent);
      border-color: var(--accent);
      color: #08130d;
      font-weight: 700;
    }
    code {
      color: var(--accent);
    }
  </style>
</head>
<body>
  <main>
    <div class="card">
      <h1>ESP32-CAM OV3660 Stream</h1>
      <p>This page uses the local MJPEG stream endpoint exposed by the board.</p>
      <div class="row">
        <button class="primary" id="start">Start Stream</button>
        <a href="/capture" target="_blank" rel="noopener">Capture JPEG</a>
        <a href="/status" target="_blank" rel="noopener">Status JSON</a>
      </div>
      <img id="stream" alt="ESP32-CAM live stream">
      <p id="hint">Open <code>/stream</code> directly if the image does not start automatically.</p>
    </div>
  </main>
  <script>
    const streamImage = document.getElementById('stream');
    document.getElementById('start').addEventListener('click', () => {
      streamImage.src = `/stream?_=${Date.now()}`;
    });
  </script>
</body>
</html>
)HTML";

#endif
