function showTab(name, btn) {
  document.querySelectorAll('.panel').forEach(p => p.classList.remove('active'));
  document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
  document.getElementById('tab-' + name).classList.add('active');
  btn.classList.add('active');

  if (name === 'graph') setTimeout(initGraph, 40);
  if (name === 'trace') renderTraceTable();
}

// Spawn floating orbs in the header
function initOrbs() {
  const wrap = document.getElementById('header-orbs');
  const orbs = [
    { w: 80,  h: 80,  left: '7%',  top: '10%', bg: '#f48fb1', dur: '4s',   delay: '0s' },
    { w: 50,  h: 50,  left: '22%', top: '60%', bg: '#90caf9', dur: '5.5s', delay: '0.8s' },
    { w: 100, h: 100, left: '74%', top: '5%',  bg: '#bbdefb', dur: '6s',   delay: '1.2s' },
    { w: 60,  h: 60,  left: '87%', top: '55%', bg: '#f8bbd0', dur: '4.8s', delay: '0.4s' },
    { w: 40,  h: 40,  left: '50%', top: '70%', bg: '#90caf9', dur: '7s',   delay: '2s' }
  ];
  orbs.forEach(o => {
    const d = document.createElement('div');
    d.className = 'orb';
    d.style.cssText =
      `width:${o.w}px;height:${o.h}px;left:${o.left};top:${o.top};` +
      `background:${o.bg};animation-duration:${o.dur};animation-delay:${o.delay};`;
    wrap.appendChild(d);
  });
}

// Boot
document.addEventListener('DOMContentLoaded', () => {
  initOrbs();
  renderGrammar();
  renderParseTable();
  renderAllStates();
  loadAndParse();
});
