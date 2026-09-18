// Range FEA icon set generator: one visual language for all UI icons.
// Grid 24x24, stroke 1.5, round caps/joins, light tinted fills, shared corner badges.
// Generates light icons into this directory and dark color scheme variants into ./dark.
// Usage: node generate_icons.js
const fs = require('fs');
const path = require('path');
const OUT = __dirname;

// Palettes: light is used on light backgrounds, dark on dark color scheme.
// Every color has one meaning: ink = neutral objects, blue = geometry/navigation,
// teal = results, violet = problem setup, green = create/ok, red = remove/error,
// amber = edit/warning. *L are tinted fills, ring/on are badge knock-out/glyph colors.
const THEMES = {
  light: {
    ink: '#64748B', paper: '#F5F7FA', hole: '#FFFFFF', ring: '#FFFFFF', on: '#FFFFFF',
    blue: '#2F7BD0', blueL: '#DCEAFB', blueXL: '#F0F6FD',
    teal: '#12929E', tealL: '#D4F0F2', tealM: '#9FDCE2', tealMid: '#4FB3BC',
    green: '#2E9E5B', greenL: '#DAF2E3',
    red: '#D9534F', redL: '#FBE3E2',
    amber: '#E09A10', amberL: '#FFF0C7',
    violet: '#7B61C9', violetL: '#EAE4F8', violetM: '#B7A8E3',
  },
  dark: {
    ink: '#A3AFBD', paper: '#2F353D', hole: '#24292F', ring: '#262A2F', on: '#1B1E22',
    blue: '#5DA4F0', blueL: '#1F3654', blueXL: '#222C39',
    teal: '#38C2CD', tealL: '#153C41', tealM: '#1F6A73', tealMid: '#2C97A2',
    green: '#4FC482', greenL: '#193A28',
    red: '#F07470', redL: '#4A2424',
    amber: '#F2B23C', amberL: '#453413',
    violet: '#A68FF2', violetL: '#2D2549', violetM: '#5F4F9C',
  },
};

