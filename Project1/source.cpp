#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Camera.h"
#include "Model.h"
#include "filesystem.h"
#include "stb_image.h"
#include "Light.h"

void
processInput(GLFWwindow* window);
void
framebuffer_size_callback(GLFWwindow* window, int width, int height);
void
mouse_callback(GLFWwindow* window, double xpos, double ypos);
void
scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int windowWidth = 1920, windowHeight = 1080;

Camera camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));
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
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);

    // START OF CODE

    // COMPILING SHADERS
    Shader texturedCubeShader("VertexShaderModelBase.glsl",
                              "FragmentShaderModelBase.glsl");
    Shader transparentWindowShader("VertexShaderModelBase.glsl",
                              "FragmentShaderModelTransparent.glsl");

    string cubePath = "resources/models/textured_cube/cube.obj";
    Model cubeModel = Model(FileSystem::getPath(cubePath));

    string transparentWindowPath = "resources/models/transparent_window/transparent_window.obj";
    Model transparentWindowModel = Model(FileSystem::getPath(transparentWindowPath));

    //string transparentWindowPath = "resources/models/red_window/red_window.obj";
    //Model transparentWindowModel = Model(FileSystem::getPath(transparentWindowPath));

    // Wireframe mode

    vector<glm::vec3> windows;
    windows.push_back(glm::vec3(-1.5f, 0.0f, -0.5f));
    windows.push_back(glm::vec3( 1.5f, 0.0f,  0.5f));
    windows.push_back(glm::vec3( 0.0f, 0.0f,  0.7f));
    windows.push_back(glm::vec3( 0.5f, 0.0f, -1.0f));

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // SETTINGS
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // physics
        elapsedTime = (float)glfwGetTime();

        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // rendering
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom),
                                      (float)windowWidth / (float)windowHeight,
                                      0.1f,
                                      100.0f);

        // main model
        for (int i = 0; i < windows.size(); i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, windows[i]);
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.5f));

            transparentWindowShader.use();

            transparentWindowShader.setMat4("model", model);
            transparentWindowShader.setMat4("view", view);
            transparentWindowShader.setMat4("projection", projection);

            // cubeModel.Draw(texturedCubeShader);
            transparentWindowModel.Draw(transparentWindowShader);
        }
        
        //model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
        //model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //transparentWindowModel.Draw(transparentWindowShader);

        // show fps in window name
        currentTime = glfwGetTime();
        delta = currentTime - lastTime;

        if (delta >= 1.0) {
            glfwSetWindowTitle(
              window, (title + "\t" + to_string(nbFrames) + "FPS").c_str());
            nbFrames = 0;
            lastTime = currentTime;
        } else {
            nbFrames++;
        }

        // finish up frame
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void
processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        std::cout << "camera position: (" + std::to_string(camera.Position.x) +
                       ", " + std::to_string(camera.Position.y) + ", " +
                       std::to_string(camera.Position.z) + ")"
                  << std::endl;
        std::cout << "(yaw, pitch): (" + std::to_string(camera.Yaw) + ", " +
                       std::to_string(camera.Pitch) + ")"
                  << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera.ProcessKeyboard(Camera_Movement::UP, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
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
