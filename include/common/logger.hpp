#pragma once
#include <string>

//========================================================================

#define ELPP_CUSTOM_COUT std::cerr

// Отлючаем DCHECK в Release
#if ((!defined(_DEBUG)) || (defined(NDEBUG)))
    #define ELPP_DISABLE_DEBUG_LOGS
#endif

#include "easylogging++.h"

// Т.к. easylogging включает библиотеку, где они переопределяются
#undef min
#undef max

//========================================================================

namespace plane_render {

void ConfigureLogger(const std::string& config_file);

} //namespace plane_render