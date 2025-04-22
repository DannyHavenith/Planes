#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include "raylib.h"
#include <string>

struct GameWindow
{
    GameWindow(int width, int height, const char* title)
        : width(width), height(height), title(title)
    {
        InitWindow(width, height, title);
    }

    /// Adapt the dimensions because some _outside_ event
    /// changed the window size.
    void UpdateDimensions(int width, int height)
    {
        this->width = width;
        this->height = height;
    }

    ~GameWindow()
    {
        CloseWindow();
    }

    void Update()
    {
        if (IsWindowResized())
        {
            width = GetScreenWidth();
            height = GetScreenHeight();
        }
    }

    int width;
    int height;
    const std::string title;
};

#endif // GAMEWINDOW_H
