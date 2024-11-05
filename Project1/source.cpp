#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"
#include "Shader.h"
#include "Camera.h"
#include "filesystem.h"


void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int load_texture(const char* path);

int windowWidth = 1920, windowHeight = 1080;

Camera camera = Camera(
	glm::vec3(4.269909f, 2.050022f, -4.690299f),	// Position
	glm::vec3(0.0f, 1.0f, 0.0f),					// World UP
	-224.580002f, -19.940031f						// Yaw, Pitch
);
int lastX, lastY;

bool blackLightOn = false;
float blackLightButtonLastTimePressed = 0.0f;

// physics
float elapsedTime = 0.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// setup for fps counter
double currentTime, delta;
double lastTime = glfwGetTime();
int nbFrames = 0;

struct DirectionalLight {
	glm::vec3 direction;
	
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

struct PointLight {
	glm::vec3 position;
	
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	
	float constant;
	float linear;
	float quadratic;
};

struct SpotLight {
	glm::vec3* direction;
	const glm::vec3* position;

	float innerCutoff;
	float outerCutoff;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	
	float constant;
	float linear;
	float quadratic;

	glm::vec3 getDirection() const {
		return *direction;
	}

	glm::vec3 getPosition() const {
		return *position;
	}
};


int main() {
	stbi_set_flip_vertically_on_load(true);

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	string title = "gpu go brrr";

	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), NULL, NULL);
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

	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	unsigned int indices[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
		10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 
		20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
		30, 31, 32, 33, 34, 35
	};

	// GENERATING VBO VAO EBO
	unsigned int VBO, VAO, EBO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);


	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);


	unsigned int lightSourceVAO;

	glGenVertexArrays(1, &lightSourceVAO);
	glBindVertexArray(lightSourceVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(0);

	std::vector<glm::vec3> cubePositions = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	// LOADING TEXTURES
	unsigned int container_texture_id = load_texture("./resources/textures/container2.png");
	unsigned int container_specular_texture_id = load_texture("./resources/textures/container2_specular.png");
	unsigned int matrix_texture_id = load_texture("./resources/textures/matrix.jpg");
	unsigned int blood_texture_id = load_texture("./resources/textures/blood.png");

	// COMPILING SHADERS
	Shader litCubeShader(
		"VertexShaderWithNormal.hlsl",
		"FragmentShaderLit.hlsl"
	);
	Shader lightSourceShader(
		"VertexShaderBase.hlsl",
		"FragmentShaderBase.hlsl"
	);

	litCubeShader.use();
	litCubeShader.setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
	litCubeShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
	litCubeShader.setInt("material.diffuseMap", 0);
	litCubeShader.setInt("material.specularMap", 1);
	litCubeShader.setInt("material.emissionMap", 2);

	glm::mat4 model, view, projection;

	// Lights setup
	DirectionalLight directionalLight = DirectionalLight{
		glm::vec3(0.5f, -1.0f, 0.2f),	//pos
		glm::vec3(0.0f),				//ambient
		glm::vec3(0.0f),				//diffuse
		glm::vec3(0.5f)					//specular
	};

	std::vector<PointLight> basePointLights = {
		PointLight{
			glm::vec3(2.5f, 0.0f, 0.0f),
			glm::vec3(0.2f),
			glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(1.0f),
			1.0f, 0.045f, 0.0075f
		},
		PointLight{
			glm::vec3(2.3f, 3.3f, -4.0f),
			glm::vec3(0.2f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(1.0f),
			1.0f, 0.045f, 0.0075f
		},
		PointLight{
			glm::vec3(0.0f, 0.0f, -3.0f),
			glm::vec3(0.2f),
			glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(1.0f),
			1.0f, 0.045f, 0.0075f
		},
		PointLight{
			glm::vec3(-4.0f, 2.0f, -12.0f),
			glm::vec3(0.2f),
			glm::vec3(1.0f),
			glm::vec3(1.0f),
			1.0f, 0.045f, 0.0075f
		}
	};
	std::vector<PointLight> pointLights = basePointLights;

	SpotLight spotLight = SpotLight{
		&camera.Front,
		&camera.Position,

		cos(glm::radians(10.0f)),
		cos(glm::radians(15.0f)),

		glm::vec3(0.2f),
		glm::vec3(1.0f),
		glm::vec3(1.0f),

		1.0f,
		0.045f,
		0.0075f
	};

	while (!glfwWindowShouldClose(window)) {
		processInput(window);

		// background
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// physics
		elapsedTime = (float)glfwGetTime();
		
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);

		// render light source cube
		for (int i = 0; i < pointLights.size(); i++) {
			model = glm::mat4(1.0f);

			float range = 10.0f;
			float speed = 1.0f;
			
			model = glm::translate(model, pointLights[i].position);
			model = glm::scale(model, glm::vec3(0.2f));

			lightSourceShader.use();
			lightSourceShader.setMat4("model", model);
			lightSourceShader.setMat4("view", view);
			lightSourceShader.setMat4("projection", projection);

			lightSourceShader.setVec3("lightColor", glm::vec3(pointLights[i].diffuse));

			glBindVertexArray(lightSourceVAO);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		// render object cube
		litCubeShader.use();

		litCubeShader.setFloat("elapsedTime", elapsedTime);

		litCubeShader.setMat4("view", view);
		litCubeShader.setMat4("projection", projection);

		litCubeShader.setVec3("viewPos", camera.Position);
		
		// directional light
		litCubeShader.setVec3("directionalLight.direction", directionalLight.direction);
		litCubeShader.setVec3("directionalLight.ambient", directionalLight.ambient);
		litCubeShader.setVec3("directionalLight.diffuse", directionalLight.diffuse);
		litCubeShader.setVec3("directionalLight.specular", directionalLight.specular);
		
		// move lights around
		pointLights[0].position = basePointLights[0].position + glm::vec3(sin(elapsedTime), 0.0f, 0.0f);
		pointLights[1].position = basePointLights[1].position + glm::vec3(0.0f, sin(elapsedTime), 0.0f);
		pointLights[2].position = basePointLights[2].position + glm::vec3(sin(elapsedTime), sin(elapsedTime), 0.0f);

		// point lights uniforms
		for (int i = 0; i < pointLights.size(); i++) {
			std::string indexString = std::to_string(i);

			litCubeShader.setVec3(
				"pointLights[" + indexString + "].position",
				pointLights[i].position
			);
			litCubeShader.setFloat(
				"pointLights[" + indexString + "].constant",
				pointLights[i].constant
			);
			litCubeShader.setFloat(
				"pointLights[" + indexString + "].linear",
				pointLights[i].linear
			);
			litCubeShader.setFloat(
				"pointLights[" + indexString + "].quadratic",
				pointLights[i].quadratic
			);
			litCubeShader.setVec3(
				"pointLights[" + indexString + "].ambient",
				pointLights[i].ambient
			);
			litCubeShader.setVec3(
				"pointLights[" + indexString + "].diffuse",
				pointLights[i].diffuse
			);
			litCubeShader.setVec3(
				"pointLights[" + indexString + "].specular",
				pointLights[i].specular
			);
		}

		// spot light uniforms
		litCubeShader.setBool("blackLightOn", blackLightOn);

		litCubeShader.setVec3("spotLight.direction", spotLight.getDirection());
		litCubeShader.setVec3("spotLight.position", spotLight.getPosition());

		litCubeShader.setFloat("spotLight.innerCutoff", spotLight.innerCutoff);
		litCubeShader.setFloat("spotLight.outerCutoff", spotLight.outerCutoff);

		litCubeShader.setVec3("spotLight.ambient", spotLight.ambient);
		litCubeShader.setVec3("spotLight.diffuse", spotLight.diffuse);
		litCubeShader.setVec3("spotLight.specular", spotLight.specular);

		litCubeShader.setFloat("spotLight.constant", spotLight.constant);
		litCubeShader.setFloat("spotLight.linear", spotLight.linear);
		litCubeShader.setFloat("spotLight.quadratic", spotLight.quadratic);
				
		// material
		litCubeShader.setVec3("material.specular", glm::vec3(0.628281, 0.555802, 0.366065));
		litCubeShader.setFloat("material.shininess", 200.0f);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, container_texture_id);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, container_specular_texture_id);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, blood_texture_id);

		for (int i = 0; i < cubePositions.size(); i++) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);

			//float angle = (20.0f + time * 10.0f) * i;
			float angle = (20.0f) * i;
			//model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

			litCubeShader.setMat4("model", model);

			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -52.5f, 0.0f));
		model = glm::scale(model, glm::vec3(100.0f));

		litCubeShader.setMat4("model", model);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		// show fps in window name
		currentTime = glfwGetTime();
		delta = currentTime - lastTime;

		if (delta >= 1.0) {
			glfwSetWindowTitle(window, (title + "\t" + to_string(nbFrames) + "FPS").c_str());
			nbFrames = 0;
			lastTime = currentTime;
		}
		else {
			nbFrames++;
		}

		// finish up frame
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
		std::cout << "camera position: (" 
			+ std::to_string(camera.Position.x)
			+ ", "
			+ std::to_string(camera.Position.y) 
			+ ", "
			+ std::to_string(camera.Position.z) 
			+ ")"
			<< std::endl;
		std::cout << "(yaw, pitch): (" 
			+ std::to_string(camera.Yaw)
			+ ", "
			+ std::to_string(camera.Pitch)
			+ ")"
			<< std::endl;
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		if (elapsedTime - blackLightButtonLastTimePressed > 0.07f)
			blackLightOn = !blackLightOn;
			blackLightButtonLastTimePressed = elapsedTime;
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

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	windowWidth = width;
	windowHeight = height;
}

unsigned int load_texture(const char* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format = GL_RGB;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}