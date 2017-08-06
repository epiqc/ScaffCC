#ifndef _INC_EXCEPTION_H
#define _INC_EXCEPTION_H

#include "precision.h"

#include <string>

class Exception 
{
public:
	std::string msg;
	Exception(std::string m) : msg(m) {}
	Exception(const char *m) : msg(m) {}
	~Exception() {}
};

#endif
