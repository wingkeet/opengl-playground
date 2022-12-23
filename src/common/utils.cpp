#include "utils.h"

// Returns the directory name of the current executable
std::filesystem::path dirname()
{
    return std::filesystem::canonical("/proc/self/exe").parent_path();
}
