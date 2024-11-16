#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Camera.h"
#include "Model.h"
#include "filesystem.h"
#include "stb_image.h"
#include "Light.h"
#include "Cube.h"

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
    Shader lightCubeShader("VertexShaderBase.hlsl", "FragmentShaderBase.hlsl");
    Cube greenLightCubeObject{ glm::vec3(0.0f, 0.0f, 2.0f) };
    Cube purpleLightCubeObject{ glm::vec3(0.0f, 0.0f, -2.0f) };

    // cube test for stencil buffer
    Cube cube{ glm::vec3(0.0f, 0.0f, 0.0f) };

    auto d1 = std::make_shared<DirectionalLight>(
      glm::vec3(0.5f, -1.0f, -1.2f), // direction
      glm::vec3(0.05f),              // ambient
      glm::vec3(0.4f),               // diffuse
      glm::vec3(0.5f)                // specular
    );
    auto d2 = std::make_shared<DirectionalLight>(
      glm::vec3(3.0f, -1.0f, 1.0f), // direction
      glm::vec3(0.05f),             // ambient
      glm::vec3(1.0f, 0.0f, 0.0f),  // diffuse
      glm::vec3(0.5f)               // specular
    );
    auto d3 = std::make_shared<DirectionalLight>(
      glm::vec3(-3.0f, -1.0f, -1.0f), // direction
      glm::vec3(0.05f),               // ambient
      glm::vec3(0.0f, 0.0f, 1.0f),    // diffuse
      glm::vec3(0.5f)                 // specular
    );
    auto p1 =
      std::make_shared<PointLight>(&greenLightCubeObject.Position, // position
                                   glm::vec3(0.05f),               // ambient
                                   glm::vec3(0.0f, 1.0f, 0.0f),    // diffuse
                                   glm::vec3(0.5f),                // specular
                                   1.0f,                           // constant
                                   0.045f,                         // linear
                                   0.0075f                         // quadratic
      );
    auto p2 =
      std::make_shared<PointLight>(&purpleLightCubeObject.Position, // position
                                   glm::vec3(0.05f),                // ambient
                                   glm::vec3(1.0f, 0.0f, 1.0f),     // diffuse
                                   glm::vec3(0.5f),                 // specular
                                   1.0f,                            // constant
                                   0.045f,                          // linear
                                   0.0075f                          // quadratic
      );

    vector<std::shared_ptr<Light>> lights;
    lights.push_back(d1);
    lights.push_back(d2);
    lights.push_back(d3);
    lights.push_back(p1);
    lights.push_back(p2);

    // COMPILING SHADERS
    Shader lightSourceShader("VertexShaderBase.hlsl",
                             "FragmentShaderBase.hlsl");
    Shader litModelShader("VertexShaderModelLit.hlsl",
                          "FragmentShaderModelLit.hlsl");
    Shader outlineModelShader("VertexShaderModelLit.hlsl",
                              "FragmentShaderSolidColor.hlsl");

    string path = "resources/models/backpack/backpack.obj";
    Model backpackModel = Model(FileSystem::getPath(path));

    // Wireframe mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // SETTINGS
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                GL_STENCIL_BUFFER_BIT);

        glStencilMask(0x00);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);

        // physics
        elapsedTime = (float)glfwGetTime();

        greenLightCubeObject.Position = glm::vec3(
          glm::sin(elapsedTime) * 2.0f, 0.0f, glm::cos(elapsedTime) * 2.0f);
        purpleLightCubeObject.Position =
          glm::vec3(glm::sin(elapsedTime + glm::pi<float>()) * 2.0f,
                    0.0f,
                    glm::cos(elapsedTime + glm::pi<float>()) * 2.0f);

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
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f));
        model = glm::scale(model, glm::vec3(0.5f));

        /*litModelShader.use();

        litModelShader.setMat4("view", view);
        litModelShader.setMat4("projection", projection);
        litModelShader.setMat4("model", model);

        litModelShader.setVec3("viewPos", camera.Position);
        litModelShader.setFloat("time", elapsedTime);

        for (auto light : lights) {
            light.get()->setUniforms(litModelShader);
        }

        backpackModel.Draw(litModelShader);*/

        outlineModelShader.use();

        outlineModelShader.setMat4("model", model);
        outlineModelShader.setMat4("view", view);
        outlineModelShader.setMat4("projection", projection);
        outlineModelShader.setVec3("solidColor", glm::vec3(1.0f, 0.0f, 0.0f));
        cube.Draw();

        // draw outline
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f));
        model = glm::scale(model, glm::vec3(0.55f));

        outlineModelShader.use();

        outlineModelShader.setMat4("model", model);
        outlineModelShader.setMat4("view", view);
        outlineModelShader.setMat4("projection", projection);
        outlineModelShader.setVec3("solidColor", glm::vec3(0.0f, 1.0f, 0.0f));

        outlineModelShader.setVec3("viewPos", camera.Position);
        outlineModelShader.setFloat("time", elapsedTime);

        //backpackModel.Draw(outlineModelShader);
        cube.Draw();

        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);

        // render light cubes
        model = glm::mat4(1.0f);
        model = glm::translate(model, greenLightCubeObject.Position);
        model = glm::scale(model, glm::vec3(0.1f));

        lightSourceShader.use();

        lightSourceShader.setMat4("model", model);
        lightSourceShader.setMat4("view", view);
        lightSourceShader.setMat4("projection", projection);

        lightSourceShader.setVec3("lightColor", p1.get()->getDiffuse());
        greenLightCubeObject.Draw();

        model = glm::mat4(1.0f);
        model = glm::translate(model, purpleLightCubeObject.Position);
        model = glm::scale(model, glm::vec3(0.1f));

        lightSourceShader.use();

        lightSourceShader.setMat4("model", model);
        lightSourceShader.setMat4("view", view);
        lightSourceShader.setMat4("projection", projection);

        lightSourceShader.setVec3("lightColor", p2.get()->getDiffuse());
        purpleLightCubeObject.Draw();

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
