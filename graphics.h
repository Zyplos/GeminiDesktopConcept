#ifndef GRAPHICS_CLASS_H
#define GRAPHICS_CLASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include<iostream>

void loadGraphics();

void drawGraphics(
    float simplexOffsetX, float simplexOffsetY,
    float revealStartTime, float revealMouseX, float revealMouseY
);

void destroyGraphics();

#endif