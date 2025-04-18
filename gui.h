#pragma once
#include <string>
#include <imgui.h>
#include <imgui_stdlib.h>
#include "geminiAPI.hpp"

struct GuiHandler {
    ImFont* FontBodyRegular;
    ImFont* FontBodyBold;
    ImFont* FontDisplayRegular;

    ImGuiWindowFlags clipboardWindowFlags;
    ImGuiWindowFlags selectionWindowFlags;
    ImGuiWindowFlags geminiStatusWindowFlags;

    float guiWindowWidth = 500;
    float guiWindowHeight = 175;
    float guiWindowMargin = 10;

    ImVec2 mouseOrigin;

    void setupStyles();

    void drawAPIKeyPromptWindow(
        std::string& GEMINI_KEY, 
        GeminiClient& geminiClient,
        bool& shouldShowGeminiKeyPrompt
    );

    void drawAPIRunningState(GeminiClient& geminiClient);

    void drawAPIFailedState(GeminiClient& geminiClient);

    // also known as the selection panel
    void drawAPIFinishedState(
        GeminiClient& geminiClient,
        std::function<void(std::string)> selectionEventHandler);
};