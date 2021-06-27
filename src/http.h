/*
 * http.h
 * 
 * module to handle http-server stuff
 */

#include "Hash.h"

bool updateReq = false;
bool loggedIn = false;
long rssi;
IPAddress oldIP;
IPAddress loginIP;

#define BUFF_LEN 300  

bool is_authenticated() {
  Serial.println("Enter is_authenticated");
  if (httpserver.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = httpserver.header("Cookie");
    Serial.println(cookie);
    String token = sha1(String(www_username) + ":" +
    String(www_password) + ":" +
    httpserver.client().remoteIP().toString());
//  token = sha1(token);
    if (cookie.indexOf("ESPSESSIONID=" + token) != -1) {
      Serial.println("Authentication Successful");
      return true;
    }
  }
  Serial.println("Authentication Failed");
  return false;
}

void handleGetData() {
  char output[BUFF_LEN] = "Not recognized";
  String tempStr;
  char temp[32];
  char temp1[32];
  char msgpub[BUFF_LEN];
  String message="";

 DEBUG_LOG("You called Get-data\n");
 DEBUG_LOG(httpserver.uri());
 DEBUG_LOG("\n");
  if (httpserver.args()>0) {
    if (httpserver.argName(0)==String("data")) {
      rssi = WiFi.RSSI();
      if (powerConsumptionDay>0) {
        message.concat("ConsDay=");
        tempStr="%lu";
        tempStr.toCharArray(temp,32);
        sprintf(temp1,temp,powerConsumptionDay);
        message.concat(temp1);
        message.concat(" ");
      }
      if (powerConsumptionNight>0) {
        message.concat("ConsNight=");
        tempStr="%lu";
        tempStr.toCharArray(temp,32);
        sprintf(temp1,temp,powerConsumptionNight);
        message.concat(temp1);
        message.concat(" ");
      }
      if (powerProductionDay>0) {
        message.concat("ProdDay=");
        tempStr="%lu";
        tempStr.toCharArray(temp,32);
        sprintf(temp1,temp,powerProductionDay);
        message.concat(temp1);
        message.concat(" ");
      }
      if (powerProductionNight>0) {
        message.concat("ProdNight=");
        tempStr="%lu";
        tempStr.toCharArray(temp,32);
        sprintf(temp1,temp,powerProductionNight);
        message.concat(temp1);
        message.concat(" ");
      }
      if (GasConsumption>0) {
        message.concat("ConsGas=");
        tempStr="%1.3f";
        tempStr.toCharArray(temp,32);
        sprintf(temp1,temp,GasConsumption);
        message.concat(temp1);
        message.concat(" ");
      }
      message.concat("ConsNow=%lu ");
      message.concat("ProdNow=%lu ");
      message.concat("Version=");
      message.concat(P1Version);
      message.concat(" Delay=");
      message.concat(PublishDelay);
      message.concat(" rssi=%d");
      
      message.toCharArray(msgpub, 256);
      sprintf(output,msgpub,CurrentPowerConsumption,CurrentPowerProduction,rssi);
      DEBUG_LOG(output);
      DEBUG_LOG("\n");
    }
    else {
      if (httpserver.argName(0)==String("config")) {
        message="HostName=";
        message.concat(hostName);
        message.concat(" MQTTServ=");
        message.concat(mqtt_server);
        message.concat(" MQTTPort=");
        message.concat(mqtt_port);
        message.concat(" MQTTUsr=");
        message.concat(mqtt_client);
        message.concat(" MQTTPass=NoneNone");
        message.concat(" MQTTfinger=");
//        message.concat("\"");
        message.concat(fingerprint);
//        message.concat("\"");
        message.concat(" MQTTTopic=");
        message.concat(mqtt_topic);
        message.concat(" Delay=");
        message.concat(PublishDelay);
        message.concat(" www-user=");
        message.concat(www_username);
        message.concat(" www-pass=NoneNone");
        message.toCharArray(msgpub, BUFF_LEN);
        sprintf(output,msgpub);
        DEBUG_LOG(output);
        DEBUG_LOG("\n");
      }
      else {
        if (httpserver.argName(0)==String("reset")) {
          DEBUG_LOG("Reset request");
          DEBUG_LOG("\n");
          resetReq=true;
          message="reset accepted";
          message.toCharArray(output,BUFF_LEN);
        }
        else {
          if (httpserver.argName(0)==String("getdefs")) {
            DEBUG_LOG("defs request");
            DEBUG_LOG("\n");
            DEBUG_LOG("HostName:");
            DEBUG_LOG(EEPROMreadString(HOSTNAME_ADDR,HOSTNAME_LEN));
            DEBUG_LOG("|\n");
            DEBUG_LOG("MQTTServ:") ;
            DEBUG_LOG(EEPROMreadString(MQTTSERVER_ADDR,MQTTSERVER_LEN));
            DEBUG_LOG("|\n");
            DEBUG_LOG("MQTTPort:") ;
            DEBUG_LOG(EEPROMreadString(MQTTPORT_ADDR,MQTTPORT_LEN));
            DEBUG_LOG("|\n");
            DEBUG_LOG("MQTTUsr:") ;
            DEBUG_LOG(EEPROMreadString(MQTTCLIENT_ADDR,MQTTCLIENT_LEN));
            DEBUG_LOG("|\n");
            DEBUG_LOG("MQTTPass:") ;
            DEBUG_LOG(EEPROMreadString(MQTTPASS_ADDR,MQTTPASS_LEN));
            DEBUG_LOG("|\n");
            DEBUG_LOG("MQTTfinger:") ;
            DEBUG_LOG(EEPROMreadString(MQTTS_ADDR,MQTTS_LEN));
            DEBUG_LOG("|\n");
            DEBUG_LOG("MQTTTopic:") ;
            DEBUG_LOG(EEPROMreadString(MQTTTOPIC_ADDR,MQTTTOPIC_LEN));
            DEBUG_LOG("|\n");
            message="defaults accepted";
            message.toCharArray(output,BUFF_LEN);
          }
          else {
            if (httpserver.argName(0)==String("update")) {
              DEBUG_LOG("update request");
              DEBUG_LOG("\n");
              message="Nog geen web-updates";
              message.toCharArray(output,BUFF_LEN);
            }
          }
        }
      }
    }
  }
 httpserver.send(200, "text/plain", output); //Send web page
}

