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

// fade text with transparent rectangle
void drawFadedTextOverlay() {
    // https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-how-can-i-display-custom-shapes-using-low-level-imdrawlist-api
    // imgui_demo.cpp 9646
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // get rect region
    // https://github.com/ocornut/imgui/issues/2486#issuecomment-482635607
    ImVec2 vMin = ImGui::GetWindowContentRegionMin();
    ImVec2 vMax = ImGui::GetWindowContentRegionMax();

    vMin.x += ImGui::GetWindowPos().x;
    vMin.y += ImGui::GetWindowPos().y;
    vMax.x += ImGui::GetWindowPos().x;
    vMax.y += ImGui::GetWindowPos().y;

    // minus half of window padding and lower top by half of window height
    vMin.x -= 8.0f;
    vMin.y += 125.0f;
    vMax.x += 8.0f;
    //vMax.y += 10.0f;

    ImU32 transparentColor = ImGui::GetColorU32(IM_COL32(28, 28, 28, 0));
    ImU32 bodyColor = ImGui::GetColorU32(IM_COL32(28, 28, 28, 255));
    draw_list->AddRectFilledMultiColor(vMin, vMax, transparentColor, transparentColor, bodyColor, bodyColor);
    //ImGui::GetForegroundDrawList()->AddRect(vMin, vMax, IM_COL32(255, 255, 0, 255));

    // bottom solid rect part
    ImVec2 solidCoordsTop = ImVec2(vMin.x, vMax.y);
    ImVec2 solidCoordsBottom = ImVec2(vMax.x, vMax.y + 15);
    draw_list->AddRectFilled(solidCoordsTop, solidCoordsBottom, bodyColor, bodyColor);
    //ImGui::GetForegroundDrawList()->AddRect(solidCoordsTop, solidCoordsBottom, IM_COL32(255, 0, 0, 255));
}


void GuiHandler::drawAPIKeyPromptWindow(
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

void GuiHandler::drawClipboardWindow(std::string& clipboardText, bool& shouldShowGeminiKeyPrompt) {
    ImGui::SetNextWindowSize(ImVec2(guiWindowWidth, guiWindowHeight));
    ImGui::SetNextWindowPos(ImVec2(mouseOrigin.x, mouseOrigin.y - guiWindowHeight - guiWindowMargin), ImGuiCond_Appearing);
    ImGui::Begin("Clipboard", NULL, clipboardWindowFlags);

    // header
    ImGui::AlignTextToFramePadding();
    ImGui::PushFont(FontBodyBold);
    ImGui::Text("Clipboard");
    ImGui::PopFont();

    //
    ImGui::PushFont(FontBodyRegular);

    // text length
    ImGui::SameLine();
    int clipboardLength = clipboardText.length();
    if (clipboardLength > 135) {
        ImGui::TextDisabled("%d characters", clipboardLength);
    }

    // settings button
    ImGui::SameLine();
    ImVec2 buttonSize = ImVec2(80.0f, 0);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - buttonSize.x);
    ImGui::BeginDisabled(shouldShowGeminiKeyPrompt);
    if (ImGui::Button("Settings", buttonSize)) {
        shouldShowGeminiKeyPrompt = true;
    }
    ImGui::EndDisabled();

    ImGui::PopFont();
    //

    ImGui::PushFont(FontDisplayRegular);
    if (clipboardText.empty()) {
        ImGui::TextDisabled("No text in your clipboard.");

        ImGui::PushFont(FontBodyRegular);
        ImGui::TextDisabled("Copying text to your clipboard will make it available here.");
        ImGui::PopFont();
    }
    else {
        ImGui::TextWrapped("%s", clipboardText.c_str());
    }
    ImGui::PopFont();

    drawFadedTextOverlay();

    ImGui::End();
}

void GuiHandler::drawAPIRunningState(GeminiClient& geminiClient) {
    ImGui::SetNextWindowSize(ImVec2(guiWindowWidth, guiWindowHeight));
    ImGui::SetNextWindowPos(mouseOrigin, ImGuiCond_Appearing);
    ImGui::Begin("gemini loading", NULL, geminiStatusWindowFlags);

    ImGui::PushFont(FontBodyRegular);

    ImGui::Text("Generating suggestions...");

    ImGui::PopFont();
    ImGui::End();
}

