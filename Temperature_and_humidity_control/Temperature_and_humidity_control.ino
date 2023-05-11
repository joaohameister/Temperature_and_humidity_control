//Include required libraries
#include "WiFi.h"
#include <HTTPClient.h>
#include "time.h"

#include <max6675.h>
#include <DHT.h>

int thermoDO = 23;
int thermoCS = 5;
int thermoCLK = 18; 

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
const char* ntpServer = "pool.ntp.org"; 
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

// WiFi credentials
const char* ssid = "Insert WiFi SSID";            // change SSID
const char* password = "Insert WiFi password";  // change password
const String sheet = "Insert Sheet name";
// Google script ID and required credentials
String GOOGLE_SCRIPT_ID = "Insert google script ID here";

//Sensores e variáveis de dados

double umidade1 = 0;
double temperatura1 = 0;

double umidade2 = 0;
double temperatura2 = 0;

double umidade3 = 0;
double temperatura3 = 0;

double umidade4 = 0;
double temperatura4 = 0;

float termopar = 0;

int contagem = 1;
//-----------------------------------------

DHT dht1(25, 22);
DHT dht2(26, 22);
DHT dht3(32, 22);
DHT dht4(33, 22);

void setup() {
  //-------------------------------------------
  dht1.begin();
  dht2.begin();
  dht3.begin();
  dht4.begin();
  //-------------------------------------------
  Serial.begin(115200);
  delay(1000);

  Serial.println("MAX6675 test");
  delay(500);
  // connect to WiFi
  Serial.println();
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  Serial.flush();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}
void loop() {
  temperatura1 = dht1.readTemperature();
  umidade1 = dht1.readHumidity();
  temperatura2 = dht2.readTemperature();
  umidade2 = dht2.readHumidity();
  temperatura3 = dht3.readTemperature();
  umidade3 = dht3.readHumidity();
  temperatura4 = dht4.readTemperature();
  umidade4 = dht4.readHumidity();

  termopar = thermocouple.readCelsius();
  delay(300);

  if (WiFi.status() == WL_CONNECTED) {
    String url = assembleUrl(sheet, temperatura1, umidade1, temperatura2, umidade2, temperatura3, umidade3, temperatura4, umidade4, termopar);
    deployData(url);
  }
  contagem += 1;
  delay(1000);
}

String assembleUrl(String sheet, double temp1, double umi1, double temp2, double umi2, double temp3, double umi3, double temp4, double umi4, double termopar){
   static bool flag = false;
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");     
    }
    char timeStringBuff[50];  //50 chars should be enough
    strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);

    String asString(timeStringBuff);
    asString.replace(" ", "-");
    String urlFinal = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?" + "sheet=" + sheet + "&contagem=" + contagem + "&date=" + asString + "&temp1=" + temp1 + "&umi1=" + umi1 + "&temp2=" + temp2 + "&umi2=" + umi2 + "&temp3=" + temp3 + "&umi3=" + umi3 + "&temp4=" + temp4 + "&umi4=" + umi4 + "&termopar=" + termopar;
    return urlFinal;
}

void deployData(String urlFinal){
    Serial.println(urlFinal);
    HTTPClient http;
    http.begin(urlFinal.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();
    String payload;
    if (httpCode > 0) {
        payload = http.getString();
        Serial.println("Payload: "+payload);    
    }
    http.end();
}