#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define DHTPIN 12 // Chân GPIO mà DHT được kết nối
#define DHTTYPE DHT11 // Hoặc DHT11
#define LDR_PIN A0 // Chân GPIO cho cảm biến ánh sáng

const char* ssid = "372 DBT";
const char* password = "0973702742";
const char* mqtt_server = "test.mosquitto.org"; // Địa chỉ IP của MQTT broker

WiFiClient espClient;
PubSubClient client(espClient); 
DHT dht(DHTPIN, DHTTYPE);

void callback(char* topic, byte* message, unsigned int length){
  Serial.print(topic);
  String stMessage;
  for (int i = 0; i < length; i++){
    stMessage += (char)message[i];
  }
  Serial.print(stMessage);
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  WiFi.setOutputPower(19.25);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("22127148")) {
      Serial.println("Connected to MQTT broker");
      client.subscribe("22127148/led");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  delay(2000); // Thời gian trễ

  // Đọc dữ liệu từ cảm biến DHT
  float Temperature = dht.readTemperature();
  float Humidity = dht.readHumidity();

  // Debugging output
  // Serial.print("Temperature: ");
  // Serial.println(Temperature);
  // Serial.print("Humidity: ");
  // Serial.println(Humidity);

  if (isnan(Humidity) || isnan(Temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  int Light = 1023 - analogRead(LDR_PIN); // Đọc giá trị từ cảm biến ánh sáng

  // Tạo JSON
  StaticJsonDocument<200> doc;
  doc["Hum"] = Humidity;
  doc["Temp"] = Temperature;
  doc["Light"] = Light;

  char buffer[256];
  size_t n = serializeJson(doc, buffer);

  // Gửi dữ liệu JSON tới MQTT broker
  if (client.connected()) {
    client.publish("home/sensors/dht", buffer, n);
    Serial.println("Data sent: ");
    Serial.println(buffer);
  } else {
    Serial.println("Connection to MQTT broker lost");
  }
}
