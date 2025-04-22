#ifndef VECTOR_MATH_H
#define VECTOR_MATH_H

#include "Angle256.h"
#include "raylib.h"

inline Vector2 Rotate( const Vector2& vector, Angle256 angle)
{
    return {
        vector.x * cos(angle) - vector.y * sin(angle),
        vector.x * sin(angle) + vector.y * cos(angle)
    };
}

// Overload operators for Vector2 math operations
inline Vector2 operator*=(Vector2& lhs, const float rhs)
{
    lhs.x *= rhs;
    lhs.y *= rhs;
    return lhs;
}

inline Vector2 operator*(Vector2 lhs, const float rhs)
{
    return lhs *= rhs;
}

inline Vector2 operator-=(Vector2& lhs, const Vector2& rhs)
{
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    return lhs;
}

inline Vector2 operator-(Vector2 lhs, const Vector2& rhs)
{
    return lhs -= rhs;
}

inline Vector2 operator+=(Vector2& lhs, const Vector2& rhs)
{
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    return lhs;
}

inline Vector2 operator+(Vector2 lhs, const Vector2& rhs)
{
    return lhs += rhs;
}

inline bool operator==(const Vector2& lhs, const Vector2& rhs)
{
    return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

inline bool operator!=(const Vector2& lhs, const Vector2& rhs)
{
    return !(lhs == rhs);
}

#endif // VECTOR_MATH_H
