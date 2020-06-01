/*
  Program written by JelleWho
  Board: https://dl.espressif.com/dl/package_esp32_index.json

  FIXME?
  On boot button stuck, when released, the buttonpressed interupt has happened, and pressed/longpressed will be executed
  Can we make light settings softcoded so we can set them with WIFI settings? '"0xF342","rgb_cct",1' = 'light_Id,type,group'
  Make a manual, and descibe things like 'button 1 (ID0) is Enable_OTA' and 'button 2 (ID1) is enable settings page'

*/
#include <WiFi.h>             //Needed for WiFi stuff
#include <WiFiClient.h>       //Needed for sending data to devices
#include <WebServer.h>        //This is to create a acces point with wifimanager if no wifi is set
#include <ArduinoJson.h>      //This is to pack/unpack data send to the hub
#include <rom/rtc.h>          //This is for rtc_get_reset_reason
WebServer server(80);
#include "functions.h"
#include "miLight.h"
#include "Button.h"
//////////////////////////////////////////////////////////////////////
//  User Settings
//////////////////////////////////////////////////////////////////////
//#define SerialEnabled
//#define SecondSwitch

MiLight LightA = {"0xF001", "rgb_cct", 1};                         //What light to control
Button SwitchA[4] = {buttons({34, 21}), buttons({35, 19}), buttons({32, 18}), buttons({33, 5})};   //Orginal layout
//Button SwitchA[4] = {buttons({35, 19}), buttons({33,  5}), buttons({34, 21}), buttons({32, 18})};   //case 90  clockwise to PCB
//Button SwitchA[4] = {buttons({33,  5}), buttons({32, 18}), buttons({35, 19}), buttons({34, 21})};   //case 180 clockwise to PCB
//Button SwitchA[4] = {buttons({32, 18}), buttons({34, 21}), buttons({33,  5}), buttons({35, 19})};   //case 270 clockwise to PCB
const byte Amount_Buttons = sizeof(SwitchA) / sizeof(SwitchA[0]);
String CommandsA[Amount_Buttons] = {"{\"commands\":[\"toggle\"]}",
                                    "{\"brightness\":1,\"color\":\"255,0,0\",\"state\":\"On\"}",
                                    "{\"brightness\":128,\"color\":\"255,0,0\",\"state\":\"On\"}",
                                    "{\"brightness\":255,\"color\":\"255,255,255\",\"state\":\"On\"}"
                                   };

#ifdef SecondSwitch
MiLight LightB = {"0xF002", "rgb_cct", 1};                         //What light to control
Button SwitchB[4] = {buttons({26, 23}), buttons({27, 22}), buttons({14,  4}), buttons({12,15})};   //Orginal layout
//Button SwitchB[4] = {buttons({27, 22}), buttons({12, 15}), buttons({26, 23}), buttons({14,  4})};   //case 90  clockwise to PCB
//Button SwitchB[4] = {buttons({12, 15}), buttons({14,  4}), buttons({27, 22}), buttons({26, 23})};   //case 180 clockwise to PCB
//Button SwitchB[4] = {buttons({14,  4}), buttons({26, 23}), buttons({12, 15}), buttons({27, 22})};   //case 270 clockwise to PCB
String CommandsB[Amount_Buttons] = {"{\"commands\":[\"toggle\"]}",
                                    "{\"brightness\":1,\"color\":\"255,0,0\",\"state\":\"On\"}",
                                    "{\"brightness\":128,\"color\":\"255,0,0\",\"state\":\"On\"}",
                                    "{\"brightness\":255,\"color\":\"255,255,255\",\"state\":\"On\"}"
                                   };
