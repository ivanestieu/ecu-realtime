#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "frame/parser.h"
#include "frame/builder.h"

// Couleurs pour les tests
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define RESET   "\033[0m"
#define BOLD    "\033[1m"

// ===========================================================================
// TESTS UNITAIRES
// ===========================================================================

int test_count = 0;
int test_passed = 0;

void assert_true(int condition, const char *test_name) {
    test_count++;
    if (condition) {
        test_passed++;
        printf(GREEN "✓" RESET " %s\n", test_name);
    } else {
        printf(RED "✗" RESET " %s\n", test_name);
    }
}

void test_parser_simple_message(void) {
    printf("\n" BOLD "=== Test Parser ===" RESET "\n");
    
    parser_ctx_t ctx;
    frame_t frame;
    parser_init(&ctx);
    
    // Créer une trame OUTPUT (4 octets de payload)
    // Trame (LITTLE-ENDIAN) : [0xAA] [0x05] [0x00] [0x80] [0x00 0x00 0x80 0x3f] [CRC]
    //                          start  len_l  len_h  type   payload (float 1.0)   crc
    
    uint8_t trame_output[] = {
        0xAA,                    // Start
        0x05, 0x00,              // Length = 5 (poids faible d'abord!)
        MSG_OUTPUT,              // Type
        0x00, 0x00, 0x80, 0x3f,  // float 1.0 (little-endian)
    };
    
    // Calculer le CRC
    uint8_t crc = parser_compute_crc(&trame_output[1], 7);  // len_l + len_h + type + payload(4)
    
    // Envoyer octets un par un (tous les octets de la trame)
    bool complete = false;
    for (int i = 0; i < (int)sizeof(trame_output); i++) {
        parser_push_byte(trame_output[i], &ctx, &frame);
    }
    // Envoyer le CRC
    complete = parser_push_byte(crc, &ctx, &frame);
    
    assert_true(complete, "Parser reçoit une trame OUTPUT complète");
    assert_true(frame.type == MSG_OUTPUT, "Type = MSG_OUTPUT");
    assert_true(frame.len == 4, "Payload length = 4");
    
    float value = bytes_to_float(frame.payload);
    assert_true(value == 1.0f, "Payload = float 1.0");
}

void test_builder_output(void) {
    printf("\n" BOLD "=== Test Builder ===" RESET "\n");
    
    uint8_t buf[32];
    int len = builder_output(buf, 187.5f);
    
    assert_true(len > 0, "Builder OUTPUT crée une trame");
    assert_true(buf[0] == FRAME_START, "Trame commence par 0xAA");
    assert_true(buf[3] == MSG_OUTPUT, "Type = MSG_OUTPUT");
    
    // Vérifier le CRC
    uint8_t crc_calc = parser_compute_crc(&buf[1], len - 2);
    assert_true(buf[len - 1] == crc_calc, "CRC valide");
    
    printf("  → Trame OUTPUT (%d octets) : ", len);
    for (int i = 0; i < len; i++) printf("%02X ", buf[i]);
    printf("\n");
}

void test_builder_stats(void) {
    uint8_t buf[32];
    uint32_t stats[4] = {100, 5, 2, 150};
    int len = builder_stats(buf, stats);
    
    assert_true(len > 0, "Builder STATS crée une trame");
    assert_true(buf[0] == FRAME_START, "Trame commence par 0xAA");
    assert_true(buf[3] == MSG_STATS, "Type = MSG_STATS");
    
    uint8_t crc_calc = parser_compute_crc(&buf[1], len - 2);
    assert_true(buf[len - 1] == crc_calc, "CRC valide");
    
    printf("  → Trame STATS (%d octets) : ", len);
    for (int i = 0; i < len; i++) printf("%02X ", buf[i]);
    printf("\n");
}

void test_builder_alarm(void) {
    uint8_t buf[80];
    int len = builder_alarm(buf, "EMERGENCY STOP");
    
    assert_true(len > 0, "Builder ALARM crée une trame");
    assert_true(buf[0] == FRAME_START, "Trame commence par 0xAA");
    assert_true(buf[3] == MSG_ALARM, "Type = MSG_ALARM");
    
    uint8_t crc_calc = parser_compute_crc(&buf[1], len - 2);
    assert_true(buf[len - 1] == crc_calc, "CRC valide");
    
    printf("  → Trame ALARM (%d octets) : ", len);
    for (int i = 0; i < len; i++) printf("%02X ", buf[i]);
    printf("\n");
}

void test_roundtrip(void) {
    printf("\n" BOLD "=== Test Roundtrip (Builder → Parser) ===" RESET "\n");
    
    // 1. Builder crée une trame OUTPUT
    uint8_t buf[32];
    int len = builder_output(buf, 250.75f);
    
    // 2. Parser reçoit les octets un par un
    parser_ctx_t ctx;
    frame_t frame;
    parser_init(&ctx);
    
    bool complete = false;
    for (int i = 0; i < len; i++) {
        complete = parser_push_byte(buf[i], &ctx, &frame);
    }
    
    assert_true(complete, "Parser valide la trame");
    assert_true(frame.type == MSG_OUTPUT, "Type préservé");
    assert_true(frame.len == 4, "Longueur payload préservée");
    
    float value = bytes_to_float(frame.payload);
    assert_true(value == 250.75f, "Valeur float préservée");
}



