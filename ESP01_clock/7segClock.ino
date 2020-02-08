/******************
  * Very rudimentary internet clock (NTP)
  * Just a simple example based on ESP8266 sample sketches
  * Use it carefully
  *****************/


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <time.h>

const char* ssid = "ASUS_XX_2G";
const char* password = "ShootMeDead";

// your time zone, in POSX string format   https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
//const char* timezone = "CET-1CEST,M3.5.0,M10.5.0/3";     // Central Europe
//const char* timezone = "EST5EDT,M3.2.0,M11.1.0"         // America/New_York
//const char* timezone = "AEST-10AEDT,M10.1.0,M4.1.0/3"  // Australia/Sydney
//const char* timezone = "CST5CDT,M3.2.0/0,M11.1.0/1"   // America/Havana
const char* timezone = "NZST-12NZDT,M9.5.0,M4.1.0/3";  // New Zealand


void setup(void) {

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output 
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off

  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  configTime(0, 0, "pool.ntp.org");  
  setenv("TZ", timezone, 0);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    
  Serial.println("");
  Serial.print("Wifi Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop(void) 
{  
  static time_t lastv = 0;
  static uint8_t cnt = 0;
  char buf[32];

  time_t tnow = time(nullptr);
  tm* tm = localtime(&tnow);

  if (lastv != tm->tm_min) 
  { // update time every minute
    lastv = tm->tm_min;
    sprintf(buf, "$D%02d%02d\n", tm->tm_hour, tm->tm_min);	// The display expects a message like this: $D[number]\n
    Serial.print(buf);
  }
  
  // debug
  //Serial.print(String(ctime(&tnow)));

  delay(1000);

  if(cnt++ > 5) { // blink the LED to idicate we're still alive
    cnt=0;
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED off    
  }
}
