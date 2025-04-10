#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
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

const GLint WIDTH = 1924, HEIGHT = 1084;
bool showOverlay = true;
std::string clipboardText = "";

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
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.Fonts->AddFontDefault();
    ImFont* FontBodyRegular = io.Fonts->AddFontFromFileTTF("Outfit-Regular.ttf", 20.0f);
    ImFont* FontBodyBold = io.Fonts->AddFontFromFileTTF("Outfit-Bold.ttf", 20.0f);
    ImFont* FontDisplayRegular = io.Fonts->AddFontFromFileTTF("Outfit-Regular.ttf", 32.0f);

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize = 1.f;
    style.WindowPadding = ImVec2(16.0f, 16.0f);;
    style.WindowRounding = 8.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 12.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 12.0f;

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
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
    colors[ImGuiCol_Button] = ImVec4(0.27f, 0.28f, 0.30f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.37f, 0.40f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.43f, 0.48f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.15f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.24f, 0.25f, 0.27f, 1.00f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.15f, 0.45f, 1.00f, 0.90f);

    ImGuiWindowFlags windowFlags = 0;
    windowFlags |= ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoTitleBar;
    windowFlags |= ImGuiWindowFlags_NoScrollbar;
    windowFlags |= ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoResize;
    windowFlags |= ImGuiWindowFlags_NoCollapse;
    windowFlags |= ImGuiWindowFlags_AlwaysAutoResize;

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

        // ===== MAIN GUI STUFF
        if (showOverlay) {
            // prompt display window
            ImGui::SetNextWindowSize(ImVec2(guiWindowWidth, guiWindowHeight));
            ImGui::SetNextWindowPos(ImVec2(startMouseX, startMouseY - guiWindowHeight - guiWindowMargin), ImGuiCond_Appearing);
            ImGui::Begin("prompt preview", NULL, windowFlags);

            ImGui::PushFont(FontBodyBold);
            ImGui::Text("Clipboard");
            ImGui::PopFont();

            ImGui::PushFont(FontDisplayRegular);
            ImGui::TextWrapped(longString.c_str());
            ImGui::PopFont();
            ImGui::End();
            

            // options window
            ImGui::SetNextWindowPos(ImVec2(startMouseX, startMouseY), ImGuiCond_Appearing);
            ImGui::Begin("edit options", NULL, windowFlags);

            ImGui::PushFont(FontBodyBold);
            //ImGui::Separator();
            ImGui::Text("Edit Text");
            //ImGui::SeparatorText("Edit Text");
            ImGui::PopFont();

            ImGui::PushFont(FontBodyRegular);

            /*
            if api returns 1 option: replace clipboard
            more than 2 show a selection thing
            */
            ImGui::Text("Synonyms for...");
            ImGui::Text("Rephrase...");
            ImGui::Text("Rewrite formally...");
            ImGui::Text("Antonyms for...");
            ImGui::Text("Ungarble...");

            ImGui::PushFont(FontBodyBold);
            //ImGui::SeparatorText("Reformat into a...");
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("Reformat into a...");
            ImGui::PopFont();

            ImGui::Text("Headline");
            ImGui::Text("Tagline");
            ImGui::Text("One word phrase");
            ImGui::Text("Two word phrase");

            /*if (clipboardText.empty()) {
                ImGui::Text("!!! NO TEXT");
            }
            else {
                ImGui::Text("%s", clipboardText.c_str());
            }*/

            ImGui::PopFont();
            ImGui::End();

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