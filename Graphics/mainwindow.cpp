#include "mainwindow.h"
#include "ui_mainwindow.h"

// Qt includes
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QDebug>       // For printing to console
#include <QKeyEvent>    // For keyboard input
#include <QFont>        // For drawing score/text

// C++ Standard Library includes
#include <cstdlib>      // For rand()
#include <ctime>        // For srand()
#include <cmath>        // For math functions
#include <numeric>      // (From original code)
#include <vector>       // (From original code)

// Define static weapon velocity
int Weapon::weapon_velocity = 2; // moves rightwards (grid units per frame)

// --- NEW Game Variables (pretend these are in mainwindow.h) ---
int jumpCount;
const int maxJumps = 2; // For double jump
int base_obstacle_speed;
bool isPaused; // --- NEW for Pause ---
bool isFlying; // --- NEW for Cheat ---

// --- NEW Staircase Variables ---
bool staircaseMode;
bool staircaseTriggered; // To ensure it only happens once per game
std::vector<QPoint> terrainBlocks; // For stairs
int staircaseTimer;
int current_stair_y;

// fire ball
const int FIREBALL_WIDTH=5;
bool fireBallsAdded;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) // <-- FIX #1: Was "new Ui_MainWindow"
{
    ui->setupUi(this);
    frame_width = ui->frame->width();
    frame_height = ui->frame->height();

    // Setup colors
    fill1 = QColor(35, 176, 106); // Dino
    obstacleColor = QColor(200, 50, 50); // Obstacle
    fill2 = QColor(18, 141, 21);
    fill3 = QColor(20, 4, 41);

    // Setup grid size and physics scaled to that size
    gap = 5;
    double scale_factor = 20.0 / double(gap);
    gravity = 0.1 * scale_factor;
    jump_power = -1.2 * scale_factor; // Shorter jump

    // --- MODIFIED: Speed ---
    base_obstacle_speed = (int)round(scale_factor); // Store the base speed
    obstacle_speed = base_obstacle_speed; // Set the current speed

    currentDrawingMode = Normal;

    // --- MODIFIED: Define a larger Dino Shape ---
    dinoShape.clear(); // Clear the old shape first
    // New, larger shape relative to (0,0) as the front foot

    // Head
    dinoShape.push_back(QPoint(1, -6));
    dinoShape.push_back(QPoint(0, -6));
    dinoShape.push_back(QPoint(1, -5));
    dinoShape.push_back(QPoint(0, -5));
    // Neck
    dinoShape.push_back(QPoint(0, -4));
    dinoShape.push_back(QPoint(0, -3));
    // Body
    dinoShape.push_back(QPoint(-3, -2));
    dinoShape.push_back(QPoint(-2, -2));
    dinoShape.push_back(QPoint(-1, -2));
    dinoShape.push_back(QPoint(0, -2));
    // Tail/Body
    dinoShape.push_back(QPoint(-4, -1));
    dinoShape.push_back(QPoint(-3, -1));
    dinoShape.push_back(QPoint(-2, -1));
    dinoShape.push_back(QPoint(-1, -1));
    // Legs
    dinoShape.push_back(QPoint(-2, 0)); // Back foot
    dinoShape.push_back(QPoint(0, 0));  // Front foot

    // Initialize game world variables
    srand(time(NULL));
    min_x = to_grid(0, 0).x();
    max_x = to_grid(frame_width, 0).x();
    world_width = max_x - min_x;
    ground_y = to_grid(0, frame_height * 0.75).y();
    dino_x = min_x + 10;

    // Connect original app signals
    connect(ui->frame, SIGNAL(Mouse_Pos()), this, SLOT(Mouse_Pressed()));
    connect(ui->frame, SIGNAL(sendMousePosition(QPoint&)), this, SLOT(showMousePosition(QPoint&)));

    // CRITICAL: Create the timer *before* calling restartGame()
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &MainWindow::gameLoop);

    // Set up the game to be on the "Game Over" screen
    restartGame(); // Set default values
    isGameOver = true; // Override to start in this mode
    gameTimer->stop(); // Stop the timer

    drawGame(); // Draw the initial "Press Space" screen
}

