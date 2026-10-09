#include "tianshang_ui.h"
#include <stdio.h>
#include <string.h>
#include "tianshang_img_assets.h"

#define COLOR_GOLD          0xFFC53D
#define COLOR_GOLD_LIGHT    0xFFE6B0
#define COLOR_GOLD_DARK     0xD48806
#define COLOR_PURPLE        0xB37FEB
#define COLOR_WHITE         0xFFFFFF
#define COLOR_SILVER        0xD9D9D9
#define COLOR_MUTED         0x8C8C8C
#define COLOR_GREEN         0x87D068
#define COLOR_RED           0xFF4D4F

// Root screen and background
static lv_obj_t *s_screen = NULL;
static lv_obj_t *s_bg_img = NULL;

// Header elements (Main view)
static lv_obj_t *s_main_view = NULL;
static lv_obj_t *s_title_label = NULL;
static lv_obj_t *s_battery_label = NULL;
static lv_obj_t *s_title_badge = NULL;
static lv_obj_t *s_title_badge_label = NULL;

// Top Status Strip (Main view: 抽数 0  祥瑞 0  天赏石 0)
static lv_obj_t *s_status_card = NULL;
static lv_obj_t *s_status_label = NULL;

// Showcase center panel (Main view: 天赏预览, 3 icons, 开鉴x次内必出天赏)
static lv_obj_t *s_showcase_panel = NULL;
static lv_obj_t *s_preview_cards[3] = {0};
static lv_obj_t *s_preview_icons[3] = {0};
static lv_obj_t *s_main_pity_countdown = NULL;

// Main bottom action bar
static lv_obj_t *s_main_action_bar = NULL;
static lv_obj_t *s_main_btn_single_icon = NULL;
static lv_obj_t *s_main_btn_single_label = NULL;
static lv_obj_t *s_main_btn_ten_icon = NULL;
static lv_obj_t *s_main_btn_ten_label = NULL;
static lv_obj_t *s_main_btn_stats_label = NULL;

// --- Result View ---
static lv_obj_t *s_result_view = NULL;
static lv_obj_t *s_result_title = NULL;
static lv_obj_t *s_res_pity_countdown = NULL;

// Single draw card in Result View (80x80)
static lv_obj_t *s_single_box = NULL;
static lv_obj_t *s_single_card_bg = NULL;
static lv_obj_t *s_single_item_icon = NULL;
static lv_obj_t *s_single_count_label = NULL;

// Ten draw grid in Result View (38x38 cards)
static lv_obj_t *s_ten_box = NULL;
static lv_obj_t *s_ten_card_boxes[10] = {0};
static lv_obj_t *s_ten_card_bgs[10] = {0};
static lv_obj_t *s_ten_item_icons[10] = {0};
static lv_obj_t *s_ten_count_labels[10] = {0};

// Result bottom action bar
static lv_obj_t *s_result_action_bar = NULL;
static lv_obj_t *s_res_btn_single_icon = NULL;
static lv_obj_t *s_res_btn_single_label = NULL;
static lv_obj_t *s_res_btn_ten_icon = NULL;
static lv_obj_t *s_res_btn_ten_label = NULL;
static lv_obj_t *s_res_btn_close_label = NULL;

// --- Stats View ---
static lv_obj_t *s_stats_view = NULL;
static lv_obj_t *s_stat_title = NULL;
static lv_obj_t *s_stat_lines[6] = {0};
static lv_obj_t *s_stat_wenyu_icon = NULL;
static lv_obj_t *s_stat_extra2 = NULL;
static lv_obj_t *s_stat_hint = NULL;

static ui_view_t s_current_view = UI_VIEW_MAIN;

static void update_status_labels(const tianshang_state_t *state)
{
    if (!state) return;

    // Title badge: 仅展示称号名称，无需“称号:”
    const char *title_str = tianshang_get_title_by_draws(state->total_draw_count);
    lv_label_set_text(s_title_badge_label, title_str);

    // Top status strip: 抽数 0  祥瑞 0  天赏石 0 (数字金色，文字银白)
    if (s_status_label) {
        lv_label_set_text_fmt(s_status_label, "抽数 #ffc53d %lu#  祥瑞 #ffc53d %lu#  天赏石 #ffc53d %lu#",
                              (unsigned long)state->total_draw_count,
                              (unsigned long)state->total_mount_count,
                              (unsigned long)state->total_stone_count);
    }

    // Remaining pity countdown: 开鉴x次内必出天赏 (x为金色，其他字体为白色)
    uint16_t non_tian = state->counters.non_tian_count;
    uint16_t remaining_pity = (non_tian >= 180) ? 0 : (180 - non_tian);

    if (s_main_pity_countdown) {
        lv_label_set_text_fmt(s_main_pity_countdown, "开鉴#ffc53d %u#次内必出天赏", remaining_pity);
    }
    if (s_res_pity_countdown) {
        lv_label_set_text_fmt(s_res_pity_countdown, "开鉴#ffc53d %u#次内必出天赏", remaining_pity);
    }
}

