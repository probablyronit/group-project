#include "dino.h"

Dino::Dino()
    : velocityY(0), onGround(true)
{
    dinoRect = QRect(50, GROUND_LEVEL - 50, 40, 50);
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
        }
    }
}

void Dino::jump() {
    if (onGround) {
        velocityY = DINO_JUMP_STRENGTH;
        onGround = false;
    }
}

void Dino::reset() {
    velocityY = 0;
    onGround = true;
    int height = dinoRect.height();
    dinoRect.moveTo(50, GROUND_LEVEL - height);
}

void Dino::draw(QPainter &painter) {
    painter.fillRect(dinoRect, Qt::white);
}
