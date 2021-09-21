#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(3, 2);
#define SS_PIN 10 //mengatur pin sda(ss)
#define RST_PIN 9 //mengatur pin rst
const int OUTPUT_PIN = 4; //mengatur pin reset program
MFRC522 mfrc522(SS_PIN, RST_PIN); //instansiasi objek dari class MFRC522

String code = ""; //variabel untuk menampung TagID saat ini dalam bentuk string

//data tag
#define jumlah_tag 4

String data[jumlah_tag][4] = {
  //{rfidTag,namaDriver,Gate,namaKendaraan}
  {"18821219756", "AgustinusNugroho", "34", "GrandMaxL300"},
  {"14720912321", "YohanesVito", "34", "SupraX125"},
  {"43", "Mukardi", "3", "GrandMaxL300"},
  {"43", "Mukardi", "3", "GrandMaxL300"}
};

void(* resetFunc) (void) = 0;  //fungsi untuk mereset reader

void setup() {
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  mySerial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println(F("Tempelkan Tag"));
}

void loop() {
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  /*print ID TAG
  for (byte i = 0; i < mfrc522.uid.size; i++) {

    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");

    Serial.print(mfrc522.uid.uidByte[i], DEC);
  }
  Serial.println("");*/

  //readTagId
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    code += (String) mfrc522.uid.uidByte[i];
  }
  if (code != "") {
    String message = String(code);
    Serial.println(message);
    cekKartu(message);
  }
  delay (2000);
}

//fungsi untuk mengecek apakah kartu terdaftar di variabel data
void cekKartu(String message) {
  for (int i = 0; i < jumlah_tag; i++) {
    if (data[i][0] == (String)message) {
      Serial.println("Kartu Anda Terdaftar");
      delay(500);
      uploadData(message);
      //sendSms(message);
      break;
    }
  }
  Serial.println("Kartu anda belum terdaftar");
  delay(200);
  Serial.println("Rfid Tag Anda: " + message);
  delay(1000);
  resetFunc();
}

void uploadData(String message) {
  //inisialisasi URL untuk HTTP REQUEST
  String api_key = "6EJA12MHN14ITVE9";
  String para = "AT+HTTPPARA=\"URL\",api.thingspeak.com/update?api_key=" + api_key;

  for (int i = 0; i < jumlah_tag; i++) {
    if (data[i][0] == (String)message) {
      //String pesan = "&field1="+rfidTag+"&field2="+namaDriver"&field3="+Gate"&field4="+namaKendaraan;
      String pesan = "&field1=" + data[i][0] + "&field2=" + data[i][1] + "&field3=" + data[i][2] + "&field4=" + data[i][3];
      Serial.println("URL berhasil dibuat");
      delay(500);
      para += pesan;
      break;
    }
  }
  sendGET(para);
}

void sendGET(String para) {
  
  mySerial.println("AT+CPIN?");
  delay(1000);
  
  mySerial.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  delay(1000);

  mySerial.println("AT+SAPBR=3,1,\"APN\",\"telkomselgprs\"");
  delay(1000);

  mySerial.println("AT+SAPBR=1,1");
  delay(1000);
  
  mySerial.println("AT+SAPBR=2,1");
  delay(1000);
  
  mySerial.println("AT+HTTPINIT");
  delay(1000);

  mySerial.println("AT+HTTPPARA=\"CID\",1");
  delay(1000);

  //AT+HTTPPARA="URL",URL
  mySerial.println(para);
  delay(1000);
  
  Serial.println(para);
  delay(1000);
  
  Serial.println("Data checkpoint sedang dikirim");
  delay(1000);

  mySerial.println("AT+HTTPACTION=0");
  delay(10000);

  mySerial.println("AT+HTTPTERM");
  delay(1000);

  mySerial.println("AT+SAPBR=0,1");
  delay(1000);

  Serial.println("Operasi selesai");
  delay(1000);

  resetFunc();
}

//mengambil respon
//  String codeRespon = (String) mySerial.read();
//  String respon = "";
//  for(int i =15;i<18;i++){
//  respon+=(String)codeRespon[i];
//  }
//  Serial.println("coderespon:"+codeRespon);
//  Serial.println("respon:"+respon);
//  if(respon.CharAt=="200"){
//  Serial.println("Data checkpoint berhasil dikirim");
//  }
//  else{
//  Serial.println("Data checkpoint gagal dikirim. Error code: "+respon);
//  }
//  delay(5000);


void sendSms(String message) {
  //Begin serial communication with Arduino and SIM800L
  mySerial.begin(9600);
  Serial.println("Sedang menyiapkan");
  delay(1000);
  mySerial.println("AT"); //Once the handshake test is successful, it will back to OK
  delay(500);
  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  delay(500);
  mySerial.println("AT+CMGS=\"+6282241116833\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  delay(500);
  mySerial.println("Truck ID:" + message); //text content
  mySerial.println("Telah melewati Gate-6251");
  delay(500);
  mySerial.write(char(26));
  delay(500);
  Serial.println("Pesan telah dikirim \n");
  delay(15000);
  resetFunc(); //call reset
}

void updateSerial()
{
  delay(500);
  while (Serial.available())
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while (mySerial.available())
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}
