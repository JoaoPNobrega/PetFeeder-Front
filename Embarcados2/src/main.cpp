#include "Arduino.h"
#include <ESP32Servo.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <WebServer.h>
#include "FS.h"
#include "LittleFS.h"

// ------------------------- DEFINIÇÕES -------------------------
#define WIFI_SSID "VIAWEB_FIBRA_PEDRINHO_33382360"
#define WIFI_PASSWORD "Apolonobrega25"
#define DATABASE_URL "https://petfeed-se-default-rtdb.firebaseio.com/"
#define DATABASE_SECRET "N85woKV3Tfz1UIWfk0RPAUsLWUcT2FPmuAGErA1h"

#define TRIG_PIN 23
#define ECHO_PIN 22
#define SERVO_PIN 27
#define POTENTIOMETER_PIN 32
#define LED_PIN 18
#define BOTAO_PIN 33

Servo servo1;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
WebServer server(80);

bool estadoAnteriorBotao = HIGH;
bool servoAtivo = false;
unsigned long tempoAcionamento = 0;
String origemPrioritaria = "potenciometro";  // padrão inicial
int tempoEmMs = 1000;
int ultimoPotIndice = -1;

String currentStatus = "Sistema pronto!"; // <-- NOVO: Guarda o status atual
bool staConectado = false; // <-- NOVO: Indica se STA conectou

// ------------------------- HTML SIMPLES (modo AP) com POLLING -------------------------
const char SIMPLE_HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head><meta charset="UTF-8" /><meta name="viewport" content="width=device-width, initial-scale=1.0"/>
<title>PetFeeder Local</title>
<style>
  body { font-family: Arial, sans-serif; background: #f0f0f0; padding: 20px; text-align: center; } 
  button, input { padding: 12px 25px; margin: 10px; font-size: 18px; border-radius: 5px; border: none; cursor: pointer; } 
  button { background-color: #4CAF50; color: white; } 
  button:hover { background-color: #45a049; }
  input { border: 1px solid #ccc; }
  #status, #liveStatus { margin-top: 20px; font-size: 16px; font-weight: bold; padding: 10px; background: #fff; border-radius: 5px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
  h1 { color: #333; }
</style>
</head>
<body>
<h1>Controle Local PetFeeder</h1>
<input type="number" id="tempo" placeholder="Tempo em ms (1000-10000)" min="1000" max="10000" value="3000"><br>
<button onclick="abrir()">🔓 Abrir Porta</button>
<div id="status">Status Ação: aguardando...</div>
<div id="liveStatus">Status Geral: carregando...</div>

<script>
  // Função para abrir a porta
  async function abrir() {
    const tempo = document.getElementById('tempo').value || 3000;
    const statusDiv = document.getElementById("status");
    statusDiv.innerText = "Status Ação: Enviando comando...";
    try {
      const res = await fetch("/abrir?tempo=" + tempo);
      const txt = await res.text();
      statusDiv.innerText = "Status Ação: " + txt;
    } catch (error) {
      statusDiv.innerText = "Status Ação: Erro ao conectar ao PetFeeder!";
      console.error("Erro no fetch /abrir:", error);
    }
  }

  // Função para buscar status (Polling)
  async function fetchStatus() {
      const liveStatusDiv = document.getElementById("liveStatus");
      try {
          const res = await fetch("/status");
          const txt = await res.text();
          liveStatusDiv.innerText = "Status Geral: " + txt;
      } catch (error) {
          liveStatusDiv.innerText = "Status Geral: Erro ao buscar status!";
          console.error("Erro no fetch /status:", error);
      }
  }

  // Inicia o Polling quando a página carrega
  window.onload = function() {
      fetchStatus(); // Busca o status imediatamente
      setInterval(fetchStatus, 2000); // Busca o status a cada 2 segundos
  };
</script>
</body>
</html>
)rawliteral";

// ------------------------- FUNÇÕES -------------------------

void atualizarStatus(const String &novoStatus) {
    currentStatus = novoStatus;
    Serial.println("Status: " + currentStatus);
}

void configurarWiFiDuplo() {
  WiFi.mode(WIFI_AP_STA); // Define o modo AP + STA

  // Inicia o Access Point
  const char* ssidAP = "PetFeeder";
  const char* senhaAP = "senha1234";
  WiFi.softAP(ssidAP, senhaAP);
  Serial.print("Access Point iniciado! IP: ");
  Serial.println(WiFi.softAPIP());

  // Tenta conectar à rede Wi-Fi local (STA)
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando à rede Wi-Fi local...");
  unsigned long tempoInicial = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - tempoInicial < 10000) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nConectado à rede local! IP: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/"); 
    staConectado = true; // <-- ATUALIZADO
  } else {
    Serial.println("\nFalha ao conectar à rede local. AP continua ativo.");
    staConectado = false; // <-- ATUALIZADO
  }
}

void iniciarFirebase() {
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;
  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096, 1024);
  Firebase.begin(&config, &auth);
}

float medirDistanciaCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duracao = pulseIn(ECHO_PIN, HIGH, 30000);
  float distancia = duracao * 0.034 / 2;
  return distancia;
}

