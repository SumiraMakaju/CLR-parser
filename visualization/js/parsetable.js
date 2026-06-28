function renderParseTable() {
  if (!DATA) return;
  const table    = document.getElementById('parse-table-el');
  const terms    = DATA.terminals.filter(t => t !== '$');
  const allTerms = [...terms, '$'];
  const nts      = DATA.nonTerminals.filter(n => n !== "S'");

  let html = '<thead><tr>';
  html += '<th class="h-state" rowspan="2">State</th>';
  html += `<th class="h-action" colspan="${allTerms.length}">ACTION</th>`;
  html += `<th class="h-goto"   colspan="${nts.length}">GOTO</th>`;
  html += '</tr><tr>';
  allTerms.forEach(t => { html += `<th class="h-action">${t}</th>`; });
  nts.forEach(n =>     { html += `<th class="h-goto">${n}</th>`; });
  html += '</tr></thead><tbody>';

  for (let i = 0; i < DATA.states.length; i++) {
    html += `<tr id="trow-${i}"><td class="state-cell">${i}</td>`;
    allTerms.forEach(t => {
      const act = (DATA.actionTable[i] || {})[t];
      if (!act) { html += '<td class="cell-empty"></td>'; return; }
      const cls = act === 'acc' ? 'cell-accept' : act.startsWith('s') ? 'cell-shift' : 'cell-reduce';
      html += `<td class="${cls}">${act}</td>`;
    });
    nts.forEach(n => {
      const gt = (DATA.gotoTable[i] || {})[n];
      html += gt !== undefined ? `<td class="cell-shift">${gt}</td>` : '<td class="cell-empty"></td>';
    });
    html += '</tr>';
  }
  table.innerHTML = html + '</tbody>';

  // Conflict banner
  const banner = document.getElementById('conflict-banner');
  if (DATA.hasConflict) {
    banner.className = 'banner banner-conflict';
    banner.textContent = 'Conflicts detected: ' + DATA.conflicts.join('; ');
  } else {
    banner.className = 'banner banner-ok';
    banner.textContent = 'No conflicts detected. The grammar is LR(1) and the parse table is deterministic.';
  }
}

function highlightTableRow(state) {
  document.querySelectorAll('.parse-table tr').forEach(r => r.classList.remove('active-row'));
  if (state < 0) return;
  const row = document.getElementById(`trow-${state}`);
  if (row) { row.classList.add('active-row'); row.scrollIntoView({ block: 'nearest', behavior: 'smooth' }); }
}