MainWindow::~MainWindow(){
    delete ui;
}

// --- Empty Stubs for Original App ---
void MainWindow::Mouse_Pressed() {}

void MainWindow::showMousePosition(QPoint &pos) {
    (void)pos; // <-- FIX #2: Prevents "unused parameter" warning
}


// --- Grid Helper Functions ---
QPoint MainWindow::to_grid(int curr_x, int curr_y){
    double rel_x = curr_x - frame_width/2, rel_y = curr_y - frame_height/2;
    rel_x /= (double)gap, rel_y /= (double)gap;
    return QPoint((int)round(rel_x), (int)round(rel_y));
}

QPoint MainWindow::from_grid(int grid_x, int grid_y) {
    double rel_x = grid_x * gap;
    double rel_y = grid_y * gap;

    int curr_x = rel_x + frame_width/2;
    int curr_y = rel_y + frame_height/2;

    return QPoint(curr_x, curr_y);
}

// Draws a single block to a painter (flicker-free)
void MainWindow::draw_grid_box(QPainter &painter, int x, int y, QColor c) {
    QPoint mid = from_grid(x, y);
    int mid_x = mid.x(), mid_y = mid.y();
    painter.setPen(Qt::NoPen);
    painter.setBrush(c);
    QRect rect(mid_x-gap/2, mid_y-gap/2, gap, gap);
    painter.drawRect(rect);
}

// Draws the background grid (currently not called)
void MainWindow::draw_grid(QPainter &painter){
    painter.setPen(QPen(Qt::black, 1));
    int centerX = frame_width / 2;
    int centerY = frame_height / 2;
    for(int i = centerX; i <= frame_width; i += gap)
        painter.drawLine(QPoint(i, 0), QPoint(i, frame_height));
    for(int i = centerX - gap; i >= 0; i -= gap)
        painter.drawLine(QPoint(i, 0), QPoint(i, frame_height));
    for(int i = centerY; i <= frame_height; i += gap)
        painter.drawLine(QPoint(0, i), QPoint(frame_width, i));
    for(int i = centerY - gap; i >= 0; i -= gap)
        painter.drawLine(QPoint(0, i), QPoint(frame_width, i));
}

// --- NEW: Draw Terrain Blocks (Stairs) updated in drawGame() ---
void MainWindow::updateStaircase() {
    staircaseTimer++;

    // 1. Move existing blocks left
    for (size_t i = 0; i < terrainBlocks.size(); ++i) {
        terrainBlocks[i].setX(terrainBlocks[i].x() - obstacle_speed);
        if (terrainBlocks[i].x() < min_x - 5) {
            terrainBlocks.erase(terrainBlocks.begin() + i);
            i--;
        }
    }

    // 2. Define phases by timer
    int rising_duration = 100; // 100 frames of rising
    int flat_duration = 300;   // 300 frames of flat
    int falling_duration = 100; // 100 frames of falling
    int step_rate = 10; // New block every 10 frames

    // 3. Spawn new blocks based on phase
    if (staircaseTimer % step_rate == 0) {
        if (staircaseTimer < rising_duration) {
            // Phase 1: Rising
            current_stair_y--;
            terrainBlocks.push_back(QPoint(max_x, current_stair_y));
        } else if (staircaseTimer < rising_duration + flat_duration) {
            // Phase 2: Flat
            terrainBlocks.push_back(QPoint(max_x, current_stair_y));
        } else if (staircaseTimer < rising_duration + flat_duration + falling_duration) {
            // Phase 3: Falling
            current_stair_y++;
            terrainBlocks.push_back(QPoint(max_x, current_stair_y));
        }
    }

    // 4. End staircase mode after it's finished and all blocks are gone
    if (staircaseTimer >= rising_duration + flat_duration + falling_duration) {
        if (terrainBlocks.empty()) { // Wait for last block to disappear
            staircaseMode = false;
        }
    }
}


