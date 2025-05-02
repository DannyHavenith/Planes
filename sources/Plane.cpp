#include "Plane.h"

#include "CloudSystem.h"
#include "DrawingUtilities.h"
#include "GameWindow.h"
#include "VectorMath.h"


#include "raylib.h"

#include <cmath>
#include <iomanip>
#include <sstream>


namespace {
    /**
     * Definition of a 'hit circle'.
     * These circles are used to determine if the plane is hit by a bullet.
     * Each plane has a set of hit circles that roughly correspond to the
     * plane's shape.
     */
    struct HitCircle
    {
        Vector2 position;
        float radius;
        float radiusSquared;
    };

    // hit circles are relative to the plane position.
    static constexpr std::array< HitCircle, 4> hitCircles = {{
        {{  11.0f, 0.0f}, 11.0f, 121.0f},
        {{  -3.0f, 0.0f},  7.0f, 49.0f},
        {{ -18.0f, 0.0f}, 8.0f, 64.0f},
        {{ -35.0f, 0.0f}, 8.0f, 64.0f}
    }};

    // Calculates the squared distance between two Vector2 points.
    float Vector2DistanceSquared(Vector2 v1, Vector2 v2)
    {
        return (v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y);
    }

    struct DebugSettings
    {
        bool drawDiagnostics = false;
    } debugSettings;

    RenderTexture2D CreateBulletTexture( Color color, int count)
    {
        // Create the bullet texture
        constexpr auto bulletCircleRadius = 8.0f;
        constexpr auto bulletCircleDistance = 2.0f;
        auto bulletTextureWidth = static_cast<int>(count * (2 * bulletCircleRadius + bulletCircleDistance) - bulletCircleDistance);
        auto bulletTexture = LoadRenderTexture(bulletTextureWidth, static_cast<int>(2 * bulletCircleRadius));

        // Begin drawing to the render texture
        BeginTextureMode(bulletTexture);
        ClearBackground(BLANK);

        for (int i = 0; i < count; ++i)
        {
            float x = i * (2 * bulletCircleRadius + bulletCircleDistance) + bulletCircleRadius;
            float y = bulletCircleRadius;
            DrawCircle(static_cast<int>(x), static_cast<int>(y), bulletCircleRadius, color);
        }

        // End drawing to the render texture
        EndTextureMode();

        return bulletTexture;
    }
}

void DrawPlaneDebugIndicators( bool doDraw)
{
    debugSettings.drawDiagnostics = doDraw;
}

Plane::Plane(
    int id,
    std::string_view skin,
    Color color,
    Vector2 position,
    float speed,
    Angle256 pitch)
    : id(id),color(color),position(position), speed(speed), pitch(pitch), bulletTexture( CreateBulletTexture( color, static_cast<int>(maxBullets)))
{
    textures = LoadPlaneTextures( skin);

    // Create the bullet texture
    constexpr auto bulletCircleRadius = 8.0f;
    constexpr auto bulletCircleDistance = 2.0f;
    auto bulletTextureWidth = static_cast<int>(maxBullets * (2 * bulletCircleRadius + bulletCircleDistance) - bulletCircleDistance);
    auto bulletTexture = LoadRenderTexture(bulletTextureWidth, static_cast<int>(2 * bulletCircleRadius));

    // Begin drawing to the render texture
    BeginTextureMode(bulletTexture);
    ClearBackground(BLANK);

    for (int i = 0; i < maxBullets; ++i)
    {
        float x = i * (2 * bulletCircleRadius + bulletCircleDistance) + bulletCircleRadius;
        float y = bulletCircleRadius;
        DrawCircle(static_cast<int>(x), static_cast<int>(y), bulletCircleRadius, color);
    }

    // End drawing to the render texture
    EndTextureMode();
}

void Plane::Reset( Vector2 position, float speed, Angle256 pitch)
{
    this->position = position;
    this->speed = speed;
    this->pitch = pitch;
    this->roll = 0;
    this->state = Newborn;
    timer = 2.0f;
}

