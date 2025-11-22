# AquaAlert+ - Sistema IoT de Hidratação e Qualidade do Ar

<img width="800" height="800" alt="Image" src="https://github.com/user-attachments/assets/83855283-405c-4731-87da-7ba5e7862ab7" />

## Descrição

Sistema IoT que promove saúde através de:

- Lembretes inteligentes de hidratação baseados em temperatura
- Controle automático de umidificador quando ar está seco
- Monitoramento remoto via protocolo MQTT

## Objetivo

Contribuir para o ODS 3 (Saúde e Bem-estar) da ONU através de tecnologia acessível de baixo custo.

## Hardware Necessário

### Componentes principais:

- 1x ESP32 DevKit v1 (microcontrolador)
- 1x Sensor DHT22 (temperatura e umidade)
- 1x Display LCD 16x2 com módulo I2C
- 1x Módulo Relé 5V
- 1x Umidificador USB 5V
- 3x LEDs 5mm (verde, amarelo, vermelho)
- 1x Buzzer piezoelétrico ativo 5V
- 2x Botões tácteis (push-button)
- 3x Resistores 220Ω
- Jumpers macho-macho e macho-fêmea
- 1x Protoboard 830 pontos

### Custo total estimado: R$ 167,80

## Configuração MQTT

O sistema utiliza o broker público HiveMQ:

- **Broker:** broker.hivemq.com
- **Porta:** 1883
- **QoS:** 0

### Tópicos MQTT:

- `aquaalert/temperatura` - Publica temperatura em °C
- `aquaalert/umidade` - Publica umidade relativa em %
- `aquaalert/umidificador` - Status: "LIGADO" ou "DESLIGADO"
- `aquaalert/copos` - Consumo de água: "X/Y" (atual/meta)

## Bibliotecas Necessárias

Instale estas bibliotecas no Arduino IDE:

```cpp
#include <Wire.h>              // Comunicação I2C (nativa)
#include <LiquidCrystal_I2C.h> // Controle do LCD I2C
#include <DHT.h>               // Sensor DHT22
#include <WiFi.h>              // Wi-Fi do ESP32 (nativa)
#include <PubSubClient.h>      // Cliente MQTT
```

Como instalar:

1. Abra Arduino IDE
2. Vá em: Sketch → Incluir Biblioteca → Gerenciar Bibliotecas
3. Busque cada biblioteca pelo nome e clique em "Instalar"

## Tabela de Conexões

| Componente   | Pino do Componente | Pino do ESP32 | Função                    |
| ------------ | ------------------ | ------------- | ------------------------- |
| DHT22        | VCC                | 3V3           | Alimentação               |
| DHT22        | SDA/DATA           | GPIO 15       | Dados do sensor           |
| DHT22        | GND                | GND           | Terra                     |
| LCD I2C      | VCC                | 5V            | Alimentação               |
| LCD I2C      | GND                | GND           | Terra                     |
| LCD I2C      | SDA                | GPIO 21       | Dados I2C                 |
| LCD I2C      | SCL                | GPIO 22       | Clock I2C                 |
| Relé         | VCC                | 5V            | Alimentação               |
| Relé         | GND                | GND           | Terra                     |
| Relé         | IN                 | GPIO 19       | Sinal de controle         |
| LED Verde    | Anodo (+)          | GPIO 2        | Sistema OK                |
| LED Amarelo  | Anodo (+)          | GPIO 4        | Alerta hidratação         |
| LED Vermelho | Anodo (+)          | GPIO 5        | Alerta urgente            |
| LEDs (todos) | Catodo (-)         | GND           | Terra (via resistor 220Ω) |
| Buzzer       | Positivo           | GPIO 18       | Som de alerta             |
| Buzzer       | Negativo           | GND           | Terra                     |
| Botão 1      | Terminal 1         | GPIO 13       | Registrar água            |
| Botão 1      | Terminal 2         | GND           | Terra                     |
| Botão 2      | Terminal 1         | GPIO 12       | Ajustar meta              |
| Botão 2      | Terminal 2         | GND           | Terra                     |

## Como Configurar Wi-Fi

No código, encontre estas linhas e altere:

```cpp
const char* ssid = "SEU_WIFI_AQUI";      // Nome da sua rede Wi-Fi
const char* password = "SUA_SENHA_AQUI";  // Senha do Wi-Fi
```

## Como Usar

### Primeira vez:

1. Configure as credenciais Wi-Fi no código (veja acima)
2. Conecte o ESP32 no computador via cabo USB
3. Abra o arquivo aquaalert_plus.ino no Arduino IDE
4. Selecione a placa: Ferramentas → Placa → ESP32 → ESP32 Dev Module
5. Selecione a porta COM correta
6. Clique em Upload (seta →)
7. Aguarde o upload completar
8. Monte o circuito conforme tabela de conexões

### Uso normal:

