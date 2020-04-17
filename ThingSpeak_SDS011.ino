/*
 ESP8266 --> ThingSpeak Channel
 
 This sketch sends the PM2.5 and PM10 values from the SDS011 particle sensor to a ThingSpeak
 channel using the ThingSpeak API (https://www.mathworks.com/help/thingspeak).
 
 Requirements:
 
   * ESP8266 Wi-Fi Device
   * Arduino 1.8.8+ IDE
   * Additional Boards URL: http://arduino.esp8266.com/stable/package_esp8266com_index.json
   * Library: esp8266 by ESP8266 Community
   * Library: ThingSpeak by MathWorks
   * Library: Nova Fitness Sds dust sensors library (version 1.3.2) by Pawel Kolodziejczk 
 
 ThingSpeak Setup:
 
   * Sign Up for New User Account - https://thingspeak.com/users/sign_up
   * Create a new Channel by selecting Channels, My Channels, and then New Channel
   * Enable one field
   * Enter SECRET_CH_ID in "secrets.h"
   * Enter SECRET_WRITE_APIKEY in "secrets.h"
 Setup Wi-Fi:
  * Enter SECRET_SSID in "secrets.h"
  * Enter SECRET_PASS in "secrets.h"
     
 Sketch origingally created for on-bord wifi signal on Feb 1, 2017 by Hans Scharler (http://nothans.com)
 Sketch re-created on April 17th 2020 for recording and send PM2.5 and PM10 values from the SDS011 particle sensor, by Robert Wisbey (https://robertwisbey.com)
*/

#include "ThingSpeak.h"
#include "secrets.h"

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;


#include <ESP8266WiFi.h>

char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key index number (needed only for WEP)
WiFiClient  client;

#include "SdsDustSensor.h"
int rxPin = D1;
int txPin = D2;
SdsDustSensor sds(rxPin, txPin);

void setup() {
  Serial.begin(9600);
  delay(100);

sds.begin();

  Serial.println(sds.queryFirmwareVersion().toString()); // prints firmware version
  Serial.println(sds.setActiveReportingMode().toString()); // ensures sensor is in 'active' reporting mode
  Serial.println(sds.setContinuousWorkingPeriod().toString()); // ensures sensor has continuous working period - default but not recommended

  WiFi.mode(WIFI_STA);

  ThingSpeak.begin(client);
}

void loop() {

  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  }

// Measure PM2.5 and PM10 values from SDS011 air particle sensor and output to serial display
  PmResult pm = sds.readPm();
  if (pm.isOk()) {
    Serial.print("PM2.5 = ");
    Serial.print(pm.pm25);
    Serial.print(", PM10 = ");
    Serial.println(pm.pm10);

            // set the fields with the values
            ThingSpeak.setField(1, pm.pm25);
            ThingSpeak.setField(2, pm.pm10);
            // ThingSpeak.setField(3, number3);
            // ThingSpeak.setField(4, number4);

            // write to the ThingSpeak channel
            int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
            if(x == 200){
              Serial.println("Channel update successful.");
            }
            else{
            Serial.println("Problem updating channel. HTTP error code " + String(x));
            }
      }

  else {
    Serial.print("Could not read values from sensor, reason: ");
    Serial.println(pm.statusToString());
  }

  // Wait 5 seconds to update the channel again
  delay(5000);
}
