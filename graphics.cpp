#include "graphics.h"
#include <Windows.h> // Needed for cursor capture functions
#include <vector>    // Needed for pixel buffer

// https://www.youtube.com/watch?v=hYZNN0MTLuc

// Simple Vertex Shader: Positions a fullscreen quad
const char* vertexShaderSource = R"(
    #version 330 core
        layout (location = 0) in vec2 aPos; // Vertex position attribute

        out vec2 TexCoords; // Pass texture coordinates to fragment shader

        void main()
        {
            // Convert vertex position (-1 to 1) to texture coordinates (0 to 1)
            // This provides a convenient 0-1 range for noise functions
            TexCoords = aPos * 0.5 + 0.5; // Correct mapping: maps [-1, 1] to [0, 1]
            //TexCoords = (aPos + 1.0) / 2.0; // Alternative way to write the same mapping
            // Output the position directly (already in clip space)
            gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
        }
)";

// Fragment Shader: Generates noise based on screen coordinates and time
// a lot of this shader is from Gemini 2.5 Pro Experimental 03-25. thanks!
const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor; // Output color for the pixel

    in vec2 TexCoords; // Received from vertex shader (range 0.0 to 1.0)

    uniform float u_time;       // Current time
    uniform float u_noiseSpeed; // How fast the random noise changes
    uniform float u_perlinScale; // Controls the scale/zoom of the Perlin mask
    uniform float u_lowerEdge; 
    uniform float u_upperEdge;
    uniform vec2 u_perlinOffset;
    uniform vec2 u_gradientOffset;

    /// --- Reveal Effect Uniforms --- <-- NEW
    uniform float u_revealStartTime; // Time the current reveal started
    uniform vec2 u_revealCenter;     // Mouse position at reveal start [0, 1]
    uniform float u_revealDuration;  // Duration of the reveal effect (e.g., 3.0s)

    // --- Simplex Noise Functions (remain the same) ---
    vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
    vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
    vec3 permute(vec3 x) { return mod289(((x*34.0)+1.0)*x); }
    float snoise(vec2 v) {
        const vec4 C = vec4(0.211324865405187, 0.366025403784439, -0.577350269189626, 0.024390243902439);
        vec2 i  = floor(v + dot(v, C.yy) );
        vec2 x0 = v -   i + dot(i, C.xx);
        vec2 i1;
        i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
        vec4 x12 = x0.xyxy + C.xxzz;
        x12.xy -= i1;
        i = mod289(i);
        vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 )) + i.x + vec3(0.0, i1.x, 1.0 ));
        vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
        m = m*m ; m = m*m ;
        vec3 x = 2.0 * fract(p * C.www) - 1.0;
        vec3 h = abs(x) - 0.5;
        vec3 ox = floor(x + 0.5);
        vec3 a0 = x - ox;
        m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
        vec3 g;
        g.x  = a0.x  * x0.x  + h.x  * x0.y;
        g.yz = a0.yz * x12.xz + h.yz * x12.yw;
        return 130.0 * dot(m, g);
    }
    // --- End Simplex Noise Functions ---

    // Alternative simple pseudo-random hash (often slightly better distribution)
    // Based on Book of Shaders examples
    float random(vec2 st) {
        return fract(sin(st.x * 12.9898 + st.y * 78.233) * 43758.5453);
    }

    // --- Easing Function (Cubic Ease-Out) --- <-- NEW HELPER
    float easeOutCubic(float t) {
      return 1.0 - pow(1.0 - t, 2.0);
    }

    void main()
    {
        // --- Calculate Random Noise (base texture) ---
        vec2 scaled_random_coords = TexCoords * 50.0;
        float randomAnimSpeedFactor = 0.0001;
        vec2 animated_random_coord = scaled_random_coords + vec2(u_time * u_noiseSpeed * randomAnimSpeedFactor);
        float randomNoiseValue = random(animated_random_coord);




        // --- Calculate Animated Perlin/Simplex Noise for Mask ---
        vec2 base_mask_coord_scaled = TexCoords * u_perlinScale;
        vec2 offset_mask_coord = base_mask_coord_scaled + u_perlinOffset; // Use mask offset
        float maskAnimSpeedFactor = 0.1; // Adjust this to control mask speed
        vec2 animated_mask_coord = offset_mask_coord + vec2(u_time * maskAnimSpeedFactor);
        float maskPerlinValue = snoise(animated_mask_coord);
        float mappedMaskPerlin = (maskPerlinValue + 1.0) * 0.5;
        // Calculate final alpha based on mask noise
        float shapedAlpha = smoothstep(u_lowerEdge, u_upperEdge, mappedMaskPerlin);
        float maskAlpha = shapedAlpha * 0.8; // Max 0.5 alpha



        // --- Calculate INDEPENDENT Perlin/Simplex Noise for COLOR GRADIENT ---
        // Uses u_gradientOffset and its own scale/animation speed
        float gradientScaleFactor = 1; // Make gradient pattern slightly larger than mask pattern? (Use 1.0 for same scale)
        vec2 base_gradient_coord_scaled = TexCoords * (u_perlinScale * gradientScaleFactor);
        vec2 offset_gradient_coord = base_gradient_coord_scaled + u_gradientOffset; // Use gradient offset <-- NEW OFFSET
        float gradientAnimSpeedFactor = 0.2; // Make color pattern move faster/slower than mask? <-- DIFFERENT SPEED
        vec2 animated_gradient_coord = offset_gradient_coord + vec2(u_time * gradientAnimSpeedFactor);
        float gradientPerlinValue = snoise(animated_gradient_coord); // Calculate separate noise value
        float mappedGradientPerlin = (gradientPerlinValue + 1.0) * 0.5; // Map to [0, 1] for color mixing




        // --- Calculate Color based on GRADIENT Perlin Value ---
        vec3 colorStart = vec3(1, 0.41, 0.47);
        vec3 colorEnd   = vec3(0.41, 0.56, 1);
        // Mix color based on the independent gradient noise value
        vec3 perlinGradientColor = mix(colorStart, colorEnd, mappedGradientPerlin);
        vec3 baseFinalColor = perlinGradientColor * randomNoiseValue; // Base color before bloom

        float baseMaskAlpha = smoothstep(u_lowerEdge, u_upperEdge, mappedMaskPerlin) * 0.8; // Base alpha before reveal

        // --- Circular Reveal Fade-In & Bloom Calculation --- <-- NEW SECTION
        float revealAlpha = 1.0; // Default to fully visible
        vec3 bloomEffect = vec3(0.0); // Default to no bloom

        float elapsedTime = u_time - u_revealStartTime;

        if (elapsedTime >= 0.0 && elapsedTime < u_revealDuration) {
            // Animation is active

            // 1. Calculate normalized linear progress (0 to 1)
            float linearProgress = elapsedTime / u_revealDuration;

            // 2. Apply easing function to the progress <-- NEW STEP
            float easedProgress = easeOutCubic(linearProgress);

            // 3. Calculate distance from current pixel to reveal center
            float dist = distance(TexCoords, u_revealCenter);

            // 4. Define the radius of the *transparent* circle expanding outwards
            float maxVisibleRadius = 1.5;
            float currentRadius = easedProgress * maxVisibleRadius; // Use eased progress for radius <-- MODIFIED

            // 5. Calculate reveal alpha: Fade in from edges (transparent circle expands)
            float revealEdgeWidth = 0.1;
            revealAlpha = 1.0 - smoothstep(currentRadius - revealEdgeWidth, currentRadius + revealEdgeWidth, dist);

            // 6. Calculate Bloom: Brightest near the edge (dist ~ currentRadius)
            float bloomEdgeWidth = 0.15;
            float distToEdge = abs(dist - currentRadius);
            float bloomAmount = (1.0 - smoothstep(0.0, bloomEdgeWidth, distToEdge));
            // Fade bloom in/out using eased progress for potentially smoother feel
            // The parabola shape still works well here: peaks at progress=0.5
            bloomAmount *= easedProgress * (1.0 - easedProgress) * 4.0; // Use easedProgress

            vec3 bloomColor = vec3(1.8, 1.8, 2.0);
            bloomEffect = bloomColor * bloomAmount * revealAlpha;

        }
        // --- End Reveal Calculation ---


        // --- Combine Final Color & Alpha ---
        vec3 finalColor = baseFinalColor + bloomEffect; // Add bloom to base color
        float finalAlpha = baseMaskAlpha * revealAlpha; // Combine Perlin alpha and reveal alpha

        FragColor = vec4(finalColor, finalAlpha);
    }
)";

    // --- New Cursor Vertex Shader ---
    const char* cursorVertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos; // Quad vertex position (0.0 to 1.0)
    layout (location = 1) in vec2 aTexCoords; // Texture coordinates

    out vec2 TexCoords;

    // Uniform providing bottom-left position (NDC) and size (NDC)
    // mat2x3: col0=translation(x,y), col1=scale(x,y) - compact way
    // Actually using mat4 for simplicity with standard transformations might be easier.
    // Let's use separate uniforms: vec2 for position, vec2 for size.
    // uniform vec2 u_cursorBottomLeftNDC;
    // uniform vec2 u_cursorSizeNDC;
    // Or even better: a single transform matrix calculated on CPU
    uniform mat4 u_transform; // Combined scale and translation matrix

    void main()
    {
        TexCoords = aTexCoords;
        // Transform local quad coords (0-1) to NDC screen position/size
        // The u_transform matrix will contain the necessary scaling and translation
        gl_Position = u_transform * vec4(aPos, 0.0, 1.0);
    }
)";

    // --- New Cursor Fragment Shader ---
    const char* cursorFragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    in vec2 TexCoords; // Texture coordinates from vertex shader

    uniform sampler2D u_cursorTexture; // The captured cursor texture

    void main()
    {
        // Sample the cursor texture
        vec4 cursorColor = texture(u_cursorTexture, TexCoords);

        // Define gradient colors (e.g., top-to-bottom, cyan to magenta)
        vec3 colorTop = vec3(0.0, 1.0, 1.0);    // Cyan
        vec3 colorBottom = vec3(1.0, 0.0, 1.0); // Magenta

        // Calculate gradient color based on vertical texture coordinate (TexCoords.y)
        // TexCoords.y goes from 0 (bottom) to 1 (top)
        vec3 gradientColor = mix(colorBottom, colorTop, TexCoords.y);

        // Overlay the gradient: Multiply the gradient with the cursor color.
        // Preserve the cursor's original alpha.
        // Use mix or other blending modes if preferred. Simple multiply might darken it.
        // Let's try lerping based on cursor alpha: show gradient in opaque parts.
        // Or additively blend? Let's try mix based on TexCoords.y for a simple overlay effect.
        // mix(cursorColor.rgb, gradientColor, 0.5) // 50% blend
        // Or overlay based on alpha:
        vec3 finalColor = mix(cursorColor.rgb, gradientColor, cursorColor.a * 0.6); // Blend gradient more where cursor is opaque

        // Output final color with original cursor alpha
        FragColor = vec4(finalColor, cursorColor.a);

        // Discard fragment if cursor texture alpha is near zero to avoid blending artifacts
        // around the cursor shape if the texture has a clear background.
        if (cursorColor.a < 0.01) {
            discard;
        }
    }
)";


