#pragma once
#include <string>
#define IMGUI_DEFINE_MATH_OPERATORS // Access ImVec2 operators (+,-,*,/)
#include <imgui_internal.h>       // Access ImLerp, ImDrawList, etc.
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
    ImVec2 clipboardOrigin;
    ImVec2 clipboardPivot = ImVec2(1.0f, 1.0f);
    ImVec2 optionsPivot = ImVec2(1.0f, 1.0f);

    ImVec4 blueColor = ImVec4(0.41f, 0.56f, 1.0f, 1.0f);
    ImVec4 redColor = ImVec4(1.0f, 0.41f, 0.47f, 1.0f);

    void setupStyles();

    // NOTE repetitively passing a bunch of the same stuff in these functions
    // probably make some AppState struct thing that holds this stuff if I ever expand on this project
    void drawSettingsWindow(
        std::string& GEMINI_KEY,
        GeminiClient& geminiClient,
        bool& shouldShowGeminiKeyPrompt,
        bool& superWindow,
        std::function<void()> enableSuperWindow,
        std::function<void()> setupOverlayInActiveMonitor,
        std::function<void()> updateStartMouseCoords
    );

    void drawClipboardWindow(std::string& clipboardText, bool& shouldShowGeminiKeyPrompt);

    void drawAPIRunningState(GeminiClient& geminiClient);

    void drawAPIFailedState(GeminiClient& geminiClient);

    // also known as the selection panel
    void drawAPIFinishedState(
        GeminiClient& geminiClient,
        std::function<void(std::string)> selectionEventHandler);

    void drawEditOptionsWindow(
        GeminiClient& geminiClient,
        std::function<void(GeminiClient::PromptType)> selectOptionEventHandler
    );

    void drawFirstRunPrompt(ImVec2 followingCoords);
};