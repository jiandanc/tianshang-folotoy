#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "tianshang_sim.h"

static void test_initial_state(void)
{
    tianshang_state_t state;
    tianshang_init_state(&state);

    assert(state.counters.non_tian_count == 0);
    assert(state.counters.fan_count == 0);
    assert(state.total_draw_count == 0);
    assert(state.total_stone_count == 0);
    assert(state.total_mount_count == 0);
    assert(state.total_tian_count == 0);
    assert(state.total_qi_count == 0);
    assert(state.total_fan_count == 0);
    assert(state.first_tian_draw == 0);
    assert(state.first_mount_draw == 0);
    assert(state.min_tian_pity == 0);
}

static void test_prob_curve(void)
{
    assert(tianshang_get_current_tian_prob(0) == 0.004f);
    assert(tianshang_get_current_tian_prob(159) == 0.004f);
    assert(tianshang_get_current_tian_prob(160) == 0.010f);
    assert(tianshang_get_current_tian_prob(161) == 0.020f);
    assert(tianshang_get_current_tian_prob(178) == 0.500f);
    assert(tianshang_get_current_tian_prob(179) == 1.000f);
    assert(tianshang_get_current_tian_prob(180) == 1.000f);
}

static void test_hard_pity_180(void)
{
    tianshang_state_t state;
    tianshang_init_state(&state);

    state.counters.non_tian_count = 179;
    state.counters.fan_count = 5;

    // Even if rand_quality is 0.999 (would normally be fan), hard pity guarantees TIAN
    tianshang_roll_result_t res = tianshang_roll_once_with_rand(&state, 0.999f, 0.5f);

    assert(res.quality == TIANSHANG_QUALITY_TIAN);
    assert(res.is_tian == true);
    assert(res.pity_used == 180);
    assert(state.counters.non_tian_count == 0);
    assert(state.counters.fan_count == 0);
    assert(state.total_draw_count == 1);
    assert(state.total_tian_count == 1);
    assert(state.first_tian_draw == 1);
}

static void test_fan_upgrade_20(void)
{
    tianshang_state_t state;
    tianshang_init_state(&state);

    state.counters.non_tian_count = 50;
    state.counters.fan_count = 19; // 19 consecutive fan pulls

    // Rolling fan (rand_quality = 0.5 > 0.044) should trigger the 20th pull upgrade to QI
    tianshang_roll_result_t res = tianshang_roll_once_with_rand(&state, 0.5f, 0.0f);

    assert(res.quality == TIANSHANG_QUALITY_QI);
    assert(res.is_tian == false);
    assert(state.counters.fan_count == 0); // resets
    assert(state.counters.non_tian_count == 51); // non-tian continues
    assert(state.total_qi_count == 1);
}

static void test_tian_item_distribution(void)
{
    tianshang_state_t state;
    tianshang_init_state(&state);

    // Stone roll (rand_quality = 0.001 < 0.004, rand_item = 0.5 < 0.90)
    tianshang_roll_result_t res1 = tianshang_roll_once_with_rand(&state, 0.001f, 0.50f);
    assert(res1.quality == TIANSHANG_QUALITY_TIAN);
    assert(res1.item_id == TIANSHANG_ITEM_STONE);
    assert(res1.is_mount == false);
    assert(res1.stone_gained == 1);

    // Yuequan mount roll (rand_quality = 0.001 < 0.004, rand_item = 0.92 in [0.90, 0.95))
    tianshang_roll_result_t res2 = tianshang_roll_once_with_rand(&state, 0.001f, 0.92f);
    assert(res2.quality == TIANSHANG_QUALITY_TIAN);
    assert(res2.item_id == TIANSHANG_ITEM_MOUNT_YUEQUAN);
    assert(res2.is_mount == true);

    // Danque mount roll (rand_quality = 0.001 < 0.004, rand_item = 0.98 >= 0.95)
    tianshang_roll_result_t res3 = tianshang_roll_once_with_rand(&state, 0.001f, 0.98f);
    assert(res3.quality == TIANSHANG_QUALITY_TIAN);
    assert(res3.item_id == TIANSHANG_ITEM_MOUNT_DANQUE);
    assert(res3.is_mount == true);
}

static void test_ten_pull_batch(void)
{
    tianshang_state_t state;
    tianshang_init_state(&state);

    tianshang_draw_batch_t batch;
    tianshang_draw(&state, 10, &batch);

    assert(batch.count == 10);
    assert(state.total_draw_count == 10);
    assert(state.total_tian_count + state.total_qi_count + state.total_fan_count == 10);
}

static void test_titles(void)
{
    assert(strcmp(tianshang_get_title_by_draws(0), "初游江湖") == 0);
    assert(strcmp(tianshang_get_title_by_draws(11), "初游江湖") == 0); // 198 RMB < 200
    assert(strcmp(tianshang_get_title_by_draws(12), "闲云自在") == 0); // 216 RMB >= 200
    assert(strcmp(tianshang_get_title_by_draws(56), "放歌清宵") == 0); // 1008 RMB >= 1000
    assert(strcmp(tianshang_get_title_by_draws(167), "凭栏听雨") == 0); // 3006 RMB >= 3000
    assert(strcmp(tianshang_get_title_by_draws(556), "把酒临风") == 0); // 10008 RMB >= 10000
    assert(strcmp(tianshang_get_title_by_draws(83334), "捧日揽月摘星辰") == 0); // > 1500000
}

static void test_total_tian_stones(void)
{
    tianshang_state_t state;
    tianshang_init_state(&state);

    assert(tianshang_get_total_tian_stones(&state) == 0);
    assert(tianshang_get_total_tian_stones(NULL) == 0);

    // 直出天赏石按 1 石计
    state.total_tian_count = 4;
    state.total_stone_count = 4;
    state.total_mount_count = 0;
    assert(tianshang_get_total_tian_stones(&state) == 4);

    // 祥瑞按 3 石折算：4 石 + 2 祥瑞 = 4 + 6 = 10
    state.total_tian_count = 6;
    state.total_mount_count = 2;
    assert(tianshang_get_total_tian_stones(&state) == 10);

    // 全为祥瑞时
    state.total_tian_count = 3;
    state.total_stone_count = 0;
    state.total_mount_count = 3;
    assert(tianshang_get_total_tian_stones(&state) == 9);
}

int main(void)
{
    test_initial_state();
    test_prob_curve();
    test_hard_pity_180();
    test_fan_upgrade_20();
    test_tian_item_distribution();
    test_ten_pull_batch();
    test_titles();
    test_total_tian_stones();

    printf("test_tianshang_sim: ALL TESTS PASSED\n");
    return 0;
}
