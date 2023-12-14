#include <WiFi.h>
#include <PubSubClient.h>

const char *ssid = "ERIXXKXON";
const char *password = "HhiperZonic2";
const char *mqttServer = "161.132.41.45";
const int mqttPort = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

const int salidaPins[] = {21, 19, 18, 27};  // Pines de salida (relés y buzzer)
const int sensorPIR = 26;  // Pin del sensor PIR
const int PIRpower = 14;  // Pin para alimentar el sensor PIR

String outStates[] = {"apagado", "apagado", "apagado", "apagado"};  // Lista de estados de cada salida

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Conexión WiFi establecida");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Control de focos
  for (int i = 0; i < 4; i++) {
    String topicForFoco = "esp/foco" + String(i + 1);
    if (String(topic) == topicForFoco) {
      digitalWrite(salidaPins[i], payload[0] == '1' ? HIGH : LOW);
      outStates[i] = payload[0] == '1' ? "encendido" : "apagado";

    }
  }

  // Control de sensor
  if (String(topic) == "esp/alarma") {
    digitalWrite(PIRpower, payload[0] == '1' ? HIGH : LOW);
    outStates[3] = payload[0] == '1' ? "encendido" : "apagado";
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado");
      for (int i = 0; i < 4; i++) {
        String topicForFoco = "esp/foco" + String(i + 1);
        client.subscribe(topicForFoco.c_str());
      }
      client.subscribe("esp/alarma");
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" Intentando de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  for (int i = 0; i < 4; i++) {
    pinMode(salidaPins[i], OUTPUT);
    digitalWrite(salidaPins[i], LOW);
  }

  pinMode(sensorPIR, INPUT);
  pinMode(PIRpower, OUTPUT);
  digitalWrite(PIRpower, LOW);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Leer el sensor PIR y encender/apagar el LED según el movimiento
  int pirValue = digitalRead(sensorPIR);
  digitalWrite(salidaPins[3], pirValue == HIGH ? HIGH : LOW);

  if (client.connected()) {
    static unsigned long lastMillis = 0;
    if (millis() - lastMillis > 5000) {
      lastMillis = millis();
    }
  }
}
