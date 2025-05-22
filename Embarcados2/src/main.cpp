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
bool modoAP = false;
String origemPrioritaria = "potenciometro";  // padrão inicial
int tempoEmMs = 1000;
int ultimoPotIndice = -1;



// ------------------------- HTML SIMPLES (modo AP) -------------------------
const char SIMPLE_HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head><meta charset="UTF-8" /><meta name="viewport" content="width=device-width, initial-scale=1.0"/>
<title>PetFeeder Local</title>
<style>body { font-family: Arial; background: #f0f0f0; padding: 20px; text-align: center; } button, input { padding: 10px 20px; margin: 10px; font-size: 16px; } #status { margin-top: 20px; }</style>
</head>
<body>
<h1>Controle PetFeeder</h1>
<input type="number" id="tempo" placeholder="Tempo em ms" min="1000" max="10000"><br>
<button onclick="abrir()">🔓 Abrir Porta</button>
<div id="status">Status: aguardando ação...</div>
<script>
async function abrir() {
  const tempo = document.getElementById('tempo').value || 3000;
  const res = await fetch("/abrir?tempo=" + tempo);
  const txt = await res.text();
  document.getElementById("status").innerText = "Status: " + txt;
}
</script>
</body>
</html>
)rawliteral";

// ------------------------- HTML COMPLEXO (Wi-Fi local) -------------------------

// ------------------------- FUNÇÕES -------------------------

void configurarWiFiDuplo() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando à rede Wi-Fi local...");
  unsigned long tempoInicial = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - tempoInicial < 10000) {
    Serial.print(".");
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nConectado! IP local: ");
    Serial.println(WiFi.localIP());
    modoAP = false;
  } else {
    Serial.println("\nFalha ao conectar. Iniciando AP...");
    const char* ssidAP = "PetFeeder";
    const char* senhaAP = "senha1234";
    WiFi.softAP(ssidAP, senhaAP);
    Serial.print("Access Point iniciado! IP: ");
    Serial.println(WiFi.softAPIP());
    modoAP = true;
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
  if (modoAP || !Firebase.ready()) return;

  Firebase.setFloat(fbdo, "/estado/distancia_cm", distancia);
  Firebase.setInt(fbdo, "/estado/potenciometro", potValue);
  Firebase.setInt(fbdo, "/estado/tempo_aberto_ms", tempo);
  Firebase.setString(fbdo, "/estado/modo", modo);
  Firebase.setString(fbdo, "/estado/evento", evento);
}



int brilhoInicial = 255;
unsigned long tempoInicioAcionamento = 0;

// Controle de frequência do Firebase
unsigned long ultimaAtualizacaoFirebase = 0;
const unsigned long intervaloFirebase = 1000;  // 1 segundo


void acionarServoNaoBloqueante(int tempoEmMs) {
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
    } else {
      // Atualiza brilho proporcional ao tempo restante
      int tempoTotal = tempoAcionamento - tempoInicioAcionamento;
      int tempoDecorrido = agora - tempoInicioAcionamento;

      // Cálculo do brilho linear decrescente
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
    acionarServoNaoBloqueante(tempo);

    String resposta = "Servo acionado por ";
    resposta.concat(tempo);
    resposta.concat(" ms");

    server.send(200, "text/plain", resposta);
  } else {
    server.send(400, "text/plain", "Parâmetro 'tempo' ausente");
  }
}

// ------------------------- SETUP & LOOP -------------------------

void setup() {
  Serial.begin(9600);

  // Inicializa o sistema de arquivos
  if (!LittleFS.begin()) {
    Serial.println("Erro ao montar LittleFS");
    return;
  }

  server.serveStatic("/", LittleFS, "/");
  server.on("/abrir", handleAbrir);

  configurarWiFiDuplo();  // chamada única
  if (!modoAP) iniciarFirebase();

  server.begin();  // servidor só depois da conexão Wi-Fi


  // Inicializa pinos
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(POTENTIOMETER_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BOTAO_PIN, INPUT_PULLUP);

  // Inicializa componentes
  servo1.attach(SERVO_PIN);

  Serial.println("Sistema pronto!");
}



