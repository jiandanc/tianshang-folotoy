#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Verify 100% font glyph coverage for all text used in the Tianshang Gacha application.
"""
import re
import os
import sys

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
FONT_FILE = os.path.join(REPO_ROOT, "main", "font_tianshang_14.c")

# All strings that appear in the UI
REQUIRED_STRINGS = [
    "神石鉴华", "天赏抽奖", "初游江湖", "闲云自在", "放歌清宵",
    "凭栏听雨", "把酒临风", "卧看千山", "一醉轻王侯", "秋水不染尘",
    "乘风登玉京", "缥缈云流鹤上仙", "捧日揽月摘星辰",
    "天赏石", "月泉衔碧", "丹阙霓天", "奇赏石*20", "风筝",
    "轻容染", "螺黛染", "照夜•花拥香雾", "映月•荧霓旌", "摇星•落花旋",
    "归初凝丹", "奇赏石",
    "天赏", "奇赏", "凡赏", "祥瑞",
    "累计抽数", "直出天赏", "直出祥瑞", "累计天赏", "首次天赏", "首次祥瑞",
    "当前称号", "保底", "单抽", "十连抽", "十连", "按键", "上键", "下键",
    "确认", "重置", "返回", "恭喜获得", "电量", "次", "抽", "颗", "份",
    "战报", "最佳保底", "消耗", "纹玉", "元", "清空重置", "绝品天赏鉴华",
    "开鉴次内必出天赏", "天赏预览",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
    "/", ":", "-", "+", "*", "•", "★", "▲", "▼", "●", "≈", "(", ")"
]

def extract_covered_glyphs(font_c_path):
    if not os.path.exists(font_c_path):
        print(f"Error: Font file not found: {font_c_path}")
        return set()
    with open(font_c_path, "r", encoding="utf-8") as f:
        content = f.read()

    # Find range_start and unicode_lists
    covered = set()
    # ASCII range
    covered.update(chr(c) for c in range(32, 127))

    # Parse unicode lists
    lists = re.findall(r'static const uint16_t unicode_list_\d+\[\] = \{([^}]+)\};', content)
    cmaps = re.findall(r'\.range_start = (\d+)[^}]+unicode_list = unicode_list_(\d+)', content)

    for range_start_str, list_idx_str in cmaps:
        range_start = int(range_start_str)
        list_idx = int(list_idx_str)
        if list_idx < len(lists):
            hex_vals = re.findall(r'0x([0-9A-Fa-f]+)', lists[list_idx])
            for h in hex_vals:
                code_point = range_start + int(h, 16)
                covered.add(chr(code_point))

    return covered

def main():
    covered = extract_covered_glyphs(FONT_FILE)
    if not covered:
        sys.exit("Failed to extract glyphs from font file.")

    missing = set()
    all_needed = set()
    for s in REQUIRED_STRINGS:
        for ch in s:
            all_needed.add(ch)
            if ch not in covered:
                missing.add(ch)

    print(f"Total distinct required characters: {len(all_needed)}")
    print(f"Total glyphs in font: {len(covered)}")

    if missing:
        print(f"FAIL: Missing glyphs: {missing}")
        sys.exit(1)

    print("PASS: 100% font coverage verified!")

if __name__ == "__main__":
    main()
