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
    style.FramePadding = ImVec2(10.0f, 4.0f);
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
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.53f, 0.66f, 1.00f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.63f, 0.73f, 1.00f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.82f, 0.87f, 1.00f, 1.00f);



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
    draw_list->AddRectFilled(solidCoordsTop, solidCoordsBottom, bodyColor);
    //ImGui::GetForegroundDrawList()->AddRect(solidCoordsTop, solidCoordsBottom, IM_COL32(255, 0, 0, 255));
}


void GuiHandler::drawAPIKeyPromptWindow(
    std::string& GEMINI_KEY, 
    GeminiClient& geminiClient,
    bool& shouldShowGeminiKeyPrompt
) {
    //ImGui::SetNextWindowSize(ImVec2(guiWindowWidth, guiWindowHeight));
    ImGui::SetNextWindowPos(mouseOrigin, ImGuiCond_Appearing);
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

    // text length
    ImGui::SameLine();
    size_t clipboardLength = clipboardText.length();
    if (clipboardLength > 135) {
        ImGui::TextDisabled("%zu characters", clipboardLength);
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

    ImGui::Text("Generating suggestions...");

    ImGui::End();
}

void GuiHandler::drawAPIFailedState(GeminiClient& geminiClient) {
    ImGui::SetNextWindowSize(ImVec2(guiWindowWidth, guiWindowHeight));
    ImGui::SetNextWindowPos(mouseOrigin, ImGuiCond_Appearing);
    ImGui::Begin("gemini failed", NULL, geminiStatusWindowFlags);

    ImGui::PushFont(FontBodyBold);
    ImGui::TextColored(ImVec4(1.0f, 0.24f, 0.24f, 1.0f), "Sorry!");
    ImGui::PopFont();

    ImGui::Text("Couldn't get suggestions.");
    ImGui::TextWrapped(geminiClient.errorFeedback.c_str());

    ImGui::Spacing();
    if (ImGui::Button("Try again")) {
        geminiClient.reset();
    }

    ImGui::End();
}

void GuiHandler::drawAPIFinishedState(
    GeminiClient& geminiClient,
    std::function<void(std::string)> selectionEventHandler
) {
    ImGuiStyle& style = ImGui::GetStyle();
    // grab window padding before overriding it to 0
    ImVec2 windowPadding = style.WindowPadding;
    ImVec2 framePadding = style.FramePadding;

    ImGui::SetNextWindowSize(ImVec2(guiWindowWidth, guiWindowHeight * 2));
    ImGui::SetNextWindowPos(mouseOrigin, ImGuiCond_Appearing);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(1.0f, 1.0f));
    ImGui::Begin("gemini suggestions", NULL, selectionWindowFlags);

    // ===== draw window header
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 startingCorner = ImGui::GetCursorScreenPos();

    ImGui::PushFont(FontBodyBold);
    // tiny bit of offset to align with the button
    ImVec2 textSize = ImGui::CalcTextSize("Suggestions");
    float buttonAlignOffset = (ImGui::GetFrameHeight() - textSize.y) * 0.5f;

    // background rect
    ImVec2 headerBgCoordTop = startingCorner;
    headerBgCoordTop.y += buttonAlignOffset;
    ImVec2 headerBgCoordBottom = ImVec2(
        headerBgCoordTop.x + textSize.x + windowPadding.x * 2,
        headerBgCoordTop.y + textSize.y + framePadding.y * 2
    );

    ImU32 windowBgColor = ImGui::GetColorU32(ImGuiCol_WindowBg);
    ImU32 borderColor = ImGui::GetColorU32(ImGuiCol_Border);
    draw_list->AddRectFilled(headerBgCoordTop, headerBgCoordBottom, windowBgColor, style.WindowRounding);
    draw_list->AddRect(headerBgCoordTop, headerBgCoordBottom, borderColor, style.WindowRounding, 0, 1.0f);

    // ===== draw header text
    ImGui::SetCursorScreenPos(ImVec2(
        startingCorner.x + windowPadding.x,
        startingCorner.y + framePadding.y
    ));
    
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Suggestions");
    ImGui::PopFont();

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(6.0f, 0));
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        geminiClient.reset();
    }

    ImGui::Spacing();

    // ===== actual suggestions window
    ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x, 0), ImGuiChildFlags_None);

    float availableWidth = ImGui::GetContentRegionAvail().x;
    // avoid zero or negative width
    if (availableWidth <= 0.0f) {
        availableWidth = 1.0f;
    }

    for (size_t i = 0; i < geminiClient.suggestions.size(); ++i) {
        const std::string& suggestion = geminiClient.suggestions[i];
        std::string buttonId = "##s" + std::to_string(i);

        // ===== calc sizes
        float textWrapWidth = availableWidth - windowPadding.x * 2.0f;
        // avoid zero or negative width
        if (textWrapWidth <= 0.0f) {
            textWrapWidth = 1.0f;
        }

        ImVec2 textSize = ImGui::CalcTextSize(suggestion.c_str(), nullptr, false, textWrapWidth);

        // availableWidth incluces the x padding we substracted up top
        ImVec2 buttonSize = ImVec2(
            availableWidth,
            textSize.y + windowPadding.y * 2.0f
        );

        // ===== draw button and text
        if (ImGui::Button(buttonId.c_str(), buttonSize)) {
            selectionEventHandler(suggestion);
        }

        ImVec2 buttonCorner = ImGui::GetItemRectMin();
        buttonCorner.x += windowPadding.x;
        buttonCorner.y += windowPadding.y;

        ImGui::GetWindowDrawList()->AddText(
            ImGui::GetFont(),
            ImGui::GetFontSize(),
            buttonCorner,
            ImGui::GetColorU32(ImGuiCol_Text),
            suggestion.c_str(),
            nullptr,
            textWrapWidth
        );

         ImGui::Spacing();
    }

    ImGui::EndChild();

    ImGui::End();
    ImGui::PopStyleVar();
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

    // https://github.com/ocornut/imgui/issues/1889
    ImGui::BeginDisabled(geminiClient.isClientDoingSomething());

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

    ImGui::End();
}