// --- Game Functions ---

void MainWindow::on_clear_clicked(){
    history.clear(); // From original app
    currentDrawingMode = Normal; // From original app
    restartGame(); // Reset the game
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    // --- MODIFIED: Pause, Fly, and Jump logic ---

    // Always allow pause/unpause, even on game over screen
    if (event->key() == Qt::Key_P) {
        if (!isGameOver) {
            isPaused = !isPaused;
            if (isPaused) {
                gameTimer->stop();
                drawGame(); // Redraw to show "PAUSED" text
            } else {
                gameTimer->start(33);
            }
        }
        return;
    }

    // Don't process other keys if paused
    if (isPaused) return;

    // Handle Space key
    if (event->key() == Qt::Key_Space) {
        if (isGameOver) {
            restartGame(); // Start a new game if it's over
            return;
        }

        if (isFlying) { // If flying, Space moves dino up
            dino_y_velocity = jump_power * 0.5; // Gentle boost up
        }
        else if (jumpCount < maxJumps) { // Allow double jump
            jumpCount++;
            isJumping = true; // Ensure gravity takes effect
            dino_y_velocity = jump_power; // Apply jump boost
        }
    }

    // Handle Fly Cheat
    if (event->key() == Qt::Key_F) {
        if (!isGameOver) {
            isFlying = !isFlying;
            if (isFlying) {
                isJumping = false; // Disable normal jump/gravity logic
                dino_y_velocity = 0; // Stop falling
            } else {
                isJumping = true; // Re-enable gravity
            }
            fireballCount=99;
        }
    }

    // --- NEW: Weapon spawn on Enter/Return ---
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (!isGameOver && !isPaused) {
            spawnWeapon();
        }
    }
}

void MainWindow::gameLoop() {
    // --- MODIFIED: Added Pause check ---
    if (isGameOver || isPaused) return; // Don't run logic if game is over or paused

    // Run all game logic
    if (staircaseMode) updateStaircase(); // --- NEW ---
    updateDino();

    updateWeapons(); // --- NEW: Update weapons before obstacles ---
    updateObstacles();
    checkAndHandleCollision();

    if (isInvincible) { // Tick down invincibility frames
        invincibilityTimer--;
        if (invincibilityTimer <= 0) {
            isInvincible = false;
        }
    }
    // parallax offsets (grid units) with wrap-around
    mountain1Offset -= mountain1Speed;
    mountain2Offset -= mountain2Speed;

    if (mountain1Offset <= -world_width) mountain1Offset += world_width;
    if (mountain2Offset <= -world_width) mountain2Offset += world_width;

    if(score%15==0){
        haveShield=true;
    }
    drawGame(); // Redraw the screen

    if (lives <= 0 && !isGameOver) { // Check for game over condition
        gameOver();
    }
}

