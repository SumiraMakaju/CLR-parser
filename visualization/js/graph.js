let nodePos      = {};
let graphReady   = false;
let dragging     = null;
let pulseT       = 0;
let animFrame    = null;

function initGraph() {
  if (graphReady) { drawGraph(); return; }

  const canvas = document.getElementById('graph-canvas');
  const W = canvas.offsetWidth || 900;
  const H = 520;
  canvas.width  = W;
  canvas.height = H;

  // Layered top-to-bottom layout
  const layout = {
    0:  { x: W * 0.50, y: 55  },
    1:  { x: W * 0.34, y: 155 },
    2:  { x: W * 0.66, y: 155 },
    3:  { x: W * 0.80, y: 255 },
    4:  { x: W * 0.20, y: 255 },
    5:  { x: W * 0.36, y: 355 },
    6:  { x: W * 0.50, y: 275 },
    7:  { x: W * 0.13, y: 385 },
    8:  { x: W * 0.26, y: 455 },
    9:  { x: W * 0.46, y: 415 },
    10: { x: W * 0.64, y: 385 },
    11: { x: W * 0.63, y: 185 },
    12: { x: W * 0.77, y: 390 },
    13: { x: W * 0.82, y: 460 }
  };
  Object.assign(nodePos, layout);

  // Drag support
  canvas.addEventListener('mousedown', e => {
    const { mx, my } = canvasMouse(e, canvas);
    for (const [id, pos] of Object.entries(nodePos)) {
      const dx = mx - pos.x, dy = my - pos.y;
      if (dx * dx + dy * dy <= 28 * 28) {
        dragging = parseInt(id);
        canvas.style.cursor = 'grabbing';
        break;
      }
    }
  });
  canvas.addEventListener('mousemove', e => {
    if (dragging === null) return;
    const { mx, my } = canvasMouse(e, canvas);
    nodePos[dragging] = { x: mx, y: my };
  });
  canvas.addEventListener('mouseup', e => {
    dragging = null;
    canvas.style.cursor = 'grab';
    onGraphClick(e, canvas);
  });
  canvas.addEventListener('mouseleave', () => {
    dragging = null;
    canvas.style.cursor = 'grab';
  });

  graphReady = true;
  animateGraph();
}

function canvasMouse(e, canvas) {
  const rect = canvas.getBoundingClientRect();
  return { mx: e.clientX - rect.left, my: e.clientY - rect.top };
}

function animateGraph() {
  pulseT += 0.035;
  drawGraph();
  animFrame = requestAnimationFrame(animateGraph);
}

