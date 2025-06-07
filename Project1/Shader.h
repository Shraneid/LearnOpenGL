#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

class Shader
{
  public:
    unsigned int ID;

    Shader(const char* vertexShaderFilePath, const char* fragmentShaderFilePath)
    {
        string vertexCode;
        string fragmentCode;
        ifstream vShaderFile;
        ifstream fShaderFile;

        vShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
        fShaderFile.exceptions(ifstream::failbit | ifstream::badbit);

        try
        {
            vShaderFile.open(vertexShaderFilePath);
            fShaderFile.open(fragmentShaderFilePath);
            stringstream vShaderStream, fShaderStream;

            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();

            vShaderFile.close();
            fShaderFile.close();

            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (ifstream::failure e)
        {
            cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << endl;
        }

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        unsigned int vertexShader, fragmentShader;
        int success;
        char infoLog[512];

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vShaderCode, NULL);
        glCompileShader(vertexShader);

        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                 << infoLog << endl;
        }

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                 << infoLog << endl;
        }

        ID = glCreateProgram();
        glAttachShader(ID, vertexShader);
        glAttachShader(ID, fragmentShader);
        glLinkProgram(ID);

        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(ID, 512, NULL, infoLog);
            cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                 << infoLog << endl;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    Shader(const char* vertexShaderFilePath, const char* geometryShaderFilePath, const char* fragmentShaderFilePath)
    {
        string vertexCode;
        string geometryCode;
        string fragmentCode;
        ifstream vShaderFile;
        ifstream gShaderFile;
        ifstream fShaderFile;

        vShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
        gShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
        fShaderFile.exceptions(ifstream::failbit | ifstream::badbit);

        try
        {
            vShaderFile.open(vertexShaderFilePath);
            gShaderFile.open(geometryShaderFilePath);
            fShaderFile.open(fragmentShaderFilePath);
            stringstream vShaderStream, gShaderStream, fShaderStream;

            vShaderStream << vShaderFile.rdbuf();
            gShaderStream << gShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();

            vShaderFile.close();
            gShaderFile.close();
            fShaderFile.close();

            vertexCode = vShaderStream.str();
            geometryCode = gShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (ifstream::failure e)
        {
            cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << endl;
        }

        const char* vShaderCode = vertexCode.c_str();
        const char* gShaderCode = geometryCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        unsigned int vertexShader, geometryShader, fragmentShader;
        int success;
        char infoLog[512];

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vShaderCode, NULL);
        glCompileShader(vertexShader);

        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                 << infoLog << endl;
        }

        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShader, 1, &gShaderCode, NULL);
        glCompileShader(geometryShader);

        glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
            cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n"
                 << infoLog << endl;
        }

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                 << infoLog << endl;
        }

        ID = glCreateProgram();
        glAttachShader(ID, vertexShader);
        glAttachShader(ID, geometryShader);
        glAttachShader(ID, fragmentShader);
        glLinkProgram(ID);

        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(ID, 512, NULL, infoLog);
            cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                 << infoLog << endl;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(geometryShader);
        glDeleteShader(fragmentShader);
    }

    void use() const { glUseProgram(ID); }

    void setBool(const string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void setInt(const string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setFloat(const string& name, float value) const
    {
        auto location = glGetUniformLocation(ID, name.c_str());
        glUniform1f(location, value);
    }
    void setVec2(const string& name, glm::vec2 value) const
    {
        auto location = glGetUniformLocation(ID, name.c_str());
        glUniform2fv(location, 1, glm::value_ptr(value));
    }
    void setVec3(const string& name, glm::vec3 value) const
    {
        auto location = glGetUniformLocation(ID, name.c_str());
        glUniform3fv(location, 1, glm::value_ptr(value));
    }
    void setMat4(const string& name, glm::mat4 value) const
    {
        auto location = glGetUniformLocation(ID, name.c_str());
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
    }
    void setUniformBlock(const string& name, unsigned int buffer) const
    {
        unsigned int matricesIndex =
          glGetUniformBlockIndex(ID, "MatricesBlock");
        glUniformBlockBinding(ID, matricesIndex, 1);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, buffer);
    }
};

#endif
