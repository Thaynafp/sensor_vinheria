# 🍇 Projeto IoT - Monitoramento da Vinheria Agnello 🍷

[![FIWARE](https://img.shields.io/badge/FIWARE-005383?style=for-the-badge&logo=fiware&logoColor=white)](https://www.fiware.org/)
[![Docker](https://img.shields.io/badge/Docker-2496ED?style=for-the-badge&logo=docker&logoColor=white)](https://www.docker.com/)
[![Postman](https://img.shields.io/badge/Postman-FF6C37?style=for-the-badge&logo=postman&logoColor=white)](https://www.postman.com/)
[![MQTT](https://img.shields.io/badge/MQTT-660066?style=for-the-badge&logo=mqtt&logoColor=white)](https://mqtt.org/)
[![Arduino](https://img.shields.io/badge/Arduino-00979D?style=for-the-badge&logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)](https://isocpp.org/)

## 1. Descrição do Projeto

Este projeto é uma solução de Internet das Coisas (IoT) desenvolvida para o Checkpoint 05 de Edge Computing & Computer Systems: "O Caso da Vinheria Agnello".

O desafio é evoluir um sistema de monitoramento para a adega, controlando os fatores críticos que afetam a qualidade do vinho (Luminosidade, Temperatura e Umidade), e atendendo aos novos requisitos dos proprietários:

> "...precisamos monitorar a temperatura e a umidade do ambiente... eu preciso saber exatamente qual a temperatura e a umidade do depósito..."

> "Preciso ter os dados históricos dos últimos dias e meses!"

> "Queremos integrar nossa empresa a distribuidoras presentes no mercado Europeu"

Para atender a essas demandas, a solução implementa a plataforma FIWARE, o protocolo MQTT, e armazena o histórico de dados para rastreabilidade.

## 2. Desenvolvedores

  - Leonardo Grosskopf RM: 562255
  - Thayná Lopes RM: 566349

## 3. Arquitetura da Solução

O fluxo de dados segue a arquitetura padrão do FIWARE, conforme ilustrado no diagrama do desafio:

1.  **Camada de IoT (ESP32):** Um microcontrolador ESP32 coleta dados dos sensores (DHT11/LDR).
2.  **Camada de Backend (FIWARE/Docker):**
      - **MQTT Broker** (fiware-mosquitto, Porta 1883) recebe os dados.
      - **IoT-Agent** (Porta 4041) traduz as mensagens MQTT (UltraLight) para o padrão NGSIv2.
      - **Orion Context Broker** (Porta 1026) armazena o estado atual da entidade.
3.  **Camada de Aplicação:**
      - **STH-Comet** (Porta 8666) assina o Orion e salva o histórico no MongoDB.
      - **Usuário** (Postman/MyMQTT) interage com o sistema via API (NGSIv2).

## 4. Conteúdo do Repositório

```
.
├── ESP32_SENSOR/
│   └── ESP32_SENSOR.ino
├── docker-compose.yml
├── fiware-vinheria.postman_collection.json
└── README.md
```

## 5. Requisitos

### Hardware 🔌

  - Placa ESP32 (ESP32-VROOM-32D)
  - Sensor DHT11 (Temperatura e Umidade)
  - Sensor LDR (Luminosidade)

### Software 💻

  - Docker e Docker Compose (para rodar os serviços FIWARE)
  - Arduino IDE (com o board do ESP32 instalado)
  - Postman (para configurar e testar a API)
  - MyMQTT (ou qualquer cliente MQTT para teste no celular)

## 6. Guia de Replicação

Siga estes passos para configurar e executar o projeto completo.

### Passo 1: Iniciar o Backend (FIWARE)

1.  Clone este repositório:
    ```bash
    git clone <URL_DO_SEU_REPOSITORIO>
    cd <NOME_DA_PASTA>
    ```
2.  Suba os contêineres do Docker usando o arquivo `docker-compose.yml`:
    ```bash
    sudo docker compose up -d
    ```
3.  Verifique se todos os serviços estão rodando e "healthy":
    ```bash
    sudo docker compose ps -a
    ```
    (Aguarde um minuto. Se o `fiware-orion` estiver "Restarting", pare com `sudo docker compose down` e tente novamente).

### Passo 2: Configurar o FIWARE (Postman)

1.  Abra o Postman e importe a coleção `fiware-vinheria.postman_collection.json`.
2.  Encontre o IP da sua máquina/VM (o host do Docker):
    ```bash
    # No Linux
    ip a
    # No macOS/Linux
    ifconfig
    ```
3.  **Configure o Postman:** Na coleção importada, clique no nome dela -> aba "Variables" -> edite a variável `{{url}}` para o IP que você acabou de anotar (ex: `http://192.168.1.10`).
4.  Execute as Requisições na ordem:
    1.  `(IoT-Agent) 1.1 Health Check` (Espera: 200 OK)
    2.  `(IoT-Agent) 2. Provisioning a Service Group` (Espera: 201 Created)
    3.  **(IMPORTANTE)** `(IoT-Agent) 3. Provision Sensor Station (Device 012)` (Espera: 201 Created)
    4.  **(IMPORTANTE)** `(STH-Comet) 2. Subscribe Sensor History (Device 012)` (Espera: 201 Created)

### Passo 3: Configurar o Hardware (ESP32)

1.  Monte o hardware conforme o diagrama do projeto:
      - DHT11 (Dados): `Pino D15`
      - LDR (Analógico): `Pino D32`
      - LED Verde: `Pino D2`
      - LED Amarelo: `Pino D4`
      - LED Vermelho: `Pino D5`
2.  Abra a Arduino IDE e instale as bibliotecas `PubSubClient` (por Nick O'Leary) e `DHT sensor library` (por Adafruit).
3.  Abra o arquivo `ESP32_SENSOR.ino` deste repositório na sua Arduino IDE.
4.  No código, atualize as variáveis de configuração:
    ```cpp
    // Configure seu WiFi
    const char* SSID = "NOME_DA_SUA_REDE_WIFI";
    const char* PASSWORD = "SENHA_DA_SUA_REDE_WIFI";

    // Configure o IP do Broker MQTT (IP da sua VM/Host Docker)
    const char* BROKER_MQTT = "SEU_IP_AQUI"; 
    ```
5.  Carregue o código no ESP32 e abra o Monitor Serial (Baud 9600). Você verá os dados sendo enviados.

### Passo 4: Teste e Uso (MyMQTT)

Este passo cumpre o requisito de "leitura e escrita dos dados de IoT pelo aplicativo MyMQTT".

#### 4.1. Leitura de Dados (ESP32 -> MyMQTT)

1.  Abra o app MyMQTT no seu celular (conectado ao mesmo Wi-Fi).
2.  Adicione o Broker:
      - Broker Address: `<SEU_IP_AQUI>` (O IP da sua VM)
      - Port: `1883`
3.  Conecte-se e vá para a aba "Subscribe".
4.  Assine o tópico de atributos: `/smart/012/attrs`
5.  **Resultado:** Você verá as leituras do seu ESP32 (ex: `t|25.50|u|60|l|400`) chegando em tempo real.