void handlePostData() {
  char output[BUFF_LEN] = "Not recognized";
  String tempStr;
//  char temp[32];
//  char temp1[32];
  char msgpub[BUFF_LEN];
  String message="";

 DEBUG_LOG("You called Post-data\n");
 DEBUG_LOG(httpserver.uri());
 DEBUG_LOG("\n");
     DEBUG_LOG("data POST\n");
     if (httpserver.args()>0) {
        if (httpserver.argName(0)==String("saveprefs") && is_authenticated()) {
          DEBUG_LOG("save prefs\n");
          for(int i=0;i<httpserver.args();i++)
          {
            DEBUG_LOG(httpserver.argName(i));
            DEBUG_LOG(":");
            DEBUG_LOG(httpserver.arg(i));
            DEBUG_LOG("\n");
            if (httpserver.argName(i)=="HostName") {
              EEPROMwriteString(HOSTNAME_ADDR,httpserver.arg(i),HOSTNAME_LEN);
              DEBUG_LOG("saved\n");
            }
            else
            if (httpserver.argName(i)=="MQTTServ") {
              EEPROMwriteString(MQTTSERVER_ADDR,httpserver.arg(i),MQTTSERVER_LEN);
              DEBUG_LOG("saved\n");
            }
            else
            if (httpserver.argName(i)=="MQTTPort") {
              EEPROMwriteString(MQTTPORT_ADDR,httpserver.arg(i),MQTTPORT_LEN);
              DEBUG_LOG("saved\n");
            }
            else
            if (httpserver.argName(i)=="MQTTUsr") {
              EEPROMwriteString(MQTTCLIENT_ADDR,httpserver.arg(i),MQTTCLIENT_LEN);
              DEBUG_LOG("saved\n");
            }
            else
            if (httpserver.argName(i)=="MQTTPass") {
              if (!(httpserver.arg(i)=="NoneNone")) {
                EEPROMwriteString(MQTTPASS_ADDR,httpserver.arg(i),MQTTPASS_LEN);
                DEBUG_LOG("saved\n");
              }
            }
            else
            if (httpserver.argName(i)=="MQTTfinger") {
                EEPROMwriteString(MQTTS_ADDR,httpserver.arg(i),MQTTS_LEN);
                DEBUG_LOG("saved\n");
            }
            else
            if (httpserver.argName(i)=="MQTTTopic") {
              EEPROMwriteString(MQTTTOPIC_ADDR,httpserver.arg(i),MQTTTOPIC_LEN);
              DEBUG_LOG("saved\n");
            }
            else
            if (httpserver.argName(i)=="www-pass") {
              if (!(httpserver.arg(i)=="NoneNone")) {
                EEPROMwriteString(WWWPASS_ADDR,httpserver.arg(i),WWWPASS_LEN);
                DEBUG_LOG("saved\n");
              }
            }
            else
            if (httpserver.argName(i)=="www-user") {
                EEPROMwriteString(WWWUSER_ADDR,httpserver.arg(i),WWWUSER_LEN);
                DEBUG_LOG("saved\n");
            }
          }
          message="prefs saved";
          message.toCharArray(output,BUFF_LEN);
          httpserver.send(200, "text/plain", output); //Send web page
        }
        else {
          message="prefs not saved (not logged in)";
          message.toCharArray(output,BUFF_LEN);
          httpserver.send(403, "text/plain", output); //Send web page
        }
     }

}

