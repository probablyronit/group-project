#include "obstacle.h"

Obstacle::Obstacle(int startX, int groundY, Type type)
    : type(type)
{
    int width, height, y;

    if (type == Cactus) {
        // Random cactus height & size variety
        width = 25 + QRandomGenerator::global()->bounded(20);
        height = 40 + QRandomGenerator::global()->bounded(20);
        y = groundY - height;
        OBS_CLR=Qt::green;
    }
    else { // Bird
        width = 50;
        height = 30;
        // Random flight height
        int flightLevel = QRandomGenerator::global()->bounded(3)+1;
        switch (flightLevel) {
        case 1: y = groundY - 80 - height / 2; break;// mid
        case 2: y = groundY - 140 - height / 2; break;// high
        case 3: y = groundY - -200 - height / 2; break;// low
        }
        OBS_CLR=Qt::gray;
    }
    rect = QRect(startX, y, width, height);
}

void Obstacle::update(int speed) {
    // Move the obstacle left each frame
    rect.translate(-speed, 0);
}

void Obstacle::draw(QPainter &painter) {
    painter.setBrush(OBS_CLR);
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect);
}
