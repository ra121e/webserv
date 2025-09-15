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

	void setTimer(time_t duration) const;
};

#endif // TIMER_HPP
