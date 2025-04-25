#include "Bullet.h"
#include "DrawingUtilities.h"
#include "GameWindow.h"
#include "VectorMath.h"


Bullet::Bullet(Color color, int owner, Vector2 position, Vector2 speed)
    : color(color), owner(owner), position(position), speed(speed) {}

bool Bullet::Update(const GameWindow &window, float deltaTime)
{
    position += speed * deltaTime;
    position = {
        Wrap(position.x, static_cast<float>(window.width)),
        Wrap(position.y, static_cast<float>(window.height))};
    return (lifeTime -= deltaTime) > 0;
}

void Bullet::Draw( const GameWindow &) const
{
    DrawCircleV(position, 4, color);
}

void Update( Bullets &bullets, const GameWindow &window, float deltaTime)
{
    for (auto it = bullets.begin(); it != bullets.end();)
    {
        if (!it->Update(window, deltaTime))
        {
            it = bullets.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
