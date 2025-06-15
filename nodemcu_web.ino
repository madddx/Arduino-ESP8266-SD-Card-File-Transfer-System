#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "ESP_FileTransfer";
const char* password = "12345678";

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  delay(1000);

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", getHTML());
  });

  server.on("/upload", HTTP_POST, []() {
    server.send(200, "text/html", "<h3>Upload complete</h3><a href='/'>Back</a>");
  }, handleUpload);

  server.on("/list", HTTP_GET, handleList);
  server.on("/download", HTTP_GET, handleDownload);
  server.on("/delete", HTTP_GET, handleDelete);

  server.begin();
}

void loop() {
  server.handleClient();
}

// HTML interface
String getHTML() {
  return R"rawliteral(
  <!DOCTYPE html><html><head>
  <title>ESP File Transfer</title>
  <style>
    body { background:#e6f2ff; font-family:sans-serif; text-align:center; padding:50px; color:#004080; }
    .box { border:2px dashed #3399ff; padding:30px; border-radius:10px; margin:auto; width:300px; background:#ffffffee; }
    input[type=file] { margin:10px; }
    button { background:#3399ff; color:white; border:none; padding:10px 20px; border-radius:5px; cursor:pointer; }
    button:hover { background:#267acc; }
  </style>
  </head><body>
    <h2>ESP8266 File Transfer</h2>
    <div class='box'>
      <form method='POST' action='/upload' enctype='multipart/form-data'>
        <input type='file' name='upload'><br><br>
        <button type='submit'>Upload</button>
      </form>
      <br>
      <form action='/list'><button type='submit'>List Files</button></form>
    </div>
  </body></html>
  )rawliteral";
}

// File upload handler
void handleUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.print("WRITE:");
    Serial.println(upload.filename);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    for (size_t i = 0; i < upload.currentSize; i++) {
      Serial.write(upload.buf[i]);
    }
    Serial.println(); // signal end
  }
}

// File listing with buttons
void handleList() {
  Serial.println("LIST");
  delay(200);
  String html = "<h2>Files</h2><ul>";
  unsigned long timeout = millis() + 2000;

  while (millis() < timeout) {
    while (Serial.available()) {
      String line = Serial.readStringUntil('\n');
      line.trim();
      if (line == "END_OF_LIST") {
        html += "</ul><a href='/'>Back</a>";
        server.send(200, "text/html", html);
        return;
      }
      html += "<li>" + line +
              " <a href='/download?file=" + line + "'><button>Download</button></a> " +
              "<a href='/delete?file=" + line + "'><button>Delete</button></a></li>";
    }
  }

  server.send(200, "text/html", "<p>Timeout or error listing files.</p><a href='/'>Back</a>");
}

// File download
void handleDownload() {
  String filename = server.arg("file");
  if (filename == "") {
    server.send(400, "text/plain", "Missing filename");
    return;
  }

  Serial.print("READ:");
  Serial.println(filename);
  delay(200);

  WiFiClient client = server.client();
  server.sendHeader("Content-Type", "application/octet-stream");
  server.sendHeader("Content-Disposition", "attachment; filename=\"" + filename + "\"");
  server.sendHeader("Connection", "close");
  server.send(200);

  unsigned long timeout = millis() + 5000;
  while (millis() < timeout) {
    while (Serial.available()) {
      String line = Serial.readStringUntil('\n');
      if (line == "END_OF_FILE") return;
      client.print(line);
    }
  }
}

// File delete
void handleDelete() {
  String filename = server.arg("file");
  if (filename == "") {
    server.send(400, "text/plain", "Missing filename");
    return;
  }

  Serial.print("DELETE:");
  Serial.println(filename);
  delay(300);

  String result = "";
  unsigned long timeout = millis() + 2000;
  while (millis() < timeout) {
    if (Serial.available()) {
      result = Serial.readStringUntil('\n');
      result.trim();
      break;
    }
  }

  if (result == "DELETE_OK") {
    server.send(200, "text/html", "<h3>Deleted successfully</h3><a href='/'>Back</a>");
  } else {
    server.send(500, "text/html", "<h3>Delete failed</h3><a href='/'>Back</a>");
  }
}
