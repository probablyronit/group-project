#ifndef DINO_H
#define DINO_H

#include <QRect>
#include <QPainter>
#include <QTimer>
#include "obstacle.h"
class Dino {

public:
    Dino();
    Dino(QRect dinoRect,float GRAVITY, float DINO_JUMP_STRENGTH, int GROUND_LEVEL, float velocityY=0, bool onGround=true);

    // --- Core functions ---
    void update();                // called every frame by your main game loop
    void draw(QPainter &painter); // paints the dino
    void jump();                  // triggers jump if grounded
    void reset();                 // resets position & velocity
    bool intersects(Obstacle obstacle){return dinoRect.intersects(obstacle.getRect());};
    void takeDamgage();
    void invincibilityTimeOut();
    void resetLives();
    void decLives();
    // --- Simple functions ---
    inline QRect getRect() const { return dinoRect; }
    inline bool isOnGround() const { return onGround; }
    inline bool gameover(){return lives<=0;}
    inline bool isInvincible(){return invincible;}
    void setInvincibility(bool arg){invincible=arg;};


private:
    QRect dinoRect;    // hitbox rectangle (x, y, w, h)
    float velocityY;   // vertical speed (for jumping)
    bool onGround;     // grounded state
    bool doubleJump=true;// for double jump mechanics
    bool invincible=false;
    QList<QPolygon> lifeShapeList;
    // constants : sync with mainwindow contants in constructor
    float GRAVITY = 1.f;
    float DINO_JUMP_STRENGTH = -18.0f;
    int GROUND_LEVEL = 350;  // Y-position of the ground
    int lives=3;
    static const int INVINCIBLE_TIMESPAN_MS = 1000; // once hit, a short span of time for invincibility
    QColor DINO_CLR = Qt::white;
    constexpr static QColor DAMAGE_CLR = QColor(255, 80, 80);
};

#endif // DINO_H
