/* Written by JelleWho https://github.com/jellewie
*/
#ifndef Log_h                               //This prevents including this file more than once
#define Log_h

class CLog {
  public:
    const static byte Amount = 32;          //Amount of erros to store
    String List[Amount];
    byte Counter = 0;
    void Add(String MSG) {
#ifdef SerialEnabled
      Serial.println("LOG: " + MSG);
#endif //SerialEnabled
      List[Counter] = MSG;
      Counter++;
      if (Counter >= Amount)
        Counter = 0;
    }
    String GetList() {
      String ReturnValue = "";
      for (byte i = 0; i < Amount; i++) {
        ReturnValue += List[ItoID(i)] + "<br>";
      } return ReturnValue;
    }
    byte ItoID(byte i) {
      i += Counter;
      while (i >= Amount)
        i = i - Amount;
      return i;
    }
};
CLog Log;

void handle_Log() {
#ifdef SerialEnabled
  Serial.println("handle_Log");
#endif //SerialEnabled
  server.send(200, "text/html", Log.GetList() + "<br>old to new(bottom)");
}
#endif
