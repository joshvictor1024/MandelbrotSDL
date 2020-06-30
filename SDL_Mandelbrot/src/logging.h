#ifndef LOGGING_H
#define LOGGING_H


#include <iostream>

#ifdef _DEBUG

//debug log
#define DLOG(message); std::cout << message << "\n(line:" << __LINE__ << ' ' << __FILE__ << ")\n";
//release log
#define RLOG(message); std::cout << message << "\n(line:" << __LINE__ << ' ' << __FILE__ << ")\n";
//debug assert
#define DASSERT(x); if ( !(x) ) { DLOG( "Debug Assertion: ( " << #x << " ) failed!") }


#else

#define DLOG(message);
#define RLOG(message); std::cout << message << "\n";
#define DASSERT(x, message);

#endif // _DEBUG


#endif // !LOGGING_H

