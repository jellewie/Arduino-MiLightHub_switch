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

byte SetLight(MiLight Light, String JsonContent) {
  String path = "/gateways/" + Light.device_id + "/" + Light.remote_type + "/" + String(Light.group_id);
  byte Answer = WiFiManager.DoRequest(MiLight_IP, 80, path, JsonContent);

#ifdef milight_Log
  Log.Add("ML: PUT " + path + " HTTP/1.1" + "_Host: " + String(MiLight_IP) + ":80 _Data=" + JsonContent);
  switch (Answer) {
    case REQ_UNK:               Log.Add("ML: Unknown error (responce out of range)" + String(Answer));  break;
    case REQ_HUB_CONNECT_ERROR: Log.Add("ML: Cant connect to HUB '" + String(MiLight_IP) + "" + "'");   break;
    case REQ_TIMEOUT:           Log.Add("ML: Timeout recieving responce code");                         break;
    case REQ_PAGE_NOT_FOUND:    Log.Add("ML: Page not found (responce code 404)");                      break;
    case REQ_SETUP_REQUIRED:    Log.Add("ML: Manual WiFi settup required");                             break;
    default:                    Log.Add("ML: UNK Responcecode=" + String(Answer));                      break;
  }
#endif //milight_Log
  return Answer;
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
//#ifdef milight_Log
//    Log.Add("ML: #ERROR Cant connect to HUB '" + String(MiLight_IP) + "" + "'");
//#endif //milight_Log
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
