#include <esp_now.h>
#include <WiFi.h>

uint8_t senderAddress[] = {0x40, 0x91, 0x51, 0xBE, 0xFB, 0x50}; //送信機のMACアドレス

esp_now_peer_info_t sender; // ESP-Nowの送信機の情報

// 受信コールバック
void onReceive(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  // MACアドレスの文字列化
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.printf("Last Packet Recv from: %s\n", macStr); // 最後に受信したパケットのMACアドレス

  // 受信データの解析
  int receivedData;
  memcpy(&brightnessData, data, sizeof(brightnessData)); //data -> brightnessData へコピー

  // 受信したデータの表示
  Serial.printf("Now brightness: %d\n", brightnessData);

  //ここに受信側の処理を記述 / brightnessDataで条件分岐など
}

void setup() {
  Serial.begin(115200); // シリアル通信の開始

  WiFi.mode(WIFI_STA); // WiFiをステーションモードに設定
  WiFi.disconnect(); // 初期化時にWiFiを切断

  // ESP-Nowの初期化
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success"); // 初期化成功
  } else {
    Serial.println("ESPNow Init Failed"); // 初期化失敗
    ESP.restart();
  }

  // 送信機の情報を設定
  memcpy(sender.peer_addr, senderAddress, 6);
  sender.channel = 0;  
  sender.encrypt = false;

  // 送信機をペアリングリストに追加
  if (esp_now_add_peer(&sender) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  // 受信コールバックの登録
  esp_now_register_recv_cb(onReceive);
}

void loop() {
  // とくになし
}
