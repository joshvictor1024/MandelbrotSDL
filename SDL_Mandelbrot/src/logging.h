#ifndef LOGGING_H
#define LOGGING_H

#include <iostream>

// List log levels here
#define LOG_LEVEL_NONE      0x0
#define LOG_LEVEL_RELEASE   0x1
#define LOG_LEVEL_INFO      0x2
#define LOG_LEVEL_DEBUG     0x4

// Set log level here
#define LOG_LEVEL LOG_LEVEL_RELEASE

#if LOG_LEVEL == LOG_LEVEL_NONE

#define RLOG(message);
#define ILOG(message);
#define DLOG(message);
#define STAMP

#elif LOG_LEVEL == LOG_LEVEL_RELEASE

#define RLOG(message); std::cout << message << "\n";
#define ILOG(message);
#define DLOG(message);
#define STAMP

#elif LOG_LEVEL == LOG_LEVEL_INFO

#define RLOG(message); std::cout << message << "\n";
#define ILOG(message); std::cout << message << "\n";
#define DLOG(message);
#define STAMP

#elif LOG_LEVEL == LOG_LEVEL_DEBUG

#define RLOG(message); std::cout << message << "\n";
#define ILOG(message); std::cout << message << "\n";
#define DLOG(message); std::cout << message << "\n";
#define STAMP std::cout << "[ln:" << __LINE__ << '\t' << __FILE__ << "\t]\n";

#endif

//debug assert
//#define DASSERT(x); if ( !(x) ) { DLOG( "Debug Assertion: ( " << #x << " ) failed!") }

#endif // !LOGGING_H

// Possible implementation with variadic macro and comma operator

//#include<iostream>
//#define LOG(...) std::cout , __VA_ARGS__
//
//// Comma operator to go with __VA_ARGS__
//
//template <typename T>
//std::ostream& operator,(std::ostream & out, const T & t) {
//    out << t;
//    return out;
//}
//
//// Overloaded version to handle all those special std::endl and others...
//// inline to stop compiler crying about ODR
//
//inline std::ostream& operator,(std::ostream& out, std::ostream& (*f)(std::ostream&)) {
//    out << f;
//    return out;
//}

