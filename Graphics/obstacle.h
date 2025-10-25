#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <QRect>
#include <QPainter>
#include <QRandomGenerator>

class Obstacle{
    friend class Dino;
public:
    constexpr static double BIRD_PROB=0.1;
    enum Type { Cactus, Bird }; // More can be added later

    // Constructor
    Obstacle(int startX, int groundY, Type type = Cactus);

    // Core game loop functions
    void update(int speed);       // move left
    void draw(QPainter &painter); // draw the obstacle

    // State & collision
    QRect getRect() const { return rect; }
    bool isOffScreen() const { return rect.right() < 0; }

    // get obstacle type
    Type getType() const { return type; }

    // spawn obstacle
    static void spawnObstacle(){
        //implemented in mainwnidow
    }

private:
    QRect rect;
    QColor OBS_CLR;
    Type type;
};

#endif // OBSTACLE_H
