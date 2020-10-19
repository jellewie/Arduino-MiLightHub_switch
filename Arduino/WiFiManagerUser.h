/* Written by JelleWho https://github.com/jellewie
   https://github.com/jellewie/Arduino-WiFiManager

  These are some examples how to 'hook' functions with code into the WiFiManager.
  This file and all functions are not required, but when defined before '#include "WiFiManagerUser.h"' they will be hooked in to WiFiManager
  if you want to declair functions after including WiFiManager, uses a 'extern <function name>' to declair a dummy here, so the hook will be made, but will be hooked to a later (extern if I may say) declaration

  NOTES
   DO NOT USE char(") in any of input stings on the webpage, use char(') if you need it. char(") will be replaced

  HOW TO ADD CUSTOM VALUES
   -"WiFiManagerUser_VariableNames_Defined" define this, and ass custom names for the values
   -"WiFiManager_VariableNames"  Add the description name of the value to get/set to this list
   -"EEPROM_size"     [optional] Make sure it is big enough for your needs, SIZE_SSID+SIZE_PASS+YourValues (1 byte = 1 character)
   -"Set_Value"       Set the action on what to do on startup with this value
   -"Get_Value"       [optional] Set the action on what to fill in in the boxes in the 'AP settings portal'
*/
//===========================================================================
const byte Pin_LED  = LED_BUILTIN;                              //Just here for some examples, It's the LED to give feedback on (like blink on error)
//===========================================================================
bool WiFiManagerUser_Set_Value(byte ValueID, String Value) {
  switch (ValueID) {                                            //Note the numbers are shifted from what is in memory, 0 is the first user value
    case 0:
      if (Value.length() > sizeof(Name))        return false;   //Length is to long, it would not fit so stop here
      Value.toCharArray(MiLight_IP, 16);        return true;
      break;
    case 1:
      if (Value.length() > sizeof(MiLight_IP))  return false;   //Length is to long, it would not fit so stop here
      Value.toCharArray(MiLight_IP, 16);        return true;
      break;
    case 2:   CommandsA[0] = Value;             return true;    break;
    case 3:   CommandsA[1] = Value;             return true;    break;
    case 4:   CommandsA[2] = Value;             return true;    break;
    case 5:   CommandsA[3] = Value;             return true;    break;
    case 6:   LightA.device_id = Value;         return true;    break;
    case 7:   LightA.remote_type = Value;       return true;    break;
    case 8:   LightA.group_id = Value.toInt();  return true;    break;
    case 9: {
        byte Rotation = ConvertRotationToByte(Value);
        if (Rotation == UNK)                    return false;   //Not set, Rotation is out of range
        if (Rotation == UNUSED)                 return false;   //Switch 1 can not be disabled
        RotationA = Rotation;                   return true;    break;
      } break;
    case 10:  CommandsB[0] = Value;             return true;    break;
    case 11:  CommandsB[1] = Value;             return true;    break;
    case 12:  CommandsB[2] = Value;             return true;    break;
    case 13:  CommandsB[3] = Value;             return true;    break;
    case 14:  LightB.device_id = Value;         return true;    break;
    case 15:  LightB.remote_type = Value;       return true;    break;
    case 16:  LightB.group_id = Value.toInt();  return true;    break;
    case 17: {
        byte Rotation = ConvertRotationToByte(Value);
        if (Rotation == UNK)                    return false;   //Not set, Rotation is out of range
        RotationB = Rotation;                   return true;    break;
      } break;
  }
  return false;                                                 //Report back that the ValueID is unknown, and we could not set it
}
//===========================================================================
String WiFiManagerUser_Get_Value(byte ValueID, bool Safe, bool Convert) {
  //if its 'Safe' to return the real value (for example the password will return '****' or '1234')
  //'Convert' the value to a readable string for the user (bool '0/1' to 'FALSE/TRUE')
  switch (ValueID) {                                            //Note the numbers are shifted from what is in memory, 0 is the first user value
    case 0:   return String(Name);                        break;
    case 1:   return String(MiLight_IP);                  break;
    case 2:   return CommandsA[0];                        break;
    case 3:   return CommandsA[1];                        break;
    case 4:   return CommandsA[2];                        break;
    case 5:   return CommandsA[3];                        break;
    case 6:   return LightA.device_id;                    break;
    case 7:   return LightA.remote_type;                  break;
    case 8:   return String(LightA.group_id);             break;
    case 9:   return ConvertRotationToString(RotationA);  break;
    case 10:  return CommandsB[0];                        break;
    case 11:  return CommandsB[1];                        break;
    case 12:  return CommandsB[2];                        break;
    case 13:  return CommandsB[3];                        break;
    case 14:  return LightB.device_id;                    break;
    case 15:  return LightB.remote_type;                  break;
    case 16:  return String(LightB.group_id);             break;
    case 17:  return ConvertRotationToString(RotationB);  break;
  }
  return "";
}
//===========================================================================
void WiFiManagerUser_Status_Start() { //Called before start of WiFi
  pinMode(Pin_LED, OUTPUT);
  digitalWrite(Pin_LED, HIGH);
}
//===========================================================================
void WiFiManagerUser_Status_Done() { //Called after succesfull connection to WiFi
  digitalWrite(Pin_LED, LOW);
}
//===========================================================================
void WiFiManagerUser_Status_Blink() { //Used when trying to connect/not connected
  digitalWrite(Pin_LED, !digitalRead(Pin_LED));
}
//===========================================================================
void WiFiManagerUser_Status_StartAP() {}
//===========================================================================
bool WiFiManagerUser_HandleAP() {                               //Called when in the While loop in APMode, this so you can exit it
  //Return true to leave APmode
#define TimeOutApMode 15 * 60 * 1000;                           //Example for a timeout, (time in ms)
  unsigned long StopApAt = millis() + TimeOutApMode;
  if (millis() > StopApAt)    return true;                      //If we are running for to long, then flag we need to exit APMode
  return false;
}
