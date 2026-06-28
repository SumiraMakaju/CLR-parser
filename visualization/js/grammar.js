function renderGrammar() {
  if (!DATA) return;

  const grid = document.getElementById('grammar-productions');
  grid.innerHTML = '';
  DATA.productions.forEach((p, i) => {
    const row = document.createElement('div');
    row.className = 'prod-row';
    row.style.animationDelay = (i * 0.05) + 's';
    row.innerHTML =
      `<span class="prod-id">${p.id}</span>` +
      `<span class="prod-text">${p.lhs} &rarr; ${p.rhs.join(' ')}</span>`;
    grid.appendChild(row);
  });

  const tEl = document.getElementById('grammar-terminals');
  tEl.innerHTML = '';
  DATA.terminals.forEach(t => {
    const s = document.createElement('span');
    s.className  = 'sym-chip sym-terminal';
    s.textContent = t;
    tEl.appendChild(s);
  });

  const ntEl = document.getElementById('grammar-nonterminals');
  ntEl.innerHTML = '';
  DATA.nonTerminals.forEach(nt => {
    const s = document.createElement('span');
    s.className  = 'sym-chip sym-nonterminal';
    s.textContent = nt;
    ntEl.appendChild(s);
  });

  const fsEl = document.getElementById('grammar-first');
  fsEl.innerHTML = '';
  const firstSets = DATA.firstSets || {};
  Object.entries(firstSets).forEach(([sym, set]) => {
    const div = document.createElement('div');
    div.className = 'first-entry';
    div.innerHTML =
      `<span class="first-sym">FIRST(${sym})</span> = { ${set.join(', ')} }`;
    fsEl.appendChild(div);
  });
}