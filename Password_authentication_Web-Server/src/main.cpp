#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>
// Replace with your network credentials
const char* ssid = "Nayan 4G";
const char* password = "Babu@1234";
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Create a WebSocket object
AsyncWebSocket ws("/ws");
// Web Server HTTP Authentication credentials
const char* http_username = "sushant";
const char* http_password = "123456";





// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
//Variables to save values from HTML form
String input1;
String input2;
// File paths to save input values permanently
const char* input1Path = "/input1.txt";
const char* input2Path = "/input2.txt";
JSONVar values;
// Initialize LittleFS
void initFS() {
 if (!LittleFS.begin()) {
 Serial.println("An error has occurred while mounting LittleFS");
 }
 Serial.println("LittleFS mounted successfully");
}
// Read File from LittleFS
String readFile(fs::FS &fs, const char * path){
 Serial.printf("Reading file: %s\r\n", path);
 File file = fs.open(path, "r");
 if(!file || file.isDirectory()){

 Serial.println("- failed to open file for reading");
 return String();
 }
 
 String fileContent;
 while(file.available()){
 fileContent = file.readStringUntil('\n');
 break; 
 }
 return fileContent;
}
// Write file to LittleFS
void writeFile(fs::FS &fs, const char * path, const char * message){
 Serial.printf("Writing file: %s\r\n", path);
 File file = fs.open(path, "w");
 if(!file){
 Serial.println("- failed to open file for writing");
 return;
 }
 if(file.print(message)){
 Serial.println("- file written");
} else {
 Serial.println("- frite failed");
 }
}
// Initialize WiFi
void initWiFi() {
 WiFi.mode(WIFI_STA);
 WiFi.begin(ssid, password);
 Serial.print("Connecting to WiFi ..");
 while (WiFi.status() != WL_CONNECTED) {
 Serial.print('.');
 delay(1000);
 }
 Serial.println(WiFi.localIP());
}
String getCurrentInputValues(){
 values["textValue"] = input1;
 values["numberValue"] = input2;
 String jsonString = JSON.stringify(values);
 return jsonString;
}
void setup() {
 // Serial port for debugging purposes
 Serial.begin(115200);
 initWiFi();
 initFS();
 // Load values saved in LittleFS

 input1 = readFile(LittleFS, input1Path);
 input2 = readFile(LittleFS, input2Path);
 // Web Server Root URL
 server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  if(!request->authenticate(http_username, http_password))
 return request->requestAuthentication();

 request->send(LittleFS, "/index.html", "text/html");
 });
 
 server.serveStatic("/", LittleFS, "/");
 server.on("/values", HTTP_GET, [](AsyncWebServerRequest *request){
 String json = getCurrentInputValues();
 if(!request->authenticate(http_username, http_password))
 return request->requestAuthentication();

 request->send(200, "application/json", json);
 json = String();
 });
 
 server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
 int params = request->params();
 for(int i=0;i<params;i++){
 AsyncWebParameter* p = request->getParam(i);
 if(p->isPost()){
 // HTTP POST input1 value
 if (p->name() == PARAM_INPUT_1) {
 input1 = p->value().c_str();
 Serial.print("Input 1 set to: ");
 Serial.println(input1);
 // Write file to save value
 writeFile(LittleFS, input1Path, input1.c_str());
 }
 // HTTP POST input2 value
 if (p->name() == PARAM_INPUT_2) {
 input2 = p->value().c_str();
 Serial.print("Input 2 set to: ");
 Serial.println(input2);
 // Write file to save value
 writeFile(LittleFS, input2Path, input2.c_str());
 }
 //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
 }
 }
 if(!request->authenticate(http_username, http_password))
 return request->requestAuthentication();

 request->send(LittleFS, "/index.html", "text/html");
 });
 server.on("/logged-out", HTTP_GET, [](AsyncWebServerRequest *request){
 request->send(LittleFS, "/logged-out.html", "text/html",false);
 }); 
 server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
 request->send(401);
 });
  server.serveStatic("/", LittleFS, "/").setAuthentication(http_username, http_password);



 
 server.begin();
}
void loop() {
 /*Serial.println(readFile(LittleFS, input1Path));
 Serial.println(readFile(LittleFS, input2Path));
 delay(10000);*/
}