#include <String.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h> 

LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(D0, D1); //SS_PIN-D0,RST_PIN-D3
ESP8266WiFiMulti WiFiMulti;
WiFiClient client;
WiFiServer server(80);
#define USE_SERIAL Serial

int state=0;
int j=11;
int t=0,h=1,s=0;
char byteId,byteIn;
int MAX_CMD_LENGTH = 10;
char cmd[20],cmd1[20];
int cmdIndex,cmdIndex1;
char incomingByte;
String x="";

struct info{
  char Name[20];
  int id;
  String content1;
};

info arr[10];

void setup() {
  Serial.begin(115200);
  delay(10);
 
  Wire.begin(D4, D3);
  lcd.begin();
  lcd.home(); 
  lcd.setCursor(0,0);
  lcd.print(" CHECK STUDENTS ");
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  mfrc522.PCD_DumpVersionToSerial();
  pinMode(D2, INPUT);
  cmdIndex = 0;
  cmdIndex1 = 0;
  WiFiMulti.addAP("Le Duc Thanh", "01213601997");
  while(WiFiMulti.run() != WL_CONNECTED){
    Serial.print(".");
    delay(1000);
  }
  Serial.println("CONNECTED");
  server.begin();
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  
//  strcpy(arr[0].Name,"L.D.Trung");
//  arr[0].id = 1613786;
//  arr[0].content1="B6 70 2B 1F";
}

void loop() {
  switch(state){
    case 0:
    {
       WiFiClient client = server.available();
        if (!client) {
        return;
        }
        while(!client.available()){
          delay(1);
        }

        String request = client.readStringUntil('\r');
        client.flush();

        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println(""); //  do not forget this one
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");
        client.println("DANH SACH SINH VIEN");
        for(int i=0;i<=10;i++){
           if(arr[i].id != 0){
               client.println("<br><br>");
               client.print(i+1);
               client.print(". ");
               client.print(arr[i].Name);
               client.print(" - ");
               client.print(arr[i].id);
           }
       }

        client.println("<br><br>");
        client.println("</html>");
        delay(5000);
        Serial.println("Client disonnected");
        state =1;
    }
    case 1:
    { 
      int butStatus = digitalRead(D2);
      if (butStatus == HIGH){
        t=1;
      }
      if(t==0){
      if ( ! mfrc522.PICC_IsNewCardPresent()) {return;}
      if ( ! mfrc522.PICC_ReadCardSerial()) {return;}
      String content= "";
      byte letter;
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      content.toUpperCase();
      Serial.println();
      x = content.substring(1);
      for(int i=0;i<=10;i++){
        if(content.substring(1) == arr[i].content1  ){
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Name: ");
          lcd.setCursor(0,1);
          lcd.print("Id: ");
          lcd.setCursor(6,0);
          lcd.print(arr[i].Name);
          lcd.setCursor(4,1);
          lcd.print(arr[i].id);
          j=i;
          break;
        }
        j=11;
      }
      if(j==11){
        lcd.clear();
        lcd.setCursor(1,0);
        lcd.print("No Information");
      }
      }
      else if(t==1){
        for(int i=0;i<=10;i++){
          if(x==arr[i].content1){
            h=2;
            state  = 2;
          }
          s++;
        }
      //  Serial.println(s);
        if(s==11){
          state = 2;
        }
        
      }
      delay(2000);
      break;}
    case 2:
     { s=0;
      if(h==2){
        h=1;
        t=0;
        Serial.println("Can't create account!!!");
        state = 1;
      }
      else if (h==1){
      Serial.println("Enter name: ");
      state = 3;
      }
      break;
    }
    case 3:
    {
      if (incomingByte=Serial.available()>0) {
          byteIn = Serial.read();
          cmd[cmdIndex] = byteIn;
          if(byteIn=='\n'){
            cmd[cmdIndex] = '\0';
            cmdIndex = 0;
            Serial.println(cmd);
            state = 4;
          }
          else{
            if(cmdIndex++ >= MAX_CMD_LENGTH){
              cmdIndex = 0;
            }
          }
      }
      break;}
    case 4:
      Serial.println("Enter id: ");
      state = 5;
      break;
    case 5:
    {
      if (incomingByte=Serial.available()>0) {
        byteId = Serial.read();
        cmd1[cmdIndex1] = byteId;
        if(byteId=='\n'){
          cmd1[cmdIndex1] = '\0';
          cmdIndex1 = 0;
          Serial.println(cmd1);
          state = 6;
          t=0;
        }
        else{
          if(cmdIndex1++ >= MAX_CMD_LENGTH){
            cmdIndex1 = 0;
          }
        }
      }
      break;}
    case 6:
    {
      for (int i=0;i<=10;i++){
        if(arr[i].id == 0){
          strcpy(arr[i].Name,cmd);
          arr[i].id = atoi(cmd1);
          arr[i].content1 = x;
          break;
        }
      }
      state = 0;
    }
    default:
      break;
  }
  
}



