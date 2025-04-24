#include <iostream>
#include <random>
#include <algorithm>

// gemini suggested these for windows include
// https://stackoverflow.com/questions/11040133/what-does-defining-win32-lean-and-mean-exclude-exactly
#define WIN32_LEAN_AND_MEAN
// windows messes with min/max, this fixes a warning
#define NOMINMAX
#include <Windows.h>

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

// NOMINMAX fixes a warning but keep these just in case
#undef max
#undef min

#define MY_HOTKEY_ID 137

#include "graphics.h"
#include "geminiAPI.hpp"
#include "gui.h"

GLFWwindow* window;

GLint WIDTH = 800;
GLint HEIGHT = 800;
int bufferWidth, bufferHeight;

bool showOverlay = false;
bool superWindow = false;

std::string clipboardText = "";
GeminiClient geminiClient("");
std::string GEMINI_KEY = "";

double startMouseX = 500;
double startMouseY = 500;

// if imgui.ini doesnt exist then UserData_ReadLine doesnt fire
// so show the key prompt by default. UserData_ReadLine will set this to false on launch
// this also acts as a firstRun variable
bool shouldShowGeminiKeyPrompt = true;

void selectOptionEventHandler(GeminiClient::PromptType type) {
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

void handleSuggestionClick(std::string suggestion) {
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

void updateStartMouseCoords() {
    glfwGetCursorPos(window, &startMouseX, &startMouseY);
    std::cout << "UPDATEMOUSECOORDS | x " << startMouseX << " y " << startMouseY << std::endl;
}

// super window. spans all monitors
// credit chatgpt for this snippet
void enableSuperWindow() {
    int monitorCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

    if (monitorCount == 0) {
        std::cerr << "No monitors found??\n";
        glfwTerminate();
        exit(1);
    }

    int minX = INT_MAX;
    int minY = INT_MAX;
    int maxX = INT_MIN;
    int maxY = INT_MIN;

    // Determine bounding box that covers all monitors
    for (int i = 0; i < monitorCount; ++i) {
        int x, y;
        glfwGetMonitorPos(monitors[i], &x, &y);

        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
        if (!mode) continue;

        int width = mode->width;
        int height = mode->height;

        minX = std::min(minX, x);
        minY = std::min(minY, y);
        maxX = std::max(maxX, x + width);
        maxY = std::max(maxY, y + height);
    }

    WIDTH = maxX - minX;
    HEIGHT = maxY - minY;

    // add a little bit to fix a bug where the screen turns all black
    WIDTH += 4;
    HEIGHT += 4;

    std::cout << "Super window size: (" << minX << ", " << minY << ") with size "
        << WIDTH << "x" << HEIGHT << "\n";

    glfwSetWindowSize(window, WIDTH, HEIGHT);
    glfwSetWindowPos(window, minX, minY);
    glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);
    glViewport(0, 0, bufferWidth, bufferHeight);
}

// moves window to whatever windows deems the main monitor
void setupPrimaryMonitor() {
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    if (!primaryMonitor) {
        std::cout << "couldn't get primary monitor" << std::endl;
        exit(1);
    }

    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    int monitorX, monitorY;
    glfwGetMonitorPos(primaryMonitor, &monitorX, &monitorY);

    if (!mode) {
        std::cout << "couldn't get required monitor properties" << std::endl;
        exit(1);
    }

    WIDTH = mode->width;
    HEIGHT = mode->height;

    // add a little bit to fix a bug where the screen turns all black
    WIDTH += 4;
    HEIGHT += 4;

    glfwSetWindowPos(window, monitorX, monitorY);
    glfwSetWindowSize(window, WIDTH, HEIGHT);
    glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);
    glViewport(0, 0, bufferWidth, bufferHeight);
}

// enableSuperWindow() can fail but this has to succeed
// this function will call exit() if it cant get any monitors
void setupOverlayInActiveMonitor() {
    // ===== get cursor position or use main monitor if it fails
    POINT cursorPos;
    if (!GetCursorPos(&cursorPos)) {
        std::cerr << "!!! setupOverlayInActiveMonitor GetCursorPos failed" << std::endl;
        setupPrimaryMonitor();
        return;
    }

    // ===== grab available monitors
    int monitorCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    if (monitorCount == 0) {
        std::cout << "couldn't get any kind of monitor???" << std::endl;
        glfwTerminate();
        exit(1);
    }

    // ===== loop through monitors to find active one
    GLFWmonitor* targetMonitor = nullptr;
    const GLFWvidmode* targetMode = nullptr;
    int targetMonitorX = 0; 
    int targetMonitorY = 0;
    for (int i = 0; i < monitorCount; ++i) {
        GLFWmonitor* currentMonitor = monitors[i];
        int monitorX;
        int monitorY;

        glfwGetMonitorPos(currentMonitor, &monitorX, &monitorY);
        const GLFWvidmode* mode = glfwGetVideoMode(currentMonitor);
        if (!mode) {
            // this might make it so no monitor is found but we catch that after this for loop
            continue;
        }

        if (cursorPos.x >= monitorX && cursorPos.x < (monitorX + mode->width) &&
            cursorPos.y >= monitorY && cursorPos.y < (monitorY + mode->height)) {
            targetMonitor = currentMonitor;
            targetMonitorX = monitorX;
            targetMonitorY = monitorY;
            targetMode = mode;
            std::cout << "!!!!!!!!!! Cursor found on monitor " << i << ": " << glfwGetMonitorName(currentMonitor) << std::endl;
            break;
        }
    }

    if (!targetMonitor) {
        setupPrimaryMonitor();
        return;
    }

    std::cout << "!!! MONITOR CORNER x " << targetMonitorX << " y " << targetMonitorY
        << " | dimensions " << targetMode->width << "x" << targetMode->height << std::endl;

    WIDTH = targetMode->width;
    HEIGHT = targetMode->height;

    // add a little bit to fix a bug where the screen turns all black
    WIDTH += 4;
    HEIGHT += 4;

    glfwSetWindowPos(window, targetMonitorX, targetMonitorY);
    glfwSetWindowSize(window, WIDTH, HEIGHT);
    glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);
    glViewport(0, 0, bufferWidth, bufferHeight);
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

    int i;
    if (sscanf_s(line, "SuperWindow=%d", &i) == 1) { 
        superWindow = (i != 0); 
        if (superWindow) {
            std::cout << "ENABLING SUPER WINDOWN IN SETTINGS\n";
            enableSuperWindow();
        }
    }
}

