#ifndef BULLET_H
#define BULLET_H

#include "raylib.h"

#include <vector>

class GameWindow;
class Bullet
{
public:
    Bullet(Color color, int owner, Vector2 position, Vector2 speed);

    bool Update(const GameWindow &window, float deltaTime);
    void Draw() const;
    int GetOwner() const { return owner; }
    friend Vector2 GetPosition( const Bullet &bullet) { return bullet.position; }

private:
    Color color = RED;
    int owner;
    Vector2 position;
    Vector2 speed;

    float lifeTime = 2.0f;
};

using Bullets = std::vector<Bullet>;
void Draw( const Bullets &bullets);
void Update( Bullets &bullets, const GameWindow &window, float deltaTime);


#endif // BULLET_H
