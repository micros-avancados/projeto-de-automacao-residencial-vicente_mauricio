#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <stdlib.h>
#include <stdio.h>
#include <EEPROM.h>
#define MEM_ALOC_SIZE 64
#include <PubSubClient.h>
#define ID_MQTT "Projeto_1"
#define TOPIC_SUBSCRIBE ""

ESP8266WebServer server(80);
WiFiClient microsavancados;
PubSubClient client(microsavancados);

// Define nome da rede e senha a ser utilizado no SETUP CONFIGURAÇÃO
const char *ssid = "micros_emissor";
const char *password = "12345678";

//Variáveis internas
float temperatura;
char mensagem [100];

String novaRede;
String novaSenha;
char rede[30];
char senha[30];


//Váriaveis do MQTT
const char* BROKER_MQTT = "m10.cloudmqtt.com"; // ip/host do broker
int BROKER_PORT = 15557; // porta do broker
const char *BROKER_USER = "fvhdnjss";
const char *BROKER_PASSWORD = "HWW0wyt9laS-";

// Rotinas
void configura ();
void salvar ();
void espaco();

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                              SETUP GERAL
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void setup()
{
  // IRsend irsend(4)

  //Define entradas (D1 - modo oper.(L) conf. (H) && A0 - sensor de temperatura)
  pinMode (D1, INPUT);
  pinMode (A0, INPUT);



  //Inicia o monitor serial
  Serial.begin(115200);
  Serial.println();
  delay(1000);

  espaco();
  Serial.println("Sistemas Microprocessados Avançados");
  Serial.println("Trabalho nota 1");
  Serial.println("Mauricio Lorenzon && Vicente Bugs");
  espaco();


  //SETUP CONFIGURAÇÃO

  if (digitalRead(D1) == 1) {

    //Cria rede WiFi
    Serial.println("MODO: Configuração");
    espaco();

    //Cria rede com parametros pré definidos
    Serial.println("Configurando ponto de acesso...");
    WiFi.softAP(ssid, password);
    espaco();


    //Apresenta endereço de IP
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("Endereço de IP: ");
    Serial.println(myIP);

    //Se acessar tal IP - direciona para a rotina "CONFIGURA"
    server.on("/", configura);
    server.on("/salvar", salvar);

    server.begin();
    Serial.println("Servidor iniciado");
    espaco();
  }


  //SETUP OPERAÇÃO

  else {
    Serial.println("MODO: operação");
    espaco();

    EEPROM.begin(MEM_ALOC_SIZE);
    EEPROM.get(0, rede);
    Serial.print("Rede Wifi cadastrada: ");
    Serial.println(rede);

    EEPROM.get(30, senha);
    Serial.print("Senha de rede WiFi cadastrada: ");
    Serial.println(senha);

    EEPROM.end();
    espaco();

    //CONECTA A REDE WIFI
    WiFi.mode(WIFI_STA);
    WiFi.begin(rede, senha);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi conectado");

    // Iniciado o servidor
    server.begin();
    Serial.println("Iniciando o servidor");

    // Apresenta o endereço de IP
    Serial.println(WiFi.localIP());

    client.setServer(BROKER_MQTT, BROKER_PORT);


    while (!client.connected()) {
      Serial.println("Conectando ao servidor MQTT .... ");

      if (client.connect("ESP8266Client", BROKER_USER, BROKER_PASSWORD)) {
        Serial.println("Conectado");

      } else {
        Serial.print("Falha na conexão ");
        Serial.print(client.state());
        delay(2000);

      }
      espaco();
    }
  }
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                              FUNÇÃO LOOP
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void loop() {

  //Loop configuração

  if (digitalRead(D1) == 1) {
    server.handleClient();
  }


  //Loop operação

  else {
    client.loop();

    //Lê valor de temperatura do sensor conectado a porta A0
    float temperatura = (analogRead(A0) * 330.0) / 1023.0;    //Lê pino A0 e converte mV para ºC
    sprintf(mensagem, "%f", temperatura);                     //Converte valor de temperatura (float) to char

    //Publica no servidor o valor lido
    client.publish("temperatura", mensagem);                  //publica na tag 'temperatura' o valor de temperatura

    Serial.print("Temperatura envidada: ");
    Serial.println(mensagem);
    espaco();

    delay(2000);
  }
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                  EM MODO CONFIGURAÇÃO - LÊ OS DADOS DIGITADOS (REDE, SENHA, SETPOINT)
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void configura () {
  server.send(200, "text/html",
              "<html>\n"
              "<body>\n"
              "<form action=\"/salvar\">\n"
              "    Rede name: <br>\n"
              "    <input type=\"text\" name=\"NomeRede\" value=\"Informe a Rede\">\n"
              "    <br> Senha name: <br>\n"
              "    <input type=\"text\" name=\"SenhaRede\" value=\"Informe a senha\">\n"
              "    <br>\n"
              "    <input type=\"submit\" value=\"Submit\">\n"
              "</form>\n"
              "<p>If you click the \"Submit\" button, the form - data will be sent to a page called \"/salvar\".</p>\n"
              "</body>\n"
              "</html>"
             );
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                   SALVA OS DADOS INFORMADOS NA MEMÓRIA
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void salvar () {

  novaRede = server.arg("NomeRede");
  novaSenha = server.arg("SenhaRede");

  //Apresenta os dados informados
  Serial.print("Nome da rede: ");
  Serial.println(novaRede);
  Serial.print("Senha: ");
  Serial.println(novaSenha);

  //Salva os dados na memória EEPROM
  novaRede.toCharArray (rede, 30);
  novaSenha.toCharArray(senha, 30);

  EEPROM.begin(MEM_ALOC_SIZE);
  EEPROM.put(0, rede);
  EEPROM.put(30, senha);
  EEPROM.end();

  EEPROM.begin(MEM_ALOC_SIZE);

  EEPROM.end();

  server.send(200, "text/html", "<p>Dados salvos com sucesso</p>");
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                  FUNÇÃO ESPAÇO
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void espaco() {
  Serial.println("--------------------------------------------");
  Serial.println("");
}