String getContentType(String filename) {

  if (filename.endsWith(F(".htm"))) return F("text/html");
  else if (filename.endsWith(F(".html"))) return F("text/html");
  else if (filename.endsWith(F(".css"))) return F("text/css");
  else if (filename.endsWith(F(".js"))) return F("application/javascript");
  else if (filename.endsWith(F(".json"))) return F("application/json");
  else if (filename.endsWith(F(".png"))) return F("image/png");
  else if (filename.endsWith(F(".gif"))) return F("image/gif");
  else if (filename.endsWith(F(".jpg"))) return F("image/jpeg");
  else if (filename.endsWith(F(".jpeg"))) return F("image/jpeg");
  else if (filename.endsWith(F(".ico"))) return F("image/x-icon");
  else if (filename.endsWith(F(".xml"))) return F("text/xml");
  else if (filename.endsWith(F(".pdf"))) return F("application/x-pdf");
  else if (filename.endsWith(F(".zip"))) return F("application/x-zip");
  else if (filename.endsWith(F(".gz"))) return F("application/x-gzip");
  return F("text/plain");
}

bool handleFileRead(String path) {
  Serial.print(F("handleFileRead: "));
  Serial.println(path);
  if (!is_authenticated()) {
    Serial.println(F("Go on not login!"));
    path = "/login.html";
  } else {
    if (path.endsWith("/")) path += F("index.html"); // If a folder is requested, send the index file
  }
  String contentType = getContentType(path);              // Get the MIME type
  String pathWithGz = path + F(".gz");
  if (LittleFS.exists(pathWithGz) || LittleFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (LittleFS.exists(pathWithGz)) // If there's a compressed version available
      path += F(".gz");                      // Use the compressed version
    fs::File file = LittleFS.open(path, "r");                 // Open the file
    size_t sent = httpserver.streamFile(file, contentType); // Send it to the client
    file.close();                                    // Close the file again
    Serial.println(
    String(F("\tSent file: ")) + path + String(F(" of size "))
    + sent);
    return true;
  }
  Serial.println(String(F("\tFile Not Found: ")) + path);
  return false;                     // If the file doesn't exist, return false
}

