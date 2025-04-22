#include <SoftwareSerial.h>
#define RX        16
#define TX        17

/* Decimal to Hex conversion Values */
#define HEXDS     0x30//HexDecimalValue start in decimal
#define HEXDE     0x39//HexDecimalValue End in decimal 
#define HEXCS     0x41//HexCharacter Start in Character 
#define HEXCE     0x46//HexCharacter End in Character 
#define HEXDR     48//Serial port decimal data will be scaled down from its decimal value in the ASCII table.
#define HEXCR     55//Serial port character data will be scaled down from its decimal value in the ASCII table.

SoftwareSerial HardSerial(RX, TX);
bool AT_Flag = 0;//AT TX Flag...
unsigned char AT_TXBuff[255] = {0};//Transmit Buffer
unsigned char AT_RXBuff1[40] = {0};//Serial Data Receive Buffer
unsigned char ATBuffer[40] = {0};//Serial Data Add Buffer
bool TDConv = 0;
bool Class_A = 0;
int cmdLen  = 0;//AT Command Byte length variable
bool ATFlag = 0;// AT_Command Flag
int availableBytes = 0;// ESP32-Serial Port
bool usDwFlag = 0; //given the downlink based set Flag
unsigned char usDwData[200];//user Downlink value stored in the array...
bool usDwStFlag = 0; //set the flag after get the downlink value...
bool joinFlag = 0; // if stm8 joined to the server after set this flag
void AT_TXSend(void);//UART Transmit Function from ESP32 to STM8
void decToHex(void);//Dec to Hex Conversion Function
void SetATChar(void);//Add the Special characters in ATCommands first bytes...
void UARTsetup(void);//ESP32 to STM8 UART setup function
void UserTXFun( unsigned char *tx, int Size);//User  data send Function from ESP32 to STM8
void recRXFun(void);//User get the data from the STM8 to ESP32
/* RTOS */
void TaskAT(void *pvParameters);

/*ESP32 to STM8 UART setup function*/
void UARTsetup(void) {
  HardSerial.begin(9600, SWSERIAL_8N1, 16, 17, false);
  xTaskCreatePinnedToCore(TaskAT,  "TaskAT",  10240,  NULL,  2,  NULL,  ARDUINO_RUNNING_CORE);
}

/*User  data send Function from ESP32 to STM8 */
void UserTXFun( unsigned char *tx, int Size) {
  unsigned char temTXBuff[255] = {0};
  temTXBuff[0] = '{';
  temTXBuff[(Size + 1)] = (Size + 3);
  temTXBuff[(Size + 2)] = '}';

  for (int f = 1; f <= Size; f++) {
    temTXBuff[f] = tx[f - 1];
  }
  for (int trans = 0; trans <= (Size + 2); trans++) {
    
    HardSerial.write(temTXBuff[trans]);
  }
  Serial.println("Data Sending to STM8 ");
  Serial.println(" ");
}

/*RTOS */
void TaskAT(void *pvParameters) {
  (void) pvParameters;
  for (;;)
  {
    if (ATFlag == 0) {
      if (Serial.available() > 0) {
        availableBytes = Serial.available();
        for (cmdLen = 0; cmdLen < availableBytes; cmdLen++) {
          AT_RXBuff1[cmdLen] = Serial.read(); // read the incoming byte:
          if (AT_RXBuff1[cmdLen] == '\0') {
            break;
          }
          if (((AT_RXBuff1[cmdLen] == 'F'  || AT_RXBuff1[cmdLen] == 'N' || AT_RXBuff1[cmdLen] == 'Z' || AT_RXBuff1[cmdLen] == 'R' || AT_RXBuff1[cmdLen] == 'C') &&  availableBytes == 4) || ((AT_RXBuff1[cmdLen] == 'D' || AT_RXBuff1[cmdLen] == 'J') &&  availableBytes == 18) || ((AT_RXBuff1[cmdLen] == 'A' || AT_RXBuff1[cmdLen] == 'S' || AT_RXBuff1[cmdLen] == 'W' ) &&  availableBytes == 34) || (AT_RXBuff1[cmdLen] == 'T' &&  availableBytes == 6 ) || (AT_RXBuff1[cmdLen] == 'V' &&  availableBytes == 10 ) || ATFlag == 1 ) {
            if (ATFlag == 1) {
              /* Decimal to Hex Convertion */
              decToHex();
            } else {
              SetATChar();
              ATFlag = 1;
            }
          }
        }
        if (((AT_RXBuff1[0] == 'F'  || AT_RXBuff1[0] == 'N' || AT_RXBuff1[0] == 'Z' || AT_RXBuff1[0] == 'R' || AT_RXBuff1[0] == 'C') &&  availableBytes == 4) || ((AT_RXBuff1[0] == 'D' || AT_RXBuff1[0] == 'J') &&  availableBytes == 18) || ((AT_RXBuff1[0] == 'A' || AT_RXBuff1[0] == 'S' || AT_RXBuff1[0] == 'W' ) &&  availableBytes == 34) || (AT_RXBuff1[0] == 'V' &&  availableBytes == 10 )) {
          for (int j = 1; j <= ((availableBytes - 2) / 2); j++) {
            AT_TXBuff[j] = ATBuffer[(j * 2) - 1] << 4 | ATBuffer[j * 2];
          }
          TDConv = 0;
          AT_Flag = 1;

          /*AT Command send function from esp32 to STM8 */
          AT_TXSend();

          memset(AT_RXBuff1, 0, sizeof (AT_RXBuff1));
          memset(ATBuffer, 0, sizeof (ATBuffer));

        } else if (AT_RXBuff1[0] == 'T' &&  availableBytes == 6 ) {
          for (int j = 1; j <= (availableBytes - 2); j++) {
            AT_TXBuff[j] = ATBuffer[j];
          }
          TDConv = 1;
          AT_Flag = 1;
          /*AT Command send function from esp32 to STM8 */
          AT_TXSend();

          memset(AT_RXBuff1, 0, sizeof (AT_RXBuff1));
          memset(ATBuffer, 0, sizeof (ATBuffer));
        }

        else Serial.println("Invaild AT Command : ");
      }
    }
    /*Received value from STM8 to ESP32 */
    recRXFun();
    vTaskDelay(200);
  }
}

