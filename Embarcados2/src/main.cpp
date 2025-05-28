#include "Arduino.h"
#include <ESP32Servo.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <WebServer.h>
#include "FS.h"
#include "LittleFS.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

//DEFINIÇÕES GERAIS
#define WIFI_SSID "VIAWEB_FIBRA_PEDRINHO_33382360"
#define WIFI_PASSWORD "Apolonobrega25"
#define DATABASE_URL "https://petfeed-se-default-rtdb.firebaseio.com/"
#define DATABASE_SECRET "N85woKV3Tfz1UIWfk0RPAUsLWUcT2FPmuAGErA1h"

//DEFINIÇÕES GEMINI API
#define GEMINI_API_KEY "AIzaSyCRPi54oj5mJjGlEU0sjA6f7KQgpkVt1to"
#define GEMINI_API_URL "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=" GEMINI_API_KEY

//DEFINIÇÕES FIREBASE
#define FB_REQUESTS_PATH "/routineRequests"
#define FB_RESPONSES_PATH "/routineResponses"

//PIN
#define TRIG_PIN 23
#define ECHO_PIN 22
#define SERVO_PIN 27
#define POTENTIOMETER_PIN 32
#define LED_PIN 18
#define BOTAO_PIN 33

//CONFIG
#define MIN_SERVO_TIME_MS 1000
#define MAX_SERVO_TIME_MS 10000
#define DEFAULT_SENSOR_THRESHOLD_CM 20

//CERTIFICADO CA GOOGLE
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFYjCCBEqgAwIBAgIQd70NbNs2+RneGU5/14876jANBgkqhkiG9w0BAQsFADBC\n" \
"MQswCQYDVQQGEwJVUzEeMBwGA1UEChMVR29vZ2xlIFRydXN0IFNlcnZpY2Vz\n" \
"MRMxETAPBgNVBAMTCEdUUyBSb290IFIxMB4XDTIwMDYxOTE3MDIzNVoXDTI3\n" \
"MDYxOTE3MDIzNVowQjELMAkGA1UEBhMCVVMxHjAcBgNVBAoTFUdvb2dsZSBU\n" \
"cnVzdCBTZXJ2aWNlczETMBEGA1UEAxMKR1RTIENBIDFDMzCCASIwDQYJKoZI\n" \
"hvcNAQEBBQADggEPADCCAQoCggEBAJm36F52/24V5P2MS/m9sP2h6L5Q5ih4\n" \
"dcfiY10jkU0ll5bNEkfes9iLS7do3q6YnO93+i6a7aW2NCi0l4EPdYBGhEGu\n" \
"j2t3i12+iNsRk65iQx6j2Cnh2V7N1XjJ7y+5+X+uL0T7Ea8KMeLh6mAn578S\n" \
"c4Hx4c25sWNzKD/8KV17kK8y+3k17W8G+2+sTwmhs6zTb+y2aX3b67/W9jqw\n" \
"0uMEd39b9OmeyvK8Xy+h+I443qN+rWwG8o3pLiWjJ0lMmx0dZrztL4kW2zJL\n" \
"zO6C/646J+s+h10j4gnhzP6vY1Y6Ie7c8g6Tm+f701/9f0DA+cECAwEAAaOC\n" \
"AQcwggEDMA4GA1UdDwEB/wQEAwIBhjAPBgNVHRMBAf8EBTADAQH/MB0GA1Ud\n" \
"DgQWBBSf7C0kDUegx5Amp+j6M72c4v88SjAfBgNVHSMEGDAWgBR5oHIeoL3x\n" \
"+VAX1zG6L5j45xS5SzB2BggrBgEFBQcBAQRqMGgwJgYIKwYBBQUHMAGGGmh0\n" \
"dHA6Ly9vY3NwLnBraS5nb29nL2d0czFyPzA+BggrBgEFBQcwAoYyaHR0cDov\n" \
"L3BraS5nb29nL3JlcG8vY2VydHMvZ3Rzcm9vdC1yMS5kZXIwEwYDVR0gBAww\n" \
"CjAIBgZngQwBAgIwMAYDVR0fBCkwJzAloCOgIYYfaHR0cDovL2NybC5wa2ku\n" \
"Z29vZy9ndHNyMXIuY3JsMA0GCSqGSIb3DQEBCwUAA4ICAQA5Krb24933QE7p\n" \
"b8J0qs74nKWXWBYUiy5Uc72QDB/wOkN1g9uZZzHMA0o3k27n1o8Z3A0Y52m0\n" \
"9n9Gyy33LRPm5xB3T2+85b1Nh6XpGg2WW0VbF2yS6+JGrQ4hWpKiLWjJDUAp\n" \
"jGZgHqL9+8ks0O636i3Fn7kdxh+Gg7GcVqNn/MEnkKqgP2AWW0j010IfsL00\n" \
"Xm0urC9mg5ygrYCMUqPzzsTh12b/cQYxfgNpeQk+D876t40iaIT1C8yCowNU\n" \
"Y2z7L9R/4y6L1IBzM6t84h74+8g7L6cCUeYkbR34Qz68qZ6Vkv8fT+1z31j/\n" \
"6DQYcG895a2LsLYAn09g1y2P409PPLdLe8NfL+3hXU+X8mY8xEkPnkf2R4aK\n" \
"AP9Sj663jLqG9s3v+H0ac3r0x2fWqG4JffvsMr/R8QjMM1K2VKXxbFl729Mv\n" \
"7jB4k0xqCaBTfsj3A4c6k8RgnWb7ZNb1hEo3s61+BfP3MY1NVCXiCg8gohNt\n" \
"c4wh4fDrsb6i8amBAT2yYj++XjA5fPOiLq2K0q47v64wVFqCj4xMy3g7P41o\n" \
"F8lYQ3wG0h60W6YejS8y0b+kX70gN8S3N5vDQXj7V5wgyLKN4XmBYG055bi6\n" \
"/A4yXw==\n" \
"-----END CERTIFICATE-----\n";

