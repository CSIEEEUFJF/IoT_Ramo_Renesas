#include "blinky_thread.h"
#include "bsp_api.h"
#include "tx_api.h"
#include <string.h> // Para memcmp e memcpy

/* --- MAPEAMENTO P3 (Seu Hardware Atual) --- */
#define RELAY_PORTA_PIN    IOPORT_PORT_03_PIN_00
#define RELAY_LAMPADA_PIN  IOPORT_PORT_03_PIN_01
#define LCD_RS_PIN         IOPORT_PORT_03_PIN_02
#define LCD_EN_PIN         IOPORT_PORT_03_PIN_03
#define LCD_D4_PIN         IOPORT_PORT_03_PIN_04
#define LCD_D5_PIN         IOPORT_PORT_03_PIN_05
#define LCD_D6_PIN         IOPORT_PORT_03_PIN_06
#define LCD_D7_PIN         IOPORT_PORT_03_PIN_07

/* --- ENTRADAS --- */
#define BTN_PORTA_PIN      IOPORT_PORT_00_PIN_05 // S1
#define BTN_LAMPADA_PIN    IOPORT_PORT_00_PIN_06 // S2 (Segurar para cadastrar)

/* --- DATABASE --- */
#define MAX_TAGS 5
// IDs de exemplo (4 bytes cada). O primeiro já vem cadastrado (Ex: Master)
uint8_t authorized_tags[MAX_TAGS][4] = {
    {0xDE, 0xAD, 0xBE, 0xEF}, // Tag Mestra (Exemplo)
    {0x00, 0x00, 0x00, 0x00}, // Vazio
    {0x00, 0x00, 0x00, 0x00}, // Vazio
    {0x00, 0x00, 0x00, 0x00}, // Vazio
    {0x00, 0x00, 0x00, 0x00}  // Vazio
};
int tags_count = 1;

/* --- FUNÇÕES AUXILIARES (Static para evitar warnings) --- */
static void force_pin_output(ioport_port_pin_t pin) {
    g_ioport.p_api->pinCfg(pin, IOPORT_CFG_PORT_DIRECTION_OUTPUT);
}

static void lcd_pulse(void) {
    g_ioport.p_api->pinWrite(LCD_EN_PIN, IOPORT_LEVEL_LOW);
    tx_thread_sleep(1);
    g_ioport.p_api->pinWrite(LCD_EN_PIN, IOPORT_LEVEL_HIGH);
    tx_thread_sleep(1);
    g_ioport.p_api->pinWrite(LCD_EN_PIN, IOPORT_LEVEL_LOW);
    tx_thread_sleep(1);
}

static void lcd_send_4bits(uint8_t val) {
    g_ioport.p_api->pinWrite(LCD_D4_PIN, (val >> 0) & 1);
    g_ioport.p_api->pinWrite(LCD_D5_PIN, (val >> 1) & 1);
    g_ioport.p_api->pinWrite(LCD_D6_PIN, (val >> 2) & 1);
    g_ioport.p_api->pinWrite(LCD_D7_PIN, (val >> 3) & 1);
    lcd_pulse();
}

static void lcd_send(uint8_t val, uint8_t mode) {
    g_ioport.p_api->pinWrite(LCD_RS_PIN, mode ? IOPORT_LEVEL_HIGH : IOPORT_LEVEL_LOW);
    lcd_send_4bits(val >> 4);
    lcd_send_4bits(val & 0x0F);
}

static void lcd_print(char *s) { while(*s) lcd_send((uint8_t)*s++, 1); }

static void lcd_clear(void) { lcd_send(0x01, 0); tx_thread_sleep(2); }

static void lcd_init_forced(void) {
    force_pin_output(RELAY_PORTA_PIN); force_pin_output(RELAY_LAMPADA_PIN);
    force_pin_output(LCD_RS_PIN); force_pin_output(LCD_EN_PIN);
    force_pin_output(LCD_D4_PIN); force_pin_output(LCD_D5_PIN);
    force_pin_output(LCD_D6_PIN); force_pin_output(LCD_D7_PIN);
    tx_thread_sleep(10);
    g_ioport.p_api->pinWrite(LCD_RS_PIN, IOPORT_LEVEL_LOW);
    g_ioport.p_api->pinWrite(LCD_EN_PIN, IOPORT_LEVEL_LOW);
    lcd_send_4bits(0x03); tx_thread_sleep(1);
    lcd_send_4bits(0x03); tx_thread_sleep(1);
    lcd_send_4bits(0x03); tx_thread_sleep(1);
    lcd_send_4bits(0x02);
    lcd_send(0x28, 0); lcd_send(0x0C, 0); lcd_send(0x06, 0);
    lcd_clear();
}

