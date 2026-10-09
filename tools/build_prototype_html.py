import json
import os
import sys

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
IMAGES_JSON = os.path.join(REPO_ROOT, "assets", "images_b64.json")
OUTPUT_HTML = os.path.join(REPO_ROOT, "build", "tianshang_prototype.html")

with open(IMAGES_JSON, "r", encoding="utf-8") as f:
    images = json.load(f)

print(f"Loaded {len(images)} base64 images.")

html_template = """<title>神石鉴华掌机</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Cinzel:wght@600;700&family=Noto+Serif+SC:wght@500;700;900&family=Noto+Sans+SC:wght@400;500;700&family=JetBrains+Mono:wght@400;600&display=swap" rel="stylesheet">

<style>
:root {
  --bg-app: #0A0B10;
  --bg-panel: #131520;
  --bg-card: #1B1D2C;
  --bg-card-hover: #24273A;
  --border-subtle: #272A3C;
  --border-gold: #C59B4B;

  --gold-primary: #FFE6B0;
  --gold-vivid: #FFC53D;
  --gold-deep: #D48806;
  --gold-dark: #8C5A03;
  --purple-rare: #B37FEB;
  --blue-normal: #69C0FF;

  --text-main: #F0F2F5;
  --text-muted: #8C92A4;
  --text-dim: #5A5F73;
  --text-gold: #FFE6B0;
}

* {
  box-sizing: border-box;
  margin: 0;
  padding: 0;
}

body {
  background: var(--bg-app);
  color: var(--text-main);
  font-family: 'Noto Sans SC', system-ui, -apple-system, sans-serif;
  min-height: 100vh;
  display: flex;
  flex-direction: column;
  padding-inline: 24px;
  padding-block: 24px;
  overflow-x: hidden;
}

/* Header bar */
.top-nav {
  display: flex;
  justify-content: space-between;
  align-items: center;
  flex-wrap: wrap;
  gap: 16px;
  padding-bottom: 20px;
  border-bottom: 1px solid var(--border-subtle);
  margin-bottom: 24px;
}

.brand-section {
  display: flex;
  align-items: center;
  gap: 14px;
}

.brand-badge {
  background: linear-gradient(135deg, var(--gold-deep), var(--gold-vivid));
  color: #12100A;
  font-weight: 900;
  font-size: 13px;
  padding: 4px 10px;
  border-radius: 6px;
  letter-spacing: 1px;
}

.brand-title {
  font-family: 'Noto Serif SC', serif;
  font-size: 22px;
  font-weight: 900;
  color: var(--gold-primary);
  letter-spacing: 1.5px;
}

.brand-subtitle {
  font-size: 13px;
  color: var(--text-muted);
  margin-top: 2px;
}

/* Prototype Version Switcher Pills */
.version-tabs {
  display: flex;
  background: var(--bg-card);
  padding: 4px;
  border-radius: 30px;
  border: 1px solid var(--border-subtle);
  gap: 4px;
}

.tab-btn {
  background: transparent;
  border: none;
  color: var(--text-muted);
  font-size: 13px;
  font-weight: 500;
  padding: 8px 18px;
  border-radius: 20px;
  cursor: pointer;
  transition: all 0.2s ease;
  display: flex;
  align-items: center;
  gap: 6px;
}

.tab-btn:hover {
  color: var(--text-main);
}

.tab-btn.active {
  background: linear-gradient(135deg, #2A2F45, #363B54);
  color: var(--gold-primary);
  font-weight: 700;
  box-shadow: 0 2px 10px rgba(0, 0, 0, 0.4);
}

/* Layout container */
.main-stage {
  display: grid;
  grid-template-columns: 340px 1fr;
  gap: 36px;
  align-items: start;
}

@media (max-width: 1080px) {
  .main-stage {
    grid-template-columns: 1fr;
  }
}

/* Device Simulator Shell */
.device-column {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 16px;
  user-select: none;
  position: sticky;
  top: 20px;
}

.device-chassis {
  width: 320px;
  background: linear-gradient(175deg, #242738 0%, #151722 45%, #0F1018 100%);
  border-radius: 32px;
  padding: 24px 20px 28px;
  box-shadow: 0 24px 60px rgba(0, 0, 0, 0.75),
              0 0 0 1px rgba(255, 255, 255, 0.08),
              inset 0 1px 1px rgba(255, 255, 255, 0.2);
  display: flex;
  flex-direction: column;
  align-items: center;
  position: relative;
}

.chassis-header {
  width: 100%;
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 0 8px 12px;
  font-size: 11px;
  font-weight: 600;
  letter-spacing: 1px;
  color: #555B73;
}

.chassis-led {
  width: 6px;
  height: 6px;
  background: #52C41A;
  border-radius: 50%;
  box-shadow: 0 0 8px #52C41A;
}

.screen-bezel {
  width: 256px;
  height: 336px;
  background: #08090D;
  border-radius: 12px;
  padding: 8px;
  box-shadow: inset 0 2px 8px rgba(0, 0, 0, 0.9), 0 0 0 1px rgba(255, 255, 255, 0.05);
  position: relative;
  overflow: hidden;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: all 0.2s ease;
}

.screen-bezel::before {
  content: '';
  position: absolute;
  top: 0;
  left: -50%;
  width: 200%;
  height: 60%;
  background: linear-gradient(135deg, rgba(255, 255, 255, 0.06) 0%, rgba(255, 255, 255, 0) 60%);
  pointer-events: none;
  z-index: 100;
  transform: rotate(-15deg);
}

.lcd-screen {
  width: 240px;
  height: 320px;
  background: #000;
  position: relative;
  overflow: hidden;
  font-size: 14px;
  color: #FFF;
  image-rendering: pixelated;
  transform-origin: center center;
  transition: transform 0.2s ease;
}

.lcd-bg {
  position: absolute;
  inset: 0;
  width: 240px;
  height: 320px;
  object-fit: cover;
  z-index: 0;
}

.chassis-buttons {
  margin-top: 22px;
  width: 100%;
  display: flex;
  justify-content: space-around;
  align-items: center;
  padding: 0 10px;
}

.hw-btn {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 6px;
  cursor: pointer;
}

.hw-btn-cap {
  width: 48px;
  height: 48px;
  background: linear-gradient(145deg, #32364B, #1E202E);
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  color: var(--gold-primary);
  font-size: 16px;
  box-shadow: 0 6px 14px rgba(0, 0, 0, 0.6), inset 0 1px 1px rgba(255, 255, 255, 0.2);
  border: 1px solid rgba(255, 255, 255, 0.08);
  transition: all 0.08s ease;
}

.hw-btn-cap:active, .hw-btn-cap.pressed {
  transform: translateY(3px);
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.8), inset 0 2px 4px rgba(0, 0, 0, 0.6);
  background: linear-gradient(145deg, #1E202E, #141620);
}

.hw-btn.center-btn .hw-btn-cap {
  width: 54px;
  height: 54px;
  background: linear-gradient(145deg, #414660, #26293A);
  color: #FFF;
}

.hw-btn-label {
  font-size: 11px;
  font-weight: 600;
  color: var(--text-muted);
}

.zoom-controls {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 12px;
  color: var(--text-muted);
}

.zoom-btn {
  background: var(--bg-card);
  border: 1px solid var(--border-subtle);
  color: var(--text-main);
  padding: 5px 12px;
  border-radius: 6px;
  font-size: 12px;
  cursor: pointer;
  transition: all 0.15s ease;
}

.zoom-btn:hover {
  background: var(--bg-card-hover);
}

.zoom-btn.active {
  border-color: var(--gold-deep);
  color: var(--gold-vivid);
  background: #25283C;
}

/* ==========================================================
   VIEW 1: REFINED MAIN SCREEN (根据用户最新反馈精简后的高品质界面)
   ========================================================== */
.view-main {
  position: absolute;
  inset: 0;
  z-index: 10;
  width: 240px;
  height: 320px;
  display: flex;
  flex-direction: column;
  justify-content: space-between;
  padding: 6px 10px 8px;
  background: radial-gradient(circle at 50% 30%, rgba(10, 11, 18, 0.1) 0%, rgba(5, 6, 12, 0.80) 100%);
}

/* Header */
.main-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.main-title-wrap {
  display: flex;
  align-items: baseline;
  gap: 6px;
}

.main-title {
  font-family: 'Noto Serif SC', serif;
  font-size: 18px;
  font-weight: 900;
  color: #FFC53D;
  text-shadow: 0 0 10px rgba(255, 197, 61, 0.6), 0 2px 4px #000;
  letter-spacing: 1.5px;
}

.main-badge {
  background: rgba(212, 136, 6, 0.25);
  border: 1px solid rgba(255, 197, 61, 0.5);
  border-radius: 10px;
  padding: 1px 7px;
  font-size: 11px;
  color: #FFE6B0;
}

.main-battery {
  display: flex;
  align-items: center;
  gap: 3px;
  font-size: 12px;
  color: #87D068;
  font-weight: 600;
}

/* Status Strip: 抽数0  祥瑞0  天赏石0 (黑色背景，无金边) */
.main-status-strip {
  background: rgba(12, 13, 21, 0.85);
  border: none;
  border-radius: 6px;
  padding: 5px 8px;
  margin-top: 2px;
}

.main-counts-row {
  display: flex;
  justify-content: space-around;
  font-size: 13px;
  color: #D9D9D9;
}

.main-counts-row b {
  color: #FFC53D;
  font-weight: 700;
}

/* Center Showcase Area: 整体向下移动，移动到底部按钮上方 */
.main-showcase-area {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: flex-end;
  flex: 1;
  position: relative;
  padding: 4px 0 6px;
  margin-top: auto;
}

/* “绝品天赏鉴华”改成“天赏预览”，去掉左右图标 */
.main-preview-title {
  font-size: 13px;
  font-weight: 700;
  color: #FFE6B0;
  letter-spacing: 1.5px;
  margin-bottom: 8px;
  text-align: center;
  text-shadow: 0 1px 4px rgba(0, 0, 0, 0.9);
}

/* 物品图标减小30%并向下移动点 */
.main-items-row {
  display: flex;
  justify-content: center;
  align-items: center;
  gap: 16px;
  width: 100%;
  margin-top: 2px;
  margin-bottom: 8px;
}

.main-item-card {
  display: flex;
  flex-direction: column;
  align-items: center;
}

/* 48px * 0.7 = 34px (减小30%) */
.main-avatar-frame {
  width: 36px;
  height: 36px;
  position: relative;
  display: flex;
  align-items: center;
  justify-content: center;
}

.main-frame-bg {
  position: absolute;
  inset: 0;
  width: 100%;
  height: 100%;
  border-radius: 6px;
  border: 1px solid rgba(255, 215, 120, 0.4);
}

.main-item-img {
  position: relative;
  width: 28px;
  height: 28px;
  object-fit: contain;
}

.main-item-lbl {
  font-size: 11px;
  color: #D9D9D9;
  margin-top: 5px;
  font-weight: 500;
}

/* “开鉴x次内必出天赏”，x为金色，其余为白色 */
.pity-countdown-line {
  font-size: 13px;
  color: #FFFFFF;
  text-align: center;
  margin-top: 4px;
  margin-bottom: 8px;
  letter-spacing: 0.5px;
}

.gold-pity-x {
  color: #FFC53D;
  font-weight: 700;
  margin: 0 2px;
}

/* 现有对的按键样式 */
.main-action-bar {
  width: 100%;
  height: 32px;
  background: rgba(16, 17, 27, 0.85);
  border: 1px solid #D48806;
  border-radius: 6px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 8px;
}

.action-btn-item {
  display: flex;
  align-items: center;
  gap: 4px;
  font-size: 13px;
  color: #FFC53D;
  cursor: pointer;
}

.action-btn-item img {
  width: 16px;
  height: 16px;
}

.action-btn-item.stats-center-btn {
  color: #D9D9D9;
}

/* ==========================================================
   RESULT MODAL OVERLAY (抽奖结果界面)
   ========================================================== */
.result-overlay {
  position: absolute;
  inset: 0;
  background: rgba(5, 6, 10, 0.94);
  z-index: 50;
  display: flex;
  flex-direction: column;
  justify-content: space-between;
  align-items: center;
  padding: 16px 10px 8px;
}

.res-header-title {
  font-family: 'Noto Serif SC', serif;
  font-size: 20px;
  font-weight: 900;
  color: #FFE6B0;
  letter-spacing: 3px;
  text-shadow: 0 0 16px rgba(255, 215, 120, 0.7);
}

/* Single Pull Card (80x80) */
.res-single-area {
  display: flex;
  align-items: center;
  justify-content: center;
  flex: 1;
  position: relative;
}

.res-halo-pulse {
  position: absolute;
  width: 120px;
  height: 120px;
  border-radius: 50%;
  background: radial-gradient(circle, rgba(255, 215, 100, 0.6) 0%, rgba(212, 136, 6, 0.2) 50%, transparent 70%);
  animation: haloBreathe 2s ease-in-out infinite;
}

@keyframes haloBreathe {
  0%, 100% { transform: scale(0.9); opacity: 0.6; }
  50% { transform: scale(1.15); opacity: 0.9; }
}

.res-single-card {
  width: 80px;
  height: 80px;
  border-radius: 10px;
  position: relative;
  overflow: hidden;
  box-shadow: 0 10px 30px rgba(0, 0, 0, 0.8);
  border: 1px solid rgba(255, 255, 255, 0.15);
}

.res-single-card .card-bg-img {
  position: absolute;
  inset: 0;
  width: 100%;
  height: 100%;
}

.res-single-card .card-item-img {
  position: absolute;
  inset: 16px;
  width: 48px;
  height: 48px;
  object-fit: contain;
}

.res-card-count {
  position: absolute;
  bottom: 2px;
  right: 5px;
  font-size: 13px;
  font-weight: 700;
  color: #FFF;
}

/* Ten Pull Grid (2x5 = 10 cards, 38x38 each) */
.res-ten-grid {
  display: grid;
  grid-template-columns: repeat(5, 38px);
  grid-template-rows: repeat(2, 38px);
  gap: 8px 6px;
  justify-content: center;
  align-content: center;
  flex: 1;
}

.res-ten-card {
  width: 38px;
  height: 38px;
  border-radius: 6px;
  position: relative;
  overflow: hidden;
  box-shadow: 0 4px 10px rgba(0, 0, 0, 0.6);
  border: 1px solid rgba(255, 255, 255, 0.1);
}

.res-ten-card .card-bg-img {
  position: absolute;
  inset: 0;
  width: 100%;
  height: 100%;
}

.res-ten-card .card-item-img {
  position: absolute;
  inset: 7px;
  width: 24px;
  height: 24px;
  object-fit: contain;
}

.res-ten-card.is-tian {
  border: 1px solid #FFC53D;
  box-shadow: 0 0 12px rgba(255, 197, 61, 0.8);
}

/* Result action bar: 样式图标按钮文案不变，中间变为“返回” */
.res-action-bar {
  width: 100%;
  height: 32px;
  background: rgba(16, 17, 27, 0.85);
  border: 1px solid #D48806;
  border-radius: 6px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 8px;
}

/* ==========================================================
   STATS VIEW (战报统计面板)
   ========================================================== */
.stats-overlay {
  position: absolute;
  inset: 10px;
  top: 24px;
  bottom: 24px;
  background: rgba(12, 13, 21, 0.95);
  border: 1px solid #D48806;
  border-radius: 10px;
  z-index: 40;
  padding: 12px 10px;
  display: flex;
  flex-direction: column;
  justify-content: space-between;
  box-shadow: 0 10px 40px rgba(0, 0, 0, 0.8);
}

.stat-head {
  text-align: center;
  font-size: 14px;
  font-weight: 700;
  color: #FFC53D;
  border-bottom: 1px solid rgba(212, 136, 6, 0.3);
  padding-bottom: 6px;
}

.stat-list-refined {
  display: flex;
  flex-direction: column;
  gap: 5px;
  width: 100%;
}

.stat-item-box {
  display: flex;
  justify-content: space-between;
  align-items: center;
  border-bottom: 1px solid rgba(255, 255, 255, 0.06);
  padding-bottom: 3px;
  font-size: 12px;
  color: #D9D9D9;
}

.stat-val-gold {
  color: #FFE6B0;
  font-weight: 600;
}

.stat-wenyu-row {
  display: flex;
  align-items: center;
  gap: 6px;
  background: rgba(255, 197, 61, 0.1);
  border: 1px solid rgba(255, 197, 61, 0.25);
  padding: 6px 8px;
  border-radius: 6px;
  font-size: 12px;
  color: #FFE6B0;
}

.stat-wenyu-row img {
  width: 18px;
  height: 18px;
}

.stat-foot-hint {
  text-align: center;
  font-size: 11px;
  color: #8C8C8C;
}

/* Right Detail Column */
.details-column {
  display: flex;
  flex-direction: column;
  gap: 20px;
}

.card-panel {
  background: var(--bg-panel);
  border: 1px solid var(--border-subtle);
  border-radius: 16px;
  padding: 22px;
}

.panel-title {
  font-size: 16px;
  font-weight: 700;
  color: var(--gold-primary);
  margin-bottom: 16px;
  display: flex;
  align-items: center;
  gap: 8px;
}

.diagnose-list {
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.diagnose-item {
  display: flex;
  gap: 14px;
  background: var(--bg-card);
  padding: 14px 16px;
  border-radius: 10px;
  border-left: 3px solid #52C41A;
  font-size: 13px;
  line-height: 1.55;
}

.diag-title {
  font-weight: 700;
  color: var(--text-main);
  margin-bottom: 3px;
}

.diag-desc {
  color: var(--text-muted);
}

.tag-good {
  background: rgba(82, 196, 26, 0.15);
  color: #52C41A;
  font-size: 11px;
  font-weight: 700;
  padding: 2px 8px;
  border-radius: 4px;
  height: fit-content;
  white-space: nowrap;
}

.test-bar {
  display: flex;
  flex-wrap: wrap;
  gap: 10px;
  margin-top: 14px;
}

.sim-btn {
  background: var(--bg-card);
  border: 1px solid var(--border-subtle);
  color: var(--text-main);
  padding: 8px 16px;
  border-radius: 8px;
  font-size: 13px;
  font-weight: 500;
  cursor: pointer;
  display: flex;
  align-items: center;
  gap: 6px;
  transition: all 0.15s ease;
}

.sim-btn:hover {
  background: var(--bg-card-hover);
  border-color: var(--border-gold);
}

.sim-btn.primary {
  background: linear-gradient(135deg, var(--gold-deep), var(--gold-vivid));
  color: #12100A;
  font-weight: 700;
  border: none;
}

/* Hidden utility */
[hidden] {
  display: none !important;
}
</style>

<div class="top-nav">
  <div class="brand-section">
    <div class="brand-badge">240×320 LCD</div>
    <div>
      <h1 class="brand-title">神石鉴华 · 最新原型设计与确认</h1>
      <p class="brand-subtitle">已按您的最新反馈精准修正所有文案、按键样式与道具布局</p>
    </div>
  </div>

  <div class="version-tabs">
    <button class="tab-btn active">
      <span>✨</span> 最新确认版原型
    </button>
  </div>
</div>

<div class="main-stage">
  <!-- Left Column: Simulated Device Chassis -->
  <div class="device-column">
    <div class="device-chassis">
      <div class="chassis-header">
        <span>FOLOTOY AI PASSPORT</span>
        <div class="chassis-led"></div>
      </div>

      <div class="screen-bezel">
        <div class="lcd-screen" id="lcdScreen">
          <!-- Fullscreen authentic background image -->
          <img class="lcd-bg" id="lcdBg" alt="bg">

          <!-- ==============================================
               1. REFINED MAIN VIEW
               ============================================== -->
          <div class="view-main" id="viewMain">
            <!-- Header -->
            <div class="main-header">
              <div class="main-title-wrap">
                <span class="main-title">神石鉴华</span>
                <span class="main-badge" id="mainBadgeTitle">初游江湖</span>
              </div>
              <div class="main-battery">
                <span>⚡</span>
                <span id="mainBattery">85%</span>
              </div>
            </div>

            <!-- Top Counts: 抽数 0  祥瑞 0  天赏石 0 -->
            <div class="main-status-strip">
              <div class="main-counts-row">
                <span>抽数 <b id="mainDraws">0</b></span>
                <span>祥瑞 <b id="mainMounts">0</b></span>
                <span>天赏石 <b id="mainStones">0</b></span>
              </div>
            </div>

            <!-- Center Showcase Area -->
            <div class="main-showcase-area">
              <!-- “绝品天赏鉴华”改成“天赏预览”，去掉左右图标 -->
              <div class="main-preview-title">天赏预览</div>

              <!-- 物品图标减小30%并向下移动点，去掉下面的物品名称 -->
              <div class="main-items-row">
                <!-- Yuequan -->
                <div class="main-item-card">
                  <div class="main-avatar-frame">
                    <img class="main-frame-bg" id="frame0" alt="card">
                    <img class="main-item-img" id="item0" alt="yuequan">
                  </div>
                </div>

                <!-- Stone -->
                <div class="main-item-card">
                  <div class="main-avatar-frame">
                    <img class="main-frame-bg" id="frame1" alt="card">
                    <img class="main-item-img" id="item1" alt="stone">
                  </div>
                </div>

                <!-- Danque -->
                <div class="main-item-card">
                  <div class="main-avatar-frame">
                    <img class="main-frame-bg" id="frame2" alt="card">
                    <img class="main-item-img" id="item2" alt="danque">
                  </div>
                </div>
              </div>

              <!-- 开鉴x次内必出天赏，x为金色，其他为白色 -->
              <div class="pity-countdown-line">
                开鉴<span class="gold-pity-x" id="pityCountX">180</span>次内必出天赏
              </div>
            </div>

            <!-- Bottom Actions: 现有对的样式，无多余xxxx纹玉 -->
            <div class="main-action-bar">
              <div class="action-btn-item" onclick="triggerDraw(1)">
                <img id="mainDaibi1" alt="coin">
                <span>▲单抽</span>
              </div>
              <div class="action-btn-item stats-center-btn" onclick="toggleStatsModal()">
                <span>●战报</span>
              </div>
              <div class="action-btn-item" onclick="triggerDraw(10)">
                <img id="mainDaibi2" alt="coin">
                <span>▼十连</span>
              </div>
            </div>
          </div>

          <!-- ==============================================
               2. RESULT OVERLAY (抽奖结果界面)
               ============================================== -->
          <div class="result-overlay" id="resultOverlay" hidden>
            <div class="res-header-title">恭喜获得</div>

            <!-- Single Result Card -->
            <div class="res-single-area" id="resSingleBox">
              <div class="res-single-card">
                <img class="card-bg-img" id="resSingleBg" alt="card-bg">
                <img class="card-item-img" id="resSingleIcon" alt="item-icon">
                <span class="res-card-count" id="resSingleCount" hidden>20</span>
              </div>
            </div>

            <!-- Ten Result Grid -->
            <div class="res-ten-grid" id="resTenGrid" hidden>
              <!-- 10 card slots -->
            </div>

            <!-- “开鉴x次内必出天赏”在抽奖结果界面也展示，位置同主界面 -->
            <div class="pity-countdown-line" style="margin-bottom: 6px;">
              开鉴<span class="gold-pity-x" id="resPityCountX">180</span>次内必出天赏
            </div>

            <!-- 抽奖结果界面：单抽/十连样式图标文案不变，仅中间变“●返回” -->
            <div class="res-action-bar">
              <div class="action-btn-item" onclick="triggerDraw(1)">
                <img id="resDaibi1" alt="coin">
                <span>▲单抽</span>
              </div>
              <div class="action-btn-item stats-center-btn" onclick="closeResultModal()">
                <span>●返回</span>
              </div>
              <div class="action-btn-item" onclick="triggerDraw(10)">
                <img id="resDaibi2" alt="coin">
                <span>▼十连</span>
              </div>
            </div>
          </div>

          <!-- ==============================================
               3. STATS VIEW (战报统计面板)
               ============================================== -->
          <div class="stats-overlay" id="statsOverlay" hidden>
            <div class="stat-head">【神石鉴华 · 战报统计】</div>

            <div class="stat-list-refined">
              <div class="stat-item-box">
                <span>当前称号:</span>
                <b class="stat-val-gold" id="statTitle">初游江湖</b>
              </div>
              <div class="stat-item-box">
                <span>累计抽数:</span>
                <span class="stat-val-gold" id="statTotalDraws">0次</span>
              </div>
              <div class="stat-item-box">
                <span>累计天赏:</span>
                <span class="stat-val-gold" id="statTotalTian">0个</span>
              </div>
              <div class="stat-item-box">
                <span>首次祥瑞:</span>
                <span class="stat-val-gold" id="statFirstMount">暂无</span>
              </div>
              <div class="stat-item-box">
                <span>直出祥瑞:</span>
                <span class="stat-val-gold" id="statMountCount">0个</span>
              </div>
              <div class="stat-item-box">
                <span>直出天赏:</span>
                <span class="stat-val-gold" id="statDirectStone">0个</span>
              </div>
              <div class="stat-item-box">
                <span>最佳保底:</span>
                <span class="stat-val-gold" id="statMinPity">暂无</span>
              </div>
            </div>

            <div class="stat-wenyu-row">
              <img id="statWenyuIcon" alt="wenyu">
              <span id="statSpentWenyu">消耗: 0 纹玉</span>
            </div>

            <div class="stat-foot-hint">
              短按 OK 返回 | 长按 OK 清空重置
            </div>
          </div>

        </div>
      </div>

      <!-- Hardware Physical Buttons -->
      <div class="chassis-buttons">
        <div class="hw-btn" onclick="handleHwButton('UP')">
          <div class="hw-btn-cap" id="btnHwUp">▲</div>
          <span class="hw-btn-label">单抽 (UP)</span>
        </div>

        <div class="hw-btn center-btn" onclick="handleHwButton('OK')" onmousedown="startLongPress()" onmouseup="cancelLongPress()">
          <div class="hw-btn-cap" id="btnHwOk">●</div>
          <span class="hw-btn-label">战报/返回</span>
        </div>

        <div class="hw-btn" onclick="handleHwButton('DOWN')">
          <div class="hw-btn-cap" id="btnHwDown">▼</div>
          <span class="hw-btn-label">十连 (DOWN)</span>
        </div>
      </div>
    </div>

    <!-- Zoom Controls -->
    <div class="zoom-controls">
      <span>检视缩放:</span>
      <button class="zoom-btn active" onclick="setZoom(1.0, this)">1.0x (实机)</button>
      <button class="zoom-btn" onclick="setZoom(1.25, this)">1.25x</button>
      <button class="zoom-btn" onclick="setZoom(1.5, this)">1.5x (舒适)</button>
    </div>
  </div>

  <!-- Right Column: Detail & Testing -->
  <div class="details-column">
    <div class="card-panel">
      <div class="panel-title">
        <span>✅</span>
        <span>本次反馈要求完成清单</span>
      </div>

      <div class="diagnose-list">
        <div class="diagnose-item">
          <span class="tag-good">已完成</span>
          <div>
            <div class="diag-title">1. 战报统计：单列展示 & 字段精简</div>
            <div class="diag-desc">
              彻底改为一行一个数据，消除横向超出边界；按顺序自上而下保留：<b>当前称号、累计抽数、累计天赏、首次祥瑞、直出祥瑞、直出天赏、最佳保底</b>，底部展示 <b>消耗 xx 纹玉</b>（去除括号中元估算），已删除首次天赏与保底概率。累计天赏按天赏石折算（祥瑞 x3），直出天赏即主界面天赏石数量。
            </div>
          </div>
        </div>

        <div class="diagnose-item">
          <span class="tag-good">已完成</span>
          <div>
            <div class="diag-title">2. 称号展示与神石鉴华同排</div>
            <div class="diag-desc">
              顶部标题行并排展示“神石鉴华”与金色称号徽章（如“把酒临风”），去除了多余的“称号:”前缀。
            </div>
          </div>
        </div>

        <div class="diagnose-item">
          <span class="tag-good">已完成</span>
          <div>
            <div class="diag-title">3. 抽数行去金边 & 黑色背景</div>
            <div class="diag-desc">
              “抽数 0  祥瑞 0  天赏石 0”状态行采用纯黑圆角背景底框，去除了矩形金边。
            </div>
          </div>
        </div>

        <div class="diagnose-item">
          <span class="tag-good">已完成</span>
          <div>
            <div class="diag-title">4. 天赏预览去除半透明黑底</div>
            <div class="diag-desc">
              去除了天赏预览展台的半透明黑底矩形与外框，图标与文案纯净融入背景。
            </div>
          </div>
        </div>
      </div>
    </div>

    <!-- Interactive Testing -->
    <div class="card-panel">
      <div class="panel-title">
        <span>🎮</span>
        <span>交互测试与操作指引</span>
      </div>

      <p style="font-size: 13px; color: var(--text-muted); line-height: 1.6;">
        可在本页面直接测试真实抽奖！支持点击左侧实机按键或使用键盘方向键：<br>
        • <kbd style="background: #25283C; padding: 2px 6px; border-radius: 4px; color: #FFE6B0;">↑</kbd> 键盘上键：触发 <b>单抽</b><br>
        • <kbd style="background: #25283C; padding: 2px 6px; border-radius: 4px; color: #FFE6B0;">↓</kbd> 键盘下键：触发 <b>十连抽</b><br>
        • <kbd style="background: #25283C; padding: 2px 6px; border-radius: 4px; color: #FFE6B0;">Space</kbd> / <kbd style="background: #25283C; padding: 2px 6px; border-radius: 4px; color: #FFE6B0;">Enter</kbd>：切换战报 / 返回
      </p>

      <div class="test-bar">
        <button class="sim-btn primary" onclick="triggerDraw(1)">▲ 测试单抽</button>
        <button class="sim-btn primary" onclick="triggerDraw(10)">▼ 测试十连抽</button>
        <button class="sim-btn" onclick="toggleStatsModal()">● 打开战报统计</button>
        <button class="sim-btn" onclick="resetState()">↺ 重置数据</button>
        <button class="sim-btn" onclick="simulateLuckyTian()" style="border-color: #FFC53D; color: #FFC53D;">✨ 模拟欧皇出天赏</button>
      </div>
    </div>
  </div>
</div>

<script>
const IMAGES = ___IMAGES_PLACEHOLDER___;

const state = {
  totalDrawCount: 0,
  nonTianCount: 0,
  consecutiveFanCount: 0,
  totalTianCount: 0,
  totalMountCount: 0,
  totalStoneCount: 0,
  firstMountDraw: 0,
  firstTianDraw: 0,
  minTianPity: 0,
  battery: 85
};

// 战报"累计天赏"口径：按天赏石折算，一个祥瑞等价 3 个天赏石，
// 与小程序 rollTianshangDraw 的 newTianRewardGained 一致。
function getTotalTianStones() {
  return state.totalStoneCount + state.totalMountCount * 3;
}

const ITEMS = [
  { id: 0, name: "天赏石", quality: 2, iconKey: "item_stone" },  { id: 1, name: "月泉衔碧", quality: 2, iconKey: "item_yuequan" },
  { id: 2, name: "丹阙霓天", quality: 2, iconKey: "item_danque" },
  { id: 3, name: "奇赏石*20", quality: 1, iconKey: "item_qi_stone_20", count: 20 },
  { id: 4, name: "风筝", quality: 0, iconKey: "item_fan_kite" },
  { id: 5, name: "轻容染", quality: 0, iconKey: "item_fan_qingrong" },
  { id: 6, name: "螺黛染", quality: 0, iconKey: "item_fan_luodai" },
  { id: 7, name: "照夜•花拥香雾", quality: 0, iconKey: "item_fan_zhaoye" },
  { id: 8, name: "映月•荧霓旌", quality: 0, iconKey: "item_fan_yingyue" },
  { id: 9, name: "摇星•落花旋", quality: 0, iconKey: "item_fan_yaoxing" },
  { id: 10, name: "归初凝丹", quality: 0, iconKey: "item_fan_guichu" },
  { id: 11, name: "奇赏石", quality: 0, iconKey: "item_fan_qishang" }
];

// 称号阶梯：与小程序 constants.js TITLE_TIERS / 固件 s_title_tiers 对齐。
// 每抽 180 纹玉 = 18 元 RMB，按累计 RMB 匹配。
const TITLE_TIERS = [
  { minRmb: 0, name: "初游江湖" },
  { minRmb: 200, name: "闲云自在" },
  { minRmb: 1000, name: "放歌清宵" },
  { minRmb: 3000, name: "凭栏听雨" },
  { minRmb: 10000, name: "把酒临风" },
  { minRmb: 50000, name: "卧看千山" },
  { minRmb: 100000, name: "一醉轻王侯" },
  { minRmb: 200000, name: "秋水不染尘" },
  { minRmb: 500000, name: "乘风登玉京" },
  { minRmb: 1000000, name: "缥缈云流鹤上仙" },
  { minRmb: 1500000, name: "捧日揽月摘星辰" }
];

function getTitle(draws) {
  const rmb = draws * 18;
  let title = TITLE_TIERS[0].name;
  for (const t of TITLE_TIERS) {
    if (rmb >= t.minRmb) title = t.name;
    else break;
  }
  return title;
}

// 与小程序 tianshangSim.js / 固件 tianshang_sim.c 对齐：
// 161-179 抽软保底递增表、天赏内部分配 0.0036:0.0002:0.0002 (即 90%/5%/5%)
const TIAN_PROB_TABLE = [
  0.010, 0.020, 0.030, 0.040, 0.055,
  0.070, 0.085, 0.100, 0.120, 0.140,
  0.165, 0.190, 0.220, 0.250, 0.285,
  0.320, 0.360, 0.420, 0.500
];

function getTianProb(nonTian) {
  if (nonTian >= 179) return 1.000;
  if (nonTian < 160) return 0.004;
  return TIAN_PROB_TABLE[nonTian - 160] !== undefined ? TIAN_PROB_TABLE[nonTian - 160] : 0.500;
}

function rollOne() {
  state.totalDrawCount++;
  const nonTian = state.nonTianCount;

  // 1. 天赏判定：180 抽硬保底 + 161-179 抽软保底递增
  let resQuality = 0;  // 0=凡赏 1=奇赏 2=天赏
  if (nonTian >= 179) {
    resQuality = 2;
  } else {
    const pTian = getTianProb(nonTian);
    const r = Math.random();
    if (r < pTian) resQuality = 2;
    else if (r < pTian + 0.04) resQuality = 1;
  }

  // 2. 更新保底计数器（非凡赏即清零连续凡赏计数）
  if (resQuality === 2) {
    state.nonTianCount = 0;
    state.consecutiveFanCount = 0;
  } else if (resQuality === 1) {
    state.nonTianCount++;
    state.consecutiveFanCount = 0;
  } else {
    state.nonTianCount++;
    state.consecutiveFanCount++;
    // 20 抽连续凡赏保底升奇赏
    if (state.consecutiveFanCount >= 20) {
      resQuality = 1;
      state.consecutiveFanCount = 0;
    }
  }

  if (resQuality === 2) {
    state.totalTianCount++;
    if (state.firstTianDraw === 0) state.firstTianDraw = state.totalDrawCount;
    const pityAt = nonTian + 1;
    if (state.minTianPity === 0 || pityAt < state.minTianPity) state.minTianPity = pityAt;

    // 天赏内部分配：0.0036 : 0.0002 : 0.0002 归一化 = 天赏石 90%、月泉衔碧 5%、丹阙霓天 5%
    // 即播出天赏时，仅 1/10 为祥瑞（与小程序 pickWeighted 口径一致）
    const rItem = Math.random();
    if (rItem < 0.90) {
      state.totalStoneCount++;
      return ITEMS[0];
    }
    state.totalMountCount++;
    if (state.firstMountDraw === 0) state.firstMountDraw = state.totalDrawCount;
    return (rItem < 0.95) ? ITEMS[1] : ITEMS[2];
  }

  if (resQuality === 1) {
    return ITEMS[3];
  }

  const fanIdx = 4 + Math.floor(Math.random() * 8);
  return ITEMS[fanIdx];
}

let audioCtx = null;
function playTianshangChime() {
  try {
    if (!audioCtx) audioCtx = new (window.AudioContext || window.webkitAudioContext)();
    const now = audioCtx.currentTime;
    const osc = audioCtx.createOscillator();
    const gain = audioCtx.createGain();
    osc.type = 'triangle';
    osc.frequency.setValueAtTime(523.25, now);
    osc.frequency.exponentialRampToValueAtTime(1046.5, now + 0.3);
    gain.gain.setValueAtTime(0.3, now);
    gain.gain.exponentialRampToValueAtTime(0.01, now + 0.8);
    osc.connect(gain);
    gain.connect(audioCtx.destination);
    osc.start(now);
    osc.stop(now + 0.8);
  } catch (e) {}
}

function initImages() {
  document.getElementById('lcdBg').src = IMAGES.bg_main;

  document.getElementById('frame0').src = IMAGES.card_juepin;
  document.getElementById('item0').src = IMAGES.item_yuequan;
  document.getElementById('frame1').src = IMAGES.card_juepin;
  document.getElementById('item1').src = IMAGES.item_stone;
  document.getElementById('frame2').src = IMAGES.card_juepin;
  document.getElementById('item2').src = IMAGES.item_danque;

  document.getElementById('mainDaibi1').src = IMAGES.icon_daibi;
  document.getElementById('mainDaibi2').src = IMAGES.icon_daibi;
  document.getElementById('resDaibi1').src = IMAGES.icon_daibi;
  document.getElementById('resDaibi2').src = IMAGES.icon_daibi;
  document.getElementById('statWenyuIcon').src = IMAGES.icon_wenyu;
}

function updateViews() {
  const title = getTitle(state.totalDrawCount);

  // Main counts
  document.getElementById('mainBadgeTitle').textContent = title;
  document.getElementById('mainDraws').textContent = state.totalDrawCount;
  document.getElementById('mainMounts').textContent = state.totalMountCount;
  document.getElementById('mainStones').textContent = state.totalStoneCount;

  // Remaining pity
  const remaining = Math.max(0, 180 - state.nonTianCount);
  document.getElementById('pityCountX').textContent = remaining;
  document.getElementById('resPityCountX').textContent = remaining;

  // Stats modal (单列，7项核心数据，无横向越界)
  document.getElementById('statTitle').textContent = `【${title}】`;
  document.getElementById('statTotalDraws').textContent = `${state.totalDrawCount}次`;
  document.getElementById('statTotalTian').textContent = `${getTotalTianStones()}个`;
  document.getElementById('statFirstMount').textContent = state.firstMountDraw > 0 ? `${state.firstMountDraw}抽` : "暂无";
  document.getElementById('statMountCount').textContent = `${state.totalMountCount}个`;
  document.getElementById('statDirectStone').textContent = `${state.totalStoneCount}个`;
  document.getElementById('statMinPity').textContent = state.minTianPity > 0 ? `${state.minTianPity}抽` : "暂无";

  const spentWenyu = state.totalDrawCount * 180;
  document.getElementById('statSpentWenyu').textContent = `消耗: ${spentWenyu} 纹玉`;
}

function triggerDraw(count) {
  closeStatsModal();
  let hasTian = false;
  const results = [];
  for (let i = 0; i < count; i++) {
    const item = rollOne();
    if (item.quality === 2) hasTian = true;
    results.push(item);
  }

  updateViews();
  showResultModal(count, results, hasTian);

  if (hasTian) {
    playTianshangChime();
  }
}

function simulateLuckyTian() {
  closeStatsModal();
  state.totalDrawCount++;
  state.totalTianCount++;
  state.totalMountCount++;
  state.nonTianCount = 0;
  if (state.firstTianDraw === 0) state.firstTianDraw = state.totalDrawCount;
  if (state.firstMountDraw === 0) state.firstMountDraw = state.totalDrawCount;

  updateViews();
  showResultModal(1, [ITEMS[1]], true);
  playTianshangChime();
}

function showResultModal(count, results, hasTian) {
  const overlay = document.getElementById('resultOverlay');
  const singleBox = document.getElementById('resSingleBox');
  const tenGrid = document.getElementById('resTenGrid');

  overlay.hidden = false;

  // 当某次抽奖结果获得天赏物品时，抽奖结果界面只需要展示天赏物品的图标，其他的奇赏凡赏物品不需要展示
  if (hasTian) {
    const tianItems = results.filter(item => item.quality === 2);
    if (tianItems.length === 1) {
      singleBox.hidden = false;
      tenGrid.hidden = true;

      const item = tianItems[0];
      document.getElementById('resSingleBg').src = IMAGES['card_juepin'];
      document.getElementById('resSingleIcon').src = IMAGES[item.iconKey];
      document.getElementById('resSingleCount').hidden = true;
    } else {
      singleBox.hidden = true;
      tenGrid.hidden = false;
      tenGrid.innerHTML = '';

      tianItems.forEach((item) => {
        const card = document.createElement('div');
        card.className = 'res-ten-card is-tian';
        card.innerHTML = `
          <img class="card-bg-img" src="${IMAGES['card_juepin']}" alt="bg">
          <img class="card-item-img" src="${IMAGES[item.iconKey]}" alt="icon">
        `;
        tenGrid.appendChild(card);
      });
    }
  } else {
    if (count === 1) {
      singleBox.hidden = false;
      tenGrid.hidden = true;

      const item = results[0];
      const bgKey = item.quality === 1 ? 'card_xishi' : 'card_liangpin';
      document.getElementById('resSingleBg').src = IMAGES[bgKey];
      document.getElementById('resSingleIcon').src = IMAGES[item.iconKey];

      const countLbl = document.getElementById('resSingleCount');
      if (item.id === 3) {
        countLbl.hidden = false;
        countLbl.textContent = "20";
      } else {
        countLbl.hidden = true;
      }
    } else {
      singleBox.hidden = true;
      tenGrid.hidden = false;
      tenGrid.innerHTML = '';

      results.forEach((item) => {
        const card = document.createElement('div');
        card.className = 'res-ten-card';

        const bgKey = item.quality === 1 ? 'card_xishi' : 'card_liangpin';
        card.innerHTML = `
          <img class="card-bg-img" src="${IMAGES[bgKey]}" alt="bg">
          <img class="card-item-img" src="${IMAGES[item.iconKey]}" alt="icon">
          ${item.id === 3 ? '<span class="res-card-count">20</span>' : ''}
        `;
        tenGrid.appendChild(card);
      });
    }
  }
}

function closeResultModal() {
  document.getElementById('resultOverlay').hidden = true;
}

function toggleStatsModal() {
  closeResultModal();
  const overlay = document.getElementById('statsOverlay');
  overlay.hidden = !overlay.hidden;
}

function closeStatsModal() {
  document.getElementById('statsOverlay').hidden = true;
}

function resetState() {
  state.totalDrawCount = 0;
  state.nonTianCount = 0;
  state.consecutiveFanCount = 0;
  state.totalTianCount = 0;
  state.totalMountCount = 0;
  state.totalStoneCount = 0;
  state.firstMountDraw = 0;
  state.firstTianDraw = 0;
  state.minTianPity = 0;
  closeResultModal();
  closeStatsModal();
  updateViews();
}

let longPressTimer = null;
function startLongPress() {
  longPressTimer = setTimeout(() => {
    alert("长按 OK 键触发：数据已清空重置！");
    resetState();
    longPressTimer = null;
  }, 1000);
}

function cancelLongPress() {
  if (longPressTimer) {
    clearTimeout(longPressTimer);
    longPressTimer = null;
  }
}

function handleHwButton(btn) {
  const cap = document.getElementById(`btnHw${btn.charAt(0) + btn.slice(1).toLowerCase()}`);
  if (cap) {
    cap.classList.add('pressed');
    setTimeout(() => cap.classList.remove('pressed'), 120);
  }

  const resultOpen = !document.getElementById('resultOverlay').hidden;

  if (btn === 'UP') {
    triggerDraw(1);
  } else if (btn === 'DOWN') {
    triggerDraw(10);
  } else if (btn === 'OK') {
    if (resultOpen) {
      closeResultModal();
    } else {
      toggleStatsModal();
    }
  }
}

window.addEventListener('keydown', (e) => {
  if (e.key === 'ArrowUp') {
    e.preventDefault();
    handleHwButton('UP');
  } else if (e.key === 'ArrowDown') {
    e.preventDefault();
    handleHwButton('DOWN');
  } else if (e.key === ' ' || e.key === 'Enter') {
    e.preventDefault();
    handleHwButton('OK');
  }
});

function setZoom(scale, btnEl) {
  const screen = document.getElementById('lcdScreen');
  const bezel = document.querySelector('.screen-bezel');
  const w = 240 * scale;
  const h = 320 * scale;

  screen.style.transform = `scale(${scale})`;
  bezel.style.width = `${w + 16}px`;
  bezel.style.height = `${h + 16}px`;

  if (btnEl) {
    document.querySelectorAll('.zoom-btn').forEach(btn => btn.classList.remove('active'));
    btnEl.classList.add('active');
  }
}

window.addEventListener('DOMContentLoaded', () => {
  initImages();
  updateViews();
});
</script>
"""

full_content = html_template.replace("___IMAGES_PLACEHOLDER___", json.dumps(images))

os.makedirs(os.path.dirname(OUTPUT_HTML), exist_ok=True)
with open(OUTPUT_HTML, "w", encoding="utf-8") as f:
    f.write(full_content)

print(f"Prototype successfully built: {OUTPUT_HTML} (Size: {os.path.getsize(OUTPUT_HTML)} bytes)")