GLuint VAO, VBO, EBO, shaderProgram;
GLint timeLocation;      
GLint noiseSpeedLocation;
GLint perlinScaleLocation;
GLint upperEdgeLocation; 
GLint lowerEdgeLocation;
GLint perlinOffsetLocation;
GLint revealStartTimeLocation; // <-- NEW
GLint revealCenterLocation;    // <-- NEW
GLint revealDurationLocation;  // <-- NEW

// --- New Cursor Graphics Global Variables ---
GLuint cursorVAO = 0;
GLuint cursorVBO = 0;
GLuint cursorShaderProgram = 0;
GLuint cursorTextureID = 0; // Initialize to 0
int cursorWidth = 0;
int cursorHeight = 0;
int cursorHotspotX = 0;
int cursorHotspotY = 0;
GLint cursorTexUniformLoc = -1;
GLint cursorTransformUniformLoc = -1; // Uniform for position and scale matrix

// --- New Cursor Graphics Functions ---

void loadCursorGraphics() {
    // Compile cursor shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &cursorVertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Check compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::CURSOR::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &cursorFragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Check compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::CURSOR::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Link cursor shader program
    cursorShaderProgram = glCreateProgram();
    glAttachShader(cursorShaderProgram, vertexShader);
    glAttachShader(cursorShaderProgram, fragmentShader);
    glLinkProgram(cursorShaderProgram);
    // Check link errors
    glGetProgramiv(cursorShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(cursorShaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::CURSOR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Get uniform locations
    cursorTexUniformLoc = glGetUniformLocation(cursorShaderProgram, "u_cursorTexture");
    cursorTransformUniformLoc = glGetUniformLocation(cursorShaderProgram, "u_transform");
    if (cursorTexUniformLoc == -1 || cursorTransformUniformLoc == -1) {
        std::cerr << "Warning: One or more uniforms not found in cursor shader!" << std::endl;
    }


    // Create VAO/VBO for a quad (vertices covering 0.0 to 1.0, texture coords 0.0 to 1.0)
    // Data format: x, y, u, v
    float cursorVertices[] = {
        // positions // texCoords (v is flipped)
        0.0f, 0.0f,  0.0f, 1.0f, // Bottom Left   (TexCoord V=1)
        1.0f, 0.0f,  1.0f, 1.0f, // Bottom Right  (TexCoord V=1)
        1.0f, 1.0f,  1.0f, 0.0f, // Top Right     (TexCoord V=0)

        0.0f, 0.0f,  0.0f, 1.0f, // Bottom Left   (TexCoord V=1)
        1.0f, 1.0f,  1.0f, 0.0f, // Top Right     (TexCoord V=0)
        0.0f, 1.0f,  0.0f, 0.0f  // Top Left      (TexCoord V=0)
    };

    glGenVertexArrays(1, &cursorVAO);
    glGenBuffers(1, &cursorVBO);

    glBindVertexArray(cursorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cursorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cursorVertices), cursorVertices, GL_STATIC_DRAW);

    // Position attribute (location = 0) - Stays the same
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texture coordinate attribute (location = 1) - Stays the same (data is now flipped)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Create placeholder texture (optional, can do it in capture)
    glGenTextures(1, &cursorTextureID);
    glBindTexture(GL_TEXTURE_2D, cursorTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Use nearest for pixel art cursors
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    std::cout << "Cursor graphics loaded." << std::endl;
}

void loadGraphics() {
    // Create Vertex Shader Object and get its reference
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // Attach Vertex Shader source to the Vertex Shader Object
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    // Compile the Vertex Shader into machine code
    glCompileShader(vertexShader);

    // Check for compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }


    // Create Fragment Shader Object and get its reference
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // Attach Fragment Shader source to the Fragment Shader Object
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    // Compile the Vertex Shader into machine code
    glCompileShader(fragmentShader);

    // Check for compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Create Shader Program Object and get its reference
    shaderProgram = glCreateProgram();
    // Attach the Vertex and Fragment Shaders to the Shader Program
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    // Wrap-up/Link all the shaders together into the Shader Program
    glLinkProgram(shaderProgram);

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    //  Delete shaders as they're linked into program now and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    // 3. Set up vertex data and buffers for a fullscreen quad
    // Vertices cover [-1, 1] in x and y (Normalized Device Coordinates)
    // Two triangles make a quad
    float vertices[] = {
        // positions
        -1.0f,  1.0f, // top left
        -1.0f, -1.0f, // bottom left
         1.0f, -1.0f, // bottom right

        -1.0f,  1.0f, // top left
         1.0f, -1.0f, // bottom right
         1.0f,  1.0f  // top right
    };

    // Create reference containers for the Vartex Array Object, the Vertex Buffer Object, and the Element Buffer Object

    // Generate the VAO, VBO, and EBO with only 1 object each
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    //glGenBuffers(1, &EBO);

    // Make the VAO the current Vertex Array Object by binding it
    glBindVertexArray(VAO);

    // Bind the VBO specifying it's a GL_ARRAY_BUFFER
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Introduce the vertices into the VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Bind the EBO specifying it's a GL_ELEMENT_ARRAY_BUFFER
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //// Introduce the indices into the EBO
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Configure vertex attributes (tell OpenGL how to interpret the VBO data)
    // layout (location = 0) in vec2 aPos; <- in vertex shader
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); // Enable vertex attribute 0

    // Bind both the VBO and VAO to 0 so that we don't accidentally modify the VAO and VBO we created
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // Bind the EBO to 0 so that we don't accidentally modify it
    // MAKE SURE TO UNBIND IT AFTER UNBINDING THE VAO, as the EBO is linked in the VAO
    // This does not apply to the VBO because the VBO is already linked to the VAO during glVertexAttribPointer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Get the location of the 'time' uniform in the shader program
    timeLocation = glGetUniformLocation(shaderProgram, "u_time");
    noiseSpeedLocation = glGetUniformLocation(shaderProgram, "u_noiseSpeed");
    perlinScaleLocation = glGetUniformLocation(shaderProgram, "u_perlinScale");
    upperEdgeLocation = glGetUniformLocation(shaderProgram, "u_upperEdge");
    lowerEdgeLocation = glGetUniformLocation(shaderProgram, "u_lowerEdge");
    perlinOffsetLocation = glGetUniformLocation(shaderProgram, "u_perlinOffset");
    revealStartTimeLocation = glGetUniformLocation(shaderProgram, "u_revealStartTime");
    revealCenterLocation = glGetUniformLocation(shaderProgram, "u_revealCenter");
    revealDurationLocation = glGetUniformLocation(shaderProgram, "u_revealDuration");

    // Update error check
    if (timeLocation == -1 || noiseSpeedLocation == -1 || perlinScaleLocation == -1 ||
        upperEdgeLocation == -1 || lowerEdgeLocation == -1 || perlinOffsetLocation == -1 ||
        revealStartTimeLocation == -1 || revealCenterLocation == -1 || revealDurationLocation == -1) { // <-- Check reveal uniforms
        std::cerr << "Warning: One or more uniforms not found in shader!" << std::endl;
    }
}

bool captureCursorTexture() {
    CURSORINFO ci = { sizeof(ci) };
    if (!GetCursorInfo(&ci)) {
        std::cerr << "Failed to get cursor info. Error: " << GetLastError() << std::endl;
        return false;
    }

    // Check if cursor is visible
    if (!(ci.flags & CURSOR_SHOWING)) {
        std::cerr << "Cursor is not visible." << std::endl;
        // Optionally clear existing texture or return false
        // Let's clear width/height so it doesn't draw old cursor
        cursorWidth = 0;
        cursorHeight = 0;
        return false; // Indicate nothing was captured
    }

    HCURSOR hCursor = ci.hCursor;
    if (!hCursor) {
        std::cerr << "Failed to get cursor handle." << std::endl;
        return false;
    }

    ICONINFOEX info = { sizeof(info) };
    if (!GetIconInfoEx(hCursor, &info)) {
        std::cerr << "Failed to get icon info ex. Error: " << GetLastError() << std::endl;
        return false;
    }

    // Important: GetIconInfoEx creates bitmaps, we need to delete them!
    HBITMAP hbmColor = info.hbmColor;
    HBITMAP hbmMask = info.hbmMask;

    BITMAP bmColor = { 0 };
    int colorBitmapValid = GetObject(hbmColor, sizeof(bmColor), &bmColor);

    if (!colorBitmapValid || bmColor.bmWidth <= 0 || bmColor.bmHeight <= 0) {
        std::cerr << "Failed to get cursor color bitmap info or invalid size." << std::endl;
        // Need to clean up the mask bitmap if it exists
        if (hbmMask) DeleteObject(hbmMask);
        if (hbmColor) DeleteObject(hbmColor); // Also delete color one even if info failed
        return false;
    }

    cursorWidth = bmColor.bmWidth;
    cursorHeight = bmColor.bmHeight;
    // Hotspot from ICONINFOEX is relative to top-left of the cursor image
    cursorHotspotX = info.xHotspot;
    cursorHotspotY = info.yHotspot;

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmColor);

    // Prepare buffer for pixel data
    // We assume 32bpp BGRA format, which is common for modern cursors with alpha.
    // If GetObject reported bmColor.bmBitsPixel != 32, more complex handling is needed.
    int bufferSize = cursorWidth * cursorHeight * 4; // 4 bytes per pixel (BGRA)
    std::vector<unsigned char> pixels(bufferSize);

    BITMAPINFO bmi = { sizeof(BITMAPINFOHEADER) };
    bmi.bmiHeader.biWidth = cursorWidth;
    bmi.bmiHeader.biHeight = -cursorHeight; // Negative height for top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32; // Request 32bpp BGRA
    bmi.bmiHeader.biCompression = BI_RGB;

    // Get the pixel data
    if (!GetDIBits(hdcMem, hbmColor, 0, cursorHeight, pixels.data(), &bmi, DIB_RGB_COLORS)) {
        std::cerr << "Failed to get DIBits for cursor. Error: " << GetLastError() << std::endl;
        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);
        DeleteObject(hbmColor);
        DeleteObject(hbmMask);
        return false;
    }

    // Upload to OpenGL Texture
    glBindTexture(GL_TEXTURE_2D, cursorTextureID);
    // Assuming Windows gave us BGRA, tell OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cursorWidth, cursorHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels.data());
    // Set filtering again just in case context changed? Or rely on loadCursorGraphics setup.
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);


    // Cleanup Win32 resources
    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
    DeleteObject(hbmColor);
    DeleteObject(hbmMask);

    std::cout << "Cursor captured: " << cursorWidth << "x" << cursorHeight
        << " Hotspot: (" << cursorHotspotX << "," << cursorHotspotY << ")" << std::endl;

    return true;
}

