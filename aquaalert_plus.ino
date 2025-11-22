#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>


// Configurações Wi-Fi
const char* ssid = "Wokwi-GUEST";  // Nome da rede (no Wokwi use este)
const char* password = "";          // Senha (vazio no Wokwi)

// Configurações MQTT
const char* mqtt_server = "broker.hivemq.com";  // Broker público gratuito
const int mqtt_port = 1883;
const char* mqtt_topic_temp = "aquaalert/temperatura";
const char* mqtt_topic_umid = "aquaalert/umidade";
const char* mqtt_topic_umidificador = "aquaalert/umidificador";
const char* mqtt_topic_copos = "aquaalert/copos";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long ultimoEnvioMQTT = 0;
const long intervaloMQTT = 30000;  // Envia a cada 30 segundos


// Definição dos pinos
#define DHTPIN 15
#define DHTTYPE DHT22
#define LED_VERDE 2
#define LED_AMARELO 4
#define LED_VERMELHO 5
#define BUZZER 18
#define BOTAO_AGUA 13
#define BOTAO_MENU 12
#define RELE_UMIDIFICADOR 19

// Inicialização dos componentes
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variáveis do sistema
int coposConsumidos = 0;
int metaDiaria = 8;
unsigned long ultimoConsumo = 0;
unsigned long intervaloLembrete = 1800000;
float temperatura = 0;
float umidade = 0;
bool alertaAtivo = false;
bool umidificadorLigado = false;
bool wifiConectado = false;


void conectarWiFi() {
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(ssid, password);

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    wifiConectado = true;
  } else {
    Serial.println("\nFalha ao conectar Wi-Fi. Continuando sem internet...");
    wifiConectado = false;
  }
}

void conectarMQTT() {
  if (!wifiConectado) return;

  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    String clientId = "AquaAlert-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("MQTT conectado!");
    } else {
      Serial.print("Falha MQTT, rc=");
      Serial.println(client.state());
      delay(2000);
      return;
    }
  }
}

void enviarDadosMQTT() {
  if (!wifiConectado || !client.connected()) {
    return;
  }

  // Envia temperatura
  char temp[10];
  dtostrf(temperatura, 4, 1, temp);
  client.publish(mqtt_topic_temp, temp);

  // Envia umidade
  char umid[10];
  dtostrf(umidade, 4, 1, umid);
  client.publish(mqtt_topic_umid, umid);

  // Envia status do umidificador
  client.publish(mqtt_topic_umidificador, umidificadorLigado ? "LIGADO" : "DESLIGADO");

  // Envia copos consumidos
  char copos[10];
  sprintf(copos, "%d/%d", coposConsumidos, metaDiaria);
  client.publish(mqtt_topic_copos, copos);

  Serial.println(">>> Dados enviados via MQTT!");
}



void setup() {
  Serial.begin(115200);

  // Configuração dos pinos
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(RELE_UMIDIFICADOR, OUTPUT);
  pinMode(BOTAO_AGUA, INPUT_PULLUP);
  pinMode(BOTAO_MENU, INPUT_PULLUP);

  // Inicialização do LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  AquaAlert+  ");
  lcd.setCursor(0, 1);
  lcd.print(" Iniciando... ");
  delay(2000);

  // Conectar Wi-Fi
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectando WiFi");
  conectarWiFi();

  // Configurar MQTT
  if (wifiConectado) {
    client.setServer(mqtt_server, mqtt_port);
    conectarMQTT();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi + MQTT OK!");
    delay(2000);
  }

  // Inicialização do sensor DHT
  dht.begin();

  // LED verde indica sistema OK
  digitalWrite(LED_VERDE, HIGH);

  Serial.println("=== Sistema AquaAlert+ Iniciado ===");
  Serial.println("Monitorando hidratacao e qualidade do ar!");
}

void loop() {
  // Mantém conexão MQTT
  if (wifiConectado && !client.connected()) {
    conectarMQTT();
  }
  if (wifiConectado) {
    client.loop();
  }

  lerSensor();
  verificarBotoes();
  verificarLembrete();
  atualizarDisplay();

  // Envia dados via MQTT a cada 30 segundos
  if (wifiConectado && (millis() - ultimoEnvioMQTT >= intervaloMQTT)) {
    enviarDadosMQTT();
    ultimoEnvioMQTT = millis();
  }

  delay(1000);
}

void lerSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t) && !isnan(h)) {
    temperatura = t;
    umidade = h;

    // Controle do umidificador
    if (umidade < 40 && !umidificadorLigado) {
      digitalWrite(RELE_UMIDIFICADOR, HIGH);
      umidificadorLigado = true;
      Serial.println(">>> Umidificador LIGADO - Ar muito seco!");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Umidade Baixa!");
      lcd.setCursor(0, 1);
      lcd.print("Ligando ar...");
      tone(BUZZER, 1500, 100);
      delay(2000);

    } else if (umidade > 60 && umidificadorLigado) {
      digitalWrite(RELE_UMIDIFICADOR, LOW);
      umidificadorLigado = false;
      Serial.println(">>> Umidificador DESLIGADO - Umidade OK");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Umidade OK!");
      lcd.setCursor(0, 1);
      lcd.print("Desligando...");
      delay(2000);
    }

    // Ajusta intervalo baseado na temperatura
    if (temperatura > 30) {
      intervaloLembrete = 900000;
    } else if (temperatura > 25) {
      intervaloLembrete = 1800000;
    } else {
      intervaloLembrete = 3600000;
    }

    Serial.print("Temp: ");
    Serial.print(temperatura);
    Serial.print("°C | Umidade: ");
    Serial.print(umidade);
    Serial.print("% | Umidificador: ");
    Serial.println(umidificadorLigado ? "LIGADO" : "DESLIGADO");
  }
}

void verificarBotoes() {
  if (digitalRead(BOTAO_AGUA) == LOW) {
    delay(50);
    if (digitalRead(BOTAO_AGUA) == LOW) {
      registrarConsumo();
      while(digitalRead(BOTAO_AGUA) == LOW);
    }
  }

  if (digitalRead(BOTAO_MENU) == LOW) {
    delay(50);
    if (digitalRead(BOTAO_MENU) == LOW) {
      metaDiaria++;
      if (metaDiaria > 12) metaDiaria = 8;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Meta ajustada:");
      lcd.setCursor(0, 1);
      lcd.print(metaDiaria);
      lcd.print(" copos/dia");
      delay(2000);
      while(digitalRead(BOTAO_MENU) == LOW);
    }
  }
}

void registrarConsumo() {
  coposConsumidos++;
  ultimoConsumo = millis();
  alertaAtivo = false;

  digitalWrite(LED_AMARELO, LOW);
  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(BUZZER, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Muito bem!  ");
  lcd.setCursor(0, 1);
  lcd.print("   Copo #");
  lcd.print(coposConsumidos);

  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_VERDE, LOW);
    delay(200);
    digitalWrite(LED_VERDE, HIGH);
    delay(200);
  }

  Serial.print(">>> Consumo registrado! Total: ");
  Serial.print(coposConsumidos);
  Serial.print("/");
  Serial.println(metaDiaria);

  // Envia imediatamente via MQTT
  if (wifiConectado && client.connected()) {
    char copos[10];
    sprintf(copos, "%d/%d", coposConsumidos, metaDiaria);
    client.publish(mqtt_topic_copos, copos);
    Serial.println(">>> Consumo enviado via MQTT!");
  }

  delay(2000);
}

void verificarLembrete() {
  unsigned long tempoDecorrido = millis() - ultimoConsumo;

  if (tempoDecorrido >= intervaloLembrete && !alertaAtivo) {
    alertaAtivo = true;
    digitalWrite(LED_AMARELO, HIGH);
    tone(BUZZER, 2000, 200);
    Serial.println(">>> LEMBRETE: Hora de beber agua!");
  }

  if (tempoDecorrido >= (intervaloLembrete * 2)) {
    digitalWrite(LED_VERMELHO, HIGH);
    if ((millis() / 500) % 2 == 0) {
      tone(BUZZER, 2500, 200);
    }
    Serial.println(">>> ALERTA URGENTE: Beba agua agora!");
  }
}

void atualizarDisplay() {
  if (!alertaAtivo) {
    lcd.clear();

    // Linha 1: Temp, Umidade e Status
    lcd.setCursor(0, 0);
    lcd.print(temperatura, 1);
    lcd.print("C ");
    lcd.print((int)umidade);
    lcd.print("%");
    if (umidificadorLigado) {
      lcd.print(" *U");
    }

    // Linha 2: Copos e tempo
    lcd.setCursor(0, 1);
    lcd.print(coposConsumidos);
    lcd.print("/");
    lcd.print(metaDiaria);
    lcd.print(" ");

    unsigned long tempoRestante = intervaloLembrete - (millis() - ultimoConsumo);
    if (tempoRestante > intervaloLembrete) tempoRestante = 0;

    int minutosRestantes = tempoRestante / 60000;
    lcd.print(minutosRestantes);
    lcd.print("min");

  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("HORA DE BEBER");
    lcd.setCursor(0, 1);
    lcd.print("    AGUA!   ");
  }
}
