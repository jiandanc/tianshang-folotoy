#ifndef TIANSHANG_STORE_H
#define TIANSHANG_STORE_H

#include <stddef.h>
#include <stdint.h>

#include "tianshang_sim.h"

// 落盘记录 = 8B 头部 + 40B 负载 + 4B CRC32 = 52B
#define TIANSHANG_STORE_MAGIC         0x54534753u  // 4 字节魔数标记
#define TIANSHANG_STORE_VERSION       1
#define TIANSHANG_STORE_PAYLOAD_SIZE  40
#define TIANSHANG_STORE_BLOB_SIZE     (8 + TIANSHANG_STORE_PAYLOAD_SIZE + 4)

typedef enum {
    TIANSHANG_STORE_OK = 0,
    TIANSHANG_STORE_NOT_FOUND,  // 还没有记录(首次开机，或刚清空过)
    TIANSHANG_STORE_INVALID,    // 记录损坏、版本不符或数值越界
    TIANSHANG_STORE_ERROR       // 存储后端本身失败
} tianshang_store_result_t;

// --- 纯序列化层:不依赖 ESP-IDF，字段按小端显式编解码，可被主机测试覆盖 ---

// 打包 state，成功时返回写入的字节数(恒为 TIANSHANG_STORE_BLOB_SIZE)，缓冲区不足返回 0。
// 只负责编码，不校验 state 合法性 —— 调用方负责只传合法状态。
size_t tianshang_store_encode(const tianshang_state_t *state, uint8_t *blob, size_t blob_size);

// 解包并校验 blob；仅在返回 TIANSHANG_STORE_OK 时写入 state，
// 其余情况(含 INVALID/ERROR)不改动 state。校验失败拒绝恢复，避免把损坏数据当成真实记录。
tianshang_store_result_t tianshang_store_decode(const uint8_t *blob, size_t blob_size,
                                                tianshang_state_t *state);

// CRC-32/ISO-HDLC (poly 0xEDB88320)，用于检测记录损坏。
uint32_t tianshang_store_crc32(const uint8_t *data, size_t len);

#ifdef ESP_PLATFORM
// --- NVS 持久化层:命名空间 "ts_gacha"，键 "state" ---
// 仅在返回 TIANSHANG_STORE_OK 时写入 state。
tianshang_store_result_t tianshang_store_load(tianshang_state_t *state);
tianshang_store_result_t tianshang_store_save(const tianshang_state_t *state);
tianshang_store_result_t tianshang_store_clear(void);
#endif

#endif // TIANSHANG_STORE_H
