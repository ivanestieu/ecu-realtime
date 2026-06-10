#include "parser.h"

/**
 * Initialiser le parser (machine à états)
 */
void parser_init(parser_ctx_t *ctx) {
    ctx->state = STATE_IDLE;
    ctx->frame_len = 0;
    ctx->payload_idx = 0;
    ctx->crc_calc = 0;
    ctx->type = 0;
}

/**
 * Calculer le CRC (XOR de tous les octets sauf le 0xAA de début)
 */
uint8_t parser_compute_crc(uint8_t *data, uint8_t len) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
    }
    return crc;
}

/**
 * Machine à états du parser
 * Retourne true si une trame complète et valide a été reçue
 * 
 * Format d'une trame (LITTLE-ENDIAN) :
 * [0xAA] [LEN_L] [LEN_H] [TYPE] [PAYLOAD...] [CRC]
 *  1oct    1oct    1oct   1oct    N octets   1oct
 */
bool parser_push_byte(uint8_t byte, parser_ctx_t *ctx, frame_t *out) {
    switch (ctx->state) {
        // ========== STATE_IDLE ==========
        // En attente du marqueur de début (0xAA)
        case STATE_IDLE:
            if (byte == FRAME_START) {
                parser_init(ctx);  // Réinitialiser la machine
                ctx->state = STATE_LEN_LOW;  // Lire d'abord le poids faible
            }
            return false;  // Pas de trame complète

        // ========== STATE_LEN_LOW ==========
        // Lire l'octet bas de la longueur (poids faible en premier)
        case STATE_LEN_LOW:
            ctx->frame_len = (uint16_t)byte;  // Poids faible
            ctx->crc_calc = byte;  // Commencer à accumuler le CRC
            ctx->state = STATE_LEN_HIGH;
            return false;

        // ========== STATE_LEN_HIGH ==========
        // Lire l'octet haut de la longueur (poids fort en second)
        case STATE_LEN_HIGH:
            ctx->frame_len |= (uint16_t)byte << 8;  // Ajouter le poids fort
            ctx->crc_calc ^= byte;  // Accumuler le CRC
            
            // Vérifier que la longueur est valide
            // Format : 1 (type) + N (payload) = frame_len
            if (ctx->frame_len == 0 || ctx->frame_len > (1 + MAX_PAYLOAD)) {
                ctx->state = STATE_IDLE;
                return false;
            }
            
            ctx->state = STATE_TYPE;
            return false;

        // ========== STATE_LEN_LOW ==========
        // Lire l'octet bas de la longueur
        // ========== STATE_TYPE ==========
        // Lire le type de message
        case STATE_TYPE:
            ctx->type = byte;
            ctx->crc_calc ^= byte;  // Accumuler le CRC
            ctx->payload_idx = 0;
            
            // Si frame_len == 1, il n'y a pas de payload, aller directement au CRC
            if (ctx->frame_len == 1) {
                ctx->state = STATE_CRC;
            } else {
                ctx->state = STATE_PAYLOAD;
            }
            return false;

        // ========== STATE_PAYLOAD ==========
        // Lire les données du payload
        case STATE_PAYLOAD:
            if (ctx->payload_idx < MAX_PAYLOAD) {
                out->payload[ctx->payload_idx] = byte;
            }
            ctx->crc_calc ^= byte;  // Accumuler le CRC
            ctx->payload_idx++;
            
            // Vérifier si on a reçu tout le payload
            // Longueur du payload = frame_len - 1 (le type)
            if (ctx->payload_idx >= (ctx->frame_len - 1)) {
                ctx->state = STATE_CRC;
            }
            return false;

        // ========== STATE_CRC ==========
        // Lire et vérifier le CRC
        case STATE_CRC: {
            uint8_t crc_received = byte;
            
            // Remplir la trame de sortie
            out->type = ctx->type;
            out->len = ctx->payload_idx;
            
            // Vérifier le CRC
            if (crc_received == ctx->crc_calc) {
                // ✅ Trame valide !
                ctx->state = STATE_IDLE;
                return true;
            } else {
                // ❌ CRC invalide, rejeter
                ctx->state = STATE_IDLE;
                return false;
            }
        }

        default:
            ctx->state = STATE_IDLE;
            return false;
    }
}