/* Decimal to Hex Convertion */
void decToHex(void) {
  if (AT_RXBuff1[cmdLen] >= HEXDS && AT_RXBuff1[cmdLen] <= HEXDE) {
    ATBuffer[cmdLen] = AT_RXBuff1[cmdLen] - HEXDR;
  } else if (AT_RXBuff1[cmdLen] >= HEXCS && AT_RXBuff1[cmdLen] <= HEXCE) {
    ATBuffer[cmdLen] = AT_RXBuff1[cmdLen] - HEXCR;
  } else {}
}

/* UART Transmit Function from ESP32 to STM8 */
void AT_TXSend(void) {
  if (AT_Flag == 1) {
    if (TDConv == 0) {
      for (int ia = 0; ia <= ((availableBytes + 1) / 2); ia++)
      {
        HardSerial.write(AT_TXBuff[ia]);
      }
    } else {
      for (int ib = 0; ib < availableBytes; ib++)
      {
        HardSerial.write(AT_TXBuff[ib]);
      }
    }
    for (int i = 0; i < 1000; i++)
    {
      ;
    }
    AT_Flag = 0;
    ATFlag = 0;
    TDConv = 0;
    availableBytes = 0;
    Serial.println(" ");
  } else {
    memset(AT_TXBuff, 0, sizeof AT_TXBuff);
  }
}

/* Start & Stop Characters add in incomming AT_Commands */
void SetATChar(void) {
  switch (AT_RXBuff1[cmdLen])
  {
    case 'F'://FDR
      joinFlag = 0;
      AT_TXBuff[0] = '%';
      AT_TXBuff[((availableBytes) / 2)] = '$';
      break;

    case 'N'://NJM
      AT_TXBuff[0] = 'm';
      AT_TXBuff[((availableBytes) / 2)] = 'j';
      break;

    case 'C'://CFM
      AT_TXBuff[0] = '|';
      AT_TXBuff[((availableBytes) / 2)] = '/';
      break;

    case 'R'://ADR
      AT_TXBuff[0] = '<';
      AT_TXBuff[((availableBytes) / 2)] = '>';
      break;

    case 'T'://TDC
      AT_TXBuff[0] = '~';
      AT_TXBuff[availableBytes - 1] = '=';
      break;

    case 'D'://DEVEUI
      AT_TXBuff[0] = ':';
      AT_TXBuff[((availableBytes) / 2)] = ';';
      break;

    case 'J'://APPEUI
      AT_TXBuff[0] = '*';
      AT_TXBuff[((availableBytes) / 2)] = '#';
      break;

    case 'A'://APPKEY
      AT_TXBuff[0] = '+';
      AT_TXBuff[((availableBytes) / 2)] = '-';
      break;

    case 'V'://DEVADDRESS
      AT_TXBuff[0] = 'd';
      AT_TXBuff[((availableBytes) / 2)] = 'a';
      break;

    case 'W'://NWSKEY
      AT_TXBuff[0] = 'n';
      AT_TXBuff[((availableBytes) / 2)] = 'k';
      break;

    case 'S'://APPSKEY
      AT_TXBuff[0] = 's';
      AT_TXBuff[((availableBytes) / 2)] = 'y';
      break;

    case 'Z'://ATZ
      joinFlag = 0;
      AT_TXBuff[0] = '@';
      AT_TXBuff[((availableBytes) / 2)] = '^';
      break;
    default:
      break;
  }
}

/*Received value from STM8 to ESP32 */
void recRXFun(void) {
  unsigned char DwArry[255];
  bool DwStrt = 0;
  if (HardSerial.available() > 0) {
    for (int k = 0; k < 255; k++)
    {
      DwArry[k] = HardSerial.read();
      unsigned char RecData = DwArry[0];
      /*Checking for start bit*/
      if (RecData == '{') {
        if (DwArry[k] == '{')
        {
          DwStrt = 1;
          Serial.println("Data Recived From STM8");
        }
        /*Extracting data b/w start&stop bit*/
        if (DwStrt == 1 && DwArry[k] != '}' && DwArry[k] != '{')
        {
          for (int m = 0; m < 1000; m++)
          {
            ;
          }
          usDwData[k - 1] = DwArry[k];
          Serial.println(DwArry[k]);
        }
        if (DwStrt == 1 && DwArry[k] == '}')
        {
          usDwStFlag = 1;
          DwStrt = 0;
          RecData = 0;
          Serial.println(" ");
        }
      } else if (RecData == '(') {
        if (DwArry[k] == 1) {
          Serial.println("AT Command ACK - Ok : ");
        } else if (DwArry[k] == ')') {
          break;
        }
      } else if (RecData == 'x' && DwArry[k] == 'y') {
        joinFlag = 1;
         if(DwArry[1] == 'A')Class_A = 1;
         else Class_A=0;
      
        Serial.println(" ");
        Serial.println(" Device Joined : ");
        Serial.println(" ");
        break;
      }else {};
    }
  }
}
