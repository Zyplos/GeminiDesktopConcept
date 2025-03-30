#include "graphics.h"

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

        // --- Combine ---
        // Modulate the gradient color by the random noise value
        vec3 finalColor = perlinGradientColor * randomNoiseValue;
        // Use the calculated gradient color and the maskAlpha derived from mask noise
        FragColor = vec4(finalColor, maskAlpha);
    }
)";

GLuint VAO, VBO, EBO, shaderProgram;
GLint timeLocation;      
GLint noiseSpeedLocation;
GLint perlinScaleLocation;
GLint upperEdgeLocation; 
GLint lowerEdgeLocation;
GLint perlinOffsetLocation;

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

    // Check if uniforms were found (optional but good practice)
    if (timeLocation == -1 || noiseSpeedLocation == -1 || perlinScaleLocation == -1 ||
        upperEdgeLocation == -1 || lowerEdgeLocation == -1 || perlinOffsetLocation == -1) {
        std::cerr << "Warning: One or more uniforms not found in shader!" << std::endl;
    }
}

void drawGraphics(float noiseSpeed, float perlinScale, float lowerEdge, float upperEdge, float perlinOffsetX, float perlinOffsetY, float gradientOffsetX, float gradientOffsetY, int bufferWidth, int bufferHeight) {
    glUseProgram(shaderProgram);

    // 2. Update the time uniform
    float currentTime = (float)glfwGetTime();
    //std::cout << "Current Time: " << currentTime << std::endl;
    glUniform1f(timeLocation, currentTime);
    glUniform1f(noiseSpeedLocation, noiseSpeed);
    glUniform1f(perlinScaleLocation, perlinScale);
    glUniform1f(upperEdgeLocation, upperEdge);
    glUniform1f(lowerEdgeLocation, lowerEdge);
    glUniform2f(perlinOffsetLocation, perlinOffsetX, perlinOffsetY);

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