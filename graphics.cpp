#include "graphics.h"
// https://www.youtube.com/watch?v=hYZNN0MTLuc

// this shader was basically made by Gemini 2.5 Pro Experimental 03-25. thanks!
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
        // Output the position directly (already in clip space)
        gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    }
)";
// Fragment Shader: Generates noise based on screen coordinates and time
const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor; // Output color for the pixel
    in vec2 TexCoords; // Received from vertex shader (range 0.0 to 1.0)
    
    // ===== Noise factors
    uniform float u_time; 
    const float u_noiseSpeed = 0.08;
    
    // ===== Simplex noise
    const float u_simplexScale = 1.8; // Controls the scale/zoom of the simplex mask
    const float u_lowerEdge = 0.0; 
    const float u_upperEdge = 1.6;
    uniform vec2 u_simplexOffset;
    
    // ===== Reveal effect uniforms
    uniform float u_revealStartTime; // Time the current reveal started
    uniform vec2 u_revealCenter;     // Mouse position at reveal start [0, 1]
    const float u_revealDuration = 3.0;  // Duration of the reveal effect (e.g., 3.0s)
    
    // ===== Simplex moise functions
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
    
    // ===== Alternative simple pseudo-random hash (often slightly better distribution)
    // Based on Book of Shaders examples
    float random(vec2 st) {
        return fract(sin(st.x * 12.9898 + st.y * 78.233) * 43758.5453);
    }
    
    // ===== Easing function (ease-out)
    float easeOutCubic(float t) {
      return 1.0 - pow(1.0 - t, 3.0);
    }
    
    // ===== Main drawing function
    void main()
    {
        // ===== Calculate random noise (base texture)
        // scaling base coords helps prevent weird banding effect when drawing noise
        vec2 scaledNoiseCoords = TexCoords * 50.0; //  arbitrary scale to get more noise detail, 50.0 does the job
        float noiseSpeedSlowFactor = 0.0001; // make the noise move even slower
        vec2 animatedRandomCoord = scaledNoiseCoords + vec2(u_time * u_noiseSpeed * noiseSpeedSlowFactor);
        // final random noise value used for base texture, mixed with simplex noise later for gradient
        float randomNoiseValue = random(animatedRandomCoord); 
    
    
    
        // ===== Calculate base value for noise mask using simplex noise
        vec2 scaledNoiseMaskCoords = TexCoords * u_simplexScale;
        // random offset helps so the effect doesn't look static every time its opened
        vec2 maskCoordOffset = scaledNoiseMaskCoords + u_simplexOffset; 
        float maskSpeed = 0.1; // Adjust this to control mask speed
        vec2 animatedMaskCoord = maskCoordOffset + vec2(u_time * maskSpeed);
        float maskSimplexValue = snoise(animatedMaskCoord);
        float mappedMaskValue = (maskSimplexValue + 1.0) * 0.5; // Map to [0, 1] range for consistency with random noise
        // value used to set alpha for the final color, creating the mask effect, mixed later with bloom for final value
        // use smoothstep to create a soft transition for the mask edges
        // multiply to limit the alpha value to be below a certain threshold, creating a more subtle effect (0.8 is good)
        float baseMaskAlphaValue = smoothstep(u_lowerEdge, u_upperEdge, mappedMaskValue) * 0.8;
    
    
    
        // ===== Calculate color gradient using the same simplex mask coords but sped up so the color moves independently of mask
        float gradientSpeed = 0.2;
        vec2 animatedGradientCoord = scaledNoiseMaskCoords + vec2(u_time * gradientSpeed); // use same mask coords
        float colorSimplexValue = snoise(animatedGradientCoord); // Calculate separate noise value
        float baseColorValue = (colorSimplexValue + 1.0) * 0.5; // Map to [0, 1] for color mixing
    
    
    
        // ===== Calculate final color in gradient by mixing the colors, simplex value, and random noise value
        vec3 colorStart = vec3(1, 0.41, 0.47);
        vec3 colorEnd   = vec3(0.41, 0.56, 1);
        // Mix color based on the independent gradient noise value
        vec3 mixedGradientColor = mix(colorStart, colorEnd, baseColorValue);
        vec3 baseFinalColor = mixedGradientColor * randomNoiseValue; // Base color before bloom
    
    
    
        // ===== Circular reveal fade-in & bloom calculation
        float revealAlpha = 1.0; // Default to fully visible
        vec3 bloomEffect = vec3(0.0); // Default to no bloom
    
        float elapsedTime = u_time - u_revealStartTime;
        if (elapsedTime >= 0.0 && elapsedTime < u_revealDuration) {
            // Animation is active
    
            // 1. Calculate normalized linear progress (0 to 1)
            float linearProgress = elapsedTime / u_revealDuration;
    
            // 2. Apply easing function to the progress
            float easedProgress = easeOutCubic(linearProgress);
    
            // 3. Calculate distance from current pixel to reveal center
            float dist = distance(TexCoords, u_revealCenter);
    
            // 4. Define the radius of the *transparent* circle expanding outwards
            float maxVisibleRadius = 1.5;
            float currentRadius = easedProgress * maxVisibleRadius; // Use eased progress for radius
    
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
    
        // ===== Actual final color by mixing the base final color with the reveal bloom effect
        vec3 finalColor = baseFinalColor + bloomEffect; // Add bloom to base color
        float finalAlpha = baseMaskAlphaValue * revealAlpha; // Combine simplex alpha and reveal alpha
    
        FragColor = vec4(finalColor, finalAlpha);
    }
)";

GLuint VAO, VBO, /*EBO,*/ shaderProgram;

GLint timeLocation;
GLint simplexOffsetLocation;
GLint revealStartTimeLocation;
GLint revealCenterLocation;

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

    // Set up vertex data and buffers for a fullscreen quad
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

    // Get the location of uniforms in the shader program to be set when drawn
    timeLocation = glGetUniformLocation(shaderProgram, "u_time");
    simplexOffsetLocation = glGetUniformLocation(shaderProgram, "u_simplexOffset");
    revealStartTimeLocation = glGetUniformLocation(shaderProgram, "u_revealStartTime");
    revealCenterLocation = glGetUniformLocation(shaderProgram, "u_revealCenter");

    // Check reveal uniforms
    if (timeLocation == -1 || simplexOffsetLocation == -1 || revealStartTimeLocation == -1 ||
        revealCenterLocation == -1) {
        std::cerr << "Warning: One or more uniforms not found in shader!" << std::endl;
    }
}

// Modify drawGraphics signature to accept reveal parameters <-- MODIFIED SIGNATURE
void drawGraphics(
    float simplexOffsetX, float simplexOffsetY,
    float revealStartTime, float revealMouseX, float revealMouseY
) {
    glUseProgram(shaderProgram);

    float currentTime = (float)glfwGetTime();
    //std::cout << "Current Time: " << currentTime << std::endl;
    glUniform1f(timeLocation, currentTime);
    glUniform2f(simplexOffsetLocation, simplexOffsetX, simplexOffsetY);
    glUniform1f(revealStartTimeLocation, revealStartTime);
    glUniform2f(revealCenterLocation, revealMouseX, revealMouseY);

    // Bind the VAO so OpenGL knows to use it
    glBindVertexArray(VAO);

    // Draw the quad (6 vertices)
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void destroyGraphics() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    //glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
}