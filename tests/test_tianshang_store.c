#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "tianshang_store.h"

// 构造一个通过结构性校验的样本状态：15 抽 = 1 天赏 + 2 奇赏 + 12 凡赏，
// 其中天赏为直出天赏石(1 石 + 0 祥瑞)。
static tianshang_state_t make_sane_state(void)
{
    tianshang_state_t state;
    tianshang_init_state(&state);

    state.counters.non_tian_count = 3;
    state.counters.fan_count = 2;
    state.total_draw_count = 15;
    state.total_tian_count = 1;
    state.total_qi_count = 2;
    state.total_fan_count = 12;
    state.total_stone_count = 1;
    state.total_mount_count = 0;
    state.first_tian_draw = 5;
    state.first_mount_draw = 0;
    state.min_tian_pity = 5;

    return state;
}

static void test_blob_size(void)
{
    assert(TIANSHANG_STORE_BLOB_SIZE == 52);

    uint8_t blob[TIANSHANG_STORE_BLOB_SIZE];
    tianshang_state_t state = make_sane_state();
    assert(tianshang_store_encode(&state, blob, sizeof(blob)) == TIANSHANG_STORE_BLOB_SIZE);
}

static void test_crc_known_vector(void)
{
    // CRC-32/ISO-HDLC 标准测试向量
    assert(tianshang_store_crc32((const uint8_t *)"123456789", 9) == 0xCBF43926u);
}

static void test_roundtrip(void)
{
    tianshang_state_t state = make_sane_state();

    uint8_t blob[TIANSHANG_STORE_BLOB_SIZE];
    assert(tianshang_store_encode(&state, blob, sizeof(blob)) == TIANSHANG_STORE_BLOB_SIZE);

    tianshang_state_t restored;
    memset(&restored, 0xAA, sizeof(restored));
    assert(tianshang_store_decode(blob, sizeof(blob), &restored) == TIANSHANG_STORE_OK);

    assert(memcmp(&state, &restored, sizeof(state)) == 0);
    assert(restored.total_draw_count == 15);
    assert(restored.first_tian_draw == 5);
    assert(restored.min_tian_pity == 5);
    assert(restored.counters.non_tian_count == 3);
    assert(restored.counters.fan_count == 2);
}

static void test_roundtrip_mount_record(void)
{
    // 祥瑞记录：天赏产出为祥瑞时石头数为 0，校验规则必须接受。
    tianshang_state_t state = make_sane_state();
    state.total_mount_count = 1;
    state.total_stone_count = 0;
    state.first_mount_draw = 5;

    uint8_t blob[TIANSHANG_STORE_BLOB_SIZE];
    assert(tianshang_store_encode(&state, blob, sizeof(blob)) == TIANSHANG_STORE_BLOB_SIZE);

    tianshang_state_t restored;
    assert(tianshang_store_decode(blob, sizeof(blob), &restored) == TIANSHANG_STORE_OK);
    assert(restored.total_mount_count == 1);
    assert(restored.first_mount_draw == 5);
}

static void test_encode_rejects_small_buffer(void)
{
    tianshang_state_t state = make_sane_state();
    uint8_t blob[TIANSHANG_STORE_BLOB_SIZE];

    assert(tianshang_store_encode(&state, blob, TIANSHANG_STORE_BLOB_SIZE - 1) == 0);
    assert(tianshang_store_encode(&state, NULL, sizeof(blob)) == 0);
    assert(tianshang_store_encode(NULL, blob, sizeof(blob)) == 0);
}

static void test_blank_flash_is_not_found(void)
{
    // 出厂/擦除后的 Flash 全 0xFF，不能当成有效记录。
    uint8_t blob[TIANSHANG_STORE_BLOB_SIZE];
    memset(blob, 0xFF, sizeof(blob));

    tianshang_state_t state = make_sane_state();
    tianshang_state_t untouched = state;

    assert(tianshang_store_decode(blob, sizeof(blob), &state) == TIANSHANG_STORE_NOT_FOUND);
    assert(memcmp(&state, &untouched, sizeof(state)) == 0);
}

static void test_wrong_size_is_invalid(void)
{
    tianshang_state_t state = make_sane_state();
    uint8_t blob[TIANSHANG_STORE_BLOB_SIZE];
    tianshang_store_encode(&state, blob, sizeof(blob));

    tianshang_state_t untouched = state;
    assert(tianshang_store_decode(blob, sizeof(blob) - 1, &state) == TIANSHANG_STORE_INVALID);
    assert(tianshang_store_decode(blob, sizeof(blob) + 1, &state) == TIANSHANG_STORE_INVALID);
    assert(memcmp(&state, &untouched, sizeof(state)) == 0);
}

static void test_corrupt_byte_is_invalid(void)
{
    tianshang_state_t state = make_sane_state();
    uint8_t blob[TIANSHANG_STORE_BLOB_SIZE];
    tianshang_store_encode(&state, blob, sizeof(blob));

    // 逐字节翻位：任何一处损坏都必须被 CRC 拦下，且不改动传入状态。
    for (size_t i = 0; i < sizeof(blob); i++) {
        uint8_t damaged[TIANSHANG_STORE_BLOB_SIZE];
        memcpy(damaged, blob, sizeof(blob));
        damaged[i] ^= 0x01;

        tianshang_state_t restored = state;
        tianshang_store_result_t res = tianshang_store_decode(damaged, sizeof(damaged), &restored);
        assert(res != TIANSHANG_STORE_OK);
        if (res != TIANSHANG_STORE_NOT_FOUND) {
            assert(res == TIANSHANG_STORE_INVALID);
        }
        assert(memcmp(&restored, &state, sizeof(state)) == 0);
    }
}

