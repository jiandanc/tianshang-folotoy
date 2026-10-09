#ifndef TIANSHANG_IMG_ASSETS_H
#define TIANSHANG_IMG_ASSETS_H

#include "lvgl.h"

extern const lv_image_dsc_t img_tianshang_bg;
extern const lv_image_dsc_t img_icon_daibi;
extern const lv_image_dsc_t img_icon_wenyu;
extern const lv_image_dsc_t img_item_lg_stone;
extern const lv_image_dsc_t img_item_sm_stone;
extern const lv_image_dsc_t img_item_lg_mount_yuequan;
extern const lv_image_dsc_t img_item_sm_mount_yuequan;
extern const lv_image_dsc_t img_item_lg_mount_danque;
extern const lv_image_dsc_t img_item_sm_mount_danque;
extern const lv_image_dsc_t img_item_lg_qi_stone_20;
extern const lv_image_dsc_t img_item_sm_qi_stone_20;
extern const lv_image_dsc_t img_item_lg_fan_kite;
extern const lv_image_dsc_t img_item_sm_fan_kite;
extern const lv_image_dsc_t img_item_lg_fan_qingrong;
extern const lv_image_dsc_t img_item_sm_fan_qingrong;
extern const lv_image_dsc_t img_item_lg_fan_luodai;
extern const lv_image_dsc_t img_item_sm_fan_luodai;
extern const lv_image_dsc_t img_item_lg_fan_zhaoye;
extern const lv_image_dsc_t img_item_sm_fan_zhaoye;
extern const lv_image_dsc_t img_item_lg_fan_yingyue;
extern const lv_image_dsc_t img_item_sm_fan_yingyue;
extern const lv_image_dsc_t img_item_lg_fan_yaoxing;
extern const lv_image_dsc_t img_item_sm_fan_yaoxing;
extern const lv_image_dsc_t img_item_lg_fan_guichu;
extern const lv_image_dsc_t img_item_sm_fan_guichu;
extern const lv_image_dsc_t img_item_lg_fan_qishang;
extern const lv_image_dsc_t img_item_sm_fan_qishang;
extern const lv_image_dsc_t img_card_bg_lg_juepin;
extern const lv_image_dsc_t img_card_bg_sm_juepin;
extern const lv_image_dsc_t img_card_bg_lg_xishi;
extern const lv_image_dsc_t img_card_bg_sm_xishi;
extern const lv_image_dsc_t img_card_bg_lg_liangpin;
extern const lv_image_dsc_t img_card_bg_sm_liangpin;
const lv_image_dsc_t *tianshang_get_item_img_lg(int item_id);
const lv_image_dsc_t *tianshang_get_item_img_sm(int item_id);
const lv_image_dsc_t *tianshang_get_card_bg_lg(int quality);
const lv_image_dsc_t *tianshang_get_card_bg_sm(int quality);

#endif // TIANSHANG_IMG_ASSETS_H
