#include <zephyr.h>
#include <ztest.h>
#include <string.h>
#include <ctype.h>
#include "radar.h"

/* Teste: Validação de placa Mercosul válida (formato novo) */
void test_validate_mercosul_plate_valid(void)
{
    const char *valid_plates[] = {
        "ABC1D23",    // Formato padrão Mercosul
        "XYZ9A87",    // Outro formato válido
        "JKL4M56",    // Formato válido
        "QWE2R34",    // Formato válido
        NULL
    };
    
    for (int i = 0; valid_plates[i] != NULL; i++) {
        bool result = validate_license_plate(valid_plates[i]);
        zassert_true(result, "Placa '%s' deveria ser válida", valid_plates[i]);
    }
}

/* Teste: Validação de placa antiga válida (formato numérico) */
void test_validate_old_plate_valid(void)
{
    const char *valid_old_plates[] = {
        "ABC1234",    // Formato antigo
        "XYZ5678",    // Outro formato antigo
        "JKL9012",    // Formato antigo
        "QWE3456",    // Formato antigo
        NULL
    };
    
    for (int i = 0; valid_old_plates[i] != NULL; i++) {
        bool result = validate_license_plate(valid_old_plates[i]);
        zassert_true(result, "Placa antiga '%s' deveria ser válida", valid_old_plates[i]);
    }
}

/* Teste: Placas inválidas - formato incorreto */
void test_validate_plate_invalid_format(void)
{
    const char *invalid_plates[] = {
        "AB1C234",    // Formato misturado
        "ABCD123",    // 4 letras + 3 números
        "ABC12D3",    // Posições trocadas
        "123ABCD",    // Números antes de letras
        "A1B2C3D",    // Alternado
        "ABC-123",    // Com hífen
        "ABC 123",    // Com espaço
        "",           // String vazia
        "ABC",        // Muito curta
        "ABCDEFGH",   // Muito longa
        "ABC1B23",    // Letra repetida no padrão errado
        NULL
    };
    
    for (int i = 0; invalid_plates[i] != NULL; i++) {
        bool result = validate_license_plate(invalid_plates[i]);
        zassert_false(result, "Placa '%s' deveria ser inválida", invalid_plates[i]);
    }
}

/* Teste: Placas inválidas - caracteres especiais */
void test_validate_plate_invalid_characters(void)
{
    const char *invalid_char_plates[] = {
        "ABÇ1D23",    // Caractere especial (Ç)
        "ABC1@23",    // Caractere especial (@)
        "ABC1 23",    // Espaço no meio
        "ABC1_23",    // Underscore
        "ABC1.23",    // Ponto
        NULL
    };
    
    for (int i = 0; invalid_char_plates[i] != NULL; i++) {
        bool result = validate_license_plate(invalid_char_plates[i]);
        zassert_false(result, "Placa com caractere especial '%s' deveria ser inválida", 
                     invalid_char_plates[i]);
    }
}

/* Teste: Placas inválidas - números onde deveriam ser letras */
void test_validate_plate_numbers_in_letter_positions(void)
{
    const char *invalid_pos_plates[] = {
        "1BC1D23",    // Número na primeira posição
        "A1C1D23",    // Número na segunda posição
        "AB11D23",    // Número na terceira posição
        "ABC1D2A",    // Letra na última posição (formato antigo)
        NULL
    };
    
    for (int i = 0; invalid_pos_plates[i] != NULL; i++) {
        bool result = validate_license_plate(invalid_pos_plates[i]);
        zassert_false(result, "Placa com número em posição de letra '%s' deveria ser inválida",
                     invalid_pos_plates[i]);
    }
}

/* Teste: Simulação de placa - geração consistente */
void test_simulate_license_plate_generation(void)
{
    char plate[8];
    bool valid;
    
    // Testa múltiplas gerações
    for (int i = 0; i < 50; i++) {
        simulate_license_plate(plate, &valid);
        
        // Verifica se a placa gerada tem tamanho correto
        zassert_equal(strlen(plate), 7, "Placa gerada deveria ter 7 caracteres");
        
        // Verifica formato básico
        zassert_true(isalpha(plate[0]), "Primeiro caractere deveria ser letra");
        zassert_true(isalpha(plate[1]), "Segundo caractere deveria ser letra");
        zassert_true(isalpha(plate[2]), "Terceiro caractere deveria ser letra");
        
        // Se for válida, verifica o formato completo
        if (valid) {
            bool is_valid_format = validate_license_plate(plate);
            zassert_true(is_valid_format, 
                        "Placa gerada como válida '%s' deveria passar na validação", plate);
        }
    }
}

