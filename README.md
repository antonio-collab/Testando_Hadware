Aqui está um modelo de **README.md** que você pode usar para documentar o projeto:

---

# 🛠️ Raspberry Pi Pico - FreeRTOS Hardware

Este projeto implementa uma rotina de autodiagnóstico de hardware usando o **Raspberry Pi Pico** com **FreeRTOS**. O código realiza testes automáticos em periféricos como LEDs RGB, buzzer, botões, joystick analógico e microfone, além de tarefas em tempo real para monitoramento contínuo.

---

## 📋 Funcionalidades

### ✅ Tarefa 1: `self_test`

* Executada uma única vez ao iniciar.
* Testa:

  * LEDs RGB (vermelho, verde, azul)
  * Buzzer (PWM em `GPIO 10`)
  * Botões A (`GPIO 5`), B (`GPIO 6`) e SW do joystick (`GPIO 22`)
  * Leitura analógica do joystick (X = `ADC1` / GPIO 27, Y = `ADC0` / GPIO 26)
  * Microfone (ADC2 = GPIO 28)
* Exibe os resultados na porta serial USB.
* Auto-deleta após a execução (`vTaskDelete(NULL)`).

### 🔴 Tarefa 2: `alive_task`

* Pisca o LED vermelho (`GPIO 13`) a cada 1 segundo (500ms ligado, 500ms desligado).
* Indica que o sistema está funcionando.

### 🎮 Tarefa 3: `joystick_monitor_task`

* Lê continuamente as tensões dos eixos X e Y do joystick.
* Imprime os valores na porta USB a cada 50ms.
* Se qualquer eixo ultrapassar **3.00V**, ativa um buzzer de alarme (`GPIO 21`).
* Desativa o buzzer quando os valores voltarem ao normal.

---

## 📦 Requisitos

* Placa **Raspberry Pi Pico**
* SDK do Raspberry Pi Pico com suporte a **FreeRTOS**
* Ferramentas de compilação CMake
* Terminal serial (ex: `minicom`, `putty`, `screen` ou integrado ao VSCode)

---

## 📌 Mapeamento de Pinos

| Componente      | GPIO | Função         |
| --------------- | ---- | -------------- |
| LED Vermelho    | 13   | Alive Task     |
| LED Verde       | 11   | Self-Test      |
| LED Azul        | 12   | Self-Test      |
| Buzzer (teste)  | 10   | Self-Test      |
| Buzzer (alarme) | 21   | Joystick Alarm |
| Botão A         | 5    | Entrada        |
| Botão B         | 6    | Entrada        |
| Joystick SW     | 22   | Entrada        |
| Joystick X      | 27   | ADC1           |
| Joystick Y      | 26   | ADC0           |
| Microfone       | 28   | ADC2           |

---

## 🚀 Compilação e Upload

1. Clone o repositório e configure seu ambiente com o SDK do Pico.
2. Compile com CMake:

```bash
mkdir build && cd build
cmake ..
make
```

3. Com o Raspberry Pi Pico em modo bootloader, copie o arquivo `.uf2` gerado para a unidade USB do Pico.

---

## 📈 Saída Esperada no Terminal Serial

```text
==== Iniciando Testes de Hardware ====
Testando LEDs RGB...
Testando buzzer...
Botão A: Solto
Botão B: Pressionado
Joystick SW: Solto
Joystick X: 2023, Y: 1995
Lendo microfone...
Microfone ADC: 789
=== Testes concluídos ===
Joystick - X: 1.98 V, Y: 2.00 V
Joystick - X: 1.99 V, Y: 2.01 V
...
```

---

## 🧠 Observações

* O buzzer de alarme (GPIO 21) é controlado via **PWM**.
* A porta serial precisa estar aberta após o upload para que a saída `printf` seja visível.
* O `stdio_init_all()` e um `sleep_ms(2000)` garantem tempo para estabilizar a conexão USB.

---

## 🧩 Expansões possíveis

* Integração com sensores adicionais (temperatura, luz, etc).
* Comunicação com outro microcontrolador via UART.
* Interface gráfica com monitoramento em tempo real via PC.

---

## 📄 Licença

Código de exemplo para fins educacionais — sem garantias. Você pode adaptar livremente.

---