void loop() {
  server.handleClient();
  atualizarServo();

  if (servoAtivo) return;

  float distancia = medirDistanciaCM();
  int potValue = analogRead(POTENTIOMETER_PIN);
  int indicePot = map(potValue, 0, 4095, 0, 18);
  int tempoPotenciometro = 1000 + (indicePot * 500);

  // Se o potenciômetro estiver em 0, entrega o controle ao Firebase
  if (indicePot == 0) {
    if (origemPrioritaria != "firebase") {
      origemPrioritaria = "firebase";
      if (!modoAP) {
        Firebase.setString(fbdo, "/comando/origem", "firebase");
      }
    }
  } else {
    // Se o potenciômetro foi girado e o índice é diferente de 0, volta ao modo local
    if (indicePot != ultimoPotIndice) {
      if (origemPrioritaria != "potenciometro") {
        origemPrioritaria = "potenciometro";
        if (!modoAP) {
          Firebase.setString(fbdo, "/comando/origem", "potenciometro");
        }
      }

      ultimoPotIndice = indicePot;
      tempoEmMs = tempoPotenciometro;

      if (!modoAP) {
        Firebase.setInt(fbdo, "/comando/tempo_remoto_ms", tempoEmMs);
      }
    }
  }

  // Verifica se a prioridade foi alterada via Firebase
  if (!modoAP && Firebase.getString(fbdo, "/comando/origem")) {
    String origemRemota = fbdo.stringData();
    if (origemRemota == "firebase" && origemPrioritaria != "firebase") {
      origemPrioritaria = "firebase";
    }
  }

  // Se o Firebase for a origem atual, use o valor remoto
  if (!modoAP && origemPrioritaria == "firebase") {
    if (Firebase.getInt(fbdo, "/comando/tempo_remoto_ms")) {
      int tempoRemoto = fbdo.intData();
      if (tempoRemoto >= 1000 && tempoRemoto <= 10000 && tempoRemoto != tempoEmMs) {
        tempoEmMs = tempoRemoto;
        ultimoPotIndice = (tempoEmMs - 1000) / 500;
      }
    }
  }

  // Verifica acionamento remoto via Firebase
  if (!modoAP && Firebase.getBool(fbdo, "/comando/abrir") && fbdo.boolData()) {
    acionarServoNaoBloqueante(tempoEmMs);
    enviarDadosParaFirebase(distancia, potValue, tempoEmMs, origemPrioritaria, "Abertura remota via Firebase");
    Firebase.setBool(fbdo, "/comando/abrir", false);
    delay(100);
    return;
  }

  // Verifica botão físico
  bool estadoBotao = digitalRead(BOTAO_PIN);
  bool cliqueDetectado = (estadoAnteriorBotao == HIGH && estadoBotao == LOW);
  estadoAnteriorBotao = estadoBotao;

  if (cliqueDetectado) {
    acionarServoNaoBloqueante(tempoEmMs);
    enviarDadosParaFirebase(distancia, potValue, tempoEmMs, origemPrioritaria, "Abertura manual via botão");
  }
  else if (distancia > 0 && distancia < 20) {
    acionarServoNaoBloqueante(tempoEmMs);
    enviarDadosParaFirebase(distancia, potValue, tempoEmMs, origemPrioritaria, "Abertura automática por sensor");
  }
  else {
    if (millis() - ultimaAtualizacaoFirebase >= intervaloFirebase) {
      ultimaAtualizacaoFirebase = millis();

      if (!modoAP && Firebase.ready()) {
        enviarDadosParaFirebase(distancia, potValue, tempoEmMs, origemPrioritaria, "Monitoramento contínuo");
      }
    }
  }

  delay(100);
}