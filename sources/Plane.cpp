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
}

Plane::Plane(
    std::string_view skin,
    Vector2 position,
    float speed,
    Angle256 pitch)
    : position(position), speed(speed), pitch(pitch)
{
    textures = LoadPlaneTextures( skin);
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

    // Draw the plane texture with wrapping.
    DrawWrapped(window, textures.at(roll/16), position, positionOffset, pitch, WHITE);

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
    speedVector = Vector2{
        cos(pitch) * speed,
        sin(pitch) * speed};
    position += speedVector * deltaTime;
    position = {
        Wrap(position.x, static_cast<float>(window.width)),
        Wrap(position.y, static_cast<float>(window.height))};
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

    for (const auto& circle : hitCircles)
    {
        Vector2 scaledPosition = position + Rotate( circle.position, pitch);
        if (Vector2DistanceSquared(point, scaledPosition) < circle.radiusSquared)
        {
            return true;
        }
    }
    return false;
}
