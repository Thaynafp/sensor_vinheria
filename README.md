🍇 Projeto IoT - Monitoramento da Vinheria Agnello 🍷
1. Desenvolvedores
Leonardo Grosskopf

Thayná Lopes

2. Descrição do Projeto
Este projeto é uma solução de Internet das Coisas (IoT) desenvolvida para o "Caso da Vinheria Agnello", como parte do Checkpoint 05 de Edge Computing & Computer Systems .

O desafio consiste em implementar um sistema de monitoramento para o depósito de vinhos, capaz de controlar fatores críticos que afetam a qualidade da bebida: Temperatura 🌡️, Umidade 💧 e Luminosidade 💡 . O sistema não só alerta sobre condições inadequadas, mas também armazena um histórico de dados para rastreabilidade e permite a leitura e escrita de dados remotamente, atendendo aos requisitos dos proprietários .



Para atender aos requisitos de interoperabilidade do mercado europeu , esta solução utiliza a plataforma FIWARE como backend, implementando o padrão NGSIv2 para a interface de dados .



3. Tecnologias Utilizadas
4. Arquitetura da Solução ⛓️
O fluxo de dados segue a arquitetura padrão do FIWARE :


Camada de IoT (ESP32): Um microcontrolador ESP32 , equipado com sensores DHT11 (Temperatura e Umidade) e LDR (Luminosidade), coleta os dados da adega.



Camada de Backend (FIWARE/Docker):

O ESP32 envia os dados via Wi-Fi para um Broker MQTT (fiware-mosquitto).

O IoT-Agent (fiware-iot-agent) assina o tópico MQTT, recebe os dados brutos (no formato UltraLight) e os traduz para o padrão NGSIv2.

O IoT-Agent encaminha os dados traduzidos para o Orion Context Broker (o "cérebro" do FIWARE), que armazena o estado atual do dispositivo.

Camada de Aplicação:

O STH-Comet assina o Orion e salva cada mudança de dados no MongoDB, criando o histórico de dados.


O usuário pode usar o Postman ou o app MyMQTT para interagir com o sistema, seja lendo os dados (atuais ou históricos) ou "escrevendo" comandos para o dispositivo.

5. Conteúdo do Repositório
docker-compose.yml: Arquivo de configuração do Docker para inicializar toda a pilha de serviços FIWARE.

fiware-vinheria.postman_collection.json: A coleção do Postman contendo todas as requisições pré-configuradas para provisionar e interagir com o sistema.

ESP32_SENSOR.ino: O código-fonte para o ESP32.

README.md: Esta documentação.

6. Requisitos para Replicação
Hardware 🔌
Placa ESP32 (como a ESP32-VROOM-32D) 

Placa Shield de Sensores (conforme a imagem do projeto) 

Sensor DHT11 (Temperatura e Umidade) 


Sensor LDR (Luminosidade) 


LEDs (Verde, Amarelo, Vermelho) e jumpers.

Software 💻
Docker

Postman

Arduino IDE (com a placa ESP32 instalada)

MyMQTT (Aplicativo móvel)

Git

7. Instruções para Replicação
Siga estes passos para configurar e executar o projeto completo.

Passo 1: Iniciar o Backend (FIWARE) 🐳
O primeiro passo é subir os serviços do FIWARE usando o Docker.

Clone este repositório:

Bash

git clone https://github.com/Thaynafp/sensor_vinheria.git
cd sensor_vinheria
Suba os contêineres do Docker usando o arquivo docker-compose.yml:

Bash

sudo docker compose up -d
Verifique se todos os serviços estão rodando e "healthy" (saudáveis). Pode levar um minuto para todos iniciarem.

Bash

sudo docker ps
(Se algum serviço, como o fiware-orion, estiver "Restarting", aguarde ou use sudo docker compose down e sudo docker compose up -d para reiniciar).

Passo 2: Configurar o FIWARE (Postman) 📮
Agora, vamos usar o Postman para provisionar (configurar) o nosso dispositivo no FIWARE.

Abra o Postman e importe a coleção fiware-vinheria.postman_collection.json deste repositório.

Encontre o IP da sua máquina/VM: No terminal onde o Docker está rodando, use o comando ip addr show (no Linux) ou ipconfig (no Windows) e anote o seu endereço IP.

Configure o Postman: Na coleção importada, clique no nome dela, vá para a aba "Variables" e mude o valor da variável {{url}} para o IP que você acabou de anotar.

Execute as Requisições: Execute as seguintes requisições na ordem:

(IoT-Agent) 1.1 Health Check: Deve retornar 200 OK.

(IoT-Agent) 2. Provisioning a Service Group: Deve retornar 201 Created.


(IMPORTANTE) (IoT-Agent) 3. Provision Sensor Station (Device 012): Esta requisição registra nosso dispositivo 012 com os 3 sensores e o comando setTempMax . Deve retornar 201 Created.


(IMPORTANTE) (STH-Comet) 2. Subscribe Sensor History (Device 012): Esta requisição diz ao FIWARE para salvar o histórico dos nossos 3 sensores . Deve retornar 201 Created.

Passo 3: Configurar o Hardware (ESP32) ⚡
Agora, vamos programar o ESP32 para enviar os dados dos sensores.

Monte o hardware conforme o diagrama do projeto .

DHT11 (Dados): Pino D15

LDR (Analógico): Pino D32

LED Verde: Pino D2

LED Amarelo: Pino D4

LED Vermelho: Pino D5

Abra a Arduino IDE e instale as bibliotecas necessárias: PubSubClient (por Nick O'Leary) e DHT sensor library (por Adafruit).

Abra o arquivo ESP32_SENSOR.ino deste repositório na sua Arduino IDE.

No código, atualize as 3 variáveis no topo:

C++

// --- Configurações (MUDE AQUI) ---
const char* ssid = "NOME_DA_SUA_REDE_WIFI";
const char* password = "SENHA_DA_SUA_REDE_WIFI";
const char* mqtt_server = "<SEU_IP_AQUI>"; // <-- MUDE AQUI para o IP do seu servidor FIWARE
// ---------------------------------
Carregue o código no seu ESP32 e abra o Monitor Serial. Você deverá ver os dados sendo enviados a cada 5 segundos.

Passo 4: Teste e Uso (Leitura e Escrita) 📱
Agora vamos interagir com o sistema, conforme o requisito do PDF.

Leitura de Dados (ESP32 -> MyMQTT)
Abra o app MyMQTT no seu celular (conectado ao mesmo Wi-Fi).

Adicione o Broker:

Broker Address: <SEU_IP_AQUI> (O IP da sua VM)


Port: 1883 

Conecte-se e vá para a aba "Subscribe".

Assine o tópico de atributos:

Tópico: /smart/012/attrs

Resultado: Você verá as leituras do seu ESP32 (ex: t|25.50|u|60|l|400) chegando em tempo real no seu celular.