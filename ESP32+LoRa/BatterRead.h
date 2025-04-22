#define BatRead   36 // Read the batrery voltage in esp32
#define Booster   32 
uint16_t BatRed = 0;

uint16_t BatteryRead(void);

uint16_t BatteryRead(void) {
  float BatValue = 0;
  BatValue = analogRead(BatRead);
  Serial.print("Battery Raw Data = ");
  Serial.println(BatValue);
  BatRed = BatValue;
}
