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

#include "utils.h"
#include "Structs.h"

#include <random>

// Link opengl32.lib, glfw3.lib and glew32.lib
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glew32.lib")

int width = 800;
int height = 800;

float degtorad = 3.141592f / 180.0f;

Camera camera;

int frameSinceLastReset = 0;

void cameraMouseCallback(GLFWwindow *window, const double posX, const double posY) {

	frameSinceLastReset = 0;

	const float offset_x = posX - camera.lastX;
	const float offset_y = posY - camera.lastY;

	camera.lastX = width / 2;
	camera.lastY = height / 2;

	const float sensitivity = 0.1f;

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

bool useFresnel = false;

void keyPressedCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	frameSinceLastReset = 0;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		useFresnel = !useFresnel;
	}

	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_W)
			camera.keys.w = true;
		if (key == GLFW_KEY_A)
			camera.keys.a = true;
		if (key == GLFW_KEY_S)
			camera.keys.s = true;
		if (key == GLFW_KEY_D)
			camera.keys.d = true;
		if (key == GLFW_KEY_SPACE)
			camera.keys.space = true;
		if (key == GLFW_KEY_LEFT_CONTROL)
			camera.keys.left_control = true;
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_W)
			camera.keys.w = false;
		if (key == GLFW_KEY_A)
			camera.keys.a = false;
		if (key == GLFW_KEY_S)
			camera.keys.s = false;
		if (key == GLFW_KEY_D)
			camera.keys.d = false;
		if (key == GLFW_KEY_SPACE)
			camera.keys.space = false;
		if (key == GLFW_KEY_LEFT_CONTROL)
			camera.keys.left_control = false;
	}
}


void initCamera() {
	camera.position = glm::vec3(0.0f, 0.5f, -5.0f);
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
	camera.keys = { };
}

std::string loadFile(std::string filename) {
	std::ifstream t(filename);
	std::stringstream buffer;
	buffer << t.rdbuf();
	return buffer.str();
}

void updateCamera(float deltaTime) {
	const float cameraSpeed = 5.0f * deltaTime;
	if (camera.keys.w)
		camera.position += cameraSpeed * camera.front;
	if (camera.keys.s)
		camera.position -= cameraSpeed * camera.front;
	if (camera.keys.a)
		camera.position -= camera.right * cameraSpeed;
	if (camera.keys.d)
		camera.position += camera.right * cameraSpeed;
	if (camera.keys.space)
		camera.position += cameraSpeed * camera.worldUp;
	if (camera.keys.left_control)
		camera.position -= cameraSpeed * camera.worldUp;

	if (camera.keys.w || camera.keys.a || camera.keys.s || camera.keys.d || camera.keys.space || camera.keys.left_control) {
		frameSinceLastReset = 0;
	}
}

bool initOpenGL(AppState *appState) {
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return 0;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a windowed mode window and its OpenGL context
	appState->window = glfwCreateWindow(800, 800, "Hello World", NULL, NULL);
	if (!appState->window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return 0;
	}
	// Make the window's context current
	glfwMakeContextCurrent(appState->window);
	glfwSwapInterval(0);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return 0;
	}

	return 1;
}

