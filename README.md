# 🐾 PetFeeder - Alimentador Automático Inteligente

Este repositório contém o código-fonte e a documentação do projeto PetFeeder, desenvolvido para a cadeira de **Sistemas Embarcados** na **Cesar School**.

---

## 📖 Sobre

Um **PetFeeder** é um alimentador automático para animais de estimação. O objetivo deste projeto, criado por mim, foi desenvolver um sistema embarcado capaz de automatizar a tarefa de alimentar pets, utilizando um microcontrolador ESP32 e integrando-o a um dashboard web para monitoramento e controle.

---

## 🛠️ Explique como é feito

O coração do PetFeeder é um **ESP32**, que gerencia os seguintes componentes de hardware:

* **Sensor Ultrassônico de Distância:** Detecta a presença do animal próximo ao comedouro, iniciando o processo de alimentação.
* **Motor Servo SG90:** Responsável por abrir e fechar a comporta que libera a ração no prato.
* **Potenciômetro:** Permite ao usuário ajustar manualmente o tempo que a comporta permanecerá aberta, controlando assim a quantidade de ração liberada.
* **ESP32:** O cérebro do sistema, processando as informações dos sensores, controlando o motor e comunicando-se com o Firebase para enviar e receber dados.

### ⏳ TEMPO por QUANTIDADE

Para controlar a quantidade de comida, ajuste o tempo de abertura da comporta através do potenciômetro. Como a vazão pode variar dependendo do tipo e tamanho da ração, não realizamos uma tradução direta de "tempo" para "gramas". **O ajuste fino fica por conta do usuário!**

---

## 📊 Sobre o Dashboard

O dashboard web, acessível através do arquivo `index.html`, é a interface de usuário para monitorar e interagir com o PetFeeder.

* **Conexão Direta com Firebase:** O dashboard se comunica em tempo real com o **Firebase**, tanto para **enviar comandos** (como uma alimentação manual, por exemplo) quanto para **receber dados** (status do sensor, histórico de alimentação).
* **Visualização de Dados:** Apresenta os dados recebidos do ESP32, os comandos enviados e um **relatório gerado** com o histórico de atividades.
* **Aba "Sobre o Dashboard":** Uma seção dedicada a explicar o propósito do desenvolvimento, os requisitos da cadeira que foram atendidos e outros detalhes relevantes sobre a interface web.

---

## 🔥 Firebase

Utilizamos o **Firebase Realtime Database** como backend e ponte de comunicação entre o hardware (ESP32) e o software (Dashboard HTML). Ele atua como um intermediário, recebendo dados do sensor e status do ESP32 e enviando comandos ou configurações do dashboard para o dispositivo físico.

---

## ⚙️ Como usar o PetFeeder

Usar o PetFeeder é super simples:

1.  **Alimentação:** Conecte o dispositivo a qualquer fonte de energia USB (5V), como a porta de um PC/Notebook ou um carregador de celular comum.
2.  **Ajuste:** Utilize o **potenciômetro** localizado na parte traseira para definir o tempo de abertura da comporta (e, consequentemente, a quantidade de ração).
3.  **Sensor:** Certifique-se de que o **sensor ultrassônico** na frente do aparelho não esteja obstruído para que ele possa detectar seu pet corretamente.
4.  **Ração e Acesso:** A parte superior do PetFeeder pode ser aberta para reabastecer a ração e, se necessário, acessar o ESP32 e os componentes internos.
5.  **Pronto!** A mágica acontecerá assim que seu pet se aproximar.

---

## 🎨 Design do PetFeeder

A ideia inicial era construir o case do PetFeeder utilizando **impressão 3D** para um acabamento mais profissional. No entanto, devido aos prazos apertados do projeto e aos riscos inerentes ao processo de impressão (tempo, possíveis falhas), optamos por uma abordagem mais **caseira e ágil**: utilizamos **papelão, fita adesiva e muita força de vontade**! 💪 O resultado, embora rústico, foi funcional e permitiu a entrega do projeto completo a tempo.

