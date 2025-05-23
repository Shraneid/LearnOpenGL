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
updateWindowNameWithFPS(GLFWwindow* window, string title);
void
framebuffer_size_callback(GLFWwindow* window, int width, int height);
void
mouse_callback(GLFWwindow* window, double xpos, double ypos);
void
scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int
loadCubemap(vector<string> texture_faces);
void
drawScene(unsigned int framebuffer,
          vector<glm::vec3> positions,
          Model modelToDraw,
          Shader modelShader,
          Shader skyboxShader,
          int environmentMappingTexture = -1);

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

// skybox setup
unsigned int skyboxVBO, skyboxVAO;
int skyboxTextureId;

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
    // glEnable(GL_CULL_FACE);

    // glCullFace(GL_BACK);

    // blending setup
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    // START OF CODE
    unsigned int mainFramebuffer;
    glGenFramebuffers(1, &mainFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mainFramebuffer);

    unsigned int mainTextureColorBuffer;
    glGenTextures(1, &mainTextureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, mainTextureColorBuffer);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 windowWidth,
                 windowHeight,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           mainTextureColorBuffer,
                           0);

    unsigned int mainRbo;
    glGenRenderbuffers(1, &mainRbo);

    glBindRenderbuffer(GL_RENDERBUFFER, mainRbo);
    glRenderbufferStorage(GL_RENDERBUFFER,
                          GL_DEPTH24_STENCIL8,
                          windowWidth,
                          windowHeight);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER,
                              mainRbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!"
                  << std::endl;
    }

    unsigned int mirrorFramebuffer;
    glGenFramebuffers(1, &mirrorFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mirrorFramebuffer);

    unsigned int mirrorTextureColorBuffer;
    glGenTextures(1, &mirrorTextureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, mirrorTextureColorBuffer);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 windowWidth,
                 windowHeight,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           mirrorTextureColorBuffer,
                           0);

    unsigned int mirrorRbo;
    glGenRenderbuffers(1, &mirrorRbo);

    glBindRenderbuffer(GL_RENDERBUFFER, mirrorRbo);
    glRenderbufferStorage(GL_RENDERBUFFER,
                          GL_DEPTH24_STENCIL8,
                          windowWidth,
                          windowHeight);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER,
                              mirrorRbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!"
                  << std::endl;
    }

    // RESETTING FRAMEBUFFER
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // COMPILING SHADERS
    Shader texturedCubeShader("VertexShaderModelBase.glsl",
                              "FragmentShaderModelBase.glsl");
    Shader reflectiveTexturedCubeShader(
      "VertexShaderEnvironmentMapReflection.glsl",
      "FragmentShaderEnvironmentMapReflection.glsl");
    Shader refractiveTexturedCubeShader(
      "VertexShaderEnvironmentMapRefraction.glsl",
      "FragmentShaderEnvironmentMapRefraction.glsl");
    Shader transparentWindowShader("VertexShaderModelBase.glsl",
                                   "FragmentShaderModelTransparent.glsl");
    Shader screenShader("VertexShaderRenderbuffer.glsl",
                        "FragmentShaderRenderbuffer.glsl");
    Shader skyboxShader("VertexShaderCubemap.glsl",
                        "FragmentShaderCubemap.glsl");

    string cubePath = "resources/models/textured_cube/cube.obj";
    Model cubeModel = Model(FileSystem::getPath(cubePath));

    // string backpackPath = "resources/models/backpack/backpack.obj";
    // Model backpackModel = Model(FileSystem::getPath(backpackPath));

    string transparentWindowPath = "resources/models/red_window/red_window.obj";
    Model transparentWindowModel =
      Model(FileSystem::getPath(transparentWindowPath));

    // SKYBOX
    // clang-format off
    float skyboxVertices[] = {
        // positions
        -1.0f, 1.0f,  -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f,  -1.0f, -1.0f,
        1.0f,  -1.0f, -1.0f,
        1.0f,  1.0f,  -1.0f,
        -1.0f, 1.0f,  -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f,  -1.0f,
        -1.0f, 1.0f,  -1.0f,
        -1.0f, 1.0f,  1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f,  -1.0f, -1.0f,
        1.0f,  -1.0f, 1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  -1.0f,
        1.0f,  -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,

        1.0f,  1.0f,  1.0f,
        1.0f,  -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f,  -1.0f,
        1.0f,  1.0f,  -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f, 1.0f,  1.0f,
        -1.0f, 1.0f,  -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f,  -1.0f, -1.0f,
        1.0f,  -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f,  -1.0f, 1.0f
    };
    // clang-format on

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);

    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(skyboxVertices),
                 skyboxVertices,
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          3 * sizeof(GL_FLOAT),
                          (void*)0);

    vector<string> faces_filepath = {
        "resources/cubemaps/yokohama/posx.jpg",
        "resources/cubemaps/yokohama/negx.jpg",
        "resources/cubemaps/yokohama/posy.jpg",
        "resources/cubemaps/yokohama/negy.jpg",
        "resources/cubemaps/yokohama/posz.jpg",
        "resources/cubemaps/yokohama/negz.jpg",
    };

    skyboxTextureId = loadCubemap(faces_filepath);

    // END SKYBOX

    vector<glm::vec3> cubePositions = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.5f, 0.3f, -0.5f),
        glm::vec3(1.0f, 0.5f, 10.0f),
    };

    // clang-format off
    float screenQuadVertices[] = {
        // positions        // texCoords
        -1.0f, +1.0f, 1.0f, 0.0f,  1.0f,  
        -1.0f, -1.0f, 1.0f, 0.0f,  0.0f,
        +1.0f, -1.0f, 1.0f, 1.0f,  0.0f,
                      
        -1.0f, +1.0f, 1.0f, 0.0f,  1.0f,
        +1.0f, -1.0f, 1.0f, 1.0f,  0.0f,  
        +1.0f, +1.0f, 1.0f, 1.0f,  1.0f,
    };
    // clang-format on

    unsigned int screenQuadVBO, screenQuadVAO;

    glGenVertexArrays(1, &screenQuadVAO);
    glGenBuffers(1, &screenQuadVBO);

    glBindVertexArray(screenQuadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(screenQuadVertices),
                 screenQuadVertices,
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          5 * sizeof(GL_FLOAT),
                          (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          5 * sizeof(GL_FLOAT),
                          (void*)(3 * sizeof(float)));

    // clang-format off
    float mirrorVertices[] = {
        // only take a small part of the top of the screen
        // positions         // texCoords
        -0.6f, +0.95f, 0.9f, 0.0f,  1.0f,   
        -0.6f, +0.70f, 0.9f, 0.0f,  0.5,    
        +0.6f, +0.70f, 0.9f, 1.0f,  0.5f,
                       
        -0.6f, +0.95f, 0.9f, 0.0f,  1.0f,   
        +0.6f, +0.70f, 0.9f, 1.0f,  0.5f,   
        +0.6f, +0.95f, 0.9f, 1.0f,  1.0f,
    };
    // clang-format on

    unsigned int mirrorVBO, mirrorVAO;

    glGenVertexArrays(1, &mirrorVAO);
    glGenBuffers(1, &mirrorVBO);

    glBindVertexArray(mirrorVAO);

    glBindBuffer(GL_ARRAY_BUFFER, mirrorVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(mirrorVertices),
                 mirrorVertices,
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          5 * sizeof(GL_FLOAT),
                          (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          5 * sizeof(GL_FLOAT),
                          (void*)(3 * sizeof(float)));

    // SETTING UP MAIN TRANSFORM MATRICES BLOCK
    glGenBuffers(1, &uboMatrixBlock);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrixBlock);
    glBufferData(GL_UNIFORM_BUFFER,
                 128,
                 NULL,
                 GL_STATIC_DRAW); // 2 * mat4 (64bits)
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // END SETTING UP MAIN TRANSFORMS

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // PHYSICS
        elapsedTime = (float)glfwGetTime();

        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // SET MIRROR FRAMEBUFFER
        glBindFramebuffer(GL_FRAMEBUFFER, mirrorFramebuffer);
        glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // MIRROR RENDERING SETUP
        // view = camera.GetFlippedViewMatrix();
        // projection = glm::perspective(glm::radians(camera.Zoom),
        //                              (float)windowWidth /
        //                              (float)windowHeight, 0.1f, 100.0f);

        // drawScene(mirrorTextureColorBuffer,
        //           cubePositions,
        //           cubeModel,
        //           texturedCubeShader,
        //           skyboxShader);

        // MAIN RENDERING SETUP
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom),
                                      (float)windowWidth / (float)windowHeight,
                                      0.1f,
                                      100.0f);

        drawScene(mainTextureColorBuffer,
                  cubePositions,
                  cubeModel,
                  refractiveTexturedCubeShader,
                  skyboxShader,
                  skyboxTextureId);

        // RESET TO DEFAULT FRAMEBUFFER
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ENABLE DEPTH TESTING TO NOT OVERDRAW BEHIND THE MIRROR
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        // DRAW MIRROR TO TOP OF THE SCREEN QUAD
        // screenShader.use();
        // screenShader.setInt("screenTexture", 0);
        // glBindVertexArray(mirrorVAO);
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, mirrorTextureColorBuffer);
        // glDrawArrays(GL_TRIANGLES, 0, 6);

        // DRAW MAIN TO FULLSCREEN QUAD
        screenShader.use();
        screenShader.setInt("screenTexture", 0);
        glBindVertexArray(screenQuadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mainTextureColorBuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        updateWindowNameWithFPS(window, title);

        // finish up frame
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &screenQuadVAO);
    glDeleteBuffers(1, &screenQuadVBO);
    glDeleteRenderbuffers(1, &mainRbo);
    glDeleteFramebuffers(1, &mainFramebuffer);

    glfwTerminate();
    return 0;
}

