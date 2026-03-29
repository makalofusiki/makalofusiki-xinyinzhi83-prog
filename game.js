/**
 * Snake Game — game.js
 *
 * Controls: Arrow keys or W/A/S/D
 */

const canvas = document.getElementById('canvas');
const ctx = canvas.getContext('2d');

const COLS = 20;
const ROWS = 20;
const CELL = canvas.width / COLS; // 20px per cell

const scoreEl = document.getElementById('score');
const highScoreEl = document.getElementById('high-score');
const overlay = document.getElementById('overlay');
const overlayTitle = document.getElementById('overlay-title');
const overlayMsg = document.getElementById('overlay-msg');
const finalScoreEl = document.getElementById('final-score');
const startBtn = document.getElementById('start-btn');

// Directions
const DIR = {
  UP:    { x: 0,  y: -1 },
  DOWN:  { x: 0,  y:  1 },
  LEFT:  { x: -1, y:  0 },
  RIGHT: { x: 1,  y:  0 },
};

let snake, direction, nextDirection, food, score, highScore, gameLoop, running;

highScore = parseInt(localStorage.getItem('snakeHighScore') || '0', 10);
highScoreEl.textContent = highScore;

function init() {
  const mid = Math.floor(COLS / 2);
  snake = [
    { x: mid,     y: Math.floor(ROWS / 2) },
    { x: mid - 1, y: Math.floor(ROWS / 2) },
    { x: mid - 2, y: Math.floor(ROWS / 2) },
  ];
  direction     = DIR.RIGHT;
  nextDirection = DIR.RIGHT;
  score = 0;
  scoreEl.textContent = 0;
  placeFood();
}

function placeFood() {
  const emptyCells = [];
  for (let x = 0; x < COLS; x++) {
    for (let y = 0; y < ROWS; y++) {
      if (!snake.some(s => s.x === x && s.y === y)) {
        emptyCells.push({ x, y });
      }
    }
  }
  food = emptyCells[Math.floor(Math.random() * emptyCells.length)];
}

function update() {
  direction = nextDirection;

  const head = { x: snake[0].x + direction.x, y: snake[0].y + direction.y };

  // Wall collision
  if (head.x < 0 || head.x >= COLS || head.y < 0 || head.y >= ROWS) {
    return gameOver();
  }
  // Self collision
  if (snake.some(s => s.x === head.x && s.y === head.y)) {
    return gameOver();
  }

  snake.unshift(head);

  if (head.x === food.x && head.y === food.y) {
    score++;
    scoreEl.textContent = score;
    if (score > highScore) {
      highScore = score;
      highScoreEl.textContent = highScore;
      localStorage.setItem('snakeHighScore', highScore);
    }
    placeFood();
  } else {
    snake.pop();
  }

  draw();
}

function draw() {
  // Background
  ctx.fillStyle = '#0f0f1e';
  ctx.fillRect(0, 0, canvas.width, canvas.height);

  // Grid (subtle)
  ctx.strokeStyle = 'rgba(255,255,255,0.03)';
  ctx.lineWidth = 0.5;
  for (let x = 0; x <= COLS; x++) {
    ctx.beginPath();
    ctx.moveTo(x * CELL, 0);
    ctx.lineTo(x * CELL, canvas.height);
    ctx.stroke();
  }
  for (let y = 0; y <= ROWS; y++) {
    ctx.beginPath();
    ctx.moveTo(0, y * CELL);
    ctx.lineTo(canvas.width, y * CELL);
    ctx.stroke();
  }

  // Food
  ctx.fillStyle = '#ff6b6b';
  ctx.shadowColor = '#ff6b6b';
  ctx.shadowBlur = 10;
  ctx.beginPath();
  const fx = food.x * CELL + CELL / 2;
  const fy = food.y * CELL + CELL / 2;
  ctx.arc(fx, fy, CELL / 2 - 2, 0, Math.PI * 2);
  ctx.fill();
  ctx.shadowBlur = 0;

  // Snake body
  snake.forEach((seg, i) => {
    const ratio = 1 - i / snake.length;
    ctx.fillStyle = i === 0
      ? '#4ecca3'
      : `rgba(78, ${Math.round(140 + 64 * ratio)}, ${Math.round(100 + 63 * ratio)}, 0.9)`;
    ctx.shadowColor = i === 0 ? '#4ecca3' : 'transparent';
    ctx.shadowBlur  = i === 0 ? 8 : 0;
    const margin = i === 0 ? 1 : 2;
    ctx.beginPath();
    ctx.roundRect(
      seg.x * CELL + margin,
      seg.y * CELL + margin,
      CELL - margin * 2,
      CELL - margin * 2,
      i === 0 ? 4 : 3
    );
    ctx.fill();
  });
  ctx.shadowBlur = 0;
}

function gameOver() {
  clearInterval(gameLoop);
  running = false;
  overlayTitle.textContent = 'GAME OVER';
  overlayTitle.className = '';
  overlayMsg.textContent = 'Your score:';
  finalScoreEl.textContent = score;
  finalScoreEl.classList.remove('hidden');
  startBtn.textContent = 'PLAY AGAIN';
  overlay.classList.remove('hidden');
}

function startGame() {
  overlay.classList.add('hidden');
  init();
  draw();
  running = true;
  clearInterval(gameLoop);
  gameLoop = setInterval(update, 120);
}

// Keyboard input
document.addEventListener('keydown', (e) => {
  switch (e.key) {
    case 'ArrowUp':    case 'w': case 'W':
      if (direction !== DIR.DOWN)  nextDirection = DIR.UP;    break;
    case 'ArrowDown':  case 's': case 'S':
      if (direction !== DIR.UP)    nextDirection = DIR.DOWN;  break;
    case 'ArrowLeft':  case 'a': case 'A':
      if (direction !== DIR.RIGHT) nextDirection = DIR.LEFT;  break;
    case 'ArrowRight': case 'd': case 'D':
      if (direction !== DIR.LEFT)  nextDirection = DIR.RIGHT; break;
    case ' ':
      if (!running) startGame();
      break;
  }
  // Prevent page scrolling with arrow keys
  if (['ArrowUp','ArrowDown','ArrowLeft','ArrowRight',' '].includes(e.key)) {
    e.preventDefault();
  }
});

startBtn.addEventListener('click', startGame);
