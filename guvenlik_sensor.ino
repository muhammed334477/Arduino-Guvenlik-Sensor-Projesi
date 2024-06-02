#include <SoftwareSerial.h>
#include <WiFi.h>

String agAdi = "GM8";                              // Kablosuz Ağ adını buraya yazıyoruz.    
String agSifresi = "123456789";                    // Ağ şifresini buraya yazıyoruz.

int rxPin = 10;                                     // ESP8266 TX pini
int txPin = 11;        

#define echoPin 6
#define trigPin 7
#define buzzerPin 8

int maximumRange = 50;
int minimumRange = 0;       
int olcum = 0;
bool status = false;

String ip = "maker.ifttt.com";
String resource = "/trigger/kapı_sensoru/with/key/bk_o6wwDJkLMplXl86iTe";

#define DEBUG true                                    // çalışmanın düzgün olduğunu takip etmek için kullandım

SoftwareSerial esp(rxPin, txPin);                    // Seri haberleşme için pin ayarlarını yapıyoruz.

void setup() {  
  Serial.begin(9600);                                         
  Serial.println("Başlatılıyor");                                    
  esp.begin(115200);                                       
  esp.println("AT");                                          
  Serial.println("AT Gönderildi");
  while(!esp.find("OK")){                                    
    esp.println("AT");
    Serial.println("ESP8266 ile iletişim kurulamadı.");
  }
  Serial.println("OK Komutu Alındı");
  esp.println("AT+CWMODE=1");                                 
  while(!esp.find("OK")){                                     
    esp.println("AT+CWMODE=1");
    Serial.println("Modül Ayarı Yapılıyor....");
  }
  Serial.println("Modül Client olarak ayarlandı");
  Serial.println("Ağa Baglaniliyor...");
  esp.println("AT+CWJAP=\""+agAdi+"\",\""+agSifresi+"\"");    
  while(!esp.find("OK"));                                
  Serial.println("Ağa Bağlanıldı.");
  delay(1000);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  olcum = mesafe(maximumRange, minimumRange);
  Serial.println(olcum);
  delay(200);
 
  if (olcum > 5){
    status = true;
  }

  if(status){
    tone(buzzerPin, 1000);  // Set frequency to 1000 Hz
    delay(1000);            // Delay for 1 second
    noTone(buzzerPin);
    IFTTTistekYap();
    status = false;
    olcum = 0;
    delay(30000); // 30 seconds delay
  }
}

int mesafe(int maxrange, int minrange) {
  long duration, distance;

  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration / 58.2;
  delay(50);

  if(distance >= maxrange || distance <= minrange)
    return 0;
  return distance;
}

void IFTTTistekYap() {
  esp.println("AT+CIPSTART=\"TCP\",\""+ip+"\",80"); // Connect to the server
  
  if(esp.find("Error")){ // Check for connection error
    Serial.println("AT+CIPSTART Error");
    return;
  }
  
  String veri = "GET " + resource + " HTTP/1.1\r\n"; // HTTP GET request
  veri += "Host: " + ip + "\r\n"; // Host header
  veri += "Connection: close\r\n\r\n"; // Connection header
  
  esp.print("AT+CIPSEND="); // Send data length
  esp.println(veri.length());
  delay(2000);
  
  if(esp.find(">")){ // Wait for prompt
    esp.print(veri); // Send HTTP request
    Serial.println(veri);
    Serial.println("Veri gonderildi.");
    delay(1000);
    
    // Read the response
    while (esp.available()) {
      String response = esp.readString();
      Serial.println("Gelen Yanıt: ");
      Serial.println(response);
    }
  }
  
  Serial.println("Baglantı Kapatildi.");
  esp.println("AT+CIPCLOSE"); // Close connection
  delay(1000);   
}
