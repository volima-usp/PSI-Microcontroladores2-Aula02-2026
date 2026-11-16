#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

// Registra o módulo de log
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

// --- Configuração de LEDs via DeviceTree ---
#define LED_A_NODE DT_ALIAS(led0)  // LED verde
#define LED_B_NODE DT_ALIAS(led2)  // LED vermelho

static const struct gpio_dt_spec ledA = GPIO_DT_SPEC_GET(LED_A_NODE, gpios);
static const struct gpio_dt_spec ledB = GPIO_DT_SPEC_GET(LED_B_NODE, gpios);

// --- Prioridades e tempos ---
#define PRIO_THREAD_A 5   // Verde - Maior prioridade (número menor) 
#define PRIO_THREAD_B 7   // Vermelho - Menor prioridade
#define TEMPO_A_MS   1500   // Thread A dorme
#define TEMPO_B_MS   1000   // Thread B dorme

// ----------------------------------------------------
// THREAD A — Tarefa curta, alta prioridade
// ----------------------------------------------------
void thread_A(void *p1, void *p2, void *p3)
{
    uint64_t start_ms1, end_ms1, mid_ms1;
    
    while (1) {
        start_ms1 = k_uptime_get();
        gpio_pin_set_dt(&ledA, 1);  // Liga LED verde
        
        LOG_DBG("Thread A: LED Verde LIGADO (Ti: %llu ms)", start_ms1);

        // Simula processamento rápido 
        for (volatile int i = 0; i < 200000; i++) {
            if (i == 100000) {
                mid_ms1 = k_uptime_get();
                LOG_DBG("Thread A: Processamento em 50%% (T_mid: %llu ms)", mid_ms1);
            }
        }

        gpio_pin_set_dt(&ledA, 0);  // Desliga LED verde
        end_ms1 = k_uptime_get();
        
        LOG_INF("Thread A Executada | Ti: %llu | Tf: %llu | Delta: %llu ms", 
                start_ms1, end_ms1, (end_ms1 - start_ms1));
                
        k_msleep(TEMPO_A_MS);       // Dorme — libera CPU
    }
}

// ----------------------------------------------------
// THREAD B — Tarefa longa, baixa prioridade
// ----------------------------------------------------
void thread_B(void *p1, void *p2, void *p3)
{
    uint64_t start_ms, end_ms, loop_ms;
    
    while (1) {
        start_ms = k_uptime_get();
        gpio_pin_set_dt(&ledB, 1);  // Liga LED vermelho
        
        LOG_DBG("Thread B: LED Vermelho LIGADO (Ti: %llu ms)", start_ms);

        // Simula processamento mais longo 
        for (volatile int i = 0; i < 10000000; i++) {
            if (i % 2500000 == 0 && i != 0) { // Loga a cada 25%
                loop_ms = k_uptime_get();
                LOG_DBG("Thread B: Processamento alcançou %d (T_loop: %llu ms)", i, loop_ms);
            }
        }
        
        gpio_pin_set_dt(&ledB, 0);  // Desliga LED vermelho
        end_ms = k_uptime_get();
        
        LOG_INF("Thread B Executada | Ti: %llu | Tf: %llu | Delta: %llu ms", 
                start_ms, end_ms, (end_ms - start_ms));
                
        k_msleep(TEMPO_B_MS);       // Dorme um pouco
    }
}

// ----------------------------------------------------
// Definição das threads
// ----------------------------------------------------
K_THREAD_DEFINE(a_tid, 1024, thread_A, NULL, NULL, NULL,
                PRIO_THREAD_A, 0, 0);
K_THREAD_DEFINE(b_tid, 1024, thread_B, NULL, NULL, NULL,
                PRIO_THREAD_B, 0, 0);
                
// Aumentei o tamanho da pilha das threads para 1024 bytes, 
// pois o sistema de logging pode consumir mais espaço na pilha do que o printk.

// ----------------------------------------------------
// Função principal
// ----------------------------------------------------
void main(void)
{
    // Inicializa GPIOs dos LEDs
    if (!device_is_ready(ledA.port) || !device_is_ready(ledB.port)) {
        LOG_ERR("Erro: Dispositivo GPIO não está pronto!");
        return;
    }
    gpio_pin_configure_dt(&ledA, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&ledB, GPIO_OUTPUT_INACTIVE);

    LOG_INF("=== Demonstração de Preempção com LEDs e Logging ===");
    LOG_INF("Thread A (Verde): prioridade %d", PRIO_THREAD_A);
    LOG_INF("Thread B (Vermelho): prioridade %d", PRIO_THREAD_B);

    while (1) {
        k_sleep(K_FOREVER); // Main dorme para liberar CPU.
    }
}