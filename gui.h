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

    void setupStyles();

    void drawAPIKeyPromptWindow(
        ImVec2 mouseOrigin, 
        std::string& GEMINI_KEY, 
        GeminiClient& geminiClient,
        bool& shouldShowGeminiKeyPrompt
    );
};