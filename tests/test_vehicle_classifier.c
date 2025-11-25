#include <zephyr.h>
#include <ztest.h>
#include <string.h>
#include "radar.h"

/* Test fixture para dados de veículo */
static vehicle_data_t create_vehicle_data(uint8_t axle_count, uint32_t time_between_sensors)
{
    vehicle_data_t vehicle = {
        .timestamp = k_uptime_get_32(),
        .time_between_sensors = time_between_sensors,
        .axle_count = axle_count,
        .speed_kmh = 0.0f,
        .type = VEHICLE_UNKNOWN,
        .direction = DIRECTION_UNKNOWN,
        .total_passage_time = 0,
        .valid_measurement = true
    };
    return vehicle;
}

/* Teste: Classificação de veículo leve (2 eixos) */
void test_classify_light_vehicle_2_axles(void)
{
    vehicle_data_t vehicle = create_vehicle_data(2, 100);
    vehicle_type_t result = classify_vehicle(&vehicle);
    
    zassert_equal(result, VEHICLE_LIGHT, 
                 "Veículo com 2 eixos deveria ser classificado como LEVE");
    zassert_equal(vehicle.type, VEHICLE_LIGHT,
                 "Tipo do veículo deveria ser atualizado para LEVE");
}

/* Teste: Classificação de veículo pesado (3 eixos) */
void test_classify_heavy_vehicle_3_axles(void)
{
    vehicle_data_t vehicle = create_vehicle_data(3, 150);
    vehicle_type_t result = classify_vehicle(&vehicle);
    
    zassert_equal(result, VEHICLE_HEAVY,
                 "Veículo com 3 eixos deveria ser classificado como PESADO");
    zassert_equal(vehicle.type, VEHICLE_HEAVY,
                 "Tipo do veículo deveria ser atualizado para PESADO");
}

/* Teste: Classificação de veículo pesado (4 eixos) */
void test_classify_heavy_vehicle_4_axles(void)
{
    vehicle_data_t vehicle = create_vehicle_data(4, 200);
    vehicle_type_t result = classify_vehicle(&vehicle);
    
    zassert_equal(result, VEHICLE_HEAVY,
                 "Veículo com 4 eixos deveria ser classificado como PESADO");
}

/* Teste: Classificação de veículo pesado (6 eixos - caminhão extra-pesado) */
void test_classify_heavy_vehicle_6_axles(void)
{
    vehicle_data_t vehicle = create_vehicle_data(6, 300);
    vehicle_type_t result = classify_vehicle(&vehicle);
    
    zassert_equal(result, VEHICLE_HEAVY,
                 "Veículo com 6 eixos deveria ser classificado como PESADO");
}

/* Teste: Veículo desconhecido (1 eixo - inválido) */
void test_classify_unknown_vehicle_1_axle(void)
{
    vehicle_data_t vehicle = create_vehicle_data(1, 50);
    vehicle_type_t result = classify_vehicle(&vehicle);
    
    zassert_equal(result, VEHICLE_UNKNOWN,
                 "Veículo com 1 eixo deveria ser classificado como DESCONHECIDO");
}

/* Teste: Veículo desconhecido (0 eixos - inválido) */
void test_classify_unknown_vehicle_0_axles(void)
{
    vehicle_data_t vehicle = create_vehicle_data(0, 0);
    vehicle_type_t result = classify_vehicle(&vehicle);
    
    zassert_equal(result, VEHICLE_UNKNOWN,
                 "Veículo com 0 eixos deveria ser classificado como DESCONHECIDO");
}

/* Teste: Veículo com número máximo de eixos */
void test_classify_vehicle_max_axles(void)
{
    vehicle_data_t vehicle = create_vehicle_data(10, 500);
    vehicle_type_t result = classify_vehicle(&vehicle);
    
    zassert_equal(result, VEHICLE_HEAVY,
                 "Veículo com 10 eixos deveria ser classificado como PESADO");
}

/* Teste: Validação de dados do veículo - válido */
void test_validate_vehicle_data_valid(void)
{
    vehicle_data_t vehicle = create_vehicle_data(2, 100);
    bool result = validate_vehicle_data(&vehicle);
    
    zassert_true(result, "Dados do veículo com 2 eixos e tempo 100ms deveriam ser válidos");
    zassert_true(vehicle.valid_measurement, "Medição deveria ser marcada como válida");
}