void MainWindow::drawGame() {
    QPixmap pm(frame_width, frame_height);
    pm.fill(Qt::white);
    QPainter painter(&pm);

    // draw_grid(painter); // Grid is removed
    DrawBackground(painter);
    // Draw Ground
    for (int x = min_x; x <= max_x; ++x) {
        draw_grid_box(painter, x, ground_y, QColor(0, 0, 0));
    }

    // Draw Dino (with invincibility flicker)

    if (!isInvincible || (isInvincible && (invincibilityTimer % 10 < 5))) {
        for (const QPoint& part : dinoShape) {
            draw_grid_box(painter, dino_x + part.x(), dino_y + part.y(), fill1);
        }
    }
    if(haveShield){
        drawShield(painter);
    }
        // Draw Obstacles (skip destroyed)
    for (const Obstacle& ob : obstacles) {
        if (ob.destroyed) continue; // don't draw destroyed obstacles
        for (int i = 0; i < ob.height; ++i) {
            draw_grid_box(painter, ob.x, ground_y - 1 - i, obstacleColor);
        }
    }

    // --- NEW: Draw Terrain Blocks (Stairs) ---
    for (const QPoint& block : terrainBlocks) {
        draw_grid_box(painter, block.x(), block.y(), QColor(100,100,100)); // Grey
    }

    // --- NEW: Draw Weapons ---
    for (const Weapon &w : weapons) {
        if (w.used) continue;
        // Visually make it look like a fireball (orange)
        draw_grid_box(painter, w.x, w.y, QColor(255,140,0));
        draw_grid_box(painter, w.x+1, w.y, QColor(255,165,0));
        draw_grid_box(painter, w.x+2, w.y, QColor(255,165,0));
        draw_grid_box(painter, w.x+3, w.y, QColor(255,165,0));
    }

    // Draw UI (Score & Lives & Fireballs)
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 16, QFont::Bold));
    painter.drawText(frame_width - 170, 40, QString("Score: %1").arg(score));
    painter.drawText(frame_width - 170, 70, QString("Lives: %1").arg(lives));
    painter.drawText(frame_width - 170, 100, QString("Fireballs: %1").arg(fireballCount));

    // --- NEW: Draw Paused Screen ---
    if (isPaused) {
        painter.setBrush(QColor(0,0,0,150)); // Semi-transparent black overlay
        painter.drawRect(rect());

        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 30, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter, "PAUSED");
    }

    // Draw Game Over Screen
    if (isGameOver) {
        painter.setBrush(QColor(0,0,0,150)); // Semi-transparent black overlay
        painter.drawRect(rect());

        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 30, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter, "GAME OVER");

        painter.setFont(QFont("Arial", 16));
        painter.drawText(rect().translated(0, 60), Qt::AlignCenter, "Press Space to Restart");
    }

    painter.end();
    ui->frame->setPixmap(pm);
}

// --- Game Logic ---

void MainWindow::updateDino() {
    // --- MODIFIED: Fly Cheat Logic ---
    if (isFlying) {
        // While flying, gravity is OFF.
        // We apply a little "air friction" so you don't
        // drift forever after tapping Space.
        dino_y_velocity *= 0.9; // Instead of adding gravity, we add drag.

        dino_y += dino_y_velocity;

        // Don't let dino fly off the top
        if (dino_y < to_grid(0,0).y() + 5) {
            dino_y = to_grid(0,0).y() + 5;
            dino_y_velocity = 0;
        }

        // Don't let dino fall through floor
        if (dino_y >= ground_y - 1) {
            dino_y = ground_y - 1;
            dino_y_velocity = 0;
        }
        return; // Skip normal jump logic
    }

    // Normal jump logic
    if (isJumping) {
        dino_y_velocity += gravity;
        dino_y += dino_y_velocity;

        // --- MODIFIED: New Landing Check ---
        int landing_y = ground_y - 1; // Default to ground
        bool onTerrain = false;

        // Check for landing on terrain blocks (only if falling)
        if (dino_y_velocity > 0) {
            for (const QPoint& block : terrainBlocks) {
                // Check if a block is right under the dino's feet
                // (dino_x-2, dino_x-1, dino_x) are the main X coords for feet
                if ( (block.x() == dino_x || block.x() == dino_x - 1 || block.x() == dino_x - 2) ) {
                    // This block is in the dino's X-path.
                    // Is it a valid landing spot? (i.e., we are about to pass it)
                    if (dino_y >= (block.y() - 1) && (dino_y - dino_y_velocity) < (block.y() - 1)) {
                        landing_y = block.y() - 1; // New "ground" is 1 block above terrain
                        onTerrain = true;
                        break; // Found our landing spot
                    }
                }
            }
        }

        if (dino_y >= landing_y) { // Check for landing
            dino_y = landing_y;
            isJumping = false;
            dino_y_velocity = 0;
            jumpCount = 0;
        }
    }
}

