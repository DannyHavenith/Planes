#include "CloudSystem.h"
#include "DrawingUtilities.h"
#include "GameWindow.h"
#include "raylib.h"
#include "VectorMath.h"
#include <cmath>


namespace
{
    Vector2 Scale( const Vector2& scale, const Vector2& position)
    {
        return { Wrap( position.x * scale.x, 1.0f), Wrap( position.y * scale.y, 1.0f) };
    }
}

Cloud CreateRandomCloud(float averageSize, float averageOpacity, int numberOfCircles)
{
    const int randomScale = 4096;

    // get a random value between 0 and 1.
    const auto getRandomValue = []{ return GetRandomValue(0, randomScale) / static_cast<float>(randomScale); };

    Cloud cloud;
    cloud.color = Fade(WHITE, averageOpacity);
    cloud.circles.reserve(numberOfCircles);
    cloud.position = { getRandomValue(), getRandomValue() };
    cloud.speed = { static_cast<float>( GetRandomValue( -10,10)) / 1024, 0.0f };

    if (numberOfCircles > 0)
    {
        // Start with a random initial position for the first circle
        CloudCircle firstCircle;
        firstCircle.position = {0.0f, 0.0f};
        firstCircle.radius = averageSize/2 + getRandomValue() * averageSize;
        firstCircle.opacity = getRandomValue();
        cloud.circles.push_back(firstCircle);

        for (int i = 1; i < numberOfCircles; ++i)
        {
            CloudCircle circle;
            const auto& referenceCircle = cloud.circles[i/4];
            float angle = getRandomValue() * 2 * M_PI;
            float distance = averageSize/2 + averageSize * getRandomValue();
            circle.position = {
                referenceCircle.position.x + std::cos(angle) * distance,
                referenceCircle.position.y + std::sin(angle) * distance
            };
            circle.radius = averageSize/2 + getRandomValue() * averageSize;
            circle.opacity = std::min( 0.2f * getRandomValue() - 0.1f + averageOpacity, 1.0f);
            cloud.circles.push_back(circle);
        }
    }
    return cloud;
}

CloudSystem CreateRandomCloudSystem(
    int numberOfClouds,
    int numberOfCircles,
    float averageSize,
    float averageOpacity)
{
    CloudSystem cloudSystem;
    cloudSystem.clouds.reserve(numberOfClouds);

    for (int i = 0; i < numberOfClouds; ++i)
    {
        cloudSystem.clouds.push_back(CreateRandomCloud(averageSize, averageOpacity, numberOfCircles));
    }

    return cloudSystem;
}

void DrawCloud( const Cloud& cloud, const GameWindow& window)
{
    const Vector2 scale = { static_cast<float>(window.width), static_cast<float>(window.height) };
    const float scalarScale = std::min(scale.x, scale.y);

    // Draw the cloud's circles
    for (const auto& circle : cloud.circles)
    {
        const auto pixelPosition = Scale(scale, circle.position + cloud.position);
        const auto pixelRadius = scalarScale * circle.radius;
        DrawCircleV(
            pixelPosition,
            pixelRadius,
            Fade(cloud.color, circle.opacity));

        if (pixelPosition.x + pixelRadius > window.width)
        {
            DrawCircleV(
                pixelPosition - Vector2{ static_cast<float>( window.width), 0.0f},
                pixelRadius,
                Fade(cloud.color, circle.opacity));
        }
        else if ( pixelPosition.x < pixelRadius)
        {
            DrawCircleV(
                pixelPosition + Vector2{ static_cast<float>( window.width), 0.0f},
                pixelRadius,
                Fade(cloud.color, circle.opacity));
        }
    }

    //DrawCircleV( Scale( scale, cloud.position), 5, RED);
}

void DrawCloudSystem(const CloudSystem& cloudSystem, const GameWindow& window)
{
    for (const auto& cloud : cloudSystem.clouds)
    {
        DrawCloud(cloud, window);
    }
}
