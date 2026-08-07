#pragma once

#include <Arduino.h>

// ============================================================
// STRONA WWW
//
// Ustawiasz:
//   START
//   CZAS TRWANIA
//
// KONIEC jest liczony automatycznie:
//   KONIEC = START + CZAS TRWANIA
//
// Liczba efektow wynika automatycznie z liczby wierszy.
// ============================================================

const char STRONA_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="pl">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">

  <meta http-equiv="Cache-Control" content="no-store,no-cache,must-revalidate,max-age=0">
  <meta http-equiv="Pragma" content="no-cache">
  <meta http-equiv="Expires" content="0">

  <title>BALON</title>

  <style>
    :root {
      color-scheme:dark;
      --bg:#0f1317;
      --panel:#181e24;
      --panel2:#20272f;
      --input:#0c1116;
      --text:#f4f7f9;
      --muted:#9da9b5;
      --border:#38434f;
      --accent:#56aaff;
      --ok:#43cb79;
      --warn:#ffb52b;
      --bad:#ff6767;
    }

    * { box-sizing:border-box; }

    body {
      margin:0;
      padding:14px;
      background:var(--bg);
      color:var(--text);
      font-family:system-ui,-apple-system,Segoe UI,Arial,sans-serif;
    }

    main {
      width:min(980px,100%);
      margin:0 auto;
    }

    h1,h2 { margin-top:0; }

    .top {
      display:flex;
      justify-content:space-between;
      align-items:center;
      gap:10px;
      flex-wrap:wrap;
    }

    #connection {
      color:var(--muted);
      font-size:13px;
    }

    .status-grid {
      display:grid;
      grid-template-columns:repeat(auto-fit,minmax(150px,1fr));
      gap:9px;
      margin-bottom:14px;
    }

    .card,section {
      background:var(--panel);
      border:1px solid var(--border);
      border-radius:14px;
      padding:14px;
      margin-bottom:14px;
    }

    .label {
      color:var(--muted);
      font-size:12px;
      margin-bottom:4px;
    }

    .value {
      font-size:18px;
      font-weight:750;
    }

    .ok { color:var(--ok); }
    .warn { color:var(--warn); }
    .bad { color:var(--bad); }

    .timer {
      font-size:34px;
      font-weight:800;
      font-variant-numeric:tabular-nums;
      margin:4px 0 8px;
    }

    .bar {
      height:12px;
      background:var(--input);
      border:1px solid var(--border);
      border-radius:999px;
      overflow:hidden;
    }

    .bar>div {
      height:100%;
      width:100%;
      background:var(--accent);
      transition:width .25s linear;
    }

    .small {
      color:var(--muted);
      font-size:13px;
      margin-top:7px;
    }

    .route-top {
      display:flex;
      justify-content:space-between;
      align-items:center;
      gap:12px;
      flex-wrap:wrap;
      margin-bottom:10px;
    }

    .route-stats {
      display:flex;
      gap:16px;
      flex-wrap:wrap;
      color:var(--muted);
      font-size:14px;
    }

    .route-stats strong {
      color:var(--text);
    }

    .toolbar {
      display:flex;
      gap:7px;
      flex-wrap:wrap;
      margin:10px 0;
    }

    .toolbar button {
      flex:1 1 145px;
    }

    button {
      min-height:41px;
      padding:9px 12px;
      border:1px solid var(--border);
      border-radius:9px;
      background:var(--panel2);
      color:var(--text);
      font:inherit;
      cursor:pointer;
    }

    button.primary {
      width:100%;
      background:var(--accent);
      color:#07111b;
      border-color:transparent;
      font-weight:800;
    }

    button.danger {
      color:var(--bad);
    }

    button:disabled {
      opacity:.45;
      cursor:default;
    }

    .effects {
      display:grid;
      gap:8px;
    }

    .effect-row {
      display:grid;
      grid-template-columns:42px minmax(110px,1fr) minmax(110px,1fr) minmax(110px,1fr) auto;
      gap:8px;
      align-items:end;
      padding:10px;
      background:var(--panel2);
      border:1px solid var(--border);
      border-radius:12px;
    }

    .effect-no {
      align-self:center;
      text-align:center;
      font-weight:800;
      font-size:17px;
    }

    .field label {
      display:block;
      margin-bottom:4px;
      color:var(--muted);
      font-size:12px;
    }

    .time {
      width:100%;
      min-height:43px;
      padding:8px 9px;
      border:1px solid var(--border);
      border-radius:9px;
      background:var(--input);
      color:var(--text);
      text-align:center;
      font-size:17px;
      font-variant-numeric:tabular-nums;
    }

    .computed {
      min-height:43px;
      display:flex;
      justify-content:center;
      align-items:center;
      border:1px solid var(--border);
      border-radius:9px;
      background:var(--input);
      font-size:17px;
      font-weight:750;
      font-variant-numeric:tabular-nums;
    }

    .row-actions {
      display:flex;
      gap:5px;
    }

    .row-actions button {
      min-width:42px;
      padding:7px;
    }

    .empty {
      padding:18px;
      text-align:center;
      color:var(--muted);
      border:1px dashed var(--border);
      border-radius:11px;
    }

    .timeline {
      position:relative;
      height:34px;
      margin-top:12px;
      overflow:hidden;
      border:1px solid var(--border);
      border-radius:9px;
      background:var(--input);
    }

    .segment {
      position:absolute;
      top:6px;
      bottom:6px;
      min-width:3px;
      border-radius:5px;
      background:var(--accent);
    }

    .timeline-labels {
      display:flex;
      justify-content:space-between;
      margin-top:3px;
      color:var(--muted);
      font-size:11px;
    }

    .save-box {
      position:sticky;
      bottom:8px;
      z-index:5;
      padding:10px;
      margin-bottom:14px;
      border:1px solid var(--border);
      border-radius:14px;
      background:var(--panel);
    }

    #saveMsg {
      min-height:20px;
      margin-top:7px;
      text-align:center;
      color:var(--muted);
      font-size:14px;
    }

    pre {
      margin:0;
      min-height:220px;
      max-height:420px;
      overflow:auto;
      white-space:pre-wrap;
      word-break:break-word;
      padding:12px;
      border:1px solid var(--border);
      border-radius:10px;
      background:#080b0e;
      color:#d9e2ea;
      font:13px/1.45 ui-monospace,SFMono-Regular,Consolas,monospace;
    }

    @media(max-width:700px) {
      .effect-row {
        grid-template-columns:34px 1fr 1fr;
      }

      .effect-no {
        grid-row:span 2;
      }

      .end-field {
        grid-column:2/3;
      }

      .row-actions {
        grid-column:3/4;
        justify-content:flex-end;
      }
    }
  </style>