void MainWindow::updateObstacles() {
    // --- NEW: Stop spawning regular obstacles during staircase ---
    if (staircaseMode) {
        obstacle_spawn_timer = 0;
    }

    // FIX #3: Use size_t for loop to prevent signed/unsigned warning
    for (size_t i = 0; i < obstacles.size(); ++i) {
        // If obstacle is destroyed, still move it left so it goes off-screen
        obstacles[i].x -= obstacle_speed; // Move obstacle left

        if (!obstacles[i].destroyed && !obstacles[i].passed && obstacles[i].x < dino_x) { // Check for scoring
            obstacles[i].passed = true;
            score++;

            // --- NEW: Increase speed on score milestones ---
            if (score > 0 && score % 25 == 0) {
                obstacle_speed = base_obstacle_speed + (score / 25);
                qDebug() << "Speed Increased! New speed:" << obstacle_speed;
            }

            // --- NEW: Trigger staircase event ---
            if (score == 100 && !staircaseTriggered) {
                staircaseMode = true;
                staircaseTriggered = true; // Only happens once
                staircaseTimer = 0;
                current_stair_y = ground_y;
                qDebug() << "STAIRCASE MODE ACTIVATED!";
            }
        }

        if (obstacles[i].x < min_x - 5) { // Remove if off-screen
            obstacles.erase(obstacles.begin() + i);
            i--; // Decrement i because we just removed an element
        }
    }

    // Check if it's time to spawn a new one
    obstacle_spawn_timer++;

    // --- MODIFIED: Decreased obstacle gap ---
    int min_separation = 25; // Was 40
    int random_separation = 20; // Was 30

    if (obstacle_spawn_timer > (min_separation + (rand() % random_separation))) {
        spawnObstacle();
        obstacle_spawn_timer = 0;
    }
}

void MainWindow::spawnObstacle() {
    // --- NEW: Don't spawn if in staircase mode ---
    if (staircaseMode) return;

    // --- MODIFIED: Taller obstacles and multi-spawn logic ---

    // 1-in-4 chance for a multi-spawn (3 or 4 obstacles)
    if (score > 50 && (rand() % 4 == 0)) {
        int totalObstacles = 3 + (rand() % 2); // 3 or 4
        for (int i = 0; i < totalObstacles; ++i) {
            int height = (rand() % 3) + 2; // 2, 3, or 4 blocks high (doubled from 1-2)
            int x_pos = max_x + (i * (8 + (rand() % 4))); // Stagger them 8, 16, 24...
            obstacles.push_back(Obstacle{x_pos, height, false, false});
        }
    }
    else { // Normal single spawn
        int height = (rand() % 4) + 4; // 4, 5, 6, or 7 blocks high (doubled from 2-4)
        obstacles.push_back(Obstacle{max_x, height, false, false}); // Spawn at right edge
    }
}

