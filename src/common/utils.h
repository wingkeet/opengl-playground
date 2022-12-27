#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <filesystem>
#include <string_view>
#include "glad.h"

extern bool ends_with(std::string_view str, std::string_view suffix);
extern bool starts_with(std::string_view str, std::string_view prefix);
extern std::string read_file(std::string_view filename);
extern std::filesystem::path dirname();

extern constexpr GLvoid* buffer_offset(GLintptr offset)
{
    return (GLvoid*) offset;
}

#endif // UTILS_H_INCLUDED
