# 🐾 PetFeeder - Alimentador Automático Inteligente

Este repositório contém o código-fonte e a documentação do projeto PetFeeder, desenvolvido para a cadeira de **Sistemas Embarcados** na **Cesar School**.

---

## 📖 Sobre

O PetFeeder é um alimentador automático inteligente para animais de estimação. O objetivo deste projeto foi desenvolver um sistema embarcado utilizando um **ESP32** para automatizar a alimentação de pets. O sistema integra hardware (sensores e atuadores) com um **dashboard web interativo**, conectado em tempo real via **Firebase**, e também oferece uma interface de controle local via **Access Point (AP)**. Além disso, incorpora a **API Gemini do Google** para sugerir rotinas de alimentação personalizadas.

---

## 🛠️ Como Funciona

O coração do PetFeeder é um **ESP32**, que gerencia os seguintes componentes e funcionalidades:

* **ESP32:** O cérebro do sistema, processando dados, controlando atuadores, gerenciando conexões Wi-Fi (STA e AP), servindo páginas web via LittleFS e comunicando-se com o Firebase e a API Gemini.
* **Sensor Ultrassônico de Distância:** Detecta a presença do animal próximo ao comedouro, podendo iniciar o processo de alimentação no modo automático.
* **Motor Servo SG90:** Abre e fecha a comporta que libera a ração, controlado com precisão pelo ESP32.
* **Potenciômetro:** Permite ajustar manualmente o tempo de abertura da comporta (quantidade de ração) ou selecionar o modo de operação (Firebase/Potenciômetro).
* **Botão Físico:** Oferece uma opção de acionamento manual imediato.
* **LED (com PWM):** Fornece feedback visual sobre o status de acionamento, com brilho que diminui gradualmente durante a abertura.
* **Firebase Realtime Database:** Atua como backend, sincronizando dados entre o ESP32 e o dashboard web principal, além de gerenciar solicitações e respostas para a API Gemini.
* **API Gemini (Google AI):** Gera sugestões de rotinas de alimentação com base nas informações do pet fornecidas pelo usuário no dashboard.
* **LittleFS & WebServer:** O ESP32 armazena e serve as interfaces web (`index.html` e `index_Simples.html`) diretamente, permitindo acesso via rede local ou AP.

### ⏳ Tempo vs. Quantidade

A quantidade de ração é controlada pelo **tempo de abertura da comporta**. Este tempo pode ser ajustado no dashboard ou pelo potenciômetro. A conversão exata de tempo para gramas depende da ração utilizada, sendo o ajuste fino de responsabilidade do usuário.

---

## 📊 Sobre os Dashboards

O projeto oferece duas interfaces web:

### 1. Dashboard Principal (`index.html`)

A interface principal, rica em recursos e conectada ao Firebase.

* **Conexão Firebase:** Sincronização em tempo real para monitoramento e controle.
* **Múltiplas Telas:** Inclui telas de Boas-Vindas, Configuração Inicial, Demonstração, Início (Dashboard), Monitoramento, Relatórios, Rotinas e Configurações.
* **Recursos Avançados:**
    * Configuração do pet (nome, ícone).
    * Múltiplos temas visuais (Escuro, Claro, Azul, Roxo).
    * Painéis de acesso rápido personalizáveis na tela inicial.
    * Monitoramento detalhado (status, distância, porta).
    * Ajustes remotos (tempo de abertura, distância do sensor).
    * Alimentação manual (Dashboard, Monitor).
    * Relatórios detalhados e histórico de alimentações.
    * Criação de rotinas manuais (com seletor de data/hora estilo iOS) e **sugestões via IA Gemini**.
    * Gerenciamento completo das configurações do app e do pet.
* **Interface Moderna:** Inspirada em interfaces móveis, com navegação intuitiva e design responsivo.

### 2. Dashboard Local (`index_Simples.html`)

Uma interface simplificada, ideal para acesso via Access Point quando não há conexão com a internet ou Firebase.

* **Acesso Local:** Funciona diretamente pela rede criada pelo ESP32.
* **Funcionalidades Básicas:**
    * Configuração inicial do pet.
    * Controle manual de abertura da porta com ajuste de tempo.
    * Criação e gerenciamento de rotinas simples (início, fim, intervalo).
    * Visualização de status local.
    * Opção de resetar o app.

---

## 🔥 Firebase & Gemini AI

* **Firebase:** Utilizamos o **Firebase Realtime Database** como a ponte de comunicação principal. A estrutura inclui:
    * `/estado`: Mantém os dados atuais do dispositivo (distância, modo, evento, etc.).
    * `/metricas`: Armazena estatísticas como refeições diárias e último timestamp.
    * `/comando`: Recebe comandos do dashboard (abrir, tempo, threshold).
    * `/routineRequests` & `/routineResponses`: Gerencia a comunicação assíncrona com a API Gemini, onde o dashboard envia uma solicitação e o ESP32 busca a API, postando a resposta.
