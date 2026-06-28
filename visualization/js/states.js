function renderAllStates() {
  if (!DATA) return;
  const el  = document.getElementById('all-states');
  let html  = '<div class="states-grid">';

  DATA.states.forEach(s => {
    const isAccept = s.items.some(i => i.includes("S' -> S ."));
    const isStart  = s.id === 0;
    html += `<div class="state-block${isAccept?' state-accept':''}${isStart?' state-start':''}"
                  onmouseover="this.style.transform='translateY(-2px)'"
                  onmouseout="this.style.transform=''">`;
    html += `<div class="state-block-title">I${s.id}`;
    if (isAccept) html += ` <span class="state-tag tag-accept">ACCEPT</span>`;
    if (isStart)  html += ` <span class="state-tag tag-start">START</span>`;
    html += `</div><div class="state-block-items">`;
    s.items.forEach(item => { html += `<div class="item-line">${formatItem(item)}</div>`; });
    html += `</div></div>`;
  });

  el.innerHTML = html + '</div>';
}