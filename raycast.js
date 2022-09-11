const TILE_SIZE = 64;
const MAP_NUM_ROWS = 11;
const MAP_NUM_COLS = 15;

const WINDOW_WIDTH = MAP_NUM_COLS * TILE_SIZE;
const WINDOW_HEIGHT = MAP_NUM_ROWS * TILE_SIZE;
const MINIMAP_SCALE_FACTOR = 0.2;

const FOV_ANGLE = 60 * (Math.PI / 180);

const WALL_STRIP_WIDTH = 5;
const NUM_RAYS = WINDOW_WIDTH / WALL_STRIP_WIDTH;

const distanceProjectedPlane = (WINDOW_WIDTH / 2) / Math.tan(FOV_ANGLE / 2);

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
    hasWallAt(x, y) {
        if (x < 0 || x > WINDOW_WIDTH || y < 0 || y > WINDOW_HEIGHT) return true;

        var i = Math.floor(y / TILE_SIZE);
        var j = Math.floor(x / TILE_SIZE);
        return this.grid[i][j] != 0;
    }
    render() {
        for (var i = 0; i < MAP_NUM_ROWS; i++) {
            for (var j = 0; j < MAP_NUM_COLS; j++) {
                var tileX = j * TILE_SIZE;
                var tileY = i * TILE_SIZE;
                var tileColor = this.grid[i][j] == 1 ? "#222" : "#fff";
                stroke("#222");
                fill(tileColor);
                rect(
                    MINIMAP_SCALE_FACTOR * tileX,
                    MINIMAP_SCALE_FACTOR * tileY,
                    MINIMAP_SCALE_FACTOR * TILE_SIZE,
                    MINIMAP_SCALE_FACTOR * TILE_SIZE
                );
            }
        }
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
        this.rotationSpeed = 5 * (Math.PI / 180);
    }
    update() {
        this.rotationAngle += this.turnDirection * this.rotationSpeed;

        let moveStep = this.moveSpeed * this.walkDirection;
        let nextX = this.x + Math.cos(this.rotationAngle) * moveStep;
        let nextY = this.y + Math.sin(this.rotationAngle) * moveStep;

        if (grid.hasWallAt(nextX, nextY)) return;

        this.x = nextX;
        this.y = nextY;
    }
    render() {
        noStroke();
        fill("red");
        circle(
            MINIMAP_SCALE_FACTOR * this.x,
            MINIMAP_SCALE_FACTOR * this.y,
            MINIMAP_SCALE_FACTOR * this.radius
        )
        stroke("red");
        line(
            MINIMAP_SCALE_FACTOR * this.x,
            MINIMAP_SCALE_FACTOR * this.y,
            MINIMAP_SCALE_FACTOR * this.x + Math.cos(this.rotationAngle) * 30,
            MINIMAP_SCALE_FACTOR * this.y + Math.sin(this.rotationAngle) * 30
        )
    }
}