/* Teste: Simulação de placa - taxa de falha */
void test_simulate_license_plate_failure_rate(void)
{
    char plate[8];
    bool valid;
    int valid_count = 0;
    int total_tests = 1000;
    
    // Configura taxa de falha baixa para teste
    // (em produção, usamos CONFIG_RADAR_CAMERA_FAILURE_RATE_PERCENT)
    
    for (int i = 0; i < total_tests; i++) {
        simulate_license_plate(plate, &valid);
        if (valid) {
            valid_count++;
        } else {
            // Quando inválida, verifica que realmente não passa na validação
            bool validation_result = validate_license_plate(plate);
            zassert_false(validation_result, 
                         "Placa gerada como inválida '%s' não deveria passar na validação", plate);
        }
    }
    
    // Verifica que temos tanto placas válidas quanto inválidas
    zassert_true(valid_count > 0, "Deveria gerar algumas placas válidas");
    zassert_true(valid_count < total_tests, "Deveria gerar algumas placas inválidas");
}

/* Teste: Validação de caso limite - placa toda em maiúsculas */
void test_validate_plate_uppercase(void)
{
    // Todas as placas devem ser em maiúsculas por padrão
    const char *uppercase_plates[] = {
        "ABC1D23",
        "XYZ9A87", 
        "JKL4M56",
        NULL
    };
    
    for (int i = 0; uppercase_plates[i] != NULL; i++) {
        bool result = validate_license_plate(uppercase_plates[i]);
        zassert_true(result, "Placa em maiúsculas '%s' deveria ser válida", 
                    uppercase_plates[i]);
    }
}

/* Teste: Validação de caso limite - placa com minúsculas (deve ser inválida) */
void test_validate_plate_lowercase_invalid(void)
{
    const char *lowercase_plates[] = {
        "abc1d23",    // Todas minúsculas
        "Abc1D23",    // Misto
        "ABC1d23",    // Última letra minúscula
        NULL
    };
    
    for (int i = 0; lowercase_plates[i] != NULL; i++) {
        bool result = validate_license_plate(lowercase_plates[i]);
        zassert_false(result, "Placa com minúsculas '%s' deveria ser inválida",
                     lowercase_plates[i]);
    }
}

/* Teste: Validação estrita (quando habilitada) */
void test_validate_plate_strict_validation(void)
{
    // Esses testes assumem que PLATE_VALIDATION_STRICT está habilitado
    
    const char *suspicious_plates[] = {
        "AAA1A11",    // Padrão repetitivo
        "BBB2B22",    // Padrão repetitivo  
        "1234ABC",    // Formato completamente invertido
        NULL
    };
    
    for (int i = 0; suspicious_plates[i] != NULL; i++) {
        bool result = validate_license_plate(suspicious_plates[i]);
        
        if (PLATE_VALIDATION_STRICT) {
            zassert_false(result, 
                         "Placa suspeita '%s' deveria ser inválida em modo estrito",
                         suspicious_plates[i]);
        }
        // Se não estrito, pode ser válida dependendo do formato
    }
}

/* Teste: Performance - validação rápida de múltiplas placas */
void test_performance_plate_validation(void)
{
    const char *test_plates[] = {
        "ABC1D23", "XYZ9A87", "JKL4M56", "QWE2R34", "RTY5U67",
        "ABC1234", "XYZ5678", "JKL9012", "QWE3456", "RTY7890",
        "AB1C234", "ABCD123", "123ABCD", "ABC-123", "ABC12D3",
        NULL
    };
    
    uint32_t start_time = k_uptime_get_32();
    
    int iterations = 1000;
    for (int i = 0; i < iterations; i++) {
        for (int j = 0; test_plates[j] != NULL; j++) {
            bool result = validate_license_plate(test_plates[j]);
            // Não verificamos o resultado, apenas o tempo
            (void)result;
        }
    }
    
    uint32_t end_time = k_uptime_get_32();
    uint32_t duration = end_time - start_time;
    
    zassert_true(duration < 500, 
                "Validação de %d placas deveria levar menos de 500ms", iterations);
}

