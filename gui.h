#pragma once
#include <imgui.h>

struct GuiHandler {
    ImFont* FontBodyRegular;
    ImFont* FontBodyBold;
    ImFont* FontDisplayRegular;

    ImGuiWindowFlags clipboardWindowFlags;
    ImGuiWindowFlags selectionWindowFlags;
    ImGuiWindowFlags geminiStatusWindowFlags;

    void setupStyles();
};