function build(C) {
const f = n => +n.toFixed(2);
const P = (d, s, fill = 'none', x = '') => `<path d="${d}" stroke="${s}" fill="${fill}"${x}/>`;
const F = (d, fill) => `<path d="${d}" fill="${fill}" stroke="none"/>`;
const circ = (cx, cy, r, s, fill = 'none', x = '') => `<circle cx="${cx}" cy="${cy}" r="${r}" stroke="${s}" fill="${fill}"${x}/>`;
const dot = (cx, cy, r, fill) => `<circle cx="${cx}" cy="${cy}" r="${r}" fill="${fill}" stroke="none"/>`;
const node = (cx, cy, s = C.blue) => circ(cx, cy, 1.6, s, C.paper);
const rect = (x, y, w, h, rx, s, fill = 'none', xx = '') => `<rect x="${x}" y="${y}" width="${w}" height="${h}" rx="${rx}" stroke="${s}" fill="${fill}"${xx}/>`;
const thin = ' stroke-width="1"';
const dash = ' stroke-dasharray="2 2"';
const text = (x, y, t, fill, size = 6) =>
  `<text x="${x}" y="${y}" fill="${fill}" stroke="none" font-family="Arial, Helvetica, sans-serif" font-size="${size}" font-weight="bold" text-anchor="middle">${t}</text>`;

// Arrow from (x1,y1) to (x2,y2) with an open head.
function arrow(x1, y1, x2, y2, s, h = 2.6) {
  const a = Math.atan2(y2 - y1, x2 - x1), w = 0.6;
  const l = [x2 - h * Math.cos(a - w), y2 - h * Math.sin(a - w)];
  const r = [x2 - h * Math.cos(a + w), y2 - h * Math.sin(a + w)];
  return P(`M${f(x1)} ${f(y1)}L${f(x2)} ${f(y2)}M${f(l[0])} ${f(l[1])}L${f(x2)} ${f(y2)}L${f(r[0])} ${f(r[1])}`, s);
}
function arrow2(x1, y1, x2, y2, s, h = 2.4) { return arrow(x1, y1, x2, y2, s, h) + arrow(x2, y2, x1, y1, s, h); }

function gear(cx, cy, ro, ri, rh, teeth, s, fill) {
  let d = '';
  const n = teeth * 4;
  for (let i = 0; i < n; i++) {
    const a = (i / n) * 2 * Math.PI - Math.PI / 2 + Math.PI / n / 2;
    const r = (i % 4 === 0 || i % 4 === 1) ? ro : ri;
    d += (i ? 'L' : 'M') + f(cx + r * Math.cos(a)) + ' ' + f(cy + r * Math.sin(a));
  }
  return P(d + 'Z', s, fill) + circ(cx, cy, rh, s, C.hole);
}

// ---- Badges (bottom-right, white knock-out ring) ----
const BX = 18, BY = 18;
const badgeBase = c => `<circle cx="${BX}" cy="${BY}" r="5" fill="${c}" stroke="${C.ring}" stroke-width="1.5"/>`;
const W = C.on;
const badges = {
  plus: () => badgeBase(C.green) + P('M18 15.6v4.8M15.6 18h4.8', W),
  remove: () => badgeBase(C.red) + P('M16.3 16.3l3.4 3.4M19.7 16.3l-3.4 3.4', W),
  check: () => badgeBase(C.green) + P('M15.8 18.2l1.5 1.5 2.9-3.1', W),
  edit: () => badgeBase(C.amber) + F('M15.5 20.5l.45-2.05 3.3-3.3 1.6 1.6-3.3 3.3z', W),
  down: () => badgeBase(C.blue) + P('M18 15.4v5M15.9 18.5l2.1 2.1 2.1-2.1', W),
  up: () => badgeBase(C.blue) + P('M18 20.6v-5M15.9 17.5l2.1-2.1 2.1 2.1', W),
  update: () => badgeBase(C.green) + P('M20.3 18a2.3 2.3 0 1 1-.8-1.75M19.9 15.2v1.3h-1.3', W, 'none', ' stroke-width="1.25"'),
  replace: () => badgeBase(C.amber) + P('M15.6 16.8h4.6l-1.2-1.2M20.4 19.2h-4.6l1.2 1.2', W, 'none', ' stroke-width="1.25"'),
  search: () => badgeBase(C.blue) + circ(17.4, 17.4, 1.8, W, 'none', ' stroke-width="1.25"') + P('M18.7 18.7l1.6 1.6', W),
  cloud: () => badgeBase(C.blue) + F('M16.2 20.2a1.45 1.45 0 0 1-.1-2.9 2 2 0 0 1 3.75-.6 1.75 1.75 0 0 1 .1 3.5z', W),
};

// ---- Base glyphs ----
const G = {
  doc: (s = C.ink, fill = C.paper) => P('M5.75 2.75h8.5l4 4v14.5h-12.5z', s, fill) + P('M14.25 2.75v4h4', s),
  docLines: () => P('M8.5 11h7M8.5 14h7M8.5 17h4.5', C.ink, 'none', thin),
  folder: () => P('M2.75 6.25a1.5 1.5 0 0 1 1.5-1.5h4.5l2 2h9a1.5 1.5 0 0 1 1.5 1.5v9.5a1.5 1.5 0 0 1-1.5 1.5h-15.5a1.5 1.5 0 0 1-1.5-1.5z', C.amber, C.amberL)
    + P('M2.75 9.75h18.5', C.amber),
  floppy: () => P('M3.75 5.25a1.5 1.5 0 0 1 1.5-1.5h11.25l3.75 3.75v11.25a1.5 1.5 0 0 1-1.5 1.5h-13.5a1.5 1.5 0 0 1-1.5-1.5z', C.blue, C.blueL)
    + P('M7.75 3.75v4h7.5v-4', C.blue, C.hole) + P('M7.25 20.25v-6h9.5v6', C.blue, C.hole),
  model: () => P('M5 6v12c0 1.38 3.13 2.5 7 2.5s7-1.12 7-2.5V6', C.ink, C.paper)
    + P('M5 12c0 1.38 3.13 2.5 7 2.5s7-1.12 7-2.5', C.ink)
    + `<ellipse cx="12" cy="6" rx="7" ry="2.5" stroke="${C.ink}" fill="${C.blueL}"/>`,
  session: () => P('M3.5 12.25L12 16.5l8.5-4.25', C.ink) + P('M3.5 16.25L12 20.5l8.5-4.25', C.ink)
    + P('M12 3.5l8.5 4.25L12 12 3.5 7.75z', C.blue, C.blueL),
  material: () => circ(12, 12, 8.5, C.amber, C.amberL) + P('M7.2 10.2a5 5 0 0 1 3.3-3.3', C.amber)
    + P('M3.6 13.5c2.6 1.6 6 2.3 9.6 1.7 2.8-.5 5.2-1.6 7.2-3', C.amber, 'none', thin),
  cube: (s = C.blue) => P('M12 3.25l8 4v9.5l-8 4-8-4v-9.5z', s, C.blueXL)
    + P('M12 3.25l8 4-8 4-8-4z', s, C.blueL) + P('M12 11.25v9.5', s),
  surface: () => P('M2.75 17.75L7 6.25h14.25L17 17.75z', C.blue, C.blueL)
    + P('M7 6.25l3.25 11.5 3.5-11.5 3.25 11.5', C.blue, 'none', thin),
  triangle: () => P('M4 18.5L11 4.5l7 11z', C.blue, C.blueL) + node(4, 18.5) + node(11, 4.5) + node(18, 15.5),
  tray: () => P('M3.75 14.75v3.5a2 2 0 0 0 2 2h12.5a2 2 0 0 0 2-2v-3.5', C.ink),
  cloud: () => P('M7 19a4.5 4.5 0 0 1-.55-8.97 6 6 0 0 1 11.55 1.02A4 4 0 0 1 17.5 19z', C.blue, C.blueL),
  circle: (cx, cy, r, s, fill) => circ(cx, cy, r, s, fill),
  power: s => P('M12 3.5v8', s) + P('M7.2 6.3a7.5 7.5 0 1 0 9.6 0', s),
  trash: s => P('M4.5 6.75h15M9.75 6.75v-2a1 1 0 0 1 1-1h2.5a1 1 0 0 1 1 1v2', s)
    + P('M6.25 6.75l.9 12.6a1.5 1.5 0 0 0 1.5 1.4h6.7a1.5 1.5 0 0 0 1.5-1.4l.9-12.6', s, C.paper) + P('M10 10.5v6.5M14 10.5v6.5', s, 'none', thin),
  axes: () => P('M4 3.5v16.75h16.5', C.ink),
  iso: () => `<ellipse cx="11" cy="11" rx="8.5" ry="6.75" stroke="${C.teal}" fill="${C.tealL}"/>`
    + `<ellipse cx="11.8" cy="11.6" rx="5.4" ry="4.2" stroke="${C.teal}" fill="${C.tealM}"/>`
    + `<ellipse cx="12.6" cy="12.2" rx="2.3" ry="1.8" stroke="${C.teal}" fill="${C.teal}"/>`,
  cut: () => circ(12, 11, 7.5, C.ink, C.paper) + P('M4.5 11a7.5 7.5 0 0 0 15 0', C.ink, C.blueXL)
    + P('M2.5 13.5l3.5-4.5h15.5l-3.5 4.5z', C.teal, C.tealL, ' fill-opacity="0.9"'),
  scalar: () => {
    const r = [[1, 1.4, 1.8, 2.2], [1.4, 1.8, 2.2, 1.8], [1.8, 2.2, 1.8, 1.4], [2.2, 1.8, 1.4, 1]];
    let s = '';
    for (let i = 0; i < 4; i++) for (let j = 0; j < 4; j++)
      s += dot(4.5 + j * 5, 4.5 + i * 5, r[i][j], r[i][j] > 1.9 ? C.teal : r[i][j] > 1.5 ? C.tealMid : C.tealM);
    return s;
  },
  vector: () => arrow(3.5, 7, 9, 4, C.teal) + arrow(12.5, 6.5, 19.5, 3.5, C.teal)
    + arrow(3.5, 13, 10, 11.5, C.teal) + arrow(12.5, 12.5, 20.5, 10.5, C.teal)
    + arrow(3.5, 19, 10.5, 18.5, C.teal) + arrow(12.5, 18.5, 20.5, 17.5, C.teal),
  stream: () => P('M2.75 7c3.5-3 6 2.5 9.5 0s5.5-2.5 9-1', C.teal)
    + P('M2.75 12.5c3.5-3 6 2.5 9.5 0s5.5-2.5 9-1', C.teal)
    + P('M2.75 18c3.5-3 6 2.5 9.5 0s5.5-2.5 9-1', C.teal)
    + P('M18.9 3.9l2.35 2.1-2.6 1.45', C.teal),
  crossTris: () => P('M2.75 5.5h12.5L6.5 19z', C.blue, C.blueL, ' fill-opacity="0.85"')
    + P('M21.25 8.5L7.5 11.5l9 8.5z', C.teal, C.tealL, ' fill-opacity="0.85"'),
  mesh: () => {
    const c = [11, 11], r = 8, v = [];
    for (let i = 0; i < 6; i++) { const a = i * Math.PI / 3; v.push([f(c[0] + r * Math.cos(a)), f(c[1] + r * Math.sin(a))]); }
    return { v, outline: P('M' + v.map(p => p.join(' ')).join('L') + 'Z', C.blue, C.blueL) };
  },
  triad: (hi) => {
    const O = [12, 13.5], ends = { X: [4.5, 18.75], Y: [19.5, 18.75], Z: [12, 3.5] }, col = { X: C.red, Y: C.green, Z: C.blue };
    let s = '';
    for (const k of ['X', 'Y', 'Z']) {
      const on = !hi || hi === k;
      s += on ? arrow(O[0], O[1], ends[k][0], ends[k][1], col[k]) : P(`M${O[0]} ${O[1]}L${ends[k][0]} ${ends[k][1]}`, C.ink, 'none', ' stroke-opacity="0.45"');
    }
    s += dot(O[0], O[1], 1.75, C.ink);
    if (hi) {
      const lp = { X: [4.2, 12.5], Y: [19.8, 12.5], Z: [18, 7.5] }[hi];
      s += text(lp[0], lp[1], hi, col[hi], 7);
    }
    return s;
  },
};

// ---- Icons ----
const I = {};
const withBadge = (base, b) => base + badges[b]();

// Application
I.application_settings = gear(12, 12, 9, 6.75, 2.75, 8, C.ink, C.paper);
I.display_preferences = rect(2.75, 3.75, 18.5, 12.5, 1.5, C.ink, C.blueXL) + P('M9 20.25h6M12 16.25v4', C.ink)
  + P('M6 8h12M6 12h12', C.ink, 'none', thin) + circ(9, 8, 1.5, C.blue, C.hole) + circ(15, 12, 1.5, C.blue, C.hole);
I.quit = G.power(C.red);
I.startup = G.power(C.green);

// Action
I.cancel = circ(12, 12, 8.75, C.red, C.redL) + P('M9 9l6 6M15 9l-6 6', C.red);
I.ok = circ(12, 12, 8.75, C.green, C.greenL) + P('M8 12.4l2.7 2.7 5.3-5.6', C.green);
I.close = P('M6 6l12 12M18 6L6 18', C.ink);
I.clear = G.trash(C.ink);
I.undo = P('M8.5 13.5L4 9l4.5-4.5', C.blue) + P('M4 9h10a5.5 5.5 0 0 1 0 11h-3', C.blue);
I.redo = P('M15.5 13.5L20 9l-4.5-4.5', C.blue) + P('M20 9H10a5.5 5.5 0 0 0 0 11h3', C.blue);
I.remove = circ(12, 12, 8.75, C.red, C.redL) + P('M8 12h8', C.red);
I.add = circ(12, 12, 8.75, C.green, C.greenL) + P('M12 8v8M8 12h8', C.green);
I.refresh = P('M19.5 12a7.5 7.5 0 0 1-13.1 5', C.blue) + P('M4.5 12a7.5 7.5 0 0 1 13.1-5', C.blue)
  + P('M18.25 3.5v3.75H14.5', C.blue) + P('M5.75 20.5v-3.75H9.5', C.blue);

// Messages
I.information = circ(12, 12, 8.75, C.blue, C.blueL) + P('M12 11v5.5', C.blue) + dot(12, 7.9, 1.1, C.blue);
I.important = P('M12 3.5L21 19.5H3z', C.amber, C.amberL) + P('M12 9.5v5', C.amber) + dot(12, 17, 1.05, C.amber);
I.severe = circ(12, 12, 8.75, C.red, C.redL) + P('M12 7.5v5.5', C.red) + dot(12, 16.2, 1.1, C.red);
I.help = circ(12, 12, 8.75, C.blue, C.blueL) + P('M9.6 9.6a2.5 2.5 0 1 1 3.4 2.3c-.6.3-1 .8-1 1.5v.6', C.blue) + dot(12, 16.8, 1.05, C.blue);

// Generic file
I.new = withBadge(G.doc() + G.docLines(), 'plus');
I.open = G.folder();
I.save = G.floppy();
I['file-document'] = G.doc() + G.docLines();
I.screenshot = P('M3 8.5a1.5 1.5 0 0 1 1.5-1.5h3l1.5-2.25h6L16.5 7h3a1.5 1.5 0 0 1 1.5 1.5v9.5a1.5 1.5 0 0 1-1.5 1.5h-15A1.5 1.5 0 0 1 3 18z', C.ink, C.paper)
  + circ(12, 13, 3.75, C.blue, C.blueL) + dot(17.75, 9.75, 0.8, C.ink);

// Session
I.session = G.session();
I.session_new = withBadge(G.session(), 'plus');
I.session_open = withBadge(G.session(), 'up');
I.session_save = withBadge(G.session(), 'down');
I.session_save_as = withBadge(G.session(), 'edit');
I.session_close = withBadge(G.session(), 'remove');

// Model
I.model = G.model();
I.model_new = withBadge(G.model(), 'plus');
I.model_open = withBadge(G.model(), 'up');
I.model_save = withBadge(G.model(), 'down');
I.model_save_as = withBadge(G.model(), 'edit');
I.model_close = withBadge(G.model(), 'remove');
I.model_drop_results = withBadge(G.iso(), 'remove');
I.model_rename = rect(2.75, 7, 18.5, 10, 1.5, C.ink, C.paper) + P('M6.5 12h4', C.ink, 'none', thin)
  + P('M14 9.5v5M12.75 9.5h2.5M12.75 14.5h2.5', C.blue);
const exportIcon = t => G.tray() + arrow(12, 9.5, 12, 16.5, C.blue) + rect(3.5, 1.75, 17, 6.5, 1.25, C.blue, C.blue) + text(12, 7, t, C.on, 5.6);
I.model_export_msh = exportIcon('MSH');
I.model_export_raw = exportIcon('RAW');
I.model_export_stl = exportIcon('STL');

// Material
I.material_new = withBadge(G.material(), 'plus');
I.material_import = withBadge(G.material(), 'down');
I.material_delete = withBadge(G.material(), 'remove');

// View reset
I.resetO = G.triad(null);
I.resetX = G.triad('X');
I.resetY = G.triad('Y');
I.resetZ = G.triad('Z');

// Media
I.play_play = P('M7 4.75v14.5L19 12z', C.blue, C.blueL);
I.play_pause = rect(6, 5, 4, 14, 1, C.blue, C.blueL) + rect(14, 5, 4, 14, 1, C.blue, C.blueL);
I.play_first = P('M5.5 5v14', C.blue) + P('M18.5 5v14L8.5 12z', C.blue, C.blueL);
I.play_last = P('M18.5 5v14', C.blue) + P('M5.5 5v14l10-7z', C.blue, C.blueL);
I.play_forward = P('M3.5 6v12l8-6zM12.5 6v12l8-6z', C.blue, C.blueL);
I.play_backward = P('M20.5 6v12l-8-6zM11.5 6v12l-8-6z', C.blue, C.blueL);
I.play_record = circ(12, 12, 8, C.red, C.redL) + dot(12, 12, 4, C.red);

// Problem / solver
I.problem_task_flow = rect(3, 3.5, 8, 5, 1.25, C.violet, C.violetL) + rect(13, 9.5, 8, 5, 1.25, C.violet, C.violetL)
  + rect(3, 15.5, 8, 5, 1.25, C.violet, C.violetL)
  + P('M11 6h4a2 2 0 0 1 2 2v1.5', C.ink) + P('M15.3 7.9L17 9.5l1.7-1.6', C.ink)
  + P('M17 14.5V16a2 2 0 0 1-2 2h-3.75', C.ink) + P('M12.9 16.3L11.25 18l1.65 1.7', C.ink);
I.problem_matrix_solver = P('M7 3.75H4.25v16.5H7M17 3.75h2.75v16.5H17', C.ink)
  + [7.5, 12, 16.5].map((y, i) => [8, 12, 16].map((x, j) => dot(x, y, i === j ? 1.7 : 1.1, i === j ? C.violet : C.violetM)).join('')).join('');
I.problem_monitoring_points = circ(12, 12, 7, C.violet, C.violetL) + dot(12, 12, 2.25, C.violet)
  + P('M12 2.75v3.5M12 17.75v3.5M2.75 12h3.5M17.75 12h3.5', C.violet);
I.problem_reset = P('M4.5 12a7.5 7.5 0 1 0 2.2-5.3', C.violet) + P('M6.2 3.2v3.8H10', C.violet) + dot(12, 12, 2, C.violet);
I['solver-start'] = circ(12, 12, 8.75, C.green, C.greenL) + P('M10 8.25v7.5L16 12z', C.green, C.green);
I['solver-stop'] = circ(12, 12, 8.75, C.amber, C.amberL) + rect(8.75, 8.75, 6.5, 6.5, 1, C.amber, C.amber);
I['solver-kill'] = P('M8.4 3.3h7.2l5.1 5.1v7.2l-5.1 5.1H8.4l-5.1-5.1V8.4z', C.red, C.redL) + P('M9.2 9.2l5.6 5.6M14.8 9.2l-5.6 5.6', C.red);

// Reports
I.report = G.doc() + rect(8.25, 13.5, 2, 4.5, 0.4, C.teal, C.tealL, thin) + rect(11.25, 10, 2, 8, 0.4, C.blue, C.blueL, thin) + rect(14.25, 12, 2, 6, 0.4, C.violet, C.violetL, thin);
I['report-convergence'] = G.axes() + P('M7 5.5c1.5 6 3.5 9.5 6.5 11s5 1.2 7 1', C.blue) + P('M4.75 15h16', C.red, 'none', ' stroke-width="1" stroke-dasharray="2 1.75"');
I.report_model_statistics = G.axes() + rect(7, 12, 3, 6, 0.6, C.blue, C.blueL) + rect(12, 6.5, 3, 11.5, 0.6, C.blue, C.blueL) + rect(17, 10, 3, 8, 0.6, C.blue, C.blueL);
I.report_monitoring_point = G.axes() + P('M6.5 15c2-5 4-7 6-3s4 3 7.5-4.5', C.blue) + circ(12.4, 11.3, 2, C.violet, C.violetL);
I.report_solver_log = rect(2.75, 3.75, 18.5, 16.5, 1.5, C.ink, C.paper) + P('M2.75 7.75h18.5', C.ink)
  + P('M6.5 11l2.5 2.25L6.5 15.5', C.blue) + P('M11 16h5', C.blue);

// Cloud
I.access_rights = rect(4.75, 10.25, 14.5, 10.5, 1.5, C.amber, C.amberL) + P('M8 10.25V7.5a4 4 0 0 1 8 0v2.75', C.amber)
  + P('M12 14.5v2.5', C.amber) + dot(12, 14.3, 1.3, C.amber);
I.download = G.tray() + arrow(12, 3.5, 12, 14.5, C.blue);
I.upload = G.tray() + arrow(12, 14.5, 12, 3.5, C.blue);
I.upload_replace = withBadge(G.tray() + arrow(10.5, 14.5, 10.5, 3.5, C.blue), 'replace');
I.upload_update = withBadge(G.tray() + arrow(10.5, 14.5, 10.5, 3.5, C.blue), 'update');
I.file_manager = withBadge(G.folder(), 'cloud');
I.session_manager = withBadge(G.session(), 'cloud');
I.generic_action = P('M13.25 2.75L5 13.5h6.25l-1.5 7.75L19 10.25h-6.25z', C.amber, C.amberL);

// Geometry - draw
const pencilBadge = b => withBadge(b, 'edit');
I.draw_geometry = P('M14.75 4.25l5 5L9 20H4v-5z', C.amber, C.amberL) + P('M12.25 6.75l5 5', C.amber) + P('M4 15l5 5', C.amber, 'none', thin);
I.draw_point = pencilBadge(P('M11 3v4M11 15v4M3 11h4M15 11h4', C.ink) + dot(11, 11, 3, C.blue));
I.draw_line = pencilBadge(P('M4.5 17.5l13-13', C.blue) + node(4.5, 17.5) + node(17.5, 4.5));
I.draw_triangle = pencilBadge(P('M11 3.5l8.25 14H2.75z', C.blue, C.blueL));
I.draw_quadrilateral = pencilBadge(P('M6 4.5h14l-3 13H3z', C.blue, C.blueL));
I.draw_circle = pencilBadge(circ(11, 11, 8, C.blue, C.blueL));
I.draw_ellipse = pencilBadge(`<ellipse cx="11.5" cy="11" rx="9" ry="6" stroke="${C.blue}" fill="${C.blueL}"/>`);
I.draw_tetrahedron = pencilBadge(P('M11 3L3 16l8.5 4z', C.blue, C.blueL) + P('M11 3l8.5 11-8 6', C.blue, C.blueXL) + P('M3 16l16.5-2', C.blue, 'none', ' stroke-width="1" stroke-dasharray="1.5 1.5"'));
I.draw_hexahedron = pencilBadge(G.cube());
I.draw_cylinder = pencilBadge(P('M5 6v12c0 1.38 3.13 2.5 7 2.5s7-1.12 7-2.5V6', C.blue, C.blueXL) + `<ellipse cx="12" cy="6" rx="7" ry="2.5" stroke="${C.blue}" fill="${C.blueL}"/>`);
I.draw_sphere = pencilBadge(circ(12, 12, 8.75, C.blue, C.blueL) + P('M3.25 12c0 1.9 3.9 3.5 8.75 3.5s8.75-1.6 8.75-3.5', C.blue, 'none', thin));
I.draw_raw = pencilBadge(G.doc() + rect(2.75, 9.5, 14.5, 6.5, 1.25, C.blue, C.blue) + text(10, 14.8, 'RAW', C.on, 5.6));

// Geometry - entities
I.entity_point = P('M12 3.5v4.5M12 16v4.5M3.5 12H8M16 12h4.5', C.ink) + dot(12, 12, 3.25, C.blue);
I.entity_line = P('M3.5 18l5-9 5.5 5.5L20.5 5', C.blue) + node(3.5, 18) + node(8.5, 9) + node(14, 14.5) + node(20.5, 5);
I.entity_surface = G.surface();
I.entity_volume = G.cube();
I.entity_line_from_surface = P('M2.75 17.75L7 6.25h14.25L17 17.75z', 'none', C.blueL)
  + P('M7 6.25h14.25L17 17.75', C.blue, 'none', ' stroke-width="1" stroke-opacity="0.5"')
  + P('M7 6.25L2.75 17.75H17', C.blue, 'none', ' stroke-width="2"') + node(7, 6.25) + node(2.75, 17.75) + node(17, 17.75);
I.entity_merge = P('M4 5l6 7M4 19l6-7h6', C.blue) + P('M4 5l6 7M4 19l6-7', C.blue) + arrow(10, 12, 20.5, 12, C.blue) + node(4, 5) + node(4, 19);
I.entity_remove = withBadge(G.cube(), 'remove');
I.entity_surface_mark = P('M2.75 20.25L6 13.75h15.25L18 20.25z', C.blue, C.blueL) + P('M11 17V3.5', C.ink)
  + P('M11 3.75h8l-2 2.5 2 2.5h-8z', C.amber, C.amberL);
I.entity_surface_swap_element_normal = P('M3 16.5L14 13l7 4-11 3z', C.blue, C.blueL) + arrow2(11.5, 16.5, 11.5, 3.5, C.ink);
I.entity_surface_swap_normals = P('M2.75 20.25L6 13.75h15.25L18 20.25z', C.blue, C.blueL)
  + arrow2(7.5, 17, 7.5, 3.5, C.ink) + arrow2(12, 17, 12, 3.5, C.ink) + arrow2(16.5, 17, 16.5, 3.5, C.ink);
I.entity_surface_sync_normals = P('M2.75 20.25L6 13.75h15.25L18 20.25z', C.blue, C.blueL)
  + arrow(7.5, 17, 7.5, 3.5, C.green) + arrow(12, 17, 12, 3.5, C.green) + arrow(16.5, 17, 16.5, 3.5, C.green);
I.entity_surface_close_hole = G.surface() + `<ellipse cx="12" cy="12" rx="4" ry="2.5" stroke="${C.green}" fill="${C.greenL}"/>`;
I.entity_surface_coarsen = rect(3.25, 4.25, 17.5, 15.5, 1, C.blue, C.blueL)
  + P('M3.25 8.1h8.75M3.25 12h8.75M3.25 15.9h8.75M7.6 4.25v15.5M12 4.25v15.5M3.25 8.1l4.35 3.9M7.6 12l4.4 3.9M3.25 15.9l4.35 3.85M7.6 4.25L12 8.1', C.blue, 'none', thin)
  + P('M12 4.25l8.75 15.5', C.blue, 'none', thin);
I.entity_surface_find_intersected = withBadge(G.crossTris() + P('M8.2 11.2l5.1-5.6', C.red, 'none', ' stroke-dasharray="1.75 1.75"'), 'search');
I.entity_surface_break_intersected = G.crossTris() + P('M8.2 11.2l5.1-5.6', C.red, 'none', ' stroke-width="2"') + dot(8.2, 11.2, 1.3, C.red) + dot(13.3, 5.6, 1.3, C.red);
// Boolean: circles A (9,12) r6 and B (15,12) r6, intersect at (12, 12±5.2)
const cA = circ(9, 12, 6.25, C.blue), cB = circ(15, 12, 6.25, C.blue);
const lens = 'M12 6.63A6.25 6.25 0 0 1 12 17.37A6.25 6.25 0 0 1 12 6.63z';
I.entity_surface_bool_operation = cA + cB;
I.entity_surface_bool_union = P('M12 6.63A6.25 6.25 0 1 1 12 17.37A6.25 6.25 0 1 1 12 6.63z', C.blue, C.blueL);
I.entity_surface_bool_intersection = circ(9, 12, 6.25, C.ink, 'none', thin + ' stroke-opacity="0.5"') + circ(15, 12, 6.25, C.ink, 'none', thin + ' stroke-opacity="0.5"') + P(lens, C.blue, C.blueL);
I.entity_surface_bool_difference = P('M12 6.63A6.25 6.25 0 1 0 12 17.37A6.25 6.25 0 0 1 12 6.63z', C.blue, C.blueL)
  + P('M12 6.63A6.25 6.25 0 1 1 12 17.37', C.ink, 'none', thin + ' stroke-dasharray="1.5 1.5"');

// Results (derived entities)
for (const [k, g] of [['cut', G.cut], ['iso', G.iso], ['scalar_field', G.scalar], ['vector_field', G.vector], ['stream_line', G.stream]]) {
  I['entity_' + k] = g();
  I['entity_' + k + '_create'] = withBadge(g(), 'plus');
  I['entity_' + k + '_edit'] = withBadge(g(), 'edit');
}

// Mesh operations
I.sliver_find = circ(10, 10, 7, C.ink, C.paper) + P('M15 15l5.5 5.5', C.ink, 'none', ' stroke-width="2.25"') + P('M5.5 11.25l9-3-5 4.5z', C.red, C.redL);
I.sliver_fix = withBadge(P('M3 13.5l17-6.5-9.5 9z', C.red, C.redL), 'check');
I.generate_volume_mesh = G.cube() + P('M8 5.25l4 6M16 5.25l-4 6M4 7.25l8 13.5M20 7.25l-8 13.5M4 12l8 4 8-4', C.blue, 'none', thin + ' stroke-opacity="0.7"');
I.geometry_transform = rect(3.25, 9.75, 11, 11, 1.25, C.ink, 'none', thin + dash) + rect(9.75, 3.25, 11, 11, 1.25, C.blue, C.blueL)
  + arrow(6.5, 16.5, 12.5, 10.5, C.ink);
I.element_create = withBadge(G.triangle(), 'plus');
I.element_remove = withBadge(G.triangle(), 'remove');
I.move_node = arrow(12, 8.5, 12, 3, C.ink) + arrow(12, 15.5, 12, 21, C.ink) + arrow(8.5, 12, 3, 12, C.ink) + arrow(15.5, 12, 21, 12, C.ink) + dot(12, 12, 2.75, C.blue);
I.merge_nodes = P('M5 5.5l7 5.5M5 18.5l7-5.5', C.ink) + arrow(12, 12, 16.5, 12, C.ink) + node(5, 5.5) + node(5, 18.5) + dot(19.25, 12, 2.5, C.blue);
I.merge_near_nodes = circ(7.5, 12, 5.25, C.ink, 'none', thin + dash) + dot(6, 10.25, 1.35, C.blue) + dot(9.25, 10.75, 1.35, C.blue) + dot(7, 14, 1.35, C.blue)
  + arrow(13.5, 12, 17, 12, C.ink) + dot(19.75, 12, 2.25, C.blue);
{
  const m = G.mesh(), v = m.v;
  const spokes = P(v.map(p => `M11 11L${p[0]} ${p[1]}`).join(''), C.blue, 'none', thin);
  I.node_remove = withBadge(m.outline + spokes + dot(11, 11, 2.25, C.red), 'remove');
  I.node_remove_and_close = withBadge(m.outline + P(`M${v[3].join(' ')}L${v[5].join(' ')}M${v[3].join(' ')}L${v[0].join(' ')}M${v[3].join(' ')}L${v[1].join(' ')}`, C.blue, 'none', thin), 'check');
}

return I;
}

// ---- Write ----
const wrap = body => `<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24">`
  + `<g fill="none" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round">${body}</g></svg>
`;
const skip = ['range-app.svg', 'range-fea.svg', 'range-qt.svg'];
for (const [theme, dir] of [['light', OUT], ['dark', path.join(OUT, 'dark')]]) {
  const I = build(THEMES[theme]);
  fs.mkdirSync(dir, { recursive: true });
  for (const [k, body] of Object.entries(I)) fs.writeFileSync(path.join(dir, `range-${k}.svg`), wrap(body));
  console.log(theme, Object.keys(I).length, 'icons written to', dir);
}
for (const file of fs.readdirSync(OUT).filter(x => x.endsWith('.svg') && !skip.includes(x)))
  if (!(file.slice(6, -4) in build(THEMES.light))) console.error('Not generated:', file);