//OBJETOS GLOBAIS
Servo servo1;
FirebaseData fbdo;
FirebaseData stream;
FirebaseAuth auth;
FirebaseConfig config;
WebServer server(80);

//VARIÁVEIS
bool estadoAnteriorBotao = HIGH;
bool servoAtivo = false;
unsigned long tempoAcionamento = 0;
String origemPrioritaria = "potenciometro";
int tempoEmMs = 5000;
int ultimoPotIndice = -1;
String currentStatus = "Sistema pronto!";
bool staConectado = false;
int brilhoInicial = 255;
unsigned long tempoInicioAcionamento = 0;
float distanciaAtual = 999.0;
int potValueAtual = 0;
int sensorThresholdCM = DEFAULT_SENSOR_THRESHOLD_CM;

//VARIÁVEIS(millis)
unsigned long lastSensorRead = 0;
unsigned long lastFirebaseCheck = 0;
unsigned long lastFirebaseUpdate = 0;
unsigned long lastInputCheck = 0;
unsigned long lastConfigCheck = 0;

const long sensorInterval = 50;
const long firebaseCheckInterval = 1000;
const long firebaseUpdateInterval = 10000;
const long inputCheckInterval = 100;
const long configCheckInterval = 15000;

const char SIMPLE_HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>PetFeeder Local</title>
    <style>
        :root {
            --bg-primary: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            --bg-secondary: rgba(28, 28, 30, 0.6);
            --bg-tertiary: rgba(44, 44, 46, 0.7);
            --text-primary: #ffffff;
            --text-secondary: #b0b0b0;
            --border-color: rgba(56, 56, 58, 0.5);
            --accent-primary: #ffffff;
            --status-green: #34c759;
            --status-red: #ff3b30;
            --status-orange: #ff9500;
            --status-purple: #5856d6;
            --button-bg: rgba(255, 255, 255, 0.25);
            --button-border: rgba(255, 255, 255, 0.3);
            --button-hover: rgba(255, 255, 255, 0.35);
            --input-bg: rgba(44, 44, 46, 0.8);
        }
        * { margin: 0; padding: 0; box-sizing: border-box; -webkit-tap-highlight-color: transparent; }
        body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; background: var(--bg-primary); min-height: 100vh; overflow-x: hidden; color: var(--text-primary); }
        .screen { display: none; flex-direction: column; align-items: center; padding: 30px 20px 95px 20px; /* Increased bottom padding */ min-height: 100vh; animation: fadeIn 0.5s forwards; }
        .screen.active { display: flex; }
        @keyframes fadeIn { from { opacity: 0; } to { opacity: 1; } }
        .screen-content { width: 100%; max-width: 380px; display: none;}
        .screen-content.active { display: block; } 

        .welcome-screen { justify-content: center; text-align: center; }
        .app-icon { font-size: 80px; margin-bottom: 20px; }
        .welcome-title { font-size: 32px; font-weight: 700; margin-bottom: 10px; }
        .welcome-subtitle { font-size: 18px; opacity: 0.8; margin-bottom: 40px; }
        .input-group { margin-bottom: 20px; text-align: left; }
        .input-label { font-size: 13px; font-weight: 600; color: var(--text-secondary); margin-bottom: 8px; text-transform: uppercase; letter-spacing: 0.5px; padding-left: 5px; display: block; }
        .pet-input, .datetime-input { width: 100%; padding: 16px; border: 1px solid var(--button-border); border-radius: 12px; background: var(--input-bg); color: var(--text-primary); font-size: 17px; backdrop-filter: blur(10px); }
        .datetime-input { padding: 12px; font-size: 15px; margin-bottom: 10px; color-scheme: dark; } /* Estilo para datetime */
        .pet-selector { display: grid; grid-template-columns: repeat(2, 1fr); gap: 15px; margin-bottom: 30px; }
        .pet-option { background: var(--input-bg); border: 2px solid transparent; border-radius: 16px; padding: 20px; text-align: center; cursor: pointer; transition: all 0.3s ease; backdrop-filter: blur(10px); }
        .pet-option.selected { border-color: var(--accent-primary); box-shadow: 0 0 8px var(--accent-primary); }
        .pet-option-emoji { font-size: 36px; margin-bottom: 10px; }
        .pet-option-name { font-size: 14px; font-weight: 500; }
        .btn { background: var(--button-bg); backdrop-filter: blur(10px); border: 1px solid var(--button-border); color: white; padding: 16px 40px; border-radius: 25px; font-size: 16px; font-weight: 600; cursor: pointer; transition: all 0.3s ease; width: 100%; }
        .btn:hover { background: var(--button-hover); }
        .btn:disabled { background: rgba(88, 88, 90, 0.5); color: #8e8e93; cursor: not-allowed; }

        /* Main Screen */
        .card { background: var(--bg-secondary); border-radius: 16px; padding: 20px; margin-bottom: 20px; backdrop-filter: blur(15px); border: 1px solid var(--border-color); }
        .header { text-align: center; margin-bottom: 30px; }
        .pet-avatar { font-size: 50px; margin-bottom: 10px; }
        .pet-name { font-size: 24px; font-weight: 700; }
        .card h3 { font-size: 18px; font-weight: 600; margin-bottom: 15px; padding-bottom: 10px; border-bottom: 1px solid var(--border-color); }
        #status, #routineStatus { margin-top: 15px; font-size: 14px; color: var(--text-secondary); background: var(--bg-tertiary); padding: 10px 15px; border-radius: 8px; text-align: center; min-height: 40px; }
        .interval-group { display: flex; gap: 10px; align-items: center; }
        .interval-group input { flex-grow: 1; }
        .interval-group span { font-size: 14px; }
        .btn.action { background-color: var(--status-green); border-color: var(--status-green); }
        .btn.action:hover { background-color: #2ca048; }
        .btn.routine { background-color: var(--status-purple); border-color: var(--status-purple); margin-top: 10px; }
        .btn.routine:hover { background-color: #4644a8; }
        .btn.cancel { background-color: var(--status-red); border-color: var(--status-red); margin-top: 10px; }
        .btn.cancel:hover { background-color: #c70015; }
        .btn.reset { background-color: var(--status-orange); border-color: var(--status-orange); margin-top: 10px; } /* Reset Button Style */
        .btn.reset:hover { background-color: #e68a00; }
        .settings-p { color: var(--text-secondary); font-size: 13px; text-align: center; margin-top: 10px; line-height: 1.4; }


        /* Nav Bar */
        .nav-bar { position: fixed; bottom: 15px; left: 50%; transform: translateX(-50%); width: auto; background: var(--bg-tertiary); backdrop-filter: blur(20px); border-radius: 28px; display: flex; z-index: 100; box-shadow: 0 4px 14px rgba(0,0,0,0.25); }
        .nav-item { color: var(--text-secondary); cursor: pointer; padding: 12px 20px; font-size: 24px; transition: color 0.3s ease; }
        .nav-item.active { color: var(--accent-primary); }
    </style>
</head>
<body>

    <div class="screen welcome-screen active" id="welcomeScreen">
        <div class="screen-content active"> <div class="app-icon">🍽️</div>
            <h1 class="welcome-title">PetFeeder</h1>
            <p class="welcome-subtitle">Controle Local Inteligente</p>
            <div class="input-group">
                <label class="input-label">Nome do Pet</label>
                <input type="text" class="pet-input" id="petNameInput" placeholder="Ex: Buddy, Luna...">
            </div>
            <div class="input-group">
                <label class="input-label">Tipo do Pet</label>
                <div class="pet-selector">
                    <div class="pet-option" data-pet="🐕">
                        <div class="pet-option-emoji">🐕</div>
                        <div class="pet-option-name">Cachorro</div>
                    </div>
                    <div class="pet-option" data-pet="🐱">
                        <div class="pet-option-emoji">🐱</div>
                        <div class="pet-option-name">Gato</div>
                    </div>
                </div>
            </div>
            <button class="btn" id="continueBtn" disabled>Continuar</button>
        </div>
    </div>

    <div class="screen" id="mainScreen">
        <div class="screen-content active" id="controlSection"> <div class="header">
                <div class="pet-avatar" id="petAvatar"></div>
                <h2 class="pet-name" id="petNameDisplay"></h2>
            </div>
            <div class="card">
                <h3>Controle Manual</h3>
                <input type="number" id="manualTime" class="pet-input" placeholder="Tempo em ms (1000-10000)" value="3500" min="1000" max="10000">
                <button class="btn action" onclick="manualFeed()" style="margin-top: 15px;">🔓 Abrir Porta Agora</button>
                <div id="status">Status: Conectando...</div>
            </div>
        </div>

        <div class="screen-content" id="routineSection">
             <div class="header">
                <div class="pet-avatar" id="petAvatarRoutine"></div>
                <h2 class="pet-name" id="petNameDisplayRoutine"></h2>
            </div>
            <div class="card">
                <h3>Configurar Rotina</h3>
                <div class="input-group">
                    <label class="input-label">Início da Rotina</label>
                    <input type="datetime-local" class="datetime-input" id="startTime">
                </div>
                <div class="input-group">
                    <label class="input-label">Fim da Rotina (Opcional)</label>
                    <input type="datetime-local" class="datetime-input" id="endTime">
                </div>
                 <div class="input-group">
                    <label class="input-label">Intervalo</label>
                    <div class="interval-group">
                       <input type="number" class="pet-input" id="intervalHours" placeholder="Horas" min="0" max="23">
                       <span>H</span>
                       <input type="number" class="pet-input" id="intervalMinutes" placeholder="Min" min="0" max="59">
                       <span>Min</span>
                    </div>
                 </div>
                 <div class="input-group">
                    <label class="input-label">Tempo Abertura (ms)</label>
                    <input type="number" id="routineTime" class="pet-input" value="3500" min="1000" max="10000">
                </div>
                <button class="btn routine" onclick="saveRoutine()">💾 Salvar e Ativar Rotina</button>
                <button class="btn cancel" onclick="cancelRoutine()" style="display: none;" id="cancelRoutineBtn">🗑️ Cancelar Rotina</button>
                <div id="routineStatus">Status Rotina: Inativa</div>
            </div>
        </div>

        <div class="screen-content" id="settingsSection"> <div class="header">
                <div class="pet-avatar" id="petAvatarSettings"></div>
                <h2 class="pet-name" id="petNameDisplaySettings"></h2>
            </div>
            <div class="card">
                <h3>Configurações Gerais</h3>
                 <button class="btn reset" onclick="resetApp()">🔄 Reiniciar App</button>
                 <p class="settings-p">Isso limpará todos os dados salvos (nome do pet e rotina) e voltará para a tela inicial.</p>
            </div>
        </div>

        <div class="nav-bar" id="navBar">
            <div class="nav-item active" id="navControl" onclick="showSection('controlSection')">🕹️</div>
            <div class="nav-item" id="navRoutine" onclick="showSection('routineSection')">⏰</div>
            <div class="nav-item" id="navSettings" onclick="showSection('settingsSection')">⚙️</div> </div>
    </div>

    <script>
        let petName = '';
        let petEmoji = '🐕';
        let routineIntervalId = null;
        let routineData = {};

        const welcomeScreen = document.getElementById('welcomeScreen');
        const mainScreen = document.getElementById('mainScreen');
        const navBar = document.getElementById('navBar');
        const petNameInput = document.getElementById('petNameInput');
        const continueBtn = document.getElementById('continueBtn');
        const petOptions = document.querySelectorAll('.pet-option');
        const statusDiv = document.getElementById('status');
        const routineStatusDiv = document.getElementById('routineStatus');
        const cancelRoutineBtn = document.getElementById('cancelRoutineBtn');

        const allSections = document.querySelectorAll('.screen-content');
        const allNavItems = document.querySelectorAll('.nav-item');
        const allPetAvatars = document.querySelectorAll('.pet-avatar');
        const allPetNames = document.querySelectorAll('.pet-name');

        function showSection(sectionId) {
            allSections.forEach(sec => sec.classList.remove('active'));
            allNavItems.forEach(item => item.classList.remove('active'));

            document.getElementById(sectionId).classList.add('active');
            document.getElementById('nav' + sectionId.replace('Section', '')).classList.add('active');
            updateHeaders();
        }

        function updateHeaders() {
            allPetAvatars.forEach(avatar => avatar.textContent = petEmoji);
            allPetNames.forEach(name => name.textContent = petName);
        }

        function checkSetupComplete() {
            continueBtn.disabled = !(petNameInput.value.trim().length >= 2 && petEmoji);
        }

        petNameInput.addEventListener('input', checkSetupComplete);
        petOptions.forEach(opt => {
            opt.addEventListener('click', () => {
                petOptions.forEach(o => o.classList.remove('selected'));
                opt.classList.add('selected');
                petEmoji = opt.dataset.pet;
                checkSetupComplete();
            });
        });

        continueBtn.addEventListener('click', () => {
            petName = petNameInput.value.trim();
            localStorage.setItem('petName', petName);
            localStorage.setItem('petEmoji', petEmoji);
            showMainScreen();
        });

        function showWelcomeScreen() {
            welcomeScreen.classList.add('active');
            mainScreen.classList.remove('active');
            navBar.style.display = 'none';
            document.querySelector('.pet-option[data-pet="🐕"]').classList.add('selected');
            petEmoji = '🐕';
            checkSetupComplete();
        }

        function showMainScreen() {
            updateHeaders();
            welcomeScreen.classList.remove('active');
            mainScreen.classList.add('active');
            navBar.style.display = 'flex';
            showSection('controlSection');
            setInterval(fetchStatus, 5000);
            loadRoutine();
        }

        async function fetchStatus() {
            try {
                const res = await fetch("/status");
                if (res.ok) {
                    const txt = await res.text();
                    statusDiv.innerText = "Status: " + txt;
                } else {
                     statusDiv.innerText = "Status: Erro ao conectar.";
                }
            } catch (error) {
                statusDiv.innerText = "Status: Desconectado.";
                console.error("Erro no fetch /status:", error);
            }
        }

        async function manualFeed() {
            const time = document.getElementById('manualTime').value || 3500;
            statusDiv.innerText = "Status: Enviando...";
            try {
                const res = await fetch("/abrir?tempo=" + time);
                const txt = await res.text();
                statusDiv.innerText = "Status: " + txt;
            } catch (error) {
                statusDiv.innerText = "Status: Erro ao enviar!";
                console.error("Erro no fetch /abrir:", error);
            }
        }

         async function routineFeed(time) {
            routineStatusDiv.innerText = `Status Rotina: Alimentando agora...`;
            try {
                await fetch("/abrir?tempo=" + time);
            } catch (error) {
                 console.error("Erro no fetch /abrir (rotina):", error);
                 routineStatusDiv.innerText = "Status Rotina: Erro ao alimentar!";
            }
        }

        function saveRoutine() {
            const startTime = document.getElementById('startTime').value;
            const endTime = document.getElementById('endTime').value;
            const hours = parseInt(document.getElementById('intervalHours').value) || 0;
            const minutes = parseInt(document.getElementById('intervalMinutes').value) || 0;
            const openTime = document.getElementById('routineTime').value;

            if (!startTime) { alert("Por favor, defina a hora de início."); return; }
            if (hours === 0 && minutes === 0) { alert("O intervalo não pode ser zero."); return; }
            if (minutes < 0 || minutes > 59 || hours < 0 || hours > 23) { alert("Intervalo inválido."); return; }

            const intervalMs = (hours * 3600 + minutes * 60) * 1000;
            let startTimestamp = new Date(startTime).getTime();

            routineData = {
                startTime: startTimestamp,
                endTime: endTime ? new Date(endTime).getTime() : null,
                intervalMs: intervalMs,
                hours: hours,
                minutes: minutes,
                openTime: openTime,
                nextFeedTime: startTimestamp
            };
            
            const now = Date.now();
            while (routineData.nextFeedTime < now && routineData.nextFeedTime < (routineData.endTime || Infinity)) {
                 routineData.nextFeedTime += routineData.intervalMs;
            }

            localStorage.setItem('routineData', JSON.stringify(routineData));
            startRoutineCheck();
            cancelRoutineBtn.style.display = 'inline-block';
            alert("Rotina salva e ativada!");
        }

        function loadRoutine() {
            const stored = localStorage.getItem('routineData');
            if (stored) {
                routineData = JSON.parse(stored);
                try {
                    document.getElementById('startTime').value = new Date(routineData.startTime).toISOString().slice(0, 16);
                    if(routineData.endTime) document.getElementById('endTime').value = new Date(routineData.endTime).toISOString().slice(0, 16);
                } catch(e) { console.error("Error setting date values:", e); }
                document.getElementById('intervalHours').value = routineData.hours;
                document.getElementById('intervalMinutes').value = routineData.minutes;
                document.getElementById('routineTime').value = routineData.openTime;

                startRoutineCheck();
                cancelRoutineBtn.style.display = 'inline-block';
            } else {
                 routineStatusDiv.innerText = "Status Rotina: Inativa";
                 cancelRoutineBtn.style.display = 'none';
            }
        }

        function cancelRoutine() {
            if (confirm("Tem certeza que deseja cancelar a rotina?")) {
                clearInterval(routineIntervalId);
                routineIntervalId = null;
                localStorage.removeItem('routineData');
                routineData = {};
                routineStatusDiv.innerText = "Status Rotina: Inativa";
                cancelRoutineBtn.style.display = 'none';
                document.getElementById('startTime').value = '';
                document.getElementById('endTime').value = '';
                document.getElementById('intervalHours').value = '';
                document.getElementById('intervalMinutes').value = '';
                document.getElementById('routineTime').value = '3500';
            }
        }

       function startRoutineCheck() {
            if (routineIntervalId) clearInterval(routineIntervalId);
            if (!routineData.startTime) return;

            const check = () => {
                const now = Date.now();

                if (now < routineData.startTime) {
                    routineStatusDiv.innerText = `Status Rotina: Aguardando início em ${new Date(routineData.startTime).toLocaleString('pt-BR')}.`;
                    return;
                }
                if (routineData.endTime && now > routineData.endTime) {
                    routineStatusDiv.innerText = "Status Rotina: Período encerrado.";
                    cancelRoutine();
                    return;
                }

                while (routineData.nextFeedTime < now) {
                    routineData.nextFeedTime += routineData.intervalMs;
                }
                
                if (routineData.endTime && routineData.nextFeedTime > routineData.endTime) {
                    routineStatusDiv.innerText = "Status Rotina: Período encerrado.";
                    cancelRoutine();
                    return;
                }

                if (now >= routineData.nextFeedTime && now < (routineData.nextFeedTime + 30000)) { 
                    routineFeed(routineData.openTime);
                    routineData.nextFeedTime += routineData.intervalMs; 
                    localStorage.setItem('routineData', JSON.stringify(routineData)); 
                }

                const nextFeedDate = new Date(routineData.nextFeedTime);
                routineStatusDiv.innerText = `Status Rotina: Ativa. Próxima: ${nextFeedDate.toLocaleString('pt-BR')}.`;
            };

            check(); 
            routineIntervalId = setInterval(check, 30000);
        }

        function resetApp() {
            if (confirm("Tem certeza que deseja reiniciar o app? Todos os dados locais (nome e rotina) serão perdidos.")) {
                localStorage.clear();
                location.reload();
            }
        }

        window.onload = () => {
            const storedName = localStorage.getItem('petName');
            const storedEmoji = localStorage.getItem('petEmoji');

            if (storedName && storedEmoji) {
                petName = storedName;
                petEmoji = storedEmoji;
                showMainScreen();
            } else {
                showWelcomeScreen();
            }
        };
    </script>
</body>
</html>
)rawliteral";

void atualizarStatus(const String &novoStatus) {
    currentStatus = novoStatus;
    Serial.print("Status: ");
    Serial.println(currentStatus);
}

void configurarWiFiDuplo() {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("PetFeeder", "senha1234");
    Serial.print("Access Point iniciado! IP: ");
    Serial.println(WiFi.softAPIP());

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
        staConectado = true;
    } else {
        Serial.println("\nFalha ao conectar à rede local. AP continua ativo.");
        staConectado = false;
    }
}

void carregarConfiguracoesIniciaisFirebase() {
    if (!staConectado || !Firebase.ready()) return;

    FirebaseData fbdoConfig;

    if (Firebase.getInt(fbdoConfig, "/comando/distancia_threshold_cm")) {
        if (fbdoConfig.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
            int threshold = fbdoConfig.intData();
            if (threshold > 0 && threshold <= 100) {
                sensorThresholdCM = threshold;
                Serial.print("Threshold do sensor carregado do Firebase: ");
                Serial.println(sensorThresholdCM);
            } else {
                 Firebase.setInt(fbdo, "/comando/distancia_threshold_cm", DEFAULT_SENSOR_THRESHOLD_CM);
                 sensorThresholdCM = DEFAULT_SENSOR_THRESHOLD_CM;
                 Serial.println("Threshold do Firebase inválido, resetado para padrão.");
            }
        } else {
            sensorThresholdCM = DEFAULT_SENSOR_THRESHOLD_CM;
            Firebase.setInt(fbdo, "/comando/distancia_threshold_cm", sensorThresholdCM);
            Serial.println("Tipo de dado do threshold inválido, resetado para padrão.");
        }
    } else {
        Serial.println("Nó /comando/distancia_threshold_cm não encontrado. Usando e definindo padrão.");
        sensorThresholdCM = DEFAULT_SENSOR_THRESHOLD_CM;
        Firebase.setInt(fbdo, "/comando/distancia_threshold_cm", sensorThresholdCM);
    }
}

String getGeminiSuggestion(const String& prompt) {
    if (WiFi.status() != WL_CONNECTED) {
        return "Erro: Sem conexão Wi-Fi para consultar a API.";
    }

    WiFiClientSecure client;
    client.setCACert(root_ca);
    HTTPClient http;

    String fullUrl = String(GEMINI_API_URL);
    Serial.print("Chamando Gemini API: ");
    Serial.println(fullUrl);

    if (http.begin(client, fullUrl)) {
        http.addHeader("Content-Type", "application/json");

        JsonDocument doc;
        doc["contents"][0]["parts"][0]["text"] = prompt;

        String requestBody;
        serializeJson(doc, requestBody);
        Serial.print("Request Body: ");
        Serial.println(requestBody);

        int httpCode = http.POST(requestBody);

        if (httpCode > 0) {
            Serial.printf("[HTTP] POST... code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
                String payload = http.getString();
                Serial.print("Payload: ");
                Serial.println(payload);

                JsonDocument responseDoc;
                DeserializationError error = deserializeJson(responseDoc, payload);

                if (error) {
                    Serial.print("deserializeJson() falhou: ");
                    Serial.println(error.f_str());
                    http.end();
                    return "Erro: Falha ao processar a resposta da API.";
                }

                const char* suggestionText = responseDoc["candidates"][0]["content"]["parts"][0]["text"];
                if (suggestionText) {
                    http.end();
                    return String(suggestionText);
                } else {
                    http.end();
                    return "Erro: Não foi possível extrair a sugestão da resposta.";
                }

            } else {
                String payload = http.getString();
                Serial.print("Erro HTTP, Payload: ");
                Serial.println(payload);
                http.end();

                String errorMsg = "Erro: API Gemini retornou código ";
                errorMsg += String(httpCode);
                return errorMsg;
            }
        } else {
            Serial.printf("[HTTP] POST... falhou, erro: %s\n", http.errorToString(httpCode).c_str());
            http.end();
            return "Erro: Falha na conexão com a API Gemini.";
        }
    } else {
        Serial.printf("[HTTP] Não foi possível conectar\n");
        return "Erro: Falha ao iniciar a conexão HTTP.";
    }
}

void handleStreamCallback(StreamData data) {
    Serial.printf("Stream: %s %s %s %s\n",
                  data.streamPath().c_str(),
                  data.dataPath().c_str(),
                  data.eventType().c_str(),
                  data.dataType().c_str());

    if (data.eventType() == "put" && data.dataPath() != "/") {
        String requestId = data.dataPath();
        requestId.remove(0, 1);

        Serial.print("Nova solicitação de sugestão recebida, ID: ");
        Serial.println(requestId);

        FirebaseJson *json = data.jsonObjectPtr();
        String petType, petWeight, petAge, petSize;

        FirebaseJsonData result;
        json->get(result, "petType"); if (result.success) petType = result.to<String>();
        json->get(result, "petWeight"); if (result.success) petWeight = result.to<String>();
        json->get(result, "petAge"); if (result.success) petAge = result.to<String>();
        json->get(result, "petSize"); if (result.success) petSize = result.to<String>();

        if (petType.length() > 0 && petWeight.length() > 0 && petAge.length() > 0 && petSize.length() > 0) {
            String prompt = "Sugira uma rotina de alimentação ideal (frequência diária e melhor intervalo entre refeições) para um ";
            prompt += petType;
            prompt += " com ";
            prompt += petAge;
            prompt += " anos, pesando ";
            prompt += petWeight;
            prompt += " kg e de porte ";
            prompt += petSize;
            prompt += ". A resposta deve ser curta e direta, adequada para exibição em um app, focando na frequência e intervalo.";


            Serial.print("Prompt Gemini: ");
            Serial.println(prompt);

            String suggestion = getGeminiSuggestion(prompt);

            String responsePath = String(FB_RESPONSES_PATH);
            responsePath += "/";
            responsePath += requestId;

            if (Firebase.setString(fbdo, responsePath, suggestion)) {
                Serial.println("Sugestão enviada para o Firebase.");

                String requestPath = String(FB_REQUESTS_PATH);
                requestPath += "/";
                requestPath += requestId;

                Firebase.deleteNode(fbdo, requestPath);
            } else {
                Serial.print("Erro ao enviar sugestão para o Firebase: ");
                Serial.println(fbdo.errorReason());
            }


        } else {
            Serial.println("Dados da requisição incompletos, ignorando.");

            String responsePath = String(FB_RESPONSES_PATH);
            responsePath += "/";
            responsePath += requestId;
            Firebase.setString(fbdo, responsePath, "Erro: Dados incompletos na requisição.");

            String requestPath = String(FB_REQUESTS_PATH);
            requestPath += "/";
            requestPath += requestId;
            Firebase.deleteNode(fbdo, requestPath);
        }
    }
}

void iniciarFirebase() {
    config.database_url = DATABASE_URL;
    config.signer.tokens.legacy_token = DATABASE_SECRET;
    Firebase.reconnectNetwork(true);
    fbdo.setBSSLBufferSize(4096, 1024);
    stream.setBSSLBufferSize(4096, 1024);
    Firebase.begin(&config, &auth);
    Firebase.setBool(fbdo, "/comando/abrir", false);
    carregarConfiguracoesIniciaisFirebase();

    if (!Firebase.beginStream(stream, FB_REQUESTS_PATH)) {
        Serial.print("Erro ao iniciar o stream do Firebase: ");
        Serial.println(stream.errorReason());
    } else {
        Serial.print("Stream do Firebase iniciado em: ");
        Serial.println(FB_REQUESTS_PATH);
    }
}

float medirDistanciaCM() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duracao = pulseIn(ECHO_PIN, HIGH, 30000);
    float distancia = (duracao > 0) ? (duracao * 0.034 / 2.0) : 999.0;
    return distancia;
}

void enviarDadosParaFirebase(float dist, int pot, int tempo, const String &modo, const String &evento) {
    if (!staConectado || !Firebase.ready()) return;

    FirebaseJson json;
    json.set("distancia_cm", String(dist, 1));
    json.set("potenciometro", pot);
    json.set("tempo_aberto_ms", tempo);
    json.set("modo", modo);
    json.set("evento", evento);
    json.set("limite_sensor_cm", sensorThresholdCM);
    Firebase.set(fbdo, "/estado", json);
}

void registrarRefeicaoFirebase() {
    if (!staConectado || !Firebase.ready()) return;

    FirebaseData fbdoMetric;

    Firebase.getInt(fbdoMetric, "/metricas/refeicoes_hoje");
    int contador = 0;
    if (fbdoMetric.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
        contador = fbdoMetric.intData();
    }
    contador++;
    Firebase.setInt(fbdoMetric, "/metricas/refeicoes_hoje", contador);

    FirebaseJson jsonTimestamp;
    jsonTimestamp.set(".sv", "timestamp");
    Firebase.set(fbdoMetric, "/metricas/ultima_refeicao_timestamp", jsonTimestamp);
}

void acionarServoNaoBloqueante(int tempo, const String &motivo) {
    if (servoAtivo) return;

    atualizarStatus(motivo);
    servo1.write(90);
    analogWrite(LED_PIN, brilhoInicial);
    tempoInicioAcionamento = millis();
    tempoAcionamento = tempoInicioAcionamento + tempo;
    servoAtivo = true;
    enviarDadosParaFirebase(distanciaAtual, potValueAtual, tempo, origemPrioritaria, motivo);
    registrarRefeicaoFirebase();
}

void atualizarServo() {
    if (servoAtivo) {
        unsigned long agora = millis();
        if (agora >= tempoAcionamento) {
            servo1.write(0);
            analogWrite(LED_PIN, 0);
            servoAtivo = false;
            atualizarStatus("Porta fechada");
        } else {
            int tempoTotal = tempoAcionamento - tempoInicioAcionamento;
            int tempoDecorrido = agora - tempoInicioAcionamento;
            int brilhoAtual = map(tempoDecorrido, 0, tempoTotal, brilhoInicial, 0);
            analogWrite(LED_PIN, constrain(brilhoAtual, 0, 255));
        }
    }
}

void handleAbrir() {
    if (server.hasArg("tempo")) {
        int tempoReq = server.arg("tempo").toInt();
        tempoReq = constrain(tempoReq, MIN_SERVO_TIME_MS, MAX_SERVO_TIME_MS);
        acionarServoNaoBloqueante(tempoReq, "Abertura via Pagina AP");
        server.send(200, "text/plain", "Servo acionado");
    } else {
        server.send(400, "text/plain", "Parâmetro 'tempo' ausente");
    }
}

void handleStatus() {
    server.send(200, "text/plain", currentStatus);
}

void handleRoot() {
    IPAddress clientIP = server.client().remoteIP();
    IPAddress apIP = WiFi.softAPIP();

        if (LittleFS.exists("/index.html")) {
            File file = LittleFS.open("/index.html", "r");
            server.streamFile(file, "text/html");
            file.close();
        } else {
            server.send(200, "text/html", SIMPLE_HTML_PAGE);
        }
}

void setup() {
    Serial.begin(9600);

    if (!LittleFS.begin(true)) {
        Serial.println("Erro critico ao montar LittleFS");
        return;
    }
    Serial.println("LittleFS montado com sucesso.");

    configurarWiFiDuplo();

    if (staConectado) {
        iniciarFirebase();
    }

    server.on("/", HTTP_GET, handleRoot);
    server.on("/abrir", HTTP_GET, handleAbrir);
    server.on("/status", HTTP_GET, handleStatus);
    server.begin();
    Serial.println("Servidor HTTP iniciado.");

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(POTENTIOMETER_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BOTAO_PIN, INPUT_PULLUP);

    servo1.attach(SERVO_PIN);
    servo1.write(0);

    atualizarStatus("Sistema pronto!");
}

void processarPotenciometro() {
    potValueAtual = analogRead(POTENTIOMETER_PIN);
    int indicePot = map(potValueAtual, 0, 4095, 0, 18);
    int tempoPotenciometro = MIN_SERVO_TIME_MS + (indicePot * 500);

    if (indicePot == 0) {
        if (origemPrioritaria != "firebase") {
            origemPrioritaria = "firebase";
            if (staConectado) Firebase.setString(fbdo, "/comando/origem", "firebase");
            atualizarStatus("Modo: Firebase");
        }
    } else {
        if (indicePot != ultimoPotIndice) {
            if (origemPrioritaria != "potenciometro") {
                origemPrioritaria = "potenciometro";
                if (staConectado) Firebase.setString(fbdo, "/comando/origem", "potenciometro");
                atualizarStatus("Modo: Potenciometro");
            }
            ultimoPotIndice = indicePot;
            tempoEmMs = tempoPotenciometro;
            if (staConectado) Firebase.setInt(fbdo, "/comando/tempo_remoto_ms", tempoEmMs);
            Serial.print("Tempo ajustado pelo potenciometro para: ");
            Serial.print(tempoEmMs);
            Serial.println("ms");
        }
    }
}

void processarBotao() {
    bool estadoBotao = digitalRead(BOTAO_PIN);
    if (estadoAnteriorBotao == HIGH && estadoBotao == LOW) {
        if (!servoAtivo) {
           acionarServoNaoBloqueante(tempoEmMs, "Abertura manual via botao");
        }
    }
    estadoAnteriorBotao = estadoBotao;
}

void verificarComandosEConfiguracoesFirebase() {
    if (!staConectado || !Firebase.ready()) return;

    FirebaseData fbdoCheck;

    if (Firebase.getString(fbdoCheck, "/comando/origem")) {
        String origemRemota = fbdoCheck.stringData();
        if (origemRemota != origemPrioritaria) {
            origemPrioritaria = origemRemota;
            String msg = "Modo alterado para ";
            msg += origemPrioritaria;
            msg += " remotamente";
            atualizarStatus(msg);

        }
    }

    if (origemPrioritaria == "firebase") {
        if (Firebase.getInt(fbdoCheck, "/comando/tempo_remoto_ms")) {
            int tempoRemoto = fbdoCheck.intData();
            if (tempoRemoto >= MIN_SERVO_TIME_MS && tempoRemoto <= MAX_SERVO_TIME_MS && tempoRemoto != tempoEmMs) {
                tempoEmMs = tempoRemoto;
                ultimoPotIndice = (tempoEmMs - MIN_SERVO_TIME_MS) / 500;
                Serial.print("Tempo atualizado via Firebase para: ");
                Serial.print(tempoEmMs);
                Serial.println("ms");
                atualizarStatus("Tempo ajustado via Firebase");
            }
        }
    }

    if (Firebase.getBool(fbdoCheck, "/comando/abrir") && fbdoCheck.boolData()) {
        if (!servoAtivo) {
            acionarServoNaoBloqueante(tempoEmMs, "Abertura remota via Firebase");
        }
        Firebase.setBool(fbdo, "/comando/abrir", false);
    }

    unsigned long agora = millis();
    if (agora - lastConfigCheck >= configCheckInterval) {
        lastConfigCheck = agora;
        if (Firebase.getInt(fbdoCheck, "/comando/distancia_threshold_cm")) {
            if (fbdoCheck.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
                int novoThreshold = fbdoCheck.intData();
                if (novoThreshold > 0 && novoThreshold <= 100 && novoThreshold != sensorThresholdCM) {
                    sensorThresholdCM = novoThreshold;
                    Serial.print("Threshold do sensor atualizado via Firebase: ");
                    Serial.println(sensorThresholdCM);
                    atualizarStatus("Limite do sensor ajustado");
                }
            }
        }
    }
}

void verificarSensorEAcionar() {
    distanciaAtual = medirDistanciaCM();
    if (distanciaAtual > 0 && distanciaAtual < sensorThresholdCM) {
        if (!servoAtivo) {
           Serial.print("Sensor detectou proximidade: ");
           Serial.print(distanciaAtual,0);
           Serial.println("cm");
           acionarServoNaoBloqueante(tempoEmMs, "Abertura automatica por sensor");
        }
    }
}

void loop() {
    unsigned long agora = millis();

    server.handleClient();
    atualizarServo();

    if (servoAtivo) return;

    if (agora - lastSensorRead >= sensorInterval) {
        lastSensorRead = agora;
        verificarSensorEAcionar();
    }

    if (agora - lastInputCheck >= inputCheckInterval) {
        lastInputCheck = agora;
        processarPotenciometro();
        processarBotao();
    }

    if (agora - lastFirebaseCheck >= firebaseCheckInterval) {
        lastFirebaseCheck = agora;
        verificarComandosEConfiguracoesFirebase();
    }

    if (agora - lastFirebaseUpdate >= firebaseUpdateInterval) {
         lastFirebaseUpdate = agora;
         if(!servoAtivo) {
            enviarDadosParaFirebase(distanciaAtual, potValueAtual, tempoEmMs, origemPrioritaria, currentStatus);
         }
    }
}