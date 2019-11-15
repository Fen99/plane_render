#include "logger.hpp"

//========================================================================

INITIALIZE_EASYLOGGINGPP

//========================================================================

namespace plane_render {

void ConfigureLogger(const std::string& config_file)
{
    el::Configurations conf(config_file);
    el::Loggers::reconfigureAllLoggers(conf);
}

} //namespace plane_render