void MainWindow::checkAndHandleCollision() {
    if (isInvincible) return; // Can't be hit if invincible

    for (const QPoint& part : dinoShape) { // Check each block of the dino
        int dino_part_x = dino_x + part.x();
        int dino_part_y = dino_y + part.y();

        // --- NEW: Check for collision with terrain ---
        if (staircaseMode) {
            for (const QPoint& block : terrainBlocks) {
                if (dino_part_x == block.x() && dino_part_y == block.y()) {
                    // Direct collision with a stair block
                    lives--;
                    isInvincible = true;
                    invincibilityTimer = 50;
                    qDebug() << "Hit a stair block!";
                    return; // Only one hit
                }
            }
        }

        // FIX #3: Use size_t for loop to prevent signed/unsigned warning
        for (size_t i = 0; i < obstacles.size(); ++i) { // Against each obstacle
            const Obstacle& ob = obstacles[i];

            if (ob.destroyed) continue; // ignore destroyed obstacles

            if (dino_part_x >= ob.x && dino_part_x < (ob.x + obstacle_speed)) { // X-axis overlap
                int ob_top_y = ground_y - ob.height;
                int ob_bottom_y = ground_y - 1;

                if (dino_part_y >= ob_top_y && dino_part_y <= ob_bottom_y) { // Y-axis overlap
                    // --- COLLISION! ---
                    isInvincible = true;
                    invincibilityTimer = 50;
                    if(haveShield){
                        haveShield=false;
                        score++;
                        continue;
                    }
                    lives--;
                    isInvincible = true;
                    invincibilityTimer = 50; // Set invincibility frames
                    // mark obstacle removed/destroyed
                    // (we erase it here to avoid double-collisions)
                    obstacles.erase(obstacles.begin() + i);
                    return; // Only handle one hit per frame
                }
            }
        }
    }
}

void MainWindow::DrawBackground(QPainter&painter){
    // --- BACKGROUND: Sky ---
    painter.fillRect(0, 0, frame_width, frame_height, QColor(135, 206, 235)); // light blue sky

    // ---- STATIC SUN (grid) ----
    for (int dx = -sunRadiusGrid; dx <= sunRadiusGrid; ++dx) {
        for (int dy = -sunRadiusGrid; dy <= sunRadiusGrid; ++dy) {
            if (dx*dx + dy*dy <= sunRadiusGrid*sunRadiusGrid) {
                draw_grid_box(painter, sunGridX + dx, sunGridY + dy, QColor(255, 210, 0));
            }
        }
    }

    // ---- FUNCTION: draw a triangular mountain layer (grid units) ----
    auto drawMountainLayer = [&](int layerOffsetGrid, int peakHeightGrid, double peakFrac, QColor color){
        int halfWidth = world_width / 2;               // how wide the mountain tile is (grid)
        int baseY = ground_y;                          // ground line in grid coords
        // draw two tiles so they seamlessly tile horizontally
        for (int tile = 0; tile <= 1; ++tile) {
            // peak X in grid coords (center fraction across world) + offset + tile*world_width
            int peakX = min_x + int(peakFrac * world_width) + layerOffsetGrid + tile * world_width;
            // for each column in the tile, compute triangular height
            for (int x = peakX - halfWidth; x <= peakX + halfWidth; ++x) {
                int dist = std::abs(x - peakX);
                // linear falloff from peak to edges
                int h = peakHeightGrid - (dist * peakHeightGrid) / halfWidth;
                if (h <= 0) continue;
                // draw vertical column of 'h' blocks
                for (int yy = 0; yy < h; ++yy) {
                    draw_grid_box(painter, x, baseY - 1 - yy, color);
                }
            }
        }
    };
    // far layer: subtle, taller peaks, slower movement
    drawMountainLayer(mountain1Offset, mountain1PeakHeight, 0.35, QColor(120,140,160));
    // near layer: stronger color, lower peaks, moves a bit faster
    drawMountainLayer(mountain2Offset, mountain2PeakHeight, 0.6, QColor(95,120,100));
}
void MainWindow::gameOver() {
    gameTimer->stop(); // Stop the game
    isGameOver = true;
    qDebug() << "GAME OVER! Final Score:" << score;
    drawGame(); // Draw the final "Game Over" text
}

