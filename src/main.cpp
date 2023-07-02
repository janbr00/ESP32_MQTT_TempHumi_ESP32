#include <Arduino.h>
#include <WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>

#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// WLAN-Zugang
const char *ssid = "";
const char *passwort = "";

// MQTT-Zugang
const char *mqtt_server = "192.168.178.36";

// Topics
#define TOPIC_TEMP "sensor/DHT22/temperatur"
#define TOPIC_HUMI "sensor/DHT22/feuchtigkeit"
#define TOPIC_HEIZ "led/ESP32/message"
#define TOPIC_BAT "Batterie"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

int lastMillis = 0;
int currentMillis;
const int interval = 5000;

float temperatur;
float feuchtigkeit;

const int Heizung = 17;

void testMQTTConnection()
{
  while (!mqttClient.connected())
  {
    Serial.println("Verbindung zum MQTT-Server wird hergestellt...");

    if (mqttClient.connect("ESP32"))
    {
      Serial.println("ESP32 mit MQTT-SERVER verbunden!");
      mqttClient.subscribe(TOPIC_HEIZ);
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  String message;
  for (int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }
  // Serial.println(message);
  // Serial.println(topic);

  if (message.equals("1"))
  {
    digitalWrite(Heizung, HIGH);
  }
  else if (message.equals("0"))
  {
    digitalWrite(Heizung, LOW);
  }
}

void setup()
{
  dht.begin();
  Serial.begin(115200);
  WiFi.begin(ssid, passwort);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Verbindung zum WiFi-Netzwerk wird hergestellt...");
  }
  Serial.println("ESP32 mit dem WiFi-Netzwerk verbunden!");
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());

  pinMode(Heizung, OUTPUT);
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);
}

void loop()
{
  if (!mqttClient.connected())
  {
    testMQTTConnection();
  }
  mqttClient.loop();

  currentMillis = millis();
  if (currentMillis - lastMillis >= interval)
  {
    lastMillis = currentMillis;

    temperatur = dht.readTemperature();
    feuchtigkeit = dht.readHumidity();

    Serial.print("Temperatur: ");
    Serial.print(temperatur);
    Serial.print(" °C");
    mqttClient.publish(TOPIC_TEMP, String(temperatur).c_str(), true);
    Serial.println();

    Serial.print("Feuchtigkeit: ");
    Serial.print(feuchtigkeit);
    Serial.print(" %");
    mqttClient.publish(TOPIC_HUMI, String(feuchtigkeit).c_str(), true);
    Serial.println();
    Serial.println("-----------------------");
  }
}