static void update_stats_view(const tianshang_state_t *state)
{
    if (!state) return;

    char first_mount_buf[24];
    if (state->first_mount_draw > 0) {
        snprintf(first_mount_buf, sizeof(first_mount_buf), "%lu抽", (unsigned long)state->first_mount_draw);
    } else {
        snprintf(first_mount_buf, sizeof(first_mount_buf), "暂无");
    }

    char min_pity_buf[24];
    if (state->min_tian_pity > 0) {
        snprintf(min_pity_buf, sizeof(min_pity_buf), "%u抽", state->min_tian_pity);
    } else {
        snprintf(min_pity_buf, sizeof(min_pity_buf), "暂无");
    }

    uint32_t spent_wenyu = state->total_draw_count * 180;

    // 单列垂直布局：顺序严格对齐原型图
    // (1.累计抽数 2.累计天赏 3.首次祥瑞 4.直出祥瑞 5.直出天赏 6.最佳保底)
    // 累计天赏按天赏石折算：直出天赏石 + 祥瑞 x3，与小程序口径一致。
    lv_label_set_text_fmt(s_stat_lines[0], "累计抽数:  #ffe6b0 %lu次#", (unsigned long)state->total_draw_count);
    lv_label_set_text_fmt(s_stat_lines[1], "累计天赏:  #ffe6b0 %lu个#", (unsigned long)tianshang_get_total_tian_stones(state));
    lv_label_set_text_fmt(s_stat_lines[2], "首次祥瑞:  #ffe6b0 %s#", first_mount_buf);
    lv_label_set_text_fmt(s_stat_lines[3], "直出祥瑞:  #ffe6b0 %lu个#", (unsigned long)state->total_mount_count);
    lv_label_set_text_fmt(s_stat_lines[4], "直出天赏:  #ffe6b0 %lu个#", (unsigned long)state->total_stone_count);
    lv_label_set_text_fmt(s_stat_lines[5], "最佳保底:  #ffe6b0 %s#", min_pity_buf);

    // 消耗（去掉了 ≈xx元）
    lv_label_set_text_fmt(s_stat_extra2, "消耗:  #ffe6b0 %lu 纹玉#", (unsigned long)spent_wenyu);
}

void tianshang_ui_update_battery(int soc)
{
    if (!s_battery_label) return;

    if (soc < 0) {
        lv_label_set_text(s_battery_label, "--");
        lv_obj_set_style_text_color(s_battery_label, lv_color_hex(COLOR_MUTED), 0);
    } else {
        lv_label_set_text_fmt(s_battery_label, "%d%%", soc);
        if (soc <= 20) {
            lv_obj_set_style_text_color(s_battery_label, lv_color_hex(COLOR_RED), 0);
        } else {
            lv_obj_set_style_text_color(s_battery_label, lv_color_hex(COLOR_GREEN), 0);
        }
    }
}

