#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <format>

#include "Camera.h"
#include "Model.h"
#include "filesystem.h"
#include "stb_image.h"
#include "Light.h"
#include "Skybox.h"
#include "Cube.h"

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

Camera camera = Camera(glm::vec3(0.0f, 1.0f, 5.0f));
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

    // gamma correction setup
    glEnable(GL_FRAMEBUFFER_SRGB);

    std::cout << std::format(
      "OpenGL Version: {}\n",
      reinterpret_cast<const char*>(glGetString(GL_VERSION)));

    // START OF CODE
    
    // SETTING UP MAIN TRANSFORM MATRICES BLOCK
    // glGenBuffers(1, &uboMatrixBlock);
    // glBindBuffer(GL_UNIFORM_BUFFER, uboMatrixBlock);
    // glBufferData(GL_UNIFORM_BUFFER,
    //             128,
    //             NULL,
    //             GL_STATIC_DRAW); // 2 * mat4 (64bytes)
    // glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // END SETTING UP MAIN TRANSFORMS

    // SKYBOX
    Shader skyboxShader("VertexShaderCubemap.glsl",
                        "FragmentShaderCubemap.glsl");
    Skybox skybox("resources/cubemaps/space", skyboxShader);

    // COMPILING SHADERS
    Shader lightSourceShader("VertexShaderBase.glsl",
                             "FragmentShaderBase.glsl");
    Shader omniDepthPassThroughShader("VertexShaderOmniShadowMap.glsl",
                                      "GeometryShaderOmniShadowMap.glsl",
                                      "FragmentShaderOmniShadowMap.glsl");
    Shader cubeLitWithOmniShadowsNormalParallaxShader(
      "VertexShaderModelLitOmniShadowsNormalMapParallaxMap.glsl",
      "FragmentShaderModelLitOmniShadowsNormalMapParallaxMap.glsl");

    string cubePath = "resources/models/textured_cube/cube.obj";
    string brickCubePath = "resources/models/brick_cube/brick.obj";
    string brickParallaxCubePath = "resources/models/displaced_brick_cube/brick.obj";
    string toyPath = "resources/models/toy/Untitled.obj";
    string brickPlanePath = "resources/models/brick_plane/brick_plane.obj";
    string backpackPath = "resources/models/backpack/backpack.obj";
    string marsPath = "resources/models/planet/planet.obj";
    string rockPath = "resources/models/rock/rock.obj";

    Model cube = Model(FileSystem::getPath(cubePath));
    //Model brickCube = Model(FileSystem::getPath(brickCubePath));
    //Model brickCube = Model(FileSystem::getPath(brickParallaxCubePath));
    Model toy = Model(FileSystem::getPath(toyPath));
    //Model brickPlane = Model(FileSystem::getPath(brickPlanePath));
    //Model backpack = Model(FileSystem::getPath(backpackPath));
    // Model mars = Model(FileSystem::getPath(marsPath));
    // Model rock = Model(FileSystem::getPath(rockPath));

    // light cube
    vector<glm::vec3> lightCube = { glm::vec3(0, 0, 0),
                                     glm::vec3(0.1),
                                     glm::vec3(1),
                                     glm::vec3(0) };

    // light cube
    vector<glm::vec3> redLightCube = { glm::vec3(0, 0, 0),
                                     glm::vec3(0.1),
                                     glm::vec3(1),
                                     glm::vec3(0) };

    float floorSize = 10.0f;
    vector<vector<glm::vec3>> posScaleRot = {
        // cube
        { glm::vec3(0, 0, 0),
          glm::vec3(floorSize),
          glm::vec3(1),
          glm::vec3(0) },

          
        { glm::vec3(0.0f, 0.0f, 0.0), // pos
          glm::vec3(1.0),             // scale
          glm::vec3(1, 1, 1),         // rotation axis
          glm::vec3(0) },             // rotation angle

        // objects
        //{ glm::vec3(0.0f, 3.5f, 0.0),  // pos
        //  glm::vec3(0.5),              // scale
        //  glm::vec3(1, 1, -1),         // rotation axis
        //  glm::vec3(0) },              // rotation angle
        //{ glm::vec3(3.0f, 0.5f, 1.0),  // pos
        //  glm::vec3(0.5),              // scale
        //  glm::vec3(1, 0.3, -0.2),     // rotation axis
        //  glm::vec3(0) },              // rotation angle
        //{ glm::vec3(-1.0f, 1.0f, 4.0), // pos
        //  glm::vec3(0.25),             // scale
        //  glm::vec3(1.0, 0.0, 1.0),    // rotation axis
        //  glm::vec3(60) },             // rotation angle
    };

    auto p1 =
      std::make_shared<PointLight>(glm::vec3(1.0f, 0.0f, 2.0f), // position
                                   glm::vec3(0.2f),             // ambient
                                   glm::vec3(0.5f),             // diffuse
                                   glm::vec3(1.0f),             // specular
                                   1.0f,                        // constant
                                   0.22f,                       // linear
                                   0.20f                        // quadratic
      );

    auto p2 =
      std::make_shared<PointLight>(glm::vec3(1.0f, 0.0f, 2.0f), // position
                                   glm::vec3(0.2f, 0.0f, 0.0f), // ambient
                                   glm::vec3(0.5f, 0.0f, 0.0f), // diffuse
                                   glm::vec3(1.0f, 0.0f, 0.0f), // specular
                                   1.0f,                        // constant
                                   0.7f,                        // linear
                                   1.8f                        // quadratic
      );

    vector<std::shared_ptr<Light>> lights;
    lights.push_back(p1);
    lights.push_back(p2);

    // OMNIDIRECTIONAL SHADOW MAP SETUP
    const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;

    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    unsigned int depthCubemapTextureID;
    glGenTextures(1, &depthCubemapTextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemapTextureID);

    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0,
                     GL_DEPTH_COMPONENT,
                     SHADOW_WIDTH,
                     SHADOW_HEIGHT,
                     0,
                     GL_DEPTH_COMPONENT,
                     GL_FLOAT,
                     NULL);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP,
                        GL_TEXTURE_WRAP_S,
                        GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP,
                        GL_TEXTURE_WRAP_T,
                        GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP,
                        GL_TEXTURE_WRAP_R,
                        GL_CLAMP_TO_EDGE);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER,
                         GL_DEPTH_ATTACHMENT,
                         depthCubemapTextureID,
                         0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // PHYSICS
        elapsedTime = (float)glfwGetTime();

        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        float fast_speed = 2.0;
        float slow_speed = 0.5;

        float fast_s = std::sin(elapsedTime * fast_speed);
        float fast_c = std::cos(elapsedTime * fast_speed);

        float slow_s = std::sin(elapsedTime * slow_speed);
        float slow_c = std::cos(elapsedTime * slow_speed);

        auto mainLight = dynamic_pointer_cast<PointLight>(lights[0]).get();
        //mainLight->position = glm::vec3(slow_s * 3.0f, 0, slow_c * 3.5f);
        mainLight->position = glm::vec3(slow_s * 3.0f, 2.0f, 0.0f);
        //mainLight->position = glm::vec3(slow_s * 0.75f, 1.3f, 1.3f);
        //mainLight->position = glm::vec3(-.65f, .0f, 2.f);

        auto redLight = dynamic_pointer_cast<PointLight>(lights[1]).get();
        redLight->position = glm::vec3(0, fast_s * 1.0f, 1.15f);

        lightCube[0] = mainLight->position;
        redLightCube[0] = redLight->position;

        // RENDER SHADOW MAP
        float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
        float near_plane = 1.0f, far_plane = 25.0f;
        glm::mat4 shadowProj =
          glm::perspective(glm::radians(90.0f), aspect, near_plane, far_plane);

        auto lightPos = mainLight->getPosition();
        std::vector<glm::mat4> shadowTransforms;

        shadowTransforms.push_back(
          shadowProj * glm::lookAt(lightPos,
                                   lightPos + glm::vec3(1.0, 0.0, 0.0),
                                   glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(
          shadowProj * glm::lookAt(lightPos,
                                   lightPos + glm::vec3(-1.0, 0.0, 0.0),
                                   glm::vec3(0.0, -1.0, 0.0)));

        shadowTransforms.push_back(
          shadowProj * glm::lookAt(lightPos,
                                   lightPos + glm::vec3(0.0, 1.0, 0.0),
                                   glm::vec3(0.0, 0.0, 1.0)));
        shadowTransforms.push_back(
          shadowProj * glm::lookAt(lightPos,
                                   lightPos + glm::vec3(0.0, -1.0, 0.0),
                                   glm::vec3(0.0, 0.0, -1.0)));

        shadowTransforms.push_back(
          shadowProj * glm::lookAt(lightPos,
                                   lightPos + glm::vec3(0.0, 0.0, 1.0),
                                   glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(
          shadowProj * glm::lookAt(lightPos,
                                   lightPos + glm::vec3(0.0, 0.0, -1.0),
                                   glm::vec3(0.0, -1.0, 0.0)));

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        omniDepthPassThroughShader.use();
        for (int i = 0; i < shadowTransforms.size(); ++i)
        {
            omniDepthPassThroughShader.setMat4("shadowMatrices[" +
                                                 std::to_string(i) + "]",
                                               shadowTransforms[i]);
        }
        omniDepthPassThroughShader.setFloat("far_plane", far_plane);
        omniDepthPassThroughShader.setVec3("lightPos", lightPos);

        for (int i = 0; i < posScaleRot.size(); i++)
        {
            auto vectors = posScaleRot[i];
            model = glm::mat4(1.0f);
            model = glm::translate(model, vectors[0]);
            model = glm::rotate(model, vectors[3].x, vectors[2]);
            model = glm::scale(model, vectors[1]);

            omniDepthPassThroughShader.setMat4("model", model);

            if (i == 0)
            {
                cube.Draw(omniDepthPassThroughShader);
            }
            else
            {
                toy.Draw(omniDepthPassThroughShader);
            }
        }

        glViewport(0, 0, windowWidth, windowHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // SHADOW MAP DONE

        // RENDER NORMAL SCENE
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom),
                                      (float)windowWidth / (float)windowHeight,
                                      0.1f,
                                      1000.0f);

        cubeLitWithOmniShadowsNormalParallaxShader.use();
        glActiveTexture(GL_TEXTURE0);
        cubeLitWithOmniShadowsNormalParallaxShader.setInt("omniShadowMap", 0);
        cubeLitWithOmniShadowsNormalParallaxShader.setFloat("far_plane",
                                                          far_plane);
        cubeLitWithOmniShadowsNormalParallaxShader.setFloat("parallax_strength",
                                                          0.1f);
        for (int i = 0; i < posScaleRot.size(); i++)
        {
            auto vectors = posScaleRot[i];

            model = glm::mat4(1.0f);
            model = glm::translate(model, vectors[0]);
            model = glm::scale(model, vectors[1]);
            model = glm::rotate(model, vectors[3].x, vectors[2]);

            for (auto light : lights)
            {
                light.get()->setUniforms(
                  cubeLitWithOmniShadowsNormalParallaxShader);
            }

            cubeLitWithOmniShadowsNormalParallaxShader.setMat4("model", model);
            cubeLitWithOmniShadowsNormalParallaxShader.setMat4("view", view);
            cubeLitWithOmniShadowsNormalParallaxShader.setMat4("projection",
                                                             projection);
            cubeLitWithOmniShadowsNormalParallaxShader.setVec3("viewPos",
                                                             camera.Position);

            if (i == 0)
            {
                cubeLitWithOmniShadowsNormalParallaxShader.setFloat(
                  "reverse_normals",
                  1.0f);
                cube.Draw(cubeLitWithOmniShadowsNormalParallaxShader);
            }
            else
            {                
                cubeLitWithOmniShadowsNormalParallaxShader.setFloat(
                  "reverse_normals",
                  0.0f);
                toy.Draw(cubeLitWithOmniShadowsNormalParallaxShader);
            }
        }

        lightSourceShader.use();

        model = glm::mat4(1.0f);
        model = glm::translate(model, lightCube[0]);
        model = glm::scale(model, lightCube[1]);
        model = glm::rotate(model, lightCube[3].x, lightCube[2]);

        lightSourceShader.setMat4("model", model);
        lightSourceShader.setMat4("view", view);
        lightSourceShader.setMat4("projection", projection);
        lightSourceShader.setVec3("lightColor", glm::vec3(1.0f));
        cube.Draw(lightSourceShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, redLightCube[0]);
        model = glm::scale(model, redLightCube[1]);
        model = glm::rotate(model, redLightCube[3].x, redLightCube[2]);

        lightSourceShader.setMat4("model", model);
        lightSourceShader.setMat4("view", view);
        lightSourceShader.setMat4("projection", projection);
        lightSourceShader.setVec3("lightColor", glm::vec3(1.0f, 0.0f, 0.0f));
        cube.Draw(lightSourceShader);

        skybox.Draw(projection, view);

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
        std::cout << "camera posScale: (" + std::to_string(camera.Position.x) +
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