void drawCursor(float normX, float normY, int windowWidth, int windowHeight) {
    if (cursorTextureID == 0 || cursorWidth <= 0 || cursorHeight <= 0) {
        return; // Don't draw if texture isn't valid or wasn't captured
    }

    glUseProgram(cursorShaderProgram);

    // Calculate position and size in Normalized Device Coordinates (NDC) [-1, 1]
    // normX, normY are [0, 1] bottom-left origin

    // Target position (hotspot) in NDC
    float targetX_NDC = normX * 2.0f - 1.0f;
    float targetY_NDC = normY * 2.0f - 1.0f;

    // Cursor dimensions in NDC
    float cursorWidth_NDC = (float)cursorWidth / windowWidth * 2.0f;
    float cursorHeight_NDC = (float)cursorHeight / windowHeight * 2.0f;

    // Hotspot offset from top-left corner in NDC units
    float hotspotX_Offset_NDC = (float)cursorHotspotX / windowWidth * 2.0f;
    // Hotspot Y offset from *top* in NDC units (positive down in Win32, positive up in NDC)
    float hotspotY_Offset_NDC = (float)cursorHotspotY / windowHeight * 2.0f;

    // Calculate the bottom-left corner of the quad in NDC
    float bottomLeftX_NDC = targetX_NDC - hotspotX_Offset_NDC;
    // Adjust Y: target is hotspot; hotspotY offset is from top; quad origin is bottom-left.
    // Position of top-left = (targetX - hotspotX_Offset, targetY + hotspotY_Offset)
    // Position of bottom-left = (top-left X, top-left Y - height)
    // bottomLeftY_NDC = targetY_NDC + hotspotY_Offset_NDC - cursorHeight_NDC; // This seems overly complex, rethink.

    // Let's use a transformation matrix approach.
    // 1. Scale matrix: Scale the 0->1 quad to cursor size in NDC
    // 2. Translation matrix: Move the scaled quad so its hotspot aligns with target NDC.
    // The origin (0,0) of our quad model is bottom-left.
    // We want to move this origin such that the pixel corresponding to the hotspot
    // lands at (targetX_NDC, targetY_NDC).
    // Hotspot position *within the quad* in NDC coords (relative to bottom-left):
    float hotspotRelX_NDC = hotspotX_Offset_NDC;
    float hotspotRelY_NDC = cursorHeight_NDC - hotspotY_Offset_NDC; // Y is flipped

    // Translation needed = Target Position - Relative Hotspot Position
    float translateX = targetX_NDC - hotspotRelX_NDC;
    float translateY = targetY_NDC - hotspotRelY_NDC;

    // Create transformation matrix (Scale then Translate)
    // Using basic math, not a matrix library
    // Scale:
    // [ sx 0  0  0 ]
    // [ 0  sy 0  0 ]
    // [ 0  0  1  0 ]
    // [ 0  0  0  1 ]
    // Translate:
    // [ 1  0  0  tx ]
    // [ 0  1  0  ty ]
    // [ 0  0  1  0 ]
    // [ 0  0  0  1 ]
    // Combined (Translate * Scale): T * S
    // [ sx 0  0  tx ]
    // [ 0  sy 0  ty ]
    // [ 0  0  1  0 ]
    // [ 0  0  0  1 ]
    float sx = cursorWidth_NDC;
    float sy = cursorHeight_NDC;
    float tx = translateX;
    float ty = translateY;

    // Column-major order for OpenGL mat4 uniform
    float transformMatrix[16] = {
        sx,   0.0f, 0.0f, 0.0f, // Col 0
        0.0f, sy,   0.0f, 0.0f, // Col 1
        0.0f, 0.0f, 1.0f, 0.0f, // Col 2
        tx,   ty,   0.0f, 1.0f  // Col 3
    };


    glUniformMatrix4fv(cursorTransformUniformLoc, 1, GL_FALSE, transformMatrix);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cursorTextureID);
    glUniform1i(cursorTexUniformLoc, 0); // Tell shader sampler uses texture unit 0

    // Bind VAO and draw
    glBindVertexArray(cursorVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6); // 6 vertices for the two triangles of the quad

    // Unbind VAO
    glBindVertexArray(0);
    // Unbind Texture (optional good practice)
    glBindTexture(GL_TEXTURE_2D, 0);
    // Switch back to default shader? No, main loop handles this.
}

