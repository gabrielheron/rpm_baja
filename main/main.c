// Include Padrão
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Include Sistema Operacional
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Include Driver 
#include "driver/gpio.h"

// GPIOs do Sensor Indutivo
#define SENSOR_INPUT_PIN GPIO_NUM_15

// TAG para depuração
static const char *TAG = "rpm_engine";

// Variáveis para a obtenção da velocidade angular do motor
volatile uint32_t pulsos = 0;

// 
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

// Força o armazenamento da variável na memória IRAM e não na Flash
void IRAM_ATTR rpm_conta_pulsos(void *args){

    portENTER_CRITICAL_ISR(&mux);
    pulsos++;
    portEXIT_CRITICAL_ISR(&mux);
}

// Iniciação da GPIO do sensor indutivo
void init_rpm_gpio(){

    // Configuração comum as GPIOs
    gpio_config_t rpm_gpio_config = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT, // Entrada de sinal
        .pin_bit_mask = (1 << SENSOR_INPUT_PIN),
        .pull_up_en = GPIO_PULLUP_ENABLE, // GPIO_PULLUP_ENABLE = 1
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // GPIO_PULDOWN_DISABLE = 0
    };
    gpio_config(&rpm_gpio_config);

    // 
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    gpio_isr_handler_add(SENSOR_INPUT_PIN, rpm_conta_pulsos, NULL);

}

void rpm_task(){

    //ESP_LOGI(TAG, "A obtencao do rpm do motor esta sendo ativado..."); // depuração

    while(1){

        uint32_t count;

        portENTER_CRITICAL(&mux);
        count = pulsos;
        pulsos = 0;
        portEXIT_CRITICAL(&mux);
        
        float rpm = (count*60.0);
        printf("Rpm: %.2f\n", rpm);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void){

    init_rpm_gpio();

    xTaskCreate(rpm_task, "rpm_task", 2048, NULL, 10, NULL);
}
