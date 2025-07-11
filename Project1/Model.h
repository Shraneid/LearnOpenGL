#ifndef MODEL
#define MODEL

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include "stb_image.h"
#include "Mesh.h"

class Model
{
  public:
    vector<Mesh> meshes;
    Model(string path) { loadModel(path); }
    void Draw(Shader& shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
        {
            meshes[i].Draw(shader);
        }
    }

  private:
    string directory;
    vector<Texture> loaded_textures;

    void loadModel(string path)
    {
        Assimp::Importer importer;

        const aiScene* scene =
          importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
            !scene->mRootNode)
        {
            cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));

        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;

            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;

            vertex.Position = vector;

            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;

            vertex.Normal = vector;

            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec;

                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;

                vertex.TexCoords = vec;
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        if (mesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            vector<Texture> diffuseMaps =
              loadMaterialTextures(material,
                                   aiTextureType_DIFFUSE,
                                   "texture_diffuse");
            textures.insert(textures.end(),
                            diffuseMaps.begin(),
                            diffuseMaps.end());

            vector<Texture> specularMaps =
              loadMaterialTextures(material,
                                   aiTextureType_SPECULAR,
                                   "texture_specular");
            textures.insert(textures.end(),
                            specularMaps.begin(),
                            specularMaps.end());
        }

        return Mesh(vertices, indices, textures);
    }

    vector<Texture> loadMaterialTextures(aiMaterial* mat,
                                         aiTextureType type,
                                         string typeName)
    {
        vector<Texture> textures;

        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString path;
            mat->GetTexture(type, i, &path);
            bool skip = false;

            for (unsigned int j = 0; j < loaded_textures.size(); j++)
            {
                if (std::strcmp(loaded_textures[j].path.data(), path.C_Str()) ==
                    0)
                {
                    textures.push_back(loaded_textures[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip)
            {
                Texture texture;

                texture.id = TextureFromFile(path.C_Str(), directory);
                texture.type = typeName;
                texture.path = path.C_Str();

                textures.push_back(texture);
                loaded_textures.push_back(texture);
            }
        }

        return textures;
    }

    unsigned int TextureFromFile(const char* localPath, string directory)
    {
        string filepath = directory + "/" + localPath;

        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data =
          stbi_load(filepath.c_str(), &width, &height, &nrComponents, 0);

        if (!data)
        {
            std::cout << "Texture failed to load at path: " << filepath
                      << std::endl;
            stbi_image_free(data);
        }

        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else
            throw std::runtime_error("issue with loading the 3D model");

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     format,
                     width,
                     height,
                     0,
                     format,
                     GL_UNSIGNED_BYTE,
                     data);

        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);

        return textureID;
    }
};
#endif
