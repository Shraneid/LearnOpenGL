#ifndef LIGHT
#define LIGHT

#include "Shader.h"
#include <random>

class Light
{
  public:
    int GetLightId() const { return id; }

    virtual void setUniforms(Shader& shader) const = 0;

  protected:
    Light() = default;

    static int directionalLightsCounter;
    static int pointLightsCounter;
    static int spotLightsCounter;

    int id;
    virtual void setId() = 0;
};

int Light::directionalLightsCounter = 0;
int Light::pointLightsCounter = 0;
int Light::spotLightsCounter = 0;

class DirectionalLight : public Light
{
  public:
    DirectionalLight(glm::vec3 dir,
                     glm::vec3 amb,
                     glm::vec3 diff,
                     glm::vec3 spec)
      : Light()
      , direction(dir)
      , ambient(amb)
      , diffuse(diff)
      , specular(spec)
    {
        setId();
        std::cout << "directional light id : " << GetLightId() << std::endl;
    }

    void setUniforms(Shader& shader) const override
    {
        shader.setInt("NUMBER_OF_DIRECTIONAL_LIGHTS",
                      Light::directionalLightsCounter);

        string lightGLID =
          "directionalLights[" + std::to_string(GetLightId()) + "]";

        shader.setVec3(lightGLID + ".direction", direction);
        shader.setVec3(lightGLID + ".ambient", ambient);
        shader.setVec3(lightGLID + ".diffuse", diffuse);
        shader.setVec3(lightGLID + ".specular", specular);
    }

  private:
    void setId() { id = directionalLightsCounter++; }

    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

class PointLight : public Light
{
  public:
    PointLight(glm::vec3* pos,
               glm::vec3 amb,
               glm::vec3 diff,
               glm::vec3 spec,
               float cons,
               float lin,
               float quad)
      : Light()
      , position(pos)
      , ambient(amb)
      , diffuse(diff)
      , specular(spec)
      , constant(cons)
      , linear(lin)
      , quadratic(quad)
    {
        setId();
        std::cout << "point light id : " << GetLightId() << std::endl;
    }

    void setUniforms(Shader& shader) const override
    {
        shader.setInt("NUMBER_OF_POINT_LIGHTS",
                      Light::pointLightsCounter);

        const string lightGLID =
          "pointLights[" + std::to_string(GetLightId()) + "]";

        shader.setVec3(lightGLID + ".ambient", ambient);
        shader.setVec3(lightGLID + ".position", *position);
        shader.setVec3(lightGLID + ".diffuse", diffuse);
        shader.setVec3(lightGLID + ".specular", specular);
        shader.setFloat(lightGLID + ".constant", constant);
        shader.setFloat(lightGLID + ".linear", linear);
        shader.setFloat(lightGLID + ".quadratic", quadratic);
    }

    glm::vec3* getPosition() { return position; }
    glm::vec3 getDiffuse() { return diffuse; }

  private:
    void setId() { id = pointLightsCounter++; }

    glm::vec3* position;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

class SpotLight : public Light
{
  public:
    SpotLight(glm::vec3 pos,
              glm::vec3 dir,
              glm::vec3 amb,
              glm::vec3 diff,
              glm::vec3 spec,
              float cons,
              float lin,
              float quad,
              float inner,
              float outer)
      : Light()
      , position(pos)
      , direction(dir)
      , ambient(amb)
      , diffuse(diff)
      , specular(spec)
      , constant(cons)
      , linear(lin)
      , quadratic(quad)
      , innerCutoff(inner)
      , outerCutoff(outer)
    {
        setId();
        std::cout << "spot light id : " << GetLightId() << std::endl;
    }

    void setUniforms(Shader& shader) const override
    {
        shader.setInt("NUMBER_OF_SPOT_LIGHTS", Light::spotLightsCounter);
 
        const string lightGLID =
          "spotLights[" + std::to_string(GetLightId()) + "]";

        shader.setVec3(lightGLID + ".position", position);
        shader.setVec3(lightGLID + ".direction", direction);
        shader.setVec3(lightGLID + ".ambient", ambient);
        shader.setVec3(lightGLID + ".diffuse", diffuse);
        shader.setVec3(lightGLID + ".specular", specular);
        shader.setFloat(lightGLID + ".constant", constant);
        shader.setFloat(lightGLID + ".linear", linear);
        shader.setFloat(lightGLID + ".quadratic", quadratic);
        shader.setFloat(lightGLID + ".innerCutoff", innerCutoff);
        shader.setFloat(lightGLID + ".outerCutoff", outerCutoff);
    }

  private:
    void setId() { id = spotLightsCounter++; }

    glm::vec3 position;
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float innerCutoff;
    float outerCutoff;
};

#endif
