// OpenGL voxel raytracer projet
// Renders a voxel scene using raytracing
// CPU side : voxel scene generation, raytracing, image saving
// Only show a fullscreen rectangle, the voxel scene is rendered by a shader
// GPU side : voxel scene rendering, fullscreen rectangle rendering

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Link opengl32.lib, glfw3.lib and glew32.lib
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glew32.lib")

int width = 800;
int height = 800;

struct Camera {
	glm::vec3 position;
	glm::vec3 direction;
	float rotationX;
	float rotationY;

	glm::vec3 worldUp;

	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;

	float lastX;
	float lastY;

	glm::mat4 projection;
	glm::mat4 view;
};

float degtorad = 3.141592f / 180.0f;

Camera camera;

void cameraMouseCallback(GLFWwindow *window, const double posX, const double posY) {

	const float offset_x = posX - camera.lastX;
	const float offset_y = posY - camera.lastY;

	camera.lastX = width / 2;
	camera.lastY = height / 2;

	const float sensitivity = 0.2f;

	camera.rotationY += offset_x * sensitivity;
	camera.rotationX -= offset_y * sensitivity;

	if (camera.rotationX > 89.0f)
		camera.rotationX = 89.0f;
	if (camera.rotationX < -89.0f)
		camera.rotationX = -89.0f;


	camera.direction.x = cos(camera.rotationY * degtorad) * cos(camera.rotationX * degtorad);
	camera.direction.y = sin(camera.rotationX * degtorad);
	camera.direction.z = sin(camera.rotationY * degtorad) * cos(camera.rotationX * degtorad);
	camera.front = glm::normalize(camera.direction);
	camera.right = glm::normalize(glm::cross(camera.front, camera.worldUp));
	camera.up = glm::normalize(glm::cross(camera.right, camera.front));

	glfwSetCursorPos(window, width / 2, height / 2);

}

void setUniformM4(const unsigned int shader, const char* name, glm::mat4 matrix) {
	const unsigned int location = glGetUniformLocation(shader, name);
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}
void setUniformV3(const unsigned int shader, const char* name, glm::vec3 vector) {
	const unsigned int location = glGetUniformLocation(shader, name);
	glUniform3fv(location, 1, glm::value_ptr(vector));
}
void setUniformF(const unsigned int shader, const char* name, float value) {
	const unsigned int location = glGetUniformLocation(shader, name);
	glUniform1f(location, value);
}
void setUniformV2(const unsigned int shader, const char* name, glm::vec2 vector) {
	const unsigned int location = glGetUniformLocation(shader, name);
	glUniform2fv(location, 1, glm::value_ptr(vector));
}


void initCamera() {
	camera.position = glm::vec3(0.0f, 0.0f, -5.0f);
	camera.direction = glm::vec3(0.0f, 0.0f, 1.0f);
	camera.rotationX = 0.0f;
	camera.rotationY = 0.0f;
	camera.worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	camera.front = glm::normalize(camera.direction);
	camera.right = glm::normalize(glm::cross(camera.front, camera.worldUp));
	camera.up = glm::normalize(glm::cross(camera.right, camera.front));
	camera.lastX = width / 2;
	camera.lastY = height / 2;
	camera.projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	camera.view = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
}

std::string loadFile(std::string filename) {
	std::ifstream t(filename);
	std::stringstream buffer;
	buffer << t.rdbuf();
	return buffer.str();
}

int main() {
	// Initialize GLFW
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	// Create a windowed mode window and its OpenGL context
	GLFWwindow* window = glfwCreateWindow(800, 800, "Hello World", NULL, NULL);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Make the window's context current
	glfwMakeContextCurrent(window);
	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	// Create the quad to render the voxel scene
	float vertices[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f
	};
	
	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Load the shader from a file
	GLuint shader = glCreateProgram();
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);


	// Loading the shaders and checking for compilation errors

	std::string vertexShaderSource = loadFile("vertex.glsl");
	std::string fragmentShaderSource = loadFile("fragment.glsl");

	const char* vertexShaderSourceC = vertexShaderSource.c_str();
	const char* fragmentShaderSourceC = fragmentShaderSource.c_str();

	glShaderSource(vertexShader, 1, &vertexShaderSourceC, NULL);
	glCompileShader(vertexShader);
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cerr << "Failed to compile vertex shader: " << infoLog << std::endl;
	}

	glShaderSource(fragmentShader, 1, &fragmentShaderSourceC, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cerr << "Failed to compile fragment shader: " << infoLog << std::endl;
	}

	glAttachShader(shader, vertexShader);
	glAttachShader(shader, fragmentShader);
	glLinkProgram(shader);
	glGetProgramiv(shader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader, 512, NULL, infoLog);
		std::cerr << "Failed to link shader program: " << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	initCamera();


	// Callbacks

	glfwSetCursorPosCallback(window, cameraMouseCallback);

	// Main rendering / event loop
	while (!glfwWindowShouldClose(window)) {
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw the main quad
		glUseProgram(shader);

		camera.projection = glm::perspective(glm::radians(70.0f), (float)width / (float)height, 0.1f, 100.0f);
		camera.view = glm::lookAt(camera.position, camera.position + camera.front, camera.up);

		setUniformV2(shader, "u_Resolution", glm::vec2(width, height));
		setUniformF(shader, "u_Time", glfwGetTime());

		setUniformM4(shader, "u_InverseView", glm::inverse(camera.view));
		setUniformM4(shader, "u_InverseProjection", glm::inverse(camera.projection));

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteProgram(shader);

	glfwTerminate();


	return 0;
}