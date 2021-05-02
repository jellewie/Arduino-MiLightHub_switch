/* Written by JelleWho https://github.com/jellewie
   https://github.com/jellewie/Arduino-WiFiManager

  These are some examples how to 'hook' functions with code into the WiFiManager.
  This file and all functions are not required, but when defined before '#include "WiFiManagerUser.h"' they will be hooked in to WiFiManager
  if you want to declair functions after including WiFiManager, uses a 'extern <function name>' to declair a dummy here, so the hook will be made, but will be hooked to a later (extern if I may say) declaration

  NOTES
   DO NOT USE char(") in any of input stings on the webpage, use char(') if you need it. char(") will be replaced
   These are the declaired triggered and function names: you can declare over these to overwrite them but be carefull
      server.on("/",        WiFiManager_handle_Connect);
      server.on("/ip",      WiFiManager_handle_Connect);     //Just as backup, so the "/" can be overwritten by user 
      server.on("/setup",   WiFiManager_handle_Settings);
      server.on("/ota",     WiFiManager_OTA_handle_uploadPage);
      server.on("/update",  HTTP_POST, WiFiManager_OTA_handle_update, WiFiManager_OTA_handle_update2);

  HOW TO ADD CUSTOM VALUES
   -"WiFiManagerUser_VariableNames_Defined" define this, and ass custom names for the values
   -"WiFiManager_VariableNames"  Add the description name of the value to get/set to this list
   -"EEPROM_size"     [optional] Make sure it is big enough for your needs, SIZE_SSID+SIZE_PASS+YourValues (1 byte = 1 character)
   -"Set_Value"       Set the action on what to do on startup with this value
   -"Get_Value"       [optional] Set the action on what to fill in in the boxes in the 'AP settings portal'
*/
//===========================================================================
// Things that need to be defined before including "WiFiManager.h"
//===========================================================================
#define WiFiManagerUser_Set_Value_Defined                       //Define we want to hook into WiFiManager
#define WiFiManagerUser_Get_Value_Defined                       //^
#define WiFiManagerUser_Status_Start_Defined                    //^
#define WiFiManagerUser_Status_Done_Defined                     //^
#define WiFiManagerUser_Status_Blink_Defined                    //^
#define WiFiManagerUser_Status_StartAP_Defined                  //^
#define WiFiManagerUser_HandleAP_Defined                        //^

#define WiFiManager_DoRequest                               //Adds a simple way to do stable URL request (with optional json)
#define WiFiManager_Restart                                     //Adds a simple handle "/restart" to restart the ESP

#define WiFiManagerUser_VariableNames_Defined           //Define that we want to use the custom user variables (Dont forget to settup WiFiManager_VariableNames and WiFiManager_Settings)
const String WiFiManager_VariableNames[] {"SSID", "Password", "Name", "MiLight_IP",
  "Button A1", "Button A2", "Button A3", "Button A4", "LightA ID", "LightA type", "LightA group", "Rotation A",
  "Button B1", "Button B2", "Button B3", "Button B4", "LightB ID", "LightB type", "LightB group", "Rotation B"
};
const int EEPROM_size = 512;                                    //Max Amount of chars for 'SSID(16) + PASSWORD(16) + extra custom vars(?) +1(NULL)' defaults to 33

#define WiFiManagerUser_Name_Defined
char Name[16] = "milight-switch";                               //The mDNS, WIFI APmode SSID name. This requires a restart to apply, can only be 16 characters long, and special characters are not recommended.

#define WiFiManager_mDNS

#define WiFiManager_OTA
#define WiFiManagerUser_UpdateWebpage_Defined
const String UpdateWebpage = "https://github.com/jellewie/Arduino-MiLightHub_switch/releases";