---

## 💻 Como foi feito o `index.html`?

O dashboard (`index.html`) foi construído utilizando as tecnologias web fundamentais:

* **HTML5:** Para a estrutura semântica da página, organizando os elementos como painéis de dados, botões e a aba "Sobre".
* **CSS3:** Para a estilização, buscando criar uma interface visualmente agradável e intuitiva. Foram aplicados estilos para layout (possivelmente usando Flexbox ou Grid), cores, tipografia e responsividade básica.
* **JavaScript (Vanilla):** O cérebro por trás da interatividade. O JS é responsável por:
    * **Conexão com o Firebase:** Utiliza a SDK do Firebase para JavaScript para estabelecer a comunicação em tempo real, "ouvindo" as atualizações do banco de dados (vindos do ESP32) e enviando dados (comandos do usuário).
    * **Manipulação do DOM:** Atualiza dinamicamente os elementos HTML para exibir os dados recebidos (status, relatórios) e responder às interações do usuário (cliques em botões).
    * **Lógica da Interface:** Implementa a navegação entre abas (como a "Sobre o Dashboard") e a lógica para gerar ou formatar os relatórios exibidos.

O foco foi criar uma interface **funcional e reativa**, que permitisse ao usuário entender rapidamente o estado do PetFeeder e interagir com ele, tudo isso se comunicando de forma eficiente com o Firebase.

---

## 🧠 Como foi feito o `main.cpp`?

O `main.cpp` é o firmware que roda no ESP32, escrito em C++ utilizando o **framework Arduino**. A lógica principal segue estes passos:

1.  **Inclusão de Bibliotecas:** Inclui as bibliotecas necessárias para:
    * Controlar o Servo Motor (`Servo.h`).
    * Interagir com o ESP32 (`Arduino.h`).
    * Conectar-se ao Wi-Fi (`WiFi.h`).
    * Comunicar-se com o Firebase (`Firebase_ESP_Client.h` ou similar).
    * Ler o sensor ultrassônico (geralmente não requer uma biblioteca específica, mas sim lógica com `pulseIn` ou `NewPing.h`).
2.  **Configurações e Definições:** Define os pinos GPIO utilizados para o sensor, servo e potenciômetro. Configura as credenciais do Wi-Fi e os detalhes de autenticação e URL do Firebase.
3.  **`setup()`:**
    * Inicializa a comunicação serial para debug.
    * Configura os pinos (sensor como `INPUT`, servo como `OUTPUT`, potenciômetro como `INPUT`).
    * Conecta-se à rede Wi-Fi.
    * Estabelece a conexão com o Firebase.
    * Move o servo para a posição inicial (fechado).
4.  **`loop()`:**
    * **Leitura do Sensor:** Constantemente mede a distância usando o sensor ultrassônico.
    * **Leitura do Potenciômetro:** Lê o valor analógico do potenciômetro para determinar o tempo de abertura desejado.
    * **Detecção do Pet:** Se a distância medida for menor que um limite pré-definido (indicando a presença do pet):
        * Chama a função `alimentar()`.
    * **Comunicação Firebase:** Verifica se há novos dados/comandos no Firebase e atualiza o Firebase com o status atual (nível do sensor, último horário de alimentação, etc.).
    * **Delay:** Adiciona um pequeno atraso para evitar leituras/processamentos excessivos.
5.  **`alimentar()`:**
    * Abre a comporta (move o servo para a posição aberta).
    * Envia um registro para o Firebase indicando que a alimentação ocorreu.
    * Aguarda o tempo definido pelo potenciômetro.
    * Fecha a comporta (move o servo para a posição fechada).
6.  **Funções Auxiliares:** Funções para lidar com a reconexão Wi-Fi e Firebase, se necessário.

O código é projetado para ser **reativo** (respondendo à presença do pet) e **conectado** (mantendo a comunicação com o dashboard via Firebase).
