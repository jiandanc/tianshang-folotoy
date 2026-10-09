#include "tianshang_sim.h"
#include <string.h>

#ifdef ESP_PLATFORM
#include "esp_random.h"
#else
#include <stdlib.h>
#endif

// 161-179 抽天赏软保底递增表（社区拟合通用参数，数学期望约 101-105 抽/颗天赏石）
static const float s_tian_prob_table[19] = {
    0.010f, 0.020f, 0.030f, 0.040f, 0.055f,
    0.070f, 0.085f, 0.100f, 0.120f, 0.140f,
    0.165f, 0.190f, 0.220f, 0.250f, 0.285f,
    0.320f, 0.360f, 0.420f, 0.500f
};

static const tianshang_item_info_t s_item_info[TIANSHANG_ITEM_COUNT] = {
    [TIANSHANG_ITEM_STONE] = {
        .id = TIANSHANG_ITEM_STONE,
        .name = "天赏石",
        .quality = TIANSHANG_QUALITY_TIAN,
        .token_reward = 1,
        .is_mount = false,
        .is_token = true
    },
    [TIANSHANG_ITEM_MOUNT_YUEQUAN] = {
        .id = TIANSHANG_ITEM_MOUNT_YUEQUAN,
        .name = "月泉衔碧",
        .quality = TIANSHANG_QUALITY_TIAN,
        .token_reward = 3,
        .is_mount = true,
        .is_token = false
    },
    [TIANSHANG_ITEM_MOUNT_DANQUE] = {
        .id = TIANSHANG_ITEM_MOUNT_DANQUE,
        .name = "丹阙霓天",
        .quality = TIANSHANG_QUALITY_TIAN,
        .token_reward = 3,
        .is_mount = true,
        .is_token = false
    },
    [TIANSHANG_ITEM_QI_STONE_20] = {
        .id = TIANSHANG_ITEM_QI_STONE_20,
        .name = "奇赏石*20",
        .quality = TIANSHANG_QUALITY_QI,
        .token_reward = 0,
        .is_mount = false,
        .is_token = false
    },
    [TIANSHANG_ITEM_FAN_KITE] = {
        .id = TIANSHANG_ITEM_FAN_KITE,
        .name = "风筝",
        .quality = TIANSHANG_QUALITY_FAN,
        .token_reward = 0,
        .is_mount = false,
        .is_token = false
    },
    [TIANSHANG_ITEM_FAN_QINGRONG] = {
        .id = TIANSHANG_ITEM_FAN_QINGRONG,
        .name = "轻容染",
        .quality = TIANSHANG_QUALITY_FAN,
        .token_reward = 0,
        .is_mount = false,
        .is_token = false
    },
    [TIANSHANG_ITEM_FAN_LUODAI] = {
        .id = TIANSHANG_ITEM_FAN_LUODAI,
        .name = "螺黛染",
        .quality = TIANSHANG_QUALITY_FAN,
        .token_reward = 0,
        .is_mount = false,
        .is_token = false
    },
    [TIANSHANG_ITEM_FAN_ZHAOYE] = {
        .id = TIANSHANG_ITEM_FAN_ZHAOYE,
        .name = "照夜•花拥香雾",
        .quality = TIANSHANG_QUALITY_FAN,
        .token_reward = 0,
        .is_mount = false,
        .is_token = false
    },
    [TIANSHANG_ITEM_FAN_YINGYUE] = {
        .id = TIANSHANG_ITEM_FAN_YINGYUE,
        .name = "映月•荧霓旌",
        .quality = TIANSHANG_QUALITY_FAN,
        .token_reward = 0,
        .is_mount = false,
        .is_token = false
    },
    [TIANSHANG_ITEM_FAN_YAOXING] = {
        .id = TIANSHANG_ITEM_FAN_YAOXING,
        .name = "摇星•落花旋",
        .quality = TIANSHANG_QUALITY_FAN,
        .token_reward = 0,
        .is_mount = false,
        .is_token = false
    },
    [TIANSHANG_ITEM_FAN_GUICHU] = {
        .id = TIANSHANG_ITEM_FAN_GUICHU,
        .name = "归初凝丹",
        .quality = TIANSHANG_QUALITY_FAN,
        .token_reward = 0,
        .is_mount = false,
        .is_token = false
    },
    [TIANSHANG_ITEM_FAN_QI_STONE] = {
        .id = TIANSHANG_ITEM_FAN_QI_STONE,
        .name = "奇赏石",
        .quality = TIANSHANG_QUALITY_FAN,
        .token_reward = 0,
        .is_mount = false,
        .is_token = false
    }
};

