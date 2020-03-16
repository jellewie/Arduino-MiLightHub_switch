/* Written by JelleWho https://github.com/jellewie

   DO NOT USE:
   Prefix 'OTA' for anything
*/
#include <ArduinoOTA.h>

#define OTA_Name "ESP32 Switch"

bool OTA_Started;
bool OTA_Enabled;

void OTA_setup() {
  OTA_Enabled = true;
  ArduinoOTA.setHostname(OTA_Name);
}
bool OTA_loop() {
  if (!OTA_Enabled)     //If OTA is disabed
    return 0;             //Stop and do nothing with OTA
  if (!OTA_Started) {   //If OTA hasn't started yet
    ArduinoOTA.begin(); //Start OTA
    OTA_Started = true;
  }
  ArduinoOTA.handle();
  return 1;
}
