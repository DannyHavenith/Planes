#include "Plane.h"

#include "DrawingUtilities.h"
#include "GameWindow.h"
#include "VectorMath.h"

#include <cmath>
#include <iomanip>
#include <sstream>

struct GameWindow;

Plane::Plane(
    std::string_view skin,
    Vector2 position,
    float speed,
    std::uint8_t pitch)
    : position(position), speed(speed), pitch(pitch)
{
    textures = LoadPlaneTextures( skin);
}

std::uint8_t Plane::DeltaPitch(std::int8_t angle)
{
    return this->pitch += angle;
}

std::uint8_t Plane::DeltaRoll(std::int8_t angle)
{
    return this->roll += angle;
}

void Plane::Draw( const GameWindow &window) const
{
    // Draw the plane texture with wrapping
    DrawWrapped(window, textures.at(roll/16), position, positionOffset, pitch, WHITE);
}

void Plane::Update( const GameWindow &window, float deltaTime)
{
    speedVector = Vector2{
        static_cast<float>(std::cos(pitch * M_PI / 128.0f) * speed),
        static_cast<float>(std::sin(pitch * M_PI / 128.0f) * speed)};
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
