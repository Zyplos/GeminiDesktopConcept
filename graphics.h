#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector> // Include vector for pixel data buffer

// Existing function declarations
void loadGraphics();
void drawGraphics(
    float noiseSpeed, float perlinScale, float lowerEdge, float upperEdge,
    float perlinOffsetX, float perlinOffsetY, float gradientOffsetX, float gradientOffsetY,
    float revealStartTime, float revealMouseX, float revealMouseY, float revealDuration,
    int bufferWidth, int bufferHeight
);
void destroyGraphics();

// --- New Cursor Graphics Declarations ---
extern GLuint cursorTextureID; // Texture ID for the captured cursor
extern int cursorWidth;        // Width of the captured cursor
extern int cursorHeight;       // Height of the captured cursor
extern int cursorHotspotX;     // Hotspot X of the captured cursor
extern int cursorHotspotY;     // Hotspot Y of the captured cursor

void loadCursorGraphics();
bool captureCursorTexture(); // Captures the current cursor, returns true on success
void drawCursor(float normX, float normY, int windowWidth, int windowHeight); // Draws the captured cursor
void destroyCursorGraphics();
// --- End New Cursor Graphics Declarations ---


#endif // GRAPHICS_H