void destroyCursorGraphics() {
    glDeleteTextures(1, &cursorTextureID);
    glDeleteVertexArrays(1, &cursorVAO);
    glDeleteBuffers(1, &cursorVBO);
    glDeleteProgram(cursorShaderProgram);

    cursorTextureID = 0; // Reset state
    cursorWidth = 0;
    cursorHeight = 0;
    cursorShaderProgram = 0;
    cursorVAO = 0;
    cursorVBO = 0;

    std::cout << "Cursor graphics destroyed." << std::endl;
}


// Modify drawGraphics signature to accept reveal parameters <-- MODIFIED SIGNATURE
void drawGraphics(
    float noiseSpeed, float perlinScale, float lowerEdge, float upperEdge,
    float perlinOffsetX, float perlinOffsetY, float gradientOffsetX, float gradientOffsetY,
    float revealStartTime, float revealMouseX, float revealMouseY, float revealDuration, // NEW PARAMS
    int bufferWidth, int bufferHeight
) {
    glUseProgram(shaderProgram);

    // 2. Update the time uniform
    float currentTime = (float)glfwGetTime();
    //std::cout << "Current Time: " << revealStartTime << std::endl;
    glUniform1f(timeLocation, currentTime);
    glUniform1f(noiseSpeedLocation, noiseSpeed);
    glUniform1f(perlinScaleLocation, perlinScale);
    glUniform1f(upperEdgeLocation, upperEdge);
    glUniform1f(lowerEdgeLocation, lowerEdge);
    glUniform2f(perlinOffsetLocation, perlinOffsetX, perlinOffsetY);

    glUniform1f(revealStartTimeLocation, revealStartTime);
    glUniform2f(revealCenterLocation, revealMouseX, revealMouseY);
    glUniform1f(revealDurationLocation, revealDuration);

    // Bind the VAO so OpenGL knows to use it
    glBindVertexArray(VAO);

    // 4. Draw the quad (6 vertices)
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void destroyGraphics() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    //glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
}