#include "utils.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
void setUniformInt(const unsigned int shader, const char* name, int value) {
	const unsigned int location = glGetUniformLocation(shader, name);
	glUniform1i(location, value);
}