void handleNotFound() {

  DEBUG_LOG("Not found\n");
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += httpserver.uri();
  message += "\nMethod: ";
  message += (httpserver.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += httpserver.args();
  message += "\n";
  for (uint8_t i = 0; i < httpserver.args(); i++) {
    message += " " + httpserver.argName(i) + ": " + httpserver.arg(i)
    + "\n";
  }
  httpserver.send(404, "text/plain", message);
  DEBUG_LOG(message);
  DEBUG_LOG("\n");
}

void handleLogin() {
  Serial.println("Handle login");
  String msg;
  if (httpserver.hasHeader("Cookie")) {
// Print cookies
    Serial.print("Found cookie: ");
    String cookie = httpserver.header("Cookie");
    Serial.println(cookie);
  }
  if (httpserver.hasArg("username") && httpserver.hasArg("password")) {
    Serial.print("Found parameter: ");
    if (httpserver.arg("username") == String(www_username) && httpserver.arg("password") == String(www_password)) {
      httpserver.sendHeader("Location", "setup.html");
      httpserver.sendHeader("Cache-Control", "no-cache");
      String token = sha1(String(www_username) + ":" + String(www_password) + ":" + httpserver.client().remoteIP().toString());
      httpserver.sendHeader("Set-Cookie", "ESPSESSIONID=" + token);
      httpserver.send(301);
      Serial.println("Log in Successful");
      return;
    }
  msg = "Wrong username/password! try again.";
  Serial.println("Log in Failed");
  httpserver.sendHeader("Location", "/login.html?msg=" + msg);
  httpserver.sendHeader("Cache-Control", "no-cache");
  httpserver.send(301);
  return;
  }
}

void handleLogout() {
Serial.println("Disconnection");
httpserver.sendHeader("Location", "/login.html?msg=User disconnected");
httpserver.sendHeader("Cache-Control", "no-cache");
httpserver.sendHeader("Set-Cookie", "ESPSESSIONID=0");
httpserver.send(301);
return;
}

void handleSetup(){
  Serial.println("setup");
  handleFileRead("setup.html");
}

void handleUpdate(){
  Serial.println("update");
  handleFileRead("update.html");
}

void httpSetup() {

  httpserver.on("/login", HTTP_POST, handleLogin);
  httpserver.on("/logout", HTTP_GET, handleLogout);
  httpserver.on("/setup.html", HTTP_GET, handleSetup);
  httpserver.on("/update.html", HTTP_GET, handleUpdate);

  httpserver.on("/data/", HTTP_GET, handleGetData);
  httpserver.on("/data/", HTTP_POST, handlePostData);
  httpserver.onNotFound([]() {               // If the client requests any URI
    Serial.println(F("On not found"));
    if (!handleFileRead(httpserver.uri())) { // send it if it exists
      handleNotFound();// otherwise, respond with a 404 (Not Found) error
    }
  });

  httpserver.on("/upload", HTTP_POST, []() {
    httpserver.sendHeader("Connection", "close");
    httpserver.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    delay(1000);
    ESP.restart();
    }, []() {
      HTTPUpload& upload = httpserver.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
        Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace)) { //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      }
      yield();
    });
  
  httpserver.serveStatic("/", LittleFS, "/");//, "max-age=31536000");
//here the list of headers to be recorded
  const char * headerkeys[] = { "User-Agent", "Cookie" };
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
//ask server to track these headers
  httpserver.collectHeaders(headerkeys, headerkeyssize);
  httpserver.begin();
  if (!MDNS.begin(hostName)) {
    DEBUG_LOG("Error setting up MDNS responder!\n");
  }
  else {
    MDNS.addService("http", "tcp", 80);
  }

}
 
