#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <string>
#include <vector>
#include "stb_image.h"
#include <iostream>
#include "Shader.h"

class Skybox
{
  public:
    int textureId;

    Skybox(const std::string& path, Shader shader)
      : directory(path)
      , shader(shader)
    {
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);

        glBindVertexArray(skyboxVAO);

        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER,
                     skyboxVertices.size() * sizeof(float),
                     skyboxVertices.data(),
                     GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              3 * sizeof(GL_FLOAT),
                              (void*)0);

        std::vector<std::string> faces_filepaths = {
            directory + "/posx.jpg", directory + "/negx.jpg",
            directory + "/posy.jpg", directory + "/negy.jpg",
            directory + "/posz.jpg", directory + "/negz.jpg",
        };

        textureId = loadCubemap(faces_filepaths);
    }

    unsigned int loadCubemap(std::vector<std::string> faces_filepath)
    {
        stbi_set_flip_vertically_on_load(false);

        unsigned int cubemapTextureID;
        glGenTextures(1, &cubemapTextureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);

        int width, height, nrChannels;
        unsigned char* data;
        for (unsigned int i = 0; i < faces_filepath.size(); i++)
        {
            data = stbi_load(faces_filepath[i].c_str(),
                             &width,
                             &height,
                             &nrChannels,
                             0);
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
        glTexParameteri(GL_TEXTURE_CUBE_MAP,
                        GL_TEXTURE_WRAP_S,
                        GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP,
                        GL_TEXTURE_WRAP_T,
                        GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP,
                        GL_TEXTURE_WRAP_R,
                        GL_CLAMP_TO_EDGE);

        stbi_set_flip_vertically_on_load(true);

        return cubemapTextureID;
    }

    void Draw(glm::mat4 projection, glm::mat4 view)
    {
        glDepthFunc(GL_LEQUAL);

        shader.use();
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
        shader.setInt("skybox", 10);
        shader.setMat4("view", glm::mat4(glm::mat3(view)));
        shader.setMat4("projection", projection);

        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glDepthFunc(GL_LESS);
    }

  private:
    Shader shader;

    std::string directory;

    unsigned int skyboxVBO, skyboxVAO;

    const std::array<float, 108> skyboxVertices{
        // clang-format off
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
    }; // clang-format 
};

#endif