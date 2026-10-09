#include "tianshang_store.h"

#include <string.h>

// 负载布局(小端，共 40B):
//   0  uint16 non_tian_count
//   2  uint8  fan_count
//   3  uint8  reserved (0)
//   4  uint32 total_draw_count
//   8  uint32 total_stone_count
//  12  uint32 total_mount_count
//  16  uint32 total_tian_count
//  20  uint32 total_qi_count
//  24  uint32 total_fan_count
//  28  uint32 first_tian_draw
//  32  uint32 first_mount_draw
//  36  uint16 min_tian_pity
//  38  uint16 reserved (0)
//
// 逐字段手工编解码，不直接 memcpy 结构体:结构体布局受对齐和编译器影响，
// 落盘格式必须稳定，否则固件升级后旧记录会读成乱码。
#define OFF_NON_TIAN       0
#define OFF_FAN            2
#define OFF_TOTAL_DRAW     4
#define OFF_TOTAL_STONE    8
#define OFF_TOTAL_MOUNT   12
#define OFF_TOTAL_TIAN    16
#define OFF_TOTAL_QI      20
#define OFF_TOTAL_FAN     24
#define OFF_FIRST_TIAN    28
#define OFF_FIRST_MOUNT   32
#define OFF_MIN_PITY      36

#define HEADER_SIZE   8
#define CRC_SIZE      4
#define PAYLOAD_OFF   HEADER_SIZE
#define CRC_OFF       (HEADER_SIZE + TIANSHANG_STORE_PAYLOAD_SIZE)

static void put_u16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
}

static void put_u32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
    p[2] = (uint8_t)((v >> 16) & 0xFFu);
    p[3] = (uint8_t)((v >> 24) & 0xFFu);
}

static uint16_t get_u16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t get_u32(const uint8_t *p)
{
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

uint32_t tianshang_store_crc32(const uint8_t *data, size_t len)
{
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; bit++) {
            uint32_t mask = (uint32_t)(-(int32_t)(crc & 1u));
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }
    return ~crc;
}

size_t tianshang_store_encode(const tianshang_state_t *state, uint8_t *blob, size_t blob_size)
{
    if (!state || !blob || blob_size < TIANSHANG_STORE_BLOB_SIZE) {
        return 0;
    }

    memset(blob, 0, TIANSHANG_STORE_BLOB_SIZE);

    put_u32(&blob[0], TIANSHANG_STORE_MAGIC);
    put_u16(&blob[4], TIANSHANG_STORE_VERSION);
    put_u16(&blob[6], 0); // reserved

    uint8_t *p = &blob[PAYLOAD_OFF];
    put_u16(&p[OFF_NON_TIAN], state->counters.non_tian_count);
    p[OFF_FAN] = state->counters.fan_count;
    put_u32(&p[OFF_TOTAL_DRAW], state->total_draw_count);
    put_u32(&p[OFF_TOTAL_STONE], state->total_stone_count);
    put_u32(&p[OFF_TOTAL_MOUNT], state->total_mount_count);
    put_u32(&p[OFF_TOTAL_TIAN], state->total_tian_count);
    put_u32(&p[OFF_TOTAL_QI], state->total_qi_count);
    put_u32(&p[OFF_TOTAL_FAN], state->total_fan_count);
    put_u32(&p[OFF_FIRST_TIAN], state->first_tian_draw);
    put_u32(&p[OFF_FIRST_MOUNT], state->first_mount_draw);
    put_u16(&p[OFF_MIN_PITY], state->min_tian_pity);

    put_u32(&blob[CRC_OFF], tianshang_store_crc32(blob, CRC_OFF));
    return TIANSHANG_STORE_BLOB_SIZE;
}

// 记录必须自洽才允许恢复。CRC 能挡住随机损坏，这些结构性约束再挡住
// "CRC 恰好通过但数值荒谬"的记录(例如残留数据被当成新格式读出)。
static bool payload_is_sane(const tianshang_state_t *s)
{
    // 保底计数上限:非天赏 179 抽后下一抽必出天赏，凡赏 20 抽必升奇赏。
    if (s->counters.non_tian_count >= TIANSHANG_HARD_PITY_LIMIT) return false;
    if (s->counters.fan_count >= TIANSHANG_FAN_UPGRADE_LIMIT) return false;

    // 每抽必然落入天赏/奇赏/凡赏之一，三者之和必须等于总抽数。
    if ((uint64_t)s->total_tian_count + s->total_qi_count + s->total_fan_count
        != s->total_draw_count) {
        return false;
    }

    // 天赏产出只有两类:直接得石，或得祥瑞(石头数为 0)。
    if ((uint64_t)s->total_stone_count + s->total_mount_count != s->total_tian_count) {
        return false;
    }

    // 首次记录只可能落在 [1, total_draw_count] 内，0 表示尚未出现。
    if (s->first_tian_draw > s->total_draw_count) return false;
    if (s->first_mount_draw > s->total_draw_count) return false;

    // 最佳保底来自 pity_used = non_tian + 1，取值范围 1..180。
    if (s->min_tian_pity > TIANSHANG_HARD_PITY_LIMIT) return false;

    return true;
}