* **Gemini AI:** Integrado ao ESP32, o sistema pode receber dados do pet (via Firebase), construir um *prompt* e consultar a API Gemini para obter sugestões inteligentes de rotinas de alimentação, que são então enviadas de volta ao dashboard.

---

## ⚙️ Como Usar

1.  **Energia:** Conecte o dispositivo a uma fonte USB 5V.
2.  **Conexão:**
    * **Internet (Recomendado):** O ESP32 tentará se conectar ao Wi-Fi configurado (`WIFI_SSID` e `WIFI_PASSWORD` no `main.cpp`). Se conectar, acesse o dashboard principal (`index.html`) pelo repositório ou acesse o IP local do ESP32 para a interface completa.
    * **Access Point:** Se não conseguir conectar à internet, ele criará uma rede Wi-Fi chamada "PetFeeder" (senha: "senha1234"). Conecte-se a essa rede e acesse o IP do AP (geralmente 192.168.4.1) para usar o `index_Simples.html`.
3.  **Configuração:** Siga os passos no dashboard para configurar o nome e tipo do seu pet.
4.  **Uso:**
    * **Automático:** O sensor detectará o pet e acionará a alimentação.
    * **Manual:** Use o botão "Alimentar Agora" no dashboard ou o botão físico.
    * **Ajustes:** Use os sliders/inputs no dashboard ou o potenciômetro para ajustar tempo e distância.
    * **Rotinas:** Crie rotinas personalizadas ou use as sugestões da IA na aba "Rotinas".
5.  **Reabastecimento:** Abra a parte superior para adicionar ração.

---

## 🎨 Design e Documentação

O design físico inicial, embora planejado para impressão 3D, foi executado com papelão e fita por questões de agilidade. A versão final foi modelada em Fusion 360 e cortada a laser, melhorando a estrutura e a funcionalidade.

A documentação detalhada, incluindo a evolução do projeto e os requisitos atendidos, pode ser encontrada em `index_sobre.html`, que apresenta uma narrativa interativa com vídeos controláveis e snippets de código.

---

## 💻 Como foi feito o `index.html`?

O dashboard principal (`index.html`) foi construído com:

* **HTML5:** Estrutura semântica para todas as telas e componentes.
* **CSS3:** Estilização avançada com variáveis CSS para temas, layout flexível/grid e design responsivo, incluindo elementos como seletores iOS.
* **JavaScript (Vanilla):**
    * **Firebase SDK:** Para conexão e comunicação em tempo real.
    * **Manipulação do DOM:** Atualiza dinamicamente todos os elementos da interface.
    * **Lógica de Negócio:** Gerencia navegação, validações, armazenamento local (LocalStorage para configurações), interações com pickers e a lógica para solicitar sugestões à IA (via Firebase).

---

## 🧠 Como foi feito o `main.cpp`?

O `main.cpp` é o firmware que roda no ESP32, escrito em C++ com o **framework Arduino**:

1.  **Bibliotecas:** Inclui `ESP32Servo`, `WiFi`, `FirebaseESP32`, `WebServer`, `LittleFS`, `HTTPClient`, `WiFiClientSecure`, e `ArduinoJson`.
2.  **Configurações:** Define pinos, credenciais Wi-Fi/Firebase, URL da API Gemini e configurações operacionais.
3.  **`setup()`:**
    * Inicializa Serial e LittleFS.
    * Configura pinos e servo.
    * Inicia o modo Wi-Fi duplo (AP+STA).
    * Se conectado à STA, inicia o Firebase e o listener de streams para a IA.
    * Configura e inicia o WebServer, mapeando rotas (`/`, `/abrir`, `/status`) para servir as páginas HTML e lidar com requisições locais.
4.  **`loop()`:**
    * Gerencia o `WebServer`.
    * Atualiza o estado do servo (controla o fechamento após tempo).
    * Executa verificações periódicas (não bloqueantes) usando `millis()` para:
        * Ler o sensor ultrassônico e acionar, se necessário.
        * Processar o potenciômetro e o botão.
        * Verificar comandos e configurações do Firebase.
        * Enviar atualizações de status para o Firebase.
5.  **Funções Principais:**
    * `configurarWiFiDuplo()`: Tenta conectar à rede local e, se falhar, mantém o AP ativo.
    * `iniciarFirebase()`: Configura e inicia a conexão e o stream.
    * `getGeminiSuggestion()`: Realiza a chamada HTTPS para a API Gemini, processa a resposta JSON.
    * `handleStreamCallback()`: Processa novas requisições de sugestão do Firebase.
    * `acionarServoNaoBloqueante()`: Controla a abertura, LED e registro no Firebase.
    * `handleRoot()`, `handleAbrir()`, `handleStatus()`: Funções do WebServer.
