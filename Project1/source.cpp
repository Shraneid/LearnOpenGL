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

    // COMPILING SHADERS
    Shader skyboxShader("VertexShaderCubemap.glsl",
                        "FragmentShaderCubemap.glsl");
    Skybox skybox("resources/cubemaps/space", skyboxShader);

    Shader fullscreenQuadShader("VertexShaderRenderbuffer.glsl",
                                "FragmentShaderRenderbuffer.glsl");
    Shader depthPassThroughShader("VertexShaderDepthPassThrough.glsl",
                                  "FragmentShaderDepthPassThrough.glsl");
    Shader cubeLitWithShadowsShader("VertexShaderModelLitShadows.glsl",
                                    "FragmentShaderModelLitShadows.glsl");

    string cubePath = "resources/models/textured_cube/cube.obj";
    string backpackPath = "resources/models/backpack/backpack.obj";
    string marsPath = "resources/models/planet/planet.obj";
    string rockPath = "resources/models/rock/rock.obj";

    Model cube = Model(FileSystem::getPath(cubePath));
    Model backpack = Model(FileSystem::getPath(backpackPath));
    // Model mars = Model(FileSystem::getPath(marsPath));
    // Model rock = Model(FileSystem::getPath(rockPath));

    float floorSize = 10.0f;
    vector<vector<glm::vec3>> posScaleRot = {
        { glm::vec3(0, -floorSize, 0),
          glm::vec3(floorSize),
          glm::vec3(1),
          glm::vec3(0) },

        { glm::vec3(0.0f, 1.5f, 0.0),  // pos
          glm::vec3(0.5),              // scale
          glm::vec3(1, 1, -1),         // rotation axis
          glm::vec3(0) },              // rotation angle
        { glm::vec3(2.0f, 0.5f, 1.0),  // pos
          glm::vec3(0.5),              // scale
          glm::vec3(1, 0.3, -0.2),     // rotation axis
          glm::vec3(0) },              // rotation angle
        { glm::vec3(-1.0f, 1.0f, 2.0), // pos
          glm::vec3(0.25),             // scale
          glm::vec3(1.0, 0.0, 1.0),    // rotation axis
          glm::vec3(60) },             // rotation angle
    };

    auto d1 = std::make_shared<DirectionalLight>(
      glm::vec3(2.0f, -4.0f, 1.0f), // direction
      glm::vec3(0.15f),             // ambient
      glm::vec3(0.35f),             // diffuse
      glm::vec3(0.5f)               // specular
    );

    // auto d1 = std::make_shared<DirectionalLight>(
    //   glm::vec3(0.5f, -1.0f, -1.2f), // direction
    //   glm::vec3(0.05f),              // ambient
    //   glm::vec3(0.4f),               // diffuse
    //   glm::vec3(0.5f)                // specular
    //);

    // auto p1 =
    //   std::make_shared<PointLight>(&greenLightCubeObject.Position, //
    //   position
    //                                glm::vec3(0.05f),               // ambient
    //                                glm::vec3(0.0f, 1.0f, 0.0f),    // diffuse
    //                                glm::vec3(0.5f),                //
    //                                specular 1.0f, // constant 0.045f, //
    //                                linear 0.0075f                         //
    //                                quadratic
    //   );

    vector<std::shared_ptr<Light>> lights;
    lights.push_back(d1);
    // lights.push_back(p1);

    // SHADOW MAP SETUP
    unsigned int shadowMapFBO;
    glGenFramebuffers(1, &shadowMapFBO);

    const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;

    unsigned int depthMapTextureID;
    glGenTextures(1, &depthMapTextureID);
    glBindTexture(GL_TEXTURE_2D, depthMapTextureID);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH,
                 SHADOW_HEIGHT,
                 0,
                 GL_DEPTH_COMPONENT,
                 GL_FLOAT,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D,
                           depthMapTextureID,
                           0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // TEMP DEPTH MAP RENDERING
    // clang-format off
    float vertices[] = {
        // positions  // texCoords
        -1.0f, +1.0f, 0.0f,  1.0f,  
        -1.0f, -1.0f, 0.0f,  0.0f,  
        +1.0f, -1.0f, 1.0f,  0.0f,

        -1.0f, +1.0f, 0.0f,  1.0f,
        +1.0f, -1.0f, 1.0f,  0.0f,  
        +1.0f, +1.0f, 1.0f,  1.0f,
    };
    // clang-format on

    unsigned int quadVBO, quadVAO;

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          4 * sizeof(GL_FLOAT),
                          (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          4 * sizeof(GL_FLOAT),
                          (void*)(2 * sizeof(float)));

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // PHYSICS
        elapsedTime = (float)glfwGetTime();

        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        dynamic_pointer_cast<DirectionalLight>(lights[0]).get()->direction =
          glm::vec3(4.0f * sin(elapsedTime), -4.0f, 4.0f * cos(elapsedTime));

        // RENDER SHADOW MAP
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glCullFace(GL_FRONT);
        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        float near_plane = 1.0f, far_plane = 15.0f;
        glm::mat4 lightProjection = glm::ortho(-far_plane,
                                               far_plane,
                                               -far_plane,
                                               far_plane,
                                               near_plane,
                                               far_plane);

        auto lightDir =
          dynamic_pointer_cast<DirectionalLight>(lights[0])->direction;
        float lightPosMoveAwayForce = 1.5f;
        glm::vec3 lightPos = -lightDir * lightPosMoveAwayForce;

        glm::mat4 lightView = glm::lookAt(lightPos,
                                          glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        depthPassThroughShader.use();
        depthPassThroughShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        for (int i = 0; i < posScaleRot.size(); i++)
        {
            auto vectors = posScaleRot[i];
            model = glm::mat4(1.0f);
            model = glm::translate(model, vectors[0]);
            model = glm::rotate(model, vectors[3].x, vectors[2]);
            model = glm::scale(model, vectors[1]);

            depthPassThroughShader.setMat4("model", model);

            if (i == 0)
            {
                cube.Draw(depthPassThroughShader);
            }
            else
            {
                backpack.Draw(depthPassThroughShader);
            }
        }

        glViewport(0, 0, windowWidth, windowHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glCullFace(GL_BACK);
        // SHADOW MAP DONE

        // TEMP
        // fullscreenQuadShader.use();
        // fullscreenQuadShader.setInt("screenTexture", 0);
        // glBindVertexArray(quadVAO);
        // glDisable(GL_DEPTH_TEST);
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, depthMapTextureID);
        // glDrawArrays(GL_TRIANGLES, 0, 6);
        // END TEMP

        // RENDER NORMAL SCENE
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom),
                                      (float)windowWidth / (float)windowHeight,
                                      0.1f,
                                      1000.0f);

        cubeLitWithShadowsShader.use();
        glActiveTexture(GL_TEXTURE0);
        cubeLitWithShadowsShader.setInt("shadowMap", 0);
        cubeLitWithShadowsShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        for (int i = 0; i < posScaleRot.size(); i++)
        {
            auto vectors = posScaleRot[i];

            model = glm::mat4(1.0f);
            model = glm::translate(model, vectors[0]);
            model = glm::scale(model, vectors[1]);
            model = glm::rotate(model, vectors[3].x, vectors[2]);

            for (auto light : lights)
            {
                light.get()->setUniforms(cubeLitWithShadowsShader);
            }

            cubeLitWithShadowsShader.setMat4("model", model);
            cubeLitWithShadowsShader.setMat4("view", view);
            cubeLitWithShadowsShader.setMat4("projection", projection);
            cubeLitWithShadowsShader.setVec3("viewPos", camera.Position);

            if (i == 0)
            {
                cube.Draw(cubeLitWithShadowsShader);
            }
            else
            {
                backpack.Draw(cubeLitWithShadowsShader);
            }
        }

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
