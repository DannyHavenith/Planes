#ifndef PLANE_H
#define PLANE_H

#include "raylib.h"

#include <array>
#include <cstdint>
#include <string_view>

struct GameWindow;
using Angle256 = std::uint8_t;
class Plane
{
public:
    Plane( std::string_view skin, Vector2 position, float speed = 200, std::uint8_t angle = 0);

    std::uint8_t DeltaPitch(std::int8_t angle);
    std::uint8_t DeltaRoll(std::int8_t angle);
    std::uint8_t GetRoll() const  { return roll; }
    std::uint8_t GetPitch() const { return pitch; }
    Vector2 GetPosition() const { return position; }
    Vector2 GetSpeedVector() const { return speedVector; }
    float GetSpeed() const { return speed; }

    void Draw( const GameWindow &window) const;
    void Update( const GameWindow &window, float deltaTime);

private:
    using PlaneTextures = std::array<Texture2D, 16>;

    Vector2 position;
    Vector2 positionOffset = { 0, 0 };
    Vector2 speedVector = { 0, 0 };
    float speed;
    std::uint8_t pitch;
    Angle256     roll = 0;
    PlaneTextures textures;

    PlaneTextures LoadPlaneTextures( std::string_view skin);
};

#endif // PLANE_H