static void test_version_mismatch_is_invalid(void)
{
    tianshang_state_t state = make_sane_state();
    uint8_t blob[TIANSHANG_STORE_BLOB_SIZE];
    tianshang_store_encode(&state, blob, sizeof(blob));

    // 模拟固件升级后读到旧版本记录：版本号不符即拒绝，不尝试按新格式解释。
    blob[4] = 0x7F;
    blob[5] = 0x00;

    tianshang_state_t restored = state;
    assert(tianshang_store_decode(blob, sizeof(blob), &restored) == TIANSHANG_STORE_INVALID);
    assert(memcmp(&restored, &state, sizeof(state)) == 0);
}

static void test_insane_payload_is_invalid(void)
{
    // CRC 正确的记录也可能数值荒谬(例如残留数据恰好凑出合法魔法字)。
    // 结构性校验必须拒绝这类记录，否则统计会被污染。
    tianshang_state_t state = make_sane_state();
    uint8_t blob[TIANSHANG_STORE_BLOB_SIZE];
    tianshang_store_encode(&state, blob, sizeof(blob));

    // 篡改总抽数，使三类结果之和对不上，然后重算 CRC 让它"看起来"完整。
    blob[8 + 4] = 0xFE;
    blob[8 + 5] = 0xFF;
    uint32_t crc = tianshang_store_crc32(blob, TIANSHANG_STORE_BLOB_SIZE - 4);
    blob[TIANSHANG_STORE_BLOB_SIZE - 4] = (uint8_t)(crc & 0xFF);
    blob[TIANSHANG_STORE_BLOB_SIZE - 3] = (uint8_t)((crc >> 8) & 0xFF);
    blob[TIANSHANG_STORE_BLOB_SIZE - 2] = (uint8_t)((crc >> 16) & 0xFF);
    blob[TIANSHANG_STORE_BLOB_SIZE - 1] = (uint8_t)((crc >> 24) & 0xFF);

    tianshang_state_t restored = state;
    assert(tianshang_store_decode(blob, sizeof(blob), &restored) == TIANSHANG_STORE_INVALID);
    assert(memcmp(&restored, &state, sizeof(state)) == 0);
}

static void test_pity_counters_out_of_range_are_invalid(void)
{
    tianshang_state_t state = make_sane_state();
    uint8_t blob[TIANSHANG_STORE_BLOB_SIZE];

    // non_tian_count 到达 180 就不可能存在(179 后的下一抽必出天赏)。
    state.counters.non_tian_count = 180;
    tianshang_store_encode(&state, blob, sizeof(blob));
    {
        tianshang_state_t restored;
        assert(tianshang_store_decode(blob, sizeof(blob), &restored) == TIANSHANG_STORE_INVALID);
    }

    // fan_count 到达 20 同理(第 20 抽必升奇赏)。
    state = make_sane_state();
    state.counters.fan_count = 20;
    tianshang_store_encode(&state, blob, sizeof(blob));
    {
        tianshang_state_t restored;
        assert(tianshang_store_decode(blob, sizeof(blob), &restored) == TIANSHANG_STORE_INVALID);
    }
}

static void test_simulated_run_roundtrips(void)
{
    // 真实模拟状态必须始终能落盘并原样读回。
    tianshang_state_t state;
    tianshang_init_state(&state);

    for (int round = 0; round < 100; round++) {
        tianshang_draw_batch_t batch;
        tianshang_draw(&state, 10, &batch);

        uint8_t blob[TIANSHANG_STORE_BLOB_SIZE];
        assert(tianshang_store_encode(&state, blob, sizeof(blob)) == TIANSHANG_STORE_BLOB_SIZE);

        tianshang_state_t restored;
        assert(tianshang_store_decode(blob, sizeof(blob), &restored) == TIANSHANG_STORE_OK);
        assert(memcmp(&state, &restored, sizeof(state)) == 0);
    }

    assert(state.total_draw_count == 1000);
}

static void test_null_arguments(void)
{
    tianshang_state_t state = make_sane_state();
    uint8_t blob[TIANSHANG_STORE_BLOB_SIZE];
    tianshang_store_encode(&state, blob, sizeof(blob));

    assert(tianshang_store_decode(NULL, sizeof(blob), &state) == TIANSHANG_STORE_ERROR);
    assert(tianshang_store_decode(blob, sizeof(blob), NULL) == TIANSHANG_STORE_ERROR);
}

int main(void)
{
    test_blob_size();
    test_crc_known_vector();
    test_roundtrip();
    test_roundtrip_mount_record();
    test_encode_rejects_small_buffer();
    test_blank_flash_is_not_found();
    test_wrong_size_is_invalid();
    test_corrupt_byte_is_invalid();
    test_version_mismatch_is_invalid();
    test_insane_payload_is_invalid();
    test_pity_counters_out_of_range_are_invalid();
    test_simulated_run_roundtrips();
    test_null_arguments();

    printf("test_tianshang_store: ALL TESTS PASSED\n");
    return 0;
}
