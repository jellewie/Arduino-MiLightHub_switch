/*
  Program written by JelleWho
  Board: https://dl.espressif.com/dl/package_esp32_index.json
  Sketch from: https://github.com/jellewie/Arduino-MiLightHub_switch
*/
#if !defined(ESP32)
#error "Please check if the 'DOIT ESP32 DEVKIT V1' board is selected, which can be downloaded at https://dl.espressif.com/dl/package_esp32_index.json"
#endif

//#define SerialEnabled
//#define Button_SerialEnabled
//#define Button_SerialEnabled_CheckButton
//#define Logging

#ifdef SerialEnabled
#define Logging
#define WiFiManager_SerialEnabled
//#define Extra_SerialEnabled
#endif //SerialEnabled

#ifdef Logging
#define milight_Log
#define Speed_Log
//Convert_Log
#endif //Logging

#include "WiFiManagerBefore.h"                                  //Define what options to use/include or to hook into WiFiManager
#include "WiFiManager/WiFiManager.h"                            //Includes <WiFi> and <WebServer.h> and setups up 'WebServer server(80)' if needed      https://github.com/jellewie/Arduino-WiFiManager

#include <WiFiClient.h>                                         //Needed for sending data to devices
#include <ArduinoJson.h>                                        //This is to pack/unpack data send to the hub
#include <rom/rtc.h>                                            //This is for rtc_get_reset_reason
#include "Log.h"
#include "functions.h"
#include "miLight.h"
#include "Button/Button.h"                                      //https://github.com/jellewie/Arduino-Button

Button SwitchA[4] = {{34, {_PIN_LED: 21}}, {35, {_PIN_LED: 19}}, {32, {_PIN_LED: 18}}, {33, {_PIN_LED: 5}}}; //Bunch up the 4 buttons to be 1 switch set (Only used for reference pin pares, not which command is connected to which pin)
Button SwitchB[4] = {{26, {_PIN_LED: 23}}, {27, {_PIN_LED: 22}}, {14, {_PIN_LED: 4}}, {12, {_PIN_LED: 15}}}; // ^

byte RotationA = NORMAL;                                        //SOFT_SETTING Rotation of the PCB seen from the case
byte RotationB = UNUSED;                                        // ^           RIGHT=PCB 90° clockwise to case
MiLight LightA = {"0xF001", "rgb_cct", 1};                      //SOFT_SETTING What light to control
MiLight LightB = {"0xF002", "rgb_cct", 1};                      // ^           device_id, remote_type, group_id
const byte Amount_Buttons = sizeof(SwitchA) / sizeof(SwitchA[0]);                         //Why filling this in if we can automate that? :)
String CommandsA[Amount_Buttons] = {"{'commands':['toggle']}",                            //SOFT_SETTING what command which button sends (Example of toggle)
                                    "{'brightness':60,'color':'255,50,10','state':'On'}", //(Example of RGB)
                                    "{'brightness':255,'color_temp':999,'state':'On'}",   //(Example of CC/WW)
                                    "{'brightness':255,'color_temp':1,'state':'On'}"      //For more see https://sidoh.github.io/esp8266_milight_hub/branches/1.10.6/ Please note not all commands might be supported by your bulb
                                   };
String CommandsB[Amount_Buttons] = {"{'commands':['toggle']}",                          // ^
                                    "{'brightness':60,'color':'255,50,10','state':'On'}",
                                    "{'brightness':255,'color_temp':999,'state':'On'}",
                                    "{'brightness':255,'color_temp':1,'state':'On'}"
                                   };
#include "WiFiManagerLater.h"                                   //Define options of WiFiManager (can also be done before), but WiFiManager can also be called here (example for DoRequest)