void GuiHandler::drawAPIFailedState(GeminiClient& geminiClient) {
    ImGui::SetNextWindowSize(ImVec2(guiWindowWidth, guiWindowHeight));
    ImGui::SetNextWindowPos(mouseOrigin, ImGuiCond_Appearing);
    ImGui::Begin("gemini failed", NULL, geminiStatusWindowFlags);

    ImGui::PushFont(FontBodyRegular);

    ImGui::PushFont(FontBodyBold);
    ImGui::TextColored(ImVec4(1.0f, 0.24f, 0.24f, 1.0f), "Sorry!");
    ImGui::PopFont();

    ImGui::Text("Couldn't get suggestions.");
    ImGui::TextWrapped(geminiClient.errorFeedback.c_str());

    ImGui::Spacing();
    if (ImGui::Button("Try again")) {
        geminiClient.reset();
    }

    ImGui::PopFont();
    ImGui::End();
}

void GuiHandler::drawAPIFinishedState(
    GeminiClient& geminiClient,
    std::function<void(std::string)> selectionEventHandler
) {
    ImGui::SetNextWindowSize(ImVec2(guiWindowWidth, guiWindowHeight));
    ImGui::SetNextWindowPos(mouseOrigin, ImGuiCond_Appearing);
    ImGui::Begin("gemini suggestions", NULL, selectionWindowFlags);

    ImGui::PushFont(FontBodyRegular);

   

    // TODO make this look good
    for (const std::string suggestion : geminiClient.suggestions) {
        if (ImGui::Button(suggestion.c_str())) { selectionEventHandler(suggestion); }
    }

    ImGui::Spacing();
    if (ImGui::Button("Reset")) {
        geminiClient.reset();
    }

    ImGui::PopFont();
    ImGui::End();
}

void GuiHandler::drawEditOptionsWindow(
    GeminiClient& geminiClient,
    std::function<void(GeminiClient::PromptType)> selectOptionEventHandler
) {
    // options window
    ImGui::SetNextWindowPos(mouseOrigin, ImGuiCond_Appearing);
    ImGui::Begin("edit options", NULL, clipboardWindowFlags);

    ImGui::PushFont(FontBodyBold);
    ImGui::Text("Edit Text");
    ImGui::PopFont();

    ImGui::PushFont(FontBodyRegular);

    // https://github.com/ocornut/imgui/issues/1889
    ImGui::BeginDisabled(geminiClient.isClientDoingSomething());

    if (ImGui::Button("LAYOUT TEST")) {
        geminiClient.debug();
    }

    if (ImGui::Button("Synonyms for...")) { selectOptionEventHandler(GeminiClient::PromptType::SYNONYMS); }
    ImGui::SetItemTooltip("Get synonyms for a word\nWill also rephrase sentences");

    //if (ImGui::Button("Rephrase...")) { handleButtonClick(GeminiClient::PromptType::REPHRASE); }

    if (ImGui::Button("Rewrite formally...")) { selectOptionEventHandler(GeminiClient::PromptType::FORMALIZE); }
    ImGui::SetItemTooltip("Keep it professional");

    if (ImGui::Button("Antonyms for...")) { selectOptionEventHandler(GeminiClient::PromptType::ANTONYMS); }
    ImGui::SetItemTooltip("Get antonyms for a word\nWill rewrite your clipboard to mean the opposite");

    if (ImGui::Button("Shorten...")) { selectOptionEventHandler(GeminiClient::PromptType::SHORTEN); }
    ImGui::SetItemTooltip("Rewrites the content of your clipboard to be shorter");

    if (ImGui::Button("Ungarble...")) { selectOptionEventHandler(GeminiClient::PromptType::UNGARBLE); }
    if (ImGui::BeginItemTooltip())
    {
        ImGui::Text("Will rewrite sentences that don't sound quite right, useful for:");
        ImGui::BulletText("Gramatically incorrect sentences");
        ImGui::BulletText("Fixing incorrect word usage");
        ImGui::BulletText("Run on sentences");

        ImGui::EndTooltip();
    }

    ImGui::EndDisabled();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::PushFont(FontBodyBold);
    ImGui::Text("Reformat into a...");
    ImGui::PopFont();

    ImGui::BeginDisabled(geminiClient.isClientDoingSomething());
    if (ImGui::Button("Headline")) { selectOptionEventHandler(GeminiClient::PromptType::HEADLINE); }
    if (ImGui::Button("Tagline")) { selectOptionEventHandler(GeminiClient::PromptType::TAGLINE); }
    if (ImGui::Button("One word phrase")) { selectOptionEventHandler(GeminiClient::PromptType::ONEWORD); }
    if (ImGui::Button("Two word phrase")) { selectOptionEventHandler(GeminiClient::PromptType::TWOWORD); }
    ImGui::EndDisabled();

    ImGui::PopFont();
    ImGui::End();
}