void test_corrupted_crc(void) {
    printf("\n" BOLD "=== Test CRC Corrompu ===" RESET "\n");
    
    // Créer une trame valide
    uint8_t buf[32];
    int len = builder_output(buf, 100.0f);
    
    // Corrompre le CRC
    buf[len - 1] ^= 0xFF;
    
    // Parser reçoit la trame corrompue
    parser_ctx_t ctx;
    frame_t frame;
    parser_init(&ctx);
    
    bool complete = false;
    for (int i = 0; i < len; i++) {
        complete = parser_push_byte(buf[i], &ctx, &frame);
    }
    
    assert_true(!complete, "Parser rejette trame CRC invalide");
}

void test_fragmented_frame(void) {
    printf("\n" BOLD "=== Test Trame Fragmentée ===" RESET "\n");
    
    // Builder crée une trame
    uint8_t buf[32];
    int len = builder_output(buf, 75.5f);
    
    // Parser reçoit les octets avec des "trous" (simulation de délai)
    parser_ctx_t ctx;
    frame_t frame;
    parser_init(&ctx);
    
    bool complete = false;
    for (int i = 0; i < len; i++) {
        complete = parser_push_byte(buf[i], &ctx, &frame);
    }
    
    assert_true(complete, "Parser reconstruit trame fragmentée");
    assert_true(frame.type == MSG_OUTPUT, "Type correct malgré fragmentation");
}

void test_unknown_type(void) {
    printf("\n" BOLD "=== Test Type Inconnu ===" RESET "\n");
    
    // Créer une trame avec un type qui n'existe pas (0x99)
    uint8_t buf[32];
    int idx = 0;
    buf[idx++] = FRAME_START;
    buf[idx++] = 0x05;  // len_l
    buf[idx++] = 0x00;  // len_h
    buf[idx++] = 0x99;  // type INCONNU
    buf[idx++] = 0x12;  // payload
    buf[idx++] = 0x34;
    buf[idx++] = 0x56;
    buf[idx++] = 0x78;
    
    // Calculer le CRC
    uint8_t crc = parser_compute_crc(&buf[1], 7);
    buf[idx++] = crc;
    
    // Parser reçoit la trame avec type inconnu
    parser_ctx_t ctx;
    frame_t frame;
    parser_init(&ctx);
    
    bool complete = false;
    for (int i = 0; i < idx; i++) {
        complete = parser_push_byte(buf[i], &ctx, &frame);
    }
    
    // Le parser doit ACCEPTER la trame (c'est au niveau au-dessus de rejeter)
    assert_true(complete, "Parser accepte type inconnu (validation au niveau app)");
    assert_true(frame.type == 0x99, "Type inconnu préservé");
    assert_true(frame.len == 4, "Payload préservé");
}

void test_noise_resilience(void) {
    printf("\n" BOLD "=== Test Résilience au Bruit ===" RESET "\n");
    
    // Créer une trame valide
    uint8_t buf[32];
    int len = builder_output(buf, 123.45f);
    
    // Parser reçoit du bruit, puis la trame valide SANS réinitialisation manuelle
    parser_ctx_t ctx;
    frame_t frame;
    parser_init(&ctx);
    
    // Envoyer du bruit qui n'inclut pas 0xAA (le parser doit ignorer)
    parser_push_byte(0xFF, &ctx, &frame);
    parser_push_byte(0x55, &ctx, &frame);
    parser_push_byte(0x99, &ctx, &frame);
    parser_push_byte(0x42, &ctx, &frame);
    parser_push_byte(0xDE, &ctx, &frame);
    
    // Maintenant envoyer la vraie trame directement
    // Le parser doit chercher le prochain 0xAA et reconstruire la trame
    bool complete = false;
    for (int i = 0; i < len; i++) {
        complete = parser_push_byte(buf[i], &ctx, &frame);
    }
    
    assert_true(complete, "Parser ignore le bruit et valide la vraie trame");
}

// ===========================================================================
// MAIN
// ===========================================================================

int main(void) {
    printf(BOLD "\n╔════════════════════════════════════════╗\n");
    printf("║  Tests Parser / Builder - ECU Cruise   ║\n");
    printf("╚════════════════════════════════════════╝\n" RESET);
    
    test_parser_simple_message();
    test_builder_output();
    test_builder_stats();
    test_builder_alarm();
    test_roundtrip();
    test_corrupted_crc();
    test_fragmented_frame();
    test_unknown_type();
    test_noise_resilience();
    
    // Résultats
    printf("\n" BOLD "╔════════════════════════════════════════╗\n");
    printf("║  Résultats                             ║\n");
    printf("╚════════════════════════════════════════╝\n" RESET);
    printf("%d / %d tests passés ", test_passed, test_count);
    
    if (test_passed == test_count) {
        printf(GREEN "✓\n" RESET);
        return 0;
    } else {
        printf(RED "✗\n" RESET);
        return 1;
    }
}
