#include <WiFi.h>
#include <PubSubClient.h> // Biblioteca MQTT
#include <DHT.h>

// --- Configurações Wi-Fi e MQTT ---
const char* default_SSID = "SUA REDE WI-FI";
const char* default_PASSWORD = "SENHA DA SUA REDE WI-FI";
const char* default_BROKER_MQTT = "SEU IP"; // IP do Broker MQTT (VM)
const int default_BROKER_PORT = 1883; // Porta do Broker MQTT

// --- Configurações - Dispositivo FIWARE ---
const char* default_TOPICO_PUBLISH_1 = "/smart/012/attrs";
const char* default_ID_MQTT = "ESP32Client_012"; // ID MQTT

// Variáveis para configurações
char* SSID = const_cast<char*>(default_SSID);
char* PASSWORD = const_cast<char*>(default_PASSWORD);
char* BROKER_MQTT = const_cast<char*>(default_BROKER_MQTT);
int BROKER_PORT = default_BROKER_PORT;
char* TOPICO_PUBLISH_1 = const_cast<char*>(default_TOPICO_PUBLISH_1);
char* ID_MQTT = const_cast<char*>(default_ID_MQTT);

WiFiClient espClient;
PubSubClient client(espClient);

// --- Pinos e Lógica dos Sensores ---
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
const int LDR = 32;
const int green = 2;
const int yellow = 18;
const int red = 5;

const int lumOK = 350;
const int lumALERT = 800;
const int tempMIN = 10;
const int tempMAX = 15;
const int umidMIN = 50;
const int umidMAX = 70;

int leiturasLDR[5] = {0};
float leiturasTemp[5] = {0};
int leiturasUmid[5] = {0};
int indice = 0;
int estado = 0;

unsigned long tempoUltimaLeitura = 0;
unsigned long tempoUltimaExibicao = 0;
unsigned long tempoUltimoMqttReconnect = 0;

const unsigned long intervaloLeitura = 1000;
const unsigned long intervaloExibicao = 5000;
const unsigned long intervaloMqttReconnect = 5000;

void setup() {
  pinMode(green, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(red, OUTPUT);
  Serial.begin(9600);
  dht.begin();

  setup_wifi();
  client.setServer(BROKER_MQTT, BROKER_PORT);
}

void setup_wifi() {
  Serial.println();
  Serial.print("Conectando em ");
  Serial.println(SSID);

  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}


void tentaReconectarMqtt() {
  Serial.print("Tentando conexão MQTT...");
  if (client.connect(ID_MQTT)) {
    Serial.println("conectado");
  } else {
    Serial.print("falhou, rc=");
    Serial.print(client.state());
    Serial.println(" tentando novamente em 5 segundos");
  }
}

void loop() {
  unsigned long agora = millis();

  if (!client.connected()) {
    if (agora - tempoUltimoMqttReconnect >= intervaloMqttReconnect) {
      tempoUltimoMqttReconnect = agora;
      tentaReconectarMqtt();
    }
  }

  client.loop();

  float temp = dht.readTemperature();
  int umid = dht.readHumidity();
  int leituraLDR = analogRead(LDR);
  leituraLDR = map(leituraLDR, 0, 1023, 140, 970);

  // Bloco de Leitura
  if (agora - tempoUltimaLeitura >= intervaloLeitura) {
    tempoUltimaLeitura = agora;

    if (isnan(temp) || isnan(umid)) {
      Serial.println("Erro na leitura do DHT11");
      return;
    }

    leiturasLDR[indice] = leituraLDR;
    leiturasTemp[indice] = temp;
    leiturasUmid[indice] = umid;
    indice = (indice + 1) % 5;
  }

  // Bloco de Exibição e Envio de Dados
  if ( agora - tempoUltimaExibicao >= intervaloExibicao) {
    tempoUltimaExibicao = agora;

    int somaLDR = 0;
    float somaTemp = 0;
    int somaUmid = 0;
    for (int i = 0; i < 5; i++) {
      somaLDR += leiturasLDR[i];
      somaTemp += leiturasTemp[i];
      somaUmid += leiturasUmid[i];
    }

    float mediaLDR = somaLDR / 5.0;
    float mediaTemp = somaTemp / 5.0;
    float mediaUmid = somaUmid / 5.0;

    Serial.println("---- Enviando Dados ----");
    Serial.print("Temperatura: "); Serial.println(mediaTemp);
    Serial.print("Umidade: "); Serial.println(mediaUmid);
    Serial.print("Luminosidade:"); Serial.println(mediaLDR);

    // --- Lógica de Estado (LEDs e Frases) ---
    String statusMessage = "";
    estado = 0;
    if ( (mediaLDR > lumOK) || (mediaTemp < tempMIN) || (mediaTemp > tempMAX) ) {
      estado = 1;
    }
    if ( (mediaLDR >= lumALERT) || (mediaUmid < umidMIN) || (mediaUmid > umidMAX) ) {
      estado = 2;
    }

    // Define a frase com base no estado
    switch(estado) {
        case 0:
            statusMessage = "Ambiente OK.";
            break;
        case 1:
            statusMessage = "Atencao: parametros fora do ideal.";
            break;
        case 2:
            statusMessage = "ALERTA: Condicoes criticas.";
            break;
    }
    Serial.print("Status: ");
    Serial.println(statusMessage); // Imprime a frase no Serial

    // --- ENVIAR PARA O FIWARE ---
    char payload[200]; // Buffer aumentado para a frase
    // Adiciona o novo atributo 'msg' (status) ao payload
    sprintf(payload, "t|%.2f|u|%.0f|l|%.0f|msg|%s",
            mediaTemp, mediaUmid, mediaLDR, statusMessage.c_str());

    Serial.print("Publicando no tópico: ");
    Serial.println(TOPICO_PUBLISH_1);
    Serial.print("Payload: ");
    Serial.println(payload);

    if (client.connected()) {
        if (client.publish(TOPICO_PUBLISH_1, payload)) {
            Serial.println("Payload enviado com sucesso!");
        } else {
            Serial.println("Falha ao enviar payload.");
        }
    } else {
        Serial.println("MQTT desconectado. Payload não enviado.");
    }
    // ----------------------------
  }

  defineEstado(estado); // Atualiza os LEDs
}

void defineEstado(int estado) {
  switch (estado) {
      case 0: // OK
        digitalWrite(red, LOW); digitalWrite(yellow, LOW); digitalWrite(green, HIGH);
        break;
      case 1: // Atenção
        digitalWrite(red, LOW); digitalWrite(yellow, HIGH); digitalWrite(green, LOW);
        break;
      case 2: // Alerta
        digitalWrite(red, HIGH); digitalWrite(yellow, LOW); digitalWrite(green, LOW);
        break;
  }
}
