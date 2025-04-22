#ifndef PLANE_H
#define PLANE_H

#include "Angle256.h"
#include "raylib.h"

#include <array>
#include <cstdint>
#include <string_view>

struct GameWindow;

class Plane
{
public:
    Plane( std::string_view skin, Vector2 position, float speed = 200, Angle256 angle = 0);

    Angle256 DeltaPitch(std::int8_t angle);
    Angle256 DeltaRoll(std::int8_t angle);
    Angle256 GetRoll() const  { return roll; }
    Angle256 GetPitch() const { return pitch; }
    Vector2 GetPosition() const { return position; }
    Vector2 GetSpeedVector() const { return speedVector; }
    float GetSpeed() const { return speed; }

    void Draw( const GameWindow &window) const;
    void Update( const GameWindow &window, float deltaTime);
    Rectangle GetBoundingBox() const;
    bool Collides( Vector2 point) const;

private:
    using PlaneTextures = std::array<Texture2D, 16>;

    Vector2 position; ///< position of the midpoint of the plane
    Vector2 positionOffset = { 0, 0 }; ///< offset of the midpoint relative to the plane texture

    float    speed;

    Angle256 pitch = 0;
    Angle256 roll = 0;
    PlaneTextures textures;
    // this is a cached value, calculated from the pitch and speed.
    mutable Vector2 speedVector = { 0, 0 };

    PlaneTextures LoadPlaneTextures( std::string_view skin);
};

#endif // PLANE_H