typedef struct {
    uint32_t min_rmb;
    const char *title;
} title_tier_t;

static const title_tier_t s_title_tiers[] = {
    { 1500000, "捧日揽月摘星辰" },
    { 1000000, "缥缈云流鹤上仙" },
    {  500000, "乘风登玉京" },
    {  200000, "秋水不染尘" },
    {  100000, "一醉轻王侯" },
    {   50000, "卧看千山" },
    {   10000, "把酒临风" },
    {    3000, "凭栏听雨" },
    {    1000, "放歌清宵" },
    {     200, "闲云自在" },
    {       0, "初游江湖" }
};

static float get_random_float(void)
{
#ifdef ESP_PLATFORM
    return (float)esp_random() / (float)UINT32_MAX;
#else
    return (float)rand() / ((float)RAND_MAX + 1.0f);
#endif
}

void tianshang_init_state(tianshang_state_t *state)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
}

const tianshang_item_info_t *tianshang_get_item_info(tianshang_item_id_t id)
{
    if (id >= TIANSHANG_ITEM_COUNT) {
        return &s_item_info[TIANSHANG_ITEM_FAN_KITE];
    }
    return &s_item_info[id];
}

const char *tianshang_get_quality_name(tianshang_quality_t q)
{
    switch (q) {
        case TIANSHANG_QUALITY_TIAN: return "天赏";
        case TIANSHANG_QUALITY_QI:   return "奇赏";
        default:                     return "凡赏";
    }
}

const char *tianshang_get_title_by_draws(uint32_t total_draws)
{
    // 每抽 180 纹玉 = 18 元 RMB
    uint32_t rmb = total_draws * 18;
    for (size_t i = 0; i < sizeof(s_title_tiers) / sizeof(s_title_tiers[0]); i++) {
        if (rmb >= s_title_tiers[i].min_rmb) {
            return s_title_tiers[i].title;
        }
    }
    return "初游江湖";
}

float tianshang_get_current_tian_prob(uint16_t non_tian_count)
{
    if (non_tian_count >= 179) {
        return 1.000f; // 180 抽硬保底
    }
    if (non_tian_count < TIANSHANG_SOFT_PITY_START) {
        return 0.004f;
    }
    if (non_tian_count <= 178) {
        uint16_t idx = non_tian_count - TIANSHANG_SOFT_PITY_START;
        return s_tian_prob_table[idx];
    }
    return 0.500f;
}

uint32_t tianshang_get_total_tian_stones(const tianshang_state_t *state)
{
    if (!state) return 0;
    // 直出天赏石 x1；祥瑞按 token_reward 折算(每件 3 石)，与小程序
    // rollTianshangDraw 里 newTianRewardGained 的口径一致。
    return state->total_stone_count + state->total_mount_count * 3u;
}

