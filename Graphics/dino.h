#ifndef DINO_H
#define DINO_H

#include <QRect>
#include <QPainter>
#include "obstacle.h"
class Dino {
public:
    Dino();
    Dino(QRect dinoRect,float GRAVITY, float DINO_JUMP_STRENGTH, int GROUND_LEVEL, float velocityY=0, bool onGround=true)
        : dinoRect(dinoRect),
        velocityY(velocityY),
        onGround(onGround),
        GRAVITY(GRAVITY),
        DINO_JUMP_STRENGTH(DINO_JUMP_STRENGTH),
        GROUND_LEVEL(GROUND_LEVEL){
    }

    // --- Core functions ---
    void update();                // called every frame by your main game loop
    void draw(QPainter &painter); // paints the dino
    void jump();                  // triggers jump if grounded
    void reset();                 // resets position & velocity
    bool intersects(Obstacle obstacle){return dinoRect.intersects(obstacle.getRect());};
    // --- State info ---
    QRect getRect() const { return dinoRect; }
    bool isOnGround() const { return onGround; }

private:
    QRect dinoRect;    // hitbox rectangle (x, y, w, h)
    float velocityY;   // vertical speed (for jumping)
    bool onGround;     // grounded state

    // constants (you can tune)
    float GRAVITY = 0.9f;
    float DINO_JUMP_STRENGTH = -15.0f;
    int GROUND_LEVEL = 350;  // Y-position of the ground
};

#endif // DINO_H
