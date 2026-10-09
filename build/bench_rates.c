/* 临时测量程序:统计固件抽奖引擎的真实概率分布。
 * 放在 build/ 下(已被 gitignore),不进入仓库。 */
#include <stdio.h>
#include <stdint.h>
#include "tianshang_sim.h"

/* splitmix64:可控、高质量,避免 libc rand() 的分布伪影 */
static uint64_t s_rng_state = 0x9E3779B97F4A7C15ull;

static uint64_t next_u64(void)
{
    uint64_t z = (s_rng_state += 0x9E3779B97F4A7C15ull);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

/* 与固件 get_random_float() 同构:[0,1] 均匀 */
static float next_float(void)
{
    return (float)(next_u64() >> 40) / (float)(1u << 24);
}

int main(void)
{
    const uint32_t N = 20000000u;

    tianshang_state_t state;
    tianshang_init_state(&state);

    uint64_t tian = 0, mount = 0, stone = 0, qi = 0, fan = 0;

    for (uint32_t i = 0; i < N; i++) {
        tianshang_roll_result_t r = tianshang_roll_once_with_rand(&state, next_float(), next_float());
        switch (r.quality) {
            case TIANSHANG_QUALITY_TIAN: tian++; break;
            case TIANSHANG_QUALITY_QI:   qi++;   break;
            default:                     fan++;  break;
        }
        if (r.is_tian) {
            if (r.is_mount) mount++;
            else            stone++;
        }
    }

    printf("=== firmware tianshang_sim (N=%u) ===\n", N);
    printf("P(天赏)            = %.6f  (1 per %.1f draws)\n", (double)tian / N, (double)N / (double)tian);
    printf("P(奇赏)            = %.6f\n", (double)qi / N);
    printf("P(凡赏)            = %.6f\n", (double)fan / N);
    printf("P(祥瑞)  overall   = %.8f\n", (double)mount / N);
    printf("P(天赏石) overall  = %.8f\n", (double)stone / N);
    printf("--- conditional on 天赏 ---\n");
    printf("P(祥瑞 | 天赏)     = %.6f  (expected 0.10)\n", (double)mount / (double)tian);
    printf("P(天赏石 | 天赏)   = %.6f  (expected 0.90)\n", (double)stone / (double)tian);
    printf("counts: tian=%llu mount=%llu stone=%llu\n",
           (unsigned long long)tian, (unsigned long long)mount, (unsigned long long)stone);
    return 0;
}