- O sistema liga automaticamente ao energizar
- LCD mostra: temperatura, umidade, copos consumidos
- **Botão 1:** Pressione quando beber água
- **Botão 2:** Pressione para ajustar meta diária (8-12 copos)

### LEDs indicadores:

- **Verde aceso:** Sistema funcionando normalmente
- **Amarelo aceso:** Hora de beber água!
- **Vermelho aceso:** Alerta urgente - você esqueceu de beber água!

### Sistema de umidificador:

- Liga automaticamente quando umidade < 40%
- Desliga automaticamente quando umidade > 60%
- Símbolo "\*U" no LCD indica umidificador ligado

## Funcionamento do Sistema

### Inicialização:

1. ESP32 liga e configura pinos
2. Display mostra "AquaAlert+ - Iniciando..."
3. Conecta ao Wi-Fi configurado
4. Conecta ao broker MQTT
5. LED verde acende (sistema pronto)

### Ciclo de operação (a cada 1 segundo):

1. Lê temperatura e umidade do DHT22
2. Se umidade < 40%: liga relé (umidificador)
3. Se umidade > 60%: desliga relé
4. Verifica se é hora de lembrar de beber água
5. Atualiza informações no LCD
6. A cada 30 segundos: envia dados via MQTT

### Lembretes de hidratação:

- **Temperatura > 30°C:** Lembra a cada 15 minutos
- **Temperatura 25-30°C:** Lembra a cada 30 minutos
- **Temperatura < 25°C:** Lembra a cada 60 minutos

## Monitoramento Remoto

### Via MQTT Explorer:

1. Baixe: http://mqtt-explorer.com
2. Conecte ao broker.hivemq.com porta 1883
3. Visualize os tópicos aquaalert/\*
4. Veja dados em tempo real

### Via aplicativo MQTT (celular):

- Android: MQTT Dashboard
- iOS: MQTTool
- Configure mesma conexão (broker.hivemq.com:1883)

## Estrutura do Código

### Variáveis principais:

- `coposConsumidos`: Contador de copos bebidos
- `metaDiaria`: Meta de copos (8-12 ajustável)
- `temperatura`: Temperatura atual em °C
- `umidade`: Umidade relativa em %
- `umidificadorLigado`: Status do relé
- `alertaAtivo`: Indica se há alerta de hidratação

### Funções principais:

- `setup()`: Inicialização do sistema
- `loop()`: Ciclo principal de operação
- `lerSensor()`: Lê DHT22 e controla umidificador
- `verificarBotoes()`: Detecta pressionamento dos botões
- `registrarConsumo()`: Registra quando bebe água
- `verificarLembrete()`: Verifica se deve alertar
- `atualizarDisplay()`: Atualiza informações no LCD
- `conectarWiFi()`: Conecta à rede Wi-Fi
- `conectarMQTT()`: Conecta ao broker MQTT
- `enviarDadosMQTT()`: Publica dados nos tópicos

## Solução de Problemas

### Sensor DHT22 retorna NaN:

- Verifique conexões (VCC, GND, DATA)
- Aguarde 2 segundos após ligar
- Sensor pode estar com defeito

### LCD não mostra nada:

- Verifique conexões I2C (SDA, SCL)
- Tente endereço 0x3F em vez de 0x27
- Ajuste contraste do LCD (potenciômetro no módulo)

### Wi-Fi não conecta:

- Verifique SSID e senha no código
- Certifique-se que é rede 2.4GHz (ESP32 não suporta 5GHz)
- Aproxime o ESP32 do roteador

### MQTT não publica:

- Verifique conexão Wi-Fi primeiro
- Teste com MQTT Explorer se broker está acessível
- Verifique porta 1883 não está bloqueada

### Relé não aciona:

- Verifique conexão no GPIO 19
- Relé precisa de alimentação 5V separada
- LED do relé deve acender quando ativo

## Melhorias Futuras

- Dashboard web para visualização de gráficos
- Integração com Google Assistant/Alexa
- Sensor de peso no copo (detecção automática)
- Bateria backup para funcionamento sem energia
- App móvel nativo Android/iOS
- Machine Learning para otimizar lembretes

## Autor

**Ivo Luis Ribeiro de Oliveira**

- Universidade Presbiteriana Mackenzie
- Faculdade de Computação e Informática
- Email: 10316483@mackenzista.com.br

## Licença

Este projeto é de código aberto sob licença MIT.
Projeto acadêmico desenvolvido para a disciplina de IoT.

## Agradecimentos

- Prof. Leandro Carlos Fernandes
- Prof. Andre Luis de Oliveira
- Universidade Presbiteriana Mackenzie

## Referências

- ODS 3 - ONU: https://odsbrasil.gov.br/objetivo/objetivo?n=3
- ESP32 Datasheet: https://www.espressif.com
- MQTT Protocol: http://mqtt.org
- Wokwi Simulator: https://wokwi.com

---

**Desenvolvido com 💙 para promover saúde e bem-estar através da tecnologia IoT**