void tianshang_ui_init(const tianshang_state_t *state)
{
    s_screen = lv_obj_create(NULL);
    lv_obj_remove_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(s_screen, 0, 0);
    lv_obj_set_style_border_width(s_screen, 0, 0);

    // 0. Fullscreen Authentic Game Wallpaper
    s_bg_img = lv_image_create(s_screen);
    lv_image_set_src(s_bg_img, &img_tianshang_bg);
    lv_obj_set_pos(s_bg_img, 0, 0);

    // =========================================================================
    // 1. MAIN VIEW (主抽卡大厅)
    // =========================================================================
    s_main_view = lv_obj_create(s_screen);
    lv_obj_remove_flag(s_main_view, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(s_main_view, 240, 320);
    lv_obj_set_pos(s_main_view, 0, 0);
    lv_obj_set_style_bg_opa(s_main_view, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_main_view, 0, 0);
    lv_obj_set_style_pad_all(s_main_view, 0, 0);

    // Header: Title & Battery
    s_title_label = lv_label_create(s_main_view);
    lv_label_set_text(s_title_label, "神石鉴华");
    lv_obj_set_style_text_font(s_title_label, &font_tianshang_20, 0);
    lv_obj_set_style_text_color(s_title_label, lv_color_hex(COLOR_GOLD), 0);
    lv_obj_set_pos(s_title_label, 12, 4);

    // Header: Title, Badge & Battery on the SAME ROW (还原原型图布局)
    s_title_label = lv_label_create(s_main_view);
    lv_label_set_text(s_title_label, "神石鉴华");
    lv_obj_set_style_text_font(s_title_label, &font_tianshang_20, 0);
    lv_obj_set_style_text_color(s_title_label, lv_color_hex(COLOR_GOLD), 0);
    lv_obj_set_pos(s_title_label, 10, 5);

    // Title Badge (紧跟在“神石鉴华”右侧同一行，无需“称号:”二字)
    s_title_badge = lv_obj_create(s_main_view);
    lv_obj_remove_flag(s_title_badge, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(s_title_badge, LV_SIZE_CONTENT, 18);
    lv_obj_set_pos(s_title_badge, 88, 7);
    lv_obj_set_style_bg_color(s_title_badge, lv_color_hex(0x282010), 0);
    lv_obj_set_style_bg_opa(s_title_badge, LV_OPA_80, 0);
    lv_obj_set_style_border_color(s_title_badge, lv_color_hex(0xC59B4B), 0);
    lv_obj_set_style_border_width(s_title_badge, 1, 0);
    lv_obj_set_style_radius(s_title_badge, 9, 0);
    lv_obj_set_style_pad_hor(s_title_badge, 6, 0);
    lv_obj_set_style_pad_ver(s_title_badge, 0, 0);

    s_title_badge_label = lv_label_create(s_title_badge);
    lv_label_set_text(s_title_badge_label, "初游江湖");
    lv_obj_set_style_text_font(s_title_badge_label, &font_tianshang_14, 0);
    lv_obj_set_style_text_color(s_title_badge_label, lv_color_hex(COLOR_GOLD_LIGHT), 0);
    lv_obj_align(s_title_badge_label, LV_ALIGN_CENTER, 0, 0);

    s_battery_label = lv_label_create(s_main_view);
    lv_label_set_text(s_battery_label, "--");
    lv_obj_set_style_text_font(s_battery_label, &font_tianshang_14, 0);
    lv_obj_set_style_text_color(s_battery_label, lv_color_hex(COLOR_MUTED), 0);
    lv_obj_set_pos(s_battery_label, 196, 7);

    // Top Status Strip (抽数 0  祥瑞 0  天赏石 0: 黑色背景矩形，去掉金边)
    s_status_card = lv_obj_create(s_main_view);
    lv_obj_remove_flag(s_status_card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(s_status_card, 220, 24);
    lv_obj_set_pos(s_status_card, 10, 32);
    lv_obj_set_style_bg_color(s_status_card, lv_color_hex(0x0C0D15), 0);
    lv_obj_set_style_bg_opa(s_status_card, LV_OPA_80, 0);
    lv_obj_set_style_border_width(s_status_card, 0, 0);
    lv_obj_set_style_radius(s_status_card, 4, 0);
    lv_obj_set_style_pad_all(s_status_card, 0, 0);

    s_status_label = lv_label_create(s_status_card);
    lv_label_set_recolor(s_status_label, true);
    lv_label_set_text(s_status_label, "抽数 #ffc53d 0#  祥瑞 #ffc53d 0#  天赏石 #ffc53d 0#");
    lv_obj_set_style_text_font(s_status_label, &font_tianshang_14, 0);
    lv_obj_set_style_text_color(s_status_label, lv_color_hex(COLOR_SILVER), 0);
    lv_obj_align(s_status_label, LV_ALIGN_CENTER, 0, 0);

    // Center Showcase Area (去掉半透明黑色的背景矩形与边框，整体向下移到按键上方)
    s_showcase_panel = lv_obj_create(s_main_view);
    lv_obj_remove_flag(s_showcase_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(s_showcase_panel, 220, 114);
    lv_obj_set_pos(s_showcase_panel, 10, 162);
    lv_obj_set_style_bg_opa(s_showcase_panel, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_showcase_panel, 0, 0);
    lv_obj_set_style_pad_all(s_showcase_panel, 0, 0);

    // Title: 天赏预览 (纯文本，无额外左右图标)
    lv_obj_t *showcase_title = lv_label_create(s_showcase_panel);
    lv_label_set_text(showcase_title, "天赏预览");
    lv_obj_set_style_text_font(showcase_title, &font_tianshang_14, 0);
    lv_obj_set_style_text_color(showcase_title, lv_color_hex(COLOR_GOLD_LIGHT), 0);
    lv_obj_align(showcase_title, LV_ALIGN_TOP_MID, 0, 6);

    // 3 Featured items preview cards (月泉衔碧, 天赏石, 丹阙霓天): 图标减小30%(38x38)，去掉文字名称
    const int preview_item_ids[3] = { 1, 0, 2 };
    const int card_x_positions[3] = { 25, 91, 157 };

    for (int i = 0; i < 3; i++) {
        s_preview_cards[i] = lv_obj_create(s_showcase_panel);
        lv_obj_remove_flag(s_preview_cards[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(s_preview_cards[i], 38, 38);
        lv_obj_set_pos(s_preview_cards[i], card_x_positions[i], 28);
        lv_obj_set_style_radius(s_preview_cards[i], 6, 0);
        lv_obj_set_style_pad_all(s_preview_cards[i], 0, 0);
        lv_obj_set_style_border_width(s_preview_cards[i], 0, 0);

        lv_obj_t *bg = lv_image_create(s_preview_cards[i]);
        lv_image_set_src(bg, &img_card_bg_sm_juepin);
        lv_obj_set_pos(bg, 0, 0);

        s_preview_icons[i] = lv_image_create(s_preview_cards[i]);
        lv_image_set_src(s_preview_icons[i], tianshang_get_item_img_sm(preview_item_ids[i]));
        lv_obj_set_pos(s_preview_icons[i], 7, 7);
    }

    // Pity countdown: 开鉴x次内必出天赏 (x为金色，其余字体为白色)
    s_main_pity_countdown = lv_label_create(s_showcase_panel);
    lv_label_set_recolor(s_main_pity_countdown, true);
    lv_label_set_text(s_main_pity_countdown, "开鉴#ffc53d 180#次内必出天赏");
    lv_obj_set_style_text_font(s_main_pity_countdown, &font_tianshang_14, 0);
    lv_obj_set_style_text_color(s_main_pity_countdown, lv_color_hex(COLOR_WHITE), 0);
    lv_obj_align(s_main_pity_countdown, LV_ALIGN_BOTTOM_MID, 0, -6);

    // Main bottom action bar (保持现有对的样式，无xxxx纹玉)
    s_main_action_bar = lv_obj_create(s_main_view);
    lv_obj_remove_flag(s_main_action_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(s_main_action_bar, 220, 32);
    lv_obj_set_pos(s_main_action_bar, 10, 282);
    lv_obj_set_style_bg_color(s_main_action_bar, lv_color_hex(0x10111B), 0);
    lv_obj_set_style_bg_opa(s_main_action_bar, LV_OPA_80, 0);
    lv_obj_set_style_border_color(s_main_action_bar, lv_color_hex(COLOR_GOLD_DARK), 0);
    lv_obj_set_style_border_width(s_main_action_bar, 1, 0);
    lv_obj_set_style_radius(s_main_action_bar, 6, 0);
    lv_obj_set_style_pad_all(s_main_action_bar, 2, 0);

    s_main_btn_single_icon = lv_image_create(s_main_action_bar);
    lv_image_set_src(s_main_btn_single_icon, &img_icon_daibi);
    lv_obj_set_pos(s_main_btn_single_icon, 4, 4);

    s_main_btn_single_label = lv_label_create(s_main_action_bar);
    lv_label_set_text(s_main_btn_single_label, "▲单抽");
    lv_obj_set_style_text_font(s_main_btn_single_label, &font_tianshang_14, 0);
    lv_obj_set_style_text_color(s_main_btn_single_label, lv_color_hex(COLOR_GOLD), 0);
    lv_obj_set_pos(s_main_btn_single_label, 26, 4);

    s_main_btn_ten_icon = lv_image_create(s_main_action_bar);
    lv_image_set_src(s_main_btn_ten_icon, &img_icon_daibi);
    lv_obj_set_pos(s_main_btn_ten_icon, 142, 4);

    s_main_btn_ten_label = lv_label_create(s_main_action_bar);
    lv_label_set_text(s_main_btn_ten_label, "▼十连");
    lv_obj_set_style_text_font(s_main_btn_ten_label, &font_tianshang_14, 0);
    lv_obj_set_style_text_color(s_main_btn_ten_label, lv_color_hex(COLOR_GOLD), 0);
    lv_obj_set_pos(s_main_btn_ten_label, 164, 4);

    s_main_btn_stats_label = lv_label_create(s_main_action_bar);
    lv_label_set_text(s_main_btn_stats_label, "●战报");
    lv_obj_set_style_text_font(s_main_btn_stats_label, &font_tianshang_14, 0);
    lv_obj_set_style_text_color(s_main_btn_stats_label, lv_color_hex(COLOR_SILVER), 0);
    lv_obj_align(s_main_btn_stats_label, LV_ALIGN_CENTER, 0, 0);


    // =========================================================================
    // 2. RESULT VIEW (抽卡结果弹窗)
    // =========================================================================
    s_result_view = lv_obj_create(s_screen);
    lv_obj_remove_flag(s_result_view, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(s_result_view, 240, 320);
    lv_obj_set_pos(s_result_view, 0, 0);
    lv_obj_set_style_bg_color(s_result_view, lv_color_hex(0x05060A), 0);
    lv_obj_set_style_bg_opa(s_result_view, LV_OPA_90, 0);
    lv_obj_set_style_border_width(s_result_view, 0, 0);
    lv_obj_set_style_pad_all(s_result_view, 0, 0);
    lv_obj_add_flag(s_result_view, LV_OBJ_FLAG_HIDDEN);

    // Title: 恭喜获得
    s_result_title = lv_label_create(s_result_view);
    lv_label_set_text(s_result_title, "恭喜获得");
    lv_obj_set_style_text_font(s_result_title, &font_tianshang_20, 0);
    lv_obj_set_style_text_color(s_result_title, lv_color_hex(COLOR_GOLD_LIGHT), 0);
    lv_obj_align(s_result_title, LV_ALIGN_TOP_MID, 0, 18);

    // --- Single Draw Card Presentation (80x80 square box) ---
    s_single_box = lv_obj_create(s_result_view);
    lv_obj_remove_flag(s_single_box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(s_single_box, 80, 80);
    lv_obj_set_pos(s_single_box, 80, 105);
    lv_obj_set_style_radius(s_single_box, 10, 0);
    lv_obj_set_style_pad_all(s_single_box, 0, 0);
    lv_obj_set_style_border_width(s_single_box, 0, 0);

    s_single_card_bg = lv_image_create(s_single_box);
    lv_obj_set_pos(s_single_card_bg, 0, 0);

    s_single_item_icon = lv_image_create(s_single_box);
    lv_obj_set_pos(s_single_item_icon, 16, 16);

    s_single_count_label = lv_label_create(s_single_box);
    lv_obj_set_style_text_font(s_single_count_label, &font_tianshang_14, 0);
    lv_obj_set_style_text_color(s_single_count_label, lv_color_hex(COLOR_WHITE), 0);
    lv_obj_align(s_single_count_label, LV_ALIGN_BOTTOM_RIGHT, -4, -2);

    // --- Ten Draw Grid (38x38 cards) ---
    s_ten_box = lv_obj_create(s_result_view);
    lv_obj_remove_flag(s_ten_box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(s_ten_box, 240, 140);
    lv_obj_set_pos(s_ten_box, 0, 85);
    lv_obj_set_style_bg_opa(s_ten_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_ten_box, 0, 0);
    lv_obj_set_style_pad_all(s_ten_box, 0, 0);
    lv_obj_add_flag(s_ten_box, LV_OBJ_FLAG_HIDDEN);

    for (int i = 0; i < 10; i++) {
        int row = (i >= 5) ? 1 : 0;
        int col = (i >= 5) ? (i - 5) : i;
        int x = 13 + col * 44;
        int y = 10 + row * 48;

        s_ten_card_boxes[i] = lv_obj_create(s_ten_box);
        lv_obj_remove_flag(s_ten_card_boxes[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(s_ten_card_boxes[i], 38, 38);
        lv_obj_set_pos(s_ten_card_boxes[i], x, y);
        lv_obj_set_style_radius(s_ten_card_boxes[i], 6, 0);
        lv_obj_set_style_pad_all(s_ten_card_boxes[i], 0, 0);
        lv_obj_set_style_border_width(s_ten_card_boxes[i], 0, 0);

        s_ten_card_bgs[i] = lv_image_create(s_ten_card_boxes[i]);
        lv_obj_set_pos(s_ten_card_bgs[i], 0, 0);

        s_ten_item_icons[i] = lv_image_create(s_ten_card_boxes[i]);
        lv_obj_set_pos(s_ten_item_icons[i], 7, 7);

        s_ten_count_labels[i] = lv_label_create(s_ten_card_boxes[i]);
        lv_obj_set_style_text_font(s_ten_count_labels[i], &font_tianshang_14, 0);
        lv_obj_set_style_text_color(s_ten_count_labels[i], lv_color_hex(COLOR_WHITE), 0);
        lv_obj_align(s_ten_count_labels[i], LV_ALIGN_BOTTOM_RIGHT, -2, 0);
    }

    // Pity countdown in Result View (位置同主界面: y=254)
    s_res_pity_countdown = lv_label_create(s_result_view);
    lv_label_set_recolor(s_res_pity_countdown, true);
    lv_label_set_text(s_res_pity_countdown, "开鉴#ffc53d 180#次内必出天赏");
    lv_obj_set_style_text_font(s_res_pity_countdown, &font_tianshang_14, 0);
    lv_obj_set_style_text_color(s_res_pity_countdown, lv_color_hex(COLOR_WHITE), 0);
    lv_obj_align(s_res_pity_countdown, LV_ALIGN_TOP_MID, 0, 254);

    // Result Action Bar (样式图标按钮文案不变，仅中间变为“●返回”)
    s_result_action_bar = lv_obj_create(s_result_view);
    lv_obj_remove_flag(s_result_action_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(s_result_action_bar, 220, 32);
    lv_obj_set_pos(s_result_action_bar, 10, 282);
    lv_obj_set_style_bg_color(s_result_action_bar, lv_color_hex(0x10111B), 0);
    lv_obj_set_style_bg_opa(s_result_action_bar, LV_OPA_80, 0);
    lv_obj_set_style_border_color(s_result_action_bar, lv_color_hex(COLOR_GOLD_DARK), 0);
    lv_obj_set_style_border_width(s_result_action_bar, 1, 0);
    lv_obj_set_style_radius(s_result_action_bar, 6, 0);
    lv_obj_set_style_pad_all(s_result_action_bar, 2, 0);

    s_res_btn_single_icon = lv_image_create(s_result_action_bar);
    lv_image_set_src(s_res_btn_single_icon, &img_icon_daibi);
    lv_obj_set_pos(s_res_btn_single_icon, 4, 4);

    s_res_btn_single_label = lv_label_create(s_result_action_bar);
    lv_label_set_text(s_res_btn_single_label, "▲单抽");
    lv_obj_set_style_text_font(s_res_btn_single_label, &font_tianshang_14, 0);
    lv_obj_set_style_text_color(s_res_btn_single_label, lv_color_hex(COLOR_GOLD), 0);
    lv_obj_set_pos(s_res_btn_single_label, 26, 4);

    s_res_btn_ten_icon = lv_image_create(s_result_action_bar);
    lv_image_set_src(s_res_btn_ten_icon, &img_icon_daibi);
    lv_obj_set_pos(s_res_btn_ten_icon, 142, 4);

    s_res_btn_ten_label = lv_label_create(s_result_action_bar);
    lv_label_set_text(s_res_btn_ten_label, "▼十连");
    lv_obj_set_style_text_font(s_res_btn_ten_label, &font_tianshang_14, 0);
    lv_obj_set_style_text_color(s_res_btn_ten_label, lv_color_hex(COLOR_GOLD), 0);
    lv_obj_set_pos(s_res_btn_ten_label, 164, 4);

    s_res_btn_close_label = lv_label_create(s_result_action_bar);
    lv_label_set_text(s_res_btn_close_label, "●返回");
    lv_obj_set_style_text_font(s_res_btn_close_label, &font_tianshang_14, 0);
    lv_obj_set_style_text_color(s_res_btn_close_label, lv_color_hex(COLOR_SILVER), 0);
    lv_obj_align(s_res_btn_close_label, LV_ALIGN_CENTER, 0, 0);


    // =========================================================================
    // 3. STATS VIEW (战报统计面板)
    // =========================================================================
    s_stats_view = lv_obj_create(s_screen);
    lv_obj_remove_flag(s_stats_view, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(s_stats_view, 220, 250);
    lv_obj_set_pos(s_stats_view, 10, 30);
    lv_obj_set_style_bg_color(s_stats_view, lv_color_hex(0x0C0D15), 0);
    lv_obj_set_style_bg_opa(s_stats_view, LV_OPA_90, 0);
    lv_obj_set_style_border_color(s_stats_view, lv_color_hex(COLOR_GOLD_DARK), 0);
    lv_obj_set_style_border_width(s_stats_view, 1, 0);
    lv_obj_set_style_radius(s_stats_view, 6, 0);
    lv_obj_set_style_pad_all(s_stats_view, 6, 0);
    lv_obj_add_flag(s_stats_view, LV_OBJ_FLAG_HIDDEN);

    s_stat_title = lv_label_create(s_stats_view);
    lv_label_set_text(s_stat_title, "【神石鉴华 · 战报统计】");
    lv_obj_set_style_text_font(s_stat_title, &font_tianshang_14, 0);
    lv_obj_set_style_text_color(s_stat_title, lv_color_hex(COLOR_GOLD), 0);
    lv_obj_align(s_stat_title, LV_ALIGN_TOP_MID, 0, 6);

    // 6项核心统计：单列自上而下展示，消除横向越界
    for (int i = 0; i < 6; i++) {
        s_stat_lines[i] = lv_label_create(s_stats_view);
        lv_label_set_recolor(s_stat_lines[i], true);
        lv_obj_set_style_text_font(s_stat_lines[i], &font_tianshang_14, 0);
        lv_obj_set_style_text_color(s_stat_lines[i], lv_color_hex(COLOR_SILVER), 0);
        lv_obj_set_pos(s_stat_lines[i], 12, 32 + i * 24);
    }

    s_stat_wenyu_icon = lv_image_create(s_stats_view);
    lv_image_set_src(s_stat_wenyu_icon, &img_icon_wenyu);
    lv_obj_set_pos(s_stat_wenyu_icon, 12, 180);

    s_stat_extra2 = lv_label_create(s_stats_view);
    lv_label_set_recolor(s_stat_extra2, true);
    lv_obj_set_style_text_font(s_stat_extra2, &font_tianshang_14, 0);
    lv_obj_set_style_text_color(s_stat_extra2, lv_color_hex(COLOR_SILVER), 0);
    lv_obj_set_pos(s_stat_extra2, 34, 182);

    s_stat_hint = lv_label_create(s_stats_view);
    lv_label_set_text(s_stat_hint, "短按 OK 返回 | 长按 OK 清空重置");
    lv_obj_set_style_text_font(s_stat_hint, &font_tianshang_14, 0);
    lv_obj_set_style_text_color(s_stat_hint, lv_color_hex(COLOR_MUTED), 0);
    lv_obj_align(s_stat_hint, LV_ALIGN_BOTTOM_MID, 0, -4);

    s_current_view = UI_VIEW_MAIN;
    lv_screen_load(s_screen);

    // 首帧就直接呈现恢复出来的历史记录，避免先闪一帧全 0 再被纠正。
    update_status_labels(state);
    update_stats_view(state);
}

void tianshang_ui_on_single_draw(const tianshang_roll_result_t *result, const tianshang_state_t *state)
{
    if (!result || !state) return;

    // Switch to Result View
    s_current_view = UI_VIEW_RESULT;
    lv_obj_add_flag(s_main_view, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_stats_view, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(s_result_view, LV_OBJ_FLAG_HIDDEN);

    lv_obj_remove_flag(s_single_box, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_ten_box, LV_OBJ_FLAG_HIDDEN);

    // Card background (80x80: bg-juepin / bg-xishi / bg-liangpin)
    int q_val = (result->quality == TIANSHANG_QUALITY_TIAN) ? 2 : ((result->quality == TIANSHANG_QUALITY_QI) ? 1 : 0);
    lv_image_set_src(s_single_card_bg, tianshang_get_card_bg_lg(q_val));

    // Item icon inside card
    lv_image_set_src(s_single_item_icon, tianshang_get_item_img_lg(result->item_id));

    // Quantity badge (bottom-right)
    if (result->item_id == TIANSHANG_ITEM_QI_STONE_20) {
        lv_label_set_text(s_single_count_label, "20");
        lv_obj_remove_flag(s_single_count_label, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(s_single_count_label, LV_OBJ_FLAG_HIDDEN);
    }

    update_status_labels(state);
}

void tianshang_ui_on_ten_draw(const tianshang_draw_batch_t *batch, const tianshang_state_t *state)
{
    if (!batch || !state) return;

    // Switch to Result View
    s_current_view = UI_VIEW_RESULT;
    lv_obj_add_flag(s_main_view, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_stats_view, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(s_result_view, LV_OBJ_FLAG_HIDDEN);

    // 当抽奖结果获得天赏物品时，只需要展示天赏物品的图标，其他的奇赏凡赏物品不需要展示
    if (batch->tian_count > 0) {
        const tianshang_roll_result_t *tian_items[10];
        int tian_count = 0;
        for (int i = 0; i < batch->count; i++) {
            if (batch->items[i].quality == TIANSHANG_QUALITY_TIAN) {
                tian_items[tian_count++] = &batch->items[i];
            }
        }

        if (tian_count == 1) {
            // 单个天赏：展示80x80大卡牌，居中亮眼展示
            lv_obj_remove_flag(s_single_box, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(s_ten_box, LV_OBJ_FLAG_HIDDEN);

            lv_image_set_src(s_single_card_bg, tianshang_get_card_bg_lg(2));
            lv_image_set_src(s_single_item_icon, tianshang_get_item_img_lg(tian_items[0]->item_id));
            lv_obj_add_flag(s_single_count_label, LV_OBJ_FLAG_HIDDEN);
        } else {
            // 多个天赏：仅在结果框展示这几个天赏，居中排列
            lv_obj_add_flag(s_single_box, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(s_ten_box, LV_OBJ_FLAG_HIDDEN);

            int total_w = tian_count * 38 + (tian_count - 1) * 8;
            int x_start = (240 - total_w) / 2;
            int y_pos = 36;

            for (int i = 0; i < 10; i++) {
                if (i < tian_count) {
                    lv_obj_remove_flag(s_ten_card_boxes[i], LV_OBJ_FLAG_HIDDEN);
                    lv_obj_set_pos(s_ten_card_boxes[i], x_start + i * 46, y_pos);
                    lv_image_set_src(s_ten_card_bgs[i], tianshang_get_card_bg_sm(2));
                    lv_image_set_src(s_ten_item_icons[i], tianshang_get_item_img_sm(tian_items[i]->item_id));
                    lv_obj_add_flag(s_ten_count_labels[i], LV_OBJ_FLAG_HIDDEN);
                } else {
                    lv_obj_add_flag(s_ten_card_boxes[i], LV_OBJ_FLAG_HIDDEN);
                }
            }
        }
    } else {
        // 无天赏：正常展示十连全部10张卡牌 (2行5列)
        lv_obj_add_flag(s_single_box, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(s_ten_box, LV_OBJ_FLAG_HIDDEN);

        for (int i = 0; i < 10; i++) {
            int row = (i >= 5) ? 1 : 0;
            int col = (i >= 5) ? (i - 5) : i;
            int x = 13 + col * 44;
            int y = 10 + row * 48;

            lv_obj_remove_flag(s_ten_card_boxes[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(s_ten_card_boxes[i], x, y);

            const tianshang_roll_result_t *res = &batch->items[i];
            int q_val = (res->quality == TIANSHANG_QUALITY_QI) ? 1 : 0;

            lv_image_set_src(s_ten_card_bgs[i], tianshang_get_card_bg_sm(q_val));
            lv_image_set_src(s_ten_item_icons[i], tianshang_get_item_img_sm(res->item_id));

            if (res->item_id == TIANSHANG_ITEM_QI_STONE_20) {
                lv_label_set_text(s_ten_count_labels[i], "20");
                lv_obj_remove_flag(s_ten_count_labels[i], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(s_ten_count_labels[i], LV_OBJ_FLAG_HIDDEN);
            }
        }
    }

    update_status_labels(state);
}

void tianshang_ui_close_result(const tianshang_state_t *state)
{
    s_current_view = UI_VIEW_MAIN;
    lv_obj_remove_flag(s_main_view, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_result_view, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_stats_view, LV_OBJ_FLAG_HIDDEN);
    update_status_labels(state);
}

void tianshang_ui_toggle_stats(const tianshang_state_t *state)
{
    if (s_current_view == UI_VIEW_STATS) {
        s_current_view = UI_VIEW_MAIN;
        lv_obj_remove_flag(s_main_view, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_result_view, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_stats_view, LV_OBJ_FLAG_HIDDEN);
        update_status_labels(state);
    } else {
        s_current_view = UI_VIEW_STATS;
        lv_obj_add_flag(s_main_view, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_result_view, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(s_stats_view, LV_OBJ_FLAG_HIDDEN);
        update_stats_view(state);
    }
}

void tianshang_ui_reset(const tianshang_state_t *state)
{
    update_status_labels(state);
    update_stats_view(state);

    s_current_view = UI_VIEW_MAIN;
    lv_obj_remove_flag(s_main_view, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_result_view, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_stats_view, LV_OBJ_FLAG_HIDDEN);
}

ui_view_t tianshang_ui_get_view(void)
{
    return s_current_view;
}
