/*
  Program written by JelleWho
  Board: https://dl.espressif.com/dl/package_esp32_index.json
  Sketch from: https://github.com/jellewie/Arduino-MiLightHub_switch

  FIXME?
  On boot button stuck, when released, the buttonpressed interupt has happened, and pressed/longpressed will be executed
  Make a manual, and descibe things like 'button 1 (ID0) is Enable_OTA and settings page'
  Maybe Reset button in the webb app?
  Fix the rotation feature, also maybe needs to reboot after change??
*/
#if !defined(ESP32)
#error "Please check if the 'DOIT ESP32 DEVKIT V1' board is selected, which can be downloaded at https://dl.espressif.com/dl/package_esp32_index.json"
#endif

#include <WiFi.h>             //Needed for WiFi stuff
#include <WiFiClient.h>       //Needed for sending data to devices
#include <WebServer.h>        //This is to create a acces point with wifimanager if no wifi is set
#include <ArduinoJson.h>      //This is to pack/unpack data send to the hub
#include <rom/rtc.h>          //This is for rtc_get_reset_reason
WebServer server(80);
extern void WiFiManager_CheckAndReconnectIfNeeded();
#include "OTA.h"
#include "functions.h"
#include "miLight.h"
#include "Button.h"
//////////////////////////////////////////////////////////////////////
//  User Settings
//////////////////////////////////////////////////////////////////////
#define SerialEnabled
//#define SerialEnabled_Extra

const buttons SetA = {34, 21};    //Only used for reference pin pares, not what button is what pin
const buttons SetB = {35, 19};
const buttons SetC = {32, 18};
const buttons SetD = {33,  5};
const buttons SetE = {26, 23};
const buttons SetF = {27, 22};
const buttons SetG = {14,  4};
const buttons SetH = {12, 15};
//////////////////////////////////////////////////////////////////////
//  Soft settings (can be changed later)
//////////////////////////////////////////////////////////////////////
buttons A_A = SetA,   A_B = SetB,   A_C = SetC,   A_D = SetD;
Button SwitchA[4] = {A_A, A_B, A_C, A_D};
byte RotationA = 1; //0=unused, 1=Orginal layout, 2=case 90 clockwise to PCB, 3=case 180 clockwise to PCB, 4=case 270 clockwise to PCB
MiLight LightA = {"0xF001", "rgb_cct", 1};                         //What light to control
const byte Amount_Buttons = sizeof(SwitchA) / sizeof(SwitchA[0]);
String CommandsA[Amount_Buttons] = {"{\"commands\":[\"toggle\"]}",
                                    "{\"brightness\":1,\"color\":\"255,0,0\",\"state\":\"On\"}",
                                    "{\"brightness\":128,\"color\":\"255,0,0\",\"state\":\"On\"}",
                                    "{\"brightness\":255,\"color\":\"255,255,255\",\"state\":\"On\"}"
                                   };
buttons B_A = SetE,   B_B = SetF,   B_C = SetG,   B_D = SetH;
Button SwitchB[4] = {B_A, B_B, B_C, B_D};
byte RotationB = 0;
MiLight LightB = {"0xF002", "rgb_cct", 1};
String CommandsB[Amount_Buttons] = {"{\"commands\":[\"toggle\"]}",
                                    "{\"brightness\":1,\"color\":\"255,0,0\",\"state\":\"On\"}",
                                    "{\"brightness\":128,\"color\":\"255,0,0\",\"state\":\"On\"}",
                                    "{\"brightness\":255,\"color\":\"255,255,255\",\"state\":\"On\"}"
                                   };
