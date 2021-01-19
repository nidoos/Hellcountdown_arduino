#include <IRremote.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include<SPI.h>
#include<MFRC522.h>
#define SS_PIN 53
#define RST_PIN 49
int input_pin = 3; // 적외선의 설정
LiquidCrystal lcd(4, 5, 6, 7, 8, 9);
MFRC522 mfrc522(SS_PIN, RST_PIN);
IRrecv irrecv(input_pin); //IRrecv 객체생성
decode_results results;  //수신 데이터 저장 구조체


// 모바일 HostSpot의 ID와 패스워드 (변경할 부분)
const String SSID = "java";
const String PASSWORD = "12345678";
// 서버주소와 포트 (변경할 부분)
const String SERVER_IP = "192.168.137.1";
const String SERVER_PORT = "8081";

// AT 명령 저장
String cmd = "";
// 전송 데이터 저장
String sendData = "";
// WiFi 접속 실패 여부
boolean FAIL_8266 = false;


// TX, RX
//SoftwareSerial esp(3, 2);

void Print();
float Distance();
void PushUp();

const int buttonPin = 10;
int trigPin = 12;
int echoPin = 13;

//운동 상태
boolean buttonStatus; //버튼상태
int Flag;                       //운동
int finish;                    //종료

boolean up_flag = false;
boolean down_flag = false;

int count = 0;              // 운동 갯수
float kcal = 0;              //칼로리

boolean a = true;
//String message = "";
String rfid_id = "";
String sum = "";
String result_count = "";
boolean text = true;
boolean text2 = true;
boolean text3 = true;




// LCD 화면에 출력
void Print() {
  lcd.setCursor(0, 0);
  lcd.print("COUNT:");
  lcd.print(count);

  lcd.setCursor(0, 1);
  lcd.print("Kcal:");
  lcd.print(kcal);
  delay(400);
}

//거리를 측정
float Distance() {

  float duration, distance;
  digitalWrite(trigPin, HIGH);
  delay(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = ((float)(340 * duration) / 10000) / 2;

  return distance;
}

//푸쉬업 함수
void PushUp() {

  float distance = Distance();

  Serial.print(distance);
  Serial.print("cm ");

  //거리가 0<distance<15이면 down_flag를 true로 설정
  if ((0 < distance) && (distance < 15) ) {
    down_flag = true;
    up_flag = false;
  }

  //거리가 20<distance<50이면 up_flag를 true로 설정
  else if ( (20 < distance) && (distance < 50) ) {
    up_flag = true;
  }
  else {
  }
  if ( (down_flag == true) && (up_flag == true) ) {
    count++;
    kcal = kcal + 0.825;
    down_flag = false;
    up_flag = false;

    lcd.clear();
  }
  Serial.print("Pushup_count  ");

  Serial.println(count);
  Print();
}


//리모콘(무게)
void remote() {
  if (irrecv.decode(&results)) {
    if (results.value == 0xFF6897) {
      sum += '0';
    } else if (results.value == 0xFF30CF) {
      sum += '1';
    } else if (results.value == 0xFF18E7) {
      sum += '2';
    } else if (results.value == 0xFF7A85) {
      sum += '3';
    } else if (results.value == 0xFF10EF) {
      sum += '4';
    } else if (results.value == 0xFF38C7) {
      sum += '5';
    } else if (results.value == 0xFF5AA5) {
      sum += '6';
    } else if (results.value == 0xFF42BD) {
      sum += '7';
    } else if (results.value == 0xFF4AB5) {
      sum += '8';
    } else if (results.value == 0xFF52AD) {
      sum += '9';
    } else if (results.value == 0xFF906F) {
      text3 = false;

    }
    Serial.println(sum);
    lcd.setCursor(9, 1);
    lcd.print(sum);
    irrecv.resume();
  }
}

void setup() {
  //pinMode(7, INPUT);
  //시리얼 모니터 시작
  Serial3.flush();
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID reading UID");

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  lcd.begin(16, 1);
  lcd.begin(16, 2);

  irrecv.enableIRIn();


  Serial.println("Start module connection");
  do {
    Serial.begin(9600);
    Serial3.begin(9600);
    // ESP8266 모듈 재시작
    Serial3.println("AT+RST");
    delay(1000);
    // 만약 재시작되었다면
    if (Serial3.find("ready")) {
      Serial.println("Module ready");
      // ESP8266 모듈의 모드를 듀얼모드로 설정 (클라이언트)
      Serial3.println("AT+CWMODE=1");
      delay(2000);
      // AP에 접속되면
      if (cwJoinAP()) {
        Serial.println("AP successful");
        FAIL_8266 = false;
        delay(5000);
        Serial.println("Start buffer initialization");
        while (Serial3.available() > 0) {
          char count = Serial3.read();  //count
          char weight = Serial3.read();  //weight
          char wid = Serial3.read();
          Serial.write(count);  //count
          Serial.write(weight);  //weight
          Serial.write(wid);

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


void loop() {

  if (text == true) {
    lcd.setCursor(0, 0);
    lcd.print("IDCARD TAG");
  }
  text = false;

  if (mfrc522.PICC_IsNewCardPresent()) {
    text2 = false;
    if (text2 == false) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("---Mode");
      lcd.setCursor(0, 1);
      lcd.print("weight : ");
    }
    if (mfrc522.PICC_ReadCardSerial()) {
      Serial.print("Tag UID:");
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        rfid_id += String(mfrc522.uid.uidByte[i], DEC);
      }
      Serial.println(rfid_id);
      sendDataToServer1(rfid_id);
    }
    rfid_id = "";
    mfrc522.PICC_HaltA();
  }



  if (digitalRead(buttonPin) == HIGH) {
    if (Flag == 0)  {
      Flag = 1;
      buttonStatus = !buttonStatus;
      lcd.clear();
    }
  }
  //초기화면
  else if (digitalRead(buttonPin) == LOW && Flag == 0 && buttonStatus == 0) {

    remote();

    Flag = 0;
    //lcd에 안내출력
    if (text3 == false) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("---Mode");
      lcd.setCursor(0, 1);
      lcd.print("Start-Button");
      delay(2000);
    }

  }
  else if (digitalRead(buttonPin) == LOW && Flag == 0 && buttonStatus == 0) {
    Flag = 0;
  }
  else {
    Flag = 0;
  }

  //버튼이 눌렸으면 운동 시작!
  if (buttonStatus) {

    lcd.clear();
    PushUp();
    delay(300);

    //버튼을 다시 누르면, 운동 종료
    if (digitalRead(buttonPin) == HIGH) {

      finish = true;
      buttonStatus = 0;

      if (finish == 1 && buttonStatus == 0) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Final Score");
        delay(1000);
        lcd.clear();
        Print();
        delay(3000);
        finish = false;
        lcd.clear();
        text = true;
        text3 = true;
        result_count = String(count);
        sendDataToServer2();
        count = 0;
        kcal = 0;
        sum = "";

      }//if 의 끝
    } //버튼의 눌림여부의 끝

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
  cmd = "GET /Project2_fitness/weight.do?equip=legpress&wid=";
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

void sendDataToServer2() {
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
  //cmd = "GET /Wifi/wifi.jsp?a=";
  cmd = "GET /Project2_fitness/weight.do?count=" + result_count + "&weight=" + sum;
  // cmd += data;
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
    sendDataToServer2();
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
