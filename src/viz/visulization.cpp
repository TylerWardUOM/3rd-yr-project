#include <glad/glad.h>   // OpenGL loader
#include <GLFW/glfw3.h>  // Window/input
#include <iostream>

// Callback: resize viewport if window size changes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
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
const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    void main()
    {
        FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
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
    
    if(!success) //if not successful, print the error log
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }


    // Create and compile fragment shader
    unsigned int fragmentShader; // Fragment Shader ID
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); //create fragment shader object and get its ID
    
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); //attach the shader source code to the shader object
    glCompileShader(fragmentShader); //compile the shader

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success); //check if compilation was successful
    
    if(!success) //if not successful, print the error log
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Create shader program and link shaders
    unsigned int shaderProgram; // Shader Program ID
    shaderProgram = glCreateProgram(); //create shader program object and get its ID
    glAttachShader(shaderProgram, vertexShader); //attach vertex shader to the program
    glAttachShader(shaderProgram, fragmentShader); //attach fragment shader to the program
    glLinkProgram(shaderProgram); //link the shaders to create the final shader program

    //check if linking was successful
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader); //delete the vertex shader as it's no longer needed
    glDeleteShader(fragmentShader); //delete the fragment shader as it's no longer needed



    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    // We have a rectangle made of two triangles. We specify the vertices and the indices.
    float vertices[] = {
        0.5f,  0.5f, 0.0f,  // top right
        0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };  

    unsigned int EBO, VAO, VBO; // Element Buffer Object, Vertex Array Object, Vertex Buffer Object
    glGenVertexArrays(1, &VAO); //generate a VAO and get its ID
    glGenBuffers(1, &EBO); //generate an EBO and get its ID
    glGenBuffers(1, &VBO); //generate a VBO and get its ID
    glBindVertexArray(VAO); //bind the VAO first, then bind and set VBOs and then configure vertex attributes(s).
    glBindBuffer(GL_ARRAY_BUFFER, VBO); //bind the VBO 
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //copy the vertex data to the VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);//bind the EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); //copy the index data to the EBO

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); //set the vertex attribute pointers
    glEnableVertexAttribArray(0); //enable the vertex attribute

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

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
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);//draw the rectangle using the indices


        // Swap buffers + poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
