/* Written by JelleWho https://github.com/jellewie
   https://github.com/jellewie/Arduino-WiFiManager

   Basic function:
   1. Load EEPROM data (if not yet done)
   2. while(no data) Set up AP mode and wait for user data
   3. try connecting, if (not) {GOTO 2}

   NOTES
   DO NOT USE char(") in any of input stings on the webpage, use char(') if you need it. char(") will be replaced

   HOW TO ADD CUSTOM VALUES
   -"WiFiManager_VariableNames" Add the 'AP settings portal' name
   -"EEPROM_size"   [optional] Make sure it is big enough for your needs, SIZE_SSID+SIZE_PASS+YourValues (1 byte = 1 character)
   -"Set_Value"     Set the action on what to do on startup with this value
   -"Get_Value"     [optional] Set the action on what to fill in in the boxes in the 'AP settings portal'
*/
#ifndef WifiManager_h                               //This prevents including this file more than once
#define WifiManager_h

#define WiFiManager_SerialEnabled                 //Disable to not send Serial debug feedback
//#define dnsServerEnabled
#ifdef dnsServerEnabled
#include <DNSServer.h>
DNSServer dnsServer;
#endif //dnsServerEnabled

#include <EEPROM.h>

#ifdef SecondSwitch
const String WiFiManager_VariableNames[] {"SSID", "Password", "MiLight_IP", "Button A1", "Button A2", "Button A3", "Button A4", "LightA ID", "LightA type", "LightA group", "Button B1", "Button B2", "Button B3", "Button B4", "LightB ID", "LightB type", "LightB group"};
#else
const String WiFiManager_VariableNames[] {"SSID", "Password", "MiLight_IP", "Button A1", "Button A2", "Button A3", "Button A4", "LightA ID", "LightA type", "LightA group"};
#endif //SecondSwitch
const byte WiFiManager_Settings = sizeof(WiFiManager_VariableNames) / sizeof(WiFiManager_VariableNames[0]); //Why filling this in if we can automate that? :)
bool WiFiManager_Connected;                         //If the ESP is WiFiManager_Connected to WIFI

