#include "log.h"

#if MDB_LOG_LEVEL != MDB_LOG_LEVEL_OFF

#include "spdlog/sinks/stdout_color_sinks.h"

namespace mdb {

Log Log::instance;

Log::Log() :
    master(spdlog::stdout_color_st("MDB"))
{
    spdlog::set_pattern("[%T] %^%n:%$ %v"); // TODO: don't set globally
}

} // namespace mdb

#endif