</head>

<body>
<main>
  <div class="top">
    <h1>Sterownik balonu</h1>
    <div id="connection">Łączenie...</div>
  </div>

  <div class="status-grid">
    <div class="card">
      <div class="label">Stan</div>
      <div class="value" id="state">---</div>
    </div>

    <div class="card">
      <div class="label">GPIO0 / ładowarka</div>
      <div class="value" id="gpio">---</div>
    </div>

    <div class="card">
      <div class="label">Następny kierunek</div>
      <div class="value" id="direction">---</div>
    </div>

    <div class="card">
      <div class="label">Płomień</div>
      <div class="value" id="effect">---</div>
    </div>

    <div class="card">
      <div class="label">Ukończone trasy</div>
      <div class="value" id="routes">---</div>
    </div>
  </div>

  <section>
    <h2>Uśpienie</h2>
    <div class="timer" id="sleepTimer">05:00</div>
    <div class="bar"><div id="sleepBar"></div></div>
    <div class="small" id="sleepInfo">LOW na GPIO0 zeruje licznik.</div>
  </section>

  <section>
    <div class="route-top">
      <h2>A → B</h2>

      <div class="route-stats">
        <span>Efektów: <strong id="ab_count">0</strong>/15</span>
        <span>Koniec programu: <strong id="ab_total">0:00</strong></span>
      </div>
    </div>

    <div class="small">
      Wpisujesz tylko <strong>START</strong> i <strong>CZAS TRWANIA</strong>.
      <strong>KONIEC</strong> oblicza się sam.
    </div>

    <div class="toolbar">
      <button type="button" id="ab_add">＋ Dodaj efekt</button>
      <button type="button" id="ab_sort">↕ Sortuj</button>
      <button type="button" id="ab_clear" class="danger">Wyczyść</button>
    </div>

    <div class="effects" id="ab_effects"></div>

    <div class="timeline" id="ab_timeline"></div>
    <div class="timeline-labels">
      <span>0:00</span>
      <span id="ab_timeline_end">0:00</span>
    </div>
  </section>

  <section>
    <div class="route-top">
      <h2>B → A</h2>

      <div class="route-stats">
        <span>Efektów: <strong id="ba_count">0</strong>/15</span>
        <span>Koniec programu: <strong id="ba_total">0:00</strong></span>
      </div>
    </div>

    <div class="small">
      Wpisujesz tylko <strong>START</strong> i <strong>CZAS TRWANIA</strong>.
      <strong>KONIEC</strong> oblicza się sam.
    </div>

    <div class="toolbar">
      <button type="button" id="ba_add">＋ Dodaj efekt</button>
      <button type="button" id="ba_sort">↕ Sortuj</button>
      <button type="button" id="ba_clear" class="danger">Wyczyść</button>
    </div>

    <div class="effects" id="ba_effects"></div>

    <div class="timeline" id="ba_timeline"></div>
    <div class="timeline-labels">
      <span>0:00</span>
      <span id="ba_timeline_end">0:00</span>
    </div>
  </section>

  <section>
    <h2>Szybkie kopiowanie</h2>

    <div class="toolbar">
      <button type="button" id="copy_ab_ba">Kopiuj A → B do B → A</button>
      <button type="button" id="copy_ba_ab">Kopiuj B → A do A → B</button>
    </div>
  </section>

  <div class="save-box">
    <button type="button" id="save" class="primary">ZAPISZ USTAWIENIA</button>
    <div id="saveMsg"></div>
  </div>

  <section>
    <h2>Log sterownika</h2>
    <pre id="log">Oczekiwanie na dane...</pre>
  </section>
