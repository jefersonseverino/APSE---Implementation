#include <Arduino.h>
#include <ArduinoJson.h> // IMPORTANTE: Instale esta biblioteca
#include <PubSubClient.h>
#include <WiFi.h>
#include <cstdlib>

// --- CONFIGURAÇÃO ---
const char *ssid = "";
const char *password = "";

const char *mqtt_server = "";
const char *mqtt_user = "";
const char *mqtt_password = "";

const char *topic_dashboard = "teste/arduino"; // Tópico que o JS escuta
const char *client_id = "ArduinoClient_01";

const int lane_1_ir_1_sensor_pin = 22;
const int lane_1_ir_2_sensor_pin = 21;
const int lane_1_ir_3_sensor_pin = 23;

const int lane_2_ir_1_sensor_pin = 34;
const int lane_2_ir_2_sensor_pin = 35;
const int lane_2_ir_3_sensor_pin = 32;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastPublishTime = 0;
const long publishInterval = 1000;

unsigned long lastLightChange = 0;
int trafficLightState = 0;
const long lightInterval = 5000;
String lastOpenState = "";

struct laneSensorConfig {
  int ir_sensor_pins[3];
};

void reconnect() {
  if (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    if (client.connect(client_id, mqtt_user, mqtt_password)) {
      Serial.println("Conectado!");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 2s.");
      delay(2000);
    }
  }
}

void setup_wifi() {
  Serial.print("Conectando-se a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}

String calculateTrafficLevel(laneSensorConfig lane) {

  int cnt = 0;
  if (lane.ir_sensor_pins[0] == HIGH)
    cnt++;
  if (lane.ir_sensor_pins[1] == HIGH)
    cnt++;
  if (lane.ir_sensor_pins[2] == HIGH)
    cnt++;

  if (cnt == 0)
    return "LIVRE";
  if (cnt == 1)
    return "LEVE";
  if (cnt == 2)
    return "MODERADO";
  if (cnt >= 3)
    return "INTENSO";

  return "LIVRE";
}

String handleTrafficLight(laneSensorConfig lane1, laneSensorConfig lane2) {
  // unsigned long now = millis();

  // if (now - lastLightChange > lightInterval) {
  //   trafficLightState++;
  //   if (trafficLightState > 3) trafficLightState = 0;
  //   lastLightChange = now;
  // }

  // switch (trafficLightState) {
  //   case 0: return "S1_VERDE";   // S1 Verde, S2 Vermelho
  //   case 1: return "S1_AMARELO"; // S1 Amarelo, S2 Vermelho
  //   case 2: return "S2_VERDE";   // S1 Vermelho, S2 Verde
  //   case 3: return "S2_AMARELO"; // S1 Vermelho, S2 Amarelo
  //   default: return "S1_VERDE";
  // }

  // int cnt = 0;
  // if (lane1.ir_sensor_pins[0] == HIGH) cnt++;
  // if (lane1.ir_sensor_pins[1] == HIGH) cnt++;
  // if (lane1.ir_sensor_pins[2] == HIGH) cnt++;

  // int cnt_opp = 0;
  // if (lane2.ir_sensor_pins[0] == HIGH) cnt_opp++;

  // Serial.println(cnt);
  // Serial.println(cnt_opp);

  // if (cnt == 3 && cnt_opp == 0) return "S1_VERDE";
  // if (cnt == 2 && cnt_opp == 0) return "S1_VERDE";
  // if (cnt == 1 && cnt_opp == 0) return "S1_AMARELO";

  // return "S2_VERDE";

  // String lane1_transit_level = calculateTrafficLevel(lane1);
  // String lane2_transit_level = calculateTrafficLevel(lane2);

  // if (lane1_transit_level == lane2_transit_level) {
  //   lastOpenState = random(0, 2) == 0 ? "S1_VERDE" : "S2_VERDE";
  // } else {
    
  // }

  // return lastOpenState;
  return "S2_VERDE";
}

void publishData(laneSensorConfig lane1, laneSensorConfig lane2) {
  StaticJsonDocument<200> doc;

  doc["estado"] = handleTrafficLight(lane1, lane2); // Ex: "S1_VERDE"
  doc["transito"] = calculateTrafficLevel(lane1);   // Ex: "INTENSO"

  doc["ambulancia"] = false;
  doc["pedestre"] = false;

  char buffer[256];
  serializeJson(doc, buffer);

  client.publish(topic_dashboard, buffer);

  Serial.print("JSON Enviado: ");
  Serial.println(buffer);
}

void setup() {
  Serial.begin(9600);

  // lane 1
  pinMode(lane_1_ir_1_sensor_pin, INPUT);
  pinMode(lane_1_ir_2_sensor_pin, INPUT);
  pinMode(lane_1_ir_3_sensor_pin, INPUT);

  // lane 2
  pinMode(lane_2_ir_1_sensor_pin, INPUT);
  pinMode(lane_2_ir_2_sensor_pin, INPUT);
  pinMode(lane_2_ir_3_sensor_pin, INPUT);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();

  if (now - lastPublishTime >= publishInterval) {
    lastPublishTime = now;

    int lane_1_ir_1 = digitalRead(lane_1_ir_1_sensor_pin);
    int lane_1_ir_2 = digitalRead(lane_1_ir_2_sensor_pin);
    int lane_1_ir_3 = digitalRead(lane_1_ir_3_sensor_pin);

    laneSensorConfig lane1 = {
        .ir_sensor_pins = {lane_1_ir_1, lane_1_ir_2, lane_1_ir_3}};

    int lane_2_ir_1 = digitalRead(lane_2_ir_1_sensor_pin);
    int lane_2_ir_2 = digitalRead(lane_2_ir_2_sensor_pin);
    int lane_2_ir_3 = digitalRead(lane_2_ir_3_sensor_pin);

    laneSensorConfig lane2 = {.ir_sensor_pins = {lane_2_ir_1, lane_2_ir_2, lane_2_ir_3}};

    Serial.print("Lane 1 Sensors: ");
    Serial.print(lane_1_ir_1);
    Serial.print(", ");
    Serial.print(lane_1_ir_2);
    Serial.print(", ");
    Serial.println(lane_1_ir_3);

    Serial.print("Lane 2 Sensors: ");
    Serial.print(lane_2_ir_1);
    Serial.print(", ");
    Serial.print(lane_2_ir_2);
    Serial.print(", ");
    Serial.println(lane_2_ir_3);

    publishData(lane1, lane2);
  }
}