tianshang_store_result_t tianshang_store_decode(const uint8_t *blob, size_t blob_size,
                                                tianshang_state_t *state)
{
    if (!blob || !state) {
        return TIANSHANG_STORE_ERROR;
    }
    if (blob_size != TIANSHANG_STORE_BLOB_SIZE) {
        return TIANSHANG_STORE_INVALID;
    }
    if (get_u32(&blob[0]) != TIANSHANG_STORE_MAGIC) {
        return TIANSHANG_STORE_NOT_FOUND;
    }
    if (get_u16(&blob[4]) != TIANSHANG_STORE_VERSION) {
        return TIANSHANG_STORE_INVALID;
    }
    if (get_u32(&blob[CRC_OFF]) != tianshang_store_crc32(blob, CRC_OFF)) {
        return TIANSHANG_STORE_INVALID;
    }

    const uint8_t *p = &blob[PAYLOAD_OFF];
    tianshang_state_t decoded;
    memset(&decoded, 0, sizeof(decoded));
    decoded.counters.non_tian_count = get_u16(&p[OFF_NON_TIAN]);
    decoded.counters.fan_count = p[OFF_FAN];
    decoded.total_draw_count = get_u32(&p[OFF_TOTAL_DRAW]);
    decoded.total_stone_count = get_u32(&p[OFF_TOTAL_STONE]);
    decoded.total_mount_count = get_u32(&p[OFF_TOTAL_MOUNT]);
    decoded.total_tian_count = get_u32(&p[OFF_TOTAL_TIAN]);
    decoded.total_qi_count = get_u32(&p[OFF_TOTAL_QI]);
    decoded.total_fan_count = get_u32(&p[OFF_TOTAL_FAN]);
    decoded.first_tian_draw = get_u32(&p[OFF_FIRST_TIAN]);
    decoded.first_mount_draw = get_u32(&p[OFF_FIRST_MOUNT]);
    decoded.min_tian_pity = get_u16(&p[OFF_MIN_PITY]);

    if (!payload_is_sane(&decoded)) {
        return TIANSHANG_STORE_INVALID;
    }

    *state = decoded;
    return TIANSHANG_STORE_OK;
}

#ifdef ESP_PLATFORM

#include "esp_log.h"
#include "nvs.h"

static const char *TAG = "ts_store";
static const char *NVS_NAMESPACE = "ts_gacha";
static const char *NVS_KEY = "state";

static tianshang_store_result_t open_nvs(nvs_open_mode_t mode, nvs_handle_t *out)
{
    esp_err_t err = nvs_open(NVS_NAMESPACE, mode, out);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        // 只读打开时命名空间尚未创建 = 从未写入过记录，不是故障。
        return TIANSHANG_STORE_NOT_FOUND;
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open(%s) failed: %s", NVS_NAMESPACE, esp_err_to_name(err));
        return TIANSHANG_STORE_ERROR;
    }
    return TIANSHANG_STORE_OK;
}

tianshang_store_result_t tianshang_store_load(tianshang_state_t *state)
{
    if (!state) return TIANSHANG_STORE_ERROR;

    nvs_handle_t handle;
    tianshang_store_result_t res = open_nvs(NVS_READONLY, &handle);
    if (res != TIANSHANG_STORE_OK) {
        return res;
    }

    uint8_t blob[TIANSHANG_STORE_BLOB_SIZE];
    size_t size = sizeof(blob);
    esp_err_t err = nvs_get_blob(handle, NVS_KEY, blob, &size);
    nvs_close(handle);

    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return TIANSHANG_STORE_NOT_FOUND;
    }
    if (err == ESP_ERR_NVS_INVALID_LENGTH) {
        // 记录比当前固件预期的大:来自更新版本或分区残留，属于格式不符而非读数故障。
        ESP_LOGW(TAG, "stored record is larger than expected; ignoring");
        return TIANSHANG_STORE_INVALID;
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_get_blob failed: %s", esp_err_to_name(err));
        return TIANSHANG_STORE_ERROR;
    }
    if (size != sizeof(blob)) {
        ESP_LOGW(TAG, "stored record size %u != %u; ignoring",
                 (unsigned)size, (unsigned)sizeof(blob));
        return TIANSHANG_STORE_INVALID;
    }

    return tianshang_store_decode(blob, size, state);
}

tianshang_store_result_t tianshang_store_save(const tianshang_state_t *state)
{
    if (!state) return TIANSHANG_STORE_ERROR;

    uint8_t blob[TIANSHANG_STORE_BLOB_SIZE];
    if (tianshang_store_encode(state, blob, sizeof(blob)) == 0) {
        return TIANSHANG_STORE_ERROR;
    }

    nvs_handle_t handle;
    tianshang_store_result_t res = open_nvs(NVS_READWRITE, &handle);
    // 只读语义的 NOT_FOUND 对写入没有意义:可写打开会创建命名空间，
    // 真走到这里只能当作后端故障上报，不能返回"无记录"。
    if (res != TIANSHANG_STORE_OK) {
        return res == TIANSHANG_STORE_NOT_FOUND ? TIANSHANG_STORE_ERROR : res;
    }

    esp_err_t err = nvs_set_blob(handle, NVS_KEY, blob, sizeof(blob));
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs write failed: %s", esp_err_to_name(err));
        return TIANSHANG_STORE_ERROR;
    }
    return TIANSHANG_STORE_OK;
}

tianshang_store_result_t tianshang_store_clear(void)
{
    nvs_handle_t handle;
    tianshang_store_result_t res = open_nvs(NVS_READWRITE, &handle);
    // 命名空间不存在 = 本来就没有记录要清，对调用方就是成功。
    if (res == TIANSHANG_STORE_NOT_FOUND) {
        return TIANSHANG_STORE_OK;
    }
    if (res != TIANSHANG_STORE_OK) {
        return res;
    }

    esp_err_t err = nvs_erase_key(handle, NVS_KEY);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);

    // 本来就没有记录，等同于已清空。
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return TIANSHANG_STORE_OK;
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_erase_key failed: %s", esp_err_to_name(err));
        return TIANSHANG_STORE_ERROR;
    }
    return TIANSHANG_STORE_OK;
}

#endif // ESP_PLATFORM