#endif //SecondSwitch
//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
void setup() {
#ifdef SerialEnabled
  Serial.begin(115200);
#endif //SerialEnabled
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  //===========================================================================
  //Attach interupts to the button pins so we can responce if they change
  //===========================================================================
  attachInterrupt(SwitchA[0].Data.PIN_Button, ISR_A0, CHANGE);
  attachInterrupt(SwitchA[1].Data.PIN_Button, ISR_A1, CHANGE);
  attachInterrupt(SwitchA[2].Data.PIN_Button, ISR_A2, CHANGE);
  attachInterrupt(SwitchA[3].Data.PIN_Button, ISR_A3, CHANGE);
#ifdef SecondSwitch
  attachInterrupt(SwitchB[0].Data.PIN_Button, ISR_B0, CHANGE);
  attachInterrupt(SwitchB[1].Data.PIN_Button, ISR_B1, CHANGE);
  attachInterrupt(SwitchB[2].Data.PIN_Button, ISR_B2, CHANGE);
  attachInterrupt(SwitchB[3].Data.PIN_Button, ISR_B3, CHANGE);
#endif //SecondSwitch
  //===========================================================================
  //Wait for all buttons to be NOT pressed
  //===========================================================================
  byte ButtonPressedID = 1;
  while (ButtonPressedID > 0) {
#ifdef SerialEnabled
    Serial.println("Waiting on a button(s) " + String(ButtonPressedID, HEX) + " before starting up");
#endif //SerialEnabled
    ButtonPressedID = 0;                          //Set to NOT pressed by default, will be overwritten
    while (!TickEveryMS(50)) {}           //Wait here for 50ms (so an error blink would be nice)

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
#ifdef SecondSwitch
    for (byte i = 0; i < Amount_Buttons; i++) {
      ButtonID = ButtonID << 1;                     //Move bits 1 to the left (its like *2)
      Button_Time Value = SwitchB[i].CheckButton();
      if (Value.Pressed) {
        ButtonID += 1;                              //Flag this button as on
        digitalWrite(SwitchB[i].Data.PIN_LED, !digitalRead(SwitchB[i].Data.PIN_LED));
      } else
        digitalWrite(SwitchB[i].Data.PIN_LED, LOW);
    }
#endif //SecondSwitch
    ButtonPressedID = ButtonID;   //Get the button state, here 1 is HIGH in the form of '0000<Button 1><2><3><4> '
    if (byte(ButtonPressedID | B00001111) == 255 or byte(ButtonPressedID | B11110000) == 255) { //If a set of 4 buttons are all pressed
      OTA_setup();                        //Enable OTA
      WiFiManager_EnableSetup(true);      //Enable the settings page
    }
  }
  for (byte i = 0; i < Amount_Buttons; i++) {
    digitalWrite(SwitchA[i].Data.PIN_LED, LOW);                 //Make sure all LED's are off
#ifdef SecondSwitch
    digitalWrite(SwitchB[i].Data.PIN_LED, LOW);                 //Make sure all LED's are off
#endif //SecondSwitch
  }
  //===========================================================================
  //Initialise server stuff
  //===========================================================================
  server.on("/",          WiFiManager_handle_Connect);    //Must be declaired before "WiFiManager_Start()" for APMode
  server.on("/setup",     WiFiManager_handle_Settings);   //Must be declaired before "WiFiManager_Start()" for APMode
#ifdef IFTTT
  server.on("/set",       IFTTT_handle_Set);
  server.on("/register",  IFTTT_handle_Register);
#endif //IFTTT
  server.onNotFound(      HandleNotFound);
  //===========================================================================
  //Start WIFI
  //===========================================================================
  byte Answer = WiFiManager_Start();
  if (Answer != 1) {
#ifdef SerialEnabled
    Serial.println("setup error code '" + String(Answer) + "'");
#endif //SerialEnabled
    ESP.restart();                //Restart the ESP
  }
  WiFiManager_StartServer();      //Start the server (so IFTTT and settings etc can be handled)
  //WiFiManager_EnableSetup(true);
#ifdef SerialEnabled
  //===========================================================================
  //Get Reset reason (This could be/is usefull for power outage)
  //===========================================================================
  if (byte a = GetResetReason()) {
    Serial.println("Done with boot, resetted due to");
    print_reset_reason(a);
  }
  /*0xc SW_CPU_RESET  "Core  1 panic'ed (Interrupt wdt timeout on CPU1)"
                    Software reset
    0x1 POWERON_RESET Power on
                    power reset     */
#endif //SerialEnabled
}
//===========================================================================
void loop() {
  server.handleClient();

  if (OTA_loop())                       //handle OTA if needed
    BlinkEveryMs(LED_BUILTIN, 1000);    //Blink every x MS to show OTA is on

  for (byte i = 0; i < Amount_Buttons; i++) {
    Check(SwitchA[i].CheckButton(), LightA, CommandsA[i], SwitchA[i].Data.PIN_LED, i);
#ifdef SecondSwitch
    Check(SwitchB[i].CheckButton(), LightB, CommandsB[i], SwitchB[i].Data.PIN_LED, i);
#endif //SecondSwitch
  }
}
//===========================================================================

void Check(Button_Time Value, MiLight Light, String Action, byte LEDpin, byte ButtonID) {
#ifdef SerialEnabled
  Serial.println("b" + String(ButtonID) + "=" + String(Value.Pressed) + " S=" + String(Value.StartPress) + " L=" + String(Value.PressedLong) + " SL=" + String(Value.StartLongPress) + " LEDpin=" + String(LEDpin));
#endif //SerialEnabled
  if (Value.StartPress) {                               //If button is just pressed in
    if (LEDpin > 0) digitalWrite(LEDpin, HIGH);         //If a LED pin was given; Set that buttons LED on
    byte TriesConnect = 2;
    for (int i = 5; i > 0; i--) {                       //Where i is amount of tries tries to do
      byte Feedback = SetLight(Light, Action);
      if (Feedback == 1) {                              //If done
        i = 0;                                          //Do not execute/try again
      } else if (Feedback == 2) {                       //Can't connect
        TriesConnect--;
        if (TriesConnect == 0)
          i = 0;                                        //Do not execute/try again
      } else {
        if (Feedback == 3)                              //If Json wrong format
          i = 0;                                        //Do not execute/try again
#ifdef SerialEnabled
        Serial.println("Error sending, code=" + String(Feedback));
#endif //SerialEnabled
      }
    }
    if (LEDpin > 0) digitalWrite(LEDpin, LOW);          //If a LED pin was given; Set that buttons LED off
  } else if (Value.StartLongPress) {
    if (ButtonID == 0)
      OTA_setup();
    else if (ButtonID == 1)
      WiFiManager_EnableSetup(true);
    if (LEDpin > 0) digitalWrite(LEDpin, LOW);       //If a LED pin was given; Set that buttons LED off
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
#ifdef SecondSwitch
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
#endif //SecondSwitch