bool Plane::Fire( Bullets &bullets)
{
    if (state == Flying and bulletCount >= 1.0f)
    {
        bulletCount -= 1.0f;
        bullets.emplace_back( color, id, position, speedVector * 2.0f);
        return true;
    }
    else
    {
        return false;
    }
}

Angle256 Plane::DeltaPitch(std::int8_t angle)
{
    return this->pitch += angle;
}

Angle256 Plane::DeltaRoll(std::int8_t angle)
{
    return this->roll += angle;
}

void Plane::Draw( const GameWindow &window) const
{

    if (state == Crashed)
    {
        return;
    }

    // Draw the plane texture with wrapping.
    DrawWrapped(window, textures.at(roll/16), position, positionOffset, pitch, state == Newborn? Fade( WHITE, 0.5f):WHITE, state == Crashing);

    if (debugSettings.drawDiagnostics)
    {
        // Draw a circle at the plane position.
        DrawCircleLinesV( position, 20, PURPLE);

        // Draw the hit circles.
        for (const auto& circle : hitCircles)
        {
            Vector2 scaledPosition = position + Rotate( circle.position, pitch);
            DrawCircleLinesV(scaledPosition, std::sqrt( circle.radiusSquared), BLACK);
        }
    }
}

void Plane::Update( const GameWindow &window, float deltaTime)
{
    // do nothing if we (crashed) offscreen
    if (state == Crashed or (state == Crashing and position.y > window.height + positionOffset.y))
    {
        state = Crashed;
        return;
    }
    else if (state == Crashing)
    {
        DeltaRoll( 4);
        if (pitch >= 192 or pitch < 32)
        {
            DeltaPitch( 120.0 * deltaTime + 0.5f);
        }
        else if (pitch >= 96)
        {
            DeltaPitch(-120.0 * deltaTime - 0.5f);
        }
    }

    if (state == Newborn)
    {
        timer -= deltaTime;
        if (timer <= 0.0f)
        {
            state = Flying;
            timer = 0.0f;
        }
    }

    speedVector = Vector2{
        cos(pitch) * speed,
        sin(pitch) * speed};
    position += speedVector * deltaTime;

    // Wrap around the screen edges, except when we are crashing.
    if (state == Flying or state == Newborn)
    {
        position = {
            Wrap(position.x, static_cast<float>(window.width)),
            Wrap(position.y, static_cast<float>(window.height))};
    }

    if (bulletCount < maxBullets)
    {
        bulletCount += 0.75f * deltaTime;
        if (bulletCount > maxBullets)
        {
            bulletCount = maxBullets;
        }
    }
}

Plane::PlaneTextures Plane::LoadPlaneTextures( std::string_view skin)
{
    PlaneTextures textures;

    for (int i = 0; i < textures.size(); ++i)
    {
        std::ostringstream oss;
        oss << ASSETS_PATH << skin << std::setw(4) << std::setfill('0') << i << ".png";
        textures[i] = LoadTexture(oss.str().c_str());
    }

    positionOffset = {
        static_cast<float>(textures[0].width) / 2.0f,
        static_cast<float>(textures[0].height) / 2.0f};

    return textures;
}

Rectangle Plane::GetBoundingBox() const
{
    return {
        position.x - positionOffset.x,
        position.y - positionOffset.y,
        static_cast<float>(textures[0].width),
        static_cast<float>(textures[0].height)};
}

bool Plane::Collides( Vector2 point) const
{
    // we can't be hit if we're crashing, or newborn.
    if (state == Flying)
    {
        for (const auto& circle : hitCircles)
        {
            Vector2 scaledPosition = position + Rotate( circle.position, pitch);
            if (Vector2DistanceSquared(point, scaledPosition) < circle.radiusSquared)
            {
                return true;
            }
        }
    }
    return false;
}
