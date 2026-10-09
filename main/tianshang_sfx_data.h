#ifndef TIANSHANG_SFX_DATA_H
#define TIANSHANG_SFX_DATA_H

#include <stdint.h>
#include <stddef.h>

#define SFX_CLICK_SAMPLES 640
#define SFX_FAN_SAMPLES   1120
#define SFX_QI_SAMPLES    3200
#define SFX_TIAN_SAMPLES  5600

extern const int16_t s_pcm_click[SFX_CLICK_SAMPLES];
extern const int16_t s_pcm_fan[SFX_FAN_SAMPLES];
extern const int16_t s_pcm_qi[SFX_QI_SAMPLES];
extern const int16_t s_pcm_tian[SFX_TIAN_SAMPLES];

#endif
