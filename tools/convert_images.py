#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Convert image assets from assets/img and nsh-price to C source files containing
LVGL 9 RGB565 image descriptors for Flash-resident (.rodata) rendering.
"""

import os
import sys
from PIL import Image

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
IMG_DIR = os.path.join(REPO_ROOT, "assets", "img")
NSH_GACHA_DIR = os.path.abspath("D:/Workspace/nsh-price/miniprogram/images/gacha")
OUT_C = os.path.join(REPO_ROOT, "main", "tianshang_img_assets.c")
OUT_H = os.path.join(REPO_ROOT, "main", "tianshang_img_assets.h")

def rgb_to_rgb565(r, g, b):
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)

def process_image_argb8888(src_path, target_w, target_h):
    im = Image.open(src_path).convert("RGBA")

    # Resize preserving aspect ratio
    im.thumbnail((target_w, target_h), Image.Resampling.LANCZOS)

    canvas = Image.new("RGBA", (target_w, target_h), (0, 0, 0, 0))
    offset = ((target_w - im.width) // 2, (target_h - im.height) // 2)
    canvas.paste(im, offset, im)

    data = bytearray()
    for y in range(target_h):
        for x in range(target_w):
            r, g, b, a = canvas.getpixel((x, y))
            # LVGL 9 LV_COLOR_FORMAT_ARGB8888 layout (lv_color32_t):
            # byte 0: Blue, byte 1: Green, byte 2: Red, byte 3: Alpha
            data.append(b)
            data.append(g)
            data.append(r)
            data.append(a)
    return data

def process_card_bg(src_path, target_w, target_h, bg_color=(20, 21, 30)):
    im = Image.open(src_path).convert("RGBA")
    im = im.resize((target_w, target_h), Image.Resampling.LANCZOS)
    canvas = Image.new("RGBA", (target_w, target_h), (*bg_color, 255))
    canvas.paste(im, (0, 0), im)
    rgb_im = canvas.convert("RGB")
    data = bytearray()
    for y in range(target_h):
        for x in range(target_w):
            r, g, b = rgb_im.getpixel((x, y))
            val565 = rgb_to_rgb565(r, g, b)
            data.append(val565 & 0xFF)
            data.append((val565 >> 8) & 0xFF)
    return data

def process_fullscreen_bg(src_path, target_w=240, target_h=320):
    im = Image.open(src_path).convert("RGB")
    w, h = im.size
    target_aspect = target_w / target_h
    current_aspect = w / h

    if current_aspect > target_aspect:
        new_w = int(h * target_aspect)
        left = (w - new_w) // 2
        im = im.crop((left, 0, left + new_w, h))
    else:
        new_h = int(w / target_aspect)
        top = (h - new_h) // 2
        im = im.crop((0, top, w, top + new_h))

    im = im.resize((target_w, target_h), Image.Resampling.LANCZOS)

    darkened = Image.new("RGB", (target_w, target_h))
    for y in range(target_h):
        for x in range(target_w):
            r, g, b = im.getpixel((x, y))
            r = int(r * 0.70)
            g = int(g * 0.70)
            b = int(b * 0.75)
            darkened.putpixel((x, y), (r, g, b))

    data = bytearray()
    for y in range(target_h):
        for x in range(target_w):
            r, g, b = darkened.getpixel((x, y))
            val565 = rgb_to_rgb565(r, g, b)
            data.append(val565 & 0xFF)
            data.append((val565 >> 8) & 0xFF)
    return data

def write_c_image(f_c, f_h, name, w, h, data, cf="LV_COLOR_FORMAT_RGB565"):
    stride = w * 2 if cf == "LV_COLOR_FORMAT_RGB565" else w * 4
    f_h.write(f"extern const lv_image_dsc_t {name};\n")

    f_c.write(f"/* {name}: {w}x{h} {cf} ({len(data)} bytes) */\n")
    f_c.write(f"static const uint8_t {name}_map[] = {{\n")
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        f_c.write("    " + ", ".join(f"0x{b:02X}" for b in chunk) + ",\n")
    f_c.write("};\n\n")
    f_c.write(f"const lv_image_dsc_t {name} = {{\n")
    f_c.write("    .header = {\n")
    f_c.write("        .magic = LV_IMAGE_HEADER_MAGIC,\n")
    f_c.write(f"        .cf = {cf},\n")
    f_c.write("        .flags = 0,\n")
    f_c.write(f"        .w = {w},\n")
    f_c.write(f"        .h = {h},\n")
    f_c.write(f"        .stride = {stride},\n")
    f_c.write("    },\n")
    f_c.write(f"    .data_size = {len(data)},\n")
    f_c.write(f"    .data = {name}_map,\n")
    f_c.write("};\n\n")

def main():
    with open(OUT_H, "w", encoding="utf-8") as f_h, open(OUT_C, "w", encoding="utf-8") as f_c:
        f_h.write("#ifndef TIANSHANG_IMG_ASSETS_H\n#define TIANSHANG_IMG_ASSETS_H\n\n#include \"lvgl.h\"\n\n")
        f_c.write('#include "tianshang_img_assets.h"\n\n')

        # 1. Main background (240x320)
        bg_path = os.path.join(IMG_DIR, "tianshang.webp")
        if os.path.exists(bg_path):
            print("Converting main background...")
            bg_data = process_fullscreen_bg(bg_path, 240, 320)
            write_c_image(f_c, f_h, "img_tianshang_bg", 240, 320, bg_data, cf="LV_COLOR_FORMAT_RGB565")

        # 2. Currency / Token icons (daibi and wenyu)
        daibi_path = os.path.join(IMG_DIR, "tianshang-daibi.webp")
        if os.path.exists(daibi_path):
            print("Converting daibi icon (ARGB8888)...")
            d_data = process_image_argb8888(daibi_path, 20, 20)
            write_c_image(f_c, f_h, "img_icon_daibi", 20, 20, d_data, cf="LV_COLOR_FORMAT_ARGB8888")

        wenyu_candidates = [
            os.path.join(IMG_DIR, "wenyu.png"),
            "D:/Workspace/nsh-price/miniprogram/images/wenyu.png",
            "/mnt/d/Workspace/nsh-price/miniprogram/images/wenyu.png",
        ]
        wenyu_path = next((p for p in wenyu_candidates if os.path.exists(p)), None)
        if wenyu_path:
            print("Converting wenyu icon (ARGB8888)...")
            w_data = process_image_argb8888(wenyu_path, 20, 20)
            write_c_image(f_c, f_h, "img_icon_wenyu", 20, 20, w_data, cf="LV_COLOR_FORMAT_ARGB8888")

        # 4. Item Icons for all 12 items:
        # Large (48x48) for single-pull result card, Compact (24x24) for ten-pull grid
        item_files = {
            "stone": "tianshang-token.webp",
            "mount_yuequan": "tianshang-yuequan.webp",
            "mount_danque": "tianshang-danni.webp",
            "qi_stone_20": "tianshang-qishang20.webp",
            "fan_kite": "tianshang-fengzheng.webp",
            "fan_qingrong": "tianshang-qingrong.webp",
            "fan_luodai": "tianshang-luodai.webp",
            "fan_zhaoye": "tianshang-zhaoye.webp",
            "fan_yingyue": "tianshang-yingyue.webp",
            "fan_yaoxing": "tianshang-yaoxing.webp",
            "fan_guichu": "tianshang-guichu.webp",
            "fan_qishang": "tianshang-qishang.webp"
        }

        for key, filename in item_files.items():
            p = os.path.join(IMG_DIR, filename)
            if os.path.exists(p):
                print(f"Converting item {key} (ARGB8888)...")
                d_large = process_image_argb8888(p, 48, 48)
                write_c_image(f_c, f_h, f"img_item_lg_{key}", 48, 48, d_large, cf="LV_COLOR_FORMAT_ARGB8888")
                d_compact = process_image_argb8888(p, 24, 24)
                write_c_image(f_c, f_h, f"img_item_sm_{key}", 24, 24, d_compact, cf="LV_COLOR_FORMAT_ARGB8888")

        # 5. Quality card backgrounds from miniprogram
        # Large (80x80) for single draw card
        # Small (38x38) for ten draw grid cards
        qual_files = {
            "juepin": "bg-juepin.png",
            "xishi": "bg-xishi.png",
            "liangpin": "bg-liangpin.png"
        }

        for qkey, qfile in qual_files.items():
            qp_candidates = [
                os.path.join(IMG_DIR, qfile),
                os.path.join(NSH_GACHA_DIR, qfile),
                os.path.join("/mnt/d/Workspace/nsh-price/miniprogram/images/gacha", qfile),
            ]
            qp = next((p for p in qp_candidates if os.path.exists(p)), None)
            if qp:
                print(f"Converting card bg {qkey} (RGB565)...")
                bg_lg = process_card_bg(qp, 80, 80)
                write_c_image(f_c, f_h, f"img_card_bg_lg_{qkey}", 80, 80, bg_lg, cf="LV_COLOR_FORMAT_RGB565")
                bg_sm = process_card_bg(qp, 38, 38)
                write_c_image(f_c, f_h, f"img_card_bg_sm_{qkey}", 38, 38, bg_sm, cf="LV_COLOR_FORMAT_RGB565")

        # Lookup helpers
        f_h.write("const lv_image_dsc_t *tianshang_get_item_img_lg(int item_id);\n")
        f_h.write("const lv_image_dsc_t *tianshang_get_item_img_sm(int item_id);\n")
        f_h.write("const lv_image_dsc_t *tianshang_get_card_bg_lg(int quality);\n")
        f_h.write("const lv_image_dsc_t *tianshang_get_card_bg_sm(int quality);\n\n")

        f_c.write("const lv_image_dsc_t *tianshang_get_item_img_lg(int item_id) {\n")
        f_c.write("    switch(item_id) {\n")
        f_c.write("        case 0: return &img_item_lg_stone;\n")
        f_c.write("        case 1: return &img_item_lg_mount_yuequan;\n")
        f_c.write("        case 2: return &img_item_lg_mount_danque;\n")
        f_c.write("        case 3: return &img_item_lg_qi_stone_20;\n")
        f_c.write("        case 4: return &img_item_lg_fan_kite;\n")
        f_c.write("        case 5: return &img_item_lg_fan_qingrong;\n")
        f_c.write("        case 6: return &img_item_lg_fan_luodai;\n")
        f_c.write("        case 7: return &img_item_lg_fan_zhaoye;\n")
        f_c.write("        case 8: return &img_item_lg_fan_yingyue;\n")
        f_c.write("        case 9: return &img_item_lg_fan_yaoxing;\n")
        f_c.write("        case 10: return &img_item_lg_fan_guichu;\n")
        f_c.write("        case 11: return &img_item_lg_fan_qishang;\n")
        f_c.write("        default: return &img_item_lg_stone;\n")
        f_c.write("    }\n")
        f_c.write("}\n\n")

        f_c.write("const lv_image_dsc_t *tianshang_get_item_img_sm(int item_id) {\n")
        f_c.write("    switch(item_id) {\n")
        f_c.write("        case 0: return &img_item_sm_stone;\n")
        f_c.write("        case 1: return &img_item_sm_mount_yuequan;\n")
        f_c.write("        case 2: return &img_item_sm_mount_danque;\n")
        f_c.write("        case 3: return &img_item_sm_qi_stone_20;\n")
        f_c.write("        case 4: return &img_item_sm_fan_kite;\n")
        f_c.write("        case 5: return &img_item_sm_fan_qingrong;\n")
        f_c.write("        case 6: return &img_item_sm_fan_luodai;\n")
        f_c.write("        case 7: return &img_item_sm_fan_zhaoye;\n")
        f_c.write("        case 8: return &img_item_sm_fan_yingyue;\n")
        f_c.write("        case 9: return &img_item_sm_fan_yaoxing;\n")
        f_c.write("        case 10: return &img_item_sm_fan_guichu;\n")
        f_c.write("        case 11: return &img_item_sm_fan_qishang;\n")
        f_c.write("        default: return &img_item_sm_stone;\n")
        f_c.write("    }\n")
        f_c.write("}\n\n")

        f_c.write("const lv_image_dsc_t *tianshang_get_card_bg_lg(int quality) {\n")
        f_c.write("    if (quality == 2) return &img_card_bg_lg_juepin;\n")
        f_c.write("    if (quality == 1) return &img_card_bg_lg_xishi;\n")
        f_c.write("    return &img_card_bg_lg_liangpin;\n")
        f_c.write("}\n\n")

        f_c.write("const lv_image_dsc_t *tianshang_get_card_bg_sm(int quality) {\n")
        f_c.write("    if (quality == 2) return &img_card_bg_sm_juepin;\n")
        f_c.write("    if (quality == 1) return &img_card_bg_sm_xishi;\n")
        f_c.write("    return &img_card_bg_sm_liangpin;\n")
        f_c.write("}\n\n")

        f_h.write("#endif // TIANSHANG_IMG_ASSETS_H\n")
    print("Done! Generated tianshang_img_assets.h and tianshang_img_assets.c")

if __name__ == "__main__":
    main()
