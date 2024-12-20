#include "yaml_parse.h"
#include "read_yml_using_SPIFFS.h"


void setup() {
  // Start Serial communication
  Serial.begin(115200);
  delay(3000); // Gives Serial monitor time to initialize
  ReadYmlUsingSPIFFS();
}

void loop() {
}
