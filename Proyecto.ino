// DHT11
#include "DHT.h"
#define DHTPIN 33   //Pin DHT11
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); //

// ULTRASONICO
#define echoPin 12
#define trigPin 14

// HUMEDAD
#define humPin 15

// BOMBA
#define rele 4

#include <WiFi.h>
#include <HTTPClient.h>
const char* ssid = "CGA2121_24";
const char* password = "MGJ49D8Wz8utSTnFFn";

void setup() {
  Serial.begin(115200);

  lcd.begin(21, 22);
  lcd.clear();
  lcd.backlight();

  dht.begin();

  pinMode(humPin, INPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(rele, OUTPUT);
  digitalWrite(rele, LOW);

  delay(1000);
}

void loop() {
  int riego = 0;
  digitalWrite(rele, LOW);
  lcd.clear();
  float humedad = getHumedad();
  float temperatura = getTemp();
  float humedadSuelo = getHumedadSuelo();
  float nivelA = getNivelA();

  String humedadString = String(humedad, 1);
  String temperaturaString = String(temperatura, 1);
  String humedadSueloString = String(humedadSuelo, 1);
  String nivelAString = String(nivelA, 1);

  Serial.println("nivelA=" + nivelAString + "&humedadSuelo=" + humedadSueloString + "&temperatura=" + temperaturaString +  "&humedad=" + humedadString +  "&riego=" + riego);

  lcd.setCursor(0, 0);
  lcd.print(temperaturaString);
  lcd.print("C ");
  lcd.print(humedadString);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print(humedadSueloString);
  lcd.print(" ");
  lcd.print(nivelAString);
  lcd.print(" ");

  delay(5000);

  if (humedadSuelo < 40) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Regando...");
    digitalWrite(rele, HIGH);
    delay(2000);
    digitalWrite(rele, LOW);
    lcd.clear();
    riego = 1;
  } else {
    riego = 0;
  }

  lcd.setCursor(0, 0);
  lcd.print(temperaturaString);
  lcd.print("C ");
  lcd.print(humedadString);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print(humedadSueloString);
  lcd.print(" ");
  lcd.print(nivelAString);
  lcd.print(" ");

  wifiBegin();
  Serial.println(WiFi.localIP());

  postHTTP(humedadString, temperaturaString, humedadSueloString, nivelAString, riego);

  WiFi.disconnect(true);
}

void postHTTP(String humedad, String temperatura, String humedadSuelo, String nivelA, int riego) {
  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;
    String dataPost = "nivelA=" + nivelA + "&humedadSuelo=" + humedadSuelo + "&temperatura=" + temperatura +  "&humedad=" + humedad;

    http.begin("url");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Authorization", "Token 05ad56b32d41ff7e4e7add4061b5d75823439de5");

    int dataRes = http.POST(dataPost);

    if (dataRes > 0) {
      Serial.println(String(dataRes));
    } else {
      Serial.print("Error:" + dataRes);
    }
    http.end();

  } else {
    Serial.println("Error al conectarse al WIFI");
  }

  if (riego == 1) {

    HTTPClient http;

    http.begin("url");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Authorization", "Token 05ad56b32d41ff7e4e7add4061b5d75823439de5");

    String text = "tipo=Automático";

    int dataRes = http.POST(text);

    if (dataRes > 0) {
      Serial.println(String(dataRes));
    } else {
      Serial.print("Error:" + dataRes);
    }
    http.end();
  }
}

float get_distance() {
  int duration, distance;
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(1000);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration / 2) / 29.1 ; //cm
  return distance;
}

float getNivelA() {
  float distancia = get_distance();
  float porcentaje;
  porcentaje = ((14 - distancia) * 100) / 11;
  return porcentaje;
}

float getHumedad () {
  float valor = dht.readHumidity();
  return valor;
}

float getHumedadSuelo () {
  analogReadResolution(10);
  float valor1 = analogRead(humPin);
  delay(100);
  return valor1;
}

float getTemp () {
  float valor = dht.readTemperature();
  return valor;
}

void wifiBegin() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Conectando...");
  }
  Serial.println("Conexión exitosa");
}
