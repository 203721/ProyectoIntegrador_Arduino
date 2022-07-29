#include <LiquidCrystal_I2C.h>
#include "WiFi.h"
#include "DHT.h"
#include <HTTPClient.h>

const int sensorHs = 34;
const int trigPin = 32;
const int echoPin = 33;
#define DHTTYPE DHT11
const int sensorDh = 14;
const int relay = 12;

int humedadS;
int lvl;
float temp, hum;
String riego = "no";

const char* ssid = "H";
const char* password = "Passwd123";


LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(sensorDh, DHTTYPE);

void setup() {
  Serial.begin(115200);
  pinMode(relay, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  dht.begin();
  lcd.begin(21, 22);
  lcd.backlight();
  conectarWifi();
}

void loop() {
  delay(1000);
  nivelAgua();
  delay(1000);
  leerdht11();
  delay(1000);
  humedadSuelo();
  delay(1000);
  if (not (isnan(temp) and isnan(hum))) {
    postDataToServer();
  }else{
    Serial.println("Siguiente...");
  }
  delay(1000);
}

//WIFI
void conectarWifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wifi conectado!");
}

//DHT11
void leerdht11() {
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  Serial.print("Temperatura: ");
  Serial.print(temp);
  Serial.print(" Humedad: ");
  Serial.print(hum);
  Serial.println();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print(" C");
  lcd.setCursor(0, 1);
  lcd.print("Humedad: ");
  lcd.print(hum);
}

//NIVEL DE AGUA
void nivelAgua() {
  float distancia = obtenerDistancia();
  float porcentaje;
  porcentaje = ((14 - distancia) * 100) / 11;
  lvl = porcentaje;
  Serial.print("Nivel: ");
  Serial.print(lvl);
  Serial.println();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nivel Agua:");
  lcd.setCursor(0, 1);
  if (lvl <= 0) {
    lcd.print("Vacio");
  }
  else if (lvl > 0 && lvl <= 25) {
    lcd.print("Bajo");
  }
  else if (lvl > 25 && lvl <= 50) {
    lcd.print("Medio");
  }
  else if (lvl > 50) {
    lcd.print("Lleno");
  }
}

float obtenerDistancia() {
  int duracion, distancia;
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(1000);
  digitalWrite(trigPin, LOW);
  duracion = pulseIn(echoPin, HIGH);
  distancia = (duracion / 2) / 29.1 ; //cm
  return distancia;
}

void humedadSuelo() {
  humedadS = analogRead(sensorHs);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Humedad Suelo:");
  lcd.setCursor(0, 1);
  if (humedadS == 4095) {
    lcd.print("Seco, ");
    lcd.print("Regando...");

    digitalWrite(relay, LOW);
    Serial.println("prender bomba");
    delay(2000);
    digitalWrite(relay, HIGH);
    Serial.println("apagar bomba");
    riego = "si";
    delay(1000);
  }
  else if (humedadS > 3000 && humedadS <= 4095) {
    lcd.print("Semiseco");
    riego = "no";
  }
  else if (humedadS > 2000 && humedadS <= 3000) {
    lcd.print("Semihumedo");
    riego = "no";
  }
  else if (humedadS < 2000) {
    lcd.print("Humedo");
    riego = "no";
  }
  Serial.print("humedad del suelo: ");
  Serial.print(humedadS);
  Serial.println();
  delay(1000);
}

void postDataToServer() {
  String dataPost = "nivelA=" + String(lvl) + "&humedadS=" + String(humedadS) + "&temperatura=" + String(temp, 1) +  "&humedad=" + String(hum, 1) + "&riego=" + riego;
  HTTPClient http;

  http.begin("https://192.168.97.49/django/datos/lista");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Authorization", "Token 9b8d43a32d446bdeb95b728d52fad59b3a4ce187");

  int res = http.POST(dataPost);
  Serial.println(String(res));
  http.end();
}
