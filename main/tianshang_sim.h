#ifndef TIANSHANG_SIM_H
#define TIANSHANG_SIM_H

#include <stdint.h>
#include <stdbool.h>

#define TIANSHANG_HARD_PITY_LIMIT      180
#define TIANSHANG_SOFT_PITY_START      160
#define TIANSHANG_FAN_UPGRADE_LIMIT     20

typedef enum {
    TIANSHANG_QUALITY_FAN = 0,     // 凡赏
    TIANSHANG_QUALITY_QI = 1,      // 奇赏
    TIANSHANG_QUALITY_TIAN = 2     // 天赏
} tianshang_quality_t;

typedef enum {
    // 天赏物品
    TIANSHANG_ITEM_STONE = 0,         // 天赏石
    TIANSHANG_ITEM_MOUNT_YUEQUAN,     // 月泉衔碧 (祥瑞)
    TIANSHANG_ITEM_MOUNT_DANQUE,      // 丹阙霓天 (祥瑞)
    // 奇赏物品
    TIANSHANG_ITEM_QI_STONE_20,       // 奇赏石*20
    // 凡赏物品 (8件)
    TIANSHANG_ITEM_FAN_KITE,          // 风筝
    TIANSHANG_ITEM_FAN_QINGRONG,      // 轻容染
    TIANSHANG_ITEM_FAN_LUODAI,        // 螺黛染
    TIANSHANG_ITEM_FAN_ZHAOYE,        // 照夜•花拥香雾
    TIANSHANG_ITEM_FAN_YINGYUE,       // 映月•荧霓旌
    TIANSHANG_ITEM_FAN_YAOXING,       // 摇星•落花旋
    TIANSHANG_ITEM_FAN_GUICHU,        // 归初凝丹
    TIANSHANG_ITEM_FAN_QI_STONE,      // 奇赏石
    TIANSHANG_ITEM_COUNT
} tianshang_item_id_t;

typedef struct {
    tianshang_item_id_t id;
    const char *name;
    tianshang_quality_t quality;
    uint8_t token_reward;    // 1 for stone, 3 for mount, 0 otherwise
    bool is_mount;
    bool is_token;
} tianshang_item_info_t;

typedef struct {
    uint16_t non_tian_count;  // 连续未出天赏次数 (0..179)
    uint8_t fan_count;        // 连续凡赏次数 (0..19)
} tianshang_counters_t;

typedef struct {
    tianshang_item_id_t item_id;
    tianshang_quality_t quality;
    bool is_tian;
    bool is_mount;
    uint8_t stone_gained;
    uint16_t pity_used;       // 若出天赏则为当前保底抽数 (1..180)，否则为 0
} tianshang_roll_result_t;

typedef struct {
    tianshang_counters_t counters;
    uint32_t total_draw_count;
    uint32_t total_stone_count;
    uint32_t total_mount_count;
    uint32_t total_tian_count;
    uint32_t total_qi_count;
    uint32_t total_fan_count;
    uint32_t first_tian_draw;
    uint32_t first_mount_draw;
    uint16_t min_tian_pity;
} tianshang_state_t;

typedef struct {
    tianshang_roll_result_t items[10];
    uint8_t count;            // 1 或 10
    uint8_t tian_count;
    uint8_t mount_count;
    uint8_t stone_gained;
    uint16_t best_pity_used;  // 本轮抽出的天赏中最小保底数
} tianshang_draw_batch_t;

void tianshang_init_state(tianshang_state_t *state);
const tianshang_item_info_t *tianshang_get_item_info(tianshang_item_id_t id);
const char *tianshang_get_quality_name(tianshang_quality_t q);
const char *tianshang_get_title_by_draws(uint32_t total_draws);
float tianshang_get_current_tian_prob(uint16_t non_tian_count);

// 战报"累计天赏"口径:按天赏石折算，一个祥瑞等价 3 个天赏石。
// 注意与 state->total_tian_count 的区别——后者是"出天赏的次数"，
// 祥瑞和直出天赏石各记 1 次，仅用于内部统计与存档校验。
uint32_t tianshang_get_total_tian_stones(const tianshang_state_t *state);

tianshang_roll_result_t tianshang_roll_once_with_rand(
    tianshang_state_t *state,
    float rand_quality,
    float rand_item
);

void tianshang_draw(
    tianshang_state_t *state,
    uint8_t count,
    tianshang_draw_batch_t *batch
);

#endif // TIANSHANG_SIM_H
