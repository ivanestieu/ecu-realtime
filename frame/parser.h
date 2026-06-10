#pragma once
#include <stdint.h>
#include <stdbool.h>

#define MAX_PAYLOAD 64
#define FRAME_START 0xAA

// États du parser (machine à états)
typedef enum {
    STATE_IDLE,         // En attente du 0xAA de début
    STATE_LEN_LOW,      // Lire octet bas de la longueur (poids faible d'abord!)
    STATE_LEN_HIGH,     // Lire octet haut de la longueur (poids fort en second)
    STATE_TYPE,         // Lire le type de message
    STATE_PAYLOAD,      // Lire le payload
    STATE_CRC,          // Lire et vérifier le CRC
} parser_state_t;

// Types de messages
typedef enum {
    MSG_SETPOINT  = 0x01,
    MSG_SPEED     = 0x02,
    MSG_MODE_SET  = 0x05,
    MSG_OUTPUT    = 0x80,
    MSG_STATS     = 0x83,
    MSG_ALARM     = 0x85,
    MSG_DBG       = 0xFF,
} msg_type_t;

// Modes d'opération
typedef enum {
    MODE_OFF    = 0x00,
    MODE_MANUAL = 0x01,
    MODE_AUTO   = 0x02,
} ecu_mode_t;

// Structure d'une trame décodée
typedef struct {
    uint8_t type;
    uint8_t payload[MAX_PAYLOAD];
    uint8_t len;
} frame_t;

// Structure interne du parser (état de la machine)
typedef struct {
    parser_state_t state;
    uint16_t frame_len;     // Longueur attendue
    uint8_t payload_idx;    // Index courant dans le payload
    uint8_t crc_calc;       // CRC calculé au fur et à mesure
    uint8_t type;           // Type du message
} parser_ctx_t;

// API publique
void    parser_init(parser_ctx_t *ctx);
bool    parser_push_byte(uint8_t byte, parser_ctx_t *ctx, frame_t *out);
uint8_t parser_compute_crc(uint8_t *data, uint8_t len);

