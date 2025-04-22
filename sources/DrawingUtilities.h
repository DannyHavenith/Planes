#ifndef DRAWING_UTILITIES_H
#define DRAWING_UTILITIES_H

#include "GameWindow.h"
#include "raylib.h"

#include <cstdint>

template <typename ValueType>
ValueType Wrap(ValueType value, ValueType max)
{
    if (value < 0)
    {
        return value + max;
    }
    else if (value >= max)
    {
        return value - max;
    }
    return value;
}

/**
 * Draw the given texture to the screen, wrapping it around the screen edges if necessary.
 * Wrapping is done by drawing the texture at its original position and at the
 * wrapped positions on the x and y axes.
 */
inline void DrawWrapped(
    const GameWindow &window,
    const Texture2D& texture,
    const Vector2& position,
    const Vector2& offset,
    std::uint8_t angle,
    const Color& tint)
{
    Vector2 textureSize = { static_cast<float>(texture.width), static_cast<float>(texture.height) };
    Rectangle sourceRect = { 0.0f, 0.0f, textureSize.x, textureSize.y };
    const Rectangle destRect = { position.x, position.y, textureSize.x, textureSize.y };
    float rotation = (angle / 256.0f) * 360.0f;

    // Draw the main texture
    DrawTexturePro(texture, sourceRect, destRect, offset, rotation, tint);

    // Handle wrapping on the x-axis
    if (position.x < textureSize.x/2) // TODO: offset.x
    {
        DrawTexturePro(texture, sourceRect, {destRect.x + window.width, destRect.y, destRect.width, destRect.height}, offset, rotation, tint);
    }
    else if (position.x >= window.width - textureSize.x/2)
    {
        DrawTexturePro(texture, sourceRect, {destRect.x - window.width, destRect.y, destRect.width, destRect.height}, offset, rotation, tint);
    }

    // Handle wrapping on the y-axis
    if (position.y < textureSize.y/2)
    {
        DrawTexturePro(texture, sourceRect, {destRect.x, destRect.y + window.height, destRect.width, destRect.height}, offset, rotation, tint);
    }
    else if (position.y >= window.height - textureSize.y/2)
    {
        DrawTexturePro(texture, sourceRect, {destRect.x, destRect.y - window.height, destRect.width, destRect.height}, offset, rotation, tint);
    }

}

#endif // DRAWING_UTILITIES_H
