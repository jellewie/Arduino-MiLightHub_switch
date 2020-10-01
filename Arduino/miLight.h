//API used: https://sidoh.github.io/esp8266_milight_hub/branches/latest/openapi.yaml 2020-02
struct MiLight {
  String device_id;
  String remote_type;
  byte group_id;
};
struct State_rgb_cct {
  bool state;
  byte brightness;
  int color_temp;
  byte R, G, B;
};
char MiLight_IP[16] = "192.168.255.255";    //http://milight-hub.local/

enum {mi_UNDEF, mi_DONE, mi_HUB_OFFLINE, mi_EXECUTION_ERROR, mi_UNK, mi_TIMEOUT};
byte SetLight(MiLight Light, String Content) {
  /*Return Value:
    1= Done (responce code 200)
    2= #ERROR Can not connect to HUB
    3= responce code 400 (Execution error)
    4= unknown error
    5= Timeout recieving responce code
  */
  WiFiManager_CheckAndReconnectIfNeeded();
  String path = "/gateways/" + Light.device_id + "/" + Light.remote_type + "/" + String(Light.group_id);
  WiFiClient client;
  client.setTimeout(1000);
  if (!client.connect(MiLight_IP, 80)) {            //(Try) connect to hub
#ifdef milight_SerialEnabled
    Serial.println("ML: #ERROR Cant connect to HUB '" + String(MiLight_IP) + "" + "'");
#endif //milight_SerialEnabled
    Blink_Amount(LED_BUILTIN, 200, 10);             //Can't connect to hub: Just blink a bit to show this error
    return mi_HUB_OFFLINE;                          //Stop here, no reason to move on
  }
#ifdef milight_SerialEnabled
  Serial.println("ML: PUT " + path + " HTTP/1.1" + "_Host: " + String(MiLight_IP) + "_Data=" + Content);
#endif //milight_SerialEnabled
  client.println("PUT " + path + " HTTP/1.1");
  client.println("Content-Length: " + String(Content.length()));
  client.println("Content-Type: application/json");
  client.println();                                         //Terminate headers with a blank line
  client.print(Content);
  //Try to look for a responce code 'HTTP/1.1 200 OK' = 200
  int connectLoop = 2500;                                   //550ms was the average responce time at tests
  int Responcecode  = 0;
  while (client.connected()) {
    while (client.available()) {
      byte recieved = client.read();
      if (recieved == 0x20) {                               //If "HTTP/1.1" is paste and we now have a SPACE
        recieved = client.read();                           //Purge space
        while (recieved != 0x20) {                          //While we read numbers and not a SPACE
          Responcecode = Responcecode * 10 + (recieved - 0x30); //Convert byte to number and put it in
          recieved = client.read();                         //Read new byte
        }
        client.stop();                                      //Stop, we already have the Responce code
      }
    }
    delay(1);               //Loop = 1+MS
    connectLoop--;          //Remove one from loop counter
    if (connectLoop <= 0)   //If loop counter = 0
      return mi_TIMEOUT;             //Stop, we had a timeout
  }
#ifdef milight_SerialEnabled
  Serial.println("ML: Responcecode=" + String(Responcecode));
#endif //milight_SerialEnabled
  if (Responcecode == 200)
    return mi_DONE;
  if (Responcecode == 400)
    return mi_EXECUTION_ERROR;
  return mi_UNK;
}
//===========================================================================
//State_rgb_cct GetLight(MiLight Light) {
//  State_rgb_cct return_Value = {};
//  String path = "/gateways/" + Light.device_id + "/" + Light.remote_type + "/" + String(Light.group_id);
//
//  WiFiClient client;
//  client.setTimeout(1000);
//  Serial.println("boot");    //wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
//  if (!client.connect(MiLight_IP, 80)) {            //(Try) connect to hub
//#ifdef milight_SerialEnabled
//    Serial.println("ML: #ERROR Cant connect to HUB '" + String(MiLight_IP) + "" + "'");
//#endif //milight_SerialEnabled
//    Blink_Amount(LED_BUILTIN, 200, 10);                            //Can't connect to hub: Just blink a bit to show this error
//    return return_Value;
//  } else {
//    Serial.println("GET data");    //wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
//    client.println("GET " + path + " HTTP/1.1");
//    client.println("Connection: close");
//    client.println();
//    Serial.println("find");    //wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
//    client.find("\r\n\r\n");
//    Serial.println("Allocate the JSON document");    //wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
//    // Allocate the JSON document
//    // Use arduinojson.org/v6/assistant to compute the capacity.
//    // Example Json {"state": "OFF","brightness": 255,"color_temp": 350,"bulb_mode": "white","color":{"r": 255,"g": 255,"b": 255}}
//#define JsonCapacityRead JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + 70
//    DynamicJsonDocument json(JsonCapacityRead);
//    Serial.println("Parse JSON object");    //wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
//    // Parse JSON object
//    DeserializationError error = deserializeJson(json, client);
//    if (error) {
//      Serial.print("deserializeJson() failed: ");
//      Serial.println(error.c_str());
//      return return_Value;
//    }
//    Serial.println("Extract values");    //wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
//    // Extract values
//    if (String(json["state"].as<char*>()) == "ON")
//      return_Value.state = true;
//    return_Value.brightness = json["brightness"].as<byte>();
//    return_Value.color_temp = json["color_temp"].as<int>();
//
//    return_Value.R = json["color"]["r"].as<byte>();
//    return_Value.G = json["color"]["g"].as<byte>();
//    return_Value.B = json["color"]["b"].as<byte>();
//    Serial.println("Done");    //wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
//    return return_Value;
//  }
//}
