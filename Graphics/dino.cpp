#include "dino.h"

Dino::Dino()
    : velocityY(0), onGround(true)
{
    dinoRect = QRect(50, GROUND_LEVEL - 50, 40, 50);
}
Dino::Dino(QRect dinoRect,float GRAVITY, float DINO_JUMP_STRENGTH, int GROUND_LEVEL, float velocityY, bool onGround)
    : dinoRect(dinoRect),
    velocityY(velocityY),
    onGround(onGround),
    GRAVITY(GRAVITY),
    DINO_JUMP_STRENGTH(DINO_JUMP_STRENGTH),
    GROUND_LEVEL(GROUND_LEVEL){
}
void Dino::update() {
    // Apply GRAVITY if in the air
    if (!onGround) {
        velocityY += GRAVITY;
        dinoRect.translate(0, static_cast<int>(velocityY));

        // Hit the ground? Stop falling!
        if (dinoRect.bottom() >= GROUND_LEVEL) {
            dinoRect.moveBottom(GROUND_LEVEL);
            velocityY = 0;
            onGround = true;
            doubleJump=true;
        }
    }
}

void Dino::jump() {
    if (onGround) {
        velocityY = DINO_JUMP_STRENGTH;
        onGround = false;
    } else if(doubleJump){
        velocityY = DINO_JUMP_STRENGTH;
        doubleJump=false;
    }
}

void Dino::reset() {
    velocityY = 0;
    onGround = true;
    DINO_CLR=Qt::white;
    lives=3;
    resetLives();
    int height = dinoRect.height();
    dinoRect.moveTo(50, GROUND_LEVEL - height);
}

void Dino::draw(QPainter &painter) {
    painter.fillRect(dinoRect, DINO_CLR);
    QBrush brushClr = painter.brush();
    painter.setBrush(Qt::red);
    for(QPolygon life:lifeShapeList){
        painter.drawPolygon(life);
    }
    painter.setBrush(brushClr);
}

void Dino::invincibilityTimeOut(){
    QTimer::singleShot(Dino::INVINCIBLE_TIMESPAN_MS,[&](){
        this->invincible=false;
    });
}

void Dino::decLives(){
    lives--;
    lifeShapeList.pop_back();
}

void Dino::takeDamgage(){
    if(!invincible){
        DINO_CLR=Qt::white;
        return;
    }
    if(DINO_CLR!=DAMAGE_CLR){
        DINO_CLR = DAMAGE_CLR;
    } else {
        DINO_CLR = Qt::white;
    }
    int TIME_SPAN = INVINCIBLE_TIMESPAN_MS/4;
    QTimer::singleShot(TIME_SPAN,[&](){
        takeDamgage();
    });
}

void Dino::resetLives(){
    lifeShapeList.clear();
    int size = 20;   // triangle size
    int gap = 10;    // space between triangles
    int startX = 10; // starting position
    int startY = 10;
    for (int i = 0; i < lives; ++i) {
        int x = startX + i * (size + gap);
        // Triangle points (pointing upwards)
        QPolygon triangle;
        triangle << QPoint(x, startY)
                 << QPoint(x + size / 2, startY + size)
                 << QPoint(x + size, startY);
        lifeShapeList.append(triangle);
    }
    return;
}
