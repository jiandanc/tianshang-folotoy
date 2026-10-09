#!/usr/bin/env node
/* 三方对拍：原型 HTML、固件 C 参考值、小程序 JS 的称号阶梯 + 概率分布一致性检查。 */
const fs = require('fs');
const vm = require('vm');
const path = require('path');

// ---- 1. 原型 HTML ----
const html = fs.readFileSync('D:/Workspace/tianshang-folotoy/build/tianshang_prototype.html', 'utf8');
const blocks = [];
const re = /<script(?![^>]*\bsrc=)[^>]*>([\s\S]*?)<\/script>/gi;
let m;
while ((m = re.exec(html)) !== null) blocks.push(m[1]);
const code = blocks.join('\n;\n') + '\n;globalThis.__probe = { state, rollOne, getTitle };';

function makeStub(name) {
  const fn = function () { return stub; };
  const stub = new Proxy(fn, {
    get(t, prop) {
      if (prop === Symbol.toPrimitive || prop === 'toString') return () => name;
      if (prop === 'length') return 0;
      if (prop === 'then') return undefined;
      return stub;
    },
    set() { return true; },
    apply() { return stub; },
  });
  return stub;
}
const sandbox = {
  console, Math, Date, JSON,
  document: makeStub('document'), window: makeStub('window'),
  Image: function () { return makeStub('img'); },
  AudioContext: undefined, webkitAudioContext: undefined,
  requestAnimationFrame: () => 0, setTimeout: () => 0,
};
sandbox.globalThis = sandbox;
vm.createContext(sandbox);
vm.runInContext(code, sandbox, { timeout: 20000 });
const { getTitle, rollOne, state } = sandbox.__probe;

// ---- 2. 小程序 JS ----
const MP = 'D:/Workspace/nsh-price/miniprogram';
const constants = require(path.join(MP, 'pages/gacha/utils/constants.js'));
const sim = require(path.join(MP, 'pages/gacha/utils/tianshangSim.js'));
const events = require(path.join(MP, 'data/gacha_events.js'));
let ev = null;
(function w(n) { if (!n || typeof n !== 'object') return; if (Array.isArray(n)) { n.forEach(w); return; } if (n.type === 'tianshang' && n.itemList && !ev) ev = n; Object.values(n).forEach(w); })(events);

// 直接调用小程序 constants.js 的权威实现，避免再抄一份阶梯导致新一轮漂移
function mpTitle(draws) {
  return constants.calculateTitleLocal((draws * 18) / 10000);
}

// ---- 3. 对比称号边界 ----
const boundaries = [0, 11, 12, 55, 56, 166, 167, 555, 556, 2777, 2778, 5555, 5556, 27777, 27778, 55555, 55556, 83333, 83334, 100000];
let titleBad = 0;
console.log('=== 称号阶梯对比 (draws -> title) ===');
console.log('draws    prototype            mini-program         match');
for (const d of boundaries) {
  const p = getTitle(d), q = mpTitle(d);
  const ok = p === q;
  if (!ok) titleBad++;
  console.log(`${String(d).padStart(6)}   ${p.padEnd(20)} ${q.padEnd(20)} ${ok ? 'OK' : 'MISMATCH'}`);
}
console.log(titleBad === 0 ? '\nTITLE TIERS: ALL MATCH\n' : `\nTITLE TIERS: ${titleBad} MISMATCH(ES)\n`);

// ---- 4. 概率对比 ----
const N = 1000000;
let tian = 0, mount = 0, stone = 0;
for (let i = 0; i < N; i++) {
  const it = rollOne();
  if (it.quality === 2) { tian++; if (it.id === 0) stone++; else mount++; }
}
console.log(`=== 原型概率 (N=${N}) ===`);
console.log(`P(天赏)          = ${(tian / N).toFixed(6)}  (same order as 固件 0.008077 / 小程序 0.008063)`);
console.log(`P(祥瑞 | 天赏)   = ${(mount / tian).toFixed(6)}  (expected 0.10)`);
console.log(`P(天赏石 | 天赏) = ${(stone / tian).toFixed(6)}  (expected 0.90)`);

process.exit(titleBad === 0 ? 0 : 1);
