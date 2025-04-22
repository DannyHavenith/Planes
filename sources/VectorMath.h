#ifndef VECTOR_MATH_H
#define VECTOR_MATH_H

#include "raylib.h"

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
