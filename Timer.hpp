#ifndef TIMER_HPP
#define TIMER_HPP

#include <sys/timerfd.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include "BaseFile.hpp"

class Timer : public BaseFile
{
public:
	Timer();
	Timer(const Timer& other);
	Timer& operator=(const Timer& other);
	~Timer();
};

#endif // TIMER_HPP
