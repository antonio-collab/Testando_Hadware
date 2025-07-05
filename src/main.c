
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"

// --- Definições de pinos ---
#define LED_R 13
#define LED_G 11
#define LED_B 12

#define BUZZER_PIN_SELFTEST 10
#define BUZZER_ALARM_PIN 21

#define BUTTON_A 5
#define BUTTON_B 6
#define JOY_SW   22

#define JOY_X 27  // ADC1
#define JOY_Y 26  // ADC0
#define MIC_PIN 28 // ADC2

#define ADC_REF_VOLTAGE 3.3f
#define ADC_MAX 4095.0f

// --- Funções auxiliares ---
void start_buzzer_pwm(uint gpio, int freq_hz, int duty_percent) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 125.0f); // 1 MHz clock
    pwm_config_set_wrap(&config, 1000000 / freq_hz);
    pwm_init(slice, &config, true);
    pwm_set_gpio_level(gpio, (1000000 / freq_hz) * duty_percent / 100);
}

void stop_buzzer_pwm(uint gpio) {
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_gpio_level(gpio, 0);
    pwm_set_enabled(slice, false);
    gpio_set_function(gpio, GPIO_FUNC_SIO);
    gpio_put(gpio, 0);
    gpio_set_dir(gpio, GPIO_OUT);
}

// --- Tarefa 1: Self Test ---
void self_test(void *param){
    // Inicializar GPIOs
    gpio_init(LED_R); gpio_set_dir(LED_R, GPIO_OUT);
    gpio_init(LED_G); gpio_set_dir(LED_G, GPIO_OUT);
    gpio_init(LED_B); gpio_set_dir(LED_B, GPIO_OUT);
    gpio_init(BUZZER_PIN_SELFTEST); gpio_set_dir(BUZZER_PIN_SELFTEST, GPIO_OUT);
    gpio_init(BUTTON_A); gpio_set_dir(BUTTON_A, GPIO_IN); gpio_pull_up(BUTTON_A);
    gpio_init(BUTTON_B); gpio_set_dir(BUTTON_B, GPIO_IN); gpio_pull_up(BUTTON_B);
    gpio_init(JOY_SW);   gpio_set_dir(JOY_SW, GPIO_IN);  gpio_pull_up(JOY_SW);

    adc_init();
    adc_gpio_init(JOY_X);  // ADC1
    adc_gpio_init(JOY_Y);  // ADC0
    adc_gpio_init(MIC_PIN);// ADC2

    stdio_init_all();
    vTaskDelay(pdMS_TO_TICKS(2000));

    printf("==== Iniciando Testes de Hardware ====\n");

    // Teste de LEDs
    printf("Testando LEDs RGB...\n");
    gpio_put(LED_R, 1); vTaskDelay(pdMS_TO_TICKS(500)); gpio_put(LED_R, 0);
    gpio_put(LED_G, 1); vTaskDelay(pdMS_TO_TICKS(500)); gpio_put(LED_G, 0);
    gpio_put(LED_B, 1); vTaskDelay(pdMS_TO_TICKS(500)); gpio_put(LED_B, 0);

    // Testar Buzzer
    printf("Testando buzzer...\n");
    for (int i = 0; i < 2; i++) {
        start_buzzer_pwm(BUZZER_PIN_SELFTEST, 1000, 50);
        sleep_ms(200);
        stop_buzzer_pwm(BUZZER_PIN_SELFTEST);
        sleep_ms(200);
    }

    // Testar Botões
    printf("Botão A: %s\n", gpio_get(BUTTON_A) ? "Solto" : "Pressionado");
    printf("Botão B: %s\n", gpio_get(BUTTON_B) ? "Solto" : "Pressionado");
    printf("Joystick SW: %s\n", gpio_get(JOY_SW) ? "Solto" : "Pressionado");

    // Testar Joystick Analógico
    printf("Lendo Joystick analógico...\n");
    adc_select_input(1); uint16_t x = adc_read(); // X = ADC1
    adc_select_input(0); uint16_t y = adc_read(); // Y = ADC0
    printf("Joystick X: %d, Y: %d\n", x, y);

    // Testar Microfone
    printf("Lendo microfone...\n");
    adc_select_input(2); uint16_t mic = adc_read();
    printf("Microfone ADC: %d\n", mic);

    printf("=== Testes concluídos ===\n");

    // Auto-deleta
    vTaskDelete(NULL);
}

// --- Tarefa 2: Alive Blink ---
void alive_task(void *param) {
    gpio_init(LED_R); gpio_set_dir(LED_R, GPIO_OUT);

    while (true) {
        gpio_put(LED_R, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_put(LED_R, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// --- Tarefa 3: Joystick Monitor + Alarme ---
void joystick_monitor_task(void *param) {
    gpio_init(BUZZER_ALARM_PIN); gpio_set_dir(BUZZER_ALARM_PIN, GPIO_OUT);
    adc_init();
    adc_gpio_init(JOY_X);
    adc_gpio_init(JOY_Y);

    while (true) {
        adc_select_input(1); // X = ADC1
        uint16_t raw_x = adc_read();
        adc_select_input(0); // Y = ADC0
        uint16_t raw_y = adc_read();

        float x_volt = (raw_x * ADC_REF_VOLTAGE) / ADC_MAX;
        float y_volt = (raw_y * ADC_REF_VOLTAGE) / ADC_MAX;

        printf("Joystick - X: %.2f V, Y: %.2f V\n", x_volt, y_volt);

        if (x_volt > 3.0 || y_volt > 3.0) {
            start_buzzer_pwm(BUZZER_ALARM_PIN, 2000, 50);
        } else {
            stop_buzzer_pwm(BUZZER_ALARM_PIN);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// --- main ---
int main() {
    stdio_init_all();
    sleep_ms(2000);

    // Criar tarefas
    xTaskCreate(self_test, "SelfTest", 1024, NULL, 2, NULL);
    xTaskCreate(alive_task, "Alive", 512, NULL, 1, NULL);
    xTaskCreate(joystick_monitor_task, "JoystickMonitor", 1024, NULL, 1, NULL);

    // Iniciar escalonador
    vTaskStartScheduler();

    // Nunca deve chegar aqui
    while (true) {
        tight_loop_contents();
    }
}
