function showTab(name, btn) {
  document.querySelectorAll('.panel').forEach(p => p.classList.remove('active'));
  document.querySelectorAll('.nav-item').forEach(t => t.classList.remove('active'));
  document.getElementById('tab-' + name).classList.add('active');
  btn.classList.add('active');

  if (name === 'graph') setTimeout(initGraph, 40);
  if (name === 'trace') renderTraceTable();
}

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

function showNoDataMessage() {
  document.querySelector('.content').innerHTML = `
    <div style="padding:48px 32px;font-family:var(--font-ui);color:var(--text-muted);max-width:520px;">
      <div style="font-size:1.1rem;font-weight:700;color:var(--text);margin-bottom:14px;">No data generated yet</div>
      <p style="line-height:1.8;margin-bottom:18px;">Run the parser to generate the visualizer data:</p>
      <pre style="background:var(--bg);border:1px solid var(--border);border-radius:6px;padding:14px;font-family:var(--mono);font-size:0.85rem;line-height:1.8;">cd Parser
mingw32-make
.\\build\\lr1_parser.exe --write-js "id = * id"</pre>
      <p style="line-height:1.8;margin-top:18px;">Then reload this page.</p>
      <div style="margin-top:24px;padding-top:14px;border-top:1px solid var(--border);font-size:0.84rem;">
        Or run the HTTP server and open <code>http://localhost:7373</code>:<br>
        <pre style="background:var(--bg);border:1px solid var(--border);border-radius:6px;padding:10px;margin-top:8px;font-family:var(--mono);font-size:0.85rem;">.\\build\\lr1_parser.exe --serve</pre>
      </div>
    </div>`;
}

document.addEventListener('DOMContentLoaded', () => {
  initOrbs();

  if (!DATA) {
    showNoDataMessage();
    return;
  }

  renderGrammar();
  renderParseTable();
  renderAllStates();
  loadAndParse();
});