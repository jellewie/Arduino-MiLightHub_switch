/*
  Program written by JelleWho
  Board: https://dl.espressif.com/dl/package_esp32_index.json

  FIXME?
  On boot button stuck, when released, the buttonpressed interupt has happened, and pressed/longpressed will be executed

*/
#include <WiFi.h>             //Needed for WiFi stuff
#include <WiFiClient.h>       //Needed for sending data to devices
#include <WebServer.h>        //This is to create a acces point with wifimanager if no wifi is set
#include <ArduinoJson.h>      //This is to pack/unpack data send to the hub
#include <rom/rtc.h>          //This is for rtc_get_reset_reason
WebServer server(80);
#include "functions.h"
#include "miLight.h"
#include "switch.h"
//////////////////////////////////////////////////////////////////////
//  User Settings
//////////////////////////////////////////////////////////////////////
#define SerialEnabled true
#define SecondSwitch

const MiLight MilightLight1  = {"0xF001", "rgb_cct", 2};
const MiLight MilightLight2  = {"0xF002", "rgb_cct", 1};

const MiLight LightA = MilightLight1;                         //What light to control
Switch SwitchA = Pin_Switch({34, 35, 32, 33, 21, 19, 18,  5});   //Orginal layout
//Switch SwitchA = Pin_Switch({35, 33, 34, 32, 19,  5, 21, 18});   //case 90  clockwise to PCB
//Switch SwitchA = Pin_Switch({33, 32, 35, 34,  5, 18, 19, 21});   //case 180 clockwise to PCB
//Switch SwitchA = Pin_Switch({32, 34, 33, 35, 18, 21,  5, 19});   //case 270 clockwise to PCB
String CommandsA[Amount_Buttons] = {"{\"commands\":[\"toggle\"]}",
                                    "{\"brightness\":1,\"color\":\"255,0,0\",\"state\":\"On\"}",
                                    "{\"brightness\":128,\"color\":\"255,0,0\",\"state\":\"On\"}",
                                    "{\"brightness\":255,\"color\":\"255,255,255\",\"state\":\"On\"}"
                                   };
#ifdef SecondSwitch
const MiLight LightB = MilightLight2;                         //What light to control
Switch SwitchB = Pin_Switch({26, 27, 14, 12, 23, 22,  4, 15});   //Orginal layout
//Switch SwitchB = Pin_Switch({27, 12, 26, 14, 22, 15, 23,  4});   //case 90  clockwise to PCB
//Switch SwitchB = Pin_Switch({12, 14, 27, 26, 15,  4, 22, 23});   //case 180 clockwise to PCB
//Switch SwitchB = Pin_Switch({14, 26, 12, 27,  4, 23, 15, 22});   //case 270 clockwise to PCB
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
  attachInterrupt(SwitchA.Data.Button[0], ISR_A0, CHANGE);
  attachInterrupt(SwitchA.Data.Button[1], ISR_A1, CHANGE);
  attachInterrupt(SwitchA.Data.Button[2], ISR_A2, CHANGE);
  attachInterrupt(SwitchA.Data.Button[3], ISR_A3, CHANGE);
#ifdef SecondSwitch
  attachInterrupt(SwitchB.Data.Button[0], ISR_B0, CHANGE);
  attachInterrupt(SwitchB.Data.Button[1], ISR_B1, CHANGE);
  attachInterrupt(SwitchB.Data.Button[2], ISR_B2, CHANGE);
  attachInterrupt(SwitchB.Data.Button[3], ISR_B3, CHANGE);
#endif //SecondSwitch
  //===========================================================================
  //Wait for all buttons to be NOT pressed
  //===========================================================================
  byte Buttons = 1;
  while (Buttons > 0) {
#ifdef SerialEnabled
    Serial.println("Waiting on a button(s) " + String(Buttons, HEX) + " before starting up");
#endif //SerialEnabled
    Buttons = 0;                          //Set to NOT pressed by default, will be overwritten
    while (!TickEveryMS(50)) {}           //Wait here for 50ms (so an error blink would be nice)
    Buttons = SwitchA.aButtonPressed();   //Get the button state, here 1 is HIGH in the form of '0000<Button 1><2><3><4> '
#ifdef SecondSwitch
    Buttons = Buttons << 4;               //Move it over, so we now have '<Bit 1=Button A1><A2><A3><A4> 0000'
    Buttons += SwitchB.aButtonPressed();  //Add 2rd button data, so we now have '<A1><A2><A3><A4> <B1><B2><B3><B4>'
#endif //SecondSwitch
    if (byte(Buttons | B00001111) == 255 or byte(Buttons | B11110000) == 255) { //If a set of 4 buttons are all pressed
      OTA_setup();                        //Enable OTA
      WiFiManager_EnableSetup(true);      //Enable the settings page
    }
  }
  SwitchA.LEDsOff();                      //Make sure all LED's are off
