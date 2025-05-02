#ifndef PLANE_H
#define PLANE_H

#include "Angle256.h"
#include "Bullet.h"
#include "raylib.h"

#include <array>
#include <cstdint>
#include <string_view>

struct GameWindow;

class Plane
{
public:
    enum State {
        Flying,
        Crashing,
        Crashed,
        Newborn
    };
    Plane(
        int id,
        std::string_view skin,
        Color color,
        Vector2 position,
        float speed = 200,
        Angle256 pitch = 0);

    void Reset( Vector2 position, float speed, Angle256 pitch);

    Angle256 DeltaPitch(std::int8_t angle);
    Angle256 DeltaRoll(std::int8_t angle);
    Angle256 GetRoll() const  { return roll; }
    Angle256 GetPitch() const { return pitch; }
    Vector2 GetPosition() const { return position; }
    Vector2 GetSpeedVector() const { return speedVector; }
    float GetSpeed() const { return speed; }
    Color GetColor() const { return color; }

    void Draw( const GameWindow &window) const;
    void Update( const GameWindow &window, float deltaTime);
    Rectangle GetBoundingBox() const;
    void SetState( State state) { this->state = state; }
    State GetState() const { return state; }
    bool Collides( Vector2 point) const;
    bool Fire( Bullets &bullets);
    void DrawBulletCount( const GameWindow &window, const Vector2 &position) const
    {
        if (bulletCount > 0) {
            float width = bulletTexture.texture.width * (bulletCount / maxBullets);
            Rectangle sourceRec = { 0, 0, width, static_cast<float>(bulletTexture.texture.height) };
            Rectangle destRec = { position.x, position.y, width, static_cast<float>(bulletTexture.texture.height) };
            Vector2 origin = { 0, 0 };
            DrawTexturePro(bulletTexture.texture, sourceRec, destRec, origin, 0.0f, WHITE);
        }
    }

private:
    using PlaneTextures = std::array<Texture2D, 16>;
    constexpr static float maxBullets = 3.0f;


    std::size_t id;
    Vector2 position; ///< position of the midpoint of the plane
    Vector2 positionOffset = { 0, 0 }; ///< offset of the midpoint relative to the plane texture

    Color   color = PURPLE;
    float    speed = 200.0f;
    Angle256 pitch = 0;
    Angle256 roll = 0;
    PlaneTextures textures;
    State state = Flying;
    float timer = 0.0f; // used for automatic state transitions
    float bulletCount = maxBullets;

    // this is a cached value, calculated from the pitch and speed.
    mutable Vector2 speedVector = { 0, 0 };
    const RenderTexture2D bulletTexture;

    PlaneTextures LoadPlaneTextures( std::string_view skin);
};

void DrawPlaneDebugIndicators( bool doDraw = true);

#endif // PLANE_H
