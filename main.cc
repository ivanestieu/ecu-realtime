#include <iomanip>
#include <iostream>

#include "frame/parser.hh"
#include "frame/builder.hh"

static constexpr std::string GREEN = "\033[32m";
static constexpr std::string RED = "\033[31m";
static constexpr std::string RESET = "\033[0m";
static constexpr std::string BOLD = "\033[1m";

// ===========================================================================
// TESTS UNITAIRES
// ===========================================================================

static int test_count = 0;
static int test_passed = 0;

static uint8_t compute_crc(std::vector<uint8_t>::const_iterator begin, const std::vector<uint8_t>::const_iterator end)
{
    uint8_t crc = 0;
    for (; begin != end; ++begin) {
        crc ^= *begin;
    }
    return crc;
}

static uint8_t compute_crc(const uint8_t *data, const uint8_t len) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
    }
    return crc;
}

static void assert_true(const int condition, const std::string& test_name) {
    test_count++;
    if (condition) {
        test_passed++;
        std::cout << GREEN << test_name << ": Passed" << RESET << std::endl;
    } else {
        std::cout << RED << test_name << ": Failed" << RESET << std::endl;
    }
}

static void test_parser_simple_message() {
    std::cout << "\n" << BOLD << "=== Test Parser ===" << RESET << "\n";

    Parser::Parser parser{};
    
    // Créer une trame OUTPUT (4 octets de payload)
    // Trame (LITTLE-ENDIAN) : [0xAA] [0x05] [0x00] [0x80] [0x00 0x00 0x80 0x3f] [CRC]
    //                          start  len_l  len_h  type   payload (float 1.0)   crc

    const std::vector<uint8_t> trame_output{
        0xAA,                    // Start
        0x05, 0x00,              // Length = 5 (poids faible d'abord!)
        Frame::MsgType::OUTPUT,              // Type
        0x00, 0x00, 0x80, 0x3f,  // float 1.0 (little-endian)
    };
    
    // Calculer le CRC
    const uint8_t crc = compute_crc(&trame_output[1], 7);  // len_l + len_h + type + payload(4)
    
    // Envoyer octets un par un (tous les octets de la trame)
    for (const auto& byte : trame_output) {
        parser.push_byte(byte);
    }
    // Envoyer le CRC
    const auto complete = parser.push_byte(crc);

    assert_true(complete != nullptr, "Parser reçoit une trame OUTPUT complète");
    assert_true(complete->get_type() == Frame::MsgType::OUTPUT, "Type = MSG_OUTPUT");
    assert_true(complete->get_payload_len() == 4, "Payload length = 4");

    const auto payload = complete->get_payload();
    const std::array<std::uint8_t, sizeof(float)> buf{payload[0], payload[1], payload[2], payload[3]};
    const float value = builder::bytes_to_float(buf);
    assert_true(value == 1.0f, "Payload = float 1.0");
}

static void test_builder_output() {
    std::cout << "\n" << BOLD << "=== Test Builder ===" << RESET << "\n";

    const Frame frame = builder::output(187.5f);
    const auto& bytes = frame.get_full_frame();
    
    assert_true(!bytes.empty(), "Builder OUTPUT crée une trame");
    assert_true(bytes[0] == Frame::START_BYTE, "Trame commence par 0xAA");
    assert_true(bytes[3] == Frame::MsgType::OUTPUT, "Type = MSG_OUTPUT");
    
    // Vérifier le CRC
    const uint8_t crc_calc = compute_crc(bytes.begin()+1, bytes.end() - 1u);
    assert_true(bytes[bytes.size() - 1] == crc_calc, "CRC valide");

    std::cout << "  --> Trame OUTPUT (%d octets) : " << bytes.size() << "\n";
    for (const auto& byte: bytes)
    {
        std::cout << std::uppercase
           << std::hex
           << std::setw(2)
           << std::setfill('0')
           << byte << " ";
    }
    std::cout << std::endl;
}

/*void test_builder_stats() {
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

void test_builder_alarm() {
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

void test_roundtrip() {
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



void test_corrupted_crc() {
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

void test_fragmented_frame() {
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

void test_unknown_type() {
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

void test_noise_resilience() {
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
}*/

// ===========================================================================
// MAIN
// ===========================================================================

int main() {
    std::cout << BOLD << "\n╔════════════════════════════════════════╗\n"
    << "║  Tests Parser / Builder - ECU Cruise   ║\n"
    << "╚════════════════════════════════════════╝\n" << RESET;
    
    test_parser_simple_message();
    test_builder_output();
    /*test_builder_stats();
    test_builder_alarm();
    test_roundtrip();
    test_corrupted_crc();
    test_fragmented_frame();
    test_unknown_type();
    test_noise_resilience();*/
    
    // Résultats
    std::cout << BOLD << "\n╔════════════════════════════════════════╗\n"
    << "║    Résultats                           ║\n"
    << "╚════════════════════════════════════════╝\n" << RESET
    << test_passed << " / "<< test_count << " tests passés ";
    
    if (test_passed == test_count) {
        std::cout << GREEN << "✓\n" << RESET;
        return 0;
    } else {
        std::cout << RED << "✗\n" << RESET;
        return 1;
    }
}
