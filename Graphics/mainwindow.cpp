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
    dino=Dino(QRect(50, GROUND_LEVEL - 50, 30, 50),GRAVITY,DINO_JUMP_STRENGTH,GROUND_LEVEL);
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
    obstacles.clear();
    spawnObstacle();
    timer->stop();
    dino.reset();
    update();
}

void MainWindow::gameLoop() {
    if (gameState == Playing) {
        updateGame();
    }
    update();
}

void MainWindow::updateGame() {
    dino.update();
    for (int i = 0; i < obstacles.size(); ++i) {
        obstacles[i].update(gameSpeed);
        if (dino.intersects(obstacles[i]) && !dino.isInvincible()) {
            dino.decLives();
            dino.setInvincibility(true);
            dino.invincibilityTimeOut();//set invincible false after timeout
            dino.takeDamgage();//animation
        }
    }
    if(dino.gameover()){
        gameState = GameOver;
        timer->stop();
        if (score > highScore) {
            highScore = score;
        }
        return;
    }
    if (!obstacles.isEmpty() && obstacles.first().isOffScreen()) {
        obstacles.removeFirst();
    }
    if (obstacles.last().getRect().right() < width() - QRandomGenerator::global()->bounded(250, 500)) {
        spawnObstacle();
    }
    score++;
    if(score%LEVEL_INC_SCORE_GAP==0){
        increaseLevel();
    }
}

void MainWindow::spawnObstacle() {
    obstacles.append(Obstacle(width(),GROUND_LEVEL,getProbability(Obstacle::BIRD_PROB)?Obstacle::Bird:Obstacle::Cactus));
}

void MainWindow::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), Qt::darkGray);
    painter.setPen(QPen(Qt::white, 2));
    dino.draw(painter);
    painter.drawLine(0, GROUND_LEVEL, width(), GROUND_LEVEL);
    painter.setBrush(Qt::green);
    painter.setPen(Qt::NoPen);
    for (Obstacle &obstacle : obstacles) {
        obstacle.draw(painter);
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
            dino.jump();
        } else {
            resetGame();
            gameState = Playing;
            timer->start(16);
        }
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

bool MainWindow::getProbability(float prob){
    return QRandomGenerator::global()->generateDouble()<prob;
}

void MainWindow::increaseLevel(){
    gameSpeed++;
}