/* --- LÓGICA DE NEGÓCIO --- */
static void acionar_porta(void) {
    lcd_clear(); lcd_print("ACESSO LIBERADO");
    g_ioport.p_api->pinWrite(RELAY_PORTA_PIN, IOPORT_LEVEL_LOW);
    tx_thread_sleep(300); // 3 segundos
    g_ioport.p_api->pinWrite(RELAY_PORTA_PIN, IOPORT_LEVEL_HIGH);
    lcd_clear(); lcd_print("APROXIME TAG");
}

static void cadastrar_nova_tag(uint8_t *new_id) {
    if (tags_count >= MAX_TAGS) {
        lcd_clear(); lcd_print("MEMORIA CHEIA!");
        tx_thread_sleep(200);
        return;
    }
    // Verifica se já existe
    for(int i=0; i<tags_count; i++) {
        if(memcmp(authorized_tags[i], new_id, 4) == 0) {
            lcd_clear(); lcd_print("JA CADASTRADO");
            tx_thread_sleep(200);
            return;
        }
    }
    // Salva
    memcpy(authorized_tags[tags_count], new_id, 4);
    tags_count++;
    lcd_clear(); lcd_print("TAG SALVA!");
    tx_thread_sleep(200);
}

/* --- SIMULAÇÃO DE LEITURA RFID (MOCK) --- */
// Substitua isso pela leitura real do g_sf_spi0 quando estiver pronto
bool check_rfid_mock(uint8_t *id_buffer) {
    // AQUI VOCÊ COLOCARIA A LEITURA REAL DO RC522
    // Por enquanto, retorna falso para não travar
    return false;
}

/* --- THREAD PRINCIPAL --- */
void blinky_thread_entry(void) {
    lcd_init_forced();
    g_ioport.p_api->pinWrite(RELAY_PORTA_PIN, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite(RELAY_LAMPADA_PIN, IOPORT_LEVEL_HIGH);

    // Se o driver SPI estiver OK e sem erros, pode descomentar:
    // g_sf_spi0.p_api->open(g_sf_spi0.p_ctrl, g_sf_spi0.p_cfg);

    lcd_print("SISTEMA PRONTO");
    tx_thread_sleep(100);
    lcd_clear(); lcd_print("APROXIME TAG");

    bool lamp = false;
    ioport_level_t b_porta, b_lamp;
    int s2_hold_counter = 0;
    uint8_t current_id[4];

    while (1) {
        g_ioport.p_api->pinRead(BTN_PORTA_PIN, &b_porta);
        g_ioport.p_api->pinRead(BTN_LAMPADA_PIN, &b_lamp);

        // 1. Lógica do Botão S1 (Porta ou Simulação de Tag Válida)
        if(b_porta == IOPORT_LEVEL_LOW) {
            // Simula passar uma tag válida
            acionar_porta();
            while(b_porta == IOPORT_LEVEL_LOW) g_ioport.p_api->pinRead(BTN_PORTA_PIN, &b_porta); // Debounce
        }

        // 2. Lógica do Botão S2 (Lâmpada ou Entrar em Cadastro)
        if(b_lamp == IOPORT_LEVEL_LOW) {
            s2_hold_counter++;
            // Se segurar por 3 segundos (300 * 10ms = 3000ms)
            if (s2_hold_counter > 300) {
                lcd_clear(); lcd_print("MODO CADASTRO...");

                // Espera soltar o botão
                while(b_lamp == IOPORT_LEVEL_LOW) g_ioport.p_api->pinRead(BTN_LAMPADA_PIN, &b_lamp);

                // Loop de Cadastro (Espera 10 segundos por uma TAG)
                int timeout = 1000;
                bool cadastrou = false;
                while(timeout > 0) {
                    // Simulação: Se apertar S1 agora, finge que leu uma TAG nova
                    g_ioport.p_api->pinRead(BTN_PORTA_PIN, &b_porta);
                    if(b_porta == IOPORT_LEVEL_LOW) {
                        uint8_t dummy_tag[] = {0xAA, 0xBB, 0xCC, 0xDD}; // ID Fictício
                        cadastrar_nova_tag(dummy_tag);
                        cadastrou = true;
                        break;
                    }
                    // Aqui entraria: if (check_rfid_mock(current_id)) cadastrar...

                    tx_thread_sleep(1);
                    timeout--;
                }

                if (!cadastrou) {
                    lcd_clear(); lcd_print("TIMEOUT!");
                    tx_thread_sleep(100);
                }

                lcd_clear(); lcd_print("APROXIME TAG");
                s2_hold_counter = 0;
            }
        } else {
            // Se soltar rápido, é só lâmpada
            if (s2_hold_counter > 0 && s2_hold_counter < 50) {
                lamp = !lamp;
                g_ioport.p_api->pinWrite(RELAY_LAMPADA_PIN, lamp ? IOPORT_LEVEL_HIGH : IOPORT_LEVEL_LOW);
            }
            s2_hold_counter = 0;
        }

        tx_thread_sleep(1); // 10ms tick
    }
}
