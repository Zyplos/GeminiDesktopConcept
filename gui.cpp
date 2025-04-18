#include "gui.h"

void GuiHandler::setupStyles() {
    // ===== fonts and styling
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.Fonts->AddFontDefault();
    FontBodyRegular = io.Fonts->AddFontFromFileTTF("Outfit-Regular.ttf", 20.0f);
    FontBodyBold = io.Fonts->AddFontFromFileTTF("Outfit-Bold.ttf", 20.0f);
    FontDisplayRegular = io.Fonts->AddFontFromFileTTF("Outfit-Regular.ttf", 32.0f);



    // ===== padding and colors
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize = 1.f;
    style.WindowPadding = ImVec2(16.0f, 16.0f);
    style.WindowRounding = 8.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 8.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 12.0f;
    style.AntiAliasedLines = true;
    style.AntiAliasedFill = true;

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = colors[ImGuiCol_Button] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.21f, 0.22f, 0.25f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.42f, 0.46f, 0.52f, 0.54f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.57f, 0.69f, 0.82f, 0.40f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.17f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.17f, 0.18f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.68f, 0.78f, 0.97f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.47f, 0.50f, 0.54f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.68f, 0.78f, 0.97f, 1.00f);
    //colors[ImGuiCol_Button] = ImVec4(0.27f, 0.28f, 0.30f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.27f, 0.28f, 0.30f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.37f, 0.40f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.15f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.24f, 0.25f, 0.27f, 1.00f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.15f, 0.45f, 1.00f, 0.90f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);



    // ===== window flags
    clipboardWindowFlags = 0;
    clipboardWindowFlags |= ImGuiWindowFlags_NoMove;
    clipboardWindowFlags |= ImGuiWindowFlags_NoTitleBar;
    clipboardWindowFlags |= ImGuiWindowFlags_NoScrollbar;
    clipboardWindowFlags |= ImGuiWindowFlags_NoScrollWithMouse;
    clipboardWindowFlags |= ImGuiWindowFlags_NoMove;
    clipboardWindowFlags |= ImGuiWindowFlags_NoResize;
    clipboardWindowFlags |= ImGuiWindowFlags_NoCollapse;
    clipboardWindowFlags |= ImGuiWindowFlags_AlwaysAutoResize;

    selectionWindowFlags = 0;
    selectionWindowFlags |= ImGuiWindowFlags_NoMove;
    selectionWindowFlags |= ImGuiWindowFlags_NoTitleBar;
    selectionWindowFlags |= ImGuiWindowFlags_NoMove;
    selectionWindowFlags |= ImGuiWindowFlags_NoResize;
    selectionWindowFlags |= ImGuiWindowFlags_NoCollapse;
    selectionWindowFlags |= ImGuiWindowFlags_AlwaysAutoResize;
    selectionWindowFlags |= ImGuiWindowFlags_NoBackground;

    geminiStatusWindowFlags = 0;
    geminiStatusWindowFlags |= ImGuiWindowFlags_NoMove;
    geminiStatusWindowFlags |= ImGuiWindowFlags_NoTitleBar;
    geminiStatusWindowFlags |= ImGuiWindowFlags_NoMove;
    geminiStatusWindowFlags |= ImGuiWindowFlags_NoResize;
    geminiStatusWindowFlags |= ImGuiWindowFlags_NoCollapse;
    geminiStatusWindowFlags |= ImGuiWindowFlags_AlwaysAutoResize;
}

void GuiHandler::drawAPIKeyPromptWindow(
    ImVec2 mouseOrigin, 
    std::string& GEMINI_KEY, 
    GeminiClient& geminiClient,
    bool& shouldShowGeminiKeyPrompt
) {
    //ImGui::SetNextWindowSize(ImVec2(guiWindowWidth, guiWindowHeight));
    ImGui::SetNextWindowPos(mouseOrigin, ImGuiCond_Appearing);
    ImGui::PushFont(FontBodyRegular);

    ImGui::Begin("api key window", NULL, geminiStatusWindowFlags);

    ImGui::PushFont(FontBodyBold);
    ImGui::Text("Settings");
    ImGui::PopFont();

    ImGui::TextWrapped("You'll need an API Key from AI Studio. Grab one here: ");
    ImGui::TextLinkOpenURL("https://aistudio.google.com/app/apikey");

    // dummy for spacing
    // https://github.com/ocornut/imgui/issues/1487
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    std::string biungus = "gungus;";
    ImGui::InputText("API Key", &GEMINI_KEY, ImGuiInputTextFlags_CharsNoBlank);
    ImGui::Spacing();

    bool shouldntLetUserSave = GEMINI_KEY.empty() || GEMINI_KEY.length() < 20;
    ImGui::BeginDisabled(shouldntLetUserSave);
    if (ImGui::Button("Save") && !shouldntLetUserSave) {
        geminiClient = GeminiClient(GEMINI_KEY);
        shouldShowGeminiKeyPrompt = false;
    }
    ImGui::EndDisabled();

    ImGui::PopFont();
    ImGui::End();
}