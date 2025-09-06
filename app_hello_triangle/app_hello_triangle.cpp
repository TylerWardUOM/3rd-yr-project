#include <glad/glad.h>   // OpenGL loader
#include <GLFW/glfw3.h>  // Window/input
#include <iostream>

// Callback: resize viewport if window size changes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Vertex shader source code
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;

    void main()
    {
        gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    }
)";

// Fragment shader source code
const char* orange_fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    uniform vec4 ourColor;

    void main()
    {
        FragColor = ourColor; // Orange color
    }
)";

// Fragment shader source code
const char* blue_fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    void main()
    {
        FragColor = vec4(0.2f, 0.6f, 1.0f, 1.0f); // Light blue color
    }
)";

int main() {
    // Init GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    // Configure GLFW: use OpenGL 3.3 core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Haptic Render Test", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Load OpenGL funcs with GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        return -1;
    }

    // Set viewport + callback
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


    // Create and compile vertex shader
    unsigned int vertexShader; // Vertex Shader ID
    vertexShader = glCreateShader(GL_VERTEX_SHADER); //create vertex shader object and get its ID

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); //attach the shader source code to the shader object
    glCompileShader(vertexShader); //compile the shader

    int  success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success); //check if compilation was successful

    if (!success) //if not successful, print the error log
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }


    // Create and compile fragment shader
    unsigned int orange_fragmentShader; // Fragment Shader ID
    orange_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); //create fragment shader object and get its ID

    glShaderSource(orange_fragmentShader, 1, &orange_fragmentShaderSource, NULL); //attach the shader source code to the shader object
    glCompileShader(orange_fragmentShader); //compile the shader

    glGetShaderiv(orange_fragmentShader, GL_COMPILE_STATUS, &success); //check if compilation was successful

    if (!success) //if not successful, print the error log
    {
        glGetShaderInfoLog(orange_fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Create and compile blue fragment shader
    unsigned int blue_fragmentShader; // Fragment Shader ID
    blue_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); //create fragment shader object and get its ID

    glShaderSource(blue_fragmentShader, 1, &blue_fragmentShaderSource, NULL); //attach the shader source code to the shader object
    glCompileShader(blue_fragmentShader); //compile the shader

    glGetShaderiv(blue_fragmentShader, GL_COMPILE_STATUS, &success); //check if compilation was successful

    if (!success) //if not successful, print the error log
    {
        glGetShaderInfoLog(blue_fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Create shader program and link shaders
    unsigned int orange_shaderProgram; // Shader Program ID
    orange_shaderProgram = glCreateProgram(); //create shader program object and get its ID
    glAttachShader(orange_shaderProgram, vertexShader); //attach vertex shader to the program
    glAttachShader(orange_shaderProgram, orange_fragmentShader); //attach fragment shader to the program
    glLinkProgram(orange_shaderProgram); //link the shaders to create the final shader program

    // Create blue shader program and link shaders
    unsigned int blue_shaderProgram; // Shader Program ID
    blue_shaderProgram = glCreateProgram(); //create shader program object and get its ID
    glAttachShader(blue_shaderProgram, vertexShader); //attach vertex shader to the program
    glAttachShader(blue_shaderProgram, blue_fragmentShader); //attach fragment shader to the program
    glLinkProgram(blue_shaderProgram); //link the shaders to create the final shader program

    //check if linking was successful
    glGetProgramiv(orange_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(orange_shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader); //delete the vertex shader as it's no longer needed
    glDeleteShader(orange_fragmentShader); //delete the fragment shader as it's no longer needed

    //check if linking was successful
    glGetProgramiv(blue_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(blue_shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader); //delete the vertex shader as it's no longer needed
    glDeleteShader(blue_fragmentShader); //delete the fragment shader as it's no longer needed



    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    // We have a rectangle made of two triangles. We specify the vertices and the indices.
    float vertices_a[] = {
        0.5f,  0.5f, 0.0f,  // top right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
    };

    float vertices_b[] = {
        0.5f,  0.5f, 0.0f,  // top right
        0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left 
    };

    unsigned int VAO_a, VBO_a; // Element Buffer Object, Vertex Array Object, Vertex Buffer Object
    glGenVertexArrays(1, &VAO_a); //generate a VAO and get its ID
    glGenBuffers(1, &VBO_a); //generate a VBO and get its ID
    glBindVertexArray(VAO_a); //bind the VAO first, then bind and set VBOs and then configure vertex attributes(s).
    glBindBuffer(GL_ARRAY_BUFFER, VBO_a); //bind the VBO 
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_a), vertices_a, GL_STATIC_DRAW); //copy the vertex data to the VBO


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); //set the vertex attribute pointers
    glEnableVertexAttribArray(0); //enable the vertex attribute

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO while a VAO is active as the bound EBO is stored in the VAO; keep the EBO bound.

    unsigned int VAO_b, VBO_b; // Element Buffer Object, Vertex Array Object, Vertex Buffer Object
    glGenVertexArrays(1, &VAO_b); //generate a VAO and get its ID
    glGenBuffers(1, &VBO_b); //generate a VBO and get its ID
    glBindVertexArray(VAO_b); //bind the VAO first, then bind and set VBOs and then configure vertex attributes(s).
    glBindBuffer(GL_ARRAY_BUFFER, VBO_b); //bind the VBO 
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_b), vertices_b, GL_STATIC_DRAW); //copy the vertex data to the VBO


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); //set the vertex attribute pointers
    glEnableVertexAttribArray(0); //enable the vertex attribute

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO while a VAO is active as the bound EBO is stored in the VAO; keep the EBO bound.

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.

    //wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Main loop
    while (!glfwWindowShouldClose(window)) {

        // Input
        processInput(window);

        // Rendering commands:

        //set background color
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f); // dark blue
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw triangle
        glUseProgram(orange_shaderProgram);
        float timeValue = glfwGetTime();
        float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
        int vertexColorLocation = glGetUniformLocation(orange_shaderProgram, "ourColor");
        glUseProgram(orange_shaderProgram);
        glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

        glBindVertexArray(VAO_a);
        glDrawArrays(GL_TRIANGLES, 0, 3); // draw the triangle_a
        glUseProgram(blue_shaderProgram);
        glBindVertexArray(VAO_b);
        glDrawArrays(GL_TRIANGLES, 0, 3); // draw the triangle_b
        glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs)


        // Swap buffers + poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
