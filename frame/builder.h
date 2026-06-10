#pragma once
#include <stdint.h>
#include <string.h>
#include "parser.h"

/**
 * Builder de trames de transmission
 * 
 * Format d'une trame :
 * [0xAA] [LEN_H] [LEN_L] [TYPE] [PAYLOAD...] [CRC]
 */

/**
 * Construire une trame OUTPUT (commande moteur)
 * @param buf      Buffer de sortie
 * @param command  Valeur 0-255 de la commande moteur
 * @return         Nombre d'octets écrits
 */
int builder_output(uint8_t *buf, float command);

/**
 * Construire une trame STATS (télémétrie)
 * @param buf      Buffer de sortie
 * @param stats    Pointeur sur tableau de 4 uint32_t [valid, corrupted, dropped, output_count]
 * @return         Nombre d'octets écrits
 */
int builder_stats(uint8_t *buf, uint32_t *stats);

/**
 * Construire une trame ALARM (alerte)
 * @param buf      Buffer de sortie
 * @param msg      Message d'erreur (string)
 * @return         Nombre d'octets écrits
 */
int builder_alarm(uint8_t *buf, const char *msg);

/**
 * Construire une trame DEBUG
 * @param buf      Buffer de sortie
 * @param msg      Message debug
 * @return         Nombre d'octets écrits
 */
int builder_debug(uint8_t *buf, const char *msg);

/**
 * Utilitaire : convertir float en 4 octets (big-endian)
 */
void float_to_bytes(float f, uint8_t *buf);

/**
 * Utilitaire : convertir 4 octets en float
 */
float bytes_to_float(uint8_t *buf);