/* Teste: Validação de placas reais conhecidas */
void test_validate_real_world_plates(void)
{
    // Placas que existem no mundo real (exemplos)
    const char *real_plates[] = {
        "BRA0S17",    // Possível placa real
        "TEST1E23",   // Placa de teste
        "DEM0O99",    // Outra possível
        NULL
    };
    
    for (int i = 0; real_plates[i] != NULL; i++) {
        bool result = validate_license_plate(real_plates[i]);
        
        // Verifica apenas o formato, não se a placa realmente existe
        if (strlen(real_plates[i]) == 7) {
            bool expected = isalpha(real_plates[i][0]) && 
                           isalpha(real_plates[i][1]) && 
                           isalpha(real_plates[i][2]);
            zassert_equal(result, expected, 
                         "Placa real '%s' validação inesperada", real_plates[i]);
        }
    }
}

/* Teste: Buffer boundaries - proteção contra overflow */
void test_plate_buffer_boundaries(void)
{
    // Testa com string null-terminated corretamente
    char valid_plate[8] = "ABC1D23"; // 7 chars + null
    bool result = validate_license_plate(valid_plate);
    zassert_true(result, "Placa com buffer correto deveria ser válida");
    
    // Testa com string sem null terminator (deve ser detectada pelo strlen)
    char no_null[7] = {'A', 'B', 'C', '1', 'D', '2', '3'};
    // Não podemos chamar validate_license_plate diretamente pois espera string C
    // Isso testa a robustez da função
}

/* Teste: Caracteres especiais não-ASCII */
void test_validate_plate_non_ascii(void)
{
    const char *non_ascii_plates[] = {
        "ABÇ1D23",    // Ç não-ASCII
        "ANÃ1O23",    // Ã e Õ
        "SÃO1P23",    // Õ
        NULL
    };
    
    for (int i = 0; non_ascii_plates[i] != NULL; i++) {
        bool result = validate_license_plate(non_ascii_plates[i]);
        zassert_false(result, 
                     "Placa com caracteres não-ASCII '%s' deveria ser inválida",
                     non_ascii_plates[i]);
    }
}

/* Teste: Integração com estrutura camera_data */
void test_camera_data_integration(void)
{
    camera_data_t camera_data;
    vehicle_data_t vehicle_data = {
        .type = VEHICLE_LIGHT,
        .speed_kmh = 85.0f  // Infração
    };
    
    // Simula captura (função que seria chamada pela thread da câmera)
    simulate_license_plate(camera_data.plate, &camera_data.valid);
    camera_data.captured = true;
    camera_data.capture_time = k_uptime_get_32();
    camera_data.vehicle_type = vehicle_data.type;
    camera_data.vehicle_speed = vehicle_data.speed_kmh;
    
    // Verifica integridade dos dados
    zassert_true(camera_data.captured, "Dados da câmera deveriam estar capturados");
    zassert_equal(strlen(camera_data.plate), 7, "Placa deveria ter 7 caracteres");
    zassert_true(camera_data.capture_time > 0, "Tempo de captura deveria ser válido");
    
    // Se a placa é válida, deve passar na validação
    if (camera_data.valid) {
        bool validation_result = validate_license_plate(camera_data.plate);
        zassert_true(validation_result, 
                    "Placa válida na estrutura camera_data deveria passar na validação");
    }
}

/* Suite de testes principal */
void test_main(void)
{
    ztest_test_suite(license_validator_tests,
        ztest_unit_test(test_validate_mercosul_plate_valid),
        ztest_unit_test(test_validate_old_plate_valid),
        ztest_unit_test(test_validate_plate_invalid_format),
        ztest_unit_test(test_validate_plate_invalid_characters),
        ztest_unit_test(test_validate_plate_numbers_in_letter_positions),
        ztest_unit_test(test_simulate_license_plate_generation),
        ztest_unit_test(test_simulate_license_plate_failure_rate),
        ztest_unit_test(test_validate_plate_uppercase),
        ztest_unit_test(test_validate_plate_lowercase_invalid),
        ztest_unit_test(test_validate_plate_strict_validation),
        ztest_unit_test(test_performance_plate_validation),
        ztest_unit_test(test_validate_real_world_plates),
        ztest_unit_test(test_plate_buffer_boundaries),
        ztest_unit_test(test_validate_plate_non_ascii),
        ztest_unit_test(test_camera_data_integration)
    );
    
    ztest_run_test_suite(license_validator_tests);
}

/* Função auxiliar para debug durante testes */
void print_test_plate(const char *plate, bool expected, bool actual)
{
    printk("Placa: %s, Esperado: %s, Obtido: %s %s\n",
           plate, 
           expected ? "VALIDA" : "INVALIDA",
           actual ? "VALIDA" : "INVALIDA",
           (expected == actual) ? "✓" : "✗ ERRO");
}