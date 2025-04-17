#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui_internal.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// needed for global keyboard shortcuts
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h> // For glfwGetWin32Window
#include <Windows.h>
// windows messes with these?
#undef max
#undef min

#define MY_HOTKEY_ID 137

#include "graphics.h"
#include <random>
#include <algorithm>
#include "geminiAPI.hpp"

const GLint WIDTH = 1924, HEIGHT = 1084;
bool showOverlay = true;
std::string clipboardText = "";
GeminiClient geminiClient("");
std::string GEMINI_KEY = "";

// if imgui.ini doesnt exist then UserData_ReadLine doesnt fire
// so show the key prompt by default. UserData_ReadLine will set this to false on launch
bool shouldShowGeminiKeyPrompt = true;

void handleButtonClick(GeminiClient::PromptType type) {
    // gui hides buttons if text is empty but we'll put this here just incase
    if (clipboardText.empty()) {
        return;
    }
    if (geminiClient.state != GeminiClient::IDLE) {
        return;
    }

    std::string prompt = geminiClient.getPrompt(type);
    
    if (prompt.empty()) {
        return;
    }

    std::cout << "USING PROMPT?: " << prompt << std::endl;

    geminiClient.callAPI(prompt, clipboardText);
}

void handleSelectionClick(std::string suggestion) {
    // dont use this https://github.com/ocornut/imgui/discussions/4021
    /*ImGui::LogToClipboard();
    ImGui::LogText(suggestion.c_str());
    ImGui::LogFinish();*/

    ImGui::SetClipboardText(suggestion.c_str());

    // clipboardText doesn't automatically update so we'll set it here
    clipboardText = suggestion;

    showOverlay = false;
}

// gemini made this thank you
std::string getClipboardText() {
    if (!OpenClipboard(nullptr)) {
        return ""; // Error or clipboard busy
    }

    // CF_TEXT exists and uses normal strings but will not get unicode characters
    std::wstring wideClipboardText = L"";
    if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (hData != nullptr) {
            wchar_t* pText = static_cast<wchar_t*>(GlobalLock(hData));
            if (pText != nullptr) {
                try {
                    wideClipboardText = pText;
                }
                catch (const std::bad_alloc&) {
                    // Handle memory allocation failure if necessary
                    wideClipboardText = L"";
                }
                GlobalUnlock(hData);
            }
        }
    }
    CloseClipboard();

    if (wideClipboardText.empty()) {
        return std::string();
    }

    // https://learn.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-widechartomultibyte?redirectedfrom=MSDN
    // Determine the required buffer size for the UTF-8 string
    // CP_UTF8: Use the UTF-8 code page
    // 0: Default flags
    // wstr.c_str(): Pointer to the wide-character string
    // -1: Indicates the string is null-terminated, let the function calculate the length
    // NULL: No buffer provided yet, we're asking for the size
    // 0: Buffer size is 0
    // NULL: Not using default char
    // NULL: Not tracking if default char was used
    int newStringSize = WideCharToMultiByte(CP_UTF8, 0, wideClipboardText.c_str(), -1, NULL, 0, NULL, NULL);
    if (newStringSize == 0) return "";

    std::string clipboardText;
    clipboardText.resize(newStringSize);

    int result = WideCharToMultiByte(CP_UTF8, 0, wideClipboardText.c_str(), -1, &clipboardText[0], newStringSize, NULL, NULL);
    // error converting wstring to string
    if (result == 0) return "";

    clipboardText.pop_back();

    return clipboardText;
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

// api key settings
// imgui.cpp 14862
// https://github.com/ocornut/imgui/issues/7489
void* UserData_ReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name) {
    return (void*)name;
}

void UserData_ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line) {
    std::cout << "READING CONFIG" << std::endl;
    std::cout << line << std::endl; 

    const char* prefix = "GeminiKey=";
    size_t prefix_len = strlen(prefix);

    if (strncmp(line, prefix, prefix_len) == 0) {
        const char* value = line + prefix_len;
        GEMINI_KEY = std::string(value);
        std::cout << "GeminiKey Loaded: " << GEMINI_KEY << std::endl;
        geminiClient = GeminiClient(GEMINI_KEY);
        if (GEMINI_KEY.empty()) {
            std::cout << "??? GeminiKey empty (default?)" << std::endl;
            shouldShowGeminiKeyPrompt = true;
        }
        else {
            shouldShowGeminiKeyPrompt = false;
        }
    }
}

void UserData_WriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf) {
    std::cout << "SAVING SETTINGS?" << std::endl;
    buf->appendf("[%s][%s]\n", "UserData", "Gemini");
    buf->appendf("GeminiKey=%s\n", GEMINI_KEY.c_str());
    buf->append("\n");
}

