function renderTraceTable() {
  if (!DATA || !parseSteps.length) return;
  const table = document.getElementById('trace-table-el');
  let html =
    `<thead><tr>
      <th class="h-state"  style="min-width:44px;">Step</th>
      <th class="h-state"  style="min-width:110px;">State Stack</th>
      <th class="h-action" style="min-width:110px;">Symbol Stack</th>
      <th class="h-goto"   style="min-width:120px;">Input</th>
      <th class="h-action" style="min-width:64px;">Action</th>
      <th class="h-goto"   style="min-width:180px;">Description</th>
    </tr></thead><tbody>`;

  parseSteps.forEach((step, i) => {
    const stateStr = step.stateStack.join(' ');
    const symStr   = ['$', ...step.symbolStack].join(' ');
    const inputStr = step.input.join(' ');
    const aType    = step.actionType;
    let short = aType==='shift'  ? `s${(step.action.match(/state (\d+)/)||['','?'])[1]}`
              : aType==='reduce' ? `r${step.reduceProduction}`
              : aType==='accept' ? 'acc' : 'err';
    const cls    = aType==='shift'?'cell-shift':aType==='reduce'?'cell-reduce':aType==='accept'?'cell-accept':'';
    const rowCls = i===currentStep?'active-row':'';
    html +=
      `<tr class="${rowCls}" onclick="renderStep(${i})" style="cursor:pointer;">
        <td class="state-cell">${i+1}</td>
        <td style="font-family:var(--mono);text-align:left;padding-left:9px;">${stateStr}</td>
        <td style="font-family:var(--mono);text-align:left;padding-left:9px;">${symStr}</td>
        <td style="font-family:var(--mono);text-align:left;padding-left:9px;">${inputStr}</td>
        <td class="${cls}" style="font-weight:700;">${short}</td>
        <td style="text-align:left;padding-left:9px;color:var(--text-muted);font-size:0.77rem;">${step.action}</td>
      </tr>`;
  });
  table.innerHTML = html + '</tbody>';
}