let currentStep  = -1;
let parseSteps   = DATA.parseResult.steps;
let autoTimer    = null;
let activeStates = new Set();

function simulateParse(tokens) {
  const steps  = [];
  const input  = [...tokens, '$'];
  const stateStack  = [0];
  const symStack    = [];
  let pos = 0;

  while (true) {
    const curState = stateStack[stateStack.length - 1];
    const tok      = input[pos];
    const step = {
      stateStack:       [...stateStack],
      symbolStack:      [...symStack],
      input:            input.slice(pos),
      reduceProduction: -1
    };

    const act = (DATA.actionTable[curState] || {})[tok];
    if (!act) {
      step.action     = `Error: no action for state ${curState} on '${tok}'`;
      step.actionType = 'error';
      steps.push(step);
      break;
    }

    if (act === 'acc') {
      step.action     = 'Accept! Input parsed successfully.';
      step.actionType = 'accept';
      steps.push(step);
      break;
    } else if (act.startsWith('s')) {
      const ns = parseInt(act.slice(1));
      step.action     = `Shift ${tok}, go to state ${ns}`;
      step.actionType = 'shift';
      steps.push(step);
      stateStack.push(ns);
      symStack.push(tok);
      pos++;
    } else if (act.startsWith('r')) {
      const pid  = parseInt(act.slice(1));
      const prod = DATA.productions[pid];
      step.action            = `Reduce by: ${prod.lhs} -> ${prod.rhs.join(' ')}`;
      step.actionType        = 'reduce';
      step.reduceProduction  = pid;
      steps.push(step);
      for (let i = 0; i < prod.rhs.length; i++) { stateStack.pop(); symStack.pop(); }
      symStack.push(prod.lhs);
      const gt = (DATA.gotoTable[stateStack[stateStack.length - 1]] || {})[prod.lhs];
      if (gt === undefined) break;
      stateStack.push(gt);
    }

    if (steps.length > 300) break;
  }
  return steps;
}

function loadAndParse() {
  const raw    = document.getElementById('input-string').value.trim();
  const tokens = raw.split(/\s+/).filter(Boolean);

  parseSteps = (tokens.join(' ') === 'id = * id')
    ? DATA.parseResult.steps
    : simulateParse(tokens);

  currentStep = 0;
  buildStepLog();
  renderStep(0);
  if (autoTimer) { clearInterval(autoTimer); autoTimer = null; }
  document.getElementById('btn-auto').textContent = 'Auto';
  document.getElementById('btn-back').disabled = false;
  document.getElementById('btn-fwd').disabled  = false;
  document.getElementById('btn-auto').disabled = false;
}

function quickLoad(str) {
  document.getElementById('input-string').value = str;
  loadAndParse();
}

function renderStep(idx) {
  if (idx < 0 || idx >= parseSteps.length) return;
  const step  = parseSteps[idx];
  currentStep = idx;
  const total = parseSteps.length;

  // Progress
  document.getElementById('step-counter').textContent = `Step ${idx + 1} / ${total}`;
  document.getElementById('progress-bar').style.width = `${((idx + 1) / total) * 100}%`;
  document.getElementById('btn-back').disabled = idx === 0;
  document.getElementById('btn-fwd').disabled  = idx === total - 1;

  // Tape
  const tapeEl     = document.getElementById('tape-display');
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
  const ssEl     = document.getElementById('state-stack');
  ssEl.innerHTML = '';
  step.stateStack.forEach((s, i) => {
    const cell = document.createElement('div');
    const isTop = i === step.stateStack.length - 1;
    cell.className = 'stack-cell cell-state' + (isTop ? ' cell-top' : '');
    cell.textContent = s;
    ssEl.appendChild(cell);
  });

  // Symbol stack
  const symEl     = document.getElementById('symbol-stack');
  symEl.innerHTML = '';
  ['$', ...step.symbolStack].forEach((s, i) => {
    const cell = document.createElement('div');
    const isTop = i === step.symbolStack.length;
    cell.className = 'stack-cell cell-sym' + (isTop ? ' cell-top' : '');
    cell.textContent = s;
    symEl.appendChild(cell);
  });

  // Action box
  const tags   = { shift:'SHF', reduce:'RED', accept:'ACC', error:'ERR' };
  const actEl  = document.getElementById('action-display');
  actEl.innerHTML =
    `<div class="action-box action-${step.actionType}">` +
    `<span class="action-tag tag-${step.actionType}">${tags[step.actionType] || '?'}</span>` +
    `<span>${step.action}</span>` +
    `</div>`;

  // Current state items
  const curState  = step.stateStack[step.stateStack.length - 1];
  const stateData = DATA.states.find(s => s.id === curState);
  document.getElementById('cur-state-num').textContent = curState;
  if (stateData) {
    document.getElementById('cur-state-items').innerHTML =
      stateData.items.map(formatItem).join('<br>');
  }

  // Highlight step log entry
  document.querySelectorAll('.log-entry').forEach((el, i) => {
    el.classList.toggle('active-log', i === idx);
    if (i === idx) el.scrollIntoView({ block: 'nearest', behavior: 'smooth' });
  });

  // Sync parse table and graph
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
    const div = document.createElement('div');
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

function autoPlay() {
  if (autoTimer) {
    clearInterval(autoTimer);
    autoTimer = null;
    document.getElementById('btn-auto').textContent = 'Auto';
    return;
  }
  document.getElementById('btn-auto').textContent = 'Stop';
  autoTimer = setInterval(() => {
    if (currentStep >= parseSteps.length - 1) {
      clearInterval(autoTimer);
      autoTimer = null;
      document.getElementById('btn-auto').textContent = 'Auto';
      return;
    }
    stepForward();
  }, 850);
}