//////////////////////////////////////////////////////////////////////
//  End of user Settings
//////////////////////////////////////////////////////////////////////
#include "WiFiManager.h"
void setup() {
#ifdef SerialEnabled
  Serial.begin(115200);
#endif //SerialEnabled
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  //===========================================================================
  //Load data from EEPROM, and set the configured pinout
  //===========================================================================
  WiFiManager.LoadData();         //Read data from EEPROM
  SetRotation(true , RotationA);  //Rotate pins if needed
  SetRotation(false, RotationB);
  //===========================================================================
  //Attach interupts to the button pins so we can responce if they change
  //===========================================================================
  attachInterrupt(SwitchA[0].Data.PIN_Button, ISR_A0, CHANGE);
  attachInterrupt(SwitchA[1].Data.PIN_Button, ISR_A1, CHANGE);
  attachInterrupt(SwitchA[2].Data.PIN_Button, ISR_A2, CHANGE);
  attachInterrupt(SwitchA[3].Data.PIN_Button, ISR_A3, CHANGE);
  if (RotationB != 0) {                                               //Do not attach interupt if this pair is unused
    attachInterrupt(SwitchB[0].Data.PIN_Button, ISR_B0, CHANGE);
    attachInterrupt(SwitchB[1].Data.PIN_Button, ISR_B1, CHANGE);
    attachInterrupt(SwitchB[2].Data.PIN_Button, ISR_B2, CHANGE);
    attachInterrupt(SwitchB[3].Data.PIN_Button, ISR_B3, CHANGE);
  }
  //===========================================================================
  //Wait for all buttons to be NOT pressed
  //===========================================================================
  byte ButtonPressedID = 1;
  while (ButtonPressedID > 0) {
#ifdef SerialEnabled
    Serial.println("Waiting on a button(s) " + String(ButtonPressedID, BIN) + " before starting up");
#endif //SerialEnabled
    ButtonPressedID = 0;                            //Set to NOT pressed by default, will be overwritten
    while (!TickEveryMS(50)) {}                     //Wait here for 50ms (so an error blink would be nice)

    //Returns the button states in bits; Like 0000<button1><b2><b3><b4> where 1 is HIGH and 0 is LOW
    //Example '00001001' = Buttons 1 and 4 are HIGH (Note we count from LSB)
    byte ButtonID = 0;
    for (byte i = 0; i < Amount_Buttons; i++) {
      ButtonID = ButtonID << 1;                     //Move bits 1 to the left (its like *2)
      Button_Time Value = SwitchA[i].CheckButton();
      if (Value.Pressed) {
        ButtonID += 1;                              //Flag this button as on
        digitalWrite(SwitchA[i].Data.PIN_LED, !digitalRead(SwitchA[i].Data.PIN_LED));
      } else
        digitalWrite(SwitchA[i].Data.PIN_LED, LOW);
    }
    if (RotationB != 0) {
      for (byte i = 0; i < Amount_Buttons; i++) {
        ButtonID = ButtonID << 1;                   //Move bits 1 to the left (its like *2)
        Button_Time Value = SwitchB[i].CheckButton();
        if (Value.Pressed) {
          ButtonID += 1;                            //Flag this button as on
          digitalWrite(SwitchB[i].Data.PIN_LED, !digitalRead(SwitchB[i].Data.PIN_LED));
        } else
          digitalWrite(SwitchB[i].Data.PIN_LED, LOW);
      }
    }
    ButtonPressedID = ButtonID;                     //Get the button state, here 1 is HIGH in the form of '0000<Button 1><2><3><4> '
  }
  for (byte i = 0; i < Amount_Buttons; i++) {
    digitalWrite(SwitchA[i].Data.PIN_LED, LOW);     //Make sure all LED's are off
    if (RotationB != 0)
      digitalWrite(SwitchB[i].Data.PIN_LED, LOW);   //Make sure all LED's are off
  }
  //===========================================================================
  //Initialise server stuff
  //===========================================================================
  server.on("/",          WiFiManager_handle_Connect);    //Must be declaired before "WiFiManager.Start()" for APMode
  server.on("/setup",     WiFiManager_handle_Settings);   //Must be declaired before "WiFiManager.Start()" for APMode
  server.on("/ota",               OTA_handle_uploadPage);
  server.on("/update", HTTP_POST, OTA_handle_update, OTA_handle_update2);
  server.onNotFound(      HandleNotFound);
  //===========================================================================
  //Start WIFI
  //===========================================================================
  byte Answer = WiFiManager.Start();
  if (Answer != 1) {
#ifdef SerialEnabled
    Serial.println("setup error code '" + String(Answer) + "'");
#endif //SerialEnabled
    ESP.restart();                //Restart the ESP
  }
  WiFiManager.StartServer();      //Start the server
  //WiFiManager.EnableSetup(true);
#ifdef SerialEnabled
  //===========================================================================
  //Get Reset reason (This could be/is usefull for power outage)
  //===========================================================================
  if (byte a = GetResetReason()) {
    Serial.print("Done with boot, resetted due to ");
    print_reset_reason(a);
  }
#endif //SerialEnabled
}
//===========================================================================
void loop() {
  server.handleClient();

  if (OTA.Enabled) BlinkEveryMs(LED_BUILTIN, 1000);    //Blink every x MS to show OTA is on

  for (byte i = 0; i < Amount_Buttons; i++) {
    Check(SwitchA[i].CheckButton(), LightA, CommandsA[i], SwitchA[i].Data.PIN_LED, i);
    if (RotationB != 0)
      Check(SwitchB[i].CheckButton(), LightB, CommandsB[i], SwitchB[i].Data.PIN_LED, i);
  }
}
//===========================================================================
void Check(Button_Time Value, MiLight Light, String Action, byte LEDpin, byte ButtonID) {
#ifdef SerialEnabled_Extra
  Serial.println("b" + String(ButtonID) + "=" + String(Value.Pressed) + " S=" + String(Value.StartPress) + " L=" + String(Value.PressedLong) + " SL=" + String(Value.StartLongPress) + " LEDpin=" + String(LEDpin));
#endif //SerialEnabled_Extra
  if (Value.StartPress) {                               //If button is just pressed in
    if (LEDpin > 0) digitalWrite(LEDpin, HIGH);         //If a LED pin was given; Set that buttons LED on
    for (int i = 5; i > 0; i--) {                       //Where i is amount of tries tries to do
      byte Feedback = SetLight(Light, Action);
      switch (Feedback) {
        case 1:                                         //If done
          i = 0;                                        //  Do not try again (
          break;
        case 2:                                         //Can't connect, hub is offline
          i = 0;                                        //  Do not try again (
          break;
        case 3:                                         //If Json wrong format
#ifdef SerialEnabled
          Serial.println("Error in Json");
#endif //SerialEnabled
          i = 0;                                        //  Do not try again (
          break;
        case 6:                                         //If we are not connected to WIFI
          WiFiManager_Connected = false;
          i = 0;                                        //  Do not try again (
          break;
      }
    }
    if (LEDpin > 0) digitalWrite(LEDpin, LOW);          //If a LED pin was given; Set that buttons LED off
  } else if (Value.StartLongPress) {
    if (ButtonID == 0) {
      OTA.Enabled = !OTA.Enabled;                       //Toggle OTA on/off
      WiFiManager.EnableSetup(OTA.Enabled);
    }
    if (LEDpin > 0) digitalWrite(LEDpin, LOW);          //If a LED pin was given; Set that buttons LED off
  }
  if (Value.PressedLong) {                              //If it is/was a long press
    if (Value.Pressed) {                                //If we are still pressing
      if (Value.PressedTime > Time_ESPrestartMS - 1000) {
        if (LEDpin > 0) BlinkEveryMs(LEDpin, 10);       //If a LED pin was given; Blink that button LED
      } else
        digitalWrite(LED_BUILTIN, HIGH);
    } else {
      digitalWrite(LED_BUILTIN, LOW);
      if (LEDpin > 0) digitalWrite(LEDpin, LOW);        //If a LED pin was given; Blink that button LED
    }
  }
}
void SetRotation(bool IsSwitchA, byte Rotation) {
  if (IsSwitchA) {
    switch (Rotation) {
      case 1:
        A_A = SetA;   A_B = SetB;   A_C = SetC;   A_D = SetD;   //Orginal layout
        break;
      case 2:
        A_A = SetB;   A_B = SetD;   A_C = SetA;   A_D = SetC;   //case 90  clockwise to PCB
        break;
      case 3:
        A_A = SetD;   A_B = SetC;   A_C = SetB;   A_D = SetA;   //Orginal layout
        break;
      case 4:
        A_A = SetC;   A_B = SetA;   A_C = SetD;   A_D = SetB;   //case 270 clockwise to PCB
        break;
    }
  } else {
    switch (Rotation) {
      case 1:
        B_A = SetA;   B_B = SetB;   B_C = SetC;   B_D = SetD;   //Orginal layout
        break;
      case 2:
        B_A = SetB;   B_B = SetD;   B_C = SetA;   B_D = SetC;   //case 90  clockwise to PCB
        break;
      case 3:
        B_A = SetD;   B_B = SetC;   B_C = SetB;   B_D = SetA;   //Orginal layout
        break;
      case 4:
        B_A = SetC;   B_B = SetA;   B_C = SetD;   B_D = SetB;   //case 270 clockwise to PCB
        break;
    }
  }
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
