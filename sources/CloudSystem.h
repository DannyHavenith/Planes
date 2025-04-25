#ifndef CLOUD_SYSTEM_H
#define CLOUD_SYSTEM_H

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

using CloudSystem = std::vector<Cloud>;
void Draw(const Cloud& cloud, const GameWindow& window);
void Update(Cloud& cloud, const GameWindow& window, float deltaTime);

Cloud CreateRandomCloud(float averageSize, float averageOpacity, int numberOfCircles);
CloudSystem CreateRandomCloudSystem(
    int numberOfClouds,
    int numberOfCircles,
    float averageSize,
    float averageOpacity);
#endif // CLOUD_SYSTEM_H
