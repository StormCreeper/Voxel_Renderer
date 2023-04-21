#pragma once

#include <glm/glm.hpp>

void setUniformM4(const unsigned int shader, const char* name, glm::mat4 matrix);
void setUniformV3(const unsigned int shader, const char* name, glm::vec3 vector);
void setUniformF(const unsigned int shader, const char* name, float value);
void setUniformV2(const unsigned int shader, const char* name, glm::vec2 vector);
void setUniformInt(const unsigned int shader, const char* name, int value);