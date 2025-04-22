#ifndef ANGLE256_H
#define ANGLE256_H

#include <cstdint>

using Angle256 = std::uint8_t;

// Returns the cosine of the given Angle256 value as a float.
float cos(Angle256 angle);

// Returns the sine of the given Angle256 value as a float.
float sin(Angle256 angle);

#endif // ANGLE256_H
