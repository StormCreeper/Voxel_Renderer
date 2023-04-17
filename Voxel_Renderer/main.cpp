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

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>

// Link opengl32.lib, glfw3.lib and glew32.lib
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glew32.lib")

struct Camera {
	glm::vec3 position;
	glm::vec3 position;
};


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


	// Loading the shaders 

	std::ifstream vertexShaderFile("vertexShader.glsl");
	std::stringstream vertexShaderBuffer;
	vertexShaderBuffer << vertexShaderFile.rdbuf();

	std::ifstream fragmentShaderFile("fragmentShader.glsl");
	std::stringstream fragmentShaderBuffer;
	fragmentShaderBuffer << fragmentShaderFile.rdbuf();

	std::string vertexShaderSource = vertexShaderBuffer.str();
	std::string fragmentShaderSource = fragmentShaderBuffer.str();


	const char* vertexShaderSourcePtr = vertexShaderSource.c_str();
	const char* fragmentShaderSourcePtr = fragmentShaderSource.c_str();

	glShaderSource(vertexShader, 1, &vertexShaderSourcePtr, NULL);
	glShaderSource(fragmentShader, 1, &fragmentShaderSourcePtr, NULL);

	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	glAttachShader(shader, vertexShader);
	glAttachShader(shader, fragmentShader);

	glLinkProgram(shader);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glUseProgram(shader);

	// Main rendering / event loop
	while (!glfwWindowShouldClose(window)) {
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw the main quad
		
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