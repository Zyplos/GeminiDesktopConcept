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

    // NOTE repetitively passing a bunch of the same stuff in these functions
    // probably make some AppState struct thing that holds this stuff if I ever expand on this project
    void drawSettingsWindow(
        std::string& GEMINI_KEY, 
        GeminiClient& geminiClient,
        bool& shouldShowGeminiKeyPrompt,
        bool& superWindow
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
};