#ifdef SecondSwitch
  SwitchB.LEDsOff();                      //Make sure all LED's are off
#endif //SecondSwitch
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
  //===========================================================================
  //Get Reset reason (This could be/is usefull for power outage)
  //===========================================================================
  if (byte a = GetResetReason()) {
#ifdef SerialEnabled
    Serial.println("Done with boot, resetted due to");
    print_reset_reason(a);
#endif //SerialEnabled
  }
  /*
    0xc SW_CPU_RESET  "Core  1 panic'ed (Interrupt wdt timeout on CPU1)"
                      Software reset
    0x1 POWERON_RESET Power on
                      power reset
  */
}
//===========================================================================
void loop() {
  server.handleClient();

  if (OTA_loop())                       //handle OTA if needed
    BlinkEveryMs(LED_BUILTIN, 1000);    //Blink every x MS to show OTA is on

  Check(SwitchA.CheckButtons(), LightA, CommandsA);
#ifdef SecondSwitch
  Check(SwitchB.CheckButtons(), LightB, CommandsB);
#endif //SecondSwitch
}
//===========================================================================
void Check(Button_Time Value, MiLight Light, String Action[Amount_Buttons] ) {
  if (Value.Button > 0) {
#ifdef SerialEnabled
    Serial.println("b" + String(Value.Button) + "=" + String(Value.Pressed) + " S=" + String(Value.StartPress) + " L=" + String(Value.PressedLong) + " SL=" + String(Value.StartLongPress) + " LED" + String(Value.LED));
#endif //SerialEnabled
    if (Value.StartPress) {                                  //If button is just pressed in
      if (Value.LED > 0) digitalWrite(Value.LED, HIGH);       //If a LED pin was given; Set that buttons LED on
      byte TriesConnect = 2;
      for (int i = 5; i > 0; i--) {   //Where i is amount of tries tries to do
        byte Feedback = SetLight(Light, Action[Value.Button - 1]);
        if (Feedback == 1) {          //If done
          i = 0;                      //Do not execute/try again
        } else if (Feedback == 2) {   //Can't connect
          TriesConnect--;
          if (TriesConnect == 0)
            i = 0;                    //Do not execute/try again
        } else {
          if (Feedback == 3)          //If Json wrong format
            i = 0;                    //Do not execute/try again
          Serial.println("Error sending, code=" + String(Feedback));
        }
      }
      if (Value.LED > 0) digitalWrite(Value.LED, LOW);       //If a LED pin was given; Set that buttons LED off
    } else if (Value.StartLongPress) {
      if (Value.Button == 1)
        OTA_setup();
      else if (Value.Button == 2)
        WiFiManager_EnableSetup(true);
      if (Value.LED > 0) digitalWrite(Value.LED, LOW);       //If a LED pin was given; Set that buttons LED off
    }
    //    delay(100);
    if (Value.PressedLong) {                    //If it is/was a long press
      if (Value.Pressed) {                      //If we are still pressing
        if (Value.PressedTime > Time_ESPrestartMS - 1000) {
          if (Value.LED > 0)BlinkEveryMs(Value.LED, 10);       //If a LED pin was given; Blink that button LED
        } else
          digitalWrite(LED_BUILTIN, HIGH);
      } else {
        digitalWrite(LED_BUILTIN, LOW);
        if (Value.LED > 0) digitalWrite(Value.LED, LOW);       //If a LED pin was given; Blink that button LED
      }
    }
  }
}
//===========================================================================
//      State_rgb_cct a = GetLight(Light);
//      Serial.print("state=" + String(a.state) + " brightness=" + String(a.brightness) + " color_temp=" + String(a.color_temp));
//      Serial.println(" RGB=" + String(a.R) + "," + String(a.G) + "," + String(a.B));
//===========================================================================
void ISR_A0() {
  SwitchA.Pinchange(0);
}
void ISR_A1() {
  SwitchA.Pinchange(1);
}
void ISR_A2() {
  SwitchA.Pinchange(2);
}
void ISR_A3() {
  SwitchA.Pinchange(3);
}
#ifdef SecondSwitch
void ISR_B0() {
  SwitchB.Pinchange(0);
}
void ISR_B1() {
  SwitchB.Pinchange(1);
}
void ISR_B2() {
  SwitchB.Pinchange(2);
}
void ISR_B3() {
  SwitchB.Pinchange(3);
}
#endif //SecondSwitch
