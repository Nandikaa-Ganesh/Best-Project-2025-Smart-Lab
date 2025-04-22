#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#include <WiFi.h>  // ✅ Needed for WiFiServer, WiFiClient, WiFi functions
#include "AT_Command.h"
#include "BatterRead.h"

#define fun_Code 2

/* WiFi Client Config (ESP32 connects to external AP) */
const char* ssid = "ReverGenie";
const char* password = "Passpass";

WiFiServer server(1234);  // ✅ Listen on port 1234
WiFiClient client;

unsigned char TXBuffer[] = {0, 0, fun_Code, 0}; // Max 25 bytes

extern bool AT_Flag;
extern bool joinFlag;
extern bool Class_A;
extern uint16_t BatRed;

bool dataReady = false;
String receivedData = "";

void setup() {
  Serial.begin(115200);
  UARTsetup();

  // ✅ ESP32 connects to external Wi-Fi, not broadcasts its own
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected. ESP32 IP: ");
  Serial.println(WiFi.localIP());

  server.begin();  // Start TCP server

  xTaskCreatePinnedToCore(TaskReceiver, "TaskReceiver", 10240, NULL, 4, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(TaskUplink, "TaskUplink", 10240, NULL, 3, NULL, ARDUINO_RUNNING_CORE);
}

void loop() {
  // Nothing in main loop, handled by tasks
}

void TaskReceiver(void *pvParameters) {
  for (;;) {
    if (!client || !client.connected()) {
      client = server.available();
    } else if (client.available()) {
      receivedData = client.readStringUntil('\n');
      Serial.println("Received from Pi: " + receivedData);
      dataReady = true;
      client.println("ACK"); // Send acknowledgment to Pi
    }
    vTaskDelay(100);
  }
}

void TaskUplink(void *pvParameters) {
  for (;;) {
    if (dataReady && AT_Flag != 1 && joinFlag == 1) {
      if (Class_A == 1) {
        BatteryRead();
        TXBuffer[0] = BatRed >> 8;
        TXBuffer[1] = BatRed;
      }

      // ✅ Fix: cast receivedData.length() to int to avoid min() template conflict
      int len = min((int)receivedData.length(), 21);
      for (int i = 0; i < len; i++) {
        TXBuffer[3 + i] = receivedData[i];
      }

      UserTXFun(TXBuffer, 3 + len);

      Serial.println("Data sent to TTN.");
      memset(TXBuffer, 0, sizeof(TXBuffer));
      TXBuffer[2] = fun_Code;
      dataReady = false;
    }
    vTaskDelay(4000);
  }
}