void MainWindow::restartGame() {
    // Reset all game variables to their default state
    score = 0;
    lives = 3;
    isGameOver = false;
    isInvincible = false;
    invincibilityTimer = 0;
    isPaused = false; // --- NEW ---
    isFlying = false; // --- NEW ---

    obstacles.clear();
    terrainBlocks.clear(); // --- NEW ---
    staircaseMode = false; // --- NEW ---
    staircaseTriggered = false; // --- NEW ---
    staircaseTimer = 0; // --- NEW ---

    dino_y = ground_y - 1;
    dino_y_velocity = 0;
    isJumping = false;
    jumpCount = 0; // --- NEW: Reset jump count ---
    obstacle_spawn_timer = 0;
    obstacle_speed = base_obstacle_speed; // --- NEW: Reset speed ---

    weapons.clear();
    fireballCount = 3; // give player 3 fireballs at start
    // parallax init (in grid units)
    mountain1Offset = 0;
    mountain2Offset = 0;
    mountain1Speed = std::max(1, obstacle_speed / 2);
    mountain2Speed = std::max(1, obstacle_speed);

    // peak heights (tweak if needed)
    mountain1PeakHeight = 18;
    mountain2PeakHeight = 12;

    // static sun position in grid coordinates (relative to world)
    sunGridX = min_x + world_width / 4;
    sunGridY = ground_y - 18;
    sunRadiusGrid = 6;

    gameTimer->start(33); // Start the game loop
}

// --- NEW: Weapon handling ---
void MainWindow::spawnWeapon() {
    if (fireballCount <= 0) return; // no ammo

    // spawn at dino's head height (we'll use dino_y as "base" height)
    Weapon w;
    w.x = dino_x + 2; // a bit in front of the dino
    w.y = dino_y - 3;     // same height as the dino's reference point (front foot)
    w.used = false;
    weapons.push_back(w);
    fireballCount--;
    qDebug() << "Fired! Remaining:" << fireballCount;
}

void MainWindow::updateWeapons() {
    // move weapons and check collisions
    if(score%10==0){
        if(!fireBallsAdded){
            fireballCount+=3;
        }
        fireBallsAdded = true;
    }
    else {fireBallsAdded=false;}
    for (size_t i = 0; i < weapons.size(); ++i) {
        if (weapons[i].used) continue;

        weapons[i].x += Weapon::weapon_velocity; // move rightwards

        // if off-screen, mark used
        if (weapons[i].x > max_x + 5) {
            weapons[i].used = true;
            continue;
        }

        // check collision with obstacles
        for (size_t j = 0; j < obstacles.size(); ++j) {
            if (obstacles[j].destroyed) continue;

            // approximate width for obstacle as 3 grid unit
            if (weapons[i].x >= obstacles[j].x && weapons[i].x <= obstacles[j].x + FIREBALL_WIDTH) {
                int ob_top_y = ground_y - obstacles[j].height;
                int ob_bottom_y = ground_y - 1;

                if (weapons[i].y >= ob_top_y && weapons[i].y <= ob_bottom_y) {
                    // hit!
                    obstacles[j].destroyed = true; // set destroyed boolean as requested
                    obstacles[j].passed = true; // so it won't increment score later
                    this->score++;
                    weapons[i].used = true;
                    qDebug() << "Weapon hit obstacle at index" << j << "-> destroyed";
                    break; // weapon consumed
                }
            }
        }
    }

    // remove used/offscreen weapons to keep vector small
    for (size_t i = 0; i < weapons.size(); ++i) {
        if (weapons[i].used) {
            weapons.erase(weapons.begin() + i);
            i--;
        }
    }
}

// Draws a circular shield around the dino using its current position
void MainWindow::drawShield(QPainter &painter, int radiusGrid, QColor color)
{
    int centerX = dino_x;      // dino's current X position
    int centerY = dino_y - 3;  // slightly above feet, roughly center of body

    for (int dx = -radiusGrid; dx <= radiusGrid; ++dx) {
        for (int dy = -radiusGrid; dy <= radiusGrid; ++dy) {
            int distSq = dx * dx + dy * dy;
            int rSq = radiusGrid * radiusGrid;
            // only draw near the outer edge to form a circle outline
            if (distSq >= rSq - radiusGrid && distSq <= rSq + radiusGrid) {
                draw_grid_box(painter, centerX + dx, centerY + dy, color);
            }
        }
    }
}


