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
    ImFont* font1 = io.Fonts->AddFontFromFileTTF("Outfit-Regular.ttf", 20.0f);

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
                        double mouseX_pixels, mouseY_pixels;
                        glfwGetCursorPos(window, &mouseX_pixels, &mouseY_pixels);

                        // Normalize to [0, 1] range (bottom-left origin for shader TexCoords)
                        revealMouseX = (float)(mouseX_pixels / WIDTH);
                        revealMouseY = 1.0f - (float)(mouseY_pixels / HEIGHT); // Flip Y

                        // Clamp to ensure it's within [0, 1] even if cursor is off-window slightly
                        revealMouseX = std::max(0.0f, std::min(1.0f, revealMouseX));
                        revealMouseY = std::max(0.0f, std::min(1.0f, revealMouseY));

                        std::cout << "Reveal Start: t=" << revealStartTime
                            << " Pos=(" << revealMouseX << ", " << revealMouseY << ")\n";
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

        if (showOverlay) {
            ImVec2 mousePosition = ImGui::GetMousePos();
            if (!ImGui::IsMousePosValid(&mousePosition)) {
                ImVec2 center = ImVec2(WIDTH / 2, HEIGHT / 2);
                std::cout << "mouse not available??\n";
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing);
            }
            else {
                //std::cout << "screen pos: " << mousePosition.x << ", " << mousePosition.y << "\n";
                ImGui::SetNextWindowPos(mousePosition, ImGuiCond_Appearing);
            }

            ImGui::PushFont(font1);
            ImGui::Begin("test window", NULL, windowFlags);

            ImGui::Text("Press ALT+Q to toggle overlay.");
            ImGui::Text("Simplex Offset: (%.1f, %.1f)", simplexOffsetX, simplexOffsetY);
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