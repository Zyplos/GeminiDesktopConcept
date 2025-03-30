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

float revealStartTime = -10.0f; // Time overlay was last shown (-ve means inactive)
float revealMouseX = 0.5f;    // Mouse X at reveal (normalized 0-1)
float revealMouseY = 0.5f;    // Mouse Y at reveal (normalized 0-1)
const float REVEAL_DURATION = 3.f; // Duration of the effect in seconds <-- NEW

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

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    float noiseSpeed = 0.080f;
    float perlinScale = 1.8f;
    float lowerEdge = 0.0f;
    float upperEdge = 1.6f;

    float perlinOffsetX = 0.0f; // Start at zero offset
    float perlinOffsetY = 0.0f; // Start at zero offset
    float gradientOffsetX = 0.0f; // Start at zero offset
    float gradientOffsetY = 0.0f; // Start at zero offset
    std::random_device rd;  // Obtain a random number from hardware
    std::mt19937 gen(rd()); // Seed the generator
    // Define range for random offset (large values ensure different parts of noise field)
    std::uniform_real_distribution<float> distrib(-1000.0f, 1000.0f);
    

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
                        perlinOffsetX = distrib(gen); // Generate random X offset
                        perlinOffsetY = distrib(gen); // Generate random Y offset
                        gradientOffsetX = distrib(gen); // Generate random X offset
                        gradientOffsetY = distrib(gen); // Generate random Y offset
                        std::cout << "New Perlin Offset: (" << perlinOffsetX << ", " << perlinOffsetY << ")\n"; // Optional debug log
                        std::cout << "New Perlin Offset: (" << gradientOffsetX << ", " << gradientOffsetY << ")\n"; // Optional debug log



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
                    }
                    else {
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
                noiseSpeed, perlinScale, lowerEdge, upperEdge,
                perlinOffsetX, perlinOffsetY, gradientOffsetX, gradientOffsetY,
                revealStartTime, revealMouseX, revealMouseY, REVEAL_DURATION,
                bufferWidth, bufferHeight
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

            ImGui::Begin("test window");
            ImGui::Text("Press ALT+Q to toggle overlay.");
            // 12 is a nice enough value
            ImGui::DragFloat("Noise Speed", &noiseSpeed, 0.0f, 0.01f); // Min 0, Max 10
            ImGui::SliderFloat("Perlin Scale", &perlinScale, 1.0f, 20.0f);
            ImGui::SliderFloat("Lower Edge", &lowerEdge, 0.0f, 1.0f);
            ImGui::SliderFloat("Upper Edge", &upperEdge, 0.0f, 1.0f);
            ImGui::Text("Perlin Offset: (%.1f, %.1f)", perlinOffsetX, perlinOffsetY);
            ImGui::Text("Gradient Offset: (%.1f, %.1f)", gradientOffsetX, gradientOffsetY);
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