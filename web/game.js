// Neon Maze – browser port of the C++/SFML desktop game.
// Same mazes, movement rules, ghost AI, scoring and screens, drawn on a canvas.
(() => {
  "use strict";

  // ---------------------------------------------------------------- config
  const W = 900, H = 780;
  const COLS = 21, ROWS = 21, TILE = 30;
  const MAZE_LEFT = 135, MAZE_TOP = 105;
  const PLAYER_RADIUS = 12, GHOST_RADIUS = 12;
  const START_LIVES = 3, TOTAL_LEVELS = 3;
  const PLAYER_SPEED = 5.3, GHOST_SPEED = 4.2;
  const TITLE_FONT = '"Press Start 2P", monospace';
  const TEXT_FONT = 'Orbitron, sans-serif';

  const C = {
    bg: "rgb(6,8,24)",
    wallFill: "rgb(12,32,76)",
    wallOutline: "rgba(0,205,255,0.82)",
    dot: "rgb(255,244,200)",
    pellet: "rgb(130,240,255)",
    yellow: "rgb(255,225,45)",
    accent: "rgb(0,235,255)",
    light: "rgb(220,235,255)",
    dim: "rgb(130,160,200)",
    danger: "rgb(255,70,120)",
    gold: "rgb(255,210,60)",
  };

  const LEVELS = [
    [
      "#####################",
      "#.........#.........#",
      "#.###.###.#.###.###.#",
      "#o###.###.#.###.###o#",
      "#...................#",
      "#.###.#.#####.#.###.#",
      "#.....#...#...#.....#",
      "#####.###.#.###.#####",
      "#.......#...#.......#",
      "#.#####.#...#.#####.#",
      "#.#...#.......#...#.#",
      "#.#.#.#.#...#.#.#.#.#",
      "#...#...#GGG#...#...#",
      "###.###.#GGG#.###.###",
      "#.......#GGG#.......#",
      "#.#####.#####.#####.#",
      "#o....#.......#....o#",
      "#####.#.#####.#.#####",
      "#.....P.......#.....#",
      "#.#################.#",
      "#####################",
    ],
    [
      "#####################",
      "#.........#.........#",
      "#.#####.#.#.#.#####.#",
      "#o....#.#.#.#.#....o#",
      "#.###.#.#.#.#.#.###.#",
      "#.#...#...#...#...#.#",
      "#.#.###.#####.###.#.#",
      "#...#...........#...#",
      "###.#.###.#.###.#.###",
      "#...#.#.......#.#...#",
      "#.###.#.#...#.#.###.#",
      "#.....#.#GGG#.#.....#",
      "#.#####.#GGG#.#####.#",
      "#.......#GGG#.......#",
      "#####.#.#####.#.#####",
      "#.....#...#...#.....#",
      "#.###.###.#.###.###.#",
      "#o..#.....#.....#..o#",
      "#...#.#...P...#.#...#",
      "###.#.#########.#.###",
      "#####################",
    ],
    [
      "#####################",
      "#.....#.......#.....#",
      "#.###.#.#####.#.###.#",
      "#o....#...#...#....o#",
      "#.#####.#.#.#.#####.#",
      "#.......#.#.#.......#",
      "###.###.#.#.#.###.###",
      "#...#...#...#...#...#",
      "#.#.#.#.#####.#.#.#.#",
      "#.#...#.......#...#.#",
      "#.#.###.#...#.###.#.#",
      "#.#.....#GGG#.....#.#",
      "#.#####.#GGG#.#####.#",
      "#.......#GGG#.......#",
      "#.#####.#####.#####.#",
      "#.#...#.......#...#.#",
      "#.#.#.#.#####.#.#.#.#",
      "#o..P..#.....#....o.#",
      "###.#####.#####.#####",
      "#...................#",
      "#####################",
    ],
  ];

  // ------------------------------------------------------------ directions
  const DIR = { none: [0, 0], up: [0, -1], down: [0, 1], left: [-1, 0], right: [1, 0] };
  const OPP = { up: "down", down: "up", left: "right", right: "left", none: "none" };
  const ANGLE = { right: 0, down: Math.PI / 2, left: Math.PI, up: Math.PI * 1.5, none: 0 };
  const tileCenter = (c, r) => ({ x: MAZE_LEFT + c * TILE + TILE / 2, y: MAZE_TOP + r * TILE + TILE / 2 });
  const pixelToTile = (p) => ({ x: Math.floor((p.x - MAZE_LEFT) / TILE), y: Math.floor((p.y - MAZE_TOP) / TILE) });
  const rand = (n) => Math.floor(Math.random() * n);

  // ------------------------------------------------------------------ maze
  const maze = {
    grid: [], remaining: 0, playerStart: { x: 1, y: 1 }, ghostStarts: [],
    load(level) {
      const idx = Math.max(0, Math.min(LEVELS.length - 1, level - 1));
      this.grid = LEVELS[idx].map((row) => row.split(""));
      this.remaining = 0;
      this.ghostStarts = [];
      for (let r = 0; r < ROWS; r++)
        for (let c = 0; c < COLS; c++) {
          const t = this.grid[r][c];
          if (t === "." || t === "o") this.remaining++;
          if (t === "P") this.playerStart = { x: c, y: r };
          if (t === "G") this.ghostStarts.push({ x: c, y: r });
        }
      if (!this.ghostStarts.length) this.ghostStarts.push({ x: 10, y: 10 });
    },
    inside: (c, r) => c >= 0 && c < COLS && r >= 0 && r < ROWS,
    tile(c, r) { return this.inside(c, r) ? this.grid[r][c] : "#"; },
    walkable(c, r) { return this.tile(c, r) !== "#"; },
    collect(c, r) {
      if (!this.inside(c, r)) return null;
      const t = this.grid[r][c];
      if (t === ".") { this.grid[r][c] = " "; this.remaining--; return { points: 10, power: false }; }
      if (t === "o") { this.grid[r][c] = " "; this.remaining--; return { points: 50, power: true }; }
      return null;
    },
    walkableTiles() {
      const out = [];
      for (let r = 0; r < ROWS; r++) for (let c = 0; c < COLS; c++) if (this.walkable(c, r)) out.push({ x: c, y: r });
      return out;
    },
  };

  // ---------------------------------------------------------------- player
  class Player {
    reset(start) {
      this.cur = { ...start }; this.next = { ...start };
      this.moving = false; this.progress = 0; this.anim = 0;
      this.dir = "none"; this.want = "none"; this.facing = "right";
      this.pos = tileCenter(start.x, start.y);
    }
    tryStart() {
      for (const d of [this.want, this.dir]) {
        if (d === "none") continue;
        const n = { x: this.cur.x + DIR[d][0], y: this.cur.y + DIR[d][1] };
        if (maze.walkable(n.x, n.y)) {
          this.dir = d; this.facing = d; this.next = n; this.progress = 0; this.moving = true;
          return;
        }
      }
      this.moving = false;
    }
    update(dt) {
      this.anim += dt;
      if (this.moving && this.want !== "none" && this.want === OPP[this.dir]) {
        const old = this.cur; this.cur = this.next; this.next = old;
        this.progress = 1 - this.progress; this.dir = this.want; this.facing = this.dir;
      }
      if (!this.moving) this.tryStart();
      if (this.moving) {
        this.progress += PLAYER_SPEED * dt;
        if (this.progress >= 1) {
          this.cur = this.next; this.progress = 0; this.moving = false;
          this.tryStart();
        }
      }
      this.updatePos();
    }
    updatePos() {
      const a = tileCenter(this.cur.x, this.cur.y);
      if (!this.moving) { this.pos = a; return; }
      const b = tileCenter(this.next.x, this.next.y);
      this.pos = { x: a.x + (b.x - a.x) * this.progress, y: a.y + (b.y - a.y) * this.progress };
    }
    draw(t, power, blink) {
      let alpha = 1;
      if (blink && Math.floor(t * 12) % 2 === 0) alpha = 0.35;
      glow(this.pos.x, this.pos.y, 18, "rgba(255,220,40,0.11)");
      if (power) {
        const p = 1 + 0.25 * Math.sin(t * 8);
        glow(this.pos.x, this.pos.y, 24 * p, "rgba(80,230,255,0.22)");
        ctx.strokeStyle = "rgba(120,240,255,0.7)"; ctx.lineWidth = 1.5;
        ctx.beginPath(); ctx.arc(this.pos.x, this.pos.y, PLAYER_RADIUS + 6, 0, Math.PI * 2); ctx.stroke();
      }
      const mouth = this.moving
        ? 12 + 18 * (0.5 + 0.5 * Math.sin(this.anim * 14))
        : 10 + 5 * (0.5 + 0.5 * Math.sin(this.anim * 3));
      ctx.globalAlpha = alpha;
      drawChomper(this.pos.x, this.pos.y, PLAYER_RADIUS, C.yellow, ANGLE[this.facing], mouth);
      ctx.globalAlpha = 1;
    }
  }

  // ----------------------------------------------------------------- ghost
  class Ghost {
    constructor(color, personality) { this.color = color; this.personality = personality; }
    reset(start) {
      this.start = { ...start }; this.cur = { ...start }; this.next = { ...start };
      this.moving = false; this.active = true; this.progress = 0; this.respawn = 0;
      this.dir = "none"; this.facing = "left";
      this.pos = tileCenter(start.x, start.y);
    }
    eat() { this.active = false; this.moving = false; this.progress = 0; this.respawn = 3; }
    update(dt, playerTile, playerDir, frightened, speedFactor) {
      if (!this.active) {
        this.respawn -= dt;
        if (this.respawn <= 0) this.reset(this.start);
        return;
      }
      if (!this.moving) this.choose(playerTile, playerDir, frightened);
      if (this.moving) {
        let speed = GHOST_SPEED * speedFactor;
        if (frightened) speed *= 0.55;
        this.progress += speed * dt;
        if (this.progress >= 1) {
          this.cur = this.next; this.progress = 0; this.moving = false;
          this.choose(playerTile, playerDir, frightened);
        }
      }
      const a = tileCenter(this.cur.x, this.cur.y);
      if (!this.moving) this.pos = a;
      else {
        const b = tileCenter(this.next.x, this.next.y);
        this.pos = { x: a.x + (b.x - a.x) * this.progress, y: a.y + (b.y - a.y) * this.progress };
      }
    }
    target(playerTile, playerDir) {
      const p = this.personality;
      if (p === 0) return playerTile;
      if (p === 1) return { x: playerTile.x + DIR[playerDir][0] * 3, y: playerTile.y + DIR[playerDir][1] * 3 };
      if (p === 2) return { x: playerTile.x + rand(5) - 2, y: playerTile.y + rand(5) - 2 };
      const dist = Math.abs(playerTile.x - this.cur.x) + Math.abs(playerTile.y - this.cur.y);
      if (dist > 7) return playerTile;
      return this.start.x < COLS / 2 ? { x: 1, y: 1 } : { x: COLS - 2, y: 1 };
    }
    choose(playerTile, playerDir, frightened) {
      const inRoom = maze.tile(this.cur.x, this.cur.y) === "G";
      let options = [];
      for (const d of ["up", "left", "down", "right"]) {
        if (!inRoom && d === OPP[this.dir]) continue;
        if (maze.walkable(this.cur.x + DIR[d][0], this.cur.y + DIR[d][1])) options.push(d);
      }
      if (!options.length) {
        const back = OPP[this.dir];
        if (back !== "none" && maze.walkable(this.cur.x + DIR[back][0], this.cur.y + DIR[back][1])) options.push(back);
      }
      if (!options.length) { this.moving = false; return; }

      let chosen = options[0];
      if (frightened && !inRoom) {
        chosen = options[rand(options.length)];
      } else {
        let tgt = this.target(playerTile, playerDir);
        if (inRoom) {
          tgt = { ...this.cur };
          while (tgt.y > 0 && maze.tile(tgt.x, tgt.y) === "G") tgt.y--;
        }
        let best = Infinity;
        for (const d of options) {
          const nx = this.cur.x + DIR[d][0], ny = this.cur.y + DIR[d][1];
          let dist = Math.abs(nx - tgt.x) + Math.abs(ny - tgt.y);
          if (this.personality === 2) dist += rand(8);
          else if (this.personality === 3) dist += rand(16);
          if (dist < best) { best = dist; chosen = d; }
        }
        if (!inRoom && rand(100) < 8) chosen = options[rand(options.length)];
      }
      this.dir = chosen; this.facing = chosen;
      this.next = { x: this.cur.x + DIR[chosen][0], y: this.cur.y + DIR[chosen][1] };
      this.progress = 0; this.moving = true;
    }
    draw(t, frightened, flash) {
      if (!this.active) return;
      const pulse = 1 + 0.08 * Math.sin(t * 4 + this.cur.x);
      glow(this.pos.x, this.pos.y, 18 * pulse, frightened ? "rgba(60,100,255,0.15)" : withAlpha(this.color, 0.13));
      drawGhost(this.pos.x, this.pos.y, GHOST_RADIUS, this.color, this.facing, frightened, flash);
    }
  }

  // ----------------------------------------------------------------- sound
  const sound = {
    enabled: localStorage.getItem("neonmaze.sound") !== "off",
    ctx: null, buffers: {},
    names: ["click", "eat", "power", "ghost", "death", "level"],
    unlock() {
      if (this.ctx) { if (this.ctx.state === "suspended") this.ctx.resume(); return; }
      const AC = window.AudioContext || window.webkitAudioContext;
      if (!AC) return;
      this.ctx = new AC();
      for (const n of this.names)
        fetch(`assets/sounds/${n}.wav`).then((r) => r.arrayBuffer())
          .then((b) => this.ctx.decodeAudioData(b)).then((buf) => { this.buffers[n] = buf; }).catch(() => {});
    },
    play(name, volume = 1) {
      if (!this.enabled || !this.ctx || !this.buffers[name]) return;
      const src = this.ctx.createBufferSource();
      const gain = this.ctx.createGain();
      gain.gain.value = volume;
      src.buffer = this.buffers[name];
      src.connect(gain).connect(this.ctx.destination);
      src.start();
    },
  };
  let lastEatSound = 0;

  // --------------------------------------------------------------- drawing
  const canvas = document.getElementById("game");
  const ctx = canvas.getContext("2d");

  function setupCanvas() {
    const dpr = Math.min(window.devicePixelRatio || 1, 2);
    const rect = canvas.getBoundingClientRect();
    const scale = Math.max(1, (rect.width / W) * dpr);
    canvas.width = Math.round(W * scale);
    canvas.height = Math.round(H * scale);
    ctx.setTransform(scale, 0, 0, scale, 0, 0);
  }

  function withAlpha(rgb, a) { return rgb.replace("rgb(", "rgba(").replace(")", `,${a})`); }

  function glow(x, y, r, color) {
    ctx.fillStyle = color;
    ctx.beginPath(); ctx.arc(x, y, r, 0, Math.PI * 2); ctx.fill();
  }

  function drawChomper(x, y, r, color, rotation, mouthDeg) {
    const m = Math.max(4, Math.min(60, mouthDeg)) * Math.PI / 180;
    ctx.save(); ctx.translate(x, y); ctx.rotate(rotation);
    ctx.fillStyle = color;
    ctx.beginPath(); ctx.moveTo(0, 0); ctx.arc(0, 0, r, m, Math.PI * 2 - m); ctx.closePath(); ctx.fill();
    ctx.restore();
  }

  function drawGhost(x, y, r, color, facing, frightened, flash) {
    let body = color;
    if (frightened) body = flash ? "rgb(245,245,255)" : "rgb(45,80,220)";
    ctx.fillStyle = body;
    ctx.beginPath(); ctx.arc(x, y - r * 0.2, r, 0, Math.PI * 2); ctx.fill();
    ctx.fillRect(x - r, y - r * 0.2, r * 2, r * 0.9);
    for (let i = 0; i < 3; i++) {
      ctx.beginPath(); ctx.arc(x - r * 0.66 + i * r * 0.66, y + r * 0.78, r * 0.32, 0, Math.PI * 2); ctx.fill();
    }
    const look = DIR[facing] || DIR.none;
    const eyeY = y - r * 0.38;
    for (const side of [-1, 1]) {
      const ex = x + side * r * 0.38;
      ctx.fillStyle = "#fff";
      ctx.beginPath(); ctx.arc(ex, eyeY, r * 0.3, 0, Math.PI * 2); ctx.fill();
      ctx.fillStyle = "rgb(10,15,45)";
      ctx.beginPath(); ctx.arc(ex + look[0] * r * 0.13, eyeY + look[1] * r * 0.13, r * 0.14, 0, Math.PI * 2); ctx.fill();
    }
    if (frightened) {
      ctx.fillStyle = flash ? "rgb(220,40,80)" : "#fff";
      ctx.fillRect(x - r * 0.375, y + r * 0.3 - r * 0.065, r * 0.75, r * 0.13);
    }
  }

  function text(str, size, x, y, color, opts = {}) {
    ctx.font = `${size}px ${opts.title ? TITLE_FONT : TEXT_FONT}`;
    ctx.textAlign = opts.align || "center";
    ctx.textBaseline = opts.baseline || "top";
    if (opts.glow) {
      ctx.shadowColor = "rgba(0,190,255,0.6)"; ctx.shadowBlur = size * 0.5;
    }
    if (opts.outline) {
      ctx.lineWidth = 4; ctx.strokeStyle = "rgb(10,15,30)"; ctx.strokeText(str, x, y);
    }
    ctx.fillStyle = color; ctx.fillText(str, x, y);
    ctx.shadowBlur = 0; ctx.shadowColor = "transparent";
  }

  const pad = (n) => String(n).padStart(6, "0");

  const stars = Array.from({ length: 90 }, () => ({
    x: rand(W), y: rand(H), speed: 3 + rand(18), phase: rand(628) / 100, r: 0.8 + rand(3) * 0.45,
  }));

  function drawBackground(t) {
    ctx.fillStyle = C.bg; ctx.fillRect(0, 0, W, H);
    for (let i = 0; i < 6; i++) glow(W / 2, 430, 460 - i * 65, "rgba(10,30,85,0.035)");
    for (const s of stars) {
      const y = (s.y + t * s.speed) % H;
      const b = 0.5 + 0.5 * Math.sin(t * 1.7 + s.phase);
      glow(s.x, y, s.r, `rgba(160,215,255,${(35 + b * 70) / 255})`);
    }
  }

  function drawMaze(t) {
    for (let r = 0; r < ROWS; r++)
      for (let c = 0; c < COLS; c++) {
        const tile = maze.grid[r][c];
        const x = MAZE_LEFT + c * TILE, y = MAZE_TOP + r * TILE;
        if (tile === "#") {
          ctx.fillStyle = "rgba(0,170,255,0.09)"; ctx.fillRect(x - 2, y - 2, TILE + 4, TILE + 4);
          ctx.fillStyle = C.wallFill; ctx.fillRect(x + 2, y + 2, TILE - 4, TILE - 4);
          ctx.strokeStyle = C.wallOutline; ctx.lineWidth = 1.5; ctx.strokeRect(x + 1.25, y + 1.25, TILE - 2.5, TILE - 2.5);
          ctx.fillStyle = "rgba(20,55,115,0.47)"; ctx.fillRect(x + 6, y + 6, TILE - 12, TILE - 12);
        } else if (tile === ".") {
          const cx = x + TILE / 2, cy = y + TILE / 2;
          glow(cx, cy, 5.5, "rgba(255,220,120,0.14)");
          glow(cx, cy, 2.8, C.dot);
        } else if (tile === "o") {
          const cx = x + TILE / 2, cy = y + TILE / 2;
          const p = 1 + 0.18 * Math.sin(t * 5);
          glow(cx, cy, 14 * p, "rgba(80,220,255,0.18)");
          glow(cx, cy, 6.5 * p, C.pellet);
          ctx.strokeStyle = "rgba(220,255,255,0.86)"; ctx.lineWidth = 1.5;
          ctx.beginPath(); ctx.arc(cx, cy, 6.5 * p, 0, Math.PI * 2); ctx.stroke();
        }
      }
  }

  // ------------------------------------------------------------ game state
  const GHOST_COLORS = ["rgb(255,70,70)", "rgb(255,120,200)", "rgb(60,220,255)", "rgb(255,160,50)"];
  const DIFF_NAMES = ["CHILL", "CLASSIC", "HARD"];
  const player = new Player();
  const g = {
    state: "menu", level: 1, score: 0, lives: START_LIVES,
    highScore: parseInt(localStorage.getItem("neonmaze.highscore") || "0", 10) || 0,
    difficulty: Math.min(2, Math.max(0, parseInt(localStorage.getItem("neonmaze.difficulty") || "1", 10) || 1)),
    power: 0, invuln: 0, ghostsEaten: 0, stateTimer: 0, time: 0,
    bonus: null, bonusTimer: 0, dotsEaten: 0, nextBonusAt: 40,
    ghosts: [], popups: [],
  };

  const saveHighScore = () => localStorage.setItem("neonmaze.highscore", String(g.highScore));
  const addScore = (p) => { g.score += p; if (g.score > g.highScore) g.highScore = g.score; };
  const ghostSpeedFactor = () => [0.85, 1, 1.1][g.difficulty] * (1 + (g.level - 1) * 0.06);
  const powerDuration = () => Math.max(4, [8, 7, 6][g.difficulty] - (g.level - 1) * 0.5);

  function changeState(s) { g.state = s; g.stateTimer = 0; menu.selected = 0; }

  function loadLevel(n) {
    g.level = n;
    maze.load(n);
    player.reset(maze.playerStart);
    const order = [4, 1, 7, 3];
    const count = n === 1 ? 3 : 4;
    g.ghosts = [];
    for (let i = 0; i < count; i++) {
      const gh = new Ghost(GHOST_COLORS[i], i);
      const idx = order[i] < maze.ghostStarts.length ? order[i] : i % maze.ghostStarts.length;
      gh.reset(maze.ghostStarts[idx]);
      g.ghosts.push(gh);
    }
    g.power = 0; g.invuln = 0; g.ghostsEaten = 0;
    g.bonus = null; g.bonusTimer = 0; g.dotsEaten = 0; g.nextBonusAt = 40; g.popups = [];
  }

  function startNewGame() {
    saveHighScore();
    g.score = 0; g.lives = START_LIVES;
    loadLevel(1);
    changeState("playing");
  }

  function resetActors() {
    player.reset(maze.playerStart);
    for (const gh of g.ghosts) gh.reset(gh.start);
    g.power = 0; g.invuln = 2; g.ghostsEaten = 0; g.bonus = null; g.bonusTimer = 0;
  }

  function spawnBonus() {
    const tiles = maze.walkableTiles();
    for (let i = 0; i < 40; i++) {
      const c = tiles[rand(tiles.length)];
      if (c.x === player.cur.x && c.y === player.cur.y) continue;
      if (maze.tile(c.x, c.y) === "G") continue;
      g.bonus = c; g.bonusTimer = 8; g.nextBonusAt += 55;
      return;
    }
  }

  function popup(textStr, x, y) { g.popups.push({ text: textStr, x, y, life: 1 }); }

  function updatePlaying(dt) {
    if (g.invuln > 0) g.invuln = Math.max(0, g.invuln - dt);
    if (g.power > 0) { g.power -= dt; if (g.power <= 0) { g.power = 0; g.ghostsEaten = 0; } }
    if (g.bonus) { g.bonusTimer -= dt; if (g.bonusTimer <= 0) g.bonus = null; }

    player.update(dt);
    const pt = pixelToTile(player.pos);
    const got = maze.collect(pt.x, pt.y);
    if (got) {
      addScore(got.points);
      g.dotsEaten++;
      if (got.power) { g.power = powerDuration(); g.ghostsEaten = 0; sound.play("power"); }
      else if (g.time - lastEatSound > 0.09) { sound.play("eat", 0.6); lastEatSound = g.time; }
      if (!g.bonus && g.dotsEaten >= g.nextBonusAt) spawnBonus();
    }

    if (maze.remaining <= 0) {
      addScore(500 * g.level); sound.play("level"); saveHighScore();
      changeState("levelComplete"); g.stateTimer = 2.2;
      return;
    }

    if (g.bonus) {
      const b = tileCenter(g.bonus.x, g.bonus.y);
      if (Math.hypot(player.pos.x - b.x, player.pos.y - b.y) < 20) {
        addScore(100 * g.level); popup(`+${100 * g.level}`, b.x, b.y); g.bonus = null; sound.play("power");
      }
    }

    const frightened = g.power > 0;
    const sf = ghostSpeedFactor();
    for (const gh of g.ghosts) gh.update(dt, player.cur, player.facing, frightened, sf);

    for (const gh of g.ghosts) {
      if (!gh.active) continue;
      const d = Math.hypot(player.pos.x - gh.pos.x, player.pos.y - gh.pos.y);
      if (d < (PLAYER_RADIUS + GHOST_RADIUS) * 0.82) {
        if (frightened) {
          const pts = 200 * (1 << g.ghostsEaten);
          g.ghostsEaten = Math.min(4, g.ghostsEaten + 1);
          popup(`+${pts}`, gh.pos.x, gh.pos.y);
          gh.eat(); addScore(pts); sound.play("ghost");
        } else if (g.invuln <= 0) {
          sound.play("death");
          changeState("dying"); g.stateTimer = 1.5;
          return;
        }
      }
    }
  }

  function update(dt) {
    for (const p of g.popups) { p.life -= dt; p.y -= dt * 22; }
    g.popups = g.popups.filter((p) => p.life > 0);

    if (g.state === "playing") updatePlaying(dt);
    else if (g.state === "dying") {
      g.stateTimer -= dt;
      if (g.stateTimer <= 0) {
        g.lives--;
        if (g.lives <= 0) { g.lives = 0; saveHighScore(); changeState("gameOver"); }
        else { resetActors(); g.invuln = 2.2; changeState("playing"); }
      }
    } else if (g.state === "levelComplete") {
      g.stateTimer -= dt;
      if (g.stateTimer <= 0) {
        if (g.level >= TOTAL_LEVELS) { saveHighScore(); changeState("victory"); }
        else { loadLevel(g.level + 1); changeState("playing"); }
      }
    }
  }

  // ------------------------------------------------------------------ menus
  const menu = {
    selected: 0, buttons: [],
    hover: null,
    setButtons(list) { this.buttons = list; if (this.selected >= list.length) this.selected = 0; },
    move(d) { const n = this.buttons.length; if (n) this.selected = (this.selected + d + n) % n; },
    activate() { const b = this.buttons[this.selected]; if (b) { sound.play("click"); action(b.action); } },
  };

  function buttonsFor(state) {
    const bx = 285, bw = 330, bh = 54;
    const mk = (y, a, label) => ({ x: bx, y, w: bw, h: bh, action: a, label });
    switch (state) {
      case "menu": return [mk(310, "play", "PLAY GAME"), mk(376, "instructions", "INSTRUCTIONS"), mk(442, "settings", "SETTINGS"), mk(508, "fullscreen", "FULLSCREEN")];
      case "instructions": return [mk(640, "back", "BACK")];
      case "settings": return [
        mk(250, "sound", `SOUND: ${sound.enabled ? "ON" : "OFF"}`),
        mk(320, "difficulty", `DIFFICULTY: ${DIFF_NAMES[g.difficulty]}`),
        mk(390, "fullscreen", "TOGGLE FULLSCREEN"),
        mk(500, "back", "BACK")];
      case "paused": return [mk(290, "resume", "RESUME"), mk(356, "restart", "RESTART GAME"), mk(422, "mainMenu", "MAIN MENU")];
      case "gameOver": case "victory": return [mk(state === "victory" ? 420 : 430, "restart", "PLAY AGAIN"), mk(state === "victory" ? 486 : 496, "mainMenu", "MAIN MENU")];
      default: return [];
    }
  }

  function toggleFullscreen() {
    const el = document.documentElement;
    if (!document.fullscreenElement) (el.requestFullscreen || el.webkitRequestFullscreen || (() => {})).call(el);
    else (document.exitFullscreen || document.webkitExitFullscreen).call(document);
  }

  function action(a) {
    switch (a) {
      case "play": case "restart": startNewGame(); break;
      case "instructions": changeState("instructions"); break;
      case "settings": changeState("settings"); break;
      case "back": changeState("menu"); break;
      case "resume": changeState("playing"); break;
      case "mainMenu": saveHighScore(); changeState("menu"); break;
      case "fullscreen": toggleFullscreen(); break;
      case "sound":
        sound.enabled = !sound.enabled;
        localStorage.setItem("neonmaze.sound", sound.enabled ? "on" : "off");
        if (sound.enabled) sound.play("click");
        break;
      case "difficulty":
        g.difficulty = (g.difficulty + 1) % 3;
        localStorage.setItem("neonmaze.difficulty", String(g.difficulty));
        break;
    }
  }

  function drawButtons() {
    menu.setButtons(buttonsFor(g.state));
    menu.buttons.forEach((b, i) => {
      const hl = i === menu.selected || menu.hover === i;
      if (hl) { ctx.fillStyle = "rgba(0,200,255,0.1)"; ctx.fillRect(b.x - 5, b.y - 5, b.w + 10, b.h + 10); }
      ctx.fillStyle = hl ? "rgba(0,55,90,0.92)" : "rgba(10,20,45,0.84)";
      ctx.fillRect(b.x, b.y, b.w, b.h);
      ctx.lineWidth = 2; ctx.strokeStyle = hl ? C.accent : "rgba(0,120,180,0.67)";
      ctx.strokeRect(b.x, b.y, b.w, b.h);
      text(b.label, 18, b.x + b.w / 2, b.y + b.h / 2 + 1, hl ? "#fff" : C.light, { baseline: "middle" });
    });
  }

  function drawPanel(x, y, w, h) {
    ctx.fillStyle = "rgba(8,14,38,0.9)"; ctx.fillRect(x, y, w, h);
    ctx.lineWidth = 2; ctx.strokeStyle = "rgba(0,190,255,0.6)"; ctx.strokeRect(x, y, w, h);
  }

  function drawMainMenu(t) {
    text("NEON", 58, W / 2, 62, C.gold, { title: true, glow: true, outline: true });
    text("MAZE", 22, W / 2, 140, C.accent, { title: true, glow: true });
    const bob = Math.sin(t * 3) * 4;
    const runX = 270 + ((t * 60) % 40);
    drawChomper(runX, 232 + bob, 18, C.yellow, 0, 10 + 25 * (0.5 + 0.5 * Math.sin(t * 10)));
    GHOST_COLORS.forEach((col, i) => {
      const gx = 380 + i * 70, gy = 232 + Math.sin(t * 3 + i) * 5;
      glow(gx, gy, 20, withAlpha(col, 0.13));
      drawGhost(gx, gy, 16, col, "left", false, false);
    });
    drawButtons();
    text(`HIGH SCORE ${pad(g.highScore)}`, 16, W / 2, 602, C.light, { title: true });
    text("ARROWS / WASD + ENTER   |   MOUSE & TOUCH SUPPORTED", 14, W / 2, 648, C.dim);
  }

  function drawInstructions() {
    text("HOW TO PLAY", 30, W / 2, 42, C.accent, { title: true, glow: true });
    drawPanel(110, 110, 680, 500);
    const lines = [
      ["MOVE", "W A S D  /  ARROW KEYS  /  SWIPE"],
      ["PAUSE / RESUME", "P  or  ESC"],
      ["FULLSCREEN", "F"],
    ];
    lines.forEach(([a, b], i) => {
      text(a, 16, 150, 145 + i * 38, C.gold, { align: "left" });
      text(b, 16, 750, 145 + i * 38, C.light, { align: "right" });
    });
    const tips = [
      "EAT EVERY DOT TO CLEAR THE MAZE.",
      "BIG GLOWING PELLETS START POWER MODE.",
      "IN POWER MODE, EAT GHOSTS FOR 200, 400, 800, 1600.",
      "COLLECT THE BONUS CHERRIES FOR EXTRA POINTS.",
      "YOU HAVE 3 LIVES. ONE GHOST TOUCH = ONE LIFE.",
      "TURN EARLY: YOU TURN AT THE NEXT TILE CENTER.",
      "CLEAR ALL 3 MAZES TO WIN THE GAME.",
    ];
    tips.forEach((s, i) => text(s, 15, 150, 290 + i * 42, C.light, { align: "left" }));
    drawButtons();
    text("ESC TO RETURN", 13, W / 2, 710, C.dim);
  }

  function drawSettings() {
    text("SETTINGS", 34, W / 2, 70, C.accent, { title: true, glow: true });
    drawButtons();
    const info = ["CHILL: SLOWER GHOSTS, LONGER POWER MODE", "CLASSIC: THE ORIGINAL BALANCE", "HARD: FASTER GHOSTS, SHORTER POWER MODE"];
    text(info[g.difficulty], 14, W / 2, 460, C.dim);
  }

  function overlay(color) { ctx.fillStyle = color; ctx.fillRect(0, 0, W, H); }

  function drawPause() {
    overlay("rgba(2,4,18,0.7)");
    drawPanel(240, 100, 420, 400);
    text("PAUSED", 40, W / 2, 130, C.accent, { title: true, glow: true });
    text(`SCORE ${pad(g.score)}   LEVEL ${g.level}`, 15, W / 2, 210, C.light);
    drawButtons();
  }

  function drawGameOver() {
    overlay("rgba(2,4,18,0.75)");
    drawPanel(200, 80, 500, 520);
    text("GAME OVER", 40, W / 2, 110, C.danger, { title: true, glow: true });
    text(`SCORE ${pad(g.score)}`, 20, W / 2, 210, C.light, { title: true });
    text(`HIGH SCORE ${pad(g.highScore)}`, 16, W / 2, 260, C.gold, { title: true });
    text(`LEVEL REACHED: ${g.level}`, 16, W / 2, 310, C.dim);
    drawButtons();
    text("R TO RESTART     M FOR MENU", 13, W / 2, 570, C.dim);
  }

  function drawVictory(t) {
    overlay("rgba(2,4,18,0.75)");
    drawPanel(200, 60, 500, 540);
    const p = 1 + 0.05 * Math.sin(t * 4);
    ctx.save(); ctx.translate(W / 2, 110); ctx.scale(p, p);
    text("YOU WIN!", 46, 0, -20, C.gold, { title: true, glow: true, outline: true });
    ctx.restore();
    text("ALL 3 MAZES CLEARED", 16, W / 2, 180, C.accent);
    text(`FINAL SCORE ${pad(g.score)}`, 18, W / 2, 240, C.light, { title: true });
    text(`HIGH SCORE ${pad(g.highScore)}`, 16, W / 2, 290, C.gold, { title: true });
    drawButtons();
    text("R TO RESTART     M FOR MENU", 13, W / 2, 570, C.dim);
  }

  function drawHUD() {
    ctx.fillStyle = "rgba(7,11,32,0.93)"; ctx.fillRect(0, 0, W, 92);
    ctx.fillStyle = "rgba(0,190,255,0.47)"; ctx.fillRect(0, 90, W, 2);
    text("SCORE", 12, 40, 14, C.accent, { align: "left" });
    text(pad(g.score), 16, 40, 38, C.light, { align: "left", title: true });
    text("HIGH SCORE", 12, W / 2, 14, C.accent);
    text(pad(g.highScore), 16, W / 2, 38, C.gold, { title: true });
    text("LEVEL", 12, W - 40, 14, C.accent, { align: "right" });
    text(String(g.level), 16, W - 40, 38, C.light, { align: "right", title: true });

    text("LIVES", 11, 40, 738, C.accent, { align: "left" });
    for (let i = 0; i < g.lives; i++) drawChomper(108 + i * 25, 757, 8, C.yellow, Math.PI, 28);

    if (g.power > 0) {
      text("POWER MODE", 11, W / 2, 736, C.accent);
      const fw = 240, ratio = Math.max(0, Math.min(1, g.power / powerDuration()));
      ctx.fillStyle = "rgb(10,24,45)"; ctx.fillRect(W / 2 - fw / 2, 754, fw, 9);
      ctx.strokeStyle = "rgb(0,150,210)"; ctx.lineWidth = 1; ctx.strokeRect(W / 2 - fw / 2, 754, fw, 9);
      ctx.fillStyle = "rgb(80,235,255)"; ctx.fillRect(W / 2 - fw / 2, 754, fw * ratio, 9);
    }
    text("DOTS LEFT", 11, W - 40, 738, C.accent, { align: "right" });
    text(String(maze.remaining), 14, W - 40, 756, C.light, { align: "right", title: true });
  }

  function drawBonus(t) {
    if (!g.bonus) return;
    const p = 1 + 0.12 * Math.sin(t * 6);
    const c = tileCenter(g.bonus.x, g.bonus.y);
    glow(c.x, c.y, 18 * p, "rgba(255,80,120,0.22)");
    ctx.strokeStyle = "rgb(80,220,120)"; ctx.lineWidth = 2;
    ctx.beginPath(); ctx.moveTo(c.x - 5, c.y + 1); ctx.quadraticCurveTo(c.x, c.y - 10, c.x + 3, c.y - 11);
    ctx.moveTo(c.x + 5, c.y + 3); ctx.quadraticCurveTo(c.x + 4, c.y - 6, c.x + 3, c.y - 11); ctx.stroke();
    glow(c.x + 6, c.y - 11, 3, "rgb(80,220,120)");
    glow(c.x - 5, c.y + 3, 6, "rgb(240,40,80)");
    glow(c.x + 5, c.y + 5, 6, "rgb(255,70,110)");
  }

  function drawPlaying(t) {
    drawMaze(t);
    drawBonus(t);
    const fr = g.power > 0;
    const flash = fr && g.power < 2 && Math.floor(g.power * 8) % 2 === 0;
    for (const gh of g.ghosts) gh.draw(t, fr, flash);
    player.draw(t, fr, g.invuln > 0 || g.state === "dying");
    for (const p of g.popups) {
      ctx.globalAlpha = Math.max(0, p.life);
      text(p.text, 10, p.x, p.y - 8, C.accent, { title: true });
      ctx.globalAlpha = 1;
    }
    if (fr) {
      const pulse = 0.5 + 0.5 * Math.sin(t * 8);
      ctx.fillStyle = `rgba(40,220,255,${(7 + pulse * 8) / 255})`;
      ctx.fillRect(MAZE_LEFT, MAZE_TOP, COLS * TILE, ROWS * TILE);
      ctx.strokeStyle = `rgba(100,240,255,${(90 + pulse * 100) / 255})`; ctx.lineWidth = 3;
      ctx.strokeRect(MAZE_LEFT - 5.5, MAZE_TOP - 5.5, COLS * TILE + 11, ROWS * TILE + 11);
    }
  }

  function draw(t) {
    drawBackground(t);
    const s = g.state;
    if (s === "menu") drawMainMenu(t);
    else if (s === "instructions") drawInstructions();
    else if (s === "settings") drawSettings();
    else {
      drawPlaying(t); drawHUD();
      if (s === "paused") drawPause();
      else if (s === "dying") {
        overlay(`rgba(130,0,25,${(35 + (0.5 + 0.5 * Math.sin(t * 12)) * 35) / 255})`);
        text("LIFE LOST", 30, W / 2, 320, C.danger, { title: true, glow: true });
        text("GET READY...", 15, W / 2, 372, C.light);
      } else if (s === "levelComplete") {
        overlay("rgba(2,4,18,0.69)");
        text(`LEVEL ${g.level} CLEARED`, 27, W / 2, 300, C.accent, { title: true, glow: true });
        text(`BONUS +${500 * g.level} POINTS`, 18, W / 2, 352, C.gold);
        text(g.level >= TOTAL_LEVELS ? "ALL MAZES CLEARED!" : "NEXT MAZE INCOMING...", 14, W / 2, 404, C.light);
      } else if (s === "gameOver") drawGameOver();
      else if (s === "victory") drawVictory(t);
      else if (s === "playing" && g.level === 1 && maze.remaining > 0 && g.dotsEaten === 0 && !player.moving && g.invuln <= 0) {
        text("PRESS AN ARROW KEY OR SWIPE TO START", 13, W / 2, 712, C.dim);
      }
    }
  }

  // ------------------------------------------------------------------ input
  const KEY_DIR = {
    ArrowUp: "up", KeyW: "up", ArrowDown: "down", KeyS: "down",
    ArrowLeft: "left", KeyA: "left", ArrowRight: "right", KeyD: "right",
  };
  const MENU_STATES = ["menu", "instructions", "settings", "paused", "gameOver", "victory"];

  function steer(dir) {
    if (g.state === "playing") player.want = dir;
    else if (MENU_STATES.includes(g.state)) {
      if (dir === "up") menu.move(-1);
      else if (dir === "down") menu.move(1);
    }
  }

  function togglePause() {
    if (g.state === "playing") { sound.play("click"); changeState("paused"); }
    else if (g.state === "paused") { sound.play("click"); changeState("playing"); }
  }

  window.addEventListener("keydown", (e) => {
    sound.unlock();
    const dir = KEY_DIR[e.code];
    if (dir || e.code === "Space") e.preventDefault();
    if (e.repeat && g.state !== "playing") return;
    if (e.code === "KeyF") { toggleFullscreen(); return; }

    if (g.state === "playing") {
      if (dir) player.want = dir;
      else if (e.code === "KeyP" || e.code === "Escape") togglePause();
      return;
    }
    if (dir) { steer(dir); return; }
    if (e.code === "Enter" || e.code === "Space") { menu.activate(); return; }
    if (g.state === "paused" && (e.code === "KeyP" || e.code === "Escape")) { togglePause(); return; }
    if ((g.state === "instructions" || g.state === "settings") && (e.code === "Escape" || e.code === "Backspace")) { changeState("menu"); return; }
    if (["paused", "gameOver", "victory"].includes(g.state)) {
      if (e.code === "KeyR") startNewGame();
      else if (e.code === "KeyM" || (e.code === "Escape" && g.state !== "paused")) { saveHighScore(); changeState("menu"); }
    }
  });

  function toLogical(e) {
    const r = canvas.getBoundingClientRect();
    return { x: ((e.clientX - r.left) / r.width) * W, y: ((e.clientY - r.top) / r.height) * H };
  }
  function buttonAt(p) {
    return menu.buttons.findIndex((b) => p.x >= b.x && p.x <= b.x + b.w && p.y >= b.y && p.y <= b.y + b.h);
  }

  canvas.addEventListener("pointermove", (e) => {
    if (!MENU_STATES.includes(g.state)) { menu.hover = null; return; }
    const i = buttonAt(toLogical(e));
    menu.hover = i >= 0 ? i : null;
    canvas.style.cursor = i >= 0 ? "pointer" : "default";
  });

  let swipe = null;
  canvas.addEventListener("pointerdown", (e) => {
    sound.unlock();
    swipe = { x: e.clientX, y: e.clientY, fired: false };
    if (MENU_STATES.includes(g.state)) {
      const i = buttonAt(toLogical(e));
      if (i >= 0) { menu.selected = i; menu.activate(); swipe = null; }
    }
  });
  canvas.addEventListener("pointermove", (e) => {
    if (!swipe || swipe.fired || g.state !== "playing") return;
    const dx = e.clientX - swipe.x, dy = e.clientY - swipe.y;
    if (Math.max(Math.abs(dx), Math.abs(dy)) < 24) return;
    player.want = Math.abs(dx) > Math.abs(dy) ? (dx > 0 ? "right" : "left") : (dy > 0 ? "down" : "up");
    swipe = { x: e.clientX, y: e.clientY, fired: false };
  });
  window.addEventListener("pointerup", () => { swipe = null; });

  document.querySelectorAll("#touch button").forEach((btn) => {
    btn.addEventListener("pointerdown", (e) => {
      e.preventDefault();
      sound.unlock();
      const d = btn.dataset.dir;
      if (d === "pause") {
        if (g.state === "playing" || g.state === "paused") togglePause();
        else menu.activate();
      } else steer(d);
    });
  });

  document.addEventListener("visibilitychange", () => {
    if (document.hidden && g.state === "playing") changeState("paused");
  });
  window.addEventListener("resize", setupCanvas);
  document.addEventListener("fullscreenchange", setupCanvas);

  // ------------------------------------------------------------- main loop
  let last = performance.now();
  function frame(now) {
    const dt = Math.min(0.05, (now - last) / 1000);
    last = now;
    g.time += dt;
    update(dt);
    draw(g.time);
    requestAnimationFrame(frame);
  }

  maze.load(1);
  setupCanvas();
  const start = () => requestAnimationFrame((t) => { last = t; frame(t); });
  if (document.fonts && document.fonts.load) {
    Promise.all([document.fonts.load(`16px ${TITLE_FONT}`), document.fonts.load(`16px ${TEXT_FONT}`)])
      .catch(() => {}).finally(start);
  } else start();
})();