void
drawScene(unsigned int framebuffer,
          vector<glm::vec3> positions,
          Model modelToDraw,
          Shader modelShader,
          Shader skyboxShader,
          int environmentMappingTextureId)
{
    // SET FRAMEBUFFER
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // DRAW TO MAIN FRAMEBUFFER
    for (auto position : positions)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, position);

        modelShader.use();

        modelShader.setMat4("model", model);

        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrixBlock);
        glBufferSubData(GL_UNIFORM_BUFFER,
                        0,
                        sizeof(glm::mat4),
                        glm::value_ptr(view));
        glBufferSubData(GL_UNIFORM_BUFFER,
                        sizeof(glm::mat4),
                        sizeof(glm::mat4),
                        glm::value_ptr(projection));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        modelShader.setUniformBlock("MatricesBlock", uboMatrixBlock);

        if (environmentMappingTextureId >= 0)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureId);
            modelShader.setInt("skybox", 0);
            modelShader.setVec3("viewPos", camera.Position);
        }

        modelToDraw.Draw(modelShader);
    }

    // DRAW SKYBOX
    glDepthFunc(GL_LEQUAL);

    skyboxShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureId);
    skyboxShader.setInt("skybox", 0);
    skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
    skyboxShader.setMat4("projection", projection);

    glBindVertexArray(skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDepthFunc(GL_LESS);
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
}