int main() {
    if (!glfwInit()) {
        std::cout << "GLFW failed\n";
        glfwTerminate();
        return 1;
    }

    // setting up opengl window stuff
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);
    glfwWindowHint(GLFW_DECORATED, GL_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    // always on top of everything
    glfwWindowHint(GLFW_FLOATING, GL_TRUE);

    // create window
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Test Window", NULL, NULL);
    if (!window) {
        std::cout << "window creation failed\n";
        glfwTerminate();
        return 1;
    }

    // set context
    glfwMakeContextCurrent(window);

    // load in opengl functions
    gladLoadGL();

    // get frame buffer size
    int bufferWidth, bufferHeight;
    glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);
    glViewport(0, 0, bufferWidth, bufferHeight);

    // https://stackoverflow.com/questions/66134141/glclearcolor-and-blending
    // place after window creation or program crashes
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    loadGraphics();

    // global keyboard shortcuts
    // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerhotkey?redirectedfrom=MSDN
    // https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    HWND hwnd = glfwGetWin32Window(window); // Get native window handle
    // Register ALT + Q as a hotkey
    if (!RegisterHotKey(hwnd, MY_HOTKEY_ID, MOD_ALT, 0x51)) {
        std::cerr << "Failed to register hotkey. Error code: " << GetLastError() << std::endl;
        // You might want to handle this more gracefully, maybe exit or inform the user.
    }
    else {
        std::cout << "Hotkey ALT+Q registered successfully.\n";
    }

    // init imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // set stuff up to safe setting to imgui.ini
    // https://github.com/ocornut/imgui/issues/2564
    ImGuiSettingsHandler ini_handler;
    ini_handler.TypeName = "UserData";
    ini_handler.TypeHash = ImHashStr("UserData");
    ini_handler.ReadOpenFn = UserData_ReadOpen;
    ini_handler.ReadLineFn = UserData_ReadLine;
    ini_handler.WriteAllFn = UserData_WriteAll;
    ImGui::AddSettingsHandler(&ini_handler);

    // fonts and styling
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.Fonts->AddFontDefault();
    ImFont* FontBodyRegular = io.Fonts->AddFontFromFileTTF("Outfit-Regular.ttf", 20.0f);
    ImFont* FontBodyBold = io.Fonts->AddFontFromFileTTF("Outfit-Bold.ttf", 20.0f);
    ImFont* FontDisplayRegular = io.Fonts->AddFontFromFileTTF("Outfit-Regular.ttf", 32.0f);

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





    ImGuiWindowFlags windowFlags = 0;
    windowFlags |= ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoTitleBar;
    windowFlags |= ImGuiWindowFlags_NoScrollbar;
    windowFlags |= ImGuiWindowFlags_NoScrollWithMouse;
    windowFlags |= ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoResize;
    windowFlags |= ImGuiWindowFlags_NoCollapse;
    windowFlags |= ImGuiWindowFlags_AlwaysAutoResize;

    ImGuiWindowFlags selectionWindowFlags = 0;
    selectionWindowFlags |= ImGuiWindowFlags_NoMove;
    selectionWindowFlags |= ImGuiWindowFlags_NoTitleBar;
    selectionWindowFlags |= ImGuiWindowFlags_NoMove;
    selectionWindowFlags |= ImGuiWindowFlags_NoResize;
    selectionWindowFlags |= ImGuiWindowFlags_NoCollapse;
    selectionWindowFlags |= ImGuiWindowFlags_AlwaysAutoResize;
    selectionWindowFlags |= ImGuiWindowFlags_NoBackground;

    ImGuiWindowFlags geminiStatusWindowFlags = 0;
    geminiStatusWindowFlags |= ImGuiWindowFlags_NoMove;
    geminiStatusWindowFlags |= ImGuiWindowFlags_NoTitleBar;
    geminiStatusWindowFlags |= ImGuiWindowFlags_NoMove;
    geminiStatusWindowFlags |= ImGuiWindowFlags_NoResize;
    geminiStatusWindowFlags |= ImGuiWindowFlags_NoCollapse;
    geminiStatusWindowFlags |= ImGuiWindowFlags_AlwaysAutoResize;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // ===== shader variables
    float simplexOffsetX = 0.0f;
    float simplexOffsetY = 0.0f;
    float revealStartTime = -10.0f; // Time overlay was last shown (-ve means inactive)
    float revealMouseX = 0.5f;    // Mouse X at reveal (normalized 0-1)
    float revealMouseY = 0.5f;    // Mouse Y at reveal (normalized 0-1)
    std::random_device rd;  // Obtain a random number from hardware
    std::mt19937 gen(rd()); // Seed the generator
    // Define range for random offset (large values ensure different parts of noise field)
    std::uniform_real_distribution<float> distrib(-1000.0f, 1000.0f);

    double startMouseX = 0;
    double startMouseY = 0;
    float guiWindowWidth = 500;
    float guiWindowHeight = 175;
    float guiWindowMargin = 10;

    std::string longString = "With its long, thin wings, it catches updrafts and flies like a glider high up into the sky. Should one of the six be lost, the next morning there will once more be six. It is so dense, while on a run it forgets why it started running in the first place.";

    std::string shortString = "When flames drip from its nose, that means it has a cold.";
    
    // ===== MAIN DRAW LOOP
    while (!glfwWindowShouldClose(window)) {
        // Process Windows messages (specifically looking for WM_HOTKEY)
        MSG msg = { 0 };
        // Use PeekMessage instead of GetMessage to avoid blocking if there are no messages
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            //std::cout << "WINDOWS: PARSING MESSAGES\n";
            if (msg.message == WM_HOTKEY) {
                // Check if the hotkey ID matches the one we registered
                if (msg.wParam == MY_HOTKEY_ID) {
                    std::cout << "!!!!!!!!! Global Hotkey pressed!\n";
                    showOverlay = !showOverlay; // Toggle visibility

                    if (showOverlay) {
                        simplexOffsetX = distrib(gen); // Generate random X offset
                        simplexOffsetY = distrib(gen); // Generate random Y offset
                        std::cout << "New simplex offset: (" << simplexOffsetX << ", " << simplexOffsetY << ")\n"; // Optional debug log

                        // Just became visible: Start reveal animation
                        revealStartTime = (float)glfwGetTime();

                        // Get mouse pos in screen coordinates (pixels, top-left origin)
                        glfwGetCursorPos(window, &startMouseX, &startMouseY);

                        // Normalize to [0, 1] range (bottom-left origin for shader TexCoords)
                        revealMouseX = (float)(startMouseX / WIDTH);
                        revealMouseY = 1.0f - (float)(startMouseY / HEIGHT); // Flip Y

                        // Clamp to ensure it's within [0, 1] even if cursor is off-window slightly
                        revealMouseX = std::max(0.0f, std::min(1.0f, revealMouseX));
                        revealMouseY = std::max(0.0f, std::min(1.0f, revealMouseY));

                        std::cout << "Reveal Start: t=" << revealStartTime
                            << " Pos=(" << revealMouseX << ", " << revealMouseY << ")\n";

                        // this function effectively only runs once in the loop
                        // so this should be fine...
                        clipboardText = getClipboardText();
                    } else {
                        revealStartTime = -10.0f; // Or just leave it
                    }
                }
            }
            // Important: Translate and dispatch other messages for general Windows functionality
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // get and handle user input
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (showOverlay) {
            drawGraphics(
                simplexOffsetX, simplexOffsetY,
                revealStartTime, revealMouseX, revealMouseY
            );
        }

        // imgui stuff
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        bool isClientDoingSomething = geminiClient.state != GeminiClient::IDLE;


        ImGui::Begin("DEBUG");
        ImGui::Text("OVERLAY");
        ImGui::Text("%d", showOverlay);
        ImGui::Text("CLIPBOARD");
        ImGui::Text(clipboardText.c_str());
        ImGui::Text("STATUS");
        ImGui::Text("%d", geminiClient.state);
        ImGui::Text("HTTP FEEDBACK");
        ImGui::TextWrapped(geminiClient.errorFeedback.c_str());
        ImGui::Text("GEMINI_KEY");
        ImGui::TextWrapped(GEMINI_KEY.c_str());
        ImGui::End();

        // ===== MAIN GUI STUFF
        // prompt display window
        if (showOverlay) {
            ImGui::SetNextWindowSize(ImVec2(guiWindowWidth, guiWindowHeight));
            ImGui::SetNextWindowPos(ImVec2(startMouseX, startMouseY - guiWindowHeight - guiWindowMargin), ImGuiCond_Appearing);
            ImGui::Begin("Clipboard", NULL, windowFlags);

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

        ImVec2 mouseOrigin = ImVec2(startMouseX, startMouseY);

        // interactive stuff
        if (showOverlay) {
            if (shouldShowGeminiKeyPrompt) {
                //ImGui::SetNextWindowSize(ImVec2(guiWindowWidth, guiWindowHeight));
                ImGui::SetNextWindowPos(mouseOrigin, ImGuiCond_Appearing);
                ImGui::PushFont(FontBodyRegular);

                ImGui::Begin("api key window", NULL, geminiStatusWindowFlags);


                ImGui::PushFont(FontBodyBold);
                ImGui::Text("Settings");
                ImGui::PopFont();

                // show gemini key prompt before anything else
                ImGui::TextWrapped("You'll need an API Key from AI Studio. Grab one here: ");
                ImGui::TextLinkOpenURL("https://aistudio.google.com/app/apikey");

                // https://github.com/ocornut/imgui/issues/1487
                ImGui::Dummy(ImVec2(0.0f, 10.0f));
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
            else {
                // gemini reponse selector
                if (isClientDoingSomething) {
                    ImGui::SetNextWindowSize(ImVec2(guiWindowWidth, guiWindowHeight));
                    ImGui::SetNextWindowPos(mouseOrigin, ImGuiCond_Appearing);
                    ImGui::Begin("gemini response", NULL, geminiClient.state == GeminiClient::FINISHED ? selectionWindowFlags : geminiStatusWindowFlags);
                    ImGui::PushFont(FontBodyRegular);

                    if (geminiClient.state == GeminiClient::RUNNING) {
                        ImGui::Text("Generating suggestions...");
                    }

                    if (geminiClient.state == GeminiClient::FAILED) {
                        ImGui::PushFont(FontBodyBold);
                        ImGui::TextColored(ImVec4(1.0f, 0.24f, 0.24f, 1.0f), "Sorry!");
                        ImGui::PopFont();

                        ImGui::Text("Couldn't get suggestions.");
                        ImGui::TextWrapped(geminiClient.errorFeedback.c_str());

                        ImGui::Spacing();
                        if (ImGui::Button("Try again")) {
                            geminiClient.reset();
                        }
                    }

                    /*
                    if api returns 1 option: replace clipboard
                    more than 2 show a selection thing
                    */
                    if (geminiClient.state == GeminiClient::FINISHED) {
                        for (const std::string suggestion : geminiClient.suggestions) {
                            if (ImGui::Button(suggestion.c_str())) { handleSelectionClick(suggestion); }
                        }

                        ImGui::Spacing();
                        if (ImGui::Button("Reset")) {
                            geminiClient.reset();
                        }
                    }

                    ImGui::PopFont();
                    ImGui::End();
                }

                // if clipboardText has text and client is idle show options
                if (!clipboardText.empty() && !isClientDoingSomething) {
                    // options window
                    ImGui::SetNextWindowPos(mouseOrigin, ImGuiCond_Appearing);
                    ImGui::Begin("edit options", NULL, windowFlags);

                    ImGui::PushFont(FontBodyBold);
                    //ImGui::Separator();
                    ImGui::Text("Edit Text");
                    //ImGui::SeparatorText("Edit Text");
                    ImGui::PopFont();

                    ImGui::PushFont(FontBodyRegular);

                    // https://github.com/ocornut/imgui/issues/1889
                    ImGui::BeginDisabled(isClientDoingSomething);
                    if (ImGui::Button("Synonyms for...")) { handleButtonClick(GeminiClient::PromptType::SYNONYMS); }
                    if (ImGui::Button("Rephrase...")) { handleButtonClick(GeminiClient::PromptType::REPHRASE); }
                    if (ImGui::Button("Rewrite formally...")) { handleButtonClick(GeminiClient::PromptType::FORMALIZE); }
                    if (ImGui::Button("Antonyms for...")) { handleButtonClick(GeminiClient::PromptType::ANTONYMS); }
                    if (ImGui::Button("Ungarble...")) { handleButtonClick(GeminiClient::PromptType::UNGARBLE); }
                    if (ImGui::Button("Shorten...")) { handleButtonClick(GeminiClient::PromptType::SHORTEN); }
                    ImGui::EndDisabled();

                    ImGui::PushFont(FontBodyBold);
                    //ImGui::SeparatorText("Reformat into a...");
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    ImGui::Text("Reformat into a...");
                    ImGui::PopFont();

                    ImGui::BeginDisabled(isClientDoingSomething);
                    if (ImGui::Button("Headline")) { handleButtonClick(GeminiClient::PromptType::HEADLINE); }
                    if (ImGui::Button("Tagline")) { handleButtonClick(GeminiClient::PromptType::TAGLINE); }
                    if (ImGui::Button("One word phrase")) { handleButtonClick(GeminiClient::PromptType::ONEWORD); }
                    if (ImGui::Button("Two word phrase")) { handleButtonClick(GeminiClient::PromptType::TWOWORD); }
                    ImGui::EndDisabled();

                    ImGui::PopFont();
                    ImGui::End();
                }
            }

            glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, GL_FALSE);
        }
        else {
            // let mouse clicks pass through our window to the desktop
            glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, GL_TRUE);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // show frame
        glfwSwapBuffers(window);
    }

    // cleanup
    UnregisterHotKey(hwnd, MY_HOTKEY_ID);
    std::cout << "Hotkey unregistered.\n";

    destroyGraphics();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}