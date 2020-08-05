#ifndef LOGGING_H
#define LOGGING_H

// Log levels
#define MDB_LOG_LEVEL_TRACE 0
#define MDB_LOG_LEVEL_INFO  2
#define MDB_LOG_LEVEL_WARN  3
#define MDB_LOG_LEVEL_ERROR 4
#define MDB_LOG_LEVEL_OFF   6

// Set log level here
#define MDB_LOG_LEVEL MDB_LOG_LEVEL_INFO

// Don't include anything if there is no logging

#if MDB_LOG_LEVEL == MDB_LOG_LEVEL_OFF

#define MDB_TRACE(...)
#define MDB_INFO(...)
#define MDB_WARN(...)
#define MDB_ERROR(...)

#else // !MDB_LOG_LEVEL_OFF

// Use spdlog

#define SPDLOG_COMPILED_LIB
#include "spdlog/spdlog.h"

namespace mdb {

    class Log
    {
    public:

        Log();

        inline static std::shared_ptr<spdlog::logger>& MDBLogger() noexcept
        {
            return instance.master;
        }

    private:

        static Log instance;

        std::shared_ptr<spdlog::logger> master; // Single-thread
    };

#if MDB_LOG_LEVEL == MDB_LOG_LEVEL_TRACE
#define MDB_TRACE(...)  ::mdb::Log::MDBLogger()->trace(__VA_ARGS__)
#define MDB_INFO(...)   ::mdb::Log::MDBLogger()->info(__VA_ARGS__)
#define MDB_WARN(...)   ::mdb::Log::MDBLogger()->warn(__VA_ARGS__)
#define MDB_ERROR(...)  ::mdb::Log::MDBLogger()->error(__VA_ARGS__)

#elif MDB_LOG_LEVEL == MDB_LOG_LEVEL_INFO
#define MDB_TRACE(...)
#define MDB_INFO(...)   ::mdb::Log::MDBLogger()->info(__VA_ARGS__)
#define MDB_WARN(...)   ::mdb::Log::MDBLogger()->warn(__VA_ARGS__)
#define MDB_ERROR(...)  ::mdb::Log::MDBLogger()->error(__VA_ARGS__)

#elif MDB_LOG_LEVEL == MDB_LOG_LEVEL_WARN
#define MDB_TRACE(...)
#define MDB_INFO(...)
#define MDB_WARN(...)   ::mdb::Log::MDBLogger()->warn(__VA_ARGS__)
#define MDB_ERROR(...)  ::mdb::Log::MDBLogger()->error(__VA_ARGS__)

#elif MDB_LOG_LEVEL == MDB_LOG_LEVEL_ERROR
#define MDB_TRACE(...)
#define MDB_INFO(...)
#define MDB_WARN(...)
#define MDB_ERROR(...)  ::mdb::Log::MDBLogger()->error(__VA_ARGS__)

#else
#error Unknown MDB_LOG_LEVEL
#endif // MDB_LOG_LEVEL

//debug assert
//#define DASSERT(x); if ( !(x) ) { DLOG( "Debug Assertion: ( " << #x << " ) failed!") }

} // namespace mdb

#undef SPDLOG_COMPILED_LIB

#endif // MDB_LOG_LEVEL_OFF

#endif // !LOGGING_H