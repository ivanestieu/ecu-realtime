#include <format>
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

static void print_frame_bytes(const std::vector<uint8_t>& bytes)
{
    // Trame (LITTLE-ENDIAN) : [0xAA] [0x05] [0x00] [0x80] [0x00 0x00 0x80 0x3f] [CRC]
    //                          start  len_l  len_h  type   payload (float 1.0)   crc
    std::cout << std::format("[{:02X}] [{:02X}] [{:02X}] [{:02X}] [", bytes[0],
        bytes[1], bytes[2], bytes[3]);
    for (size_t i = 4; i < bytes.size() - 1; i++)
    {
        std::cout
           <<std::format("{:02X}", bytes[i]);
        if (i < bytes.size() - 2)
        {
            std::cout << " ";
        }
    }
    std::cout << std::format("] [{:02X}]\n", bytes[bytes.size() - 1]);
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
    assert_true(bytes[3] == Frame::MsgType::OUTPUT, "Type = OUTPUT");
    
    // Vérifier le CRC
    const uint8_t crc_calc = compute_crc(bytes.begin()+1, bytes.end() - 1u);
    assert_true(bytes[bytes.size() - 1] == crc_calc, "CRC valide");

    std::cout << "  --> Trame OUTPUT (%d octets) : " << bytes.size() << "\n";
    print_frame_bytes(bytes);
}

static void test_builder_stats() {
    const std::vector<uint32_t> stats{100, 5, 2, 150};
    const Frame frame = builder::stats(stats);
    const auto& bytes = frame.get_full_frame();

    assert_true(!bytes.empty(), "Builder STATS crée une trame");
    assert_true(bytes[0] == Frame::START_BYTE, "Trame commence par 0xAA");
    assert_true(bytes[3] == Frame::MsgType::STATS, "Type = STATS");

    const uint8_t crc_calc = compute_crc(bytes.begin()+1, bytes.end() - 1u);
    assert_true(bytes[bytes.size() - 1] == crc_calc, "CRC valide");

    std::cout << "  --> Trame OUTPUT (%d octets) : " << bytes.size() << "\n";
    print_frame_bytes(bytes);
}

static void test_builder_alarm() {
    const Frame frame = builder::alarm("EMERGENCY STOP");
    const auto& bytes = frame.get_full_frame();

    assert_true(!bytes.empty(), "Builder ALARM crée une trame");
    assert_true(bytes[0] == Frame::START_BYTE, "Trame commence par 0xAA");
    assert_true(bytes[3] == Frame::MsgType::ALARM, "Type = ALARM");

    const uint8_t crc_calc = compute_crc(bytes.begin()+1, bytes.end() - 1u);
    assert_true(bytes[bytes.size() - 1] == crc_calc, "CRC valide");

    std::cout << "  --> Trame OUTPUT (%d octets) : " << bytes.size() << "\n";
    print_frame_bytes(bytes);
}

static void test_roundtrip() {
    std::cout << BOLD << "\n=== Test Roundtrip (Builder → Parser) ===\n" << RESET;
    
    // 1. Builder crée une trame OUTPUT
    const Frame frame = builder::output(250.75f);
    const auto& bytes = frame.get_full_frame();

    // 2. Parser reçoit les octets un par un
    Parser::Parser parser{};

    std::unique_ptr<Frame> complete = nullptr;
    for (const auto& byte : bytes) {
        complete = parser.push_byte(byte);
    }
    
    assert_true(complete != nullptr, "Parser reçoit une trame OUTPUT complète");
    assert_true(complete->get_type() == Frame::MsgType::OUTPUT, "Type = MSG_OUTPUT");
    assert_true(complete->get_payload_len() == 4, "Payload length = 4");

    const auto payload = complete->get_payload();
    const std::array<std::uint8_t, sizeof(float)> buf{payload[0], payload[1], payload[2], payload[3]};
    const float value = builder::bytes_to_float(buf);
    assert_true(value == 250.75f, "Payload = float 250.75");
}

static void test_corrupted_crc() {
    std::cout << BOLD << "\n=== Test CRC Corrompu ===\n" << RESET;

    // 1. Builder crée une trame OUTPUT
    const Frame frame = builder::output(100.0f);
    auto bytes = frame.get_full_frame();
    bytes[bytes.size() - 1] ^= 0xFF;

    // 2. Parser reçoit les octets un par un
    Parser::Parser parser{};

    std::unique_ptr<Frame> complete = nullptr;
    for (const auto& byte : bytes) {
        complete = parser.push_byte(byte);
    }

    assert_true(complete == nullptr, "Parser ne considère pas la trame comme complète");
}

static void test_unknown_type() {
    std::cout << BOLD << "\n=== Test Type Inconnu ===\n" << RESET;

    // Créer une trame avec un type qui n'existe pas (0x99)
    std::vector<uint8_t> bytes{};
    bytes.push_back(Frame::START_BYTE);
    bytes.push_back(0x05);
    bytes.push_back(0x00);
    bytes.push_back(0x99);
    bytes.push_back(0x12);
    bytes.push_back(0x34);
    bytes.push_back(0x56);
    bytes.push_back(0x78);

    // Calculer le CRC
    uint8_t crc = compute_crc(&bytes[1], 7);
    bytes.push_back(crc);
    
    Parser::Parser parser{};

    std::unique_ptr<Frame> complete = nullptr;
    for (const auto& byte : bytes) {
        complete = parser.push_byte(byte);
    }
    // Le parser doit ACCEPTER la trame (c'est au niveau au-dessus de rejeter)
    assert_true(complete != nullptr, "Parser reçoit une trame OUTPUT complète");
    assert_true(complete->get_type() == 0x99, "Type = MSG_OUTPUT");
    assert_true(complete->get_payload_len() == 4, "Payload length = 4");

}

void test_noise_resilience() {
    std::cout << BOLD << "\n=== Test Résilience au Bruit ===\n" << RESET;

    // Créer une trame valide
    const Frame frame = builder::output(100.0f);
    auto bytes = frame.get_full_frame();

    // Parser reçoit du bruit, puis la trame valide SANS réinitialisation manuelle
    Parser::Parser parser{};

    // Envoyer du bruit qui n'inclut pas 0xAA (le parser doit ignorer)
    parser.push_byte(0xFF);
    parser.push_byte(0x55);
    parser.push_byte(0x99);
    parser.push_byte(0x42);
    parser.push_byte(0xDE);
    std::unique_ptr<Frame> complete = nullptr;
    for (const auto& byte : bytes) {
        complete = parser.push_byte(byte);
    }

    // Maintenant envoyer la vraie trame directement
    // Le parser doit chercher le prochain 0xAA et reconstruire la trame

    assert_true(complete != nullptr, "Parser ignore le bruit et valide la vraie trame");
}

// ===========================================================================
// MAIN
// ===========================================================================

int main() {
    std::cout << BOLD << "\n╔════════════════════════════════════════╗\n"
    << "║  Tests Parser / Builder - ECU Cruise   ║\n"
    << "╚════════════════════════════════════════╝\n" << RESET;
    
    test_parser_simple_message();
    test_builder_output();
    test_builder_stats();
    test_builder_alarm();
    test_roundtrip();
    test_corrupted_crc();
    test_unknown_type();
    test_noise_resilience();
    
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
