const TILE_SIZE = 32;
const MAP_NUM_ROWS = 11;
const MAP_NUM_COLS = 15;

const WINDOW_WIDTH = MAP_NUM_COLS * TILE_SIZE;
const WINDOW_HEIGHT = MAP_NUM_ROWS * TILE_SIZE;

const FOV_ANGLE = 60 * Math.PI / 180;

const WALL_STRIP_WIDTH = 20;
const NUM_RAIS = WINDOW_WIDTH / WALL_STRIP_WIDTH;

class Map {
    constructor() {
        this.grid = [
            [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1],
            [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1],
            [1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
            [1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
            [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
        ];
    }
    render() {
        for (var i = 0; i < MAP_NUM_ROWS; i++) {
            for (var j = 0; j < MAP_NUM_COLS; j++) {
                var tileX = j * TILE_SIZE;
                var tileY = i * TILE_SIZE;
                var tileColor = this.grid[i][j] == 1 ? "#222" : "#fff";
                stroke("#222");
                fill(tileColor);
                rect(tileX, tileY, TILE_SIZE, TILE_SIZE);
            }
        }
    }
    isWall(x, y) {
        var i = Math.floor(y / TILE_SIZE);
        var j = Math.floor(x / TILE_SIZE);

        if (j < 0 || j > MAP_NUM_COLS || i < 0 || i > MAP_NUM_ROWS) return true;

        return this.grid[i][j] === 1;
    }
}

class Player {
    constructor() {
        this.x = WINDOW_WIDTH / 2;
        this.y = WINDOW_HEIGHT / 2;
        this.radius = 3;
        this.turnDirection = 0; // -1 if left, 1 if right
        this.walkDirection = 0; // -1 if back, 1 if front
        this.rotationAngle = Math.PI / 2; // 90
        this.moveSpeed = 2;
        this.rotationSpeed = 5 * Math.PI / 180;
    }
    render() {
        noStroke();
        fill("red");
        circle(
            this.x,
            this.y,
            this.radius
        )
        stroke("red");
        line(
            this.x,
            this.y,
            this.x + Math.cos(this.rotationAngle) * 30,
            this.y + Math.sin(this.rotationAngle) * 30
        )
    }

    update() {
        this.rotationAngle += this.turnDirection * this.rotationSpeed;

        var nextX = this.x + Math.cos(this.rotationAngle) * this.moveSpeed * this.walkDirection;
        var nextY = this.y + Math.sin(this.rotationAngle) * this.moveSpeed * this.walkDirection;

        if (grid.isWall(nextX, nextY)) return;

        this.x = nextX;
        this.y = nextY;
    }
}

class Ray {
    constructor(rayAngle) {
        this.rayAngle = rayAngle;
    }
    render() {
        stroke("rgba(255,0,0, 0.3)");
        line(
            player.x,
            player.y,
            player.x + Math.cos(this.rayAngle) * 50,
            player.y + Math.sin(this.rayAngle) * 50,
        )
    }
}

var grid = new Map();
var player = new Player();
var rays = [];

function setup() {
    createCanvas(WINDOW_WIDTH, WINDOW_HEIGHT);
}

function keyPressed() {
    if (keyCode == UP_ARROW) {
        player.walkDirection = 1;
    } else if (keyCode == DOWN_ARROW) {
        player.walkDirection = -1;
    }

    if (keyCode == RIGHT_ARROW) {
        player.turnDirection = 1;
    } else if (keyCode == LEFT_ARROW) {
        player.turnDirection = -1;
    }
}

function keyReleased() {
    if (keyCode == UP_ARROW) {
        player.walkDirection = 0;
    } else if (keyCode == DOWN_ARROW) {
        player.walkDirection = 0;
    }

    if (keyCode == RIGHT_ARROW) {
        player.turnDirection = 0;
    } else if (keyCode == LEFT_ARROW) {
        player.turnDirection = 0;
    }
}

function castAllRays() {
    // Start casting by half the FOV
    const rayAngleStep = FOV_ANGLE / NUM_RAIS;
    let rayAngle = player.rotationAngle - FOV_ANGLE / 2;
    rays = [];
    // for (let i = 0; i < NUM_RAIS; i++) {
    for (let i = 0; i < 1; i++) {
        var ray = new Ray(rayAngle);
        // ray.cast();
        rays.push(ray);
        rayAngle += rayAngleStep;
    }
}

function update() {
    player.update();
    castAllRays();
}

function draw() {
    update();

    grid.render();
    for (const ray of rays) {
        ray.render();
    }
    player.render();
}