void setup() {
#ifdef SerialEnabled
  Serial.begin(115200);
#endif //SerialEnabled
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  //===========================================================================
  //Load data, since we need this to check if RotationB is !UNUSED)
  //===========================================================================
  WiFiManager.LoadData();
  //===========================================================================
  //Attach interrupts to the button pins so we can response if they change
  //===========================================================================
  attachInterrupt(SwitchA[0].PIN_Button, ISR_A0, CHANGE);
  attachInterrupt(SwitchA[1].PIN_Button, ISR_A1, CHANGE);
  attachInterrupt(SwitchA[2].PIN_Button, ISR_A2, CHANGE);
  attachInterrupt(SwitchA[3].PIN_Button, ISR_A3, CHANGE);
  if (RotationB != UNUSED) {                                    //Do not attach interrupt if this switch set is unused
    attachInterrupt(SwitchB[0].PIN_Button, ISR_B0, CHANGE);
    attachInterrupt(SwitchB[1].PIN_Button, ISR_B1, CHANGE);
    attachInterrupt(SwitchB[2].PIN_Button, ISR_B2, CHANGE);
    attachInterrupt(SwitchB[3].PIN_Button, ISR_B3, CHANGE);
  }
  //===========================================================================
  //Wait for all buttons to be NOT pressed
  //===========================================================================
  byte ButtonPressedID = 1;
  while (ButtonPressedID > 0) {
#ifdef SerialEnabled
    Serial.println("Waiting on a button(s) " + String(ButtonPressedID, BIN) + " before starting up");
#endif //SerialEnabled
    ButtonPressedID = 0;                                        //Set to NOT pressed by default, will be overwritten
    while (!TickEveryMS(50)) {}                                 //Wait here for 50ms (so an error blink would be nice)

    //Returns the button states in bits; Like 0000<button1><b2><b3><b4> where 1 is HIGH and 0 is LOW
    //Example '00001001' = Buttons 1 and 4 are HIGH (Note we count from LSB)
    byte ButtonID = 0;
    for (byte i = 0; i < Amount_Buttons; i++) {
      ButtonID = ButtonID << 1;                                 //Move bits 1 to the left (it’s like *2)
      Button_Time Value = SwitchA[i].CheckButton();
      if (Value.Pressed) {
        ButtonID += 1;                                          //Flag this button as on
        digitalWrite(SwitchA[i].PIN_LED, !digitalRead(SwitchA[i].PIN_LED));
      } else
        digitalWrite(SwitchA[i].PIN_LED, LOW);
    }
    if (RotationB != UNUSED) {
      for (byte i = 0; i < Amount_Buttons; i++) {
        ButtonID = ButtonID << 1;                               //Move bits 1 to the left (it’s like *2)
        Button_Time Value = SwitchB[i].CheckButton();
        if (Value.Pressed) {
          ButtonID += 1;                                        //Flag this button as on
          digitalWrite(SwitchB[i].PIN_LED, !digitalRead(SwitchB[i].PIN_LED));
        } else
          digitalWrite(SwitchB[i].PIN_LED, LOW);
      }
    }
    ButtonPressedID = ButtonID;                                 //Get the button state, here 1 is HIGH in the form of '0000<Button 1><2><3><4> '
  }
  for (byte i = 0; i < Amount_Buttons; i++) {
    digitalWrite(SwitchA[i].PIN_LED, LOW);                      //Make sure all LED's are off
    if (RotationB != UNUSED)
      digitalWrite(SwitchB[i].PIN_LED, LOW);                    //Make sure all LED's are off
  }
  //===========================================================================
  //Initialise server stuff
  //===========================================================================
  server.on("/log",               handle_Log);
  server.onNotFound(              HandleNotFound);
  //===========================================================================
  //Start WIFI
  //===========================================================================
  byte Answer = WiFiManager.Start();
  if (Answer != 1) {
#ifdef SerialEnabled
    Serial.println("setup error code '" + String(Answer) + "'");  delay(10);
#endif //SerialEnabled
    ESP.restart();                                              //Restart the ESP
  }
  WiFiManager.OTA_Enabled = false;
  WiFiManager.EnableSetup(true);        //Start the server (if you also need it for other stuff)
  //===========================================================================
  //Get Reset reason (This could be/is useful for power outage)
  //===========================================================================
#ifdef SerialEnabled
  if (byte Reason = GetResetReason()) {
    Serial.println("Done with boot, resetted due to " + ResetReasonToString(Reason));
  }
#endif //SerialEnabled
}
//===========================================================================
void loop() {
  if (WiFiManager.OTA_Enabled) {
    WiFiManager.RunServer();                                    //Do WIFI server stuff if needed
    BlinkEveryMs(LED_BUILTIN, 1000);                            //Blink every x MS to show OTA is on
  }

  static byte OldRotationB = RotationB;                         //Check if a second switch is added (if a rotation is set)
  if (RotationB != OldRotationB) {
    if (OldRotationB == 0 or RotationB == 0) {
#ifdef Extra_SerialEnabled
      Serial.println("ESP must reset due to Second pair added/removed");  delay(10);
#endif //Extra_SerialEnabled
      ESP.restart();                                            //Restart the ESP
    }
  }

  for (byte i = 0; i < Amount_Buttons; i++) {
    Check(SwitchA[i].CheckButton(), LightA, CommandsA[RotateButtonID(RotationA, i)], SwitchA[i].PIN_LED, RotateButtonID(RotationA, i));
    if (RotationB != UNUSED)
      Check(SwitchB[i].CheckButton(), LightB, CommandsB[RotateButtonID(RotationB, i)], SwitchB[i].PIN_LED, RotateButtonID(RotationB, i));
  }
}
//===========================================================================
void Check(Button_Time Value, MiLight Light, String Action, byte LEDpin, byte ButtonID) {
#ifdef Extra_SerialEnabled
  Serial.println("b" + String(ButtonID) + "=" + String(Value.Pressed) + " S=" + String(Value.StartPress) + " L=" + String(Value.PressedLong) + " SL=" + String(Value.StartLongPress) + " LEDpin=" + String(LEDpin));
#endif //Extra_SerialEnabled
#ifdef Speed_Log
  unsigned long StartMS = 0;
#endif //Speed_Log
  if (Value.StartPress) {                                       //If button is just pressed in
#ifdef Speed_Log
    StartMS = millis();
#endif //Speed_Log
    if (LEDpin > 0) digitalWrite(LEDpin, HIGH);                 //If a LED pin was given; Set that buttons LED on
    for (int TryAgain = 3; TryAgain > 0; TryAgain--) {          //Where TryAgain is amount of tries tries to do
      byte Feedback = SetLight(Light, Action);
      switch (Feedback) {
        case REQ_UNK:                               break;      //Keep trying again
        case REQ_SUCCES:              TryAgain = 0; break;      //We have succeeded, Do not try again
        case REQ_HUB_CONNECT_ERROR:   TryAgain = 0; break;      //The hub is unreachable, Do not try again
        case REQ_TIMEOUT:             TryAgain = 0; break;      //The hub doesn't send a responcecode back in time, some unknowns, so do not try again
        case REQ_PAGE_NOT_FOUND:      TryAgain = 0; break;      //If Json wrong format, Do not try again
        case REQ_SETUP_REQUIRED:      TryAgain = 0; break;      //If Json wrong format, Do not try again
        default:                      TryAgain = 0; break;      //Some uncatched error, Do not try again
      }
    }
    if (LEDpin > 0) digitalWrite(LEDpin, LOW);                  //If a LED pin was given; Set that buttons LED off
#ifdef Speed_Log
    Log.Add("Command processing time (start to finisch)=" + String(millis() - StartMS) + "ms");
#endif //Speed_Log
  } else if (Value.StartLongPress) {
    WiFiManager.OTA_Enabled = !WiFiManager.OTA_Enabled;         //Toggle OTA on/off
    if (LEDpin > 0) digitalWrite(LEDpin, LOW);                  //If a LED pin was given; Set that buttons LED off
  }
  if (Value.PressedLong) {                                      //If it is/was a long press
    if (Value.Pressed) {                                        //If we are still pressing
      if (Value.PressedTime > Time_ESPrestartMS - 1000) {
        if (LEDpin > 0) BlinkEveryMs(LEDpin, 10);               //If a LED pin was given; Blink that button LED
      } else
        digitalWrite(LED_BUILTIN, HIGH);
    } else {
      digitalWrite(LED_BUILTIN, LOW);
      if (LEDpin > 0) digitalWrite(LEDpin, LOW);        //If a LED pin was given; Blink that button LED
    }
  }
}
byte RotateButtonID(byte Rotation, byte i) {
  switch (Rotation) {
    case NORMAL:
      return i;
    case LEFT:
      switch (i) {
        case 0:   return 1;
        case 1:   return 3;
        case 2:   return 0;
        case 3:   return 2;
      }
    case UPSIDE_DOWN:
      switch (i) {
        case 0:   return 3;
        case 1:   return 2;
        case 2:   return 1;
        case 3:   return 0;
      }
    case RIGHT:
      switch (i) {
        case 0:   return 2;
        case 1:   return 0;
        case 2:   return 3;
        case 3:   return 1;
      }
  }
  return 0;
}
//===========================================================================
void ISR_A0() {
  SwitchA[0].Pinchange();
}
void ISR_A1() {
  SwitchA[1].Pinchange();
}
void ISR_A2() {
  SwitchA[2].Pinchange();
}
void ISR_A3() {
  SwitchA[3].Pinchange();
}
void ISR_B0() {
  SwitchB[0].Pinchange();
}
void ISR_B1() {
  SwitchB[1].Pinchange();
}
void ISR_B2() {
  SwitchB[2].Pinchange();
}
void ISR_B3() {
  SwitchB[3].Pinchange();
}
