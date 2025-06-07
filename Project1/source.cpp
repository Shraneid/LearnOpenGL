#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Camera.h"
#include "Model.h"
#include "filesystem.h"
#include "stb_image.h"
#include "Light.h"
#include "Skybox.h"

void
processInput(GLFWwindow* window);
void
updateWindowNameWithFPS(GLFWwindow* window, string title);
void
framebuffer_size_callback(GLFWwindow* window, int width, int height);
void
mouse_callback(GLFWwindow* window, double xpos, double ypos);
void
scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int
loadCubemap(vector<string> texture_faces);

int windowWidth = 1920, windowHeight = 1080;

Camera camera = Camera(glm::vec3(0.0f, 0.0f, 5.0f));
int lastX, lastY;

// physics
float elapsedTime = 0.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// setup for fps counter
double currentTime, delta;
double lastTime = glfwGetTime();
int nbFrames = 0;

// transformation matrices
glm::mat4 model, view, projection;

// uniform matrices setup
unsigned int uboMatrixBlock;

int
main()
{
    // GLAD / GLFW3 SETUP
    stbi_set_flip_vertically_on_load(true);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    string title = "gpu go brrr";

    GLFWwindow* window =
      glfwCreateWindow(windowWidth, windowHeight, title.c_str(), NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);

    // blending setup
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    // START OF CODE
    // SETTING UP MAIN TRANSFORM MATRICES BLOCK
    glGenBuffers(1, &uboMatrixBlock);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrixBlock);
    glBufferData(GL_UNIFORM_BUFFER,
                 128,
                 NULL,
                 GL_STATIC_DRAW); // 2 * mat4 (64bytes)
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // END SETTING UP MAIN TRANSFORMS

    // QUAD DATA
    // clang-format off
    float quadVertices[] = {
        // position     // colors
        -0.05f,  0.05f, 1.0f, 0.0f, 0.0f,
         0.05f, -0.05f, 0.0f, 1.0f, 0.0f,
        -0.05f, -0.05f, 0.0f, 0.0f, 1.0f,

        -0.05f,  0.05f, 1.0f, 0.0f, 0.0f,
         0.05f,  0.05f, 1.0f, 1.0f, 0.0f,
         0.05f, -0.05f, 0.0f, 1.0f, 0.0f,
    };
    // clang-format

    unsigned int  quadVAO, quadVBO;

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(quadVertices),
                 quadVertices,
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          5 * sizeof(GL_FLOAT),
                          (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          5 * sizeof(GL_FLOAT),
                          (void*)(2 * sizeof(float)));


    // COMPILING SHADERS
    Shader skyboxShader("VertexShaderCubemap.glsl",
                        "FragmentShaderCubemap.glsl");
    Skybox skybox("resources/cubemaps/yokohama", skyboxShader);

    Shader instancedShader("VertexShaderInstancing.glsl", "FragmentShaderInstancing.glsl");


    string cubePath = "resources/models/textured_cube/cube.obj";
    string backpackPath = "resources/models/backpack/backpack.obj";

    Model modelToDraw = Model(FileSystem::getPath(cubePath));
    //Model modelToDraw = Model(FileSystem::getPath(backpackPath));

    glm::vec2 translations[100];
    int index = 0;

    for (int y = -10; y < 10; y += 2) {
        for (int x = -10; x < 10; x += 2) {
            glm::vec2 translation;
            translation.x = (float)x / 10.0f + 0.1f;
            translation.y = (float)y / 10.0f + 0.1f;
            
            translations[index] = translation;
            index++;
        } 
    }


    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // PHYSICS
        elapsedTime = (float)glfwGetTime();

        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //view = camera.GetViewMatrix();
        //projection = glm::perspective(glm::radians(camera.Zoom),
        //                              (float)windowWidth / (float)windowHeight,
        //                              0.1f,
        //                              100.0f);

        instancedShader.use();

        for (int i = 0; i < std::size(translations); i++) {
            instancedShader.setVec2("offsets["+std::to_string(i)+"]", translations[i]);
        }

        glBindVertexArray(quadVAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);


        updateWindowNameWithFPS(window, title);

        // finish up frame
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void
updateWindowNameWithFPS(GLFWwindow* window, string title)
{
    currentTime = glfwGetTime();
    delta = currentTime - lastTime;

    if (delta >= 1.0)
    {
        glfwSetWindowTitle(
          window,
          (title + "\t" + to_string(nbFrames) + "FPS").c_str());
        nbFrames = 0;
        lastTime = currentTime;
    }
    else
    {
        nbFrames++;
    }
}

void
processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
    {
        std::cout << "camera position: (" + std::to_string(camera.Position.x) +
                       ", " + std::to_string(camera.Position.y) + ", " +
                       std::to_string(camera.Position.z) + ")"
                  << std::endl;
        std::cout << "(yaw, pitch): (" + std::to_string(camera.Yaw) + ", " +
                       std::to_string(camera.Pitch) + ")"
                  << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(Camera_Movement::UP, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(Camera_Movement::DOWN, deltaTime);
    }
}

void
mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos);

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    camera.ProcessMouseMovement(xoffset, yoffset);

    lastX = windowWidth / 2;
    lastY = windowHeight / 2;
    glfwSetCursorPos(window, lastX, lastY);
}

void
scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void
framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    windowWidth = width;
    windowHeight = height;
}

unsigned int
loadCubemap(vector<string> faces_filepath)
{
    stbi_set_flip_vertically_on_load(false);

    unsigned int cubemapTextureID;
    glGenTextures(1, &cubemapTextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);

    int width, height, nrChannels;
    unsigned char* data;
    for (unsigned int i = 0; i < faces_filepath.size(); i++)
    {
        data =
          stbi_load(faces_filepath[i].c_str(), &width, &height, &nrChannels, 0);
        if (!data)
        {
            stbi_image_free(data);
            std::cout << "Cubemap tex failed to load at path: "
                      << faces_filepath[i] << std::endl;
            throw;
        }
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0,
                     GL_RGB,
                     width,
                     height,
                     0,
                     GL_RGB,
                     GL_UNSIGNED_BYTE,
                     data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    stbi_set_flip_vertically_on_load(true);

    return cubemapTextureID;
}
