/*
""" 
		Get the Youtube subscriber count from google.apis and send it to the display every 60s.
		Loops until killed.
		
		You will need to get your own API-Key from google but that is quite simple. Google it ;)
"""
*/


#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#define STASSID "ASUS_XX_2G"	// your wifi AP
#define STAPSK  "ShootMeDead"	// AP password

const char* ssid = STASSID;
const char* password = STAPSK;

const char* host = "www.googleapis.com";
String url = "/youtube/v3/channels?part=statistics&id=<YOUR_CHANNEL_ID>&fields=items/statistics/subscriberCount&key=<YOUR_API_KEY>";
const int httpsPort = 443;

// Use web browser to view and copy SHA1 fingerprint of the certificate
const char fingerprint[] PROGMEM = "43 2f 75 94 fb 9c 00 ab ee 26 22 61 57 50 bf b4 d2 05 85 c7";


// The meat
//
int get_subscriber_count()
{
  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  client.setFingerprint(fingerprint);
  client.setTimeout(10000);

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return -1;
  }

  if(client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n") == 0)
  {
    Serial.println("Failed to send http request");
    return -1; 
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print("Unexpected response: ");
    Serial.println(status);
    return -1;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println("Failed to read header");
    return -1;
  }

  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 200;
  DynamicJsonDocument doc(capacity);

  // Parse JSON object
  DeserializationError error = deserializeJson(doc, client);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return -1;
  }

  // debug
  //serializeJsonPretty(doc, Serial);
  return doc["items"][0]["statistics"]["subscriberCount"].as<int>();
}


void InitWifi()
{
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  
}

void setup() {
  delay(2000);
  Serial.begin(9600);
  InitWifi();
}

void loop() 
{
  int subCount = get_subscriber_count();
  if(subCount > 0) {
    //Serial.print("getSubCount returned: ");
    //Serial.println(subCount);
    Serial.print(String("$D")+String(subCount)+"\n");
  }
  delay(60000); // 1 minute wait
  delay(60000); // 1 minute wait
  delay(60000); // 1 minute wait
  delay(60000); // 1 minute wait
  delay(60000); // 1 minute wait
}
