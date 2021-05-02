enum {UNUSED, NORMAL, LEFT, UPSIDE_DOWN, RIGHT, UNK};
String RotationNames[] = {"UNUSED", "NORMAL", "LEFT", "UPSIDE_DOWN", "RIGHT", "UNK"};
const byte Rotation_Amount = sizeof(RotationNames) / sizeof(RotationNames[0]); //Why filling this in if we can automate that? :)

bool StringisDigit(String IN) {
  for (byte i = 0; i < IN.length(); i++) {
    if (not isDigit(IN.charAt(i)))
      return false;
  }
  return true;
}
byte ConvertRotationToByte(String IN) {
#ifdef Convert_Log
  Log.Add("CV: ConvertRotationToByte '" + String(IN) + "'");
#endif //Convert_Log
  if (StringisDigit(IN)) {
    if (IN.toInt() < Rotation_Amount)
      return IN.toInt();
    else
      return UNK;
  }
  IN.trim();
  IN.toUpperCase();
  for (byte i = 0; i < Rotation_Amount; i++) {
    if (IN == RotationNames[i])
      return i;
  }
  return UNK;
}
String ConvertRotationToString(byte IN) {
#ifdef Convert_Log
  Log.Add("CV: ConvertRotationToString '" + String(IN) + "'");
#endif //Convert_Log
  if (IN < Rotation_Amount)
    return RotationNames[IN];
  return "UNK";
}
void HandleNotFound() {
#ifdef SerialEnabled
  Serial.println("Method: " + String(server.method()) + " URI: " + server.uri());
  for (int i = 0; i < server.args(); i++)
    Serial.println("arg '" + String(server.argName(i)) + "' = '" + String(server.arg(i)) + "'");
#endif //SerialEnabled
}
void Blink_Amount(byte _LED, int _DelayMS, byte _amount) {
  for (byte i = 0; i < _amount; i++) {
    digitalWrite(_LED, !digitalRead(_LED)); //Blink LED
    delay(_DelayMS);
  }
  digitalWrite(_LED, LOW);
}
bool TickEveryMS(int _Delay) {
  static unsigned long _LastTime = 0;       //Make it so it returns 1 if called for the FIST time
  if (millis() > _LastTime + _Delay) {
    _LastTime = millis();
    return true;
  }
  return false;
}
void BlinkEveryMs(byte _LED, int _Delay) {
  if (TickEveryMS(_Delay))
    digitalWrite(_LED, !digitalRead(_LED)); //Blink LED
}
String macToStr(const byte* _mac) {
  String _result;
  for (int i = 0; i < 6; ++i) {
    _result += String(_mac[i], 16);
    if (i < 5)
      _result += ':';
  }
  return _result;
}
#ifdef SerialEnabled
String ResetReasonToString(byte Reason) {
  //https://github.com/espressif/esp-idf/blob/release/v3.0/components/esp32/include/rom/rtc.h#L80
  switch (Reason) {
    case 0  : return "NO_MEAN"; break;
    case 1  : return "POWERON_RESET"; break;           /**<1,  Vbat power on reset                         Rebooted by; power |or| Reset button |or| Software upload USB*/
    case 3  : return "SW_RESET"; break;                /**<3,  Software reset digital core                 rebooted by; software command "ESP.restart();"*/
    case 4  : return "OWDT_RESET"; break;              /**<4,  Legacy watch dog reset digital core         */
    case 5  : return "DEEPSLEEP_RESET"; break;         /**<5,  Deep Sleep reset digital core               */
    case 6  : return "SDIO_RESET"; break;              /**<6,  Reset by SLC module, reset digital core     */
    case 7  : return "TG0WDT_SYS_RESET"; break;        /**<7,  Timer Group0 Watch dog reset digital core   */
    case 8  : return "TG1WDT_SYS_RESET"; break;        /**<8,  Timer Group1 Watch dog reset digital core   */
    case 9  : return "RTCWDT_SYS_RESET"; break;        /**<9,  RTC Watch dog Reset digital core            */
    case 10 : return "INTRUSION_RESET"; break;         /**<10, Instrusion tested to reset CPU*/
    case 11 : return "TGWDT_CPU_RESET"; break;         /**<11, Time Group reset CPU*/
    case 12 : return "SW_CPU_RESET"; break;            /**<12, Software reset CPU*/
    case 13 : return "RTCWDT_CPU_RESET"; break;        /**<13, RTC Watch dog Reset CPU*/
    case 14 : return "EXT_CPU_RESET"; break;           /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : return "RTCWDT_BROWN_OUT_RESET"; break;  /**<15, Reset when the vdd voltage is not stable*/
    case 16 : return "RTCWDT_RTC_RESET"; break;        /**<16, RTC Watch dog reset digital core and rtc module*/
  }
  return "UNK";
}
byte GetResetReason() {
  byte ResetReason = rtc_get_reset_reason(0);
  if (ResetReason == 0)
    ResetReason = rtc_get_reset_reason(1);
  return ResetReason;
}
#endif //SerialEnabled