</main>

<script>
  const MAX = 15;

  // UI przechowuje start + czas trwania.
  const routes = {
    ab: [],
    ba: []
  };

  const $ = id => document.getElementById(id);

  function secToTime(value) {
    let sec = Math.max(0, Math.round(Number(value) || 0));
    const min = Math.floor(sec / 60);
    sec %= 60;

    return `${min}:${String(sec).padStart(2,"0")}`;
  }

  function timeToSec(value) {
    const text = String(value ?? "").trim();

    // "15" = 15 sekund
    if (/^\d+$/.test(text)) {
      return Number(text);
    }

    // "1:20" = 80 sekund
    const m = text.match(/^(\d+):([0-5]?\d)$/);

    if (!m) {
      return NaN;
    }

    return Number(m[1]) * 60 + Number(m[2]);
  }

  function koniec(e) {
    return e.start + e.duration;
  }

  function koniecProgramu(prefix) {
    return routes[prefix].reduce(
      (max,e) => Math.max(max,koniec(e)),
      0
    );
  }

  function dodajEfekt(prefix) {
    if (routes[prefix].length >= MAX) {
      return;
    }

    const list = routes[prefix];
    const prev = list[list.length - 1];

    // Automatyczna propozycja:
    // 5 sekund po poprzednim efekcie,
    // domyslnie 5 sekund trwania.
    const start = prev ? koniec(prev) + 5 : 0;

    list.push({
      start:start,
      duration:5
    });

    render(prefix);
  }

  function usunEfekt(prefix,index) {
    routes[prefix].splice(index,1);
    render(prefix);
  }

  function duplikujEfekt(prefix,index) {
    if (routes[prefix].length >= MAX) {
      return;
    }

    const old = routes[prefix][index];

    routes[prefix].splice(index + 1,0,{
      start:koniec(old) + 5,
      duration:old.duration
    });

    render(prefix);
  }

  function sortuj(prefix) {
    routes[prefix].sort((a,b) => a.start - b.start);
    render(prefix);
  }

  function wyczysc(prefix) {
    routes[prefix] = [];
    render(prefix);
  }

  function kopiuj(from,to) {
    routes[to] = routes[from].map(e => ({
      start:e.start,
      duration:e.duration
    }));

    render(to);
  }

  function poleCzasu(prefix,index,key,labelText,value) {
    const box = document.createElement("div");
    box.className = "field";

    const label = document.createElement("label");
    label.textContent = labelText;

    const input = document.createElement("input");
    input.className = "time";
    input.type = "text";
    input.inputMode = "numeric";
    input.placeholder = "0:00";
    input.value = secToTime(value);

    input.addEventListener("change",() => {
      const sec = timeToSec(input.value);

      if (
        !Number.isFinite(sec) ||
        sec < 0 ||
        sec > 3600 ||
        (key === "duration" && sec <= 0)
      ) {
        input.classList.add("bad");
        return;
      }

      input.classList.remove("bad");
      routes[prefix][index][key] = sec;
      input.value = secToTime(sec);

      aktualizujObliczenia(prefix);
    });

    box.append(label,input);
    return box;
  }

  function render(prefix) {
    const box = $(prefix + "_effects");
    box.innerHTML = "";

    if (routes[prefix].length === 0) {
      const empty = document.createElement("div");
      empty.className = "empty";
      empty.textContent = "Brak efektów. Naciśnij „Dodaj efekt”.";
      box.appendChild(empty);

      aktualizujObliczenia(prefix);
      return;
    }

    routes[prefix].forEach((e,index) => {
      const row = document.createElement("div");
      row.className = "effect-row";

      const no = document.createElement("div");
      no.className = "effect-no";
      no.textContent = index + 1;

      const start = poleCzasu(
        prefix,index,"start","Start",e.start
      );

      const duration = poleCzasu(
        prefix,index,"duration","Czas trwania",e.duration
      );

      const endBox = document.createElement("div");
      endBox.className = "field end-field";

      const endLabel = document.createElement("label");
      endLabel.textContent = "Koniec — AUTO";

      const end = document.createElement("div");
      end.className = "computed";
      end.dataset.end = `${prefix}-${index}`;
      end.textContent = secToTime(koniec(e));

      endBox.append(endLabel,end);

      const actions = document.createElement("div");
      actions.className = "row-actions";

      const clone = document.createElement("button");
      clone.type = "button";
      clone.title = "Duplikuj";
      clone.textContent = "⧉";
      clone.addEventListener("click",() => duplikujEfekt(prefix,index));

      const remove = document.createElement("button");
      remove.type = "button";
      remove.title = "Usuń";
      remove.className = "danger";
      remove.textContent = "✕";
      remove.addEventListener("click",() => usunEfekt(prefix,index));

      actions.append(clone,remove);

      row.append(no,start,duration,endBox,actions);
      box.appendChild(row);
    });

    aktualizujObliczenia(prefix);
  }

  function aktualizujObliczenia(prefix) {
    const total = koniecProgramu(prefix);

    $(prefix + "_count").textContent = routes[prefix].length;
    $(prefix + "_total").textContent = secToTime(total);
    $(prefix + "_timeline_end").textContent = secToTime(total);
    $(prefix + "_add").disabled = routes[prefix].length >= MAX;

    routes[prefix].forEach((e,index) => {
      const end = document.querySelector(
        `[data-end="${prefix}-${index}"]`
      );

      if (end) {
        end.textContent = secToTime(koniec(e));
      }
    });

    renderTimeline(prefix);
  }

  function renderTimeline(prefix) {
    const box = $(prefix + "_timeline");
    box.innerHTML = "";

    const total = koniecProgramu(prefix);

    if (total <= 0) {
      return;
    }

    routes[prefix].forEach(e => {
      const segment = document.createElement("div");
      segment.className = "segment";

      segment.style.left =
        `${100 * e.start / total}%`;

      segment.style.width =
        `${100 * e.duration / total}%`;

      segment.title =
        `${secToTime(e.start)} → ${secToTime(koniec(e))}`;

      box.appendChild(segment);
    });
  }

  function waliduj(prefix) {
    for (let i = 0; i < routes[prefix].length; i++) {
      const e = routes[prefix][i];

      if (
        e.start < 0 ||
        e.duration <= 0 ||
        koniec(e) > 3600
      ) {
        return `Efekt ${i + 1} ma nieprawidłowy czas.`;
      }
    }

    return "";
  }

  function daneDoZapisu() {
    const body = new URLSearchParams();

    for (const prefix of ["ab","ba"]) {
      body.set(prefix + "_count",routes[prefix].length);

      for (let i = 0; i < MAX; i++) {
        const e = routes[prefix][i];

        if (e) {
          // Firmware nadal dostaje START i KONIEC.
          body.set(prefix + "_s" + i,e.start);
          body.set(prefix + "_e" + i,koniec(e));
        } else {
          body.set(prefix + "_s" + i,0);
          body.set(prefix + "_e" + i,0);
        }
      }
    }

    return body;
  }

  async function wczytajKonfiguracje() {
    const response = await fetch("/api/config",{
      cache:"no-store"
    });

    const data = await response.json();

    function convert(src) {
      return src.effects
        .slice(0,src.count)
        .filter(e => e[1] > e[0])
        .map(e => ({
          start:e[0],
          duration:e[1] - e[0]
        }));
    }

    routes.ab = convert(data.ab);
    routes.ba = convert(data.ba);

    render("ab");
    render("ba");
  }

  function formatSleep(ms) {
    let sec = Math.max(0,Math.ceil(ms / 1000));
    const min = Math.floor(sec / 60);
    sec %= 60;

    return `${String(min).padStart(2,"0")}:${String(sec).padStart(2,"0")}`;
  }

  async function updateStatus() {
    try {
      const response = await fetch("/api/status",{
        cache:"no-store"
      });

      const data = await response.json();

      $("connection").textContent = "Połączono";
      $("connection").className = "ok";

      $("state").textContent = data.stan;

      $("gpio").textContent = data.gpio0;
      $("gpio").className =
        "value " + (data.ladowanie ? "ok" : "warn");

      $("direction").textContent = data.kierunek;

      $("effect").textContent =
        data.efekt
          ? "MOCNY + DŹWIĘK"
          : "SPOKOJNY";

      $("effect").className =
        "value " + (data.efekt ? "warn" : "ok");

      $("routes").textContent = data.trasy;

      $("sleepTimer").textContent =
        formatSleep(data.sleepRemainingMs);

      const pct = Math.max(
        0,
        Math.min(
          100,
          100 * data.sleepRemainingMs / data.sleepTotalMs
        )
      );

      $("sleepBar").style.width = pct + "%";

      $("sleepInfo").textContent = data.ladowanie
        ? "Ładowarka wykryta — licznik uśpienia jest zerowany."
        : "Brak ładowania — przy 00:00 sterownik przejdzie w Deep Sleep.";
    } catch {
      $("connection").textContent = "Brak połączenia";
      $("connection").className = "bad";
    }
  }

  async function updateLog() {
    try {
      const response = await fetch("/api/log",{
        cache:"no-store"
      });

      const text = await response.text();
      const box = $("log");

      const atBottom =
        box.scrollHeight - box.scrollTop - box.clientHeight < 70;

      box.textContent = text;

      if (atBottom) {
        box.scrollTop = box.scrollHeight;
      }
    } catch {}
  }

  async function zapisz() {
    const msg = $("saveMsg");

    const errAB = waliduj("ab");
    const errBA = waliduj("ba");

    if (errAB || errBA) {
      msg.textContent =
        errAB ? "A → B: " + errAB : "B → A: " + errBA;
      msg.className = "bad";
      return;
    }

    // Sortowanie odbywa sie automatycznie przy zapisie.
    routes.ab.sort((a,b) => a.start - b.start);
    routes.ba.sort((a,b) => a.start - b.start);

    render("ab");
    render("ba");

    msg.textContent = "Zapisywanie...";
    msg.className = "";

    try {
      const response = await fetch("/api/save",{
        method:"POST",
        body:daneDoZapisu()
      });

      const text = await response.text();

      if (!response.ok) {
        throw new Error(text);
      }

      msg.textContent = "Ustawienia zapisane.";
      msg.className = "ok";
    } catch(error) {
      msg.textContent = "Błąd: " + error.message;
      msg.className = "bad";
    }
  }

  $("ab_add").addEventListener("click",() => dodajEfekt("ab"));
  $("ba_add").addEventListener("click",() => dodajEfekt("ba"));

  $("ab_sort").addEventListener("click",() => sortuj("ab"));
  $("ba_sort").addEventListener("click",() => sortuj("ba"));

  $("ab_clear").addEventListener("click",() => wyczysc("ab"));
  $("ba_clear").addEventListener("click",() => wyczysc("ba"));

  $("copy_ab_ba").addEventListener("click",() => kopiuj("ab","ba"));
  $("copy_ba_ab").addEventListener("click",() => kopiuj("ba","ab"));

  $("save").addEventListener("click",zapisz);

  wczytajKonfiguracje().catch(() => {
    render("ab");
    render("ba");
  });

  updateStatus();
  updateLog();

  setInterval(updateStatus,1000);
  setInterval(updateLog,1000);
</script>
</body>
</html>
)HTML";
