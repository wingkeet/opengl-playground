#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED

#include <filesystem>
#include <initializer_list>
#include "glad.h"

extern GLuint compile_shaders();
extern GLuint compile_shaders(std::initializer_list<std::filesystem::path> paths);

#endif // SHADER_H_INCLUDED
