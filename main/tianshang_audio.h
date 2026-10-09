#ifndef TIANSHANG_AUDIO_H
#define TIANSHANG_AUDIO_H

#include <stdbool.h>
#include <stdint.h>
#include "tianshang_sim.h"

typedef enum {
    TIANSHANG_SFX_CLICK = 0,
    TIANSHANG_SFX_FAN,
    TIANSHANG_SFX_QI,
    TIANSHANG_SFX_TIAN
} tianshang_sfx_t;

bool tianshang_audio_init(void);
void tianshang_audio_play(tianshang_sfx_t sfx);

#endif // TIANSHANG_AUDIO_H
