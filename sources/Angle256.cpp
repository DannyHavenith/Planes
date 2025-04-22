#include "Angle256.h"

#include <array>
#include <cmath>

// Precompute sine values for 256 angles (0 to 255).
// The cosine values can be derived from the sine values using a phase shift.
namespace {
    constexpr int TABLE_SIZE = 256;
    constexpr float TWO_PI = 2.0f * static_cast<float>(M_PI);

    // Lookup table for sine values.
    std::array<float, TABLE_SIZE> generateSinTable()
    {
        std::array<float, TABLE_SIZE> table = {};
        for (int i = 0; i < TABLE_SIZE; ++i) {
            table[i] = std::sin(i * TWO_PI / TABLE_SIZE);
        }
        return table;
    }

    // Precomputed sine table.
    // this can be constexpr as of C++26
    const auto SIN_TABLE = generateSinTable();
}

// Returns the cosine of the given Angle256 value.
float cos(Angle256 angle)
{
    // Cosine is a phase-shifted sine: cos(x) = sin(x + 90 degrees).
    return SIN_TABLE[(angle + 64) % 256];
}

// Returns the sine of the given Angle256 value.
float sin(Angle256 angle)
{
    return SIN_TABLE[angle];
}