void enviarDadosParaFirebase(float distancia, int potValue, int tempo, const String &modo, const String &evento) {
  // Enviar apenas se conectado à rede local (STA) e Firebase estiver pronto
  if (!staConectado || !Firebase.ready()) return;

  Firebase.setFloat(fbdo, "/estado/distancia_cm", distancia);
  Firebase.setInt(fbdo, "/estado/potenciometro", potValue);
  Firebase.setInt(fbdo, "/estado/tempo_aberto_ms", tempo);
  Firebase.setString(fbdo, "/estado/modo", modo);
  Firebase.setString(fbdo, "/estado/evento", evento); // Usa o 'evento' que é o mesmo 'currentStatus'
  Serial.println("Dados enviados para Firebase.");
}

int brilhoInicial = 255;
unsigned long tempoInicioAcionamento = 0;
unsigned long ultimaAtualizacaoFirebase = 0;
const unsigned long intervaloFirebase = 5000;  // Aumentado para 5s para evitar flood

void acionarServoNaoBloqueante(int tempoEmMs, const String &motivo) {
  if (servoAtivo) return; // Não aciona se já estiver ativo

  atualizarStatus(motivo); // <-- ATUALIZADO: Define o status ANTES de abrir
  servo1.write(90);
  analogWrite(LED_PIN, brilhoInicial);  // Começa com LED no máximo
  tempoInicioAcionamento = millis();
  tempoAcionamento = tempoInicioAcionamento + tempoEmMs;
  servoAtivo = true;
}

void atualizarServo() {
  if (servoAtivo) {
    unsigned long agora = millis();

    if (agora >= tempoAcionamento) {
      // Tempo terminou: fechar servo e apagar LED
      servo1.write(0);
      analogWrite(LED_PIN, 0);
      servoAtivo = false;
      atualizarStatus("Porta fechada"); // <-- ATUALIZADO: Status ao fechar
    } else {
      // Atualiza brilho proporcional ao tempo restante
      int tempoTotal = tempoAcionamento - tempoInicioAcionamento;
      int tempoDecorrido = agora - tempoInicioAcionamento;
      int brilhoAtual = map(tempoDecorrido, 0, tempoTotal, brilhoInicial, 0);
      brilhoAtual = constrain(brilhoAtual, 0, 255);
      analogWrite(LED_PIN, brilhoAtual);
    }
  }
}

// ------------------------- WEB SERVER -------------------------

