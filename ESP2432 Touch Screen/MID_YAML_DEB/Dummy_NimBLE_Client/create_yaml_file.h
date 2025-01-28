
#ifndef CREATE_YAML_H
#define CREATE_YAML_H

#include <FS.h>
#include <SPIFFS.h>
#include <Arduino.h>




const String config_yaml="/config.yaml";

void writeYAMLFile(String yamlContent) {
  // Open or create a file on SPIFFS in write mode
  File file = SPIFFS.open(config_yaml, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  file.print(yamlContent);
  // Close the file
  file.close();
}

String readYAML(){
  File file = SPIFFS.open(config_yaml, FILE_READ);
  String yamlContent = file.readString();
  file.close();
  Serial.printf("\nYAML:\n%s\n",yamlContent);
  return yamlContent;
}


String ReadYmlUsingSPIFFS(String DefaultYamlContent) {
  if (!SPIFFS.begin(true)) {
      Serial.println("Failed to initialize SPIFFS!");
      return "0";
  }
  Serial.println("SPIFFS initialized successfully!");
  // /////////////// in case you want to remove the yaml file before rewriting
  // if (SPIFFS.exists(config_yaml)) {
  //       Serial.print("Deleting file: ");
  //       Serial.println(config_yaml);

  //       // Delete the file
  //       if (SPIFFS.remove(config_yaml)) {
  //           Serial.println("File deleted successfully");
  //       } else {
  //           Serial.println("Failed to delete the file");
  //       }
  //   } else {
  //       Serial.print("File does not exist: ");
  //       Serial.println(config_yaml);
  // }
  ///////////////
  if ( SPIFFS.exists(config_yaml)) {
    Serial.println("File "+ config_yaml +" found!");
  } else {
    Serial.println("Failed to fine config file, writing new " +config_yaml+ " file!");
  // Create and write to a file
    writeYAMLFile(DefaultYamlContent);
  } 
  return readYAML();
}

#endif //CREATE_YAML_H