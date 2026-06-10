#include "builder.h"

/**
 * Convertir float en 4 octets (big-endian IEEE 754)
 */
void float_to_bytes(float f, uint8_t *buf) {
        memcpy(buf, &f, sizeof(float));  // le CPU est déjà little endian
}

/**
 * Convertir 4 octets en float
 */
float bytes_to_float(uint8_t *buf) {
    float f;
    memcpy(&f, buf, sizeof(float));
    return f;
}

/**
 * Construire le préambule d'une trame et retourner l'index du premier payload
 * @param buf      Buffer de sortie
 * @param type     Type de message
 * @param payload_len  Longueur du payload (sans le type)
 * @return         Index du premier octet de payload dans buf
 */
static int build_header(uint8_t *buf, uint8_t type, uint16_t payload_len) {
    uint16_t frame_len = 1 + payload_len;  // type + payload
    
    buf[0] = FRAME_START;
    buf[1] = frame_len & 0xFF;          // poids faible en premier
    buf[2] = (frame_len >> 8) & 0xFF;  // poids fort en second
    buf[3] = type;
    
    return 4;  // index du payload
}

/**
 * Terminer une trame : calculer et ajouter le CRC
 * @param buf      Buffer contenant la trame
 * @param len      Longueur actuelle (sans CRC)
 * @return         Longueur totale avec CRC
 */
static int finalize_frame(uint8_t *buf, int len) {
    // CRC = XOR de tous les octets sauf le 0xAA de début
    uint8_t crc = parser_compute_crc(&buf[1], len - 1);
    buf[len] = crc;
    return len + 1;
}

/**
 * OUTPUT : commande moteur (float)
 * Payload : 4 octets
 */
int builder_output(uint8_t *buf, float command) {
    int idx = build_header(buf, MSG_OUTPUT, 4);
    float_to_bytes(command, &buf[idx]);
    return finalize_frame(buf, idx + 4);
}

/**
 * STATS : télémétrie (4x uint32)
 * Payload : 16 octets (4 compteurs de 4 octets chacun)
 */
int builder_stats(uint8_t *buf, uint32_t *stats) {
    int idx = build_header(buf, MSG_STATS, 16);
    
    for (int i = 0; i < 4; i++) {
        uint32_t val = stats[i];
        buf[idx + 0] = (val >> 24) & 0xFF;
        buf[idx + 1] = (val >> 16) & 0xFF;
        buf[idx + 2] = (val >> 8) & 0xFF;
        buf[idx + 3] = val & 0xFF;
        idx += 4;
    }
    
    return finalize_frame(buf, idx);
}

/**
 * ALARM : message d'erreur (string)
 * Payload : string (max 60 octets pour respecter le max_payload)
 */
int builder_alarm(uint8_t *buf, const char *msg) {
    int len = strlen(msg);
    if (len > 60) len = 60;  // Limiter à 60 octets
    
    int idx = build_header(buf, MSG_ALARM, len);
    memcpy(&buf[idx], msg, len);
    
    return finalize_frame(buf, idx + len);
}

/**
 * DEBUG : message de debug (string)
 * Payload : string (max 60 octets)
 */
int builder_debug(uint8_t *buf, const char *msg) {
    int len = strlen(msg);
    if (len > 60) len = 60;
    
    int idx = build_header(buf, MSG_DBG, len);
    memcpy(&buf[idx], msg, len);
    
    return finalize_frame(buf, idx + len);
}
