const BACKEND = 'http://localhost:7373';

let DATA         = null;
let currentStep  = -1;
let parseSteps   = [];
let autoTimer    = null;
let activeStates = new Set();

// Speed levels: 1=slowest, 5=fastest
const SPEED_MAP = { 1: 1600, 2: 1000, 3: 600, 4: 300, 5: 100 };
let autoSpeed = 600;

// Called on page load and on every Parse button press
async function loadAndParse() {
  const raw = document.getElementById('input-string').value.trim();
  if (!raw) return;

  setLoading(true);

  try {
    const res = await fetch(`${BACKEND}/parse?q=${encodeURIComponent(raw)}`);
    if (!res.ok) throw new Error(`Server returned ${res.status}`);
    const json = await res.json();

    // Store globally so all other modules can read it
    DATA = json;

    // Re-render all tabs with fresh data
    renderGrammar();
    renderParseTable();
    renderAllStates();

    // Load parse steps
    parseSteps  = DATA.parseResult.steps;
    currentStep = 0;
    buildStepLog();
    renderStep(0);

    setLoading(false);
    document.getElementById('btn-back').disabled = false;
    document.getElementById('btn-fwd').disabled  = false;
    document.getElementById('btn-auto').disabled = false;
    if (autoTimer) { clearInterval(autoTimer); autoTimer = null; }
    document.getElementById('btn-auto').textContent = 'Auto';

  } catch (err) {
    setLoading(false);
    showError(`Cannot reach backend at ${BACKEND}. Run: .\\build\\lr1_parser.exe --serve`);
  }
}

function quickLoad(str) {
  document.getElementById('input-string').value = str;
  loadAndParse();
}

function setLoading(on) {
  const btn = document.querySelector('.btn-pink');
  if (btn) btn.textContent = on ? 'Parsing...' : 'Parse';
}

function showError(msg) {
  const el = document.getElementById('action-display');
  if (el) el.innerHTML =
    `<div class="action-box action-error">
       <span class="action-tag tag-error">ERR</span>
       <span>${msg}</span>
     </div>`;
}

function renderStep(idx) {
  if (!DATA || idx < 0 || idx >= parseSteps.length) return;
  const step  = parseSteps[idx];
  currentStep = idx;
  const total = parseSteps.length;

  document.getElementById('step-counter').textContent = `Step ${idx + 1} / ${total}`;
  document.getElementById('progress-bar').style.width = `${((idx + 1) / total) * 100}%`;
  document.getElementById('btn-back').disabled = idx === 0;
  document.getElementById('btn-fwd').disabled  = idx === total - 1;

  // Tape
  const tapeEl    = document.getElementById('tape-display');
  tapeEl.innerHTML = '';
  const allTokens  = parseSteps[0].input;
  const consumed   = allTokens.length - step.input.length;
  allTokens.forEach((tok, i) => {
    const cell = document.createElement('span');
    cell.className = 'tape-cell' +
      (i === consumed ? ' current' : i < consumed ? ' consumed' : '') +
      (tok === '$' ? ' eof' : '');
    cell.textContent = tok;
    tapeEl.appendChild(cell);
  });

  // State stack
  const ssEl      = document.getElementById('state-stack');
  ssEl.innerHTML  = '';
  step.stateStack.forEach((s, i) => {
    const cell    = document.createElement('div');
    const isTop   = i === step.stateStack.length - 1;
    cell.className = 'stack-cell cell-state' + (isTop ? ' cell-top' : '');
    cell.textContent = s;
    ssEl.appendChild(cell);
  });

  // Symbol stack
  const symEl     = document.getElementById('symbol-stack');
  symEl.innerHTML = '';
  ['$', ...step.symbolStack].forEach((s, i) => {
    const cell    = document.createElement('div');
    const isTop   = i === step.symbolStack.length;
    cell.className = 'stack-cell cell-sym' + (isTop ? ' cell-top' : '');
    cell.textContent = s;
    symEl.appendChild(cell);
  });

  // Action box
  const tags  = { shift:'SHF', reduce:'RED', accept:'ACC', error:'ERR' };
  document.getElementById('action-display').innerHTML =
    `<div class="action-box action-${step.actionType}">
       <span class="action-tag tag-${step.actionType}">${tags[step.actionType] || '?'}</span>
       <span>${step.action}</span>
     </div>`;

  // Current state items
  const curState  = step.stateStack[step.stateStack.length - 1];
  const stateData = DATA.states.find(s => s.id === curState);
  document.getElementById('cur-state-num').textContent = curState;
  if (stateData) {
    document.getElementById('cur-state-items').innerHTML =
      stateData.items.map(formatItem).join('<br>');
  }

  // Sync log
  document.querySelectorAll('.log-entry').forEach((el, i) => {
    el.classList.toggle('active-log', i === idx);
    if (i === idx) el.scrollIntoView({ block: 'nearest', behavior: 'smooth' });
  });

  // Sync parse table highlight and graph
  highlightTableRow(curState);
  activeStates.clear();
  step.stateStack.forEach(s => activeStates.add(s));
  if (typeof drawGraph === 'function') drawGraph();
}

function formatItem(str) {
  return str
    .replace(/\./g, '<span class="item-dot">&#9679;</span>')
    .replace(/, ([^,]+)$/, ', <span class="item-la">$1</span>');
}

function buildStepLog() {
  const log     = document.getElementById('step-log');
  log.innerHTML = '';
  parseSteps.forEach((step, i) => {
    const div     = document.createElement('div');
    div.className = 'log-entry';
    div.onclick   = () => renderStep(i);
    div.innerHTML =
      `<span class="log-num">${i + 1}</span>` +
      `<span class="log-action">${step.action}</span>` +
      `<span class="log-type type-${step.actionType}">${step.actionType}</span>`;
    log.appendChild(div);
  });
}

function stepForward() { if (currentStep < parseSteps.length - 1) renderStep(currentStep + 1); }
function stepBack()    { if (currentStep > 0) renderStep(currentStep - 1); }

function updateSpeed(val) {
  autoSpeed = SPEED_MAP[val] || 600;
  document.getElementById('speed-val').textContent = val;
  if (autoTimer) { clearInterval(autoTimer); autoTimer = setInterval(autoStep, autoSpeed); }
}

function autoStep() {
  if (currentStep >= parseSteps.length - 1) {
    clearInterval(autoTimer); autoTimer = null;
    document.getElementById('btn-auto').textContent = 'Auto';
    return;
  }
  stepForward();
}

function autoPlay() {
  if (autoTimer) {
    clearInterval(autoTimer); autoTimer = null;
    document.getElementById('btn-auto').textContent = 'Auto';
    return;
  }
  document.getElementById('btn-auto').textContent = 'Stop';
  autoTimer = setInterval(autoStep, autoSpeed);
}