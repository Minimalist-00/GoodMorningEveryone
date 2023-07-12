#include <esp_now.h>
#include <WiFi.h>
#include <time.h>

#define LIGHT_SENSOR A4 // 光センサのピン番号, GPIO32
#define SWITCH_PIN 2
const int interval = 1; // 秒数を指定
int brightnessData = 0; // 光センサからのデータ
uint8_t slaveAddress[] = { 0x40, 0x91, 0x51, 0xBD, 0xDC, 0x8C }; //受信側のmacアドレス
esp_now_peer_info_t slave; // ESP-Nowのスレーブデバイスの情報

int switchState = 0;

/*
#define NTP_SERVER "pool.ntp.org"
#define TIME_ZONE 9 // 日本のタイムゾーン
#define DST 0 // 夏時間 (日本では通常は0)

const char* ssid     = "ここにSSIDを設定";
const char* password = "ここにパスワードを設定";
*/

// 送信コールバック
void onSend(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  // MACアドレスの文字列化
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Sent to: ");
  Serial.println(macStr); // 最後に送信したパケットのMACアドレス
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Failed"); // 送信成功か失敗かを表示
}

void setup() {
  Serial.begin(115200); // シリアル通信の開始
  pinMode(LIGHT_SENSOR, ANALOG); // 光センサのピン設定
  pinMode(SWITCH_PIN, INPUT); //Switchのピン設定

  WiFi.mode(WIFI_STA); // WiFiをステーションモードに設定
  WiFi.disconnect(); // 初期化時にWiFiを切断

  /*
  // WiFi設定
  WiFi.mode(WIFI_STA); // WiFiをステーションモードに設定
  const char* ssid     = "your_SSID"; // ネットワーク名
  const char* password = "your_PASSWORD"; // ネットワークパスワード

  // WiFiに接続
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { // WiFiに接続するまで待つ
    delay(1000);
    Serial.println("Connecting ...");
  }
  Serial.println("Successfully connected to WiFi");
  */

  // ESP-Nowの初期化
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow 初期化成功");
  } else {
    Serial.println("ESPNow 初期化失敗");
    ESP.restart(); //再起動
  }

  // スレーブデバイスの設定
  memcpy(slave.peer_addr, slaveAddress, 6);
  slave.channel = 0;  
  slave.encrypt = false;

  // スレーブの追加
  esp_err_t addStatus = esp_now_add_peer(&slave);
  if (addStatus == ESP_OK) {  // ペアリング成功
    Serial.println("ペアリング成功");
  } else { // ペアリング失敗
    Serial.print("ペアリング失敗, error code: ");
    Serial.println(addStatus);
  }

  // 送信コールバックの登録
  esp_now_register_send_cb(onSend);

  // NTPサーバから時間を取得し、ESP32の時計を設定
  /*
  configTime(TIME_ZONE * 3600, DST * 0, NTP_SERVER);
  delay(1000);
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  Serial.println(&timeinfo, "時間設定: %B %d %Y %H:%M:%S");
  */
}

void EspNowSend() {
  // uint16_tの値を2つのuint8_tに変換
  uint8_t dataToSend[2]; // 送信するデータ配列
  dataToSend[0] = brightnessData >> 8; // 上位8ビット
  dataToSend[1] = brightnessData & 0xFF; // 下位8ビット

  esp_err_t result = esp_now_send(slaveAddress, dataToSend, sizeof(dataToSend)); // データの送信

  Serial.print("Send Status: ");
  if (result == ESP_OK) {
    Serial.println("Success"); // 送信成功
  } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
    Serial.println("ESPNOW not Init."); // ESP-Now未初期化
  } else if (result == ESP_ERR_ESPNOW_ARG) {
    Serial.println("Invalid Argument"); // 不正な引数
  } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
    Serial.println("Internal Error"); // 内部エラー
  } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
    Serial.println("ESP_ERR_ESPNOW_NO_MEM"); // メモリ不足
  } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
    Serial.println("Pair not found."); // ペアが見つからない
  } else {
    Serial.println("Not sure what happened"); // 不明なエラー
  }
  Serial.printf("明るさ: %d\n", brightnessData); // 取得したデータをシリアルモニターに出力
}

void loop() {
  /* 時間の制約
  struct tm timeinfo;
  getLocalTime(&timeinfo); // 時間の指定
  if (timeinfo.tm_hour < 5 || timeinfo.tm_hour >= 8) {
    return;
  }
  */
  brightnessData = analogRead(LIGHT_SENSOR); // 光センサからデータを読み取る
  switchState = digitalRead(SWITCH_PIN); //Switchの状態を読み取る
  
  if(brightnessData > 3900 && switchState == HIGH) { // 光が閾値以上のときのみデータを送信
    EspNowSend(); 
  }
  
  delay(interval * 1000); // 指定したインターバルでデータを送信
}