int main() {
	srand(time(NULL));

	AppState appState;
	
	if(!initOpenGL(&appState))
		return -1;


	// Create the quad to render the voxel scene
	float vertices[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f
	};

	glGenVertexArrays(1, &appState.vao);
	glGenBuffers(1, &appState.vbo);
	
	glBindVertexArray(appState.vao);
	glBindBuffer(GL_ARRAY_BUFFER, appState.vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Load the shader from a file
	appState.shader = glCreateProgram();
	appState.quad_shader = glCreateProgram();
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint quad_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);


	// Loading the shaders and checking for compilation errors

	std::string vertexShaderSource = loadFile("vertex.glsl");
	std::string fragmentShaderSource = loadFile("fragment.glsl");
	std::string quadFragShaderSource = loadFile("quad_frag.glsl");

	// TODO: 

	const char* vertexShaderSourceC = vertexShaderSource.c_str();
	const char* fragmentShaderSourceC = fragmentShaderSource.c_str();
	const char* quadFragShaderSourceC = quadFragShaderSource.c_str();

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

	glShaderSource(quad_fragmentShader, 1, &quadFragShaderSourceC, NULL);
	glCompileShader(quad_fragmentShader);
	glGetShaderiv(quad_fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cerr << "Failed to compile quad fragment shader: " << infoLog << std::endl;
	}

	glAttachShader(appState.shader, vertexShader);
	glAttachShader(appState.shader, fragmentShader);
	glLinkProgram(appState.shader);
	glGetProgramiv(appState.shader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(appState.shader, 512, NULL, infoLog);
		std::cerr << "Failed to link shader program: " << infoLog << std::endl;
	}

	glAttachShader(appState.quad_shader, vertexShader);
	glAttachShader(appState.quad_shader, quad_fragmentShader);
	glLinkProgram(appState.quad_shader);
	glGetProgramiv(appState.quad_shader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(appState.shader, 512, NULL, infoLog);
		std::cerr << "Failed to link quad shader program: " << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(quad_fragmentShader);

	// Init shader storage buffer

	shader_data s_data = { 50, 2, 10};
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			for (int k = 0; k < 10; k++) {
				s_data.data[i + 10 * j + 100 * k] = (rand() % 2) * (rand() % 10);
			}
		}
	}
	for (int i = 0; i < 10; i++) {
		s_data.palette[i] = glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX);
	}

	glGenBuffers(1, &appState.ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, appState.ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(shader_data), &s_data, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, appState.ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// Init the frame buffers

	Framebuffer *fb1 = &appState.fb1;
	fb1->width = width;
	fb1->height = height;
	glGenFramebuffers(1, &fb1->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fb1->fbo);
	
	glGenTextures(1, &fb1->colorTexture);
	glBindTexture(GL_TEXTURE_2D, fb1->colorTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, fb1->width, fb1->height, 0, GL_RGBA, GL_FLOAT, nullptr);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fb1->colorTexture, 0);

	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Failed to create framebuffer 1" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Framebuffer *fb2 = &appState.fb2;
	fb2->width = width;
	fb2->height = height;
	glGenFramebuffers(1, &fb2->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fb2->fbo);

	glGenTextures(1, &fb2->colorTexture);
	glBindTexture(GL_TEXTURE_2D, fb2->colorTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, fb2->width, fb2->height, 0, GL_RGBA, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fb2->colorTexture, 0);

	//GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Failed to create framebuffer 2" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Init the camera
	
	bool s_data_changed = false;

	int spp = 1;
	int bounces = 10;

	initCamera();

	float lastChangeTime = glfwGetTime();

	// Callbacks

	glfwSetCursorPosCallback(appState.window, cameraMouseCallback);
	glfwSetKeyCallback(appState.window, keyPressedCallback);

	float lastTime = glfwGetTime();
	float lastTimeFPS = glfwGetTime();
	int frameCount = 0;
	int frame = 0;

	// Main rendering / event loop
	while (!glfwWindowShouldClose(appState.window)) {

		// Update the camera
		float currentTime = glfwGetTime();
		float deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		frameCount++;
		if (currentTime > lastTimeFPS + 1) {
			std::cout << "FPS : " << frameCount << std::endl << "SPP : " << spp << std::endl << std::endl;
			frameCount = 0;
			lastTimeFPS = currentTime;
		}

		if (abs(1.0f/deltaTime - 60) > 2) {
			float sampleTime = deltaTime / spp;
			float newSPP = 1.0f / (sampleTime * 60);
			spp = (int)(spp + (newSPP - spp) * 0.1f);
			if(spp < 1) spp = 1;
			if (spp > 500) spp = 500;
		}

		updateCamera(deltaTime);

		fb1 = (frame % 2 == 0) ? &appState.fb1 : &appState.fb2;
		fb2 = (frame % 2 == 1) ? &appState.fb1 : &appState.fb2;

		glBindFramebuffer(GL_FRAMEBUFFER, fb1->fbo);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glBindVertexArray(appState.vao);

		glViewport(0, 0, width, height);

		glUseProgram(appState.shader);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, appState.ssbo);

		if (s_data_changed) {

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, appState.ssbo);
			GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
			memcpy(p, &s_data, sizeof(shader_data));
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

			s_data_changed = false;
		}

		camera.projection = glm::perspective(glm::radians(70.0f ), (float)width / (float)height, 0.1f, 100.0f);
		camera.view = glm::lookAt(camera.position, camera.position + camera.front, camera.up);

		// First Pass
		// TODO: Create the quad shader, make second and first pass, edit the fragment shader to do framebuffer.

		// Draw the main quad

		setUniformV2(appState.shader, "u_Resolution", glm::vec2(width, height));
		setUniformF(appState.shader, "u_Time", glfwGetTime());

		setUniformM4(appState.shader, "u_InverseView", glm::inverse(camera.view));
		setUniformM4(appState.shader, "u_InverseProjection", glm::inverse(camera.projection));

		setUniformInt(appState.shader, "useFresnel", useFresnel);
		
		setUniformInt(appState.shader, "u_SPP", spp);
		setUniformInt(appState.shader, "u_Bounces", bounces);

		setUniformInt(appState.shader, "u_FrameSinceLastReset", frameSinceLastReset);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fb2->colorTexture);

		setUniformInt(appState.shader, "u_LastColors", 0);

		glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//  Second pass

		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, width, height);
		
		glBindVertexArray(appState.vao);
		glUseProgram(appState.quad_shader);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fb1->colorTexture);

		setUniformInt(appState.quad_shader, "u_Texture", 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindVertexArray(0);

		// Update screen

		glfwSwapBuffers(appState.window);
		glfwPollEvents();

		frame++;
		frameSinceLastReset++;
	}

	// Cleanup
	glDeleteVertexArrays(1, &appState.vao);
	glDeleteBuffers(1, &appState.vbo);
	glDeleteProgram(appState.shader);
	glDeleteBuffers(1, &appState.ssbo);
	glDeleteFramebuffers(1, &fb1->fbo);
	glDeleteFramebuffers(1, &fb2->fbo);
	glDeleteTextures(1, &fb1->colorTexture);
	glDeleteTextures(1, &fb2->colorTexture);

	glfwTerminate();

	return 0;
}