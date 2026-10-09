#ifndef TIANSHANG_UI_H
#define TIANSHANG_UI_H

#include "lvgl.h"
#include "tianshang_sim.h"

LV_FONT_DECLARE(font_tianshang_14);
LV_FONT_DECLARE(font_tianshang_20);

typedef enum {
    UI_VIEW_MAIN = 0,    // 主抽卡大厅
    UI_VIEW_RESULT,      // 抽卡结果弹窗 (贴合小程序恭喜获得界面)
    UI_VIEW_STATS        // 战报统计面板
} ui_view_t;

void tianshang_ui_init(const tianshang_state_t *state);
void tianshang_ui_update_battery(int soc);
void tianshang_ui_on_single_draw(const tianshang_roll_result_t *result, const tianshang_state_t *state);
void tianshang_ui_on_ten_draw(const tianshang_draw_batch_t *batch, const tianshang_state_t *state);
void tianshang_ui_close_result(const tianshang_state_t *state);
void tianshang_ui_toggle_stats(const tianshang_state_t *state);
void tianshang_ui_reset(const tianshang_state_t *state);
ui_view_t tianshang_ui_get_view(void);

#endif // TIANSHANG_UI_H
