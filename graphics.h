#ifndef GRAPHICS_CLASS_H
#define GRAPHICS_CLASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include<iostream>

void loadGraphics();

void drawGraphics(float noiseSpeed, float perlinScale, float lowerEdge, float upperEdge, float perlinOffsetX, float perlinOffsetY, float gradientOffsetX, float gradientOffsetY, int bufferWidth, int bufferHeight);

void destroyGraphics();

#endif