tianshang_roll_result_t tianshang_roll_once_with_rand(
    tianshang_state_t *state,
    float rand_quality,
    float rand_item
) {
    tianshang_roll_result_t result;
    memset(&result, 0, sizeof(result));

    uint16_t non_tian = state->counters.non_tian_count;
    uint8_t fan_count = state->counters.fan_count;
    tianshang_quality_t quality = TIANSHANG_QUALITY_FAN;

    // 1. 180 抽硬保底判定
    if (non_tian >= 179) {
        quality = TIANSHANG_QUALITY_TIAN;
    } else {
        // 2. 获取当前生效天赏概率 pTian
        float p_tian = tianshang_get_current_tian_prob(non_tian);
        if (rand_quality < p_tian) {
            quality = TIANSHANG_QUALITY_TIAN;
        } else if (rand_quality < p_tian + 0.04f) {
            quality = TIANSHANG_QUALITY_QI;
        } else {
            quality = TIANSHANG_QUALITY_FAN;
        }
    }

    // 3. 更新保底计数器
    uint16_t new_non_tian = non_tian;
    uint8_t new_fan = fan_count;

    if (quality == TIANSHANG_QUALITY_TIAN) {
        result.pity_used = non_tian + 1;
        new_non_tian = 0;
        new_fan = 0;
    } else if (quality == TIANSHANG_QUALITY_QI) {
        result.pity_used = 0;
        new_non_tian++;
        new_fan = 0;
    } else {
        result.pity_used = 0;
        new_non_tian++;
        new_fan++;
    }

    // 4. 20 抽凡赏保底升级判定
    if (new_fan == TIANSHANG_FAN_UPGRADE_LIMIT && quality == TIANSHANG_QUALITY_FAN) {
        quality = TIANSHANG_QUALITY_QI;
        new_fan = 0;
    }

    state->counters.non_tian_count = new_non_tian;
    state->counters.fan_count = new_fan;

    // 5. 确定物品产出
    tianshang_item_id_t item_id;
    if (quality == TIANSHANG_QUALITY_TIAN) {
        // 权重：天赏石 0.0036 : 月泉衔碧 0.0002 : 丹阙霓天 0.0002 (36 : 2 : 2)
        // 归一化至 [0, 1)
        if (rand_item < 0.90f) {
            item_id = TIANSHANG_ITEM_STONE;
        } else if (rand_item < 0.95f) {
            item_id = TIANSHANG_ITEM_MOUNT_YUEQUAN;
        } else {
            item_id = TIANSHANG_ITEM_MOUNT_DANQUE;
        }
    } else if (quality == TIANSHANG_QUALITY_QI) {
        item_id = TIANSHANG_ITEM_QI_STONE_20;
    } else {
        // 凡赏 8 种物品等概率 (每种 12.5%)
        int fan_idx = (int)(rand_item * 8.0f);
        if (fan_idx < 0) fan_idx = 0;
        if (fan_idx > 7) fan_idx = 7;
        item_id = (tianshang_item_id_t)(TIANSHANG_ITEM_FAN_KITE + fan_idx);
    }

    const tianshang_item_info_t *info = &s_item_info[item_id];
    result.item_id = item_id;
    result.quality = quality;
    result.is_tian = (quality == TIANSHANG_QUALITY_TIAN);
    result.is_mount = info->is_mount;
    result.stone_gained = info->is_token ? 1 : 0; // 祥瑞不视为直接获得石头（获得的是祥瑞，拆解才得3石）

    // 6. 更新全局统计
    state->total_draw_count++;
    if (result.is_tian) {
        state->total_tian_count++;
        if (state->first_tian_draw == 0) {
            state->first_tian_draw = state->total_draw_count;
        }
        if (result.is_mount) {
            state->total_mount_count++;
            if (state->first_mount_draw == 0) {
                state->first_mount_draw = state->total_draw_count;
            }
        }
        if (result.pity_used > 0) {
            if (state->min_tian_pity == 0 || result.pity_used < state->min_tian_pity) {
                state->min_tian_pity = result.pity_used;
            }
        }
    } else if (quality == TIANSHANG_QUALITY_QI) {
        state->total_qi_count++;
    } else {
        state->total_fan_count++;
    }
    state->total_stone_count += result.stone_gained;

    return result;
}

void tianshang_draw(
    tianshang_state_t *state,
    uint8_t count,
    tianshang_draw_batch_t *batch
) {
    if (!state || !batch) return;
    if (count != 1 && count != 10) count = 1;

    memset(batch, 0, sizeof(*batch));
    batch->count = count;

    for (uint8_t i = 0; i < count; i++) {
        float r_q = get_random_float();
        float r_i = get_random_float();
        tianshang_roll_result_t res = tianshang_roll_once_with_rand(state, r_q, r_i);
        batch->items[i] = res;
        if (res.is_tian) {
            batch->tian_count++;
            if (res.pity_used > 0) {
                if (batch->best_pity_used == 0 || res.pity_used < batch->best_pity_used) {
                    batch->best_pity_used = res.pity_used;
                }
            }
        }
        if (res.is_mount) {
            batch->mount_count++;
        }
        batch->stone_gained += res.stone_gained;
    }
}