class CWiFiManager {
  public:
    const char EEPROM_Seperator = char(9);          //use 'TAB' as a seperator
    const int ConnectionTimeOutMS = 10000;
    bool SettingsEnabled = false;                   //This holds the flag to enable settings, else it would not responce to settings commands
    bool WaitOnAPMode = true;                       //This holds the flag if we should wait in Apmode for data
    int EEPROM_USED = 0;                            //Howmany bytes we have used for data in the EEPROM
    //==============================
    //User variables
    //==============================
    char password[16] = "";                         //Also efines howmany characters can be in the SSID
    char ssid[16] = "";                             //^
    //#define strip_ip, gateway_ip, subnet_mask to use static IP
    char APSSID[16] = "ESP32";
    const int EEPROM_size = 512;                    //Max Amount of chars of 'SSID + PASSWORD' (+1) (+extra custom vars)
    const byte Pin_LED  = LED_BUILTIN;              //The LED to give feedback on (like blink on error)
    bool Set_Value(byte ValueID, String Value) {
      //From EEPROM to RAM
#ifdef WiFiManager_SerialEnabled
      Serial.println("WM: Set current value: " + String(ValueID) + " = " + Value);
#endif //WiFiManager_SerialEnabled
      switch (ValueID) {
        case 1:
          Value.toCharArray(ssid, Value.length() + 1);
          break;
        case 2:
          for (byte i = 0; i < String(Value).length(); i++) {
            if (Value.charAt(i) != '*') {           //if the password is set (and not just the '*****' we have given the client)
              Value.toCharArray(password, Value.length() + 1);
              return true;                          //Stop for loop
            }
          }
          return false;                             //Not set, the password was just '*****'
          break;
        case 3:
          Value.toCharArray(MiLight_IP, 16);
          break;
        case 4:
          CommandsA[0] = Value;
          break;
        case 5:
          CommandsA[1] = Value;
          break;
        case 6:
          CommandsA[2] = Value;
          break;
        case 7:
          CommandsA[3] = Value;
          break;
        case 8:
          LightA.device_id = Value;
          break;
        case 9:
          LightA.remote_type = Value;
          break;
        case 10:
          LightA.group_id = Value.toInt();
          break;
#ifdef SecondSwitch
        case 11:
          CommandsB[0] = Value;
          break;
        case 12:
          CommandsB[1] = Value;
          break;
        case 13:
          CommandsB[2] = Value;
          break;
        case 14:
          CommandsB[3] = Value;
          break;
        case 15:
          LightB.device_id = Value;
          break;
        case 16:
          LightB.remote_type = Value;
          break;
        case 17:
          LightB.group_id = Value.toInt();
          break;
#endif //SecondSwitch
      }
      return true;
    }
    String Get_Value(byte ValueID, bool Safe, bool Convert) {
      //Safe == true will return the real password,
      //From RAM to EEPROM
#ifdef WiFiManager_SerialEnabled
      Serial.print("WM: Get current value of: " + String(ValueID) + " safe=" + String(Safe) + " conv=" + String(Convert));
#endif //WiFiManager_SerialEnabled
      String Return_Value = "";                     //Make sure to return something, if we return bad data of NULL, the HTML page will break
      switch (ValueID) {
        case 1:
          Return_Value += String(ssid);
          break;
        case 2:
          if (Safe)                                 //If's it's safe to return password.
            Return_Value += String(password);
          else {
            for (byte i = 0; i < String(password).length(); i++)
              Return_Value += "*";
          }
          break;
        case 3:
          Return_Value = String(MiLight_IP);
          break;
        case 4:
          Return_Value = CommandsA[0];
          break;
        case 5:
          Return_Value = CommandsA[1];
          break;
        case 6:
          Return_Value = CommandsA[2];
          break;
        case 7:
          Return_Value = CommandsA[3];
          break;
        case 8:
          Return_Value = LightA.device_id;
          break;
        case 9:
          Return_Value = LightA.remote_type;
          break;
        case 10:
          Return_Value = LightA.group_id;
          break;
#ifdef SecondSwitch
        case 11:
          Return_Value = CommandsB[0];
          break;
        case 12:
          Return_Value = CommandsB[1];
          break;
        case 13:
          Return_Value = CommandsB[2];
          break;
        case 14:
          Return_Value = CommandsB[3];
          break;
        case 15:
          Return_Value = LightB.device_id;
          break;
        case 16:
          Return_Value = LightB.remote_type;
          break;
        case 17:
          Return_Value = LightB.group_id;
          break;
#endif //SecondSwitch
      }
#ifdef WiFiManager_SerialEnabled
      Serial.println(" = " + Return_Value);
#endif //WiFiManager_SerialEnabled
      Return_Value.replace("\"", "'");              //Make sure to change char("), since we can't use that, change to char(')
      Return_Value.replace(String(EEPROM_Seperator), " ");  //Make sure to change the EEPROM seperator, since we can't use that
      return String(Return_Value);
    }
    void Status_Start() {
      pinMode(Pin_LED, OUTPUT);
      digitalWrite(Pin_LED, HIGH);
    }
    void Status_Done() {
      digitalWrite(Pin_LED, LOW);
    }
    void Status_Blink() {
      digitalWrite(Pin_LED, !digitalRead(Pin_LED));
    }
    void Status_StartAP() {

    }
    bool HandleAP() {                               //Called when in the While loop in APMode, this so you can exit it
      //#define TimeOutApMode 15 * 60 * 1000;       //Example for a timeout, re-enable these 3 lines to apply. (time in ms)
      //  unsigned long StopApAt = millis() + TimeOutApMode;
      //  if (millis() > StopApAt) return true;     //If we are running for to long, then flag we need to exit APMode
      return false;
    }
    //==============================
    //End of user variables
    //==============================
    void StartServer() {                            //Start the webserver
      static bool ServerStarted = false;
      if (!ServerStarted) {                         //If the server hasn't started yet
        ServerStarted = true;
        server.begin();                             //Begin server
      }
    }
    void EnableSetup(bool State) {                  //Enable/disable setup page
#ifdef WiFiManager_SerialEnabled
      if (State) {
        if (WiFiManager_Connected)
          Serial.println("WM: Settings page online");
        else {
          Serial.print("WM: Settings page online ip=");
          Serial.println(WiFi.softAPIP());
        }
      } else
        Serial.println("WM: Settings page offline");
#endif //WiFiManager_SerialEnabled
      SettingsEnabled = State;
    }
    bool TickEveryMS(int _Delay) {
      static unsigned long _LastTime = 0;           //Make it so it returns 1 if called for the FIST time
      if (millis() > _LastTime + _Delay) {
        _LastTime = millis();
        return true;
      }
      return false;
    }
    byte APMode() {                                 //Start a WIFI APmode
      //IP of AP = 192.168.4.1
      /* <Return> <meaning>
        2 Soft-AP setup Failed
        3 custom exit
      */
      if (!WiFi.softAP(APSSID))                     //config doesn't seem to work, so do not use it: 'WiFi.softAPConfig(ap_local_IP, ap_gateway, ap_subnet)'
        return 2;
      Status_StartAP();
      EnableSetup(true);                            //Flag we need to responce to settings commands
      StartServer();                                //start server (if we havn't already)
#ifdef dnsServerEnabled
      dnsServer.start(53, "*", IPAddress(192, 168, 4, 1)); //Start a DNS server at the default DNS port, and send ALL trafic to it OWN IP (DNS_port, DNS_domainName, DNS_resolvedIP)
#endif //dnsServerEnabled
#ifdef WiFiManager_SerialEnabled
      Serial.print("WM: APMode on; SSID=" + String(APSSID) + " ip=");
      Serial.println(WiFi.softAPIP());
#endif //WiFiManager_SerialEnabled
      while (WaitOnAPMode) {
        if (TickEveryMS(100)) Status_Blink();       //Let the LED blink to show we are not WiFiManager_Connected
        server.handleClient();
#ifdef dnsServerEnabled
        dnsServer.processNextRequest();
#endif //dnsServerEnabled
        if (HandleAP()) {
#ifdef SerialEnabled
          Serial.println("WM: Manual leaving APMode");
#endif //SerialEnabled
          EnableSetup(false);                       //Flag to stop responce to settings commands
#ifdef dnsServerEnabled
          dnsServer.stop();
#endif //dnsServerEnabled
          return 3;
        }
      }
      //dnsServer.stop();
#ifdef WiFiManager_SerialEnabled
      Serial.println("WM: Leaving APmode");
#endif //WiFiManager_SerialEnabled
      WaitOnAPMode = true;                          //reset flag for next time
      EnableSetup(false);                           //Flag to stop responce to settings commands
      return 1;
    }
    String LoadEEPROM() {                           //Get raw data from EEPROM
      String Value;
#ifdef WiFiManager_SerialEnabled
      Serial.print("WM: EEPROM LOAD");
#endif //WiFiManager_SerialEnabled
      for (int i = 0; i < EEPROM_size; i++) {
        byte Input = EEPROM.read(i);
        if (Input == 255) {                         //If at the end of data
#ifdef WiFiManager_SerialEnabled
          Serial.println();
#endif //WiFiManager_SerialEnabled
          EEPROM_USED = Value.length();
          return Value;                             //Stop and return all data stored
        }
        if (Input == 0) {                           //If no data found (NULL)
          EEPROM_USED = Value.length();
          return String(EEPROM_Seperator);
        }
        Value += char(Input);
#ifdef WiFiManager_SerialEnabled
        Serial.print("_" + String(char(Input)) + "_");
#endif //WiFiManager_SerialEnabled
      }
#ifdef WiFiManager_SerialEnabled
      Serial.println();
#endif //WiFiManager_SerialEnabled
      EEPROM_USED = Value.length();
      return String(EEPROM_Seperator);              //ERROR; [maybe] not enough space
    }
    byte LoadData() {                               //Only load data from EEPROM to memory
      if (!EEPROM.begin(EEPROM_size))
        return 2;
      String Value = LoadEEPROM();
#ifdef WiFiManager_SerialEnabled
      Serial.println("WM: EEPROM data=" + Value);
#endif //WiFiManager_SerialEnabled
      if (Value != String(EEPROM_Seperator)) {      //If there is data in EEPROM
        for (byte i = 1; i < WiFiManager_Settings + 1; i++) {
          byte j = Value.indexOf(char(EEPROM_Seperator));
          if (j == 255)
            j = Value.length();
          String _Value = Value.substring(0, j);
          if (_Value != "")                         //If there is a value
            Set_Value(i, _Value);                   //set the value in memory (and thus overwrite the Hardcoded stuff)
          Value = Value.substring(j + 1);
        }
      }
      return 0;
    }
    bool Connect(int TimeOutMS) {
#ifdef WiFiManager_SerialEnabled
      Serial.println("WM: Connecting to ssid='" + String(ssid) + "' password='" + String(password) + "'");
#endif //WiFiManager_SerialEnabled
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
#if defined(strip_ip) && defined(gateway_ip) && defined(subnet_mask)
      WiFi.config(strip_ip, gateway_ip, subnet_mask);
#endif
      unsigned long StopTime = millis() + TimeOutMS;
      while (WiFi.status() != WL_CONNECTED) {
        if (millis() > StopTime) {                  //If we are in overtime
#ifdef WiFiManager_SerialEnabled
          Serial.println("WM: Could not connect within " + String(TimeOutMS) + "ms to given SSID, aborting with code " + ConvertWifistatus(WiFi.status()));
#endif //WiFiManager_SerialEnabled
          return false;
        }
        if (TickEveryMS(500)) Status_Blink();       //Let the LED blink to show we are trying to connect
      }
      return true;
    }
    byte Start() {                                  //Start all WIFI stuff
      Status_Start();
      //starts wifi stuff, only returns when WiFiManager_Connected. will create Acces Point when needed
      /* <Return> <meaning>
         2 Can't begin EEPROM
         3 Can't write [all] data to EEPROM
      */
      if (ssid[0] == 0 and password[0] == 0)        //If the ssid and password are not yet in memory
        if (byte temp = LoadData()) return temp;    //load the EEPROM to get the ssid and password. Exit with code if failed
      bool FlagApMode = false;
      while (!WiFiManager_Connected) {
        if ((strlen(ssid) == 0 or strlen(password) == 0 or FlagApMode)) {
          FlagApMode = false;
          APMode();                                 //No ssid or password given, or ssid not found. Entering APmode
        } else {
          if (Connect(ConnectionTimeOutMS)) //try to WiFiManager_Connected to ssid password
            WiFiManager_Connected = true;
          else
            FlagApMode = true;                      //Flag so we will enter AP mode
        }
      }
      Status_Done();
#ifdef WiFiManager_SerialEnabled
      Serial.print("WM: Connected; SSID=" + String(ssid) + " ip=");
      Serial.println(WiFi.localIP());
#endif //WiFiManager_SerialEnabled
      WiFiManager_Connected = true;
      return 1;
    }
    bool WriteEEPROM() {
      String Value;                                 //Save to mem:
      for (byte i = 0; i < WiFiManager_Settings; i++) {
        Value += Get_Value(i + 1, true, false);     //^     <Seperator>
        if (WiFiManager_Settings - i > 1)
          Value += EEPROM_Seperator;                //^            <Value>  (only if there more values)
      }
      Value += char(255);                           //^            <emthy bit> (we use a emthy bit to mark the end)
#ifdef WiFiManager_SerialEnabled
      Serial.println("WM: EEPROM WRITE; '" + Value + "'");
#endif //WiFiManager_SerialEnabled
      if (Value.length() > EEPROM_size)             //If not enough room in the EEPROM
        return false;                               //Return false; not all data is stored
      for (int i = 0; i < Value.length(); i++)      //For each character to save
        EEPROM.write(i, (int)Value.charAt(i));      //Write it to the EEPROM
      EEPROM.commit();
      EEPROM_USED = Value.length();
      return true;
    }
    bool RunServer() {
      if (WiFiManager_Connected) server.handleClient();
      return WiFiManager_Connected;
    }
    void handle_Connect() {
      if (!SettingsEnabled) return;                 //If settingscommand is disabled: Stop right away, and do noting
      String HTML = "<strong>" + String(APSSID) + " settings</strong><br><br><form action=\"/setup?\" method=\"get\">";
      for (byte i = 1; i < WiFiManager_Settings + 1; i++)
        HTML += "<div><label>" + WiFiManager_VariableNames[i - 1] + " </label><input type=\"text\" name=\"" + i + "\" value=\"" + Get_Value(i, false, true) + "\"></div>";
      HTML += "<button>Send</button></form>"
              "" + String(EEPROM_USED) + "/" + String(EEPROM_size) + " Bytes used<br>"
              "MAC adress = " +  String(WiFi.macAddress());
      server.send(200, "text/html", HTML);
    }
    void handle_Settings() {
      if (!SettingsEnabled) return;                 //If settingscommand is disabled: Stop right away, and do noting
      String HTML = "";
      int    Code = 200;
      for (int i = 0; i < server.args(); i++) {
        int j = server.argName(i).toInt();
        String ArgValue = server.arg(i);
        ArgValue.trim();
        if (j > 0 and j < 255 and ArgValue != "") {
          if (Set_Value(j, ArgValue))
            HTML += "Succesfull '" + String(j) + "'='" + ArgValue + "'\n";
          else
            HTML += "ERROR Set; '" + String(j) + "'='" + ArgValue + "'\n";
        } else {
          Code = 422;   //Flag we had a error
          HTML += "ERROR ID; '" + server.argName(i) + "'='" + ArgValue + "'\n";
        }
      }
      WaitOnAPMode = false;                         //Flag we have input data, and we can stop waiting in APmode on data
      WriteEEPROM();
      HTML += String(EEPROM_USED) + "/" + String(EEPROM_size) + " Bytes used";
      server.send(Code, "text/plain", HTML);
      for (byte i = 50; i > 0; i--) {               //Add some delay here, to send feedback to the client, i is delay in MS to still wait
        server.handleClient();
        delay(1);
      }
      static String OldSSID = ssid;
      static String Oldpassword = password;
      if (OldSSID != String(ssid) or Oldpassword != String(password)) {
#ifdef WiFiManager_SerialEnabled
        Serial.println("WM: Auto disconnect, new SSID recieved, from " + OldSSID + " to " + String(ssid));
#endif //WiFiManager_SerialEnabled
        OldSSID = String(ssid);
        Oldpassword = String(password);
        WiFiManager_Connected = false;              //Flag that WIFI is off, and we need to reconnect (In case user requested to switch WIFI)
      }
    }
    void CheckAndReconnectIfNeeded() {
      //Checks if WIFI is connected, and if so tries to reconnect
      if (String(WiFi.localIP()) = "0.0.0.0") Start();
    }
#ifdef WiFiManager_SerialEnabled
    String ConvertWifistatus(byte IN) {
      switch (IN) {
        case WL_CONNECTED:
          return "WL_CONNECTED";
          break;
        case WL_NO_SHIELD:
          return "WL_NO_SHIELD";
          break;
        case WL_IDLE_STATUS:
          return "WL_IDLE_STATUS";
          break;
        case WL_NO_SSID_AVAIL:
          return "WL_NO_SSID_AVAILABLE";
          break;
        case WL_SCAN_COMPLETED:
          return "WL_SCAN_COMPLETED";
          break;
        case WL_CONNECT_FAILED:
          return "WL_CONNECT_FAILED";
          break;
        case WL_CONNECTION_LOST:
          return "WL_CONNECTION_LOST";
          break;
        case WL_DISCONNECTED:
          return "WL_DISCONNECTED";
          break;
      }
      return "UNKNOWN";
    }
#endif //WiFiManager_SerialEnabled
};
CWiFiManager WiFiManager;

//ISR must return nothing and take no arguments, so we need this sh*t
void WiFiManager_handle_Connect() {
  WiFiManager.handle_Connect();
}
void WiFiManager_handle_Settings() {
  WiFiManager.handle_Settings();
}
void WiFiManager_CheckAndReconnectIfNeeded() {
  WiFiManager.CheckAndReconnectIfNeeded();
}

#endif
