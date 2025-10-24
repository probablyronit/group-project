#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QRect>
#include <QList>
#include <QKeyEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // Event handlers for painting and keyboard input
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    // This function is our main game loop, called by the timer
    void gameLoop();

private:
    void resetGame();      // Resets the game to its initial state
    void spawnObstacle();  // Creates a new cactus obstacle
    void updateGame();     // Updates all game logic for one frame

    Ui::MainWindow *ui;
    QTimer *timer;

    // --- Game Variables ---
    enum GameState { Ready, Playing, GameOver };
    GameState gameState;

    QRect dino;
    int dinoVelocityY;
    bool isGrounded;

    QList<QRect> obstacles;
    int gameSpeed;
    int score;
    int highScore;

    // Constants for game physics and appearance
    const int GROUND_LEVEL = 350;
    const int DINO_JUMP_STRENGTH = -18;
    const int GRAVITY = 1;
};
#endif // MAINWINDOW_H
