#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(800, 400);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::gameLoop);
    highScore = 0;
    resetGame();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resetGame() {
    gameState = Ready;
    score = 0;
    gameSpeed = 5;

    dino = QRect(50, GROUND_LEVEL - 50, 40, 50);
    dinoVelocityY = 0;
    isGrounded = true;
    obstacles.clear();
    spawnObstacle();
    timer->stop();
    update();
}

void MainWindow::gameLoop() {
    if (gameState == Playing) {
        updateGame();
    }
    update();
}

void MainWindow::updateGame() {
    if (!isGrounded) {
        dinoVelocityY += GRAVITY;
        dino.moveTop(dino.top() + dinoVelocityY);
    }
    if (dino.bottom() >= GROUND_LEVEL) {
        dino.moveBottom(GROUND_LEVEL);
        dinoVelocityY = 0;
        isGrounded = true;
    }
    for (int i = 0; i < obstacles.size(); ++i) {
        obstacles[i].moveLeft(obstacles[i].left() - gameSpeed);
        if (dino.intersects(obstacles[i])) {
            gameState = GameOver;
            timer->stop();
            if (score > highScore) {
                highScore = score;
            }
            return;
        }
    }
    if (!obstacles.isEmpty() && obstacles.first().right() < 0) {
        obstacles.removeFirst();
    }
    if (obstacles.last().right() < width() - QRandomGenerator::global()->bounded(250, 500)) {
        spawnObstacle();
    }
    score++;
}

void MainWindow::spawnObstacle() {
    int obstacleHeight = QRandomGenerator::global()->bounded(30, 60);
    int obstacleWidth = 20;
    obstacles.append(QRect(width(), GROUND_LEVEL - obstacleHeight, obstacleWidth, obstacleHeight));
}

void MainWindow::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), Qt::darkGray);
    painter.setPen(QPen(Qt::white, 2));
    painter.drawLine(0, GROUND_LEVEL, width(), GROUND_LEVEL);
    painter.fillRect(dino, Qt::white);
    painter.setBrush(Qt::green);
    painter.setPen(Qt::NoPen);
    for (const QRect &obstacle : obstacles) {
        painter.drawRect(obstacle);
    }
    painter.setPen(Qt::white);
    painter.setFont(QFont("Consolas", 16));
    QRectF textArea = this->rect().adjusted(10, 10, -10, -10);
    QString hiScoreText = QString("HI: %1").arg(highScore, 5, 10, QChar('0'));
    QString currentScoreText = QString("SCORE: %1").arg(score, 5, 10, QChar('0'));
    painter.drawText(textArea, Qt::AlignTop | Qt::AlignRight, hiScoreText);
    textArea.translate(0, painter.fontMetrics().height());
    painter.drawText(textArea, Qt::AlignTop | Qt::AlignRight, currentScoreText);
    if (gameState == GameOver) {
        painter.setFont(QFont("Consolas", 30, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter, "GAME OVER\nPress Space to Restart");
    } else if (gameState == Ready) {
        painter.setFont(QFont("Consolas", 30, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter, "Press Space to Start");
    }
}
void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Space) {
        if (gameState == Playing) {
            if (isGrounded) {
                isGrounded = false;
                dinoVelocityY = DINO_JUMP_STRENGTH;
            }
        } else {
            resetGame();
            gameState = Playing;
            timer->start(16);
        }
    } else {
        QMainWindow::keyPressEvent(event);
    }
}
