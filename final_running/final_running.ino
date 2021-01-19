
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include<SPI.h>
#include<MFRC522.h>
#define SS_PIN 53
#define RST_PIN 49


LiquidCrystal lcd(4, 5, 6, 7, 8, 9);
MFRC522 mfrc522(SS_PIN, RST_PIN);

// 모바일 HostSpot의 ID와 패스워드 (변경할 부분)

const String SSID = "java";
const String PASSWORD = "12345678";
// 서버주소와 포트 (변경할 부분)
const String SERVER_IP = "192.168.137.1";
const String SERVER_PORT = "8081";
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {31, 36, 35, 33}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {32, 30, 34}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


// AT 명령 저장
String cmd = "";
// 전송 데이터 저장
String sendData = "";
// WiFi 접속 실패 여부
boolean FAIL_8266 = false;

// TX, RX

void setup() {
  Serial3.flush();
  lcd.begin(16, 1);
  lcd.begin(16, 2);

  
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID reading UID");
  Serial.println("Start module connection");
  do {
    Serial3.begin(9600);
    // Serial38266 모듈 재시작
    Serial3.println("AT+RST");
    delay(1000);
    // 만약 재시작되었다면
    if (Serial3.find("ready")) {
      Serial.println("Module ready");
      // Serial38266 모듈의 모드를 듀얼모드로 설정 (클라이언트)
      Serial3.println("AT+CWMODE=1");
      delay(2000);
      // AP에 접속되면
      if (cwJoinAP()) {
        Serial.println("AP successful");
        FAIL_8266 = false;
        delay(5000);
        Serial.println("Start buffer initialization");
        while (Serial3.available() > 0) {
          char rid = Serial3.read();
          Serial.write(rid);
          char speed = Serial3.read();
          Serial.write(speed);
        }
        Serial.println();
        Serial.println("Buffer initialization terminated");
      } else {
        Serial.println("AP connection failure");
        delay(500);
        FAIL_8266 = true;
      }
    } else {
      Serial.println("Module connection failure");
      delay(500);
      FAIL_8266 = true;
    }
  } while (FAIL_8266);
  Serial.println("Module connection complete");
}

boolean a = true;
String sum = "";
String rfid_id = "";

void loop() {
  lcd.setCursor(0, 0);
  lcd.print("ID CARD TAG");
  if ( mfrc522.PICC_IsNewCardPresent()) {
    lcd.setCursor(0, 1);
    lcd.print("Speed : ");
    if (mfrc522.PICC_ReadCardSerial()) {
      Serial.print("Tag UID:");
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        rfid_id += String(mfrc522.uid.uidByte[i], DEC);
      }
      Serial.println(rfid_id);
      sendDataToServer1(rfid_id);
      rfid_id = "";
      mfrc522.PICC_HaltA();
    }
  }

  char key = keypad.getKey();
  if (key){
    Serial.print(key);
    lcd.setCursor(8, 1);
    
    if (key == '*') {
      sendDataToServer2(sum);
      lcd.clear();
      sum = "";
    } else {
      sum += key;
      lcd.print(sum);
    }
  }
  


}


void sendDataToServer1(String data) {
  Serial.println("Start the data transfer part");
  cmd = "AT+CIPSTART=\"TCP\",\"" + SERVER_IP + "\"," + SERVER_PORT + "\r\n";
  Serial.println("Attempt to connect to server");
  Serial3.println(cmd);
  // 웹 서버에 접속되면
  if (Serial3.find("OK")) {
    Serial.println("Server connection successful");
  } else {
    Serial.println("Server connection failed");
  }

  // 서버로 GET 메시지 전송
  cmd = "GET /Project2_fitness/running2.do?rid=";
  cmd += data;
  cmd += "\r\nConnection: close\r\n\r\n";

  Serial.println(cmd);
  Serial3.print("AT+CIPSEND=");
  Serial3.println(cmd.length());
  if (Serial3.find("OK")) {
    Serial.println("Ready to send to server");
  } else {
    Serial.println("Failed to prepare to send to server");
  }
  Serial3.println(cmd);

  //데이터 전송이 완료되면
  if (Serial3.find("OK")) {
    Serial.println("Data transfer successful");
    Serial.println();
    delay(5000);
    Serial.println("Attempt to receive data");
    while (Serial3.available()) {
      Serial.write(Serial3.read());
    }
    Serial.println();
    Serial.println("End Receiving Data");
  } else {
    Serial.println("Data transfer failed");
    sendDataToServer1(data);
  }
  delay(10000);
}

void sendDataToServer2(String data) {
  Serial.println("Start the data transfer part");
  cmd = "AT+CIPSTART=\"TCP\",\"" + SERVER_IP + "\"," + SERVER_PORT + "\r\n";
  Serial.println("Attempt to connect to server");
  Serial3.println(cmd);
  // 웹 서버에 접속되면
  if (Serial3.find("OK")) {
    Serial.println("Server connection successful");
  } else {
    Serial.println("Server connection failed");
  }

  // 서버로 GET 메시지 전송
  cmd = "GET /Project2_fitness/running2.do?speed=";
  cmd += data;
  cmd += "\r\nConnection: close\r\n\r\n";

  Serial.println(cmd);
  Serial3.print("AT+CIPSEND=");
  Serial3.println(cmd.length());
  if (Serial3.find("OK")) {
    Serial.println("Ready to send to server");
  } else {
    Serial.println("Failed to prepare to send to server");
  }
  Serial3.println(cmd);

  //데이터 전송이 완료되면
  if (Serial3.find("OK")) {
    Serial.println("Data transfer successful");
    Serial.println();
    delay(5000);
    Serial.println("Attempt to receive data");
    while (Serial3.available()) {
      Serial.write(Serial3.read());
    }
    Serial.println();
    Serial.println("End Receiving Data");
  } else {
    Serial.println("Data transfer failed");
    sendDataToServer2(data);
  }
  delay(10000);
}

boolean cwJoinAP() {
  String cmd = "AT+CWJAP=\"" + SSID + "\",\"" + PASSWORD + "\"";
  Serial3.println(cmd);
  if (Serial3.find("OK")) {
    return true;
  } else {
    return false;
  }
}