/* Teste: Validação de dados do veículo - eixos inválidos */
void test_validate_vehicle_data_invalid_axles(void)
{
    vehicle_data_t vehicle = create_vehicle_data(11, 100); // 11 eixos > máximo
    bool result = validate_vehicle_data(&vehicle);
    
    zassert_false(result, "Dados do veículo com 11 eixos deveriam ser inválidos");
    zassert_false(vehicle.valid_measurement, "Medição deveria ser marcada como inválida");
}

/* Teste: Validação de dados do veículo - tempo inválido */
void test_validate_vehicle_data_invalid_time(void)
{
    vehicle_data_t vehicle = create_vehicle_data(2, 5); // 5ms < mínimo
    bool result = validate_vehicle_data(&vehicle);
    
    zassert_false(result, "Dados do veículo com tempo 5ms deveriam ser inválidos");
}

/* Teste: Determinação de direção - para frente */
void test_determine_direction_forward(void)
{
    uint32_t sensor1_time = 1000;
    uint32_t sensor2_time = 1100; // Sensor2 depois do Sensor1
    
    direction_t result = determine_direction(sensor1_time, sensor2_time);
    
    zassert_equal(result, DIRECTION_FORWARD,
                 "Direção deveria ser PARA FRENTE quando Sensor2 é acionado após Sensor1");
}

/* Teste: Determinação de direção - para trás */
void test_determine_direction_backward(void)
{
    uint32_t sensor1_time = 1100;
    uint32_t sensor2_time = 1000; // Sensor2 antes do Sensor1
    
    direction_t result = determine_direction(sensor1_time, sensor2_time);
    
    zassert_equal(result, DIRECTION_BACKWARD,
                 "Direção deveria ser PARA TRÁS quando Sensor2 é acionado antes do Sensor1");
}

/* Teste: Determinação de direção - desconhecida (tempos iguais) */
void test_determine_direction_unknown(void)
{
    uint32_t sensor1_time = 1000;
    uint32_t sensor2_time = 1000; // Mesmo tempo
    
    direction_t result = determine_direction(sensor1_time, sensor2_time);
    
    zassert_equal(result, DIRECTION_UNKNOWN,
                 "Direção deveria ser DESCONHECIDA quando sensores são acionados no mesmo tempo");
}

/* Teste: Funções de utilidade - get_vehicle_type_str */
void test_vehicle_type_strings(void)
{
    zassert_str_equal(get_vehicle_type_str(VEHICLE_UNKNOWN), "DESCONHECIDO",
                     "String para VEHICLE_UNKNOWN incorreta");
    zassert_str_equal(get_vehicle_type_str(VEHICLE_LIGHT), "LEVE",
                     "String para VEHICLE_LIGHT incorreta");
    zassert_str_equal(get_vehicle_type_str(VEHICLE_HEAVY), "PESADO",
                     "String para VEHICLE_HEAVY incorreta");
    zassert_str_equal(get_vehicle_type_str((vehicle_type_t)99), "INVALIDO",
                     "String para tipo inválido incorreta");
}

/* Teste: Funções de utilidade - is_valid_axle_count */
void test_is_valid_axle_count(void)
{
    zassert_true(is_valid_axle_count(1), "1 eixo deveria ser válido");
    zassert_true(is_valid_axle_count(5), "5 eixos deveria ser válido");
    zassert_true(is_valid_axle_count(10), "10 eixos deveria ser válido");
    zassert_false(is_valid_axle_count(0), "0 eixos deveria ser inválido");
    zassert_false(is_valid_axle_count(11), "11 eixos deveria ser inválido");
}

