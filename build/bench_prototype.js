#!/usr/bin/env node
/* 对拍:从生成的原型 HTML 中抽取真实脚本，跑 rollOne 统计概率分布。
 * 用 DOM stub 吞掉所有 UI 调用，只保留抽奖逻辑。 */
const fs = require('fs');
const vm = require('vm');

const html = fs.readFileSync('D:/Workspace/tianshang-folotoy/build/tianshang_prototype.html', 'utf8');

// 抽出所有内联 <script> 块（无 src 属性）
const blocks = [];
const re = /<script(?![^>]*\bsrc=)[^>]*>([\s\S]*?)<\/script>/gi;
let m;
while ((m = re.exec(html)) !== null) blocks.push(m[1]);
if (blocks.length === 0) { console.error('FAIL: no inline script found'); process.exit(1); }
const code = blocks.join('\n;\n') + '\n;globalThis.__probe = { state, rollOne };';

// 万能 DOM stub：任何属性访问都返回自身，可调用、可索引
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
  console,
  Math,
  Date,
  JSON,
  document: makeStub('document'),
  window: makeStub('window'),
  Image: function () { return makeStub('img'); },
  AudioContext: undefined,
  webkitAudioContext: undefined,
  requestAnimationFrame: () => 0,
  setTimeout: () => 0,
};
sandbox.globalThis = sandbox;
vm.createContext(sandbox);

try {
  vm.runInContext(code, sandbox, { timeout: 20000 });
} catch (e) {
  console.error('FAIL running prototype script:', e.message);
  process.exit(1);
}

const { state, rollOne } = sandbox.__probe || {};
if (typeof rollOne !== 'function' || !state) {
  console.error('FAIL: rollOne/state not exposed by prototype script');
  process.exit(1);
}

const N = 2000000;
let tian = 0, mount = 0, stone = 0, qi = 0, fan = 0;

for (let i = 0; i < N; i++) {
  const it = rollOne();
  // 通过品质+名称归类，避免依赖内部计数
  if (it.quality === 2) {
    tian++;
    if (it.id === 0) stone++; else mount++;
  } else if (it.quality === 1) qi++;
  else fan++;
}

console.log(`=== prototype rollOne (N=${N}) ===`);
console.log(`P(天赏)            = ${(tian / N).toFixed(6)}  (1 per ${(N / tian).toFixed(1)} draws)`);
console.log(`P(奇赏)            = ${(qi / N).toFixed(6)}`);
console.log(`P(凡赏)            = ${(fan / N).toFixed(6)}`);
console.log(`P(祥瑞)  overall   = ${(mount / N).toFixed(8)}`);
console.log(`P(天赏石) overall  = ${(stone / N).toFixed(8)}`);
console.log(`--- conditional on 天赏 ---`);
console.log(`P(祥瑞 | 天赏)     = ${(mount / tian).toFixed(6)}  (expected 0.10)`);
console.log(`P(天赏石 | 天赏)   = ${(stone / tian).toFixed(6)}  (expected 0.90)`);
console.log(`counts: tian=${tian} mount=${mount} stone=${stone}`);
