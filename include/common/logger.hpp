#pragma once
#include <string>

//========================================================================

#define ELPP_CUSTOM_COUT std::cerr

#include "easylogging++.h"

// Runtime-проверки, которые сохраняются в release
#define CHECK_ALWAYS_COMMENT(x, comment) if(!(x)) { throw std::runtime_error(std::string("Runtime error: ")+(comment)+"; failed: "+(#x)); }
#define CHECK_ALWAYS(x) CHECK_ALWAYS_COMMENT(x, "always checked condition")

#ifdef NDEBUG
    #undef CHECK
    #define CHECK(x)
#endif

// Т.к. easylogging включает библиотеку, где они переопределяются
#undef min
#undef max

//========================================================================

namespace plane_render {

void ConfigureLogger(const std::string& config_file);

} //namespace plane_render