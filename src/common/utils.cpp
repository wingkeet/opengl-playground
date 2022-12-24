#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <string_view>
#include "utils.h"

bool ends_with(std::string_view str, std::string_view suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

bool starts_with(std::string_view str, std::string_view prefix)
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

std::string read_file(std::string_view filename)
{
    std::ifstream file{filename.data()};
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Returns the directory name of the current executable
std::filesystem::path dirname()
{
    return std::filesystem::canonical("/proc/self/exe").parent_path();
}
