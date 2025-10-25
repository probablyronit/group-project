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
    }
    else { // Bird
        width = 50;
        height = 30;
        // Random flight height
        int flightLevel = QRandomGenerator::global()->bounded(3);
        switch (flightLevel) {
        case 0: y = groundY - height; break;           // low
        case 1: y = groundY - 80 - height / 2; break;  // mid
        case 2: y = groundY - 140 - height / 2; break; // high
        }
    }

    rect = QRect(startX, y, width, height);
}

void Obstacle::update(int speed) {
    // Move the obstacle left each frame
    rect.translate(-speed, 0);
}

void Obstacle::draw(QPainter &painter) {
    if (type == Cactus) {
        painter.setBrush(Qt::green);
        painter.setPen(Qt::NoPen);
        painter.drawRect(rect);
    } else { // Bird
        painter.setBrush(Qt::gray);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(rect);
    }
}