/* Teste: Cenário completo - veículo leve passando */
void test_complete_light_vehicle_scenario(void)
{
    vehicle_data_t vehicle = create_vehicle_data(2, 120);
    
    // Classifica veículo
    vehicle_type_t type = classify_vehicle(&vehicle);
    zassert_equal(type, VEHICLE_LIGHT, "Deveria ser veículo leve");
    
    // Valida dados
    bool valid = validate_vehicle_data(&vehicle);
    zassert_true(valid, "Dados deveriam ser válidos");
    
    // Calcula velocidade
    calculate_speed(&vehicle);
    zassert_true(is_valid_speed(vehicle.speed_kmh), "Velocidade deveria ser válida");
    
    // Verifica status
    speed_status_t status = check_speed_status(vehicle.speed_kmh, vehicle.type);
    zassert_true(status >= SPEED_NORMAL && status <= SPEED_INFRACTION, 
                "Status deveria ser válido");
}

/* Teste: Cenário completo - veículo pesado passando */
void test_complete_heavy_vehicle_scenario(void)
{
    vehicle_data_t vehicle = create_vehicle_data(4, 180);
    
    // Classifica veículo
    vehicle_type_t type = classify_vehicle(&vehicle);
    zassert_equal(type, VEHICLE_HEAVY, "Deveria ser veículo pesado");
    
    // Valida dados
    bool valid = validate_vehicle_data(&vehicle);
    zassert_true(valid, "Dados deveriam ser válidos");
    
    // Calcula velocidade
    calculate_speed(&vehicle);
    zassert_true(is_valid_speed(vehicle.speed_kmh), "Velocidade deveria ser válida");
    
    // Determina direção
    vehicle.direction = determine_direction(1000, 1180);
    zassert_equal(vehicle.direction, DIRECTION_FORWARD, "Deveria ser direção para frente");
}

/* Teste: Performance - múltiplas classificações rápidas */
void test_performance_multiple_classifications(void)
{
    uint32_t start_time = k_uptime_get_32();
    
    for (int i = 0; i < 100; i++) {
        vehicle_data_t vehicle = create_vehicle_data(2 + (i % 3), 100 + (i * 10));
        vehicle_type_t type = classify_vehicle(&vehicle);
        zassert_true(type != VEHICLE_UNKNOWN, "Classificação deveria ser válida");
    }
    
    uint32_t end_time = k_uptime_get_32();
    uint32_t duration = end_time - start_time;
    
    zassert_true(duration < 100, "100 classificações deveriam levar menos de 100ms");
}

/* Teste: Boundary values - valores limite */
void test_boundary_values(void)
{
    // Testa valores limite para contagem de eixos
    zassert_equal(classify_vehicle(&(vehicle_data_t){.axle_count = 1}), VEHICLE_UNKNOWN);
    zassert_equal(classify_vehicle(&(vehicle_data_t){.axle_count = 2}), VEHICLE_LIGHT);
    zassert_equal(classify_vehicle(&(vehicle_data_t){.axle_count = 3}), VEHICLE_HEAVY);
    zassert_equal(classify_vehicle(&(vehicle_data_t){.axle_count = 10}), VEHICLE_HEAVY);
}

/* Suite de testes principal */
void test_main(void)
{
    ztest_test_suite(vehicle_classifier_tests,
        ztest_unit_test(test_classify_light_vehicle_2_axles),
        ztest_unit_test(test_classify_heavy_vehicle_3_axles),
        ztest_unit_test(test_classify_heavy_vehicle_4_axles),
        ztest_unit_test(test_classify_heavy_vehicle_6_axles),
        ztest_unit_test(test_classify_unknown_vehicle_1_axle),
        ztest_unit_test(test_classify_unknown_vehicle_0_axles),
        ztest_unit_test(test_classify_vehicle_max_axles),
        ztest_unit_test(test_validate_vehicle_data_valid),
        ztest_unit_test(test_validate_vehicle_data_invalid_axles),
        ztest_unit_test(test_validate_vehicle_data_invalid_time),
        ztest_unit_test(test_determine_direction_forward),
        ztest_unit_test(test_determine_direction_backward),
        ztest_unit_test(test_determine_direction_unknown),
        ztest_unit_test(test_vehicle_type_strings),
        ztest_unit_test(test_is_valid_axle_count),
        ztest_unit_test(test_complete_light_vehicle_scenario),
        ztest_unit_test(test_complete_heavy_vehicle_scenario),
        ztest_unit_test(test_performance_multiple_classifications),
        ztest_unit_test(test_boundary_values)
    );
    
    ztest_run_test_suite(vehicle_classifier_tests);
}