// ===== mainwindow.h (updated) =====

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPoint>
#include <QColor>
#include <vector>
#include <numeric>
#include <cmath>
#include <QTimer>     // Required for game loop
#include <QKeyEvent>  // Required for keyboard input

// Forward declaration
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Struct for original drawing app (unused by game)
struct point_info {
    int x, y;
    QColor c;
};

// Struct to hold all data for an obstacle
struct Obstacle {
    int x;
    int height;
    bool passed; // For score tracking
    bool destroyed;
    Obstacle(int x,int height,bool passed,bool destroyed):x(x),height(height),passed(passed),destroyed(destroyed){}
};

// Weapon struct for fireball / dragon-ball
struct Weapon{
    static int weapon_velocity; // define in cpp
    int x;      // x position in grid coords
    int y;      // y position in grid coords (height)
    bool used;  // true when it hit something or went off-screen
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override; // Handles all keyboard input

private slots:
    void gameLoop(); // The main timer tick for game logic
    void on_clear_clicked(); // Clears screen and resets game

    // Stubs for your original drawing app
    void Mouse_Pressed();
    void showMousePosition(QPoint &pos);

private:
    Ui::MainWindow *ui;

    // UI, Frame, & Grid
    int frame_width, frame_height;
    int gap; // The size of one "pixel" in our game

    // Game State
    QTimer *gameTimer;
    bool isGameOver;
    bool isInvincible; // For flashing after being hit
    int invincibilityTimer;
    int score;
    int lives;

    // World
    int ground_y;
    int min_x, max_x; // Left and right edges of the screen in grid units
    int world_width;

    // Dino
    int dino_x, dino_y; // Dino's base position (front foot)
    std::vector<QPoint> dinoShape; // The blocks that make up the dino
    double dino_y_velocity;
    bool isJumping;
    bool haveShield;

    // Physics
    double gravity;
    double jump_power;
    int obstacle_speed;

    // Obstacles
    std::vector<Obstacle> obstacles;
    int obstacle_spawn_timer;

    // weapons
    std::vector<Weapon> weapons;
    int fireballCount; // number of fireballs the player currently has

    // background
    // parallax (grid units)
    int mountain1Offset = 0;   // offset in grid units (can be negative)
    int mountain2Offset = 0;
    int mountain1Speed = 1;    // grid units per frame
    int mountain2Speed = 2;
    int mountain1PeakHeight = 18; // height in grid blocks
    int mountain2PeakHeight = 12;

    // static sun (grid coords & radius)
    int sunGridX = 0;
    int sunGridY = 0;
    int sunRadiusGrid = 6;

    // Original Drawing App State
    QColor fill1, fill2, fill3, obstacleColor;
    std::vector<point_info> history;
    enum DrawingMode { Normal, SelectingPoints };
    DrawingMode currentDrawingMode;

    // --- Game Functions ---
    QPoint to_grid(int curr_x, int curr_y); // Converts window pixels to grid units
    QPoint from_grid(int grid_x, int grid_y); // Converts grid units to window pixels
    void draw_grid_box(QPainter &painter, int x, int y, QColor c); // Draws one grid-sized block
    void draw_grid(QPainter &painter); // Draws the background grid (currently unused)
    void DrawBackground(QPainter&painter);
    void restartGame(); // Resets all game variables and starts
    void drawGame(); // Draws the entire game state to the screen
    void updateDino(); // Handles dino's jump physics
    void updateObstacles(); // Moves, spawns, and scores obstacles
    void spawnObstacle(); // Creates a new obstacle
    void checkAndHandleCollision(); // Checks for hits and updates lives
    void gameOver(); // Stops the game and sets game over state
    void updateStaircase();

    // Weapon helpers
    void updateWeapons();
    void spawnWeapon();
    void drawShield(QPainter &painter, int radiusGrid=5, QColor color=Qt::blue);

};
#endif // MAINWINDOW_H