void handleAbrir() {
  if (server.hasArg("tempo")) {
    int tempo = server.arg("tempo").toInt();
    tempo = constrain(tempo, 1000, 10000);
    
    String motivo = "Abertura via Pagina AP";
    acionarServoNaoBloqueante(tempo, motivo); // <-- ATUALIZADO: Passa o motivo
    enviarDadosParaFirebase(medirDistanciaCM(), analogRead(POTENTIOMETER_PIN), tempo, origemPrioritaria, motivo);

    String resposta = "Servo acionado por ";
    resposta.concat(tempo);
    resposta.concat(" ms");
    server.send(200, "text/plain", resposta);
  } else {
    server.send(400, "text/plain", "Parâmetro 'tempo' ausente");
  }
}

// <-- NOVO: Handler para o status
void handleStatus() {
    server.send(200, "text/plain", currentStatus);
}

void handleRoot() {
    IPAddress clientIP = server.client().remoteIP();
    IPAddress apIP = WiFi.softAPIP();

    // Compara os 3 primeiros octetos do IP do cliente com o IP do AP.
    if (clientIP[0] == apIP[0] && clientIP[1] == apIP[1] && clientIP[2] == apIP[2]) {
      // Se o IP do cliente está na mesma sub-rede do AP, sirva a página simples.
      server.send(200, "text/html", SIMPLE_HTML_PAGE);
      Serial.println("Cliente acessou via AP. Servindo HTML Simples.");
    } else {
      // Se não, o cliente veio pela rede local (STA). Sirva o index.html do LittleFS.
      if (!LittleFS.exists("/index.html")) {
          Serial.println("Erro: /index.html nao encontrado no LittleFS.");
          server.send(404, "text/plain", "Arquivo index.html nao encontrado no LittleFS.");
          return;
      }
      File file = LittleFS.open("/index.html", "r");
      if (!file) {
          Serial.println("Falha ao abrir index.html no LittleFS.");
          server.send(500, "text/plain", "Erro ao abrir index.html.");
          return;
      }
      server.streamFile(file, "text/html");
      file.close();
      Serial.println("Cliente acessou via STA. Servindo index.html do LittleFS.");
    }
}

// ------------------------- SETUP & LOOP -------------------------

void setup() {
  Serial.begin(9600);

  if (!LittleFS.begin(true)) { // true formata se não conseguir montar
    Serial.println("Erro critico ao montar LittleFS");
    return;
  }
  Serial.println("LittleFS montado com sucesso.");

  configurarWiFiDuplo();  // Configura AP + STA

  if (staConectado) {
      iniciarFirebase(); // Inicia Firebase SÓ se conectou ao Wi-Fi
  }

  // Configura Handlers do Servidor
  server.on("/", HTTP_GET, handleRoot); // <-- ATUALIZADO: Usa handleRoot
  server.on("/abrir", HTTP_GET, handleAbrir);
  server.on("/status", HTTP_GET, handleStatus); // <-- NOVO: Handler de status

  // Serve arquivos estáticos (CSS, JS) do LittleFS para a versão STA
  // IMPORTANTE: Isso só funciona bem se você não tiver arquivos com os mesmos nomes
  // dos seus handlers (/abrir, /status). É melhor servir arquivos específicos
  // ou usar server.serveStatic("/", LittleFS, "/"); SE handleRoot não cobrir tudo.
  // Por segurança, vamos deixar como está, assumindo que index.html carrega seus assets.
  // Se index.html precisa de outros arquivos (ex: /style.css), adicione handlers
  // ou use server.serveStatic. Como index.html não foi fornecido, mantemos simples.


  server.begin();
  Serial.println("Servidor HTTP iniciado.");

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(POTENTIOMETER_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BOTAO_PIN, INPUT_PULLUP);

  servo1.attach(SERVO_PIN);
  servo1.write(0); // Garante que começa fechado

  atualizarStatus("Sistema pronto!");
}

