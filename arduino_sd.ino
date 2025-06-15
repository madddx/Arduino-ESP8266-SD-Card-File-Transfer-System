#include <SPI.h>
#include <SD.h>

const int chipSelect = 4;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!SD.begin(chipSelect)) {
    Serial.println("SD_ERROR");
    return;
  }
  Serial.println("SD_OK");
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.startsWith("WRITE:")) {
      String filename = command.substring(6);
      File file = SD.open(filename, FILE_WRITE);
      if (!file) {
        Serial.println("FILE_WRITE_ERR");
        return;
      }
      while (!Serial.available());
      String content = Serial.readStringUntil('\n');
      file.print(content);
      file.close();
      Serial.println("WRITE_DONE");
    }

    else if (command.startsWith("READ:")) {
      String filename = command.substring(5);
      File file = SD.open(filename);
      if (!file) {
        Serial.println("FILE_NOT_FOUND");
        return;
      }
      while (file.available()) {
        Serial.write(file.read());
      }
      Serial.println("\nEND_OF_FILE");
      file.close();
    }

    else if (command == "LIST") {
      File root = SD.open("/");
      while (true) {
        File entry = root.openNextFile();
        if (!entry) break;
        Serial.println(entry.name());
        entry.close();
      }
      Serial.println("END_OF_LIST");
    }

    else if (command.startsWith("DELETE:")) {
      String filename = command.substring(7);
      if (SD.exists(filename)) {
        SD.remove(filename);
        Serial.println("DELETE_OK");
      } else {
        Serial.println("DELETE_FAIL");
      }
    }
  }
}