class Ray {
    constructor(rayAngle) {
        this.rayAngle = normalizeAngle(rayAngle);
        this.wallHitPoint = 0;
        this.distance = 0;

        this.isRayFacingDown = this.rayAngle > 0 && this.rayAngle < Math.PI;
        this.isRayFacingUp = !this.isRayFacingDown;

        this.isRayFacingRight = this.rayAngle < Math.PI * 0.5 || this.rayAngle > Math.PI * 1.5;
        this.isRayFacingLeft = !this.isRayFacingRight;
    }
    horizontalInterception() {
        let xintercept, yintercept = 0;
        yintercept = Math.floor(player.y / TILE_SIZE) * TILE_SIZE;
        yintercept += this.isRayFacingDown ? TILE_SIZE : 0;

        let opositeSide = yintercept - player.y;

        xintercept = player.x + opositeSide / Math.tan(this.rayAngle);

        return { xintercept, yintercept }
    }
    horizontalStep(xintercept, yintercept) {
        let foundWallHit = false;
        let xstep, ystep = 0;
        let nextXTouch = xintercept;
        let nextYTouch = yintercept;

        ystep = TILE_SIZE;
        ystep *= this.isRayFacingUp ? -1 : 1;

        xstep = TILE_SIZE / Math.tan(this.rayAngle);
        xstep *= (this.isRayFacingLeft && xstep > 0) ? -1 : 1;
        xstep *= (this.isRayFacingRight && xstep < 0) ? -1 : 1;

        while (!foundWallHit) {
            // fill("red");
            // stroke("red");
            // circle(
            //     nextXTouch,
            //     nextYTouch,
            //     3
            // )
            if (grid.hasWallAt(nextXTouch, nextYTouch - (this.isRayFacingUp ? 1 : 0))) {
                foundWallHit = true;
            } else {
                nextXTouch += xstep;
                nextYTouch += ystep;
            }
        }
        return { x: nextXTouch, y: nextYTouch };
    }
    verticalInterception() {
        let xintercept, yintercept = 0;
        xintercept = Math.floor(player.x / TILE_SIZE) * TILE_SIZE;
        xintercept += this.isRayFacingRight ? TILE_SIZE : 0;

        let adjecentSide = xintercept - player.x;

        yintercept = player.y + adjecentSide * Math.tan(this.rayAngle);
        return { xintercept, yintercept }
    }
    verticalStep(xintercept, yintercept) {
        let foundWallHit = false;
        let xstep, ystep = 0;
        let nextXTouch = xintercept;
        let nextYTouch = yintercept;

        xstep = TILE_SIZE;
        xstep *= this.isRayFacingLeft ? -1 : 1;

        ystep = TILE_SIZE * Math.tan(this.rayAngle);
        ystep *= (this.isRayFacingUp && ystep > 0) ? -1 : 1;
        ystep *= (this.isRayFacingDown && ystep < 0) ? -1 : 1;

        while (!foundWallHit) {
            // fill("green");
            // stroke("green");
            // circle(
            //     nextXTouch,
            //     nextYTouch,
            //     3
            // )
            if (grid.hasWallAt(nextXTouch - (this.isRayFacingLeft ? 1 : 0), nextYTouch)) {
                foundWallHit = true;
            } else {
                nextXTouch += xstep;
                nextYTouch += ystep;
            }
        }
        return { x: nextXTouch, y: nextYTouch };
    }
    cast() {
        const horizontal = this.horizontalInterception();
        const horizontalWallHit = this.horizontalStep(horizontal.xintercept, horizontal.yintercept);
        const vertical = this.verticalInterception();
        const verticalWallHit = this.verticalStep(vertical.xintercept, vertical.yintercept);

        const horizontalDistance = distanceBetweenPoints(player.x, player.y, horizontalWallHit.x, horizontalWallHit.y);
        const verticalDistance = distanceBetweenPoints(player.x, player.y, verticalWallHit.x, verticalWallHit.y);

        this.wasHitVertical = verticalDistance < horizontalDistance;
        this.wallHitPoint = !this.wasHitVertical ? horizontalWallHit : verticalWallHit;
        this.distance = this.wasHitVertical ? verticalDistance : horizontalDistance;
    }
    render() {
        stroke("rgba(255, 0, 0, 0.3)");
        line(
            MINIMAP_SCALE_FACTOR * player.x,
            MINIMAP_SCALE_FACTOR * player.y,
            MINIMAP_SCALE_FACTOR * this.wallHitPoint.x,
            MINIMAP_SCALE_FACTOR * this.wallHitPoint.y
        );
    }
}

var grid = new Map();
var player = new Player();
var rays = [];

function distanceBetweenPoints(x1, y1, x2, y2) {
    const x = Math.pow(x2 - x1, 2);
    const y = Math.pow(y2 - y1, 2);
    return Math.sqrt(x + y);
}

function normalizeAngle(angle) {
    angle = angle % (Math.PI * 2);
    if (angle < 0) {
        angle = (2 * Math.PI) + angle;
    }
    return angle;
}

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
    // start first ray subtracting half of the FOV
    var rayAngle = player.rotationAngle - (FOV_ANGLE / 2);

    rays = [];

    // loop all columns casting the rays
    for (var i = 0; i < NUM_RAYS; i++) {
        // for (var i = 0; i < 1; i++) {
        var ray = new Ray(rayAngle);
        ray.cast();
        rays.push(ray);

        rayAngle += FOV_ANGLE / NUM_RAYS;
    }
}

function update() {
    player.update();
    castAllRays();
}

function render3DProjectedWalls() {
    for (var i = 0; i < NUM_RAYS; i++) {
        const ray = rays[i];

        const fixedRaydistance = ray.distance * Math.cos((ray.rayAngle - player.rotationAngle));

        //Projected wall height
        const wallStripHeight = TILE_SIZE / fixedRaydistance * distanceProjectedPlane;

        //Draw a rectangle with the calculated height
        // const alpha = 170 / fixedRaydistance;
        const alpha = 1;
        const colorIntencity = ray.wasHitVertical ? 255 : 180;

        fill(`rgba(${colorIntencity}, ${colorIntencity}, ${colorIntencity}, ${alpha})`);
        noStroke();
        rect(
            i * WALL_STRIP_WIDTH,
            WINDOW_HEIGHT / 2 - wallStripHeight / 2,
            WALL_STRIP_WIDTH,
            wallStripHeight
        )
    }
}

function draw() {
    clear('#212121');
    update();

    render3DProjectedWalls();

    grid.render();
    for (var i = 0; i < NUM_RAYS; i++) {
        const ray = rays[i];
        ray.render();
    }
    player.render();
}