void UserData_WriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf) {
    std::cout << "SAVING SETTINGS?" << std::endl;
    buf->appendf("[%s][%s]\n", "UserData", "Gemini");
    buf->appendf("GeminiKey=%s\n", GEMINI_KEY.c_str());
    buf->appendf("SuperWindow=%d\n", superWindow);
    buf->append("\n");
}




//int WINAPI WinMain(
//    _In_ HINSTANCE hInstance,
//    _In_opt_ HINSTANCE hPrevInstance,
//    _In_ LPSTR lpCmdLine,
//    _In_ int nShowCmd
//) {
int main() {
    if (!glfwInit()) {
        std::cout << "GLFW failed\n";
        glfwTerminate();
        return 1;
    }

    // set up 
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    if (!primaryMonitor) {
        std::cout << "couldn't get primary monitor" << std::endl;
        return 1;
    }

    const GLFWvidmode* monitorMode = glfwGetVideoMode(primaryMonitor);
    if (!monitorMode) {
        std::cout << "couldn't get monitor properties to get dimensions" << std::endl;
        return -1;
    }
    
    WIDTH = monitorMode->width;
    HEIGHT = monitorMode->height;

    // add a little bit to fix a bug where the screen turns all black
    WIDTH += 4;
    HEIGHT += 4;

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
    window = glfwCreateWindow(WIDTH, HEIGHT, "Latent Writer", NULL, NULL);
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

    GuiHandler guiHandler;
    guiHandler.setupStyles();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // ===== shader variables
    float simplexOffsetX = 0.0f;
    float simplexOffsetY = 0.0f;
    float revealStartTime = -10.0f;   // Time overlay was last shown
    float revealMouseX = 0.5f;        // Mouse X at reveal (normalized 0-1)
    float revealMouseY = 0.5f;        // Mouse Y at reveal (normalized 0-1)
    std::random_device rd;            // Obtain a random number from hardware
    std::mt19937 gen(rd());           // Seed the generator
    // Define range for random offset (large values ensure different parts of noise field)
    std::uniform_real_distribution<float> distrib(-1000.0f, 1000.0f);



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

                        // move and resize window to active monitor
                        // super window handled in settings functions
                        if (!superWindow) {
                            setupOverlayInActiveMonitor();
                        }

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

                        // check if clipboardText only has whitespaces
                        // https://stackoverflow.com/a/18240446
                        if (std::all_of(clipboardText.begin(), clipboardText.end(), isspace)) {
                            clipboardText = "";
                        }

                        // sometimes the overlay gets put in the background
                        // sometimes inputs stay on the window the user was on before calling the overlay
                        // this should fix this
                        glfwFocusWindow(window);
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

        ImGui::PushFont(guiHandler.FontBodyRegular);

        /*ImGui::Begin("DEBUG");
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
        ImGui::End();*/

        guiHandler.mouseOrigin = ImVec2(static_cast<float>(startMouseX), static_cast<float>(startMouseY));

        // firstRun prompt to show keybind
        if (shouldShowGeminiKeyPrompt && !showOverlay) {
            double followingMouseX = 0;
            double followingMouseY = 0;
            glfwGetCursorPos(window, &followingMouseX, &followingMouseY);
            guiHandler.drawFirstRunPrompt(ImVec2(static_cast<float>(followingMouseX), static_cast<float>(followingMouseY)));
        }

        // ===== MAIN GUI STUFF
        // prompt display window
        if (showOverlay) {
            guiHandler.drawClipboardWindow(clipboardText, shouldShowGeminiKeyPrompt);
        }

        // interactive stuff
        if (showOverlay) {
            if (shouldShowGeminiKeyPrompt) {
                guiHandler.drawSettingsWindow(
                    GEMINI_KEY,
                    geminiClient,
                    shouldShowGeminiKeyPrompt,
                    superWindow,
                    enableSuperWindow,
                    setupOverlayInActiveMonitor,
                    updateStartMouseCoords
                );
            }
            else {
                // gemini reponse selector
                if (geminiClient.state == GeminiClient::RUNNING) {
                    guiHandler.drawAPIRunningState(geminiClient);
                }

                if (geminiClient.state == GeminiClient::FAILED) {
                    guiHandler.drawAPIFailedState(geminiClient);
                }

                if (geminiClient.state == GeminiClient::FINISHED) {
                    guiHandler.drawAPIFinishedState(geminiClient, handleSuggestionClick);
                }

                // if clipboardText has text and client is idle show options
                if (!clipboardText.empty() && !geminiClient.isClientDoingSomething()) {
                    guiHandler.drawEditOptionsWindow(geminiClient, selectOptionEventHandler);
                }
            }

            glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, GL_FALSE);
        }
        else {
            // let mouse clicks pass through our window to the desktop
            glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, GL_TRUE);
        }

        ImGui::PopFont();

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