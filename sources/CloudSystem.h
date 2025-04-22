#ifndef CLOUD_SYSTEM_H
#define CLOUD_SYSTEM_H

#include "DrawingUtilities.h"
#include "raylib.h"

#include <vector>

struct GameWindow;

struct CloudCircle
{
    Vector2 position;
    float radius;
    float opacity;
};

struct Cloud
{
    Vector2 position;
    Vector2 speed;
    std::vector<CloudCircle> circles;
    Color color;
};

struct CloudSystem
{
    void Update(const GameWindow& window, float deltaTime)
    {
        for (auto& cloud : clouds)
        {
            cloud.position.x += cloud.speed.x * deltaTime;
            cloud.position.y += cloud.speed.y * deltaTime;
            cloud.position = {
                Wrap(cloud.position.x, 1.0f),
                Wrap(cloud.position.y, 1.0f)};
        }
    }
    std::vector<Cloud> clouds;
};

void DrawCloud(const Cloud& cloud, const GameWindow& window);
void DrawCloudSystem(const CloudSystem& cloudSystem, const GameWindow& window);
Cloud CreateRandomCloud(float averageSize, float averageOpacity, int numberOfCircles);
CloudSystem CreateRandomCloudSystem(
    int numberOfClouds,
    int numberOfCircles,
    float averageSize,
    float averageOpacity);
#endif // CLOUD_SYSTEM_H
