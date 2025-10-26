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
    dinoShape = {
        // Start at the tip of the tail (now on the left)
        QPoint(50, GROUND_LEVEL - 20),

        // Top of the back and neck
        QPoint(65, GROUND_LEVEL - 20),
        QPoint(65, GROUND_LEVEL - 35),
        QPoint(70, GROUND_LEVEL - 50), // Back of head

        // Head and snout
        QPoint(90, GROUND_LEVEL - 50), // Top of head
        QPoint(90, GROUND_LEVEL - 40), // Tip of upper jaw

        // Open mouth (inner part)
        QPoint(75, GROUND_LEVEL - 40),
        QPoint(75, GROUND_LEVEL - 35),
        QPoint(90, GROUND_LEVEL - 35), // Tip of lower jaw

        // Chin and neck
        QPoint(90, GROUND_LEVEL - 30),
        QPoint(80, GROUND_LEVEL - 30), // Base of neck

        // Front leg (now the right-most leg)
        QPoint(80, GROUND_LEVEL - 15), // Top of front leg
        QPoint(85, GROUND_LEVEL - 15), // Front of front leg
        QPoint(85, GROUND_LEVEL),      // Bottom-front of front foot
        QPoint(80, GROUND_LEVEL),      // Bottom-back of front foot

        // Gap between legs (moves up, across, and down)
        QPoint(80, GROUND_LEVEL - 10), // Up from front leg
        QPoint(70, GROUND_LEVEL - 10), // Across to back leg
        QPoint(70, GROUND_LEVEL),      // Down to back foot

        // Back leg (now the left-most leg)
        QPoint(65, GROUND_LEVEL),      // Back of back foot
        QPoint(65, GROUND_LEVEL - 15), // Up the back of the leg

        // Underside of tail
        QPoint(50, GROUND_LEVEL - 15)

        // Polygon closes by connecting back to (50, GROUND_LEVEL - 20)
    };
}
void Dino::update() {
    // Apply GRAVITY if in the air
    if (!onGround) {
        velocityY += GRAVITY;
        dinoRect.translate(0, static_cast<int>(velocityY));
        dinoShape.translate(0, static_cast<int>(velocityY));

        // Hit the ground? Stop falling!
        if (dinoRect.bottom() >= GROUND_LEVEL) {
            dinoRect.moveBottom(GROUND_LEVEL);
            movePolygonBottom(dinoShape,dinoRect.bottom());
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
    dinoRect.moveTo(60, GROUND_LEVEL - height);
    movePolygonBottom(dinoShape,GROUND_LEVEL);
}

void Dino::draw(QPainter &painter) {
    // painter.fillRect(dinoRect, DINO_CLR);//dont show collision shape
    QBrush brushClr = painter.brush();
    painter.setBrush(DINO_CLR);
    painter.setPen(Qt::black);
    painter.drawPolygon(dinoShape);
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

void Dino::movePolygonBottom(QPolygon &poly, int newBottomY)
{
    if (poly.isEmpty()) return;

    // Find current bottom (max y)
    int currentBottom = poly.first().y();
    for (const QPoint &p : poly)
        currentBottom = std::max(currentBottom, p.y());

    // Compute offset
    int dy = newBottomY - currentBottom;

    // Move polygon vertically
    poly.translate(0, dy);
}


