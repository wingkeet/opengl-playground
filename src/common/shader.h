#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED

#include <initializer_list>
#include <string_view>
#include "glad.h"

extern GLuint compile_shaders(std::initializer_list<std::string_view> filenames);

#endif // SHADER_H_INCLUDED