void loop() {
  server.handleClient();
  atualizarServo();

  // Só processa lógicas de abertura se o servo não estiver ativo
  if (servoAtivo) return;

  float distancia = medirDistanciaCM();
  int potValue = analogRead(POTENTIOMETER_PIN);
  int indicePot = map(potValue, 0, 4095, 0, 18);
  int tempoPotenciometro = 1000 + (indicePot * 500);

  // Lógica de Potenciômetro e Origem (sem alterações drásticas)
  if (indicePot == 0) {
      if (origemPrioritaria != "firebase") {
          origemPrioritaria = "firebase";
          if (staConectado) Firebase.setString(fbdo, "/comando/origem", "firebase");
      }
  } else {
      if (indicePot != ultimoPotIndice) {
          if (origemPrioritaria != "potenciometro") {
              origemPrioritaria = "potenciometro";
              if (staConectado) Firebase.setString(fbdo, "/comando/origem", "potenciometro");
          }
          ultimoPotIndice = indicePot;
          tempoEmMs = tempoPotenciometro;
          if (staConectado) Firebase.setInt(fbdo, "/comando/tempo_remoto_ms", tempoEmMs);
      }
  }

  // Verifica se a prioridade foi alterada via Firebase
  if (staConectado && Firebase.getString(fbdo, "/comando/origem")) {
      String origemRemota = fbdo.stringData();
      if (origemRemota == "firebase" && origemPrioritaria != "firebase") {
          origemPrioritaria = "firebase";
      }
  }

  // Se o Firebase for a origem atual, use o valor remoto
  if (staConectado && origemPrioritaria == "firebase") {
      if (Firebase.getInt(fbdo, "/comando/tempo_remoto_ms")) {
          int tempoRemoto = fbdo.intData();
          if (tempoRemoto >= 1000 && tempoRemoto <= 10000 && tempoRemoto != tempoEmMs) {
              tempoEmMs = tempoRemoto;
              ultimoPotIndice = (tempoEmMs - 1000) / 500;
          }
      }
  }

  // Verifica acionamento remoto via Firebase
  if (staConectado && Firebase.getBool(fbdo, "/comando/abrir") && fbdo.boolData()) {
      String motivo = "Abertura remota via Firebase";
      acionarServoNaoBloqueante(tempoEmMs, motivo);
      enviarDadosParaFirebase(distancia, potValue, tempoEmMs, origemPrioritaria, motivo);
      Firebase.setBool(fbdo, "/comando/abrir", false);
      delay(100); // Pequeno delay após comando Firebase
      return; // Retorna para não processar outros no mesmo ciclo
  }

  // Verifica botão físico
  bool estadoBotao = digitalRead(BOTAO_PIN);
  bool cliqueDetectado = (estadoAnteriorBotao == HIGH && estadoBotao == LOW);
  estadoAnteriorBotao = estadoBotao;

  if (cliqueDetectado) {
      String motivo = "Abertura manual via botao";
      acionarServoNaoBloqueante(tempoEmMs, motivo);
      enviarDadosParaFirebase(distancia, potValue, tempoEmMs, origemPrioritaria, motivo);
  }
  // Verifica sensor ultrassônico
  else if (distancia > 0 && distancia < 20) {
      String motivo = "Abertura automatica por sensor";
      acionarServoNaoBloqueante(tempoEmMs, motivo);
      enviarDadosParaFirebase(distancia, potValue, tempoEmMs, origemPrioritaria, motivo);
  }
  // Envio periódico de status (apenas se nada aconteceu)
  else {
      if (millis() - ultimaAtualizacaoFirebase >= intervaloFirebase) {
          ultimaAtualizacaoFirebase = millis();
          if (staConectado && Firebase.ready()) {
              // Só envia se o status não for o padrão "porta fechada/pronto"
              // ou se quiser monitorar constantemente. Aqui enviamos sempre.
              enviarDadosParaFirebase(distancia, potValue, tempoEmMs, origemPrioritaria, currentStatus);
          }
      }
  }

  delay(100); // Delay geral do loop
}