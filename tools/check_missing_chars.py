import glob
import re

TEST_CHARS = (
    # 核心界面与抽卡名词
    "天赏奇赏凡赏祥瑞抽卡抽奖神石鉴华单抽十连五十连"
    "天赏石月泉衔碧丹阙霓天奇赏石风筝轻容染螺黛染照夜花拥香雾映月荧霓旌摇星落花旋归初凝丹"
    # 统计与面板标签
    "累计抽数当前保底保底次数直出祥瑞累计天赏首次天赏首次祥瑞当前称号最佳保底"
    "总计消耗按键确认返回重置恭喜获得已获得获得个颗份次抽纹玉代币元电量本期奖池"
    # 称号库 (11档称号完整字符)
    "初游江湖闲云自在放歌清宵凭栏听雨把酒临风卧看千山一醉轻王侯秋水不染尘乘风登玉京缥缈云流鹤上仙捧日揽月摘星辰"
    # 提示与状态
    "上键下键长按短按单次连抽历史统计详细统计背包规则说明暂无数据清空重置成功战报核心基础记录命中结算"
    # 扩展常用字
    "绝品硬必升约任停内存冲分创刷务动化区可启复外失始字屏建张待恢复意擦时未止法紧继续缓缩节设败超配除音等"
    # 标点符号与特殊几何图形
    "•·★※×：，。！？（）[]【】▲▼●≈"
)

strings = []
for p in glob.glob("main/*.[ch]"):
    with open(p, encoding="utf-8", errors="ignore") as f:
        content = f.read()
    for s in re.findall(r'"([^"]*)"', content):
        strings.append(s)

all_chars = set("".join(strings))
ascii_chars = set(chr(c) for c in range(0x20, 0x7F))
font_chars = set(TEST_CHARS) | ascii_chars

missing = set()
for ch in all_chars:
    if ch not in font_chars and ord(ch) >= 128:
        missing.add(ch)

print(f"Missing count: {len(missing)}")
if missing:
    for ch in sorted(list(missing)):
        print(f"  U+{ord(ch):04X} ({ch})")
else:
    print("ALL non-ASCII characters in main/*.[ch] are 100% COVERED!")
