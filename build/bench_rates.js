#!/usr/bin/env node
/* 对拍:直接调用小程序的 oneTianshangRoll 真代码，统计同样的概率分布。 */
const path = require('path');

const MP = 'D:/Workspace/nsh-price/miniprogram';
const sim = require(path.join(MP, 'pages/gacha/utils/tianshangSim.js'));
const events = require(path.join(MP, 'data/gacha_events.js'));

// 找到 tianshang 活动配置
let event = null;
const candidates = [];
(function walk(node) {
  if (!node || typeof node !== 'object') return;
  if (Array.isArray(node)) { node.forEach(walk); return; }
  if (node.type === 'tianshang' && node.itemList) candidates.push(node);
  Object.values(node).forEach(walk);
})(events);
event = candidates[0];

if (!event) {
  console.error('FAIL: could not locate tianshang event config');
  process.exit(1);
}

const tianItems = event.itemList.filter(i => i.quality === '天赏');
console.log('=== mini-program tianshang item weights ===');
let totalW = 0;
for (const it of tianItems) {
  const w = typeof it.probability === 'number' ? it.probability : 0;
  totalW += w;
}
for (const it of tianItems) {
  const w = typeof it.probability === 'number' ? it.probability : 0;
  console.log(`  ${it.name.padEnd(8)} prob=${w}  share=${(w / totalW).toFixed(6)}`);
}
console.log(`  (sum of 天赏 weights = ${totalW})`);

const N = 2000000;
let counters = { consecutiveNonTianCount: 0, consecutiveFanCount: 0 };
const backpack = {};
let tian = 0, mount = 0, stone = 0, qi = 0, fan = 0;

for (let i = 0; i < N; i++) {
  const r = sim.oneTianshangRoll(counters, event, backpack);
  counters = r.newCounters;
  if (r.resQuality === '天赏') {
    tian++;
    if (r.item.isMount) mount++; else stone++;
  } else if (r.resQuality === '奇赏') qi++;
  else fan++;
}

console.log(`\n=== mini-program oneTianshangRoll (N=${N}) ===`);
console.log(`P(天赏)            = ${(tian / N).toFixed(6)}  (1 per ${(N / tian).toFixed(1)} draws)`);
console.log(`P(奇赏)            = ${(qi / N).toFixed(6)}`);
console.log(`P(凡赏)            = ${(fan / N).toFixed(6)}`);
console.log(`P(祥瑞)  overall   = ${(mount / N).toFixed(8)}`);
console.log(`P(天赏石) overall  = ${(stone / N).toFixed(8)}`);
console.log(`--- conditional on 天赏 ---`);
console.log(`P(祥瑞 | 天赏)     = ${(mount / tian).toFixed(6)}  (expected 0.10)`);
console.log(`P(天赏石 | 天赏)   = ${(stone / tian).toFixed(6)}  (expected 0.90)`);
console.log(`counts: tian=${tian} mount=${mount} stone=${stone}`);
