/**
   藍燈 長亮表示無法連接至 Wi-Fi 訊號
   正常情況下 應只有 紅燈(電源燈) 長亮
   建議使用前綴為 H 之測站 效果較佳
   測站列表 => https://github.com/ExpTechTW/API/blob/master/Json/earthquake/station.json
   Discord 群組 => https://exptech.com.tw/f?v=discord
 **/

// >>> version 2.0.0 <<<

#include <ESP32Servo.h>   //伺服馬達 庫
#include <ArduinoJson.h>  // Json 庫
#include <HTTPClient.h>   // HTTP 庫

StaticJsonDocument<4096> JSON;  // 初始化 Json
Servo myservo;                  // 初始化 伺服馬達

const char* ssid = "ASUS_A8";                     // Wi-Fi SSID
const char* password = "koala_3249";              // Wi-Fi Password
String station = "H-979-11336952-11";             // 欲使用之測站 UUID
String key = "45c3807ddfb55e774cb7699e78fc9ac";  // 金鑰
int enlarge = 1000;                               // H 版取 1000 L 取 1

float angle(float a) {
  if (a > 115) return 115;
  if (a < 65) return 65;
  return a;
}

void setup() {
  myservo.setPeriodHertz(50);
  myservo.attach(13, 500, 2400);                         // 定義 伺服馬達 腳位
  myservo.write(90);                                     // 初始化(置中) 伺服馬達 角度
  pinMode(LED_BUILTIN, OUTPUT);                          // 設置 LED 腳位為 OUTPUT
  digitalWrite(LED_BUILTIN, HIGH);                       // 開啟 LED
  WiFi.begin(ssid, password);                            // 連接至 Wi-Fi
  for (int i = 0; WiFi.status() != WL_CONNECTED; i++) {  // Wi-Fi 連接
    delay(200);
    if (i > 30) {
      ESP.restart();  // 連接失敗重啟
    }
  }
  digitalWrite(LED_BUILTIN, LOW);  // 關閉 LED
}

void loop() {
  HTTPClient http;                                                                                                // HTTP 對象
  http.begin("http://exptech.com.tw/api/v1/trem/original?uuid=" + station + "&type=original-vector&key=" + key);  // 初始化 HTTP GET 請求
  int httpResponseCode = http.GET();                                                                              // 發起 HTTP GET 請求
  if (httpResponseCode == 200) {
    JSON.clear();
    deserializeJson(JSON, http.getString());                            // Json 反序列化
    for (int i = 0; i < JSON["length"].as<float>(); i++) {              // 控制 伺服馬達
      myservo.write(angle(90 + (JSON["Z"][i].as<float>() * enlarge)));  // >>> 取的軸向 ( X、Y、Z、PGA ) <<<
      delay(45);  // 原始數據為 20Hz 故此處 delay 50ms
    }
  }
  if (httpResponseCode == -1) ESP.restart();  // 例外情況
  http.end();                                 // 結束 HTTP
}