function drawGraph() {
  const canvas = document.getElementById('graph-canvas');
  if (!canvas) return;
  const ctx = canvas.getContext('2d');
  const W   = canvas.width;
  const H   = canvas.height;
  ctx.clearRect(0, 0, W, H);

  // Dot-grid background
  ctx.save();
  ctx.strokeStyle = '#f5e8f0';
  ctx.lineWidth   = 1;
  for (let x = 0; x < W; x += 36) { ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, H); ctx.stroke(); }
  for (let y = 0; y < H; y += 36) { ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(W, y); ctx.stroke(); }
  ctx.restore();

  const R     = 26;
  const pulse = 0.5 + 0.5 * Math.sin(pulseT * 2);

  // Draw edges
  DATA.transitions.forEach(t => {
    const from = nodePos[t.from];
    const to   = nodePos[t.to];
    if (!from || !to) return;
    const bothActive = activeStates.has(t.from) && activeStates.has(t.to);
    ctx.save();

    if (t.from === t.to) {
      // Self-loop
      ctx.beginPath();
      ctx.arc(from.x, from.y - R - 14, 14, 0, 2 * Math.PI);
      ctx.strokeStyle = bothActive ? `rgba(233,30,140,${0.7 + 0.3 * pulse})` : '#ddd';
      ctx.lineWidth   = bothActive ? 2.5 : 1.5;
      if (bothActive) { ctx.shadowColor = '#e91e8c'; ctx.shadowBlur = 8; }
      ctx.stroke();
      ctx.shadowBlur  = 0;

      ctx.font      = '700 10px Consolas, monospace';
      ctx.fillStyle = bothActive ? '#b71c5e' : '#aaa';
      ctx.textAlign = 'center';
      ctx.fillText(t.label, from.x, from.y - R - 32);
    } else {
      const dx  = to.x - from.x, dy = to.y - from.y;
      const len = Math.sqrt(dx * dx + dy * dy) || 1;
      const ux  = dx / len, uy = dy / len;
      const ox  = -uy * 18, oy = ux * 18;
      const mx  = (from.x + to.x) / 2 + ox;
      const my  = (from.y + to.y) / 2 + oy;
      const sx  = from.x + ux * R, sy = from.y + uy * R;
      const ex  = to.x   - ux * R, ey = to.y   - uy * R;

      // Glow layer for active edges
      if (bothActive) {
        ctx.beginPath();
        ctx.moveTo(sx, sy);
        ctx.quadraticCurveTo(mx, my, ex, ey);
        ctx.strokeStyle = `rgba(233,30,140,${0.12 + 0.1 * pulse})`;
        ctx.lineWidth   = 10;
        ctx.stroke();
      }

      // Main edge
      ctx.beginPath();
      ctx.moveTo(sx, sy);
      ctx.quadraticCurveTo(mx, my, ex, ey);
      ctx.strokeStyle = bothActive ? `rgba(233,30,140,${0.85 + 0.15 * pulse})` : '#d0c4d0';
      ctx.lineWidth   = bothActive ? 2.5 : 1.5;
      if (bothActive) { ctx.shadowColor = '#e91e8c'; ctx.shadowBlur = 6; }
      ctx.stroke();
      ctx.shadowBlur = 0;

      // Arrowhead
      const tmx = mx - ex, tmy = my - ey;
      const tl  = Math.sqrt(tmx * tmx + tmy * tmy) || 1;
      const ax  = tmx / tl, ay = tmy / tl;
      ctx.beginPath();
      ctx.moveTo(ex, ey);
      ctx.lineTo(ex + ax * 11 - ay * 5, ey + ay * 11 + ax * 5);
      ctx.lineTo(ex + ax * 11 + ay * 5, ey + ay * 11 - ax * 5);
      ctx.closePath();
      ctx.fillStyle = bothActive ? '#e91e8c' : '#c4b4c4';
      if (bothActive) { ctx.shadowColor = '#e91e8c'; ctx.shadowBlur = 5; }
      ctx.fill();
      ctx.shadowBlur = 0;

      // Edge label background + text
      const isTermLabel = DATA.terminals.includes(t.label);
      ctx.font      = '700 10px Consolas, monospace';
      const tw      = ctx.measureText(t.label).width;
      ctx.fillStyle = 'rgba(253,246,249,0.88)';
      ctx.fillRect(mx - tw / 2 - 3, my - 11, tw + 6, 14);
      ctx.fillStyle = bothActive ? '#b71c5e' : (isTermLabel ? '#1565c0' : '#6a1b9a');
      ctx.textAlign = 'center';
      ctx.fillText(t.label, mx, my);
    }
    ctx.restore();
  });

  // Draw nodes
  DATA.states.forEach(s => {
    const pos      = nodePos[s.id];
    if (!pos) return;
    const isActive = activeStates.has(s.id);
    const isAccept = s.items.some(i => i.includes("S' -> S ."));
    const isStart  = s.id === 0;
    ctx.save();

    // Pulse ring for active nodes
    if (isActive) {
      const pr = R + 5 + 4 * pulse;
      ctx.beginPath();
      ctx.arc(pos.x, pos.y, pr, 0, 2 * Math.PI);
      ctx.strokeStyle = `rgba(233,30,140,${0.15 + 0.2 * pulse})`;
      ctx.lineWidth   = 4;
      ctx.stroke();
    }

    // Node fill
    ctx.beginPath();
    ctx.arc(pos.x, pos.y, R, 0, 2 * Math.PI);
    if (isActive) {
      ctx.fillStyle   = '#fce4ec';
      ctx.strokeStyle = '#e91e8c';
      ctx.lineWidth   = 3;
      ctx.shadowColor = '#e91e8c';
      ctx.shadowBlur  = 12 * pulse;
    } else if (isAccept) {
      ctx.fillStyle   = '#e8f5e9';
      ctx.strokeStyle = '#43a047';
      ctx.lineWidth   = 3;
    } else if (isStart) {
      ctx.fillStyle   = '#fffde7';
      ctx.strokeStyle = '#f9a825';
      ctx.lineWidth   = 2.5;
    } else {
      ctx.fillStyle   = '#e3f2fd';
      ctx.strokeStyle = '#90caf9';
      ctx.lineWidth   = 2;
    }
    ctx.fill();
    ctx.stroke();
    ctx.shadowBlur = 0;

    // Double ring for accept state
    if (isAccept) {
      ctx.beginPath();
      ctx.arc(pos.x, pos.y, R - 5, 0, 2 * Math.PI);
      ctx.strokeStyle = '#43a047';
      ctx.lineWidth   = 1.5;
      ctx.stroke();
    }

    // Node label
    ctx.font      = `600 12px 'Helvetica Neue', Arial, sans-serif`;
    ctx.fillStyle = isActive ? '#b71c5e' : isAccept ? '#2e7d32' : isStart ? '#e65100' : '#1565c0';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.fillText(`I${s.id}`, pos.x, pos.y);
    ctx.restore();
  });
}

let lastClick = 0;
function onGraphClick(e, canvas) {
  const now = Date.now();
  if (now - lastClick < 200) return;
  lastClick = now;

  const { mx, my } = canvasMouse(e, canvas);
  for (const [id, pos] of Object.entries(nodePos)) {
    const dx = mx - pos.x, dy = my - pos.y;
    if (dx * dx + dy * dy <= 26 * 26) {
      const stateId  = parseInt(id);
      const state    = DATA.states.find(s => s.id === stateId);
      if (!state) break;
      const info     = document.getElementById('graph-info');
      info.style.display = 'block';
      info.innerHTML =
        `<strong>State I${stateId}</strong><br><br>` +
        state.items.map(formatItem).join('<br>');
      break;
    }
  }
}
